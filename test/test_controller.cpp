#include <nacs-utils/timer.h>
#include <nacs-utils/number.h>
#include <nacs-pulser/controller.h>

#include <assert.h>
#include <thread>
#include <chrono>

using namespace NaCs;
using namespace std::literals;

int
main()
{
    uint32_t dummyRegs[32];
    Pulser::Controller ctrl(dummyRegs);
    Pulser::Request req(ctrl, 0, 0, 50, true);
    ctrl.setRes(req, 10);
    ctrl.wait(req);
    assert(req.res == 10);

    Pulser::Request req2(ctrl, 0, 0, 50, true);
    Pulser::Request req3(ctrl, 0, 0, 50, true);
    Pulser::Request req4(ctrl, 0, 0, 50, true);
    std::thread t([&] {
            std::this_thread::sleep_for(1s);
            ctrl.setRes(req2, 20);
            ctrl.setRes(req3, 30);
            ctrl.setRes(req4, 40);
        });
    ctrl.wait(req3);
    ctrl.wait(req2);
    ctrl.wait(req4);
    assert(req2.res == 20);
    assert(req3.res == 30);
    assert(req4.res == 40);
    t.join();

    return 0;
}
