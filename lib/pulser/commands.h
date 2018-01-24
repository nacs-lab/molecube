#ifndef __NACS_PULSER_COMMANDS_H__
#define __NACS_PULSER_COMMANDS_H__

#include "converter.h"
#include <nacs-pulser/pulser-config.h>
#include <nacs-utils/number.h>

namespace NaCs {
namespace Pulser {

template<bool _has_res>
struct BaseCmd {
    static constexpr bool has_res = _has_res;
    static inline uint32_t
    convertRes(uint32_t res)
    {
        return res;
    }
};

template<bool has_res>
struct SimpleCmd : BaseCmd<has_res> {
    constexpr SimpleCmd(uint32_t ctrl, uint32_t op, uint64_t len)
        : m_ctrl(ctrl), m_op(op), m_len(len)
    {}
    constexpr uint64_t
    length() const
    {
        return m_len;
    }
    constexpr uint32_t
    control() const
    {
        return m_ctrl;
    }
    constexpr uint32_t
    operand() const
    {
        return m_op;
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
                      isBaseOf<CmdType<true>, T> ||
                      isBaseOf<CmdType<false>, T>>> : std::true_type {
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
    constexpr ClockOut(uint32_t divider)
        : SimpleCmd<false>(0x50000000, divider & 0xff, 5)
    {}
};

struct SPICmd : SimpleCmd<false> {
private:
    constexpr SPICmd(uint32_t opcode, uint32_t data)
        : SimpleCmd<false>(opcode | 0x60000000, data, 45)
    {}
    static constexpr uint32_t getOpcode(uint8_t clk_div, uint8_t spi_id)
    {
        return ((uint32_t(spi_id & 3) << 11) | clk_div);
    }
public:
    constexpr SPICmd(uint8_t clk_div, uint8_t spi_id, uint32_t data)
        : SPICmd(getOpcode(clk_div, spi_id), data)
    {}
};

struct DACSetVolt : SPICmd {
private:
    static constexpr uint32_t getData(uint8_t dac, uint16_t V)
    {
        return ((dac & 3) << 16) | V;
    }
public:
    constexpr DACSetVolt(uint8_t dac, uint16_t V)
        : SPICmd(0, 0, getData(dac, V))
    {}
};

struct DACSetVoltF : DACSetVolt {
private:
    static constexpr uint16_t getVoltData(double volt)
    {
        if (volt >= 10)
            return uint16_t(0);
        if (volt <= -10)
            return uint16_t(0xffff);
        // this is for the DAC8814 chip in SPI0
        double scale = 65535 / 20.0;
        double offset = 10.0;
        return uint16_t(((offset - volt) * scale) + 0.5);
    }
public:
    constexpr DACSetVoltF(uint8_t dac, double volt)
        : DACSetVolt(dac, getVoltData(volt))
    {}
};

template<bool has_res>
struct DDSCmd : SimpleCmd<has_res> {
    constexpr DDSCmd(uint32_t ctrl, uint32_t op)
        : SimpleCmd<has_res>(0x10000000 | ctrl, op, 50)
    {}
};

// set bytes at addr + 1 and addr
// note that get_dds_two bytes also returns data at addr+1 and addr
struct DDSSetTwoBytes : DDSCmd<false> {
    // put addr in bits 15...9 (maps to DDS opcode_reg[14:9])?
    // put data in bits 15...0 (maps to DDS operand_reg[15:0])?
    constexpr DDSSetTwoBytes(int i, uint32_t addr, uint32_t data)
        : DDSCmd<false>(0x2 | (i << 4) | (((addr + 1) & 0x7f) << 9),
                        data & 0xffff)
    {}
};

// set bytes addr + 3 ... addr
struct DDSSetFourBytes : DDSCmd<false> {
    // put addr in bits 15...9 (maps to DDS opcode_reg[14:9])?
    constexpr DDSSetFourBytes(int i, uint32_t addr, uint32_t data)
        : DDSCmd<false>(0xf | (i << 4) | (((addr + 1) & 0x7f) << 9), data)
    {}
};

// make dummy pulses
// if t > t_max, subdivide into shorter pulses
struct WaitPulse : BaseCmd<false> {
    constexpr WaitPulse(uint64_t t)
        : m_t(t)
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
        static constexpr uint32_t t_max = 0xffffff;
        auto t = m_t;
        do {
            uint32_t t_step = uint32_t(min(t, t_max));
            v.shortPulse(0x20000000 | t_step, 0);
            t -= t_step;
        } while (t > 0);
    }
private:
    const uint64_t m_t;
};

// make timed pulses
// if t > t_max, subdivide into shorter pulses
struct TTLPulse : BaseCmd<false> {
    constexpr TTLPulse(uint64_t t, uint32_t val)
        : m_t(t), m_val(val)
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
        static constexpr uint32_t t_max = 0xffffff;
        auto t = m_t;
        do {
            uint32_t t_step = uint32_t(min(t, t_max));
            v.shortPulse(t_step, m_val);
            t -= t_step;
        } while (t > 0);
    }
    constexpr uint32_t
    ttlVal() const
    {
        return m_val;
    }
private:
    const uint64_t m_t;
    const uint32_t m_val;
};

// clear timing check (clear failures)
struct ClearTimingCheck : SimpleCmd<false> {
    constexpr ClearTimingCheck() : SimpleCmd<false>(0x30000000, 0, 5)
    {}
};

struct LoopBack : SimpleCmd<true> {
    constexpr LoopBack(uint32_t data) : SimpleCmd<true>(0x40000000, data, 5)
    {}
};

struct DDSSetFreq : DDSCmd<false> {
    constexpr DDSSetFreq(int i, uint32_t ftw)
        : DDSCmd<false>(i << 4, ftw)
    {}
};

struct DDSSetFreqF : DDSSetFreq {
    constexpr DDSSetFreqF(int i, double f)
        : DDSSetFreq(i, DDSCvt::freq2num(f, PULSER_AD9914_CLK))
    {}
};

struct DDSSetAmp : DDSSetTwoBytes {
    constexpr DDSSetAmp(int i, uint32_t amp)
        : DDSSetTwoBytes(i, 0x32, amp)
    {}
};

struct DDSSetAmpF : DDSSetAmp {
    constexpr DDSSetAmpF(int i, double amp)
        : DDSSetAmp(i, DDSCvt::amp2num(amp))
    {}
};

struct DDSSetPhase : DDSSetTwoBytes {
    constexpr DDSSetPhase(int i, uint16_t phase)
        : DDSSetTwoBytes(i, 0x30, phase)
    {}
};

struct DDSSetPhaseF : DDSSetPhase {
    constexpr DDSSetPhaseF(int i, double phase)
        : DDSSetPhase(i, DDSCvt::phase2num(phase))
    {}
};

struct DDSReset : DDSCmd<false> {
    constexpr DDSReset(int i)
        : DDSCmd<false>(0x4 | (i << 4), 0)
    {}
};

// reset DDS selected by bitmask mask
struct DDSResetSel : DDSCmd<false> {
    constexpr DDSResetSel(uint32_t mask)
        : DDSCmd<false>(0x5, mask)
    {}
};

struct DDSSetSel : DDSCmd<false> {
    constexpr DDSSetSel(uint32_t mask)
        : DDSCmd<false>(0x6, mask)
    {}
};

// get byte from address on DDS i
struct DDSGetByte : DDSCmd<true> {
    constexpr DDSGetByte(int i, uint32_t addr)
        : DDSCmd<true>(0x3 | (i << 4) | (addr << 9), 0)
    {}
    static inline constexpr uint32_t
    convertRes(uint32_t res)
    {
        return (res >> 8) & 0x000000ff;
    }
};

// get two bytes from address + 1 ... adress on DDS i
struct DDSGetTwoBytes : DDSCmd<true> {
    constexpr DDSGetTwoBytes(int i, uint32_t addr)
        : DDSCmd<true>(0x3 | (i << 4) | ((addr + 1) << 9), 0)
    {}
    static inline constexpr uint32_t
    convertRes(uint32_t res)
    {
        return res & 0x0000ffff;
    }
};

struct DDSGetFourBytes : DDSCmd<true> {
    constexpr DDSGetFourBytes(int i, uint32_t addr)
        : DDSCmd<true>(0xe | (i << 4) | ((addr + 1) << 9), 0)
    {}
};

struct DDSGetPhase : DDSGetTwoBytes {
    constexpr DDSGetPhase(int i)
        : DDSGetTwoBytes(i, 0x30)
    {}
};

struct DDSGetPhaseF : DDSGetPhase {
    using DDSGetPhase::DDSGetPhase;
    static inline constexpr double
    convertRes(uint32_t res)
    {
        return DDSCvt::num2phase(res);
    }
};

struct DDSGetAmp : DDSGetTwoBytes {
    constexpr DDSGetAmp(int i)
        : DDSGetTwoBytes(i, 0x32)
    {}
};

struct DDSGetAmpF : DDSGetAmp {
    using DDSGetAmp::DDSGetAmp;
    static inline constexpr double
    convertRes(uint32_t res)
    {
        return DDSCvt::num2amp(res);
    }
};

template<typename Cmd>
static inline constexpr int
_numCmdResult()
{
    static_assert(isSimpleCmd<Cmd>, "");
    return isBaseOf<SimpleCmd<true>, Cmd> ? 1 : 0;
}

template<typename... Cmds>
static constexpr int numCmdResults = sumAll(_numCmdResult<Cmds>()...);

static constexpr struct {
    template<typename... Cmd>
    inline constexpr uint64_t
    operator()(Cmd&&... cmd)
    {
        return sumAll(cmd.length()...);
    }
} totalCmdLen{};

template<typename T>
struct CmdRunner {
    CmdRunner(T &v) : m_v(v)
    {}
    T &m_v;
    template<typename First>
    inline void
    operator()(First &&first) const
    {
        first.run(m_v);
    }
    template<typename First, typename... Rest>
    inline auto
    operator()(First &&first, Rest&&... rest) const
    {
        operator()(std::forward<First>(first));
        operator()(std::forward<Rest>(rest)...);
    }
};

template<size_t Cur, typename Cmd, size_t... I>
auto __resIndexes(std::index_sequence<I...> idx)
    -> std::conditional_t<isBaseOf<SimpleCmd<true>, Cmd>,
                          std::index_sequence<Cur, I...>, decltype(idx)>;
template<size_t Cur, size_t... I>
std::index_sequence<I...> _resIndexes(std::index_sequence<I...> idx);

template<size_t Cur, typename First, typename... Rest, size_t... I>
auto _resIndexes(std::index_sequence<I...> idx)
{
    return __resIndexes<Cur, First>(_resIndexes<Cur + 1, Rest...>(idx));
}

template<typename... Cmd>
using ResIndexes = decltype(_resIndexes<0, Cmd...>(std::index_sequence<>()));

template<typename CmdTuple>
struct CompositeCmd;

template<typename... Cmd>
struct CompositeCmd<std::tuple<Cmd...> > :
        std::tuple<Cmd...>, BaseCmd<numCmdResults<Cmd...> != 0> {
    static constexpr auto numRes = numCmdResults<Cmd...>;
    typedef std::tuple<Cmd...> tupleType;
    typedef ResIndexes<Cmd...> resIndexes;
    template<typename... T>
    CompositeCmd(T&&... v)
        : std::tuple<Cmd...>(std::forward<T>(v)...)
    {}
    inline constexpr uint64_t
    length() const
    {
        return applyTuple(totalCmdLen,
                          static_cast<const std::tuple<Cmd...>&>(*this));
    }
    template<typename T>
    inline void
    run(T &v) const
    {
        applyTuple(CmdRunner<T>(v),
                   static_cast<const std::tuple<Cmd...>&>(*this));
    }
};

template<typename Cmd, class=void>
struct _isCompositeCmd : std::false_type {};

template<typename Cmd>
struct _isCompositeCmd<
    Cmd, std::enable_if_t<
             isBaseOf<CompositeCmd<typename std::decay_t<Cmd>::tupleType>,
                      Cmd>>> : std::true_type {};

template<typename Cmd>
static constexpr bool isCompositeCmd = _isCompositeCmd<Cmd>::value;

struct DDSGetFreq : CompositeCmd<std::tuple<DDSGetTwoBytes, DDSGetTwoBytes> > {
    DDSGetFreq(int i)
        : CompositeCmd<std::tuple<DDSGetTwoBytes,
                                  DDSGetTwoBytes> >(DDSGetTwoBytes(i, 0x2c),
                                                    DDSGetTwoBytes(i, 0x2e))
    {}
    static inline uint32_t
    convertRes(uint32_t res1, uint32_t res2)
    {
        return res1 | (res2 << 16);
    }
};

struct DDSGetFreqF : DDSGetFreq {
    using DDSGetFreq::DDSGetFreq;
    static inline double
    convertRes(uint32_t res1, uint32_t res2)
    {
        return DDSCvt::num2freq(DDSGetFreq::convertRes(res1, res2),
                                PULSER_AD9914_CLK);
    }
};

struct DDSExists : CompositeCmd<std::tuple<DDSSetTwoBytes, DDSGetTwoBytes,
                                           DDSSetTwoBytes, DDSGetTwoBytes> > {
    DDSExists(int i)
        : CompositeCmd<std::tuple<DDSSetTwoBytes, DDSGetTwoBytes,
                                  DDSSetTwoBytes,
                                  DDSGetTwoBytes> >(DDSSetTwoBytes(i, 0x68, 0),
                                                    DDSGetTwoBytes(i, 0x68),
                                                    DDSSetTwoBytes(i, 0x68, 1),
                                                    DDSGetTwoBytes(i, 0x68))
    {}
    static inline bool
    convertRes(uint32_t res1, uint32_t res2)
    {
        return res1 == 0 && res2 == 1;
    }
};

}
}

#endif
