//

#include "instruction.h"

#include <nacs-seq/pulser.h>

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

static inline __attribute__((flatten, hot)) void
runWaitMeta(Controller *__restrict__ ctrler, CtrlState *__restrict__ state,
            uint32_t ctrl, uint32_t op)
{
    const uint32_t flags = ControlBit::TimingCheck;
    uint64_t t = combTime(ctrl, op); // time in 10ns
    // If the wait time is too short, don't do anything fancy
    if (t < 50) {
        ctrler->shortPulse(0x20000000 | uint32_t(t) | flags, 0);
        return;
    } else if (t < 10000) {
        uint32_t t32 = uint32_t(t);
        state->wait_time += t32;
        if (state->wait_time > 8192 || t >= 1024) {
            state->wait_time = 0;
            // Allocate 1.28us for each pulse (except the first one)
            uint32_t max_requests = (t32 - 50) / 256 + 1;
            t32 -= (uint32_t)ctrler->writeRequests(max_requests, false, flags);
        }
        ctrler->shortPulse(0x20000000 | t32 | flags, 0);
        return;
    }
    state->wait_time = 0;
    static constexpr uint32_t t_max = 8000; // 80us
    static constexpr auto t_sleep = 10us;
    while (true) {
        // Proceed 80us each time. If no requests are written, sleep for
        // 10us to wait for new request in order to minimize request latency
        // Also unblock the queue if some requests are written.
        if (t >= t_max) {
            static constexpr uint32_t max_requests = t_max / 1000;
            auto t_write = uint32_t(ctrler->writeRequests(max_requests,
                                                          true, flags));
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

static inline __attribute__((flatten, hot)) void
runMetaInstruction(Controller *__restrict__ ctrler,
                   CtrlState *__restrict__ state, uint32_t ctrl, uint32_t op)
{
    switch (ctrl & ControlBit::MetaInstMask) {
    case ControlBit::WaitMeta:
        // After removing the Meta bits, the maximum time is
        // 2^(32 + 24) * 10ns ~ 22 years. Hopefully that's enough...
        runWaitMeta(ctrler, state, ctrl, op);
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

NACS_EXPORT() void BlockBuilder::fromSeq(const Seq::Sequence &seq)
{
    using Inst = Pulser::InstWriter;
    static constexpr int start_ttl = 0;
    static constexpr int start_ttl_mask = (1 << start_ttl);

    Seq::PulsesBuilder seq_builder =
        [&] (Seq::Channel chn, Seq::Val val, uint64_t t, uint64_t tlim) -> uint64_t {
        uint64_t mint = 50;
        if (chn.typ == Seq::Channel::TTL) {
            mint = 3;
        }
        else if (chn.typ == Seq::Channel::CLOCK) {
            mint = 5;
        }
        if (t + mint > tlim)
            return 0;
        this->pulseAbsT(t, [&] (uint64_t *tp) {
                switch (chn.typ) {
                case Seq::Channel::TTL:
                    return Inst::ttlAll(val.val.i32 & ~start_ttl_mask, tp);
                case Seq::Channel::DDS_FREQ:
                    return Inst::DDS::setFreqF(chn.id, val.val.f64, tp);
                case Seq::Channel::DDS_AMP:
                    return Inst::DDS::setAmpF(chn.id, val.val.f64, tp);
                case Seq::Channel::DAC:
                    return Inst::dacSetVolt(uint8_t(chn.id), val.val.f64, tp);
                case Seq::Channel::CLOCK:
                    return Inst::clockOut(val.val.i32 - 1, tp);
                default:
                    throw std::runtime_error("Invalid Pulse.");
                }
            });
        return mint;
    };
    auto seq_cb = [&] (auto &, uint64_t cur_t, Seq::Event evt) {
        if (evt == Seq::Event::start) {
            // wait 100us
            cur_t += 10000;
            this->pulseAbsT(cur_t, [&] (uint64_t *tp) {
                    return Inst::ttl(start_ttl, 1, tp);
                });
            // 1us
            cur_t += 100;
            this->pulseAbsT(cur_t, [&] (uint64_t *tp) {
                    return Inst::ttl(start_ttl, 0, tp);
                });
            // 5us
            cur_t += 500;
        } else {
            // This is a hack that is believed to make the NI card happy.
            // 1us
            cur_t += 100;
            this->pulseAbsT(cur_t, [&] (uint64_t *tp) {
                    return Inst::clockOut(59, tp);
                });
            // 30ms
            cur_t += 3000000;
            // Turn off the clock even when it is not used just as a
            // place holder for the end of the sequence.
            this->pulseAbsT(cur_t, [&] (uint64_t *tp) {
                    return Inst::clockOut(255, tp);
                });
        }
        return cur_t;
    };
    seq_builder.schedule(const_cast<Seq::Sequence&>(seq), seq_cb);
}

}
}
