#include <nacs-pulser/controller.h>

#include <nacs-utils/timer.h>

#include <assert.h>
#include <thread>
#include <chrono>
#include <iostream>

using namespace NaCs;
using namespace std::literals;

int
main()
{
    Pulser::Controller ctrl(Pulser::mapPulserAddr());

    {
        Pulser::CtrlLocker locker(ctrl);
        int64_t prev_val = 0;
        auto get_expect = [&] (int32_t w) {
            uint32_t res = uint32_t(w * prev_val);
            prev_val = w;
            return res;
        };
        for (uint32_t i = 0;i < 1024;i++) {
            auto res = ctrl.run(Pulser::LoopBack(i));
            auto expect = get_expect(i);
            if (res != expect) {
                std::cout << "write: " << i << ", read: " << res
                          << ", expect: " << expect << std::endl;
            }
        }
        for (int32_t i = 0;i < 1024;i++) {
            auto res = ctrl.run(Pulser::LoopBack(uint32_t(-i)));
            auto expect = get_expect(-i);
            if (res != expect)
                std::cout << "write: " << -i << ", read: "
                          << int32_t(res)
                          << ", expect: " << expect << std::endl;
        }
    }

    return 0;
}
