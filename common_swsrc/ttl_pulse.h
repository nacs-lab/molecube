#ifndef TTL_PULSE_H_
#define TTL_PULSE_H_

#include <stdio.h>
#include "fpga.h"
#include "verbosity.h"

#ifdef WIN32
#define scalb _scalb
#define snprintf _snprintf
#endif

extern bool bDebugPulses;
extern FILE* gLog;
extern unsigned g_tSequence; //accumulated sequence duration in PULSER units

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

void print_timing_info(verbosity* v, unsigned t0, unsigned dt, char last='\n');
void print_pulse_info(verbosity* v, unsigned t, unsigned ttl, const char* info = 0);

//make an RF pulse of specified frequency and duration
inline void TTL_pulse(unsigned t, unsigned ttl = 0, verbosity* v=0)
{
    if (t > 4) {
        PULSER_pulse(pulser, t, 0, ttl);

        if (v)
            print_pulse_info(v, t, ttl);

        g_tSequence += t;
    }
}

#endif /*TTL_PULSE_H_*/

