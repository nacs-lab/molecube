//

#include "instruction.h"

#include <nacs-seq/bytecode.h>

#include <iostream>

namespace NaCs {
namespace Pulser {

static inline __attribute__((flatten, hot)) void
checkedShortPulse(Controller *__restrict__ ctrler, uint32_t ctrl, uint32_t op)
{
    ctrl |= ControlBit::TimingCheck;
    ctrler->shortPulse(ctrl, op);
}

template<typename Cmd>
static inline __attribute__((flatten, hot))
std::enable_if_t<isSimpleCmd<Cmd> >
checkedShortPulse(Controller *__restrict__ ctrler, Cmd &&cmd)
{
    checkedShortPulse(ctrler, cmd.control(), cmd.operand());
}

static inline __attribute__((flatten, hot)) void
runDDSSetPhase(Controller *__restrict__ ctrler, CtrlState *__restrict__ state,
               int dds_num, uint16_t phase)
{
    state->dds_phases[dds_num] = phase;
    checkedShortPulse(ctrler, DDSSetPhase(dds_num, phase));
}

template<typename T>
static inline __attribute__((flatten, hot)) void
runWait(Controller *__restrict__ ctrler, uint64_t &wait_time, uint64_t t,
        T &&release_after)
{
    const uint32_t flags = ControlBit::TimingCheck;
    // If the wait time is too short, don't do anything fancy
    static constexpr uint32_t t_max = 8000; // 80us
    static constexpr auto t_sleep = 10us;
    if (t < 50) {
        ctrler->shortPulse(0x20000000 | uint32_t(t) | flags, 0);
        return;
    } else if (t < t_max * 2) {
        uint32_t t32 = uint32_t(t);
        wait_time += t32;
        if (wait_time > 8192 || t >= 1024) {
            wait_time = 0;
            // Allocate 1.28us for each pulse (except the first one)
            uint32_t max_requests = (t32 - 50) / 256 + 1;
            t32 -= (uint32_t)ctrler->writeRequests(max_requests, false, flags);
            if (release_after) {
                ctrler->releaseHold();
                release_after = false;
            }
        }
        ctrler->shortPulse(0x20000000 | t32 | flags, 0);
        return;
    }
    wait_time = 0;
    while (true) {
        // Proceed 80us each time. If no requests are written, sleep for
        // 10us to wait for new request in order to minimize request latency
        // Also unblock the queue if some requests are written.
        if (t >= 2 * t_max) {
            static constexpr uint32_t max_requests = t_max / 1000;
            auto t_write = uint32_t(ctrler->writeRequests(max_requests,
                                                          true, flags));
            auto t_step = t_max - t_write;
            ctrler->shortPulse(0x20000000 | t_step | flags, 0);
            if (!t_write) {
                std::this_thread::sleep_for(t_sleep);
            }
            else if (release_after) {
                ctrler->releaseHold();
                release_after = false;
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
           uint32_t ttl_ctrl, uint32_t ttl_val)
{
    uint32_t ttl_addr = ttl_ctrl & ControlBit::TTLAll;
    uint32_t ttl_time = ttl_ctrl >> 18;
    if (ttl_addr == ControlBit::TTLAll) {
        state->curr_ttl = ttl_val;
    } else {
        state->curr_ttl = setBit(state->curr_ttl, uint8_t(ttl_addr),
                                 bool(ttl_val));
    }
    checkedShortPulse(ctrler, ttl_time, state->curr_ttl);
}

static inline uint64_t
combTime(uint64_t ctrl, uint32_t op)
{
    return (ctrl & ControlBit::MetaContentMask) << 32 | op;
}

static inline __attribute__((flatten, hot)) void
runMetaInstruction(Controller *__restrict__ ctrler,
                   CtrlState *__restrict__ state, uint32_t ctrl, uint32_t op)
{
    switch (ctrl & ControlBit::MetaInstMask) {
    case ControlBit::WaitMeta:
        // After removing the Meta bits, the maximum time is
        // 2^(32 + 24) * 10ns ~ 22 years. Hopefully that's enough...
        runWait(ctrler, state->wait_time, combTime(ctrl, op), false);
        break;
    case ControlBit::DDSSetPhaseMeta:
        // Truncate ctrl to 16 bits to get phase
        runDDSSetPhase(ctrler, state, int(op), uint16_t(ctrl));
        break;
    case ControlBit::DDSShiftPhaseMeta:
        // Truncate ctrl to 16 bits to get phase
        runDDSSetPhase(ctrler, state, int(op),
                       uint16_t(ctrl + state->dds_phases[op]));
        break;
    case ControlBit::DDSResetMeta:
        state->dds_phases[op] = 0;
        checkedShortPulse(ctrler, DDSReset(int(op)));
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
    } else {
        ctrler->shortPulse(ctrl | ControlBit::TimingCheck, op);
    }
}

NACS_EXPORT() void
runInstructionList(Controller *__restrict__ ctrler,
                   CtrlState *__restrict__ state,
                   const Instruction *__restrict__ inst, size_t n)
{
    for (size_t i = 0;i < n;i++) {
        auto cur_inst = inst + i;
        __builtin_prefetch(cur_inst + 2);
        runInstruction(ctrler, state, cur_inst);
    }
    ctrler->shortPulse(0x20000000 | 3, 0);
}

namespace {

struct ByteCodeRunner {
    void ttl(uint32_t ttl, uint64_t t)
    {
        ttl = ttl | preserve_ttl;
        if (t <= 1000) {
            // 10us
            checkedShortPulse(ctrler, (uint32_t)t, ttl);
        }
        else {
            checkedShortPulse(ctrler, 100, ttl);
            wait(t - 100);
        }
    }
    void dds_freq(uint8_t chn, uint32_t freq)
    {
        checkedShortPulse(ctrler, DDSSetFreq(chn, freq));
    }
    void dds_amp(uint8_t chn, uint16_t amp)
    {
        checkedShortPulse(ctrler, DDSSetAmp(chn, amp));
    }
    void dac(uint8_t chn, uint16_t V)
    {
        checkedShortPulse(ctrler, DACSetVolt(chn, V));
    }
    void wait(uint64_t t)
    {
        runWait(ctrler, wait_time, t, release_after);
    }
    void clock(uint8_t period)
    {
        checkedShortPulse(ctrler, ClockOut(period));
    }
    Controller *ctrler;
    uint32_t preserve_ttl;
    uint64_t wait_time{0};
    bool release_after{true};
};

}

NACS_EXPORT() __attribute__((flatten, hot))
void runByteCode(Controller *__restrict__ ctrler,
                 const uint8_t *__restrict__ code, size_t code_len, uint32_t ttl_mask)
{
    uint32_t preserve_ttl = 0;
    if (~ttl_mask != 0)
        preserve_ttl = (~ttl_mask) & ctrler->getCurTTL();
    ByteCodeRunner runner{ctrler, preserve_ttl};
    Seq::ByteCode::ExeState exestate;
    exestate.run(runner, code, code_len);
    ctrler->shortPulse(0x20000000 | 3, 0);
}

NACS_EXPORT() void runEpilogue(Controller *__restrict__ ctrler)
{
    uint64_t wait_time = 0;
    // This is a hack that is believed to make the NI card happy.
    // 1us
    checkedShortPulse(ctrler, ClockOut(59));
    // 30ms
    runWait(ctrler, wait_time, 3000000, false);
    checkedShortPulse(ctrler, ClockOut(255));
    ctrler->run(Pulser::ClearTimingCheck());
}

}
}
