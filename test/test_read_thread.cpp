#include <nacs-utils/timer.h>
#include <nacs-pulser/pulser.h>
#include <nacs-pulser/sequence.h>

#include <mutex>
#include <atomic>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

#include <assert.h>

static constexpr int N = 100000000;

using namespace NaCs;
using namespace std::literals;

static uint32_t
pulser_num_results(Pulser::Pulser &pulser)
{
    return (pulser.read_reg(2) >> 4) & 31;
}

uint64_t
get_time()
{
    using namespace std::chrono;
    return time_point_cast<nanoseconds>(system_clock::now())
        .time_since_epoch().count();
}

static inline uint32_t
read_results(Pulser::Pulser &pulser)
{
    uint32_t n_res = pulser_num_results(pulser);
    for (auto n = n_res;n != 0;n--) {
        pulser.read_reg(31);
    }
    return n_res;
}

int
main()
{
    auto &pulser = Pulser::get_pulser();
    volatile bool finished = false;
    volatile int res_read = 0;
    read_results(pulser);
    std::thread reader([&] {
            while (!finished) {
                res_read += read_results(pulser);
            }
        });

    tic();
    for (int i = 0;i < N;i++) {
        pulser.short_pulse(0x10000003, 0);
        while (i + 1 - res_read > 16) {
            // std::cout << "Sleep: " << i << ", " << res_read << std::endl;
            // std::this_thread::sleep_for(1us);
            std::this_thread::yield();
        }
    }
    auto write_time = toc();

    std::cout << "Average time per write: "
              << double(write_time) / double(N) / 1e3 << " us"
              << std::endl;
    std::cout << "res_read: " << res_read << std::endl;
    std::this_thread::sleep_for(1ms);
    finished = true;
    reader.join();
    std::cout << "N: " << N << std::endl;
    std::cout << "res_read: " << res_read << std::endl;
    assert(res_read == N);
    return 0;
}
