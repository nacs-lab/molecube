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
#include <nacs-old-pulser/program.h>
#include <nacs-pulser/commands.h>
#include <math.h>

#include <thread>
#include <chrono>

using namespace std::literals;

namespace NaCs {

namespace self_test {

bool
other_test(Pulser::OldPulser &pulser)
{
    unsigned j, r2;

    pulser.add(Pulser::ClearTimingCheck());

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
        pulser.shortPulse(0x40000000, j);
    }

    std::this_thread::sleep_for(1ms);

    // read back data
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
