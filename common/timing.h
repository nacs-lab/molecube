#include <nacs-utils/utils.h>

#ifndef TIMING_H
#define TIMING_H

NACS_BEGIN_DECLS

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC         (1000000)
#endif

#define TICKS_PER_SECOND       (CLOCKS_PER_SEC)
#define CPU_FREQ_HZ            (667000000)
typedef unsigned long long XTime;
#include <unistd.h> //contains sleep functions

//re-implement XTime_ functions.  Should probably rename these.
void XTime_GetTime(XTime *t);

#define TICKS_PER_10NS (TICKS_PER_SECOND / 100000000.0)
#define TICKS_PER_MICROSECOND (TICKS_PER_SECOND / 1000000.0)
#define TICKS_PER_US (TICKS_PER_SECOND / 1000000.0)
#define TICKS_PER_MS (TICKS_PER_SECOND / 1000.0)

NACS_END_DECLS

#endif //TIMING_H
