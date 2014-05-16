/*****************************************************************************
* Filename:          pulse_controller_v4_00_b/src/pulse_controller.h
* Version:           4.00.b
* Description:       pulse_controller Driver Header File
* Date:              Tue Jul 10 23:45:58 2012 (by Create and Import Peripheral Wizard)
*****************************************************************************/

#ifndef PULSE_CONTROLLER_H
#define PULSE_CONTROLLER_H

#include "pulse_controller_io.h"

//time resolution of pulse controller in ns, us, and 1/us
#define DT_ns     (10)
#define DT_us     (0.01)
#define DT_per_us (100)

//minimum pulse durations in units of the time resolution
#define T_TTL_MIN (5)
#define T_DDS_MIN (50)

extern unsigned PULSE_CONTROLLER_vacancy;

#define PULSE_CONTROLLER_COUNTING_PULSE_FLAG    (0x04000000)
#define PULSE_CONTROLLER_INVERT_SYNC            (0x02000000)

//synchronize these parameters with N_BITS/N_BINS in the verilog files
//I haven't figured out how to reuse them automatically in the Xilinx toolchain
#define PULSE_CONTROLLER_N_PMT_BINS				      (16)
#define PULSE_CONTROLLER_N_PMT_BITS				      (8)

//! Unsafe because FIFO PULSE_CONTROLLER_vacancy is not checked before write.
//! Only call if you *know* there's space on the write FIFO.
inline void PULSE_CONTROLLER_unsafe_pulse(void* base_addr, const unsigned control, const unsigned operand);

//! Set function to be called while waiting for FPGA
void PULSE_CONTROLLER_set_idle_function(void (*new_idle_func)(void));

void PULSE_CONTROLLER_init(void* base_addr, unsigned nDDS, unsigned bResetDDS, int debug_level );
void PULSE_CONTROLLER_self_test(void* base_addr, int nIO);
int PULSE_CONTROLLER_test_slave_registers(void* base_addr);
int PULSE_CONTROLLER_test_dds(void* base_addr, char nDDS);

//re-initialize all DDS, keeping current frequencies
void PULSE_CONTROLLER_reinit_DDS(void* base_addr, unsigned nDDS);

int PULSE_CONTROLLER_check_dds(void* base_addr, char i);
int PULSE_CONTROLLER_check_all_dds(void* base_addr);

void PULSE_CONTROLLER_write_slave_reg(void* base_addr, char n, unsigned offset, unsigned val);
unsigned PULSE_CONTROLLER_read_slave_reg(void* base_addr, char n, unsigned offset);

//! return whether current pulse sequence is finished
unsigned PULSE_CONTROLLER_old_is_finished(void* base_addr);

//! return whether current pulse sequence is finished
unsigned PULSE_CONTROLLER_is_finished(void* base_addr);

//! wait for the current pulse sequence to finish
void PULSE_CONTROLLER_wait_for_finished(void* base_addr);

void PULSE_CONTROLLER_ensure_vacancy(void* base_addr, unsigned n);
int PULSE_CONTROLLER_read_empty(void* base_addr);
unsigned PULSE_CONTROLLER_num_results(void* base_addr);

unsigned PULSE_CONTROLLER_read_sr(void* base_addr, unsigned i);
unsigned PULSE_CONTROLLER_pulse(void* base_addr, unsigned t, const unsigned flags, const unsigned operand);
void PULSE_CONTROLLER_short_pulse(void* base_addr, const unsigned control, const unsigned operand);
unsigned PULSE_CONTROLLER_pop_result(void* base_addr);

unsigned PULSE_CONTROLLER_get_write_status(void* base_addr);
unsigned PULSE_CONTROLLER_get_read_status(void* base_addr);

//! get/set stored previous PMT counter value.  
//! The hardware counter increments unformly and is never reset
unsigned PULSE_CONTROLLER_get_last_PMT();
void PULSE_CONTROLLER_set_last_PMT(unsigned);

//! return counts accumulated during last detection
unsigned PULSE_CONTROLLER_get_PMT(void* base_addr);

//! configure PMT correlation
void PULSE_CONTROLLER_config_PMT_correlation(void* base_addr, unsigned removeFromQueue, unsigned clkDiv);

//! reset PMT correlation data
void PULSE_CONTROLLER_reset_PMT_correlation(void* base_addr);

//! read PMT correlation data.  add data to unsigned histogram[N_BINS]
void PULSE_CONTROLLER_get_PMT_correlation(void* base_addr, unsigned* histogram);

// enable / disable clock_out
void PULSE_CONTROLLER_enable_clock_out(void* base_addr, unsigned divider);

//TTL functions
void PULSE_CONTROLLER_set_ttl(void* base_addr, unsigned high_mask, unsigned low_mask);
void PULSE_CONTROLLER_get_ttl(void* base_addr, unsigned* high_mask, unsigned* low_mask);

//DDS functions
void PULSE_CONTROLLER_dds_reset(void* base_addr, char i);
void PULSE_CONTROLLER_set_dds_div2(void* base_addr, char i, int b);

void PULSE_CONTROLLER_set_dds_freq(void* base_addr, char i, unsigned freq);
void PULSE_CONTROLLER_set_dds_phase(void* base_addr, char i, unsigned short phase);
unsigned int PULSE_CONTROLLER_get_dds_freq(void* base_addr, char i);
unsigned int PULSE_CONTROLLER_get_dds_phase(void* base_addr, char i);

unsigned PULSE_CONTROLLER_get_dds_byte(void* base_addr, char i, unsigned address);
unsigned PULSE_CONTROLLER_get_dds_two_bytes(void* base_addr, char i, unsigned address);

/*
Timing-check functions to help figure out if experiment timing is being met.
Timing failure will occur, if the pulse buffer underflows.

While timing_check is enabled, all pulses that are sent to the PULSE_CONTROLLER
are considered time-critical.  If a pulse finishes, and there is not another pulse
waiting in the buffer, a timing error is stored in one of the status registers.
This can be detected by calling PULSE_CONTROLLER_timing_ok (returns false if
a timing error occured).  The error status can be cleared by calling PULSE_CONTROLLER_clear_timing_check.
*/

void PULSE_CONTROLLER_enable_timing_check(void* base_addr);
void PULSE_CONTROLLER_disable_timing_check(void* base_addr);

void PULSE_CONTROLLER_clear_timing_check(void* base_addr);
int  PULSE_CONTROLLER_timing_ok(void* base_addr);

//! toggle init.  reset prior to new sequence.  needed for pulse hold to work
//and maybe other feastures
void PULSE_CONTROLLER_toggle_init(void* base_addr);

//! set hold.  pulses are stopped.  automatically release if FIFO is full
//this avoids deadlock if FIFO is full, but hold is set
//filling the buffer before releasing avoids timing errors due to underflow
void PULSE_CONTROLLER_set_hold(void* base_addr);

//! release hold.  pulses can run
void PULSE_CONTROLLER_release_hold(void* base_addr);

#endif /** PULSE_CONTROLLER_H */
