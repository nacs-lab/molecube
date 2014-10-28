/* dds_pulse.h
 * Define small interface functions to DDS hardware.
 * set/get phase, amplitude, frequency
 * Physical or device units can be used.
 * Most functions are inline for speed and programming confenience.
 * Include some customization info for DDS names, which are printed
 * in the log file.
 *
 * Also define DDS pulse functions, which turn the DDS on & off to
 * generate a pulse.
 */

#ifndef DDS_PULSE_H_
#define DDS_PULSE_H_

#include "common.h"
#include "ttl_pulse.h"
#include "fpga.h"
#include <float.h>
#include "verbosity.h"

#ifdef WIN32
#define _USE_MATH_DEFINES 1
#define scalb _scalb
#define snprintf _snprintf
#endif

#include <math.h>
#include <algorithm>

//#include "common.h"
#include "AD9914.h"

#ifdef CONFIG_BB
static inline const char*
DDS_name(unsigned iDDS)
{
    switch (iDDS) {
    default:
        return "";
    }
}
#endif

#ifdef CONFIG_AL
#include "custom/ttl_Al.h"
#endif

#ifdef CONFIG_HG
#include "custom/ttl_Hg.h"
#endif

#ifdef CONFIG_SHB
#include "custom/ttl_SHB.h"
#endif

#define DDS_NONE     (100)

#define PHASE_0      (0)
#define PHASE_90     (1 << 14)
#define PHASE_180    (PHASE_90 << 1)
#define PHASE_270    (PHASE_90 + PHASE_180)
#define PHASE_360    (PHASE_90 << 2)

unsigned FTW2Hz(unsigned ftw, double fClock);
double FTW2HzD(unsigned ftw, double fClock);
int FTW2HzI(int ftw);
unsigned int Hz2FTW(double f, double fClock);
int Hz2FTWI(double f);
unsigned int MHz2FTW(double f, double fClock);
int MHz2FTWI(double f);

void print_pulse_info(unsigned iDDS, unsigned ftwOn, unsigned ftwOff,
                      unsigned t, unsigned ttl, const char* info = 0);

void print_pulse_info(unsigned iDDS, unsigned ftwOn, unsigned ftwOff,
                      unsigned aOn, unsigned aOff, unsigned t, unsigned ttl,
                      const char* info = 0);

//Get clock freq. for iDDS.
//Ueful when the DDS do not all have the same clock.
double dds_clk(int iDDS);

static inline void
DDS_off(unsigned iDDS, verbosity* v=0)
{
    PULSER_set_dds_freq(pulser, iDDS, 0);

    if (v) {
        fprintf(gLog, "%7s DDS(%2u) off %30s t = %8.2f us\n",
                DDS_name(iDDS), iDDS, "", (double)0.5);
    }

    g_tSequence += PULSER_DDS_SET_FTW_DURATION;
}

//set FTW=frequency tuning word
static inline void
DDS_set_ftw(unsigned iDDS, unsigned ftw, verbosity* v=0)
{
    PULSER_set_dds_freq(pulser, iDDS, ftw);

    if (v){
        v->printf("%7s DDS(%2u) f   = %13.3f Hz    ",
                  DDS_name(iDDS), iDDS, FTW2HzD(ftw, dds_clk(iDDS)), ftw);
        print_timing_info(v, g_tSequence, PULSER_DDS_SET_FTW_DURATION);
    }

    g_tSequence += PULSER_DDS_SET_FTW_DURATION;
}

static inline void
DDS_set_freqHz(unsigned iDDS, unsigned Hz, verbosity* v=0)
{
    DDS_set_ftw(iDDS, Hz2FTW(Hz, dds_clk(iDDS)), v);
}

static inline unsigned
DDS_get_ftw(unsigned iDDS)
{
    return PULSER_get_dds_freq(pulser, iDDS);
}

static inline double
DDS_get_freqHz(unsigned iDDS) //get freq in Hz
{
    return FTW2HzD(DDS_get_ftw(iDDS), AD9914_CLK);
}

//set PTW=phase tuning word
static inline void
DDS_set_ptw(unsigned iDDS, unsigned ptw, verbosity* v=0)
{
    PULSER_set_dds_phase(pulser, iDDS, ptw);

    if (v){
        v->printf("%7s DDS(%2u) p   = %9.3f deg. %4s ",
                  DDS_name(iDDS), iDDS, (ptw * 360.0 / PHASE_360), "");
        print_timing_info(v, g_tSequence, PULSER_DDS_SET_PTW_DURATION);
    }
    g_tSequence += PULSER_DDS_SET_PTW_DURATION;
}

static inline void
DDS_shift_ptw(unsigned iDDS, unsigned ptw, verbosity* v=0)
{
    PULSER_shift_dds_phase(pulser, iDDS, ptw);

    if (v){
        v->printf("%7s DDS(%2u) p  += %9.3f deg. %4s ",
                  DDS_name(iDDS), iDDS, (ptw * 360.0 / PHASE_360), "");
        print_timing_info(v, g_tSequence, PULSER_DDS_SET_PTW_DURATION);
    }
    g_tSequence += PULSER_DDS_SET_PTW_DURATION;
}

static inline void
DDS_set_phase_deg(unsigned iDDS, double phase, verbosity* v=0)
{
    DDS_set_ptw(iDDS, (int)(PHASE_360 * phase / 360.0 + 0.5), v);
}

static inline unsigned
DDS_get_ptw(unsigned iDDS)
{
    return PULSER_get_dds_two_bytes(pulser, iDDS, 0x30);
}

static inline double
DDS_get_phase_deg(unsigned iDDS)
{
    unsigned u0 = DDS_get_ptw(iDDS);
    return u0 * 360.0 / 65536.0;
}

//set ATW=amplitude tuning word
static inline void
DDS_set_atw(unsigned iDDS, unsigned atw, verbosity* v=0)
{
    PULSER_set_dds_amp(pulser, iDDS, atw);

    if (v){
        v->printf("%7s DDS(%2u) A   = %9.6f / 1  %4s ",
                  DDS_name(iDDS), iDDS, atw / 4095.0, "");

        print_timing_info(v, g_tSequence, PULSER_DDS_SET_ATW_DURATION);
    }

    g_tSequence += PULSER_DDS_SET_ATW_DURATION;
}

static inline void
DDS_set_amp(unsigned iDDS, double A, verbosity *v=0)
{
    DDS_set_atw(iDDS, (unsigned)(A * 4095.0 + 0.5), v);
}

static inline unsigned
DDS_get_atw(unsigned iDDS)
{
    return PULSER_get_dds_two_bytes(pulser, iDDS, 0x32);
}

static inline double
DDS_get_amp(unsigned iDDS)
{
    unsigned u0 = DDS_get_atw(iDDS);
    return u0 / 4095.0;
}


//make an RF pulse of specified frequency and duration
static inline void
DDS_pulse(unsigned iDDS, unsigned ftwOn, unsigned ftwOff,
          unsigned t, unsigned ttl = 0)
{
    if (t > 4) {
        DDS_set_ftw(iDDS, ftwOn);
        PULSER_short_pulse(pulser, t, ttl);
        DDS_set_ftw(iDDS, ftwOff);

        if (bDebugPulses)
            print_pulse_info(iDDS, ftwOn, ftwOff, t, ttl);

        g_tSequence += t;
    }
}

static inline void
DDS_pulse(unsigned iDDS, unsigned ftwOn, unsigned ftwOff, unsigned aOn,
          unsigned aOff, unsigned t, unsigned ttl = 0)
{
    if (t > 4) {
        PULSER_set_dds_freq(pulser, iDDS, ftwOn);
        PULSER_set_dds_amp(pulser, iDDS, aOn);
        PULSER_short_pulse(pulser, t, ttl);
        PULSER_set_dds_amp(pulser, iDDS, aOff);
        PULSER_set_dds_freq(pulser, iDDS, ftwOff);

        if (bDebugPulses)
            print_pulse_info(iDDS, ftwOn, ftwOff, aOn, aOff, t, ttl);

        g_tSequence += t;
    }
}

static inline void
DDS_long_pulse(unsigned iDDS, unsigned ftwOn, unsigned ftwOff, unsigned t,
               unsigned flags, unsigned ttl)
{
    if (t > 4) {
        PULSER_set_dds_freq(pulser, iDDS, ftwOn);
        PULSER_pulse(pulser, t, flags, ttl);
        PULSER_pulse(pulser, 10, 0, 0);
        PULSER_set_dds_freq(pulser, iDDS, ftwOff);

        if (bDebugPulses)
            print_pulse_info(iDDS, ftwOn, ftwOff, t, ttl, "LONG");

        g_tSequence += t;
    }
}

static inline void
DDS_long_pulse(unsigned iDDS, unsigned ftwOn, unsigned ftwOff, unsigned aOn,
               unsigned aOff, unsigned t, unsigned flags, unsigned ttl)
{
    if (t > 4) {
        PULSER_set_dds_freq(pulser, iDDS, ftwOn);
        PULSER_set_dds_amp(pulser, iDDS, aOn);
        PULSER_pulse(pulser, t, flags, ttl);
        PULSER_pulse(pulser, 10, 0, 0);
        PULSER_set_dds_amp(pulser, iDDS, aOff);
        PULSER_set_dds_freq(pulser, iDDS, ftwOff);

        if (bDebugPulses)
            print_pulse_info(iDDS, ftwOn, ftwOff, t, ttl, "LONG");

        g_tSequence += t;
    }
}

#ifdef CONFIG_AL
#include "Al_pulses.h"
#endif // CONFIG_AL

#endif /*DDS_PULSE_H_*/
