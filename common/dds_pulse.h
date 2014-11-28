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

#include <pulse_controller.h>
#include <nacs-pulser/pulser.h>

#define PHASE_90 (1 << 14)
#define PHASE_360 (PHASE_90 * 4)

static NACS_INLINE void
DDS_set_freqHz(volatile void *pulse_addr, unsigned iDDS, double Hz)
{
    auto fnum = NaCs::Pulser::DDSConverter::freq2num(Hz, PULSER_AD9914_CLK);
    PULSER_set_dds_freq(pulse_addr, iDDS, fnum);
}

static NACS_INLINE double
DDS_get_freqHz(volatile void *pulse_addr, unsigned iDDS) //get freq in Hz
{
    auto fnum = PULSER_get_dds_freq(pulse_addr, iDDS);
    return NaCs::Pulser::DDSConverter::num2freq(fnum, PULSER_AD9914_CLK);
}

static NACS_INLINE void
DDS_set_phase_deg(volatile void *pulse_addr, unsigned iDDS, double phase)
{
    PULSER_set_dds_phase(pulse_addr, iDDS,
                         (int)(PHASE_360 * phase / 360.0 + 0.5));
}

static NACS_INLINE double
DDS_get_phase_deg(volatile void *pulse_addr, unsigned iDDS)
{
    unsigned u0 = PULSER_get_dds_phase(pulse_addr, iDDS);
    return u0 * 360.0 / 65536.0;
}

static NACS_INLINE void
DDS_set_amp(volatile void *pulse_addr, unsigned iDDS, double A)
{
    PULSER_set_dds_amp(pulse_addr, iDDS, (unsigned)(A * 4095.0 + 0.5));
}

static NACS_INLINE double
DDS_get_amp(volatile void *pulse_addr, unsigned iDDS)
{
    unsigned u0 = PULSER_get_dds_amp(pulse_addr, iDDS);
    return u0 / 4095.0;
}

#endif
