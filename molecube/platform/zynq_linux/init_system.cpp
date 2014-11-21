#include "init_system.h"
#include "init_platform.h"

#include "fpga.h"
#include "common.h"
#include "dac.h"
#include "self_test.h"
#include "AD9914.h"
#include "dds_pulse.h"
#include "../../parser/parseMisc.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>

static spi_struct g_spi[NSPI];

void init_system()
{
    flocker fl(g_fPulserLock);

    fprintf(gLog, "Processor clock frequency: %9.3f MHz\r\n",
            1e-6 * CPU_FREQ_HZ);
    fprintf(gLog, "NDDS = %d  (REF_CLK = %u MHz)   NSPI = %d\n",
            (int)NDDS, (unsigned)(AD9914_CLK * 1e-6), (int)NSPI);
    fflush(gLog);

    // set priority
    // -20 = highest priority, 0 = default, 19 = lowest priority
    const int nice = -20;
    int ret = setpriority(PRIO_PROCESS, 0, nice);
    if (ret == 0) {
        fprintf(gLog, "Set priority to %d.  SUCCESS\n", nice);
    } else {
        fprintf(gLog, "Set priority to %d.  FAILURE  ERRNO=%d\n",
                nice, errno);
    }

    init_pulse_controller();
    fprintf(gLog, "Initializing pulse controller at address %p...\r\n",
            (void*)pulser);
    PULSER_init(pulser, NDDS, false, gDebugLevel);
    fprintf(gLog, "Initializing pulse controller...done.\r\n");
    fflush(gLog);

    PULSER_disable_timing_check(pulser);
    PULSER_clear_timing_check(pulser);

    bool spi_active_low[4] = {true, true, false, false};
    char spi_clock_phase[4] = {0, 0, 0, 0};

    for (int i = 0; i < NSPI; i++) {
        fprintf(gLog, "Initializing SPI %d.\r\n", i);

        SPI_init(g_spi + i, i, spi_active_low[i],
                 spi_clock_phase[i], gDebugLevel);
    }

    fprintf(gLog, "Initializing GPIO ... ");
    init_gpio();
    fprintf(gLog, "done.\n\n");
    fflush(gLog);

    // detect active DDS
    for (unsigned j = 0;j < PULSER_MAX_NDDS;j++) {
        if (PULSER_dds_exists(pulser, j)) {
            active_dds.push_back(j);
        }
    }

    // initialize active DDS if necessary
    for (unsigned j = 0;j < active_dds.size();j++) {
        unsigned i = active_dds[j];
        init_AD9914(pulser, i, false, gLog);
        print_AD9914_registers(pulser, i, gLog);
        fflush(gLog);
    }
}
