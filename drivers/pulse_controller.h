/*****************************************************************************
* Filename:          PULSER_v4_00_b/src/pulse_controller.h
* Version:           4.00.b
* Description:       pulse_controller Driver Header File
* Date:              Tue Jul 10 23:45:58 2012 (by Create and Import Peripheral Wizard)
*****************************************************************************/

/* The name PULSER is due to Chris Langer, who developed the original FPGA
 * pulse generator while doing his PhD research at NIST */

#include <nacs-utils/utils.h>

#ifndef PULSER_H
#define PULSER_H

NACS_BEGIN_DECLS

//time resolution of pulse controller in ns, us, and 1/us
#define PULSER_DT_ns     (10)
#define PULSER_DT_us     (0.01)
#define PULSER_DT_per_us (100)

#define PULSER_ENABLE_CLK_DURATION 5
#define PULSER_DDS_SET_FTW_DURATION 30
#define PULSER_DDS_SET_PTW_DURATION 30
#define PULSER_DDS_SET_ATW_DURATION 30

//maximum number of DDS available
#define PULSER_MAX_NDDS 32

//minimum pulse durations in units of the time resolution
#define PULSER_T_TTL_MIN (5)
#define PULSER_T_DDS_MIN (30)

#define PULSER_COUNTING_PULSE_FLAG    (0x04000000)
#define PULSER_INVERT_SYNC            (0x02000000)

//synchronize these parameters with N_BITS/N_BINS in the verilog files
//I haven't figured out how to reuse them automatically in the Xilinx toolchain
#define PULSER_N_PMT_BINS (16)
#define PULSER_N_PMT_BITS (8)

void PULSER_init(volatile void *base_addr, unsigned nDDS, unsigned bResetDDS);
void PULSER_self_test(volatile void *base_addr, int nIO);
int PULSER_test_slave_registers(volatile void *base_addr);
int PULSER_test_dds(volatile void *base_addr, char nDDS);

//! If DDS i is present return non-zero, otherwise 0.
int PULSER_dds_exists(volatile void *base_addr, char i);

// int PULSER_check_dds(volatile void *base_addr, char i);
// int PULSER_check_all_dds(volatile void *base_addr);

void PULSER_write_slave_reg(volatile void *base_addr, char n,
                            unsigned offset, unsigned val);
unsigned PULSER_read_slave_reg(volatile void *base_addr, char n,
                               unsigned offset);

//! return whether current pulse sequence is finished
unsigned PULSER_old_is_finished(volatile void *base_addr);

//! return whether current pulse sequence is finished
// unsigned PULSER_is_finished(volatile void *base_addr);

//! wait for the current pulse sequence to finish
void PULSER_wait_for_finished(volatile void *base_addr);

// void PULSER_ensure_vacancy(volatile void *base_addr, unsigned n);
int PULSER_read_empty(volatile void *base_addr);
unsigned PULSER_num_results(volatile void *base_addr);

unsigned PULSER_read_sr(volatile void *base_addr, unsigned i);
void PULSER_pulse(volatile void *base_addr, unsigned t, const unsigned flags,
                  const unsigned operand);
void PULSER_short_pulse(volatile void *base_addr, const unsigned control,
                        const unsigned operand);
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
void PULSER_dds_reset(volatile void *base_addr, char i);
void PULSER_dds_reset_sel(volatile void *base_addr, unsigned mask);
void PULSER_dds_set_sel(volatile void *base_addr, unsigned mask);

void PULSER_set_dds_freq(volatile void *base_addr, char i, unsigned freq);
void PULSER_set_dds_phase(volatile void *base_addr,
                          char i, unsigned short phase);
void PULSER_set_dds_amp(volatile void *base_addr, char i, unsigned short amp);

//shift phase.  Works only if DDS phase has been set via PULSER_set_dds_phase.
//Setting phase via PULSER_set_dds_two_bytes will break this.
void PULSER_shift_dds_phase(volatile void *base_addr, char i,
                            unsigned short phase);

unsigned int PULSER_get_dds_freq(volatile void *base_addr, char i);
unsigned int PULSER_get_dds_phase(volatile void *base_addr, char i);
unsigned PULSER_get_dds_amp(volatile void *base_addr, char i);

//! DDS functions - set bytes addr+1 ... addr
void PULSER_set_dds_two_bytes(volatile void *base_addr, char i,
                              unsigned addr, unsigned data);

//! DDS functions - set bytes addr+3 ... addr
void PULSER_set_dds_four_bytes(volatile void *base_addr, char i,
                               unsigned addr, unsigned data);

unsigned PULSER_get_dds_byte(volatile void *base_addr, char i, unsigned address);
unsigned PULSER_get_dds_two_bytes(volatile void *base_addr, char i,
                                  unsigned address);

//! DDS functions - get four bytes from address+3 ... adress on DDS i
unsigned PULSER_get_dds_four_bytes(volatile void *base_addr, char i,
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

void PULSER_enable_timing_check(volatile void *base_addr);
void PULSER_disable_timing_check(volatile void *base_addr);

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

#endif /** PULSER_H */
