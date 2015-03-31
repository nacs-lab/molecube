#include "instruction.h"

namespace NaCs {
namespace Pulser {

static inline __attribute__((flatten, hot)) void
runDDSSetPhase(Controller *__restrict__ ctrler, CtrlState *__restrict__ state,
               int dds_num, uint16_t phase)
{
    state->dds_phases[dds_num] = phase;
    ctrler->write(DDSSetPhase(dds_num, phase));
}

static inline __attribute__((flatten, hot)) void
runWaitMeta(Controller *__restrict__ ctrler, uint32_t t_high, uint32_t t_low)
{
    uint64_t t = uint64_t(t_high) << 32 | t_low; // time in 10ns
    // If the wait time is too short, don't do anything fancy
    if (t < 50) {
        ctrler->shortPulse(0x20000000 | uint32_t(t), 0);
        return;
    } else if (t < 6000) {
        // Allocate 1.28us for each pulse (except the first one)
        uint32_t t32 = uint32_t(t);
        uint32_t max_requests = (t32 - 50) / 128 + 1;
        t32 -= ctrler->writeRequests(max_requests, false);
        ctrler->shortPulse(0x20000000 | t32, 0);
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
                                                          false));
            auto t_step = t_max - t_write;
            ctrler->shortPulse(0x20000000 | t_step, 0);
            if (!t_write) {
                std::this_thread::sleep_for(t_sleep);
            } else {
                ctrler->releaseHold();
            }
            t -= t_max;
        } else {
            ctrler->shortPulse(0x20000000 | uint32_t(t), 0);
            break;
        }
    }
}

static inline __attribute__((flatten, hot)) void
runMetaInstruction(Controller *__restrict__ ctrler,
                   CtrlState *__restrict__ state, uint32_t ctrl, uint32_t op)
{
    switch (ctrl & ControlBit::MetaInstMask) {
    case ControlBit::WaitMeta:
        // After removing the Meta bits, the maximum time is
        // 2^(32 + 24) * 10ns ~ 22 years. Hopefully that's enough...
        runWaitMeta(ctrler, ctrl & ~ControlBit::MetaInstMask, op);
        break;
    case ControlBit::DDSSetPhaseMeta:
        // Truncate ctrl to 16 bits to get phase
        runDDSSetPhase(ctrler, state, int(op), uint16_t(ctrl));
        break;
    case ControlBit::DDSShiftPhaseMeta: {
        int dds_num = op;
        // Truncate ctrl to 16 bits to get phase
        runDDSSetPhase(ctrler, state, dds_num,
                       uint16_t(ctrl) + state->dds_phases[dds_num]);
    }
        break;
    case ControlBit::TimingCheckMeta:
        state->timing_check = bool(op);
        break;
    case ControlBit::DDSResetMeta: {
        int dds_num = op;
        state->dds_phases[dds_num] = 0;
        ctrler->write(DDSReset(dds_num));
    }
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

}
}

using namespace NaCs::Pulser;

NACS_EXPORT __attribute__((flatten, hot)) void
runInstructionList(Controller *__restrict__ ctrler,
                   CtrlState *__restrict__ state,
                   const Instruction *__restrict__ inst,
                   size_t n)
{
    for (size_t i = 0;i < n;i++) {
        __builtin_prefetch(inst + 2);
        runInstruction(ctrler, state, &inst[n]);
    }
}
