#ifndef __NACS_PULSER_INSTRUCTION_H__
#define __NACS_PULSER_INSTRUCTION_H__

#include "controller.h"

#include <nacs-seq/seq.h>

#include <vector>

namespace NaCs {
namespace Seq {
struct Sequence;
};

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
    static constexpr uint32_t DDSResetMeta = 0x4000000;
    static constexpr uint32_t TTLMeta = 0x5000000;

    static constexpr uint32_t TTLAll = 0x03ffff; // 18 bits
    static constexpr uint32_t MetaContentMask = ~(MetaInstMask | InstMask);
};

struct CtrlState {
    uint16_t dds_phases[22];
    uint32_t curr_ttl;
    uint64_t wait_time;
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
        setPhase(int i, double phase, uint64_t *tp=nullptr)
        {
            accumTime(tp, Seq::PulseTime::DDSPhase);
            return Instruction(ControlBit::MetaCmd |
                               ControlBit::DDSSetPhaseMeta | DDSCvt::phase2num(phase),
                               uint32_t(i));
        }
        static inline Instruction
        shiftPhase(int i, double phase, uint64_t *tp=nullptr)
        {
            accumTime(tp, Seq::PulseTime::DDSPhase);
            return Instruction(ControlBit::MetaCmd |
                               ControlBit::DDSShiftPhaseMeta | DDSCvt::phase2num(phase),
                               uint32_t(i));
        }
        static inline Instruction
        setFreq(int i, double freq, uint64_t *tp=nullptr)
        {
            accumTime(tp, Seq::PulseTime::DDSFreq);
            return DDSSetFreqF(i, freq);
        }
        static inline Instruction
        setAmp(int i, double amp, uint64_t *tp=nullptr)
        {
            accumTime(tp, Seq::PulseTime::DDSAmp);
            return DDSSetAmpF(i, amp);
        }
        static inline Instruction
        reset(int i, uint64_t *tp=nullptr)
        {
            accumTime(tp, Seq::PulseTime::DDSReset);
            return Instruction(ControlBit::MetaCmd |
                               ControlBit::DDSResetMeta, uint32_t(i));
        }
    };
    // variable time
    static inline Instruction
    wait(uint64_t t, uint64_t *tp=nullptr)
    {
        accumTime(tp, t);
        return Instruction(ControlBit::MetaCmd |
                           ControlBit::WaitMeta | uint32_t(t >> 32),
                           uint32_t(t));
    }
    // variable time, < 640ns
    static inline Instruction
    ttlAllT(uint32_t val, uint8_t t, uint64_t *tp=nullptr)
    {
        accumTime(tp, t);
        return Instruction(ControlBit::MetaCmd | ControlBit::TTLMeta |
                           ControlBit::TTLAll | uint32_t(t) << 18, val);
    }
    // variable time, < 640ns
    static inline Instruction
    ttlT(uint8_t addr, bool val, uint8_t t, uint64_t *tp=nullptr)
    {
        accumTime(tp, t);
        return Instruction(ControlBit::MetaCmd | ControlBit::TTLMeta |
                           addr | uint32_t(t) << 18, val);
    }
    // 30ns
    static inline Instruction
    ttlAll(uint32_t val, uint64_t *tp=nullptr)
    {
        return ttlAllT(val, Seq::PulseTime::Min, tp);
    }
    // 30ns
    static inline Instruction
    ttl(uint8_t addr, bool val, uint64_t *tp=nullptr)
    {
        return ttlT(addr, val, Seq::PulseTime::Min, tp);
    }
    // 50ns
    static inline Instruction
    clockOut(uint32_t div, uint64_t *tp=nullptr)
    {
        accumTime(tp, 5);
        return ClockOut(div);
    }
    static inline Instruction
    dacSetVolt(uint8_t dac, double volt, uint64_t *tp=nullptr)
    {
        DACSetVoltF cmd{dac, volt};
        accumTime(tp, cmd.length());
        return cmd;
    }
};

void runInstructionList(Controller *__restrict__ ctrler,
                        CtrlState *__restrict__ state,
                        const Instruction *__restrict__ inst, size_t n);
template<typename T>
static inline void
runInstructionList(Controller *__restrict__ ctrler,
                   CtrlState *__restrict__ state, T &&v)
{
    runInstructionList(ctrler, state, v.data(), v.size());
}

void runByteCode(Controller *__restrict__ ctrler,
                 const uint8_t *__restrict__ code, size_t code_len,
                 uint32_t ttl_mask, bool short_seq);
void runEpilogue(Controller *__restrict__ ctrler);

struct BlockBuilder : public std::vector<Instruction> {
    unsigned lineNum;
    uint64_t currT;
public:
    BlockBuilder()
        : std::vector<Instruction>(),
          lineNum(0),
          currT(0)
    {}
    template<typename Func, typename... Args>
    inline void
    pushPulse(Func &&func, Args&&... args)
    {
        Instruction inst(std::forward<Func>(func)(
                             std::forward<Args>(args)..., &currT));
        push_back(std::move(inst));
    }
    template<typename Func, typename... Args>
    inline void
    pulseAbsT(uint64_t t, Func &&func, Args&&... args)
    {
        if (t < currT) {
            throw std::runtime_error("Going back in time.");
        } else if (t - currT >= Seq::PulseTime::Min) {
            pushPulse(InstWriter::wait, t - currT);
        }
        pushPulse(std::forward<Func>(func), std::forward<Args>(args)...);
    }
    template<typename Func, typename... Args>
    inline void
    pulseDT(uint64_t dt, Func &&func, Args&&... args)
    {
        uint64_t final_t = currT + dt;
        pushPulse(std::forward<Func>(func), std::forward<Args>(args)...);
        if (final_t < currT) {
            throw std::runtime_error("Pulse too short.");
        }
        pushPulse(InstWriter::wait, final_t - currT);
    }
    inline size_t
    cacheSize() const
    {
        return size() * sizeof(Instruction) + sizeof(*this);
    }
};

}
}


#endif
