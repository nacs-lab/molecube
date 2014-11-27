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

#include <pulse_controller.h>
#include "dds_pulse.h"

#include <math.h>

namespace self_test {

bool
check_register(volatile void *pulse_addr, char n)
{
    unsigned r;
    bool bOK = true;

    r = PULSER_read_sr(pulse_addr, n);
    printf("Register %d = %08X\n", (int) n, r);
    bOK = bOK && (r == 0);

    PULSER_write_sr(pulse_addr, n, 0xFFFFFFFF);
    usleep(100);
    r = PULSER_read_sr(pulse_addr, n);
    printf("Register %d = %08X (wrote 0xFFFFFFFF)\n", (int) n, r);
    bOK = bOK && (r == 0xFFFFFFFF);

    PULSER_write_sr(pulse_addr, n, 0);
    usleep(100);
    r = PULSER_read_sr(pulse_addr, n);
    printf("Register %d = %08X (wrote 0x00000000)\n", (int) n, r);

    bOK = bOK && (r == 0);

    return bOK;
}

bool
check_timing(volatile void *pulse_addr)
{
    unsigned j;

    bool bTimingOK = true;
    check_register(pulse_addr, 0);
    check_register(pulse_addr, 1);

    unsigned r2 = PULSER_read_sr(pulse_addr, 2);
    printf("Register 2 = %08X\n", r2);

    printf("Generating 1,000,000 x 1 us pulses + 10 x 100 ms pulses...\n");

    PULSER_disable_timing_check();
    PULSER_clear_timing_check(pulse_addr);

    PULSER_pulse(pulse_addr, 100, 0, 0);
    PULSER_enable_timing_check();
    uint64_t t0 = nacsGetTime();

    for (j = 0; j < 1000000; j++) {
        PULSER_pulse(pulse_addr, 100, 0, 0);
    }

    for (j = 0; j < 5; j++) {
        PULSER_pulse(pulse_addr, 10000000, 0, 0xFFFFFFFF);
        PULSER_pulse(pulse_addr, 10000000, 0, 0x00000000);
    }

    r2 = PULSER_read_sr(pulse_addr, 2);

    uint64_t t1 = nacsGetTime();
    double dt = ((float) (t1 - t0)) / TICKS_PER_US;
    bTimingOK &= fabs(dt - 1000000) < 1000;

    printf(
        "Detected pulses_finished = %d at t = %8.0f us.  Register 2 = %08X\n",
        r2 & 4 ? 1 : 0, dt, r2);
    printf("Wait up to three seconds for pulses_finished to go high\n");

    do {
        t1 = nacsGetTime();
        r2 = PULSER_read_sr(pulse_addr, 2);
    } while (((t1 - t0) < 3000 * TICKS_PER_MS) && !(r2 & 4));

    dt = ((float) (t1 - t0)) / TICKS_PER_US;
    bTimingOK &= fabs(dt - 2000000) < 10;
    printf(
        "Detected pulses_finished = 1 at t = %8.0f us.  Register 2 = %08X\n",
        dt, r2);

    if (bTimingOK)
        printf("TIMING TEST: PASS.\n");
    else
        printf("TIMING TEST: FAIL.\n");

    return bTimingOK;
}

bool
other_test(volatile void *pulse_addr)
{
    unsigned j, r2;

    PULSER_disable_timing_check();
    PULSER_clear_timing_check(pulse_addr);

    //	PULSER_short_pulse(pulse_addr, (0x20000000), 0); //push PMT counter value onto result FIFO
    //	PULSER_pulse(pulse_addr, 1000000, 0, 0); //10 ms
    //	unsigned r = PULSER_pop_result(pulse_addr);


    for (j = 0; j < PULSER_NDDS; j++) {
        unsigned ftw = PULSER_get_dds_freq(pulse_addr, j);
        printf("DDS[%02i] frequency: %12.3f Hz\n",
               j, FTW2HzD(ftw, PULSER_AD9914_CLK));
    }

    r2 = PULSER_read_sr(pulse_addr, 2);
    printf("Register 2 = %08X (%03d results available)\n", r2, r2 >> 4);

    //loop-back testing
    unsigned nLoopback = 31;
    printf("Pushing values 0...%u onto result FIFO\n", nLoopback - 1);
    for (unsigned j = 0; j < nLoopback; j++) {
        //push value j onto result FIFO
        PULSER_short_pulse(pulse_addr, (0x40000000), j);
    }

    usleep(1000);

    //read back data
    unsigned nResults = 0;
    unsigned k = 0;

    r2 = PULSER_read_sr(pulse_addr, 2);
    nResults = r2 >> 4;
    printf("Register 2 = %08X (%03d results available)\n", r2, nResults);

    bool checkLoopback = true;

    do {
        r2 = PULSER_read_sr(pulse_addr, 2);
        nResults = r2 >> 4;
        //printf("Register 2 = %08X (%03d results available)\n", r2, nResults);

        if (nResults) {
            unsigned r = PULSER_pop_result(pulse_addr);
            checkLoopback &= (k == r);
            //printf("result[%02i] = %d\n", k, r);
            k++;
        }

    } while (nResults);

    checkLoopback &= (k == nLoopback);

    if (checkLoopback)
        printf("LOOP-BACK TEST: PASS.\n");
    else
        printf("LOOP-BACK TEST: FAIL.\n");

    return checkLoopback;
}

}
