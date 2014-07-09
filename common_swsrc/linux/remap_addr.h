/*
 * remap_addr.h
 *
 *  Created on: Jan 11, 2014
 *      Author: trosen
 */

#ifndef REMAP_ADDR_H_
#define REMAP_ADDR_H_

#ifdef __cplusplus
extern "C"
{
#endif

//remap physical address to virtual one
void* remap_device_addr(void* phys_addr);

#ifdef __cplusplus
}
#endif


#endif /* REMAP_ADDR_H_ */
