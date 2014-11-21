#ifndef TTL_PULSE_H_
#define TTL_PULSE_H_

#include <common.h>
#include "fpga.h"
#include "verbosity.h"

#ifdef WIN32
#define scalb _scalb
#define snprintf _snprintf
#endif

extern bool bDebugPulses;
extern unsigned g_tSequence; //accumulated sequence duration in PULSER units

#define TIME_UNIT    (1e-8)

#define TTL_NOTHING     (0)

#ifdef CONFIG_AL
#include "custom/ttl_Al.h"
#endif

#ifdef CONFIG_HG
#include "custom/ttl_Hg.h"
#endif

#ifdef CONFIG_SHB
#include "custom/ttl_SHB.h"
#endif

#ifdef CONFIG_BB

#define TTL_START_EXP   (1 << 12)
static inline const char*
TTL_name(unsigned ttl)
{
    switch (ttl) {
    default:
        return "     wait";
    }
}

#endif

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
        g_tSequence += t;
    }
}

#endif /*TTL_PULSE_H_*/
