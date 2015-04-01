#include "instruction.h"

namespace NaCs {
namespace Pulser {

static inline __attribute__((flatten, hot)) void
writeShortPulse(Controller *__restrict__ ctrler, CtrlState *__restrict__ state,
                uint32_t ctrl, uint32_t op)
{
    if (nacsLikely(state->timing_check))
        ctrl |= ControlBit::TimingCheck;
    ctrler->shortPulse(ctrl, op);
}

template<typename Cmd>
static inline __attribute__((flatten, hot))
std::enable_if_t<isSimpleCmd<Cmd> >
writeShortPulse(Controller *__restrict__ ctrler,
                CtrlState *__restrict__ state, Cmd &&cmd)
{
    writeShortPulse(ctrler, state, cmd.control(), cmd.operand());
}

static inline __attribute__((flatten, hot)) void
runDDSSetPhase(Controller *__restrict__ ctrler, CtrlState *__restrict__ state,
               int dds_num, uint16_t phase)
{
    state->dds_phases[dds_num] = phase;
    writeShortPulse(ctrler, state, DDSSetPhase(dds_num, phase));
}

static inline __attribute__((flatten, hot)) void
runWaitMeta(Controller *__restrict__ ctrler, CtrlState *__restrict__ state,
            uint32_t t_high, uint32_t t_low)
{
    const uint32_t flags = (nacsLikely(state->timing_check) ?
                            ControlBit::TimingCheck : 0);
    uint64_t t = uint64_t(t_high) << 32 | t_low; // time in 10ns
    // If the wait time is too short, don't do anything fancy
    if (t < 50) {
        ctrler->shortPulse(0x20000000 | uint32_t(t) | flags, 0);
        return;
    } else if (t < 6000) {
        // Allocate 1.28us for each pulse (except the first one)
        uint32_t t32 = uint32_t(t);
        uint32_t max_requests = (t32 - 50) / 128 + 1;
        t32 -= ctrler->writeRequests(max_requests, false, flags);
        ctrler->shortPulse(0x20000000 | t32 | flags, 0);
        return;
    }
    static constexpr uint32_t t_max = 5000; // 50us
    static constexpr auto t_sleep = 25us; // 25us
    while (true) {
        // Proceed 50us each time. If no requests are written, sleep for
        // 25us to wait for new request in order to minimize request latency
        // Also unblock the queue if some requests are written.
        if (t > t_max) {
            static constexpr uint32_t max_requests = t_max / 256 + 1;
            auto t_write = uint32_t(ctrler->writeRequests(max_requests,
                                                          false, flags));
            auto t_step = t_max - t_write;
            ctrler->shortPulse(0x20000000 | t_step | flags, 0);
            if (!t_write) {
                std::this_thread::sleep_for(t_sleep);
            } else {
                ctrler->releaseHold();
            }
            t -= t_max;
        } else {
            ctrler->shortPulse(0x20000000 | uint32_t(t) | flags, 0);
            break;
        }
    }
}

static inline __attribute__((flatten, hot)) void
runTTLMeta(Controller *__restrict__ ctrler, CtrlState *__restrict__ state,
           uint32_t ttl_addr, uint32_t ttl_val)
{
    if (ttl_addr & ControlBit::TTLAll) {
        state->curr_ttl = ttl_val;
    } else {
        state->curr_ttl = setBit(state->curr_ttl, uint8_t(ttl_addr),
                                 bool(ttl_val));
    }
    writeShortPulse(ctrler, state, 3, state->curr_ttl);
}

static inline __attribute__((flatten, hot)) void
runMetaInstruction(Controller *__restrict__ ctrler,
                   CtrlState *__restrict__ state, uint32_t ctrl, uint32_t op)
{
    switch (ctrl & ControlBit::MetaInstMask) {
    case ControlBit::WaitMeta:
        // After removing the Meta bits, the maximum time is
        // 2^(32 + 24) * 10ns ~ 22 years. Hopefully that's enough...
        runWaitMeta(ctrler, state, ctrl & ControlBit::MetaContentMask, op);
        break;
    case ControlBit::DDSSetPhaseMeta:
        // Truncate ctrl to 16 bits to get phase
        runDDSSetPhase(ctrler, state, int(op), uint16_t(ctrl));
        break;
    case ControlBit::DDSShiftPhaseMeta:
        // Truncate ctrl to 16 bits to get phase
        runDDSSetPhase(ctrler, state, int(op),
                       uint16_t(ctrl) + state->dds_phases[op]);
        break;
    case ControlBit::TimingCheckMeta:
        state->timing_check = bool(op);
        break;
    case ControlBit::DDSResetMeta:
        state->dds_phases[op] = 0;
        writeShortPulse(ctrler, state, DDSReset(int(op)));
        break;
    case ControlBit::TTLMeta:
        runTTLMeta(ctrler, state, ctrl & ControlBit::MetaContentMask, op);
        break;
    }
}

static inline __attribute__((flatten, hot)) void
runInstruction(Controller *__restrict__ ctrler, CtrlState *__restrict__ state,
               const Instruction *__restrict__ inst)
{
    uint32_t ctrl = inst->ctrl;
    uint32_t op = inst->op;
    if ((ctrl & ControlBit::InstMask) == ControlBit::MetaCmd) {
        runMetaInstruction(ctrler, state, ctrl, op);
    } else if (nacsLikely(state->timing_check)) {
        ctrler->shortPulse(ctrl | ControlBit::TimingCheck, op);
    } else {
        ctrler->shortPulse(ctrl & ~ControlBit::TimingCheck, op);
    }
}

NACS_EXPORT __attribute__((flatten, hot)) void
runInstructionList(Controller *__restrict__ ctrler,
                   CtrlState *__restrict__ state,
                   const Instruction *__restrict__ inst,
                   size_t n)
{
    for (size_t i = 0;i < n;i++) {
        auto cur_inst = inst + i;
        __builtin_prefetch(cur_inst + 2);
        runInstruction(ctrler, state, cur_inst);
    }
}

}
}
