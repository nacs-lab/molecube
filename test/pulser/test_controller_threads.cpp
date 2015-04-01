#include <nacs-pulser/controller.h>

#include <nacs-utils/timer.h>

#include <assert.h>
#include <thread>
#include <chrono>
#include <iostream>

using namespace NaCs;
using namespace std::literals;

struct LoopBack2 : Pulser::CompositeCmd<std::tuple<Pulser::LoopBack,
                                                   Pulser::ClockOut,
                                                   Pulser::LoopBack> > {
    LoopBack2(uint32_t v1, uint32_t v2)
        : Pulser::CompositeCmd<std::tuple<Pulser::LoopBack,
                                          Pulser::ClockOut,
                                          Pulser::LoopBack> >(v1, 255, v2)
    {}
    static inline uint64_t
    convertRes(uint32_t res1, uint32_t res2)
    {
        return res1 | (uint64_t(res2) << 32);
    }
};

int
main()
{
    Pulser::Controller ctrl(Pulser::mapPulserAddr());

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
    auto write_loopback3 = [&] {
        for (uint32_t i = 0;i < 128;i++) {
            for (uint32_t j = 0;j < 128;j++) {
                auto res = ctrl.reqSync(LoopBack2(i, j));
                assert(res == (i | (uint64_t(j) << 32)));
            }
        }
    };

    std::thread ts[] = {
        std::thread(write_loopback1),
        std::thread(write_loopback2),
        std::thread(write_loopback1),
        std::thread(write_loopback2),
        std::thread(write_loopback1),
        std::thread(write_loopback2),
        std::thread(write_loopback2),
        std::thread(write_loopback1),
        std::thread(write_loopback3),
        std::thread(write_loopback2),
        std::thread(write_loopback1),
        std::thread(write_loopback1),
        std::thread(write_loopback2),
        std::thread(write_loopback2),
        std::thread(write_loopback1),
        std::thread(write_loopback2),
        std::thread(write_loopback1),
        std::thread(write_loopback3),
    };

    for (auto &t: ts) {
        t.join();
    }

    {
        Pulser::CtrlLocker locker(ctrl);
        for (uint32_t i = 0;i < 128;i++) {
            for (uint32_t j = 0;j < 128;j++) {
                auto res = ctrl.run(LoopBack2(i, j));
                assert(res == (i | (uint64_t(j) << 32)));
            }
        }
    }

    return 0;
}
