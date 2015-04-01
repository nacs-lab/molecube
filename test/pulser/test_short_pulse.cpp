#include <nacs-utils/timer.h>
#include <nacs-pulser/controller.h>

#include <stdint.h>

#include <iostream>
#include <cmath>

using namespace NaCs;

static inline void
test_short_pulse(Pulser::Controller &ctrl, uint32_t t)
{
    ctrl.releaseHold();
    tic();
    ctrl.shortPulse(0x20000000 | t, 0);
    while (!ctrl.isFinished()) {
    }
    auto time = toc();
    std::cout << "Average time per run for t = 0x" << std::hex << t
              << ": " << double(time) / 1e6 << " ms"
              << std::endl;
}

int
main()
{
    Pulser::Controller ctrl(Pulser::mapPulserAddr());
    Pulser::CtrlLocker locker(ctrl);
    test_short_pulse(ctrl, 0x1fffff);
    test_short_pulse(ctrl, 0x3fffff);
    test_short_pulse(ctrl, 0x7fffff);
    test_short_pulse(ctrl, 0xffffff);
    return 0;
}
