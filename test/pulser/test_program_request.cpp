#include <nacs-utils/timer.h>
#include <nacs-pulser/instruction.h>

#include <iostream>

using namespace NaCs;
using Inst = Pulser::InstWriter;
using namespace std::literals;

int
main()
{
    Pulser::Controller ctrl(Pulser::mapPulserAddr());
    Pulser::BlockBuilder builder;
    Timer timer;
    for (int i = 0;i < 6;i++) {
        builder.pulseDT(100000000, Inst::DDS::setFreq, 2, 0);
    }
    timer.print();

    std::cout << builder.size() << std::endl;

    auto send_reqs = [&] {
        std::this_thread::sleep_for(100ms);
        for (int i = 0;i < 50;i++) {
            std::cout << "Start request: " << i << std::endl;
            ctrl.reqSync(Pulser::DDSGetFreq(2));
            std::cout << "Request returns: " << i << std::endl;
        }
    };

    std::thread t(send_reqs);
    std::thread t2(send_reqs);
    std::thread t3(send_reqs);

    {
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
    }

    t.join();
    t2.join();
    t3.join();

    return 0;
}
