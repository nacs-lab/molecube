/*
 * self_test.cpp
 *
 *  Created on: May 17, 2013
 *      Author: trosen
 *
 *  Perform some self-tests of FPGA functions.
 *  Functions are probably called at startup.
 */

#include "self_test.h"

#include <nacs-utils/timer.h>
#include <nacs-pulser/program.h>

#include <math.h>

namespace NaCs {

namespace self_test {

bool
check_register(Pulser::Pulser &pulser, int n)
{
    unsigned r;
    bool bOK = true;

    r = pulser.read_reg(n);
    printf("Register %d = %08X\n", n, r);
    bOK = bOK && (r == 0);

    pulser.write_reg(n, 0xFFFFFFFF);
    usleep(100);
    r = pulser.read_reg(n);
    printf("Register %d = %08X (wrote 0xFFFFFFFF)\n", n, r);
    bOK = bOK && (r == 0xFFFFFFFF);

    pulser.write_reg(n, 0);
    usleep(100);
    r = pulser.read_reg(n);
    printf("Register %d = %08X (wrote 0x00000000)\n", n, r);

    bOK = bOK && (r == 0);

    return bOK;
}

bool
check_timing(Pulser::Pulser &pulser)
{
    unsigned j;

    bool bTimingOK = true;
    check_register(pulser, 0);
    check_register(pulser, 1);

    unsigned r2 = pulser.read_reg(2);
    printf("Register 2 = %08X\n", r2);

    printf("Generating 1,000,000 x 1 us pulses + 10 x 100 ms pulses...\n");

    pulser.clear_timing_check();

    pulser.pulse(100, 0, 0);

    Pulser::Program prog(true);
    prog.enable_timing_check();

    for (j = 0; j < 1000000; j++) {
        prog.pulse(100, 0, 0);
    }

    for (j = 0; j < 5; j++) {
        prog.pulse(10000000, 0, 0xFFFFFFFF);
        prog.pulse(10000000, 0, 0x00000000);
    }

    uint64_t t0 = getTime();
    pulser.run(prog);

    r2 = pulser.read_reg(2);

    uint64_t t1 = getTime();

    double dt = ((float)(t1 - t0)) / TICKS_PER_US;
    bTimingOK &= fabs(dt - 1000000) < 1000;

    printf("Detected pulses_finished = %d at t = %8.0f us.  "
           "Register 2 = %08X\n", r2 & 4 ? 1 : 0, dt, r2);
    printf("Wait up to three seconds for pulses_finished to go high\n");

    do {
        t1 = getTime();
        r2 = pulser.read_reg(2);
    } while (((t1 - t0) < 3000 * TICKS_PER_MS) && !(r2 & 4));

    dt = ((float) (t1 - t0)) / TICKS_PER_US;
    bTimingOK &= fabs(dt - 2000000) < 10;
    printf("Detected pulses_finished = 1 at t = %8.0f us.  "
           "Register 2 = %08X\n", dt, r2);

    if (bTimingOK) {
        printf("TIMING TEST: PASS.\n");
    } else {
        printf("TIMING TEST: FAIL.\n");
    }

    return bTimingOK;
}

bool
other_test(Pulser::Pulser &pulser)
{
    unsigned j, r2;

    pulser.clear_timing_check();

    // //push PMT counter value onto result FIFO
    // pulser.short_pulse(0x20000000, 0);
    // pulser.pulse(1000000, 0, 0); // 10 ms
    // unsigned r = pulser.pop_result();


    for (j = 0; j < PULSER_NDDS; j++) {
        double freq = pulser.get_dds_freq_f(j);
        printf("DDS[%02i] frequency: %12.3f Hz\n", j, freq);
    }

    r2 = pulser.read_reg(2);
    printf("Register 2 = %08X (%03d results available)\n", r2, r2 >> 4);

    // loop-back testing
    unsigned nLoopback = 31;
    printf("Pushing values 0...%u onto result FIFO\n", nLoopback - 1);
    for (unsigned j = 0; j < nLoopback; j++) {
        // push value j onto result FIFO
        pulser.short_pulse(0x40000000, j);
    }

    usleep(1000);

    //read back data
    unsigned nResults = 0;
    unsigned k = 0;

    r2 = pulser.read_reg(2);
    nResults = r2 >> 4;
    printf("Register 2 = %08X (%03d results available)\n", r2, nResults);

    bool checkLoopback = true;

    do {
        r2 = pulser.read_reg(2);
        nResults = r2 >> 4;
        // printf("Register 2 = %08X (%03d results available)\n",
        //        r2, nResults);

        if (nResults) {
            unsigned r = pulser.pop_result();
            checkLoopback &= (k == r);
            // printf("result[%02i] = %d\n", k, r);
            k++;
        }

    } while (nResults);

    checkLoopback &= (k == nLoopback);

    if (checkLoopback) {
        printf("LOOP-BACK TEST: PASS.\n");
    } else {
        printf("LOOP-BACK TEST: FAIL.\n");
    }
    return checkLoopback;
}

}
}
