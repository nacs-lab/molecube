#include "init_system.h"

#include <nacs-utils/log.h>

#include "../parser/parseMisc.h"

#include "spi_util.h"
#include "AD9914.h"
#include "fpga.h"

#include <sys/resource.h>
#include <errno.h>

#include <mutex>

namespace NaCs {

static spi_struct g_spi[NSPI];

Pulser::Pulser
init_system()
{
    std::lock_guard<FLock> fl(g_fPulserLock);

    nacsInfo("Processor clock frequency: %9.3f MHz\n", 1e-6 * CPU_FREQ_HZ);
    nacsLog("NDDS = %d  (REF_CLK = %u MHz)   NSPI = %d\n",
            PULSER_NDDS, (unsigned)(PULSER_AD9914_CLK * 1e-6), (int)NSPI);

    // set priority
    // -20 = highest priority, 0 = default, 19 = lowest priority
    const int nice = -20;
    int ret = setpriority(PRIO_PROCESS, 0, nice);
    if (ret == 0) {
        nacsInfo("Set priority to %d.  SUCCESS\n", nice);
    } else {
        nacsError("Set priority to %d.  FAILURE  ERRNO=%d\n",
                  nice, errno);
    }

    auto pulser = Pulser::get_pulser();
    nacsInfo("Initializing pulse controller at address %p...\n",
             pulser.get_base());
    pulser.init(false);
    nacsLog("Initializing pulse controller...done.\n");

    pulser.clear_timing_check();

    bool spi_active_low[4] = {true, true, false, false};
    char spi_clock_phase[4] = {0, 0, 0, 0};

    for (int i = 0; i < NSPI; i++) {
        nacsInfo("Initializing SPI %d.\n", i);
        SPI_init(g_spi + i, i, spi_active_low[i], spi_clock_phase[i]);
    }

    nacsInfo("Initializing GPIO ... ");
    init_gpio();
    nacsInfo("done.\n\n");

    // detect active DDS
    for (unsigned j = 0;j < PULSER_MAX_NDDS;j++) {
        if (pulser.dds_exists(j)) {
            active_dds.push_back(j);
        }
    }

    // initialize active DDS if necessary
    for (unsigned j = 0;j < active_dds.size();j++) {
        unsigned i = active_dds[j];
        init_AD9914(pulser, i, false);
        print_AD9914_registers(pulser, i);
    }

    return pulser;
}

}
