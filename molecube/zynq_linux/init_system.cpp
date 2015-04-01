#include "init_system.h"

#include <nacs-utils/log.h>
#include <nacs-utils/number.h>
#include <nacs-pulser/controller.h>

#include "spi_util.h"
#include "AD9914.h"
#include "fpga.h"
#include "common.h"

#include <sys/resource.h>
#include <errno.h>
#include <inttypes.h>

#include <mutex>

namespace NaCs {

static spi_struct g_spi[NSPI];

using namespace Pulser;

Controller&
init_system()
{
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

    static Controller ctrl(mapPulserAddr());
    CtrlLocker locker(ctrl);
    nacsInfo("Initializing pulse controller at address %" PRIxPTR "...\n",
             ctrl.getBase());
    ctrl.init(false);
    nacsLog("Initializing pulse controller...done.\n");

    ctrl.run(ClearTimingCheck());

    const bool spi_active_low[max(4, NSPI)] = {true, true, false, false};
    const char spi_clock_phase[max(4, NSPI)] = {0, 0, 0, 0};

    for (uint16_t i = 0; i < NSPI; i++) {
        nacsInfo("Initializing SPI %d.\n", i);
        SPI_init(g_spi + i, i, spi_active_low[i], spi_clock_phase[i]);
    }

    nacsInfo("Initializing GPIO ... ");
    GPIO::init();
    nacsInfo("done.\n\n");

    // detect active DDS
    for (unsigned j = 0;j < PULSER_MAX_NDDS;j++) {
        if (ctrl.run(DDSExists(j))) {
            active_dds.push_back(j);
        }
    }

    // initialize active DDS if necessary
    for (auto i: active_dds) {
        AD9914::init(ctrl, i, false);
        AD9914::print_registers(ctrl, i);
    }

    return ctrl;
}

}
