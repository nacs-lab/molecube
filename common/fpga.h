#include "timing.h"

#include <nacs-utils/utils.h>
#include <pulse_controller.h>

#ifndef FPGA_H
#define FPGA_H

//Some basic definitions and includes for the FPGA go here

extern void *pulser; // pointer to pulse controller HW

NACS_BEGIN_DECLS

void init_gpio();
int read_gpio(unsigned channel);
int gpio_set_pin(int channel, int val);

NACS_END_DECLS


#endif // FPGA_H
