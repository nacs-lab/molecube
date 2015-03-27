#ifndef __NACS_PULSER_CONTROLLER_H__
#define __NACS_PULSER_CONTROLLER_H__

#include "driver.h"
#include "commands.h"

#include <nacs-utils/utils.h>
#include <nacs-utils/container.h>

#include <condition_variable>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>

#include <assert.h>

namespace NaCs {
namespace Pulser {

class Controller;

using namespace std::literals;

/**
 * Each request should be writing two 32-bit words to the FIFO (slave reg 31)
 * and should last for no more than 500ns, the precise length of the pulse
 * is stored in `length` (< 50) in unit of FPGA clock (10ns)
 */
class Request {
    struct RequestFactory {
        uint32_t m_ctrl;
        uint32_t m_op;
        template<typename Cmd>
        RequestFactory(Cmd &&cmd)
        {
            cmd.run(*this);
        }
        inline void
        shortPulse(uint32_t ctrl, uint32_t op)
        {
            m_ctrl = ctrl;
            m_op = op;
        }
    };
    Request(Controller &ctrl, RequestFactory &&req, uint8_t len, bool has_res)
        : Request(ctrl, req.m_ctrl, req.m_op, len, has_res)
    {}
public:
    bool ready: 1;
    const bool has_res: 1;
    const uint8_t length;
    const uint8_t cond_id;
    // Result
    uint32_t res;
    // Keep this as is until we have variable length requests
    const uint32_t ctrl;
    const uint32_t op;
    Request(Controller&, uint32_t ctrl, uint32_t op,
            uint8_t len, bool _has_res);
    template<typename Cmd, class=std::enable_if_t<isSimpleCmd<Cmd> > >
    Request(Controller &ctrl, Cmd &&cmd)
        : Request(ctrl, RequestFactory(cmd),
                  uint8_t(cmd.length()), cmd.has_res)
    {
    }
private:
    Request() = delete;
};

/**
 * This is the object that manages the threads that talks to the FPGA.
 */
class Controller: public Driver {
    static constexpr unsigned numLocks = 32;
    static constexpr unsigned numLocksMask = numLocks - 1;
public:
    Controller(volatile void *base)
        : Driver(base),
          m_num_read(0),
          m_num_written(0),
          m_res_queue(64),
          m_req_queue(64),
          m_notify_queue(64),
          m_cond_vars{},
          m_cond_locks{},
          m_quit(false),
          m_reader_thread(&Controller::runReader, this),
          m_reader_cond(),
          m_reader_lock(),
          m_writer_cond(),
          m_writer_lock()
    {}
    ~Controller()
    {
        m_quit = true;
        m_reader_thread.join();
    }

    // For creating requests
    inline uint8_t
    getCondId()
    {
        return uint8_t(m_cond_id.fetch_add(1) & numLocksMask);
    }

    // For requester
    void wait(const Request &req);

    inline void
    pushReq(Request &req)
    {
        m_req_queue.push(&req);
    }

    // Send a request and wait for it to finish
    template<typename R>
    std::enable_if_t<isBaseOf<Request, R>, uint32_t>
    reqSync(R &&req)
    {
        pushReq(req);
        wait(req);
        return req.res;
    }
    template<typename Cmd>
    std::enable_if_t<isSimpleCmd<Cmd>, uint32_t>
    reqSync(Cmd &&cmd)
    {
        return reqSync(Request(cmd));
    }

    // For result reader
    void setRes(Request &req, uint32_t res);
private:
    uint32_t popResults();
    void dumpNotifyQueue();
    uint32_t popRemaining();
    void runReader();

    /**
     * For a request that returns a result, the writer should first push it
     * into the result queue (`m_res_queue`), then write the request to the
     * FPGA, and increment m_num_written.
     * For a request that does not return a result, the writer should write the
     * request first and then either push it to the list to be freed or notify
     * the requester directly.
     */
    uint64_t writeRequests(uint32_t max_num, bool notify);

    /**
     * Use atomic_uint for num_read and num_written to ensure atomic load and
     * write (which is hopefully trivial).
     *
     * @m_num_read: number of results read
     * @m_num_written: number of request that returns a result written
     *     (i.e.) the number of results expected
     *
     * It is important to keep the difference between these small since the
     * the FPGA only has a finite result buffer size.
     */
    std::atomic_uint m_num_read;
    std::atomic_uint m_num_written;

    /**
     * @m_res_queue: queue of written requests
     * @m_req_queue: queue of requests to be written
     */
    FIFO<Request*> m_res_queue;
    FIFO<Request*> m_req_queue;
    /**
     * @m_notify_queue: For write only request, the time it cost to do a
     *     notify might be a little too expensive for the real time writer
     *     thread. Therefore, although the request is technically done when it
     *     is written to the FPGA, we push it to a queue and let the reader
     *     thread send the notification.
     */
    FIFO<Request*> m_notify_queue;

    /**
     * @m_cond_vars
     * @m_cond_locks
     *     Condition variables to notify the requester of finished request
     *     Split into multiple bins so that we don't need to create a new
     *     condition variable (and mutex) for each single request, which can
     *     be too expensive for RT thread or wake up everyone for every events
     *     when there are more then one pending requests.
     */
    mutable std::condition_variable m_cond_vars[numLocks];
    mutable std::mutex m_cond_locks[numLocks];
    /**
     * m_cond_id: counter to assign condition variables.
     */
    mutable std::atomic_uint m_cond_id;

    /**
     * @m_quit: the controller is (being) destructed and the helper thread(s)
     *     should all quit.
     */
    bool m_quit;
    /**
     * For the reader thread
     *
     * @m_reader_thread: Helper thread that reads values from the FPGA
     *     continiously
     * @m_reader_cond:
     * @m_reader_lock: For notifying the reader that some requests has been
     *     written to the FPGA. This might not happen for all requests written
     *     (e.g. when RT thread write a request that does not need immediate
     *     result) so the reader thread should timeout on the wait and check
     *     if there's anything to read.
     */
    std::thread m_reader_thread;
    mutable std::condition_variable m_reader_cond;
    mutable std::mutex m_reader_lock;

    /**
     * For the writer thread
     *
     * @m_writer_cond:
     * @m_writer_lock: For notifying the writer that some results has been
     *     read from the FPGA so that it can write more requests in case
     *     it was waiting for enough space in the result buffer.
     */
    mutable std::condition_variable m_writer_cond;
    mutable std::mutex m_writer_lock;
};

inline
Request::Request(Controller &ctrl, uint32_t _ctrl, uint32_t _op,
                 uint8_t len, bool _has_res)
    : ready(false),
      has_res(_has_res),
      length(len),
      cond_id(ctrl.getCondId()),
      res(0),
      ctrl(_ctrl),
      op(_op)
{}

}
}

#endif
