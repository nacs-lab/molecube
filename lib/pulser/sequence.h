#include "program.h"

#include <nacs-utils/log.h>

#include <stdexcept>

#ifndef __NACS_PULSER_SEQUENCE_H__
#define __NACS_PULSER_SEQUENCE_H__

namespace NaCs {
namespace Pulser {

class NACS_EXPORT SequenceBuilder : public Program {
    unsigned m_line_num;
    bool m_has_ttl;
    uint64_t m_curr_t;
    uint32_t m_next_ttl;
    uint32_t m_curr_ttl;
public:
    SequenceBuilder();
    unsigned &line_num();
    bool &has_ttl();
    uint64_t &curr_t();
    uint32_t &next_ttl();
    uint32_t &curr_ttl();

    bool ttl_changed() const;

    void ttl_pulse(uint64_t t, uint32_t ttl);
    void handle_curr_ttl(uint64_t t_new, uint64_t t_min);

    void push_ttl(uint64_t t, unsigned chn, bool val);
    void push_ttl_all(uint64_t t, uint32_t val);
    void finish_ttl();
private:
    void make_curr_ttl(uint64_t t_end);
};

inline void
SequenceBuilder::finish_ttl()
{
    if (m_has_ttl) {
        //disable timing check prior to last pulse
        disable_timing_check();
        ttl_pulse(PULSER_T_TTL_MIN, m_next_ttl);
    }
    m_curr_ttl = m_next_ttl;
}

inline void
SequenceBuilder::push_ttl_all(uint64_t t, uint32_t val)
{
    if (m_has_ttl) {
        make_curr_ttl(t);
    }

    m_next_ttl = val;
    m_curr_t = t;
    m_has_ttl = true;
}

inline void
SequenceBuilder::push_ttl(uint64_t t, unsigned chn, bool val)
{
    push_ttl_all(t, nacsSetBit(m_next_ttl, chn, val));
}

/**
 * SequenceBuilder::handle_curr_ttl:
 * @t_new: spec'd time for new pulse
 * @t_min: minimum duration of new pulse *before* update
 */
inline void
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
            nacsError("Pulse too short @ t = %.2f us.\n",
                      t_new * PULSER_DT_us);
            nacsError("Previous t = %.2f us.  Earliest is t = %.2f us.\n",
                      m_curr_t * PULSER_DT_us, t_start * PULSER_DT_us);
            throw std::runtime_error("The pulse at t = " +
                                     std::to_string(t_new * PULSER_DT_us) +
                                     " us is too early.  ");
        } else if (t_new > UINT32_MAX) {
            nacsError("timer overflow @ t = %.2f us.\n", t_new * PULSER_DT_us);
            throw std::runtime_error("Sequence too long.");
        }

        ttl_pulse(t_new - m_curr_t - t_min, m_next_ttl);
        m_curr_t = t_new - t_min;
        m_curr_ttl = m_next_ttl;
    } else {
        throw std::runtime_error("Must run a TTL pulse first.");
    }
}

inline
SequenceBuilder::SequenceBuilder()
    : Program(),
      m_line_num(0),
      m_has_ttl(false),
      m_curr_t(0),
      m_next_ttl(0),
      m_curr_ttl(0)
{
}

inline void
SequenceBuilder::ttl_pulse(uint64_t t, uint32_t ttl)
{
    if (t < PULSER_T_TTL_MIN) {
        nacsError("TTL pulse 0x%08X too short: %.2f us\n",
                  ttl, t * PULSER_DT_us);
        throw std::runtime_error("The pulse at t = " +
                                 std::to_string(t * PULSER_DT_us) +
                                 " us is too short or early.");
    }
    pulse(t, 0, ttl);
}

inline void
SequenceBuilder::make_curr_ttl(uint64_t t_end)
{
    if (t_end < m_curr_t + PULSER_T_TTL_MIN) {
        nacsError("TTL pulse too short at t_end = %.2f us.\n",
                  t_end * PULSER_DT_us);
        nacsError("Previous t = %.2f us.  Earliest is %.2f us.\n",
                  m_curr_t * PULSER_DT_us,
                  (PULSER_T_TTL_MIN + m_curr_t) * PULSER_DT_us);
        throw std::runtime_error("The pulse at t = " +
                                 std::to_string(t_end * PULSER_DT_us) +
                                 " us is too early.");
    }
    ttl_pulse(t_end - m_curr_t, m_next_ttl);
    m_curr_ttl = m_next_ttl;
}

inline bool
SequenceBuilder::ttl_changed() const
{
    return m_next_ttl != m_curr_ttl;
}

inline unsigned&
SequenceBuilder::line_num()
{
    return m_line_num;
}

inline bool&
SequenceBuilder::has_ttl()
{
    return m_has_ttl;
}

inline uint64_t&
SequenceBuilder::curr_t()
{
    return m_curr_t;
}

inline uint32_t&
SequenceBuilder::next_ttl()
{
    return m_next_ttl;
}

inline uint32_t&
SequenceBuilder::curr_ttl()
{
    return m_curr_ttl;
}

}
}

#endif
