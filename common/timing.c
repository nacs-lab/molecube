#include "timing.h"

#include <time.h>

void XTime_GetTime(XTime* xt)
{
    // clock_gettime has 1 ns resolution but requires GLIBC >= 2.17
    //struct timespec t;
    //clock_gettime(CLOCK_MONOTONIC, &t);
    //*xt = t.tv_nsec + 1000000000*t.tv_sec;

    *xt = clock();
}
