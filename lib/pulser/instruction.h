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

class InstWriter {
    template<typename... T>
    static inline void
    write(Instruction *inst, T&&... v)
    {
        new (inst) Instruction(std::forward<T>(v)...);
    }
public:
    // all 500ns
    struct DDS {
        static inline void
        setPhase(Instruction *inst, int i, uint16_t phase)
        {
            write(inst, ControlBit::DDSSetPhaseMeta | phase, uint32_t(i));
        }
        static inline void
        setPhaseF(Instruction *inst, int i, double phase)
        {
            setPhase(inst, i, DDSCvt::phase2num(phase));
        }
        static inline void
        shiftPhase(Instruction *inst, int i, uint16_t phase)
        {
            write(inst, ControlBit::DDSShiftPhaseMeta | phase, uint32_t(i));
        }
        static inline void
        shiftPhaseF(Instruction *inst, int i, double phase)
        {
            shiftPhase(inst, i, DDSCvt::phase2num(phase));
        }
        static inline void
        setFreq(Instruction *inst, int i, uint32_t freq)
        {
            write(inst, DDSSetFreq(i, freq));
        }
        static inline void
        setFreqF(Instruction *inst, int i, double freq)
        {
            write(inst, DDSSetFreqF(i, freq));
        }
        static inline void
        setAmp(Instruction *inst, int i, uint32_t amp)
        {
            write(inst, DDSSetAmp(i, amp));
        }
        static inline void
        setAmpF(Instruction *inst, int i, double amp)
        {
            write(inst, DDSSetAmpF(i, amp));
        }
        static inline void
        reset(Instruction *inst, int i)
        {
            write(inst, ControlBit::DDSResetMeta, uint32_t(i));
        }
    };
    // 0ns
    static inline void
    enableTimingCheck(Instruction *inst)
    {
        write(inst, ControlBit::TimingCheckMeta, 1);
    }
    // 0ns
    static inline void
    disableTimingCheck(Instruction *inst)
    {
        write(inst, ControlBit::TimingCheckMeta, 0);
    }
    // 50ns
    static inline void
    clearTimingCheck(Instruction *inst)
    {
        write(inst, ClearTimingCheck());
    }
    // variable time
    static inline void
    wait(Instruction *inst, uint64_t t)
    {
        write(inst, ControlBit::WaitMeta | uint32_t(t >> 32), uint32_t(t));
    }
    // 30ns
    static inline void
    ttl(Instruction *inst, uint32_t val)
    {
        write(inst, 3, val);
    }
    // 50ns
    static inline void
    clockOut(Instruction *inst, uint32_t div)
    {
        write(inst, ClockOut(div));
    }
};

}
}

extern "C"
void runInstructionList(NaCs::Pulser::Controller *__restrict__ ctrler,
                        NaCs::Pulser::CtrlState *__restrict__ state,
                        const NaCs::Pulser::Instruction *__restrict__ inst,
                        size_t n);

#endif
