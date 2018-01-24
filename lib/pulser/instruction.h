#ifndef __NACS_PULSER_INSTRUCTION_H__
#define __NACS_PULSER_INSTRUCTION_H__

#include "controller.h"

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

static uint64_t
combTime(uint64_t ctrl, uint32_t op)
{
    return (ctrl & ControlBit::MetaContentMask) << 32 | op;
}

static uint64_t
combTime(Instruction &inst)
{
    return combTime(inst.ctrl, inst.op);
}

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
            accumTime(tp, 50);
            return Instruction(ControlBit::MetaCmd |
                               ControlBit::DDSSetPhaseMeta | DDSCvt::phase2num(phase),
                               uint32_t(i));
        }
        static inline Instruction
        shiftPhase(int i, double phase, uint64_t *tp=nullptr)
        {
            accumTime(tp, 50);
            return Instruction(ControlBit::MetaCmd |
                               ControlBit::DDSShiftPhaseMeta | DDSCvt::phase2num(phase),
                               uint32_t(i));
        }
        static inline Instruction
        setFreq(int i, double freq, uint64_t *tp=nullptr)
        {
            accumTime(tp, 50);
            return DDSSetFreqF(i, freq);
        }
        static inline Instruction
        setAmp(int i, double amp, uint64_t *tp=nullptr)
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
        return ttlAllT(val, 3, tp);
    }
    // 30ns
    static inline Instruction
    ttl(uint8_t addr, bool val, uint64_t *tp=nullptr)
    {
        return ttlT(addr, val, 3, tp);
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
                 const uint8_t *__restrict__ code, size_t code_len);

struct BlockBuilder;

static inline bool
mergeWaitWait(Instruction &prev_inst, Instruction &inst)
{
    prev_inst = InstWriter::wait(combTime(prev_inst) + combTime(inst));
    return true;
}

static inline bool
mergeTTLWait(Instruction &prev_inst, Instruction &inst)
{
    uint64_t ttl_time = (prev_inst.ctrl & ControlBit::MetaContentMask) >> 18;
    uint64_t wait_time = combTime(inst);
    uint64_t total_time = ttl_time + wait_time;
    if (total_time < 64) {
        prev_inst.ctrl = ((prev_inst.ctrl & ~(0x3f << 18)) |
                          uint32_t(total_time) << 18);
        return true;
    } else if (ttl_time != 3) {
        prev_inst.ctrl = (prev_inst.ctrl & ~(0x3f << 18)) | uint32_t(3) << 18;
        inst = InstWriter::wait(total_time - 3);
    }
    return false;
}

static inline bool
compatibleTTLs(Instruction &prev_inst, Instruction &inst)
{
    auto prev_ctrl = prev_inst.ctrl;
    auto ctrl = inst.ctrl;
    auto prev_addr = prev_ctrl & ControlBit::TTLAll;
    auto addr = ctrl & ControlBit::TTLAll;
    if (prev_addr == addr && prev_inst.op == inst.op)
        return true;
    if (prev_addr == ControlBit::TTLAll && addr != ControlBit::TTLAll &&
        prev_inst.op == setBit(prev_inst.op, uint8_t(addr), bool(inst.op))) {
        return true;
    }
    return false;
}

static inline bool
mergeTTLTTL(Instruction &prev_inst, Instruction &inst)
{
    if (compatibleTTLs(prev_inst, inst)) {
        inst = InstWriter::wait((inst.ctrl &
                                 ControlBit::MetaContentMask) >> 18);
        return mergeTTLWait(prev_inst, inst);
    }
    return false;
}

static inline bool
tryMergeMeta(Instruction &prev_inst, Instruction &inst)
{
    auto prev_ctrl = prev_inst.ctrl;
    auto ctrl = inst.ctrl;
    auto prev_meta = prev_ctrl & ControlBit::MetaInstMask;
    auto meta = ctrl & ControlBit::MetaInstMask;
    if (prev_meta == ControlBit::TTLMeta) {
        if (meta == ControlBit::WaitMeta) {
            return mergeTTLWait(prev_inst, inst);
        } else if (meta == ControlBit::TTLMeta) {
            return mergeTTLTTL(prev_inst, inst);
        }
    } else if (prev_meta == ControlBit::WaitMeta) {
        if (meta == ControlBit::WaitMeta) {
            return mergeWaitWait(prev_inst, inst);
        }
    }
    return false;
}

static inline bool
tryMergeInst(Instruction &prev_inst, Instruction &inst)
{
    if ((prev_inst.ctrl & ControlBit::InstMask) != ControlBit::MetaCmd ||
        (inst.ctrl & ControlBit::InstMask) != ControlBit::MetaCmd)
        return false;
    return tryMergeMeta(prev_inst, inst);
}

static inline bool
mergeTTLWaitTTL(Instruction &prev2_inst, Instruction &prev_inst,
                Instruction &inst)
{
    if (compatibleTTLs(prev2_inst, inst)) {
        inst = InstWriter::wait((inst.ctrl &
                                 ControlBit::MetaContentMask) >> 18);
        return mergeWaitWait(prev_inst, inst);
    }
    return false;
}

static inline bool
tryMergeMeta3(Instruction &prev2_inst, Instruction &prev_inst,
              Instruction &inst)
{
    auto prev2_ctrl = prev2_inst.ctrl;
    auto prev_ctrl = prev_inst.ctrl;
    auto ctrl = inst.ctrl;
    auto prev2_meta = prev2_ctrl & ControlBit::MetaInstMask;
    auto prev_meta = prev_ctrl & ControlBit::MetaInstMask;
    auto meta = ctrl & ControlBit::MetaInstMask;
    if (prev2_meta == ControlBit::TTLMeta &&
        prev_meta == ControlBit::WaitMeta &&
        meta == ControlBit::TTLMeta) {
        return mergeTTLWaitTTL(prev2_inst, prev_inst, inst);
    }
    return false;
}

static inline bool
tryMergeInst3(Instruction &prev2_inst, Instruction &prev_inst,
              Instruction &inst)
{
    if ((prev_inst.ctrl & ControlBit::InstMask) != ControlBit::MetaCmd ||
        (prev2_inst.ctrl & ControlBit::InstMask) != ControlBit::MetaCmd ||
        (inst.ctrl & ControlBit::InstMask) != ControlBit::MetaCmd)
        return false;
    return tryMergeMeta3(prev2_inst, prev_inst, inst);
}

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
        size_t len = size();
        auto *ptr = data();
        if (len && tryMergeInst(ptr[size() - 1], inst))
            return;
        if (len >= 2 && tryMergeInst3(ptr[size() - 2], ptr[size() - 1], inst))
            return;
        push_back(std::move(inst));
    }
    template<typename Func, typename... Args>
    inline void
    pulseAbsT(uint64_t t, Func &&func, Args&&... args)
    {
        if (t < currT) {
            throw std::runtime_error("Going back in time.");
        } else if (t - currT >= 3) {
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
    void fromSeq(const Seq::Sequence &seq);
};

}
}


#endif
