#include <nacs-utils/timer.h>
#include <nacs-pulser/pulser.h>
#include <nacs-pulser/sequence.h>

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

    nacsTic();
    pulser.set_hold();
    pulser.toggle_init();
    pulser.run(builder);
    nacsPrintToc();

    // wait for pulses finished.
    pulser.wait();

    return 0;
}
