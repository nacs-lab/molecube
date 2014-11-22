/*****************************************************************************
 * Filename:          pulse_controller.c
 * Version:           4.00.b
 * Description:       pulse_controller Driver Source File
 * Date:              10:33 PM Friday, July 13, 2012
 *****************************************************************************/


/***************************** Include Files *******************************/

#include "pulse_controller.h"
#include <unistd.h>

#include "pulse_controller_io.h"

#include <xil_io.h>

static unsigned nDDS_boards = 0;
static unsigned extra_flags = 0;

static void (*idle_func)(void);

/* static unsigned int ddsFTW[PULSER_MAX_NDDS]; */
static unsigned short ddsPhase[PULSER_MAX_NDDS];
/* static unsigned short ddsAmp[PULSER_MAX_NDDS]; */

#define ENABLE_TIMING_CHECK    (0x08000000)


//! Unsafe because FIFO PULSER_vacancy is not checked before write.
//! Only call if you *know* there's space on the write FIFO.
static inline void
PULSER_unsafe_pulse(volatile void *base_addr, const unsigned control,
                    const unsigned operand)
{
    // does not check if there is space in buffer before writing.
    // but AXI bus will wait until space is available

    //   PULSER_vacancy--;
    //   __asm__ volatile ("stw %0,0(%2); stw %1,4(%2); eieio" : : "r" (control),  "r" (operand), "b" (base_addr + PULSER_WRFIFO_DATA_OFFSET));
    PULSER_mWriteSlaveReg31(base_addr, 0, operand);
    PULSER_mWriteSlaveReg31(base_addr, 0, control);
}

static void
PULSER_debug_regs(volatile void *base_addr)
{
    printf("PULSE_CONTROLLER registers:\r\n");

    unsigned i;

    for (i = 0;i < 31;i++) {
        if (i % 4 == 0) {
            printf("[%2d...%2d]: ", i, i + 3);
        }
        printf("%08X ", PULSER_read_slave_reg(base_addr, i, 0));

        if (i % 4 == 3) {
            printf("\r\n");
        }
    }
    printf("\r\n");
}

void
PULSER_init(volatile void *base_addr, unsigned nDDS, unsigned bResetDDS, int debug_level)
{
    //soft reset
    //PULSER_mReset(base_addr);

    if (debug_level > 0)
        PULSER_debug_regs(base_addr);

    nDDS_boards = nDDS;

    if (debug_level > 0)
        printf("PULSER_init... disable timing check\r\n");
    PULSER_disable_timing_check(base_addr);

    if (debug_level > 0)
        printf("PULSER_init... reset DDS\r\n");
    for (char iDDS = 0;iDDS < nDDS_boards;iDDS++) {
        if (bResetDDS) {
            PULSER_dds_reset(base_addr, iDDS);
        }
        //TR: no div2 for AD9914 PULSER_set_dds_div2(base_addr, iDDS, 0);
    }
}

//! Make sure there is space for at least n pulses on the FIFO.
// NOT NEEDED ON AXI
void
PULSER_ensure_vacancy(volatile void *base_addr, unsigned n)
{
    (void)base_addr;
    (void)n;
    /* while (PULSER_vacancy < n) { */
    /*     PULSER_vacancy = PULSER_mWriteFIFOVacancy((Xuint32)base_addr); */
    /* } */
}

//! Is the read FIFO empty?.
int PULSER_read_empty(volatile void *base_addr)
{
    return PULSER_num_results(base_addr) == 0;
}

//! TTL functions: pulse_io = (ttl_out | high_mask) & (~low_mask);
void PULSER_set_ttl(volatile void *base_addr, unsigned high_mask, unsigned low_mask)
{
    PULSER_mWriteSlaveReg0(base_addr, 0, high_mask);
    PULSER_mWriteSlaveReg1(base_addr, 0, low_mask);
}

//! TTL functions: pulse_io = (ttl_out | high_mask) & (~low_mask);
void PULSER_get_ttl(volatile void *base_addr, unsigned* high_mask, unsigned* low_mask)
{
    *high_mask = PULSER_read_slave_reg(base_addr, 0, 0);
    *low_mask = PULSER_read_slave_reg(base_addr, 0, 4);
}


//! enable / disable clock_out
//! divider = 0..254 means emit clock with period 2 x (divider+1) in pulse controller timing units (DT_ns)
//! divider = 255 means disable
void PULSER_enable_clock_out(volatile void *base_addr, unsigned divider)
{
    PULSER_short_pulse(base_addr, 0x50000000, divider & 0xFF);
}

//! If DDS i is present return non-zero, otherwise 0.
int PULSER_dds_exists(volatile void *base_addr, char i)
{
    unsigned addr = 0x68;
    //Check whether it's possible to set phase of profile 7 to 0 and 1
    PULSER_set_dds_two_bytes(base_addr, i, addr, 0);
    unsigned u0 = PULSER_get_dds_two_bytes(base_addr, i, addr);

    PULSER_set_dds_two_bytes(base_addr, i, addr, 1);
    unsigned u1 = PULSER_get_dds_two_bytes(base_addr, i, addr);

    return (u0 == 0) && (u1 == 1);
}

//! DDS functions - reset DDS i
void
PULSER_dds_reset(volatile void *base_addr, char _i)
{
    int i = _i;
    PULSER_short_pulse(base_addr, 0x10000004 | (i << 4), 0);

    /* ddsFTW[i] = 0; */
    ddsPhase[i] = 0;
}

//! DDS functions - reset DDS selected by bitmask mask
void PULSER_dds_reset_sel(volatile void *base_addr, unsigned mask)
{
    PULSER_short_pulse(base_addr, 0x10000005, mask);
}

//! DDS functions - select DDS (high bits in mask).  Remember to unselect (mask = 0)
void PULSER_dds_set_sel(volatile void *base_addr, unsigned mask)
{
    PULSER_short_pulse(base_addr, 0x10000006, mask);
}

//! DDS functions - get byte from address on DDS i
unsigned PULSER_get_dds_byte(volatile void *base_addr, char i, unsigned address)
{
    PULSER_short_pulse(base_addr, 0x10000003 | (i << 4) | ((address) << 9), 0);
    return (PULSER_pop_result(base_addr) >> 8) & 0x000000ff;
}

//! DDS functions - get two bytes from address+1 ... adress on DDS i
unsigned PULSER_get_dds_two_bytes(volatile void *base_addr, char i, unsigned address)
{
    PULSER_short_pulse(base_addr, 0x10000003 | (i << 4) | ((address+1) << 9), 0);
    return PULSER_pop_result(base_addr) & 0x0000ffff;
}

//! DDS functions - get four bytes from address+3 ... adress on DDS i
unsigned PULSER_get_dds_four_bytes(volatile void *base_addr, char i, unsigned address)
{
    PULSER_short_pulse(base_addr, 0x1000000E | (i << 4) | ((address+1) << 9), 0);
    return PULSER_pop_result(base_addr);
}

//! toggle init.  reset prior to new sequence
void PULSER_toggle_init(volatile void *base_addr)
{
    unsigned r3 = PULSER_read_slave_reg(base_addr, 3, 0);
    PULSER_write_slave_reg(base_addr, 3, 0, r3 |   0x00000100);
    PULSER_write_slave_reg(base_addr, 3, 0, r3 & ~(0x00000100));
}

//! set hold.  pulses are stopped
void PULSER_set_hold(volatile void *base_addr)
{
    unsigned r3 = PULSER_read_slave_reg(base_addr, 3, 0);
    PULSER_write_slave_reg(base_addr, 3, 0, r3 | 0x00000080);
}

//! release hold.  pulses can run
void PULSER_release_hold(volatile void *base_addr)
{
    unsigned r3 = PULSER_read_slave_reg(base_addr, 3, 0);
    PULSER_write_slave_reg(base_addr, 3, 0, r3 & ~(0x00000080));
}

//! enable timing check for pulses
void
PULSER_enable_timing_check(volatile void *base_addr)
{
    (void)base_addr;
    extra_flags = extra_flags | ENABLE_TIMING_CHECK;
}

//! disable timing check for pulses
void PULSER_disable_timing_check(volatile void *base_addr)
{
    (void)base_addr;
    extra_flags = extra_flags & ~ENABLE_TIMING_CHECK;
}

//! clear timing check (clear failures)
void PULSER_clear_timing_check(volatile void *base_addr)
{
    PULSER_short_pulse(base_addr, 0x30000000, 0);
}

//! were there any timing failures?
int  PULSER_timing_ok(volatile void *base_addr)
{
    return !(PULSER_read_sr(base_addr, 2) & 0x1);
}

//! return whether current pulse sequence is finished
unsigned
PULSER_is_finished(volatile void *base_addr)
{
    return PULSER_read_sr(base_addr, 2) & 0x4;
}

//! wait for the current pulse sequence to finish
void
PULSER_wait_for_finished(volatile void *base_addr)
{
    PULSER_release_hold(base_addr);

    while (!PULSER_is_finished(base_addr)) {
        // I'm not sure if the sleep here is doing anything at all....
        // Too lazy to remove or replace with sth like nanosleep for now.
        sleep(0);
    }
}

//! get status register from write FIFO
//unsigned PULSER_get_write_status(volatile void *base_addr)
//{
//   return PULSER_mReadReg(base_addr, PULSER_WRFIFO_SR_OFFSET);
//}

//! get status register from read FIFO
//unsigned PULSER_get_read_status(volatile void *base_addr)
//{
//   return PULSER_mReadReg(base_addr, PULSER_RDFIFO_SR_OFFSET);
//}

void PULSER_write_slave_reg(volatile void *base_addr, char n, unsigned offset, unsigned val)
{
    PULSER_mWriteSlaveReg0(base_addr, offset + 4*n, val);
}

unsigned PULSER_read_slave_reg(volatile void *base_addr, char n, unsigned offset)
{
    volatile unsigned u = PULSER_mReadSlaveReg0(base_addr, offset +4*n);
    return u;
}

void
PULSER_self_test(volatile void *base_addr, int nIO)
{
    unsigned ftw[PULSER_MAX_NDDS];
    unsigned nBad = 0;

    PULSER_test_slave_registers(base_addr);
    printf("\r\n");

    if (nIO > 0) {
        for (char iDDS = 0;iDDS < nDDS_boards;iDDS++) {
            PULSER_test_dds(base_addr, iDDS);
        }

        printf("Testing %d random read/writes on DDS boards 0-%d ... ",
               nIO, nDDS_boards - 1);

        //initialize to 0 Hz
        for (char iDDS = 0;iDDS < nDDS_boards;iDDS++) {
            ftw[(int)iDDS] = 0;
            PULSER_set_dds_freq(base_addr, iDDS, 0);
        }

        int cycle = 0;
        while (cycle < nIO) {
            char iDDS = rand() % nDDS_boards;
            unsigned ftw_read = PULSER_get_dds_freq(base_addr, iDDS);

            if(ftw_read != ftw[(int)iDDS]) {
                printf("\r\n");
                printf("ERROR on DDS %d : wrote FTW %08X\r\n", (int)iDDS,
                       ftw[(int)iDDS]);
                printf("                   read FTW %08X\r\n", ftw_read);
                nBad++;
            }

            ftw[(int)iDDS] = rand();
            PULSER_set_dds_freq(base_addr, iDDS, ftw[(int)iDDS]);

            cycle++;
        }

        for (char iDDS = 0;iDDS < nDDS_boards;iDDS++) {
            PULSER_set_dds_freq(base_addr, iDDS, 0);
        }

        if (nBad == 0) {
            printf("OK\r\n");
        } else {
            printf("FAILURE: %d errors\r\n", nBad);
        }
    }
}

int PULSER_test_slave_registers(volatile void *base_addr)
{
    int sr_ok = 1;
    unsigned test_val, read;
    int i, j, k;

    for(i=0; i<2; i++) {
        test_val = 0;

        for(j=0; j<8; j++)
            test_val = test_val + ((i*0xF) << (j*4));

        printf("Testing %08X   ", test_val);
        for(k=0; k<8; k++) {
            PULSER_write_slave_reg(base_addr, k, 0, test_val);

            read = PULSER_read_slave_reg(base_addr, k, 0);

            printf("SR%d = %08X   ", k, read);

            sr_ok = sr_ok && (read == test_val);
        }

        if(sr_ok)
            printf("OK\r\n");
        else
            printf("FAILED\r\n");
    }

    return sr_ok;
}

int
PULSER_test_dds(volatile void *base_addr, char nDDS)
{
    int ftw_ok = 1;
    int phase_ok = 1;
    unsigned phase = 0;
    unsigned read, test_val;

    printf("Testing DDS %d ... ", (int)nDDS);
    for (int i = 0;i < 2;i++) {
        test_val = 0;

        for (int j = 0;j < 8;j++)
            test_val = test_val + ((i * 0xF) << (j * 4));

        PULSER_set_dds_freq(base_addr, nDDS, test_val);
        read = PULSER_get_dds_freq(base_addr, nDDS);
        ftw_ok = ftw_ok && (read == test_val);

        if (read != test_val) {
            printf("ERROR !\r\n");
            printf("wrote FTW %08X\r\n", test_val);
            printf(" read FTW %08X\r\n", read);
        }
    }

    while (phase < 0x4000) {
        PULSER_set_dds_phase(base_addr, nDDS, phase);
        read = PULSER_get_dds_phase(base_addr, nDDS);

        phase_ok = phase_ok && (read == phase);

        if (read != phase) {
            printf("ERROR on DDS %d !\r\n", (int)nDDS);
            printf("wrote PHASE %04X\r\n", test_val);
            printf(" read PHASE %04X\r\n", read);
        }

        phase++;
    }

    if (ftw_ok && phase_ok) {
        printf("DDS %d OK\r\n", (int)nDDS);
    } else {
        printf("DDS %d FAILED\r\n", (int)nDDS);
    }

    return ftw_ok && phase_ok;
}

unsigned PULSER_read_sr(volatile void *base_addr, unsigned i)
{
    return PULSER_mReadSlaveReg0(base_addr, 4*i);
}

//make timed pulses
//if t > t_max, subdivide into shorter pulses
//returns number of pulses made
void
PULSER_pulse(volatile void *base_addr, unsigned t, const unsigned flags,
             const unsigned operand)
{
    static const unsigned t_max = 0x001FFFFF;
    static const unsigned t_big = 0x001FFFF0;
    while (t > t_max) {
        PULSER_short_pulse(base_addr, t_big | flags, operand);
        t -= t_big;
    }
    PULSER_short_pulse(base_addr, t | flags, operand);
}

//make short timed pulses
//FPGA can only handle pulse lengths up to t_max = 0x001FFFFF (about 40 ms)
void
PULSER_short_pulse(volatile void *base_addr, const unsigned control,
                   const unsigned operand)
{
    PULSER_unsafe_pulse(base_addr, control | extra_flags, operand);
}

void PULSER_set_idle_function(void (*new_idle_func)(void))
{
    idle_func = new_idle_func;
}

unsigned PULSER_num_results(volatile void *base_addr)
{
    unsigned r = PULSER_mReadSlaveReg2(base_addr, 0);
    return (r  >> 4) & 31;
}

unsigned
PULSER_pop_result(volatile void *base_addr)
{
    while(PULSER_num_results(base_addr) == 0) {
        if(idle_func)
            idle_func();
    }
    return PULSER_mReadSlaveReg31(base_addr, 0);
}

// set bytes at addr+1 and addr
// note that get_dds_two bytes also returns data at addr+1 and addr
void
PULSER_set_dds_two_bytes(volatile void *base_addr, char i,
                         unsigned addr, unsigned data)
{
    //put addr in bits 15...9 (maps to DDS opcode_reg[14:9] )?
    unsigned dds_addr = (addr + 1) & 0x7F;
    // put data in bits 15...0 (maps to DDS operand_reg[15:0] )?
    unsigned dds_data = data & 0xFFFF;
    PULSER_short_pulse(base_addr,
                       0x10000002 | (i << 4) | (dds_addr << 9), dds_data);
}

//! set bytes addr+3 ... addr
void
PULSER_set_dds_four_bytes(volatile void *base_addr, char i,
                          unsigned addr, unsigned data)
{
    //put addr in bits 15...9 (maps to DDS opcode_reg[14:9])?
    unsigned dds_addr = (addr + 1) & 0x7F;
    PULSER_short_pulse(base_addr,
                       0x1000000F | (i << 4) | (dds_addr << 9), data);
}

void
PULSER_set_dds_freq(volatile void *base_addr, char i, unsigned ftw)
{
    PULSER_short_pulse(base_addr, 0x10000000 | (i << 4), ftw);
    /* ddsFTW[(int)i] = ftw; */
}

void
PULSER_set_dds_amp(volatile void *base_addr, char i, unsigned short A)
{
    PULSER_set_dds_two_bytes(base_addr, i, 0x32, A);
    /* ddsAmp[(int)i] = A; */
}

void
PULSER_set_dds_phase(volatile void *base_addr, char i, unsigned short phase)
{
    PULSER_set_dds_two_bytes(base_addr, i, 0x30, phase);
    ddsPhase[(int)i] = phase;
}

void
PULSER_shift_dds_phase(volatile void *base_addr, char i, unsigned short phase)
{
    PULSER_set_dds_phase(base_addr, i, phase + ddsPhase[(int)i]);
}

/* int */
/* PULSER_check_all_dds(volatile void *base_addr) */
/* { */
/*     for (char i = 0;i < nDDS_boards;i++) { */
/*         if (!PULSER_check_dds(base_addr, i)) { */
/*             printf("ERROR on DDS %d !\r\n", (int)i); */
/*             return 0; */
/*         } */
/*     } */
/*     return 1; */
/* } */

/* int */
/* PULSER_check_dds(volatile void *base_addr, char i) */
/* { */
/*     return ((ddsFTW[(int)i] == PULSER_get_dds_freq(base_addr, i)) && */
/*             (ddsPhase[(int)i] == PULSER_get_dds_phase(base_addr, i)) && */
/*             (ddsAmp[(int)i] == PULSER_get_dds_amp(base_addr, i))); */
/* } */

unsigned
PULSER_get_dds_freq(volatile void *base_addr, char i)
{
    //PULSER_short_pulse(base_addr, 0x1000000E | (i << 4) | (0x2D << 9), 0);
    //return PULSER_pop_result(base_addr);
    unsigned u0 = PULSER_get_dds_two_bytes(base_addr, i, 0x2C);
    unsigned u2 = PULSER_get_dds_two_bytes(base_addr, i, 0x2E);

    return u0 | (u2 << 16);
}

unsigned
PULSER_get_dds_phase(volatile void *base_addr, char i)
{
    return PULSER_get_dds_two_bytes(base_addr, i, 0x30);
}

unsigned
PULSER_get_dds_amp(volatile void *base_addr, char i)
{
    return PULSER_get_dds_two_bytes(base_addr, i, 0x32);
}
