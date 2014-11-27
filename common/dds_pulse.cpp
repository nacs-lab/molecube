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
