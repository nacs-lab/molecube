#ifndef TTL_PULSE_H_
#define TTL_PULSE_H_

#include <common.h>
#include "fpga.h"

extern bool bDebugPulses;

#define TIME_UNIT    (1e-8)

#define TTL_NOTHING     (0)

#define TTL_START_EXP   (1 << 12)
static inline const char*
TTL_name(unsigned ttl)
{
    (void)ttl;
    return "     wait";
}

static inline unsigned int
us2TW(double t)
{
    return static_cast<unsigned int>(t * 100);
}

static inline unsigned int
ms2TW(double t)
{
    return us2TW(t * 1e3);
}

//make an RF pulse of specified frequency and duration
static inline void
TTL_pulse(unsigned t, unsigned ttl)
{
    if (t > 4) {
        PULSER_pulse(pulser, t, 0, ttl);
    }
}

#endif /*TTL_PULSE_H_*/
