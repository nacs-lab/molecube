#include "ttl_pulse.h"
#include <stdio.h>
#include "fpga.h"

unsigned g_tSequence=0; //accumulated sequence duration in PULSER units

void
print_timing_info(verbosity *v, unsigned t0, unsigned dt, char last)
{
  v->printf("%8.2f ..%8.2f (%8.2f) us%c", t0 * PULSER_DT_us,
            (t0 + dt) * PULSER_DT_us, dt * PULSER_DT_us, last);
}

void
print_pulse_info(verbosity *v, unsigned t, unsigned ttl, const char *info)
{

    v->printf("%12s          TTL = %08X %4s ", TTL_name(ttl), ttl, "");

    print_timing_info(v, g_tSequence, t, info ? ' ' : '\n');

    if (info) {
        v->printf("(%s)\n");
    }
}
