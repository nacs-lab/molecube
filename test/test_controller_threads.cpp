#include <nacs-pulser/commands.h>
#include <nacs-pulser/controller.h>

#include <nacs-utils/timer.h>
#include <nacs-utils/number.h>

#include <nacs-old-pulser/pulser.h>

#include <assert.h>
#include <thread>
#include <chrono>
#include <iostream>

using namespace NaCs;
using namespace std::literals;

int
main()
{
    auto &pulser = Pulser::get_pulser();
    Pulser::Controller ctrl((void*)pulser.get_base());

    auto write_loopback1 = [&] {
        for (uint32_t i = 0;i < 1024;i++) {
            uint32_t res = ctrl.reqSync(Pulser::LoopBack(i));
            assert(i == res);
        }
    };
    auto write_loopback2 = [&] {
        for (uint32_t i = 1024;i > 0;i--) {
            uint32_t res = ctrl.reqSync(Pulser::LoopBack(i));
            assert(i == res);
        }
    };

    std::thread ts[16] = {
        std::thread(write_loopback1),
        std::thread(write_loopback2),
        std::thread(write_loopback1),
        std::thread(write_loopback2),
        std::thread(write_loopback1),
        std::thread(write_loopback2),
        std::thread(write_loopback2),
        std::thread(write_loopback1),
        std::thread(write_loopback2),
        std::thread(write_loopback1),
        std::thread(write_loopback1),
        std::thread(write_loopback2),
        std::thread(write_loopback2),
        std::thread(write_loopback1),
        std::thread(write_loopback2),
        std::thread(write_loopback1)
    };

    for (auto &t: ts) {
        t.join();
    }

    return 0;
}
