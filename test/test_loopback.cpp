#include <nacs-utils/timer.h>
#include <nacs-old-pulser/pulser.h>
#include <nacs-old-pulser/sequence.h>

#include <mutex>
#include <atomic>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

#include <assert.h>

static constexpr uint32_t N = 10000000;

using namespace NaCs;
using namespace std::literals;

static inline void
read_results(Pulser::Pulser &pulser, uint32_t &i)
{
    uint32_t n_res = pulser.num_results();
    if (!n_res) {
        std::this_thread::yield();
        return;
    }
    for (auto n = n_res;n != 0;n--) {
        auto res = pulser.read_reg(31);
        assert(res == i);
        i++;
    }
}

int
main()
{
    auto &pulser = Pulser::get_pulser();
    pulser.release_hold();
    uint32_t num_read = 0;

    std::thread reader([&] {
            std::this_thread::yield();
            for (;num_read < N;) {
                read_results(pulser, num_read);
            }
        });

    tic();
    for (uint32_t i = 0;i < N;i++) {
        pulser.add(Pulser::LoopBack(i));
        while (i + 1 - num_read > 16) {
            std::this_thread::yield();
        }
    }
    auto write_time = toc();

    std::cout << "Average time per write: "
              << double(write_time) / double(N) / 1e3 << " us"
              << std::endl;
    reader.join();
    return 0;
}
