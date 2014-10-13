/*
 * remap_addr.h
 *
 *  Created on: Jan 11, 2014
 *      Author: trosen
 */

#ifndef REMAP_ADDR_H_
#define REMAP_ADDR_H_

#ifdef LINUX_OS

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

extern unsigned gDebugLevel; //control printf debugging
extern FILE *gLog;

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

//remap physical address to virtual one
void*
remap_device_addr(void* phys_addr)
{
    void *mapped_base = 0;
    void *mapped_dev_base;
    off_t dev_base = (off_t)phys_addr;

    int memfd = open("/dev/mem", O_RDWR | O_SYNC);
    if (memfd == -1) {
        printf("Can't open /dev/mem.  Do you have permission?\n");
        printf("Try from a terminal: more /dev/mem\n");
    }

    if (gDebugLevel > 0)
        fprintf(gLog, "/dev/mem opened.\n");

    // Map one page of memory into user space such that the device is in
    // that page, but it may not be at the start of the page

    mapped_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                       memfd, dev_base & ~MAP_MASK);
    close(memfd);
    if (mapped_base == (void*)-1) {
        printf("Can't map the memory to user space.\n");
        exit(0);
    }
    fprintf(gLog, "Memory mapped address %p to %p.\n", phys_addr, mapped_base);

    // get the address of the device in user space which will be an offset
    // from the base that was mapped as memory is mapped at the start of a page

    mapped_dev_base = mapped_base + (dev_base & MAP_MASK);

    return mapped_dev_base;
}
#endif // LINUX_OS

#endif /* REMAP_ADDR_H_ */
