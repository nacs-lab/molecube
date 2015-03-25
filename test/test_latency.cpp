#include <nacs-utils/timer.h>
#include <nacs-utils/number.h>
#include <nacs-old-pulser/pulser.h>
#include <nacs-old-pulser/commands.h>

#include <stdint.h>

#include <iostream>
#include <iomanip>
#include <cmath>

using namespace NaCs;

template<bool wait=true>
double
test_dds(Pulser::Pulser &pulser, unsigned nrun)
{
    pulser.release_hold();
    tic();
    for (unsigned i = 0;i < nrun;i++) {
        setDDSFreq(pulser, 0, 0);
    }
    if (wait) {
        while (!pulser.is_finished()) {
        }
    }
    auto time = toc();
    if (!wait) {
        while (!pulser.is_finished()) {
        }
    }
    return double(time) / double(nrun) / 1e3;
}

template<bool wait=true>
void
test_latencies(Pulser::Pulser &pulser, unsigned ncycles)
{
    for (unsigned i = 1;i <= 16;i++) {
        auto nrun = i * 256;
        std::cout << "Time per write for "
                  << std::setw(4) << nrun << " Pulses: ";
        double tmin = 0;
        double tmax = 0;
        double sum = 0;
        double sum2 = 0;
        for (unsigned j = 0;j < ncycles;j++) {
            auto t = test_dds<wait>(pulser, nrun);
            if (tmin == 0) {
                tmin = t;
            } else {
                tmin = min(t, tmin);
            }
            tmax = max(t, tmax);
            sum += t;
            sum2 += t * t;
        }
        double taverage = sum / ncycles;
        double tstd = std::sqrt(sum2 / ncycles - taverage * taverage);
        std::cout << "mean (us): " << std::fixed
                  << std::setw(6) << std::setprecision(4) << taverage;
        std::cout << ", std (ns): " << std::fixed
                  << std::setw(7) << std::setprecision(4) << tstd * 1e3;
        std::cout << ", min (us): " << std::fixed
                  << std::setw(6) << std::setprecision(4) << tmin;
        std::cout << ", max (us): " << std::fixed
                  << std::setw(6) << std::setprecision(4)
                  << tmax << std::endl;
    }
}

int
main()
{
    auto &pulser = Pulser::get_pulser();
    test_latencies<true>(pulser, 4096);
    test_latencies<false>(pulser, 4096);
    return 0;
}
