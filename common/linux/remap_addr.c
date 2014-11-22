/*
 * remap_addr.h
 *
 *  Created on: Jan 11, 2014
 *      Author: trosen
 */

#include "remap_addr.h"

#include <nacs-utils/log.h>
#include <nacs-utils/fd_utils.h>

#include <stdlib.h>

#define MAP_SIZE 4096UL

//remap physical address to virtual one
void*
remap_device_addr(void *phys_addr)
{
    void *map = nacsMapFile("/dev/mem", (intptr_t)phys_addr, MAP_SIZE);
    if (nacsUnlikely(!map)) {
        nacsError("Can't map the memory to user space.\n");
        exit(0);
    }
    nacsInfo("Memory mapped address %p to %p.\n", phys_addr, map);
    return map;
}
