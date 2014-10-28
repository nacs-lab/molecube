#include <common.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/mman.h>
#include <sched.h>
#include <errno.h>

#include "init_platform.h"
#include "remap_addr.h"

#include <xparameters.h>

//Platform specific initialization
//Linux: get device addresses

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

extern FILE* gLog;
void *mapped_base = 0;
int memfd = 0;

void*
get_pulse_controller_phys_addr()
{
    void *addr = (void*)XPAR_PULSE_CONTROLLER_0_BASEADDR;

    return addr;

    //code below works too, but it is slow (20 ms or so)

    /*
     * These next lines may stop working if the output format of ls -d changes.
     *
     * Works ok for BusyBox v1.20.1 (2012-11-27 13:37:12 MST) multi-call binary.
     *
     *    zynq> ls -d /proc/device-tree/amba@0/pulse-controller*
     *    /proc/device-tree/amba@0/pulse-controller@73000000
     */

    const char cmd[] = "find /proc/device-tree -name pulse-controller*";
    char ret[200];

    if (addr == 0) {
        if (FILE* f = popen(cmd, "r")) {
            if (fgets(ret, 200, f)) {
                char* p = strstr(ret, "pulse-controller");
                if (p) {
                    sscanf(p, "pulse-controller@%p", &addr);
                }
            }
            fclose(f);
        }
    }

    if (0 == addr) {
        printf("Can't determine address of pulse-controller!\n");
        printf("Command: %s\nReturn string: %s\n", cmd, ret);
        printf("So sad...\n");
        exit(0);
    }

    fprintf(gLog, "Pulse controller physical address is %p\r\n", addr);
    return addr;
}

void close_pulse_controller_mapped_addr()
{
    // unmap the memory before exiting
    if (munmap(mapped_base, MAP_SIZE) == -1) {
        printf("Can't unmap memory from user space.\n");
    }

    close(memfd);
    fprintf(gLog, "Memory unmapped from %p.\n", mapped_base);
}

void init_pulse_controller()
{
    fprintf(gLog, "Initializing pulse controller\r\n");
    void* phys_addr = get_pulse_controller_phys_addr();
    pulser = remap_device_addr(phys_addr);
}
