#include "driver.h"
#include "xparameters.h"

#include <nacs-utils/utils.h>
#include <nacs-utils/fd_utils.h>
#include <nacs-utils/log.h>

namespace NaCs {
namespace Pulser {

NACS_EXPORT() volatile void*
mapPulserAddr()
{
    // Can also be determined by looking for
    // /proc/device-tree/amba@0/pulse-controller@73000000
    // with propery device tree file.
    static constexpr auto phy_addr = XPAR_PULSE_CONTROLLER_0_BASEADDR;
    static_assert(sizeof(off_t) == 8, "");
    static auto map_addr = [] {
        Log::info("Initializing pulse controller\n");
        auto addr = mapFile("/dev/mem", phy_addr, 4096);
        if (unlikely(!addr)) {
            Log::error("Can't map the memory to user space.\n");
            exit(1);
        }
        return addr;
    }();
    return map_addr;
}

}
}
