#include "sequence.h"

#include <nacs-utils/log.h>
#include <nacs-utils/timer.h>

#include <stdexcept>

namespace NaCs {
namespace Pulser {

NACS_EXPORT void
SequenceBuilder::finish_ttl()
{
    if (log_on()) {
        nacsLog("Finish TTL\n");
    }
    LogHolder holder();
    if (m_has_ttl) {
        // disable timing check prior to last pulse
        disable_timing_check();
        ttl_pulse(PULSER_T_TTL_MIN, m_next_ttl);
    }
    m_curr_ttl = m_next_ttl;
}

NACS_EXPORT void
SequenceBuilder::push_ttl_all(uint64_t t, uint32_t val)
{
    if (log_on()) {
        nacsLog("Push TTL all t=%" PRTime ", val=%" PRIx32 "\n", t, val);
    }
    LogHolder holder();
    if (m_has_ttl) {
        make_curr_ttl(t);
    }

    m_next_ttl = val;
    m_curr_t = t;
    m_has_ttl = true;
}

NACS_EXPORT void
SequenceBuilder::push_ttl(uint64_t t, unsigned chn, bool val)
{
    if (log_on()) {
        nacsLog("Push TTL(%u) t=%" PRTime ", val=%s\n", chn, t,
                val ? "true" : "false");
    }
    LogHolder holder();
    push_ttl_all(t, setBit(m_next_ttl, uint8_t(chn), val));
}

/**
 * SequenceBuilder::handle_curr_ttl:
 * @t_new: spec'd time for new pulse
 * @t_min: minimum duration of new pulse *before* update
 */
NACS_EXPORT void
SequenceBuilder::handle_curr_ttl(uint64_t t_new, uint64_t t_min)
{
    if (has_ttl()) {
        uint64_t t_start = m_curr_t + t_min;

        if (ttl_changed()) {
            t_start += PULSER_T_TTL_MIN;
        } else if (t_start == t_new) {
            return;
        }

        if (t_new < t_start) {
            nacsError("Pulse too short @ t = %.2Lf us.\n",
                      (long double)t_new * PULSER_DT_us);
            nacsError("Previous t = %.2Lf us.  Earliest is t = %.2Lf us.\n",
                      (long double)m_curr_t * PULSER_DT_us,
                      (long double)t_start * PULSER_DT_us);
            throw std::runtime_error("The pulse at t = " +
                                     std::to_string((long double)t_new *
                                                    PULSER_DT_us) +
                                     " us is too early.  ");
        }

        ttl_pulse(t_new - m_curr_t - t_min, m_next_ttl);
        m_curr_t = t_new - t_min;
        m_curr_ttl = m_next_ttl;
    } else {
        throw std::runtime_error("Must run a TTL pulse first.");
    }
}

NACS_EXPORT void
SequenceBuilder::ttl_pulse(uint64_t t, uint32_t ttl)
{
    if (t < PULSER_T_TTL_MIN) {
        nacsError("TTL pulse 0x%08X too short: %.2Lf us\n",
                  ttl, (long double)t * PULSER_DT_us);
        throw std::runtime_error("The pulse at t = " +
                                 std::to_string((long double)t *
                                                PULSER_DT_us) +
                                 " us is too short or early.");
    }
    pulse(t, 0, ttl);
}

NACS_EXPORT void
SequenceBuilder::make_curr_ttl(uint64_t t_end)
{
    if (t_end < m_curr_t + PULSER_T_TTL_MIN) {
        nacsError("TTL pulse too short at t_end = %.2Lf us.\n",
                  (long double)t_end * PULSER_DT_us);
        nacsError("Previous t = %.2Lf us.  Earliest is %.2Lf us.\n",
                  (long double)m_curr_t * PULSER_DT_us,
                  (long double)(PULSER_T_TTL_MIN + m_curr_t) * PULSER_DT_us);
        throw std::runtime_error("The pulse at t = " +
                                 std::to_string((long double)t_end *
                                                PULSER_DT_us) +
                                 " us is too early.");
    }
    ttl_pulse(t_end - m_curr_t, m_next_ttl);
    m_curr_ttl = m_next_ttl;
}

}
}
