#ifndef FPGA_H
#define FPGA_H

//Some basic definitions and includes for the FPGA go here

extern void* pulser; // pointer to pulse controller HW

#ifdef ALUMINIZER_SIM

#ifdef __cplusplus
#include <vector>
extern std::vector<short> AD7656_values;
extern std::vector<short> AD5668_values;
#endif

#ifdef __cplusplus
extern "C"
{
void sim_increment_time(int us);
void XTime_GetTime(XTime* t);
}
#else
void sim_increment_time(int us);
void XTime_GetTime(XTime* t);
#endif

#include <pulse_controller.h>

#define __INTEL__
#define CPU_FREQ_HZ 4000000000
#define TICKS_PER_SECOND (CPU_FREQ_HZ)

#define XPAR_CPU_PPC405_CORE_CLOCK_FREQ_HZ 200000000
#define XPAR_PULSE_CONTROLLER_0_BASEADDR (0)


typedef unsigned XGpio;
unsigned XGpio_DiscreteRead(XGpio* TTLchan, unsigned /*dir*/);

#ifndef WIN32
#include "unistd.h"
#endif

#else //ALUMINIZER_SIM


#ifdef __cplusplus
extern "C"
{
#include <pulse_controller.h>
}
#else
#include <pulse_controller.h>
#endif



#endif //ALUMINIZER_SIM

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
