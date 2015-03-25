#ifndef __NACS_PULSER_CONTROLLER_H__
#define __NACS_PULSER_CONTROLLER_H__

#include <nacs-utils/container.h>

#include "driver.h"

#include <condition_variable>
#include <mutex>
#include <atomic>
#include <type_traits>

namespace NaCs {
namespace Pulser {

class Controller;

struct Request {
    bool ready;
    // Result
    uint32_t res;
    const uint8_t cond_id;
    // Keep this as is until we have variable length requests
    const uint32_t ctrl;
    const uint32_t op;
    Request(Controller &, uint32_t ctrl, uint32_t op);
private:
    Request() = delete;
};

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
          m_cond_vars{},
          m_cond_locks{}
    {
    }
    uint8_t
    getCondId()
    {
        return uint8_t(m_cond_id.fetch_add(1) & numLocksMask);
    }
    void
    wait(const Request &req)
    {
        std::unique_lock<std::mutex> locker(m_cond_locks[req.cond_id]);
        m_cond_vars[req.cond_id].wait(locker, [&] {
                return req.ready;
            });
    }
    void
    pushReq(Request &req)
    {
        m_req_queue.push(&req);
    }
    template<typename R>
    std::enable_if_t<std::is_base_of<Request,
                                     std::remove_reference_t<R> >::value,
                     uint32_t>
    reqSync(R &&req)
    {
        pushReq(req);
        wait(req);
        return req.res;
    }
    void
    setRes(Request &req, uint32_t res)
    {
        {
            std::lock_guard<std::mutex> locker(m_cond_locks[req.cond_id]);
            req.res = res;
            req.ready = true;
        }
        m_cond_vars[req.cond_id].notify_all();
    }
private:
    size_t m_num_read;
    size_t m_num_written;
    FIFO<Request*> m_res_queue;
    FIFO<Request*> m_req_queue;
    mutable std::condition_variable m_cond_vars[numLocks];
    mutable std::mutex m_cond_locks[numLocks];
    mutable std::atomic_uint m_cond_id;
};

inline
Request::Request(Controller &ctrl, uint32_t _ctrl, uint32_t _op)
    : ready(false),
      res(0),
      cond_id(ctrl.getCondId()),
      ctrl(_ctrl),
      op(_op)
{
}

}
}

#endif
