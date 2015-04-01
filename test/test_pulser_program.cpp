#include <nacs-utils/timer.h>
#include <nacs-pulser/instruction.h>

#include <iostream>

using namespace NaCs;
using Inst = Pulser::InstWriter;

int
main()
{
    Pulser::Controller ctrl(Pulser::mapPulserAddr());
    Pulser::BlockBuilder builder;
    builder.pushPulse(Inst::enableTimingCheck);
    for (int i = 0;i < 10000000;i++) {
        builder.pulseDT(100, Inst::ttlAll, 0);
    }
    builder.finalPulse();

    Pulser::CtrlLocker locker(ctrl);
    ctrl.setHold();
    ctrl.toggleInit();
    Pulser::CtrlState state;

    tic();
    // tic();
    runInstructionList(&ctrl, &state, builder.data(), builder.size());
    // printToc();
    // wait for pulses finished.
    ctrl.waitFinish();
    printToc();
    std::cout << "TimingOK: " << ctrl.timingOK() << std::endl;

    return 0;
}
