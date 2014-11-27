#include <nacs-utils/utils.h>

#ifndef FPGA_H
#define FPGA_H

NACS_BEGIN_DECLS

void init_gpio();
void close_gpio();

int gpio_set_pin(int channel, int val);

int read_gpio(unsigned channel);

NACS_END_DECLS


#endif // FPGA_H
