//

#ifdef NDEBUG
#  undef NDEBUG
#endif

#include <nacs-utils/timer.h>
#include <nacs-pulser/instruction.h>

#include <iostream>

using namespace NaCs;
using Inst = Pulser::InstWriter;

static void
test_register(Pulser::Controller &ctrl, int n)
{
    std::cout << "Testing register " << n << ":" << std::endl;
    for (int i = 0;i < 1000;i++) {
        ctrl.writeReg(n, 0xffffffff);
        unsigned r = ctrl.readReg(n);
        assert(r == 0xffffffff);

        ctrl.writeReg(n, 0);
        r = ctrl.readReg(n);
        assert(r == 0);
    }
}

static void
test_timing(Pulser::Controller &ctrl)
{
    uint32_t r2 = ctrl.readReg(2);
    std::cout << "Register 2 = 0x" << std::hex << r2 << std::endl;
    ctrl.run(Pulser::ClearTimingCheck());
    ctrl.run(Pulser::TTLPulse(100, 0));

    Pulser::BlockBuilder builder;

    for (int j = 0; j < 1000000; j++) {
        builder.pulseDT(100, Inst::ttlAll, 0);
    }

    for (int j = 0; j < 5; j++) {
        builder.pulseDT(10000000, Inst::ttlAll, 0xffffffff);
        builder.pulseDT(10000000, Inst::ttlAll, 0);
    }

    ctrl.toggleInit();
    Pulser::CtrlState state;

    uint64_t t0 = getTime();
    runInstructionList(&ctrl, &state, builder);

    r2 = ctrl.readReg(2);

    uint64_t t1 = getTime();

    double dt = double(t1 - t0) / TICKS_PER_US;
    bool timingOK = fabs(dt - 1000000) < 1000;

    printf("Detected pulses_finished = %d at t = %8.0f us.  "
           "Register 2 = %08X\n", r2 & 4 ? 1 : 0, dt, r2);
    printf("Wait up to three seconds for pulses_finished to go high\n");

    do {
        t1 = getTime();
        r2 = ctrl.readReg(2);
    } while (((t1 - t0) < 3000 * TICKS_PER_MS) && !(r2 & 4));

    dt = ((float) (t1 - t0)) / TICKS_PER_US;
    timingOK &= fabs(dt - 2000000) < 10;
    printf("Detected pulses_finished = 1 at t = %8.0f us.  "
           "Register 2 = %08X\n", dt, r2);
    assert(timingOK);
}

int
main()
{
    Pulser::Controller ctrl(Pulser::mapPulserAddr());
    Pulser::CtrlLocker locker(ctrl);
    test_register(ctrl, 0);
    test_register(ctrl, 1);
    test_timing(ctrl);
    return 0;
}
