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

#include "fpga.h"
#include <common.h>

#include <float.h>
#include <math.h>
#include <algorithm>

#include "AD9914.h"

static inline const char*
DDS_name(unsigned iDDS)
{
    (void)iDDS;
    return "";
}

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

//Get clock freq. for iDDS.
//Ueful when the DDS do not all have the same clock.
double dds_clk(int iDDS);

//set FTW=frequency tuning word
static inline void
DDS_set_ftw(unsigned iDDS, unsigned ftw)
{
    PULSER_set_dds_freq(pulser, iDDS, ftw);
}

static inline void
DDS_set_freqHz(unsigned iDDS, unsigned Hz)
{
    DDS_set_ftw(iDDS, Hz2FTW(Hz, dds_clk(iDDS)));
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
DDS_set_ptw(unsigned iDDS, unsigned ptw)
{
    PULSER_set_dds_phase(pulser, iDDS, ptw);
}

static inline void
DDS_shift_ptw(unsigned iDDS, unsigned ptw)
{
    PULSER_shift_dds_phase(pulser, iDDS, ptw);
}

static inline void
DDS_set_phase_deg(unsigned iDDS, double phase)
{
    DDS_set_ptw(iDDS, (int)(PHASE_360 * phase / 360.0 + 0.5));
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
DDS_set_atw(unsigned iDDS, unsigned atw)
{
    PULSER_set_dds_amp(pulser, iDDS, atw);
}

static inline void
DDS_set_amp(unsigned iDDS, double A)
{
    DDS_set_atw(iDDS, (unsigned)(A * 4095.0 + 0.5));
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

#endif /*DDS_PULSE_H_*/
