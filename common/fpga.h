#ifndef FPGA_H
#define FPGA_H

//Some basic definitions and includes for the FPGA go here

extern void *pulser; // pointer to pulse controller HW

#ifdef __cplusplus
extern "C"
{
#include <pulse_controller.h>
}
#else
#include <pulse_controller.h>
#endif

#ifdef PLATFORM_ZYNQ
#define NO_LOG_UART
#endif

#ifdef __cplusplus
extern "C"
{
#endif
void init_gpio();
int read_gpio(unsigned channel);
int gpio_set_pin(int channel, int val);
#ifdef __cplusplus
}
#endif

#include "timing.h"

#endif // FPGA_H
