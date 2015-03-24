#ifndef __NACS_PULSER_COMMANDS_P_H__
#define __NACS_PULSER_COMMANDS_P_H__

#include "converter.h"
#include <nacs-old-pulser/pulser-config.h>

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
    uint32_t dds_addr = (addr + 1) & 0x7f;
    // put data in bits 15...0 (maps to DDS operand_reg[15:0] )?
    uint32_t dds_data = data & 0xffff;
    v.shortPulse(0x10000002 | (i << 4) | (dds_addr << 9), dds_data);
}

// set bytes addr + 3 ... addr
template<typename T>
static inline void
setDDSFourBytes(T &v, int i, uint32_t addr, uint32_t data)
{
    //put addr in bits 15...9 (maps to DDS opcode_reg[14:9])?
    uint32_t dds_addr = (addr + 1) & 0x7f;
    v.shortPulse(0x1000000f | (i << 4) | (dds_addr << 9), data);
}

// make timed pulses
// if t > t_max, subdivide into shorter pulses
// returns number of pulses made
template<typename T>
static inline void
makePulse(T &v, uint64_t t, unsigned flags, unsigned operand)
{
    static constexpr uint32_t t_max = 0x001fffff;
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

template<typename T>
static inline void
setDDSAmp(T &v, int i, uint32_t amp)
{
    setDDSTwoBytes(v, i, 0x32, amp);
}

template<typename T>
static inline void
setDDSAmpF(T &v, int i, double amp)
{
    setDDSAmp(v, i, DDSConverter::amp2num(amp));
}

template<typename T>
static inline void
setDDSPhase(T &v, int i, uint16_t phase)
{
    setDDSTwoBytes(v, i, 0x30, phase);
}

template<typename T>
static inline void
ddsReset(T &v, int i)
{
    v.shortPulse(0x10000004 | (i << 4), 0);
}

// reset DDS selected by bitmask mask
template<typename T>
static inline void
resetDDSSel(T &v, uint32_t mask)
{
    v.shortPulse(0x10000005, mask);
}

template<typename T>
static inline void
setDDSSel(T &v, uint32_t mask)
{
    v.shortPulse(0x10000006, mask);
}

// get byte from address on DDS i
template<typename T>
static inline void
ddsByteReq(T &v, int i, uint32_t address)
{
    v.shortPulse(0x10000003 | (i << 4) | (address << 9), 0);
}

// get two bytes from address + 1 ... adress on DDS i
template<typename T>
static inline void
ddsTwoBytesReq(T &v, int i, uint32_t address)
{
    v.shortPulse(0x10000003 | (i << 4) | ((address + 1) << 9), 0);
}

// get four bytes from address + 3 ... adress on DDS i
template<typename T>
static inline void
ddsFourBytesReq(T &v, int i, uint32_t address)
{
    v.shortPulse(0x1000000e | (i << 4) | ((address + 1) << 9), 0);
}

}
}

#endif
