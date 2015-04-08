#include "init_system.h"

#include <nacs-utils/log.h>
#include <nacs-utils/number.h>
#include <nacs-pulser/controller.h>

#include "spi_util.h"
#include "AD9914.h"
#include "common.h"

#include <sys/resource.h>
#include <errno.h>
#include <inttypes.h>

#include <mutex>

namespace NaCs {

using namespace Pulser;

Controller&
init_system()
{
    nacsInfo("Processor clock frequency: %9.3f MHz\n", 1e-6 * CPU_FREQ_HZ);
    nacsLog("NDDS = %d  (REF_CLK = %u MHz)   NSPI = %d\n",
            PULSER_NDDS, (unsigned)(PULSER_AD9914_CLK * 1e-6), 2);

    // set priority
    // -20 = highest priority, 0 = default, 19 = lowest priority
    const int nice = -20;
    int ret = setpriority(PRIO_PROCESS, 0, nice);
    if (ret == 0) {
        nacsInfo("Set priority to %d.  SUCCESS\n", nice);
    } else {
        nacsError("Set priority to %d.  FAILURE  ERRNO=%d\n", nice, errno);
    }

    static Controller ctrl(mapPulserAddr());
    CtrlLocker locker(ctrl);
    ctrl.init(false);
    nacsLog("Initializing pulse controller...done.\n");

    ctrl.run(ClearTimingCheck());

    XSpi spis[2] = {};
    for (uint16_t i = 0; i < 2; i++) {
        nacsInfo("Initializing SPI %d.\n", i);
        SPI_init(spis + i, i);
    }

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
