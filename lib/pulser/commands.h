#ifndef __NACS_PULSER_COMMANDS_H__
#define __NACS_PULSER_COMMANDS_H__

#include "converter.h"
#include <nacs-pulser/pulser-config.h>
#include <nacs-utils/number.h>

#include <type_traits>

namespace NaCs {
namespace Pulser {

template<bool _has_res>
struct BaseCmd {
    static constexpr bool has_res = _has_res;
};

template<bool has_res>
struct SimpleCmd : BaseCmd<has_res> {
    SimpleCmd(uint32_t ctrl, uint32_t op, uint64_t len)
        : m_ctrl(ctrl), m_op(op), m_len(len)
    {}
    constexpr uint64_t
    length() const
    {
        return m_len;
    }
    template<typename T>
    inline void
    run(T &v) const
    {
        v.shortPulse(m_ctrl, m_op);
    }
private:
    const uint32_t m_ctrl;
    const uint32_t m_op;
    const uint64_t m_len;
};

template<template<bool> class CmdType, typename T, class=void>
struct _isCmdType : std::false_type {};

template<template<bool> class CmdType, typename T>
struct _isCmdType<CmdType, T,
                  std::enable_if_t<
                      std::is_base_of<
                          CmdType<std::remove_reference_t<T>::has_res>,
                          std::remove_reference_t<T>>::value>> :
        std::true_type {
};

template<typename T>
static constexpr bool isBaseCmd = _isCmdType<BaseCmd, T>::value;
template<typename T>
static constexpr bool isSimpleCmd = _isCmdType<SimpleCmd, T>::value;

// enable / disable clock_out
// divider = 0..254 means emit clock with period 2 x (divider + 1)
// in pulse controller timing units (DT_ns)
// divider = 255 means disable
struct ClockOut : SimpleCmd<false> {
    ClockOut(uint32_t divider) :
        SimpleCmd<false>(0x50000000, divider & 0xff, 5)
    {}
};

template<bool has_res>
struct DDSCmd : SimpleCmd<has_res> {
    DDSCmd(uint32_t ctrl, uint32_t op) :
        SimpleCmd<has_res>(0x10000000 | ctrl, op, 50)
    {}
};

// set bytes at addr + 1 and addr
// note that get_dds_two bytes also returns data at addr+1 and addr
struct DDSSetTwoBytes : DDSCmd<false> {
    // put addr in bits 15...9 (maps to DDS opcode_reg[14:9])?
    // put data in bits 15...0 (maps to DDS operand_reg[15:0])?
    DDSSetTwoBytes(int i, uint32_t addr, uint32_t data)
        : DDSCmd<false>(0x2 | (i << 4) | (((addr + 1) & 0x7f) << 9),
                        data & 0xffff)
    {}
};

// set bytes addr + 3 ... addr
struct DDSSetFourBytes : DDSCmd<false> {
    // put addr in bits 15...9 (maps to DDS opcode_reg[14:9])?
    DDSSetFourBytes(int i, uint32_t addr, uint32_t data)
        : DDSCmd<false>(0xf | (i << 4) | (((addr + 1) & 0x7f) << 9), data)
    {}
};

// make timed pulses
// if t > t_max, subdivide into shorter pulses
// returns number of pulses made
struct LongPulse : BaseCmd<false> {
    LongPulse(uint64_t t, unsigned flags, unsigned op)
        : m_t(t), m_flags(flags), m_op(op)
    {}
    constexpr uint64_t
    length() const
    {
        return m_t;
    }
    template<typename T>
    inline void
    run(T &v) const
    {
        static constexpr uint32_t t_max = 0x001fffff;
        auto t = m_t;
        do {
            uint32_t t_step = uint32_t(min(t, t_max));
            v.shortPulse(t_step | m_flags, m_op);
            t -= t_step;
        } while (t > 0);
    }
private:
    const uint64_t m_t;
    const uint32_t m_flags;
    const uint32_t m_op;
};

// clear timing check (clear failures)
struct ClearTimingCheck : SimpleCmd<false> {
    ClearTimingCheck() : SimpleCmd<false>(0x30000000, 0, 5)
    {}
};

struct LoopBack : SimpleCmd<true> {
    LoopBack(uint32_t data) : SimpleCmd<true>(0x40000000, data, 5)
    {}
};

struct DDSSetFreq : DDSCmd<false> {
    DDSSetFreq(int i, uint32_t ftw)
        : DDSCmd<false>(i << 4, ftw)
    {}
};

struct DDSSetFreqF : DDSSetFreq {
    DDSSetFreqF(int i, double f)
        : DDSSetFreq(i, DDSCvt::freq2num(f, PULSER_AD9914_CLK))
    {}
};

struct DDSSetAmp : DDSSetTwoBytes {
    DDSSetAmp(int i, uint32_t amp)
        : DDSSetTwoBytes(i, 0x32, amp)
    {}
};

struct DDSSetAmpF : DDSSetAmp {
    DDSSetAmpF(int i, double amp)
        : DDSSetAmp(i, DDSCvt::amp2num(amp))
    {}
};

struct DDSSetPhase : DDSSetTwoBytes {
    DDSSetPhase(int i, uint16_t phase)
        : DDSSetTwoBytes(i, 0x30, phase)
    {}
};

struct DDSReset : DDSCmd<false> {
    DDSReset(int i)
        : DDSCmd<false>(0x4 | (i << 4), 0)
    {}
};

// reset DDS selected by bitmask mask
struct DDSResetSel : DDSCmd<false> {
    DDSResetSel(uint32_t mask)
        : DDSCmd<false>(0x5, mask)
    {}
};

struct DDSSetSel : DDSCmd<false> {
    DDSSetSel(uint32_t mask)
        : DDSCmd<false>(0x6, mask)
    {}
};

// get byte from address on DDS i
struct DDSByteReq : DDSCmd<true> {
    DDSByteReq(int i, uint32_t addr)
        : DDSCmd<true>(0x3 | (i << 4) | (addr << 9), 0)
    {}
};

// get two bytes from address + 1 ... adress on DDS i
struct DDSTwoBytesReq : DDSCmd<true> {
    DDSTwoBytesReq(int i, uint32_t addr)
        : DDSCmd<true>(0x3 | (i << 4) | ((addr + 1) << 9), 0)
    {}
};

struct DDSFourBytesReq : DDSCmd<true> {
    DDSFourBytesReq(int i, uint32_t addr)
        : DDSCmd<true>(0xe | (i << 4) | ((addr + 1) << 9), 0)
    {}
};

}
}

#endif
