#include "pulser.h"
#include <nacs-utils/log.h>

#ifndef __NACS_PULSER_PROGRAM_H__
#define __NACS_PULSER_PROGRAM_H__

namespace NaCs {
namespace Pulser {

class NACS_EXPORT BaseProgram {
    uint32_t *m_prog;
    size_t m_len;
    size_t m_alloc_len;
    BaseProgram(const BaseProgram&) = delete;
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
        // for (unsigned i = 0;i < nargs;i++) {
        //     nacsLog("Write prog[%zu] = %x\n", m_len + i, data[i]);
        // }
        m_len += nargs;
    }
private:
    void reserve_space(size_t len);
};

class NACS_EXPORT Program : public BaseProgram, public PulserBase {
    uint32_t m_flags;
    uint16_t m_phases[32];
public:
    Program(bool debug=false);
    void short_pulse(uint32_t control, uint32_t operand) override;
    void enable_timing_check();
    void disable_timing_check();
    void dds_reset(int i);
    void set_dds_phase(int i, uint16_t phase);
    void shift_dds_phase(int i, uint16_t phase);
    void shift_dds_phase_f(int i, double f);
private:
    void write_reg(unsigned reg, uint32_t val) override;
};

NACS_INLINE
Program::Program(bool debug)
    : BaseProgram(),
      PulserBase(debug),
      m_flags(0)
{
}

NACS_INLINE void
Program::shift_dds_phase_f(int i, double p)
{
    shift_dds_phase(i, DDSConverter::phase2num(p));
}

}
}

#endif
