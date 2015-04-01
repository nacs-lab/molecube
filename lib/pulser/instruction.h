#ifndef __NACS_PULSER_INSTRUCTION_H__
#define __NACS_PULSER_INSTRUCTION_H__

#include "controller.h"

#include <vector>

namespace NaCs {
namespace Pulser {

struct Instruction {
    uint32_t ctrl;
    uint32_t op;
    Instruction(uint32_t _ctrl, uint32_t _op)
        : ctrl(_ctrl),
          op(_op)
    {}
    template<typename Cmd, class=std::enable_if_t<isSimpleCmd<Cmd> > >
    Instruction(Cmd &&cmd)
        : Instruction(cmd.control(), cmd.operand())
    {}
};

struct ControlBit {
    static constexpr uint32_t InstMask = 0xf0000000;
    static constexpr uint32_t MetaCmd = 0x20000000;
    static constexpr uint32_t TimingCheck = 0x8000000;
    // TODO: we need to implement command that returns values at some point.
    // However, it is not clear yet what data transfer method is going to be
    // used.

    static constexpr uint32_t MetaInstMask = 0xf000000;
    static constexpr uint32_t WaitMeta = 0x0;
    static constexpr uint32_t DDSSetPhaseMeta = 0x1000000;
    static constexpr uint32_t DDSShiftPhaseMeta = 0x2000000;
    static constexpr uint32_t TimingCheckMeta = 0x3000000;
    static constexpr uint32_t DDSResetMeta = 0x4000000;
    static constexpr uint32_t TTLMeta = 0x5000000;

    static constexpr uint32_t TTLAll = 0x800000;
    static constexpr uint32_t MetaContentMask = ~(MetaInstMask | InstMask);
};

struct CtrlState {
    bool timing_check;
    uint16_t dds_phases[22];
    uint32_t curr_ttl;
};

class InstWriter {
    static inline void
    accumTime(uint64_t *tp, uint64_t t)
    {
        if (tp) {
            *tp += t;
        }
    }
public:
    // all 500ns
    struct DDS {
        static inline Instruction
        setTwoBytes(int i, uint32_t addr, uint32_t val, uint64_t *tp=nullptr)
        {
            accumTime(tp, 50);
            return DDSSetTwoBytes(i, addr, val);
        }
        static inline Instruction
        setPhase(int i, uint16_t phase, uint64_t *tp=nullptr)
        {
            accumTime(tp, 50);
            return Instruction(ControlBit::MetaCmd |
                               ControlBit::DDSSetPhaseMeta | phase,
                               uint32_t(i));
        }
        static inline Instruction
        setPhaseF(int i, double phase, uint64_t *tp=nullptr)
        {
            accumTime(tp, 50);
            return setPhase(i, DDSCvt::phase2num(phase));
        }
        static inline Instruction
        shiftPhase(int i, uint16_t phase, uint64_t *tp=nullptr)
        {
            accumTime(tp, 50);
            return Instruction(ControlBit::MetaCmd |
                               ControlBit::DDSShiftPhaseMeta | phase,
                               uint32_t(i));
        }
        static inline Instruction
        shiftPhaseF(int i, double phase, uint64_t *tp=nullptr)
        {
            accumTime(tp, 50);
            return shiftPhase(i, DDSCvt::phase2num(phase));
        }
        static inline Instruction
        setFreq(int i, uint32_t freq, uint64_t *tp=nullptr)
        {
            accumTime(tp, 50);
            return DDSSetFreq(i, freq);
        }
        static inline Instruction
        setFreqF(int i, double freq, uint64_t *tp=nullptr)
        {
            accumTime(tp, 50);
            return DDSSetFreqF(i, freq);
        }
        static inline Instruction
        setAmp(int i, uint32_t amp, uint64_t *tp=nullptr)
        {
            accumTime(tp, 50);
            return DDSSetAmp(i, amp);
        }
        static inline Instruction
        setAmpF(int i, double amp, uint64_t *tp=nullptr)
        {
            accumTime(tp, 50);
            return DDSSetAmpF(i, amp);
        }
        static inline Instruction
        reset(int i, uint64_t *tp=nullptr)
        {
            accumTime(tp, 50);
            return Instruction(ControlBit::MetaCmd |
                               ControlBit::DDSResetMeta, uint32_t(i));
        }
    };
    // 0ns
    static inline Instruction
    enableTimingCheck(uint64_t* =nullptr)
    {
        return Instruction(ControlBit::MetaCmd |
                           ControlBit::TimingCheckMeta, 1);
    }
    // 0ns
    static inline Instruction
    disableTimingCheck(uint64_t* =nullptr)
    {
        return Instruction(ControlBit::MetaCmd |
                           ControlBit::TimingCheckMeta, 0);
    }
    // 50ns
    static inline Instruction
    clearTimingCheck(uint64_t *tp=nullptr)
    {
        accumTime(tp, 50);
        return ClearTimingCheck();
    }
    // variable time
    static inline Instruction
    wait(uint64_t t, uint64_t *tp=nullptr)
    {
        accumTime(tp, t);
        return Instruction(ControlBit::MetaCmd |
                           ControlBit::WaitMeta | uint32_t(t >> 32),
                           uint32_t(t));
    }
    // 30ns
    static inline Instruction
    ttlAll(uint32_t val, uint64_t *tp=nullptr)
    {
        accumTime(tp, 3);
        return Instruction(ControlBit::MetaCmd |
                           ControlBit::TTLMeta | ControlBit::TTLAll, val);
    }
    // 30ns
    static inline Instruction
    ttl(uint8_t addr, bool val, uint64_t *tp=nullptr)
    {
        accumTime(tp, 3);
        return Instruction(ControlBit::MetaCmd |
                           ControlBit::TTLMeta | addr, val);
    }
    // 50ns
    static inline Instruction
    clockOut(uint32_t div, uint64_t *tp=nullptr)
    {
        accumTime(tp, 5);
        return ClockOut(div);
    }
};

void runInstructionList(Controller *__restrict__ ctrler,
                        CtrlState *__restrict__ state,
                        const Instruction *__restrict__ inst, size_t n);

class BlockBuilder : std::vector<Instruction> {
    unsigned m_line_num;
    uint64_t m_curr_t;
public:
    BlockBuilder()
        : std::vector<Instruction>(),
          m_line_num(0),
          m_curr_t(0)
    {
    }
    inline auto&
    lineNum()
    {
        return m_line_num;
    }
    const Instruction*
    data() const
    {
        return std::vector<Instruction>::data();
    }
    size_t
    size() const
    {
        return std::vector<Instruction>::size();
    }
    template<typename Func, typename... Args>
    inline void
    pushPulse(Func &&func, Args&&... args)
    {
        push_back(std::forward<Func>(func)(std::forward<Args>(args)...,
                                           &m_curr_t));
    }
    template<typename Func, typename... Args>
    inline void
    pulseAbsT(uint64_t t, Func &&func, Args&&... args)
    {
        if (t < m_curr_t) {
            throw std::runtime_error("Going back in time.");
        } else if (t - m_curr_t >= 3) {
            pushPulse(InstWriter::wait, t - m_curr_t);
        }
        pushPulse(std::forward<Func>(func), std::forward<Args>(args)...);
    }
    template<typename Func, typename... Args>
    inline void
    pulseDT(uint64_t dt, Func &&func, Args&&... args)
    {
        uint64_t final_t = m_curr_t + dt;
        pushPulse(std::forward<Func>(func), std::forward<Args>(args)...);
        if (final_t < m_curr_t) {
            throw std::runtime_error("Pulse too short.");
        }
        pushPulse(InstWriter::wait, final_t - m_curr_t);
    }
    inline void
    finalPulse()
    {
        pushPulse(InstWriter::disableTimingCheck);
        pushPulse(InstWriter::wait, 3);
    }
};

}
}


#endif
