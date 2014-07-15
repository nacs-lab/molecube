#ifndef TTL_PULSE_H_
#define TTL_PULSE_H_

#include <stdio.h>
#include "fpga.h"

#ifdef WIN32
#define scalb _scalb
#define snprintf _snprintf
#endif

extern bool bDebugPulses;
extern FILE* gLog;

#define TIME_UNIT    (1e-8)

#define TTL_NOTHING     (0)

#ifdef CONFIG_AL
  #include "ttl_Al.h"
#endif

#ifdef CONFIG_HG
  #include "ttl_Hg.h"
#endif

#ifdef CONFIG_SHB
  #include "ttl_SHB.h"
#endif

#ifdef CONFIG_BB

#define TTL_START_EXP   (1 << 12)
inline const char* TTL_name(unsigned ttl)
{
    switch (ttl) {
    default:
        return "     wait";
    }
}

#endif

inline unsigned int us2TW(double t)
{
    return static_cast<unsigned int>(t * 100);
}

inline unsigned int ms2TW(double t)
{
    return us2TW(t * 1e3);
}

inline void print_pulse_info(unsigned t, unsigned ttl, const char* info = 0)
{

    if (info)
        fprintf(gLog, "%12s %36s t = %8.2f us TTL=%08X (%s)\n", TTL_name(ttl), "",
                0.01 * (double) t, ttl, info);
    else
        fprintf(gLog, "%12s %36s t = %8.2f us TTL=%08X\n", TTL_name(ttl), "",
                0.01 * (double) t, ttl);
}

//make an RF pulse of specified frequency and duration
inline void TTL_pulse(unsigned t, unsigned ttl = 0)
{
    if (t > 4) {
        PULSER_pulse(pulser, t, 0, ttl);

        if (bDebugPulses)
            print_pulse_info(t, ttl);
    }
}

#endif /*TTL_PULSE_H_*/

