/*****************************************************************************
* Filename:          PULSER_v4_00_b/src/pulse_controller.h
* Version:           4.00.b
* Description:       pulse_controller Driver Header File
* Date:              Tue Jul 10 23:45:58 2012 (by Create and Import Peripheral Wizard)
*****************************************************************************/

/* The name PULSER is due to Chris Langer, who developed the original FPGA
 * pulse generator while doing his PhD research at NIST */

#include <nacs-utils/utils.h>
#include <nacs-pulser/pulser-config.h>

#ifndef PULSER_H
#define PULSER_H

NACS_BEGIN_DECLS

void PULSER_init(volatile void *base_addr, unsigned nDDS, unsigned bResetDDS);
// void PULSER_self_test(volatile void *base_addr, int nIO);

//! If DDS i is present return non-zero, otherwise 0.
int PULSER_dds_exists(volatile void *base_addr, int i);

// int PULSER_check_dds(volatile void *base_addr, int i);
// int PULSER_check_all_dds(volatile void *base_addr);

void PULSER_write_sr(volatile void *base_addr, int n, unsigned val);
unsigned PULSER_read_sr(volatile void *base_addr, int n);

//! wait for the current pulse sequence to finish
void PULSER_wait_for_finished(volatile void *base_addr);

// void PULSER_ensure_vacancy(volatile void *base_addr, unsigned n);
int PULSER_read_empty(volatile void *base_addr);
unsigned PULSER_num_results(volatile void *base_addr);

void PULSER_pulse(volatile void *base_addr, unsigned t, unsigned flags,
                  unsigned operand);
void PULSER_short_pulse(volatile void *base_addr, unsigned control,
                        unsigned operand);
unsigned PULSER_pop_result(volatile void *base_addr);

unsigned PULSER_get_write_status(volatile void *base_addr);
unsigned PULSER_get_read_status(volatile void *base_addr);

// enable / disable clock_out
void PULSER_enable_clock_out(volatile void *base_addr, unsigned divider);

//TTL functions
void PULSER_set_ttl(volatile void *base_addr, unsigned high_mask,
                    unsigned low_mask);
void PULSER_get_ttl(volatile void *base_addr, unsigned *high_mask,
                    unsigned *low_mask);

//DDS functions
void PULSER_dds_reset(volatile void *base_addr, int i);
void PULSER_dds_reset_sel(volatile void *base_addr, unsigned mask);
void PULSER_dds_set_sel(volatile void *base_addr, unsigned mask);

void PULSER_set_dds_freq(volatile void *base_addr, int i, unsigned freq);
void PULSER_set_dds_phase(volatile void *base_addr, int i,
                          unsigned short phase);
void PULSER_set_dds_amp(volatile void *base_addr, int i, unsigned short amp);

//shift phase.  Works only if DDS phase has been set via PULSER_set_dds_phase.
//Setting phase via PULSER_set_dds_two_bytes will break this.
void PULSER_shift_dds_phase(volatile void *base_addr, int i,
                            unsigned short phase);

unsigned int PULSER_get_dds_freq(volatile void *base_addr, int i);
unsigned int PULSER_get_dds_phase(volatile void *base_addr, int i);
unsigned PULSER_get_dds_amp(volatile void *base_addr, int i);

//! DDS functions - set bytes addr+1 ... addr
void PULSER_set_dds_two_bytes(volatile void *base_addr, int i,
                              unsigned addr, unsigned data);

//! DDS functions - set bytes addr+3 ... addr
void PULSER_set_dds_four_bytes(volatile void *base_addr, int i,
                               unsigned addr, unsigned data);

unsigned PULSER_get_dds_byte(volatile void *base_addr, int i,
                             unsigned address);
unsigned PULSER_get_dds_two_bytes(volatile void *base_addr, int i,
                                  unsigned address);

//! DDS functions - get four bytes from address+3 ... adress on DDS i
unsigned PULSER_get_dds_four_bytes(volatile void *base_addr, int i,
                                   unsigned address);

/*
Timing-check functions to help figure out if experiment timing is being met.
Timing failure will occur if the pulse buffer underflows.

While timing_check flag is enabled, all pulses that are sent to the PULSER
are considered time-critical.  If a pulse finishes, and there is not another pulse
waiting in the buffer, a timing error is stored in one of the status registers.
This can be detected by calling PULSER_timing_ok (returns false if
a timing error occured).  The error status can be cleared by calling PULSER_clear_timing_check.
*/

void PULSER_enable_timing_check();
void PULSER_disable_timing_check();

void PULSER_clear_timing_check(volatile void *base_addr);
int  PULSER_timing_ok(volatile void *base_addr);

//! toggle init.  reset prior to new sequence.  needed for pulse hold to work
//and maybe other feastures
void PULSER_toggle_init(volatile void *base_addr);

//! set hold.  pulses are stopped.  automatically release if FIFO is full
//this avoids deadlock if FIFO is full, but hold is set
//filling the buffer before releasing avoids timing errors due to underflow
void PULSER_set_hold(volatile void *base_addr);

//! release hold.  pulses can run
void PULSER_release_hold(volatile void *base_addr);

NACS_END_DECLS

#endif
