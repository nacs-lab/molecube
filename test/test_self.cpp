#include <nacs-utils/timer.h>
#include <nacs-utils/number.h>
#include <nacs-pulser/controller.h>

#include <stdint.h>

using namespace NaCs;

static constexpr int ndds = 22;

void
test_dds(Pulser::Controller &ctrl, int dds_id, unsigned freq_step,
         unsigned num_steps, unsigned &fails, unsigned &runs)
{
    for (unsigned i = 0;i < num_steps;i++) {
        runs++;
        unsigned fword = (i + 1) * freq_step - 1;
        ctrl.run(Pulser::DDSSetFreq(dds_id, fword));
        unsigned read = ctrl.run(Pulser::DDSGetFreq(dds_id));
        if (read != fword) {
            fails++;
        }
    }
}

int
main()
{
    double fails[ndds] = {0.0};
    double runs[ndds] = {0.0};
    double fails2[ndds] = {0.0};
    const constexpr int ncycle = 0x100;
    bool dds_exists[ndds] = {};
    Pulser::Controller ctrl(Pulser::mapPulserAddr());
    std::lock_guard<Pulser::Controller> locker(ctrl);

    auto prev_time = time(nullptr);

    for (int dds = 0;dds < ndds;dds++) {
        dds_exists[dds] = ctrl.run(Pulser::DDSExists(dds));
        if (!dds_exists[dds]) {
            printf("Missing DDS: %d\n", dds);
        }
    }

    for (int i = 0;i < ncycle;i++) {
        for (int dds = 0;dds < ndds;dds++) {
            if (!dds_exists[dds])
                continue;
            unsigned nfail = 0;
            unsigned nrun = 0;
            test_dds(ctrl, dds, 0x80000, 0x800, nfail, nrun);
            fails[dds] += double(nfail) / ncycle;
            runs[dds] += double(nrun) / ncycle;
            fails2[dds] += square(double(nfail)) / ncycle;
        }
        auto new_time = time(nullptr);
        if (new_time > prev_time) {
            prev_time = new_time;
            printf("%d / %d (%.3f%%) done.\n",
                   i + 1, ncycle, double(i + 1) / ncycle * 100);
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
        printf("Failing rate for DDS %d: %.3f+-%.3f%%\n", dds, rate * 100,
               unc / std::sqrt(double(ncycle - 1)) / runs[dds] * 100);
    }
    double total_rate = total_fails / total_runs;
    printf("Total failing rate: %.3f%%\n", total_rate * 100);
    return 0;
}
