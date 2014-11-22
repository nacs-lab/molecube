#include <nacs-utils/utils.h>

#ifndef TIMING_H
#define TIMING_H

NACS_BEGIN_DECLS

#ifdef LINUX_OS
#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC         (1000000)
#endif

#define TICKS_PER_SECOND       (CLOCKS_PER_SEC)
#define CPU_FREQ_HZ            (667000000)
typedef unsigned long long XTime;
#include <unistd.h> //contains sleep functions

//re-implement XTime_ functions.  Should probably rename these.
void XTime_GetTime(XTime* t);

#else
#include <xtime_l.h>
#include <xparameters.h>
#include <sleep.h>
#ifdef __arm__
#  define CPU_FREQ_HZ XPAR_CPU_CORTEXA9_CORE_CLOCK_FREQ_HZ
#  define T_PRESCALE 2 /* cycles per clock tick on ARM (Zynq) */
#  define TICKS_PER_SECOND (CPU_FREQ_HZ / (T_PRESCALE))
#endif

#ifdef __PPC__
#  define CPU_FREQ_HZ XPAR_CPU_PPC405_CORE_CLOCK_FREQ_HZ
#  define TICKS_PER_SECOND (CPU_FREQ_HZ)
#endif
#endif

void XTime_wait(unsigned ns10);

#define TICKS_PER_10NS (TICKS_PER_SECOND / 100000000.0)
#define TICKS_PER_MICROSECOND (TICKS_PER_SECOND / 1000000.0)
#define TICKS_PER_US (TICKS_PER_SECOND / 1000000.0)
#define TICKS_PER_MS (TICKS_PER_SECOND / 1000.0)

NACS_END_DECLS

#endif //TIMING_H
