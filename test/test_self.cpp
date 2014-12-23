#include <nacs-utils/timer.h>
#include <nacs-pulser/pulser.h>

#include <stdint.h>
#include <cmath>

using namespace NaCs;

static const int skip_dds = 10;
static constexpr int ndds = 22;

void
test_dds(Pulser::Pulser &pulser, int dds_id, unsigned freq_step,
         unsigned num_steps, unsigned &fails, unsigned &runs)
{
    for (unsigned i = 0;i < freq_step * num_steps;i += freq_step) {
        runs++;
        pulser.set_dds_freq(dds_id, i);
        pulser.set_dds_freq(dds_id, i);
        pulser.set_dds_freq(dds_id, i);
        pulser.get_dds_freq(dds_id);
        unsigned read = pulser.get_dds_freq(dds_id);
        if (read != i) {
            fails++;
            // nacsError("DDS: %d: Write %x, read %x\n", dds_id, i, read);
        }
        // if (read != (i | ((i & 0xff0000) << 8))) {
        //     nacsError("DDS: %d: Write %x, read %x\n", dds_id, i, read);
        // }
    }
}

template <typename T>
static inline T
square(T v)
{
    return v * v;
}

int
main()
{
    double fails[ndds] = {0.0};
    double runs[ndds] = {0.0};
    double fails2[ndds] = {0.0};
    static constexpr int ncycle = 0x100;
    auto &pulser = Pulser::get_pulser();
    for (int i = 0;i < ncycle;i++) {
        for (int dds = 0;dds < ndds;dds++) {
            if (dds == skip_dds)
                continue;
            unsigned nfail = 0;
            unsigned nrun = 0;
            test_dds(pulser, dds, 0x10000, 0x80, nfail, nrun);
            fails[dds] += double(nfail) / ncycle;
            runs[dds] += double(nrun) / ncycle;
            fails2[dds] += square(double(nfail)) / ncycle;
        }
    }
    double total_fails = 0;
    double total_runs = 0;
    for (int dds = 0;dds < ndds;dds++) {
        total_fails += fails[dds];
        total_runs += runs[dds];
        if (fails[dds] == 0)
            continue;
        double rate = fails[dds] / runs[dds];
        double unc = std::sqrt(fails2[dds] - square(fails[dds]));
        printf("Failing rate for DDS %d: %.2f+-%.2f%%\n", dds, rate * 100,
               unc / std::sqrt(double(ncycle - 1)) / runs[dds] * 100);
    }
    double total_rate = total_fails / total_runs;
    printf("Total failing rate: %.2f%%\n", total_rate * 100);
    return 0;
}
