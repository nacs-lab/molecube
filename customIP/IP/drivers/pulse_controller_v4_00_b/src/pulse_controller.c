/*****************************************************************************
* Filename:          pulse_controller.c
* Version:           4.00.b
* Description:       pulse_controller Driver Source File
* Date:              10:33 PM Friday, July 13, 2012
*****************************************************************************/


/***************************** Include Files *******************************/

#include "pulse_controller.h"
#include <stdio.h>

#ifndef ALUMINIZER_SIM
   #include "pulse_controller_io.h"
#endif

unsigned nDDS_boards = 0;
unsigned PULSER_vacancy = 0;

unsigned last_PMT_value = 0;

unsigned extra_flags = 0;

void (*idle_func)(void);

#define MAX_NDDS 32

unsigned int ddsFTW[MAX_NDDS];
unsigned short ddsPhase[MAX_NDDS];
unsigned short ddsAmp[MAX_NDDS];

#define ENABLE_TIMING_CHECK    (0x08000000)


inline void PULSER_unsafe_pulse(void* base_addr, const unsigned control, const unsigned operand)
{
  //does not check if there is space in buffer before writing.
  //but AXI bus will wait until space is available
  
//   PULSER_vacancy--;
//   __asm__ volatile ("stw %0,0(%2); stw %1,4(%2); eieio" : : "r" (control),  "r" (operand), "b" (base_addr + PULSER_WRFIFO_DATA_OFFSET));
	PULSER_mWriteSlaveReg31(base_addr, 0, operand);
	PULSER_mWriteSlaveReg31(base_addr, 0, control);
}

void PULSER_debug_regs(void* base_addr)
{
  printf("PULSE_CONTROLLER registers:\r\n");

	unsigned i;

	for(i=0; i<31; i++)
	{
		if(i%4 == 0)
			printf("[%2d...%2d]: ", i, i+3);

		printf("%08X ", PULSER_read_slave_reg(base_addr, i, 0));

		if(i%4 == 3)
			printf("\r\n", i, i+3);
	}

	printf("\r\n", i, i+3);
}

void PULSER_init(void* base_addr, unsigned nDDS, unsigned bResetDDS, int debug_level)
{
   //soft reset
   //PULSER_mReset(base_addr);
  
   if(debug_level > 0) PULSER_debug_regs(base_addr);

   char iDDS;
   unsigned r = 0;

   nDDS_boards = nDDS;

   if(debug_level > 0) printf("PULSER_init... disable timing check\r\n");
   PULSER_disable_timing_check(base_addr);

   if(debug_level > 0) printf("PULSER_init... reset DDS\r\n");
   for(iDDS=0; iDDS<nDDS_boards; iDDS++)
   {
	   if(bResetDDS)
			PULSER_dds_reset(base_addr, iDDS);

      //TR: no div2 for AD9914 PULSER_set_dds_div2(base_addr, iDDS, 0);
   }

//   printf("PULSER_init... get PMT\r\n");
//   r = PULSER_get_PMT(base_addr);
//   printf("Result = %08X\r\n", r);
}

void PULSER_reinit_DDS(void* base_addr, unsigned nDDS)
{
   char iDDS;
   unsigned ftw;

   for(iDDS=0; iDDS<nDDS; iDDS++)
   {
      ftw = PULSER_get_dds_freq(base_addr, iDDS);
      PULSER_dds_reset(base_addr, iDDS);
      //TR: no div2 for AD9914 PULSER_set_dds_div2(base_addr, iDDS, 0);
	    PULSER_set_dds_freq(base_addr, iDDS, ftw);
   }

   PULSER_get_PMT(base_addr);
}

//! Make sure there is space for at least n pulses on the FIFO.  NOT NEEDED ON AXI
void PULSER_ensure_vacancy(void* base_addr, unsigned n)
{
//   while(PULSER_vacancy < n)
//      PULSER_vacancy = PULSER_mWriteFIFOVacancy((Xuint32)base_addr);
}

//! Is the read FIFO empty?.
int PULSER_read_empty(void* base_addr)
{
   return PULSER_num_results(base_addr) == 0;
}

//! TTL functions: pulse_io = (ttl_out | high_mask) & (~low_mask);
void PULSER_set_ttl(void* base_addr, unsigned high_mask, unsigned low_mask)
{
   PULSER_mWriteSlaveReg0(base_addr, 0, high_mask);
   PULSER_mWriteSlaveReg1(base_addr, 0, low_mask);
}

//! TTL functions: pulse_io = (ttl_out | high_mask) & (~low_mask);
void PULSER_get_ttl(void* base_addr, unsigned* high_mask, unsigned* low_mask)
{
   *high_mask = PULSER_read_slave_reg(base_addr, 0, 0);
   *low_mask = PULSER_read_slave_reg(base_addr, 0, 4);
}


//! enable / disable clock_out
//! divider = 0..254 means emit clock with period 2 x (divider+1) in pulse controller timing units (DT_ns)
//! divider = 255 means disable
void PULSER_enable_clock_out(void* base_addr, unsigned divider)
{
   PULSER_short_pulse(base_addr, 0x50000000, divider & 0xFF);
}

//! DDS functions - reset DDS i
void PULSER_dds_reset(void* base_addr, char i)
{
   PULSER_short_pulse(base_addr, 0x10000004 | (i << 4), 0);

   ddsFTW[i] = 0;
   ddsPhase[i] = 0;
}

//! DDS functions - reset DDS selected by bitmask mask
void PULSER_dds_reset_sel(void* base_addr, unsigned mask)
{
  PULSER_short_pulse(base_addr, 0x10000005, mask);
}

//! DDS functions - select DDS (high bits in mask).  Remember to unselect (mask = 0)
void PULSER_dds_set_sel(void* base_addr, unsigned mask)
{
  PULSER_short_pulse(base_addr, 0x10000006, mask);
}

//! DDS functions - get byte from address on DDS i
unsigned PULSER_get_dds_byte(void* base_addr, char i, unsigned address)
{
   PULSER_short_pulse(base_addr, 0x10000003 | (i << 4) | ((address) << 9), 0);
   return (PULSER_pop_result(base_addr) >> 8) & 0x000000ff;
}

//! DDS functions - get two bytes from address+1 and adress on DDS i
unsigned PULSER_get_dds_two_bytes(void* base_addr, char i, unsigned address)
{
   PULSER_short_pulse(base_addr, 0x10000003 | (i << 4) | ((address+1) << 9), 0);
   return PULSER_pop_result(base_addr) & 0x0000ffff;
}

//! toggle init.  reset prior to new sequence
void PULSER_toggle_init(void* base_addr)
{
  unsigned r3 = PULSER_read_slave_reg(base_addr, 3, 0);
  PULSER_write_slave_reg(base_addr, 3, 0, r3 |   0x00000100);
  PULSER_write_slave_reg(base_addr, 3, 0, r3 & ~(0x00000100));
}

//! set hold.  pulses are stopped
void PULSER_set_hold(void* base_addr)
{
  unsigned r3 = PULSER_read_slave_reg(base_addr, 3, 0);
  PULSER_write_slave_reg(base_addr, 3, 0, r3 | 0x00000080);
}

//! release hold.  pulses can run
void PULSER_release_hold(void* base_addr)
{
  unsigned r3 = PULSER_read_slave_reg(base_addr, 3, 0);
  PULSER_write_slave_reg(base_addr, 3, 0, r3 & ~(0x00000080));
}

//! enable timing check for pulses
void PULSER_enable_timing_check(void* base_addr)
{
   extra_flags = extra_flags | ENABLE_TIMING_CHECK;
}

//! disable timing check for pulses
void PULSER_disable_timing_check(void* base_addr)
{
   extra_flags = extra_flags & ~ENABLE_TIMING_CHECK;
}

//! clear timing check (clear failures)
void PULSER_clear_timing_check(void* base_addr)
{
   PULSER_short_pulse(base_addr, 0x30000000, 0);
}

//! were there any timing failures?
int  PULSER_timing_ok(void* base_addr)
{
   return !(PULSER_read_sr(base_addr, 2) & 0x1);
}

//! return whether current pulse sequence is finished
unsigned PULSER_is_finished(void* base_addr)
{
    PULSER_read_sr(base_addr, 2) & 0x4;
}

//! wait for the current pulse sequence to finish
void PULSER_wait_for_finished(void* base_addr)
{
  PULSER_release_hold(base_addr);
  
    while(!PULSER_is_finished(base_addr))
      sleep(0);
}

//! get status register from write FIFO
//unsigned PULSER_get_write_status(void* base_addr)
//{
//   return PULSER_mReadReg(base_addr, PULSER_WRFIFO_SR_OFFSET);
//}

//! get status register from read FIFO
//unsigned PULSER_get_read_status(void* base_addr)
//{
//   return PULSER_mReadReg(base_addr, PULSER_RDFIFO_SR_OFFSET);
//}

void PULSER_write_slave_reg(void* base_addr, char n, unsigned offset, unsigned val)
{
	PULSER_mWriteSlaveReg0(base_addr, offset + 4*n, val);
}

unsigned PULSER_read_slave_reg(void* base_addr, char n, unsigned offset)
{
	volatile unsigned u = PULSER_mReadSlaveReg0(base_addr, offset +4*n);
  return u;
}

void PULSER_self_test(void* base_addr, int nIO)
{
   unsigned ftw [MAX_NDDS];
   unsigned cycle = 0;
   unsigned nBad = 0;
   char iDDS;

   PULSER_test_slave_registers(base_addr);
   printf("\r\n");

   if(nIO > 0)
   {
      for(iDDS=0; iDDS<nDDS_boards; iDDS++)
         PULSER_test_dds(base_addr, iDDS);

      printf("Testing %d random read/writes on DDS boards 0-%d ... ", nIO, nDDS_boards-1);

      //initialize to 0 Hz
      for(iDDS=0; iDDS<nDDS_boards; iDDS++)
      {
         ftw[iDDS] = 0;
         PULSER_set_dds_freq(base_addr, iDDS, ftw[iDDS]);
      }

      while(cycle < nIO)
      {
         iDDS = rand() % nDDS_boards;
         unsigned ftw_read = PULSER_get_dds_freq(base_addr, iDDS);

         if(ftw_read != ftw[iDDS])
         {
            printf("\r\n");
            printf("ERROR on DDS %d : wrote FTW %08X\r\n", (int)iDDS, ftw[iDDS]);
            printf("                   read FTW %08X\r\n", ftw_read);
            nBad++;
         }

         ftw[iDDS] = rand();
         PULSER_set_dds_freq(base_addr, iDDS, ftw[iDDS]);

         cycle++;
      }

      for(iDDS=0; iDDS<nDDS_boards; iDDS++)
      {
         PULSER_set_dds_freq(base_addr, iDDS, 0);
      }

      if(nBad == 0)
         printf("OK\r\n");
      else
         printf("FAILURE: %d errors\r\n", nBad);
   }
}

int PULSER_test_slave_registers(void* base_addr)
{
   int sr_ok = 1;
   unsigned test_val, read;
   int i, j, k;

   for(i=0; i<2; i++)
   {
      test_val = 0;

      for(j=0; j<8; j++)
         test_val = test_val + ((i*0xF) << (j*4));

      printf("Testing %08X   ", test_val);
      for(k=0; k<8; k++)
      {
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

int PULSER_test_dds(void* base_addr, char nDDS)
{
   int ftw_ok = 1;
   int phase_ok = 1;
   unsigned phase = 0;
   int i, j;
   unsigned read, test_val;

   printf("Testing DDS %d ... ", (int)nDDS);
   for(i=0; i<2; i++)
   {
      test_val = 0;

      for(j=0; j<8; j++)
         test_val = test_val + ((i*0xF) << (j*4));

      PULSER_set_dds_freq(base_addr, nDDS, test_val);
      read = PULSER_get_dds_freq(base_addr, nDDS);
      ftw_ok = ftw_ok && (read == test_val);

      if(read != test_val)
      {
         printf("ERROR !\r\n", nDDS);
         printf("wrote FTW %08X\r\n", test_val);
         printf(" read FTW %08X\r\n", read);
      }
   }

   while(phase < 0x4000)
   {
      PULSER_set_dds_phase(base_addr, nDDS, phase);
      read = PULSER_get_dds_phase(base_addr, nDDS);

      phase_ok = phase_ok && (read == phase);

      if(read != phase)
      {
         printf("ERROR on DDS %d !\r\n", nDDS);
         printf("wrote PHASE %04X\r\n", test_val);
         printf(" read PHASE %04X\r\n", read);
      }

      phase++;
   }

   if(ftw_ok && phase_ok)
      printf("OK\r\n", (int)nDDS);
   else
      printf("DDS %d FAILED\r\n", (int)nDDS);

   return ftw_ok && phase_ok;
}

unsigned PULSER_get_last_PMT()
{
    return last_PMT_value;
}

void PULSER_set_last_PMT(unsigned n)
{
    last_PMT_value = n;
}

unsigned PULSER_get_PMT(void* base_addr)
{
   unsigned new_PMT_value, d;

   PULSER_short_pulse(base_addr, (0x20000000), 0);
   new_PMT_value = PULSER_pop_result(base_addr);
   d = new_PMT_value - last_PMT_value;

   last_PMT_value = new_PMT_value;
   return d;
}

unsigned PULSER_read_sr(void* base_addr, unsigned i)
{
	return PULSER_mReadSlaveReg0(base_addr, 4*i);
}

//make timed pulses
//if t > t_max, subdivide into shorter pulses
//returns number of pulses made
unsigned PULSER_pulse(void* base_addr, unsigned t, const unsigned flags, const unsigned operand)
{
   unsigned t_max = 0x001FFFFF;
   unsigned t_big = 0x001FFFF0;

   unsigned nPulses = 1;

   while(t > t_max)
   {
      PULSER_short_pulse(base_addr, t_big | flags, operand);
      t -= t_big;
      nPulses++;
   }

   PULSER_short_pulse(base_addr, t | flags, operand);

   return nPulses;
}

//make short timed pulses
//FPGA can only handle pulse lengths up to t_max = 0x001FFFFF (about 40 ms)
void PULSER_short_pulse(void* base_addr, const unsigned control, const unsigned operand)
{
   PULSER_unsafe_pulse(base_addr, control | extra_flags, operand);
}


void PULSER_set_idle_function(void (*new_idle_func)(void))
{
	idle_func = new_idle_func;
}

unsigned PULSER_num_results(void* base_addr)
{
	unsigned r = PULSER_mReadSlaveReg2(base_addr, 0);
	return (r  >> 4) & 31;
}

unsigned PULSER_pop_result(void* base_addr)
{
   unsigned r;

   while(PULSER_num_results(base_addr) == 0) 
   {
	   if(idle_func)
		   idle_func();
   }

   return PULSER_mReadSlaveReg31(base_addr, 0);
}
 
// set bytes at addr+1 and addr
// note that get_dds_two bytes also returns data at addr+1 and addr
void PULSER_set_dds_two_bytes(void* base_addr, char i, unsigned addr, unsigned data)
{
//	printf("AD9914 board=%i set byte [0x%02X] = 0x%02X\n", addr, data);

    unsigned dds_addr = (addr+1) & 0x3F; //put addr in bits 14...9 (maps to DDS opcode_reg[14:9] )?
    unsigned dds_data = data & 0xFFFF; // put data in bits 15...0 (maps to DDS operand_reg[15:0] )?
    PULSER_short_pulse(base_addr, 0x10000002 | (i << 4) | (dds_addr << 9), dds_data); // takes 0.3 us
}

void PULSER_set_dds_freq(void* base_addr, char i, unsigned ftw)
{
   PULSER_short_pulse(base_addr, 0x10000000 | (i << 4), ftw);

   ddsFTW[i] = ftw;
}

void PULSER_set_dds_amp(void* base_addr, char i, unsigned A)
{
   PULSER_set_dds_two_bytes(base_addr, i, 0x32, A & 0x0FFF);

   ddsAmp[i] = A;
}

void PULSER_set_dds_phase(void* base_addr, char i, unsigned short phase)
{
   PULSER_short_pulse(base_addr, 0x10001000 | (i << 4), phase);

   ddsPhase[i] = phase;
}

int PULSER_check_all_dds(void* base_addr)
{
   char i;

   for(i=0; i<nDDS_boards; i++)
   {
      if(!PULSER_check_dds(base_addr, i))
      {
         printf("ERROR on DDS %d !\r\n", (int)i);
         return 0;
      }
   }

   return 1;
}

int PULSER_check_dds(void* base_addr, char i)
{
   return (ddsFTW[i] == PULSER_get_dds_freq(base_addr, i)) && (ddsPhase[i] == PULSER_get_dds_phase(base_addr, i));
}

unsigned PULSER_get_dds_freq(void* base_addr, char i)
{
   PULSER_short_pulse(base_addr, 0x10005000 | (i << 4), 0);
   return PULSER_pop_result(base_addr);
}

unsigned PULSER_get_dds_phase(void* base_addr, char i)
{
   PULSER_short_pulse(base_addr, 0x10006000 | (i << 4), 0);
   return PULSER_pop_result(base_addr) & 0x3fff;
}

void PULSER_config_PMT_correlation(void* base_addr, unsigned removeFromQueue, unsigned clkDiv)
{
  PULSER_write_slave_reg(base_addr, 3, 0, 0x00000040 | (removeFromQueue << 5) | ((0x0F & clkDiv) << 1));
  PULSER_write_slave_reg(base_addr, 3, 0, 0x00000000);
}

void PULSER_reset_PMT_correlation(void* base_addr)
{
  PULSER_write_slave_reg(base_addr, 3, 0, 0x00000001);
  PULSER_write_slave_reg(base_addr, 3, 0, 0x00000000);
}

void PULSER_get_PMT_correlation(void* base_addr, unsigned* histogram)
{
  unsigned k, j;
  const unsigned bins_per_word = 32/PULSER_N_PMT_BITS;
  const unsigned n_words = PULSER_N_PMT_BINS/bins_per_word;
  const unsigned max_val = (1 << PULSER_N_PMT_BITS)-1;
  
  for(j = 0; j < n_words; j++)
  {
    unsigned r = PULSER_read_slave_reg(base_addr, 4+j, 0);
    for(k=0; k<bins_per_word; k++)
      histogram[j*bins_per_word + k] += max_val & (r >> (PULSER_N_PMT_BITS*k));
  }
}
