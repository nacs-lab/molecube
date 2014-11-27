#include "dds_pulse.h"

#include <algorithm>

double
FTW2HzD(unsigned ftw, double fClock)
{
    return ftw * fClock * pow(2., -32);
}

unsigned
Hz2FTW(double f, double fClock)
{
    return static_cast<unsigned>(floor(0.5 + (f * pow(2., 32.) / fClock)));
}

double
dds_clk(int)
{
    // return iDDS < NDDS ? 1e9 : AD9914_CLK;
    return AD9914_CLK;
}
