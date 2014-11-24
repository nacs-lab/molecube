#include "init_platform.h"
#include <nacs-utils/log.h>
#include <nacs-utils/fd_utils.h>
#include <nacs-xspi/xparameters.h>

#include <common.h>

// Platform specific initialization
// Linux: get device addresses

static intptr_t
get_pulse_controller_phys_addr()
{
    return XPAR_PULSE_CONTROLLER_0_BASEADDR;

    //code below works too, but it is slow (20 ms or so)

    /*
     * These next lines may stop working if the output format of ls -d changes.
     *
     * Works ok for BusyBox v1.20.1 (2012-11-27 13:37:12 MST) multi-call binary.
     *
     *    zynq> ls -d /proc/device-tree/amba@0/pulse-controller*
     *    /proc/device-tree/amba@0/pulse-controller@73000000
     */

    // const char cmd[] = "find /proc/device-tree -name pulse-controller*";
    // char ret[200];

    // if (addr == 0) {
    //     if (FILE* f = popen(cmd, "r")) {
    //         if (fgets(ret, 200, f)) {
    //             char* p = strstr(ret, "pulse-controller");
    //             if (p) {
    //                 sscanf(p, "pulse-controller@%p", &addr);
    //             }
    //         }
    //         fclose(f);
    //     }
    // }

    // if (0 == addr) {
    //     printf("Can't determine address of pulse-controller!\n");
    //     printf("Command: %s\nReturn string: %s\n", cmd, ret);
    //     printf("So sad...\n");
    //     exit(0);
    // }

    // nacsInfo("Pulse controller physical address is %p\n", addr);
    // return addr;
}

volatile void*
init_pulse_controller()
{
    nacsInfo("Initializing pulse controller\n");
    volatile void *pulse_addr =
        nacsMapFile("/dev/mem", get_pulse_controller_phys_addr(), 4096);
    if (nacsUnlikely(!pulse_addr)) {
        nacsError("Can't map the memory to user space.\n");
        exit(0);
    }
    return pulse_addr;
}
