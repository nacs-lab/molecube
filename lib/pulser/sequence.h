#include "program.h"

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

    void handle_curr_ttl(uint64_t t_new, uint64_t t_min);
    void push_ttl(uint64_t t, unsigned chn, bool val);
    void push_ttl_all(uint64_t t, uint32_t val);
    void finish_ttl();
private:
    void ttl_pulse(uint64_t t, uint32_t ttl);
    void make_curr_ttl(uint64_t t_end);
};

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
