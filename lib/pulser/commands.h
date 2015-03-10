#ifndef __NACS_PULSER_COMMANDS_P_H__
#define __NACS_PULSER_COMMANDS_P_H__

#include "converter.h"
#include <nacs-pulser/pulser-config.h>

#include <nacs-utils/number.h>

namespace NaCs {
namespace Pulser {

// enable / disable clock_out
// divider = 0..254 means emit clock with period 2 x (divider + 1)
// in pulse controller timing units (DT_ns)
// divider = 255 means disable
template<typename T>
static inline void
clockOut(T &v, uint32_t divider)
{
    v.shortPulse(0x50000000, divider & 0xff);
}

// set bytes at addr + 1 and addr
// note that get_dds_two bytes also returns data at addr+1 and addr
template<typename T>
static inline void
setDDSTwoBytes(T &v, int i, uint32_t addr, uint32_t data)
{
    // put addr in bits 15...9 (maps to DDS opcode_reg[14:9] )?
    uint32_t dds_addr = (addr + 1) & 0x7F;
    // put data in bits 15...0 (maps to DDS operand_reg[15:0] )?
    uint32_t dds_data = data & 0xFFFF;
    v.shortPulse(0x10000002 | (i << 4) | (dds_addr << 9), dds_data);
}

// set bytes addr + 3 ... addr
template<typename T>
static inline void
setDDSFourBytes(T &v, int i, uint32_t addr, uint32_t data)
{
    //put addr in bits 15...9 (maps to DDS opcode_reg[14:9])?
    uint32_t dds_addr = (addr + 1) & 0x7F;
    v.shortPulse(0x1000000F | (i << 4) | (dds_addr << 9), data);
}

// make timed pulses
// if t > t_max, subdivide into shorter pulses
// returns number of pulses made
template<typename T>
static inline void
makePulse(T &v, uint64_t t, unsigned flags, unsigned operand)
{
    static constexpr uint32_t t_max = 0x001FFFFF;
    do {
        uint32_t t_step = uint32_t(min(t, t_max));
        v.shortPulse(t_step | flags, operand);
        t -= t_step;
    } while (t > 0);
}

// clear timing check (clear failures)
template<typename T>
static inline void
clearTimingCheck(T &v)
{
    v.shortPulse(0x30000000, 0);
}

template<typename T>
static inline void
setDDSFreq(T &v, int i, uint32_t ftw)
{
    v.shortPulse(0x10000000 | (i << 4), ftw);
}

template<typename T>
static inline void
setDDSFreqF(T &v, int i, double f)
{
    setDDSFreq(v, i, DDSConverter::freq2num(f, PULSER_AD9914_CLK));
}

}
}

#endif
