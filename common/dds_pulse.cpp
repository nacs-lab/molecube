#include "dds_pulse.h"

#include <algorithm>

unsigned
Hz2FTW(double f, double fClock)
{
    return static_cast<unsigned>(floor(0.5 + (f * pow(2., 32.) / fClock)));
}
