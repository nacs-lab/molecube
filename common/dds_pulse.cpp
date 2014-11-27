#include "dds_pulse.h"

#include <algorithm>

unsigned
FTW2Hz(unsigned ftw, double fClock)
{
    return (unsigned)floor(ftw * fClock * pow(2., -32) + 0.5);
}

int
FTW2HzI(int ftw)
{
    return (int)floor(ftw * 1.0e9 * pow(2., -32) + 0.5);
}

double
FTW2HzD(unsigned ftw, double fClock)
{
    return ftw * fClock * pow(2., -32);
}

unsigned int
Hz2FTW(double f, double fClock)
{
    return static_cast<unsigned int>(floor(0.5 + (f * pow(2., 32.) / fClock)));
}

int
Hz2FTWI(double f)
{
    return static_cast<int>(floor(0.5 + (f * pow(2., 32.) / 1e9)));
}

unsigned int
MHz2FTW(double f, double fClock)
{
    return Hz2FTW(f * 1e6, fClock);
}

int
MHz2FTWI(double f)
{
    return Hz2FTWI(f * 1e6);
}

double
dds_clk(int)
{
    //return iDDS < NDDS ? 1e9 : AD9914_CLK;
    return AD9914_CLK;
}
