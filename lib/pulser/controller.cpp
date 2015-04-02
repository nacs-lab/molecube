#include "controller.h"

#include <nacs-utils/log.h>

namespace NaCs {
namespace Pulser {

NACS_EXPORT void
Controller::init(bool reset)
{
    releaseHold();
    FILE *log_f = nacsGetLog();
    fprintf(log_f, "PULSE_CONTROLLER registers:\n");
    for (unsigned i = 0;i < 31;i++) {
        if (i % 4 == 0) {
            fprintf(log_f, "[%2d...%2d]: ", i, i + 3);
        }
        fprintf(log_f, "%08X ", readReg(i));
        if (i % 4 == 3) {
            fprintf(log_f, "\n");
        }
    }
    fprintf(log_f, "\n");

    if (reset) {
        nacsInfo("PULSER_init... reset DDS\n");
        for (unsigned i = 0;i < PULSER_NDDS;i++) {
            run(DDSReset(i));
        }
    }
}

/**
 * Wait for the request to finish.
 */
NACS_EXPORT void
Controller::wait(const Request &req)
{
    std::unique_lock<std::mutex> locker(m_cond_locks[req.cond_id]);
    m_cond_vars[req.cond_id].wait(locker, [&] {
            return req.ready;
        });
}

/**
 * (Mainly) For the reader thread.
 * Set the result (and ready) of a request.
 */
NACS_EXPORT void
Controller::setRes(Request &req, uint32_t res)
{
    {
        std::lock_guard<std::mutex> locker(m_cond_locks[req.cond_id]);
        req.res = res;
        req.ready = true;
    }
    m_cond_vars[req.cond_id].notify_all();
}

/**
 * For reader thread.
 * Pop all the results that are in the result buffer.
 */
uint32_t
Controller::popResults()
{
    static constexpr uint32_t buf_size = 32;
    uint32_t results[buf_size];
    // numResults should always be smaller than 32, the min() here makes
    // it safer if the result buffer is extended in the future and the
    // compiler can probably optimize it away since numResults is inlined
    const uint32_t n_res = min(numResults(), buf_size);
    if (!n_res) {
        std::this_thread::yield();
        return 0;
    }
    for (uint32_t i = 0;i < n_res;i++) {
        results[i] = readResult();
    }
    // Atomic increment is not necessary since there should only be one
    // thread writing to it
    {
        std::lock_guard<std::mutex> locker(m_writer_lock);
        m_num_read = m_num_read + n_res;
    }
    // There should be enough requests queued in the result queue
    Request *reqs[buf_size];
    auto num_pop = m_res_queue.pop(reqs, n_res);
    assert(num_pop == n_res);
    m_writer_cond.notify_all();
    for (uint32_t i = 0;i < n_res;i++) {
        setRes(*reqs[i], results[i]);
    }
    return n_res;
}

/**
 * For reader thread.
 * Clear the queue of the write only request queue.
 */
void
Controller::dumpNotifyQueue()
{
    static constexpr uint32_t buf_size = 32;
    Request *reqs[buf_size];
    auto num_pop = m_notify_queue.pop(reqs, buf_size);
    for (uint32_t i = 0;i < num_pop;i++) {
        setRes(*reqs[i], 0);
    }
}

/**
 * Read all remaining results
 */
uint32_t
Controller::popRemaining()
{
    uint32_t n_read = 0;
    // Should be safe even in case of overflow since the difference between
    // them should be kept small by the writter thread
    while (int(m_num_written - m_num_read) > 0 && !m_quit) {
        n_read += popResults();
        std::this_thread::yield();
    }
    dumpNotifyQueue();
    return n_read;
}

/**
 * Wait for event or timeout to thread the result buffer from FPGA
 */
NACS_EXPORT void
Controller::runReader()
{
    while (!m_quit) {
        std::unique_lock<std::mutex> locker(m_reader_lock);
        // Sleep for a shorter time if there's pending requests
        m_reader_cond.wait_for(locker, m_req_queue.size() ? 200us : 2000us);
        popRemaining();
    }
}

/**
 * Write at most @max_num requests (each at most 500us long) to FPGA.
 * @notify controls whether to notify other threads of the write being done.
 * Use `false` for RT thread.
 *
 * Return: the length of the pulese written.
 */
uint64_t
Controller::writeRequests(uint32_t max_num, bool notify, uint32_t flags)
{
    static constexpr int max_write = 15;
    int res_buff_space = resBuffSpace();
    if (res_buff_space <= 0)
        return 0;
    // Block writer if there's not enough space in the result buffer
    // This also means that requests that does not return a result will be
    // blocked too. Probably not a big issue.
    uint32_t num_to_write = min(max_num, uint32_t(res_buff_space));
    if (num_to_write == 0)
        return 0;
    Request *reqs[max_write];
    num_to_write = m_req_queue.pop(reqs, num_to_write);
    uint64_t total_time = 0;
    uint32_t num_return = 0;
    for (uint32_t i = 0;i < num_to_write;i++) {
        Request *req = reqs[i];
        if (req->has_res) {
            m_res_queue.push(req);
            num_return++;
        }
        total_time += req->length;
        shortPulse(req->ctrl | flags, req->op);
        if (!req->has_res) {
            // Notify the requester or push it to the notify queue after
            // the it has been written to the FPGA since the request is invalid
            // (might be freed / destructed at any time) after that.
            if (notify) {
                // If notify is enabled for the writer thread (i.e. this is
                // not a RT thread), notify the requester directly.
                setRes(*req, 0);
            } else {
                m_notify_queue.push(req);
            }
        }
    }
    if (num_return) {
        m_num_written = m_num_written + num_return;
        if (notify) {
            // If notify is on, the reader thread only need to be waken up
            // for the requests that return results.
            m_reader_cond.notify_all();
        }
    }
    return total_time;
}

/**
 * Write requests
 */
NACS_EXPORT void
Controller::runWriter()
{
    while (!m_quit) {
        {
            std::unique_lock<std::mutex> locker(m_writer_lock);
            m_writer_cond.wait(locker, [&] {
                    return m_quit || (resBuffSpace() > 0 &&
                                      m_req_queue.size());
                });
        }
        std::lock_guard<std::mutex> controller_locker(m_lock);
        writeRequests(32, true);
    }
}

}
}
