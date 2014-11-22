/*
 * remap_addr.h
 *
 *  Created on: Jan 11, 2014
 *      Author: trosen
 */

#include <nacs-utils/utils.h>
#include <nacs-utils/log.h>
#include <nacs-utils/fd_utils.h>

#include <stdlib.h>

#ifndef REMAP_ADDR_H_
#define REMAP_ADDR_H_

//remap physical address to virtual one
static NACS_INLINE void*
remap_device_addr(void *phys_addr)
{
    void *map = nacsMapFile("/dev/mem", (intptr_t)phys_addr, 4096);
    if (nacsUnlikely(!map)) {
        nacsError("Can't map the memory to user space.\n");
        exit(0);
    }
    nacsInfo("Memory mapped address %p to %p.\n", phys_addr, map);
    return map;
}

#endif
