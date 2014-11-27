#include "pulser.h"

#ifndef __NACS_PULSER_PROGRAM_H__
#define __NACS_PULSER_PROGRAM_H__

namespace NaCs {
namespace Pulser {

class NACS_EXPORT BaseProgram {
    uint32_t *m_prog;
    size_t m_len;
    size_t m_alloc_len;
public:
    BaseProgram()
        : m_prog(nullptr),
          m_len(0),
          m_alloc_len(0)
    {
    }
    ~BaseProgram();
    inline const uint32_t*
    program() const
    {
        return m_prog;
    }
    inline size_t
    len() const
    {
        return m_len;
    }
protected:
    template <typename... ArgTypes>
    inline void
    writes(ArgTypes&&... args)
    {
        const size_t nargs = sizeof...(args);
        uint32_t data[] = {args...};
        reserve_space(nargs);
        memcpy(m_prog + m_len, data, nargs * sizeof(uint32_t));
        m_len += nargs;
    }
private:
    void reserve_space(size_t len);
};

class NACS_EXPORT Program : public BaseProgram, public PulserBase {
    uint32_t m_flags;
    uint16_t m_phases[32];
public:
    Program()
        : BaseProgram(),
          m_flags(0)
    {
    }
    void short_pulse(uint32_t control, uint32_t operand) override;
    void enable_timing_check();
    void disable_timing_check();
    void dds_reset(int i);
    void set_dds_phase(int i, uint16_t phase);
    void shift_dds_phase(int i, uint16_t phase);
private:
    void write_reg(unsigned reg, uint32_t val) override;
};

}
}

#endif
