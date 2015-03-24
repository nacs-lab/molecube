#include <nacs-utils/timer.h>
#include <nacs-old-pulser/pulser.h>
#include <nacs-old-pulser/sequence.h>

using namespace NaCs;

int
main()
{
    auto &pulser = Pulser::get_pulser();
    Pulser::SequenceBuilder builder;
    builder.enable_timing_check();

    for (int i = 0;i < 10000;i++) {
        builder.push_ttl_all((i + 1) * 5, 0);
    }
    builder.finish_ttl();

    NaCs::tic();
    pulser.set_hold();
    pulser.toggle_init();
    pulser.run(builder);
    NaCs::printToc();

    // wait for pulses finished.
    pulser.wait();

    return 0;
}
