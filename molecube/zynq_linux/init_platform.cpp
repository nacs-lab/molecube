#include "init_platform.h"
#include <nacs-utils/log.h>
#include <nacs-utils/fd_utils.h>
#include <nacs-xspi/xparameters.h>

// Platform specific initialization
// Linux: get device addresses

namespace NaCs {

static intptr_t
get_pulser_phys_addr()
{
    // Can also be determined by looking for
    // /proc/device-tree/amba@0/pulse-controller@73000000
    // with propery device tree file.

    return XPAR_PULSE_CONTROLLER_0_BASEADDR;
}

Pulser::Pulser
get_pulser()
{
    nacsInfo("Initializing pulse controller\n");
    auto addr = nacsMapFile("/dev/mem", get_pulser_phys_addr(), 4096);
    if (nacsUnlikely(!addr)) {
        nacsError("Can't map the memory to user space.\n");
        exit(0);
    }
    return Pulser::Pulser(addr);
}

}
