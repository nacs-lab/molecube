#ifndef __NACS_PULSER_INSTRUCTION_H__
#define __NACS_PULSER_INSTRUCTION_H__

#include "controller.h"

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
};

struct CtrlState {
    bool timing_check;
    uint16_t dds_phases[22];
};

struct InstWriter {
    // all 500ns
    struct DDS {
        static inline Instruction
        setPhase(int i, uint16_t phase)
        {
            return Instruction(ControlBit::DDSSetPhaseMeta | phase,
                               uint32_t(i));
        }
        static inline Instruction
        setPhaseF(int i, double phase)
        {
            return setPhase(i, DDSCvt::phase2num(phase));
        }
        static inline Instruction
        shiftPhase(int i, uint16_t phase)
        {
            return Instruction(ControlBit::DDSShiftPhaseMeta | phase,
                               uint32_t(i));
        }
        static inline Instruction
        shiftPhaseF(int i, double phase)
        {
            return shiftPhase(i, DDSCvt::phase2num(phase));
        }
        static inline Instruction
        setFreq(int i, uint32_t freq)
        {
            return DDSSetFreq(i, freq);
        }
        static inline Instruction
        setFreqF(int i, double freq)
        {
            return DDSSetFreqF(i, freq);
        }
        static inline Instruction
        setAmp(int i, uint32_t amp)
        {
            return DDSSetAmp(i, amp);
        }
        static inline Instruction
        setAmpF(int i, double amp)
        {
            return DDSSetAmpF(i, amp);
        }
        static inline Instruction
        reset(int i)
        {
            return Instruction(ControlBit::DDSResetMeta, uint32_t(i));
        }
    };
    // 0ns
    static inline Instruction
    enableTimingCheck()
    {
        return Instruction(ControlBit::TimingCheckMeta, 1);
    }
    // 0ns
    static inline Instruction
    disableTimingCheck()
    {
        return Instruction(ControlBit::TimingCheckMeta, 0);
    }
    // 50ns
    static inline Instruction
    clearTimingCheck()
    {
        return ClearTimingCheck();
    }
    // variable time
    static inline Instruction
    wait(uint64_t t)
    {
        return Instruction(ControlBit::WaitMeta | uint32_t(t >> 32),
                           uint32_t(t));
    }
    // 30ns
    static inline Instruction
    ttl(uint32_t val)
    {
        return Instruction(3, val);
    }
    // 50ns
    static inline Instruction
    clockOut(uint32_t div)
    {
        return ClockOut(div);
    }
};

}
}

// In case we want to use with LLVM
extern "C"
void runInstructionList(NaCs::Pulser::Controller *__restrict__ ctrler,
                        NaCs::Pulser::CtrlState *__restrict__ state,
                        const NaCs::Pulser::Instruction *__restrict__ inst,
                        size_t n);

#endif
