/*
 * remap_addr.h
 *
 *  Created on: Jan 11, 2014
 *      Author: trosen
 */

#include <nacs-utils/utils.h>

#ifndef REMAP_ADDR_H_
#define REMAP_ADDR_H_

NACS_BEGIN_DECLS

//remap physical address to virtual one
void *remap_device_addr(void *phys_addr);

NACS_END_DECLS

#endif
