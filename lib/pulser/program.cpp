#include "program.h"
#include "ctrl_io.h"

#include <nacs-utils/number.h>
#include <nacs-utils/log.h>
#include <nacs-utils/timer.h>

#include <stdexcept>

namespace NaCs {
namespace Pulser {

NACS_EXPORT
BaseProgram::~BaseProgram()
{
    free(m_prog);
}

NACS_EXPORT void
BaseProgram::reserve_space(size_t len)
{
    size_t new_len = m_len + len;
    if (nacsLikely(new_len <= m_alloc_len)) {
        return;
    }
    const size_t block_size = 4096;
    m_alloc_len = alignTo(new_len, block_size);
    m_prog = (uint32_t*)realloc(m_prog, m_alloc_len * sizeof(uint32_t));
}

NACS_EXPORT void
Program::write_reg(unsigned reg, uint32_t val)
{
    if (reg >= 32) {
        throw std::runtime_error("Register number out of range.");
    }
    if (log_on()) {
        nacsLog("Write Register(%u), %" PRIx32 "\n", reg, val);
    }
    writes(reg, val);
}

void
Program::short_pulse(uint32_t control, uint32_t operand)
{
    PulserBase::short_pulse(control | m_flags, operand);
}

#define ENABLE_TIMING_CHECK (0x08000000)

// enable timing check for pulses
NACS_EXPORT void
Program::enable_timing_check()
{
    if (log_on()) {
        nacsLog("Enable timing check\n");
    }
    m_flags = m_flags | ENABLE_TIMING_CHECK;
}

// disable timing check for pulses
NACS_EXPORT void
Program::disable_timing_check()
{
    if (log_on()) {
        nacsLog("Disable timing check\n");
    }
    m_flags = m_flags & ~ENABLE_TIMING_CHECK;
}

NACS_EXPORT void
Program::dds_reset(int i)
{
    PulserBase::dds_reset(i);
    m_phases[i] = 0;
}

NACS_EXPORT void
Program::set_dds_phase(int i, uint16_t phase)
{
    PulserBase::set_dds_phase(i, phase);
    m_phases[i] = phase;
}

NACS_EXPORT void
Program::shift_dds_phase(int i, uint16_t phase)
{
    if (log_on()) {
        nacsLog("Shift DDS(%i) phase %" PRId16 "\n", i, phase);
    }
    LogHolder holder;
    // TODO: let's see what is the ``documented'' behavior of the set
    // phase command
    m_phases[i] = uint16_t(m_phases[i] + phase);
    set_dds_phase(i, m_phases[i]);
}

}
}
