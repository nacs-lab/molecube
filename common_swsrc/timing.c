#include "timing.h"

#ifdef LINUX_OS
#include <time.h>

void XTime_GetTime(XTime* xt)
{
    // clock_gettime has 1 ns resolution but requires GLIBC >= 2.17
    //struct timespec t;
    //clock_gettime(CLOCK_MONOTONIC, &t);
    //*xt = t.tv_nsec + 1000000000*t.tv_sec;

    *xt = clock();
}
#endif

#ifdef ALUMINIZER_SIM
unsigned long long nTicks = 0;

void sim_increment_time(int us)
{
    nTicks += us * 200;
}

void XTime_GetTime(XTime* t)
{
    *t = nTicks;
    nTicks += 400;
}
#endif //ALUMINIZER_SIM
