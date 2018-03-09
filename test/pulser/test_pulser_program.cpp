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
    Timer timer;
    for (int i = 0;i < 10000000;i++) {
        builder.pulseDT(40, Inst::ttlAll, 0);
    }
    timer.print();

    std::cout << builder.size() << std::endl;

    Pulser::CtrlLocker locker(ctrl);
    ctrl.setHold();
    ctrl.toggleInit();
    Pulser::CtrlState state;

    timer.restart();
    runInstructionList(&ctrl, &state, builder);
    // wait for pulses finished.
    ctrl.waitFinish();
    timer.print();
    std::cout << "TimingOK: " << ctrl.timingOK() << std::endl;

    return 0;
}
