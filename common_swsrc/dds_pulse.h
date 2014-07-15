#ifndef DDS_PULSE_H_
#define DDS_PULSE_H_

#include "common.h"
#include "ttl_pulse.h"
#include "fpga.h"
#include <float.h>

#ifdef WIN32
#define _USE_MATH_DEFINES 1
#endif

#include <math.h>
#include <algorithm>

//#include "common.h"
#include "AD9914.h"

#ifdef WIN32
#define scalb _scalb
#define snprintf _snprintf
#endif

#ifdef CONFIG_BB
inline const char* DDS_name(unsigned iDDS)
{
    switch (iDDS) {
    default:
        return "";
    }
}
#endif

#ifdef CONFIG_AL
#define DDS_PRECOOL     (0)
#define DDS_3P1x2    	(1)
#define DDS_3P0     	(2)
#define DDS_DETECT      (3)
#define DDS_3P0_BR      (4)
#define DDS_HEAT     	(5)
#define DDS_HEAT_AM    	(5)
#define DDS_RAMAN    	(6)
#define DDS_RF          (7)

inline const char* DDS_name(unsigned iDDS)
{
    switch (iDDS) {
    case DDS_PRECOOL:
        return "  Precool";
    case DDS_3P1x2:
        return "  3P1 x 2";
    case DDS_DETECT:
        return "   Detect";
    case DDS_RF:
        return "    HF RF";
    case DDS_RAMAN:
        return "    Raman";
    case DDS_3P0:
        return "  3P0    ";
    case DDS_3P0_BR:
        return "  3P0(BR)";
    default:
        return "  unknown";

    }
}

#endif

#ifdef CONFIG_HG

#define DDS_DOPPLER_B1  	(5)
#define DDS_DOPPLER_B2 		(6)
#define DDS_DOPPLER_B3		(7)

inline const char* DDS_name(unsigned iDDS)
{
    switch (iDDS) {
    case DDS_DOPPLER_B1:
        return "  Doppler B1";
    case DDS_DOPPLER_B2:
        return "  Doppler B2";
    case DDS_DOPPLER_B3:
        return "  Doppler B3";
    default:
        return "  unknown";
    }
}

#endif


#ifdef CONFIG_SHB
#define DDS_BURNER      (0)
#define DDS_CAVITY      (1)
#define DDS_RF       	(2)
#define DDS_BURNER2     (3)
#define DDS_3P0_BR      (4)
#define DDS_HEAT     	(5)
#define DDS_RAMAN    	(6)
#define DDS_3P0         (7)

inline const char* DDS_name(unsigned iDDS)
{
    switch (iDDS) {
    case  DDS_BURNER:
        return "  Burner";
    case  DDS_CAVITY:
        return "  Cavity";
    case DDS_BURNER2:
        return " Burner2";
    default:
        return "  unknown";

    }
}

#endif

#ifdef CONFIG_LMS
inline const char* DDS_name(unsigned iDDS)
{
    switch (iDDS) {
    case 0:
        return "   DDS0";
    case 1:
        return "   DDS1";
    default:
        return "unknown";
    }
}
#endif


#define DDS_NONE     	(100)

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

double dds_clk(int iDDS);

inline void DDS_off(unsigned iDDS)
{
    PULSER_set_dds_freq(pulser, iDDS, 0);


    if (bDebugPulses) {
        fprintf(gLog, "%7s DDS(%2u) off %30s t = %8.2f us\n",
                DDS_name(iDDS), iDDS, "", (double)0.5);
    }
}

inline void DDS_set_ftw(unsigned iDDS, unsigned ftw, FILE* fLog)
{
    PULSER_set_dds_freq(pulser, iDDS, ftw);

    if (fLog) {
        fprintf(fLog, "%7s DDS(%2u) f   = %9u Hz (ftw=%08X) t = %8.2f us\n",
                DDS_name(iDDS), iDDS, FTW2Hz(ftw, AD9914_CLK), ftw, (double)0.3);
    }
}

inline void DDS_set_ftw(unsigned iDDS, unsigned ftw)
{
    DDS_set_ftw(iDDS, ftw, bDebugPulses ? gLog : 0);
}

inline void DDS_set_freqHz(unsigned iDDS, unsigned Hz)
{
    DDS_set_ftw(iDDS, Hz2FTW(Hz, AD9914_CLK));
}

inline void DDS_set_freqHz(unsigned iDDS, unsigned Hz, FILE* fLog)
{
    DDS_set_ftw(iDDS, Hz2FTW(Hz, AD9914_CLK), fLog);
}

inline unsigned DDS_get_ftw(unsigned iDDS)
{
    unsigned u0 = PULSER_get_dds_two_bytes(pulser, iDDS, 0x2C);
    unsigned u1 = PULSER_get_dds_two_bytes(pulser, iDDS, 0x2E);

    //unsigned u0 = PULSER_get_dds_two_bytes(base_addr, i, 0x10);
    //unsigned u1 = PULSER_get_dds_two_bytes(base_addr, i, 0x12);

    unsigned ftw = u0 | (u1 << 16);
    return ftw;
}

inline double DDS_get_freqHz(unsigned iDDS) //get freq in Hz
{
    return FTW2HzD(DDS_get_ftw(iDDS), AD9914_CLK);
}

//set PTW=phase tuning word
inline void DDS_set_ptw(unsigned iDDS, unsigned ptw, FILE* fLog)
{
    PULSER_set_dds_two_bytes(pulser, iDDS, 0x30, ptw & 0xFFFF);

    if (fLog) {
        fprintf(fLog, "%7s DDS(%2u) p   = %9.3f deg. %12s t = %8.2f us\n",
                DDS_name(iDDS), iDDS, (ptw * 360.0 / PHASE_360), "", (double)0.3);
    }
}

inline void DDS_set_ptw(unsigned iDDS, unsigned ptw)
{
    DDS_set_ptw(iDDS, ptw, bDebugPulses ? gLog : 0);
}

inline void DDS_set_phase_deg(unsigned iDDS, double phase, FILE* fLog)
{
  DDS_set_ptw(iDDS, (int)(PHASE_360*phase/360.0 + 0.5), fLog);
}


inline unsigned DDS_get_ptw(unsigned iDDS)
{
  return PULSER_get_dds_two_bytes(pulser, iDDS, 0x30);
}

inline double DDS_get_phase_deg(unsigned iDDS)
{
  unsigned u0 = DDS_get_ptw(iDDS);
  return u0*360.0/65536.0;
}

//set ATW=amplitude tuning word
inline void DDS_set_atw(unsigned iDDS, unsigned atw, FILE* fLog)
{
    PULSER_set_dds_two_bytes(pulser, iDDS, 0x32, atw & 0x0FFF);

    if (fLog) {
        fprintf(fLog, "%7s DDS(%2u) A   = %9.6f / 1  %12s t = %8.2f us\n",
                DDS_name(iDDS), iDDS, atw / 4095.0, "", (double)0.3);
    }
}

inline void DDS_set_atw(unsigned iDDS, unsigned atw)
{
    DDS_set_atw(iDDS, atw, bDebugPulses ? gLog : 0);
}

inline void DDS_set_amp(unsigned iDDS, double A, FILE* fLog)
{
  DDS_set_atw(iDDS, (unsigned)(A*4095.0 + 0.5), fLog);
}

inline unsigned DDS_get_atw(unsigned iDDS)
{
  return PULSER_get_dds_two_bytes(pulser, iDDS, 0x32);
}

inline double DDS_get_amp(unsigned iDDS)
{
  unsigned u0 = DDS_get_atw(iDDS);
  return u0/4095.0;
}


//make an RF pulse of specified frequency and duration
inline void DDS_pulse(unsigned iDDS, unsigned ftwOn, unsigned ftwOff,
                      unsigned t, unsigned ttl = 0)
{
    if (t > 4) {
        DDS_set_ftw(iDDS, ftwOn);
        PULSER_short_pulse(pulser, t, ttl);
        DDS_set_ftw(iDDS, ftwOff);

        if (bDebugPulses)
            print_pulse_info(iDDS, ftwOn, ftwOff, t, ttl);
    }
}

inline void DDS_pulse(unsigned iDDS, unsigned ftwOn, unsigned ftwOff,
                      unsigned aOn, unsigned aOff, unsigned t, unsigned ttl = 0)
{
    if (t > 4) {
        PULSER_set_dds_freq(pulser, iDDS, ftwOn);
        PULSER_set_dds_amp(pulser, iDDS, aOn);
        PULSER_short_pulse(pulser, t, ttl);
        PULSER_set_dds_amp(pulser, iDDS, aOff);
        PULSER_set_dds_freq(pulser, iDDS, ftwOff);

        if (bDebugPulses)
            print_pulse_info(iDDS, ftwOn, ftwOff, aOn, aOff, t, ttl);
    }
}

inline void DDS_long_pulse(unsigned iDDS, unsigned ftwOn, unsigned ftwOff,
                           unsigned t, unsigned flags, unsigned ttl)
{
    if (t > 4) {
        PULSER_set_dds_freq(pulser, iDDS, ftwOn);
        PULSER_pulse(pulser, t, flags, ttl);
        PULSER_pulse(pulser, 10, 0, 0);
        PULSER_set_dds_freq(pulser, iDDS, ftwOff);

        if (bDebugPulses)
            print_pulse_info(iDDS, ftwOn, ftwOff, t, ttl, "LONG");
    }
}

inline void DDS_long_pulse(unsigned iDDS, unsigned ftwOn, unsigned ftwOff,
                           unsigned aOn, unsigned aOff, unsigned t,
                           unsigned flags, unsigned ttl)
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
    }
}

#ifdef CONFIG_AL
#include "Al_pulses.h"
#endif // CONFIG_AL

#endif /*DDS_PULSE_H_*/
