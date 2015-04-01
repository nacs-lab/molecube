#include <nacs-utils/timer.h>
#include <nacs-pulser/instruction.h>

using namespace NaCs;
using Inst = Pulser::InstWriter;

int
main()
{
    Pulser::Controller ctrl(Pulser::mapPulserAddr());
    Pulser::BlockBuilder builder;
    builder.pushPulse(Inst::enableTimingCheck);
    for (int i = 0;i < 10000;i++) {
        builder.pushPulse(Inst::ttlAll, 0);
        builder.pushPulse(Inst::wait, 5);
    }
    builder.pushPulse(Inst::disableTimingCheck);
    builder.pushPulse(Inst::wait, 3);

    tic();
    ctrl.setHold();
    ctrl.toggleInit();
    Pulser::CtrlState state;
    runInstructionList(&ctrl, &state, builder.data(), builder.size());
    printToc();

    // wait for pulses finished.
    ctrl.waitFinish();

    return 0;
}
