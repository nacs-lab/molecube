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
unsigned PULSE_CONTROLLER_vacancy = 0;

unsigned last_PMT_value = 0;

unsigned extra_flags = 0;

void (*idle_func)(void);

#define PULSE_CONTROLLER_MAX_NDDS (8)

unsigned int ddsFTW[PULSE_CONTROLLER_MAX_NDDS];
unsigned short ddsPhase[PULSE_CONTROLLER_MAX_NDDS];

#define ENABLE_TIMING_CHECK    (0x08000000)


inline void PULSE_CONTROLLER_unsafe_pulse(void* base_addr, const unsigned control, const unsigned operand)
{
  //does not check if there is space in buffer before writing.
  //but AXI bus will wait until space is available
  
//   PULSE_CONTROLLER_vacancy--;
//   __asm__ volatile ("stw %0,0(%2); stw %1,4(%2); eieio" : : "r" (control),  "r" (operand), "b" (base_addr + PULSE_CONTROLLER_WRFIFO_DATA_OFFSET));
	PULSE_CONTROLLER_mWriteSlaveReg31(base_addr, 0, operand);
	PULSE_CONTROLLER_mWriteSlaveReg31(base_addr, 0, control);
}

void PULSE_CONTROLLER_debug_regs(void* base_addr)
{
  printf("PULSE_CONTROLLER registers:\r\n");

	unsigned i;

	for(i=0; i<31; i++)
	{
		if(i%4 == 0)
			printf("[%2d...%2d]: ", i, i+3);

		printf("%08X ", PULSE_CONTROLLER_read_slave_reg(base_addr, i, 0));

		if(i%4 == 3)
			printf("\r\n", i, i+3);
	}

	printf("\r\n", i, i+3);
}

void PULSE_CONTROLLER_init(void* base_addr, unsigned nDDS, unsigned bResetDDS, int debug_level)
{
   //soft reset
   //PULSE_CONTROLLER_mReset(base_addr);
  
   if(debug_level > 0) PULSE_CONTROLLER_debug_regs(base_addr);

   char iDDS;
   unsigned r = 0;

   nDDS_boards = nDDS;

   if(debug_level > 0) printf("PULSE_CONTROLLER_init... disable timing check\r\n");
   PULSE_CONTROLLER_disable_timing_check(base_addr);

   if(debug_level > 0) printf("PULSE_CONTROLLER_init... reset DDS\r\n");
   for(iDDS=0; iDDS<nDDS_boards; iDDS++)
   {
	   if(bResetDDS)
			PULSE_CONTROLLER_dds_reset(base_addr, iDDS);

      PULSE_CONTROLLER_set_dds_div2(base_addr, iDDS, 0);
   }

//   printf("PULSE_CONTROLLER_init... get PMT\r\n");
//   r = PULSE_CONTROLLER_get_PMT(base_addr);
//   printf("Result = %08X\r\n", r);
}

void PULSE_CONTROLLER_reinit_DDS(void* base_addr, unsigned nDDS)
{
   char iDDS;
   unsigned ftw;

   for(iDDS=0; iDDS<nDDS; iDDS++)
   {
      ftw = PULSE_CONTROLLER_get_dds_freq(base_addr, iDDS);
      PULSE_CONTROLLER_dds_reset(base_addr, iDDS);
      PULSE_CONTROLLER_set_dds_div2(base_addr, iDDS, 0);
	  PULSE_CONTROLLER_set_dds_freq(base_addr, iDDS, ftw);
   }

   PULSE_CONTROLLER_get_PMT(base_addr);
}

//! Make sure there is space for at least n pulses on the FIFO.  NOT NEEDED ON AXI
void PULSE_CONTROLLER_ensure_vacancy(void* base_addr, unsigned n)
{
//   while(PULSE_CONTROLLER_vacancy < n)
//      PULSE_CONTROLLER_vacancy = PULSE_CONTROLLER_mWriteFIFOVacancy((Xuint32)base_addr);
}

//! Is the read FIFO empty?.
int PULSE_CONTROLLER_read_empty(void* base_addr)
{
   return PULSE_CONTROLLER_num_results(base_addr) == 0;
}

//! TTL functions: pulse_io = (ttl_out | high_mask) & (~low_mask);
void PULSE_CONTROLLER_set_ttl(void* base_addr, unsigned high_mask, unsigned low_mask)
{
   PULSE_CONTROLLER_mWriteSlaveReg0(base_addr, 0, high_mask);
   PULSE_CONTROLLER_mWriteSlaveReg1(base_addr, 0, low_mask);
}

//! TTL functions: pulse_io = (ttl_out | high_mask) & (~low_mask);
void PULSE_CONTROLLER_get_ttl(void* base_addr, unsigned* high_mask, unsigned* low_mask)
{
   *high_mask = PULSE_CONTROLLER_read_slave_reg(base_addr, 0, 0);
   *low_mask = PULSE_CONTROLLER_read_slave_reg(base_addr, 0, 4);
}


//! enable / disable clock_out
//! divider = 0..254 means emit clock with period 2 x (divider+1) in pulse controller timing units (DT_ns)
//! divider = 255 means disable
void PULSE_CONTROLLER_enable_clock_out(void* base_addr, unsigned divider)
{
   PULSE_CONTROLLER_short_pulse(base_addr, 0x50000000, divider & 0xFF);
}

//! DDS functions - reset DDS i
void PULSE_CONTROLLER_dds_reset(void* base_addr, char i)
{
   PULSE_CONTROLLER_short_pulse(base_addr, 0x10000004 | (i << 4), 0);

   ddsFTW[i] = 0;
   ddsPhase[i] = 0;
}

//! DDS functions - set whether GHz DDS clock is divided by 2
void PULSE_CONTROLLER_set_dds_div2(void* base_addr, char i, int b)
{
   if(b)
      PULSE_CONTROLLER_short_pulse(base_addr, 0x10000002 | (i << 4), 0x18000000);
   else
      PULSE_CONTROLLER_short_pulse(base_addr, 0x10000002 | (i << 4), 0x58000000);
}

//! DDS functions - get byte from address on DDS i
unsigned PULSE_CONTROLLER_get_dds_byte(void* base_addr, char i, unsigned address)
{
   PULSE_CONTROLLER_short_pulse(base_addr, 0x10000003 | (i << 4), address << 18);
   return (PULSE_CONTROLLER_pop_result(base_addr) & 0xff);
}

//! toggle init.  reset prior to new sequence
void PULSE_CONTROLLER_toggle_init(void* base_addr)
{
  unsigned r3 = PULSE_CONTROLLER_read_slave_reg(base_addr, 3, 0);
  PULSE_CONTROLLER_write_slave_reg(base_addr, 3, 0, r3 |   0x00000100);
  PULSE_CONTROLLER_write_slave_reg(base_addr, 3, 0, r3 & ~(0x00000100));
}

//! set hold.  pulses are stopped
void PULSE_CONTROLLER_set_hold(void* base_addr)
{
  unsigned r3 = PULSE_CONTROLLER_read_slave_reg(base_addr, 3, 0);
  PULSE_CONTROLLER_write_slave_reg(base_addr, 3, 0, r3 | 0x00000080);
}

//! release hold.  pulses can run
void PULSE_CONTROLLER_release_hold(void* base_addr)
{
  unsigned r3 = PULSE_CONTROLLER_read_slave_reg(base_addr, 3, 0);
  PULSE_CONTROLLER_write_slave_reg(base_addr, 3, 0, r3 & ~(0x00000080));
}

//! enable timing check for pulses
void PULSE_CONTROLLER_enable_timing_check(void* base_addr)
{
   extra_flags = extra_flags | ENABLE_TIMING_CHECK;
}

//! disable timing check for pulses
void PULSE_CONTROLLER_disable_timing_check(void* base_addr)
{
   extra_flags = extra_flags & ~ENABLE_TIMING_CHECK;
}

//! clear timing check (clear failures)
void PULSE_CONTROLLER_clear_timing_check(void* base_addr)
{
   PULSE_CONTROLLER_short_pulse(base_addr, 0x30000000, 0);
}

//! were there any timing failures?
int  PULSE_CONTROLLER_timing_ok(void* base_addr)
{
   return !(PULSE_CONTROLLER_read_sr(base_addr, 2) & 0x1);
}

//! return whether current pulse sequence is finished
unsigned PULSE_CONTROLLER_is_finished(void* base_addr)
{
    PULSE_CONTROLLER_read_sr(base_addr, 2) & 0x4;
}

//! wait for the current pulse sequence to finish
void PULSE_CONTROLLER_wait_for_finished(void* base_addr)
{
  PULSE_CONTROLLER_release_hold(base_addr);
  
    while(!PULSE_CONTROLLER_is_finished(base_addr))
      sleep(0);
}

//! get status register from write FIFO
//unsigned PULSE_CONTROLLER_get_write_status(void* base_addr)
//{
//   return PULSE_CONTROLLER_mReadReg(base_addr, PULSE_CONTROLLER_WRFIFO_SR_OFFSET);
//}

//! get status register from read FIFO
//unsigned PULSE_CONTROLLER_get_read_status(void* base_addr)
//{
//   return PULSE_CONTROLLER_mReadReg(base_addr, PULSE_CONTROLLER_RDFIFO_SR_OFFSET);
//}

void PULSE_CONTROLLER_write_slave_reg(void* base_addr, char n, unsigned offset, unsigned val)
{
	PULSE_CONTROLLER_mWriteSlaveReg0(base_addr, offset + 4*n, val);
}

unsigned PULSE_CONTROLLER_read_slave_reg(void* base_addr, char n, unsigned offset)
{
	volatile unsigned u = PULSE_CONTROLLER_mReadSlaveReg0(base_addr, offset +4*n);
  return u;
}

void PULSE_CONTROLLER_self_test(void* base_addr, int nIO)
{
   unsigned ftw [PULSE_CONTROLLER_MAX_NDDS];
   unsigned cycle = 0;
   unsigned nBad = 0;
   char iDDS;

   PULSE_CONTROLLER_test_slave_registers(base_addr);
   printf("\r\n");

   if(nIO > 0)
   {
      for(iDDS=0; iDDS<nDDS_boards; iDDS++)
         PULSE_CONTROLLER_test_dds(base_addr, iDDS);

      printf("Testing %d random read/writes on DDS boards 0-%d ... ", nIO, nDDS_boards-1);

      //initialize to 0 Hz
      for(iDDS=0; iDDS<nDDS_boards; iDDS++)
      {
         ftw[iDDS] = 0;
         PULSE_CONTROLLER_set_dds_freq(base_addr, iDDS, ftw[iDDS]);
      }

      while(cycle < nIO)
      {
         iDDS = rand() % nDDS_boards;
         unsigned ftw_read = PULSE_CONTROLLER_get_dds_freq(base_addr, iDDS);

         if(ftw_read != ftw[iDDS])
         {
            printf("\r\n");
            printf("ERROR on DDS %d : wrote FTW %08X\r\n", (int)iDDS, ftw[iDDS]);
            printf("                   read FTW %08X\r\n", ftw_read);
            nBad++;
         }

         ftw[iDDS] = rand();
         PULSE_CONTROLLER_set_dds_freq(base_addr, iDDS, ftw[iDDS]);

         cycle++;
      }

      for(iDDS=0; iDDS<nDDS_boards; iDDS++)
      {
         PULSE_CONTROLLER_set_dds_freq(base_addr, iDDS, 0);
      }

      if(nBad == 0)
         printf("OK\r\n");
      else
         printf("FAILURE: %d errors\r\n", nBad);
   }
}

int PULSE_CONTROLLER_test_slave_registers(void* base_addr)
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
         PULSE_CONTROLLER_write_slave_reg(base_addr, k, 0, test_val);

         read = PULSE_CONTROLLER_read_slave_reg(base_addr, k, 0);

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

int PULSE_CONTROLLER_test_dds(void* base_addr, char nDDS)
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

      PULSE_CONTROLLER_set_dds_freq(base_addr, nDDS, test_val);
      read = PULSE_CONTROLLER_get_dds_freq(base_addr, nDDS);
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
      PULSE_CONTROLLER_set_dds_phase(base_addr, nDDS, phase);
      read = PULSE_CONTROLLER_get_dds_phase(base_addr, nDDS);

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

unsigned PULSE_CONTROLLER_get_last_PMT()
{
    return last_PMT_value;
}

void PULSE_CONTROLLER_set_last_PMT(unsigned n)
{
    last_PMT_value = n;
}

unsigned PULSE_CONTROLLER_get_PMT(void* base_addr)
{
   unsigned new_PMT_value, d;

   PULSE_CONTROLLER_short_pulse(base_addr, (0x20000000), 0);
   new_PMT_value = PULSE_CONTROLLER_pop_result(base_addr);
   d = new_PMT_value - last_PMT_value;

   last_PMT_value = new_PMT_value;
   return d;
}

unsigned PULSE_CONTROLLER_read_sr(void* base_addr, unsigned i)
{
	return PULSE_CONTROLLER_mReadSlaveReg0(base_addr, 4*i);
}

//make timed pulses
//if t > t_max, subdivide into shorter pulses
//returns number of pulses made
unsigned PULSE_CONTROLLER_pulse(void* base_addr, unsigned t, const unsigned flags, const unsigned operand)
{
   unsigned t_max = 0x001FFFFF;
   unsigned t_big = 0x001FFFF0;

   unsigned nPulses = 1;

   while(t > t_max)
   {
      PULSE_CONTROLLER_short_pulse(base_addr, t_big | flags, operand);
      t -= t_big;
      nPulses++;
   }

   PULSE_CONTROLLER_short_pulse(base_addr, t | flags, operand);

   return nPulses;
}

//make short timed pulses
//FPGA can only handle pulse lengths up to t_max = 0x001FFFFF (about 40 ms)
void PULSE_CONTROLLER_short_pulse(void* base_addr, const unsigned control, const unsigned operand)
{
   PULSE_CONTROLLER_unsafe_pulse(base_addr, control | extra_flags, operand);
}


void PULSE_CONTROLLER_set_idle_function(void (*new_idle_func)(void))
{
	idle_func = new_idle_func;
}

unsigned PULSE_CONTROLLER_num_results(void* base_addr)
{
	unsigned r = PULSE_CONTROLLER_mReadSlaveReg2(base_addr, 0);
	return (r  >> 4) & 31;
}

unsigned PULSE_CONTROLLER_pop_result(void* base_addr)
{
   unsigned r;

   while(PULSE_CONTROLLER_num_results(base_addr) == 0) 
   {
	   if(idle_func)
		   idle_func();
   }

   return PULSE_CONTROLLER_mReadSlaveReg31(base_addr, 0);
}

void PULSE_CONTROLLER_set_dds_freq(void* base_addr, char i, unsigned freq)
{
   PULSE_CONTROLLER_short_pulse(base_addr, 0x10000000 | (i << 4), freq);

   ddsFTW[i] = freq;
}

void PULSE_CONTROLLER_set_dds_phase(void* base_addr, char i, unsigned short phase)
{
   PULSE_CONTROLLER_short_pulse(base_addr, 0x10001000 | (i << 4), phase);

   ddsPhase[i] = phase;
}

int PULSE_CONTROLLER_check_all_dds(void* base_addr)
{
   char i;

   for(i=0; i<nDDS_boards; i++)
   {
      if(!PULSE_CONTROLLER_check_dds(base_addr, i))
      {
         printf("ERROR on DDS %d !\r\n", (int)i);
         return 0;
      }
   }

   return 1;
}

int PULSE_CONTROLLER_check_dds(void* base_addr, char i)
{
   return (ddsFTW[i] == PULSE_CONTROLLER_get_dds_freq(base_addr, i)) && (ddsPhase[i] == PULSE_CONTROLLER_get_dds_phase(base_addr, i));
}

unsigned PULSE_CONTROLLER_get_dds_freq(void* base_addr, char i)
{
   PULSE_CONTROLLER_short_pulse(base_addr, 0x10005000 | (i << 4), 0);
   return PULSE_CONTROLLER_pop_result(base_addr);
}

unsigned PULSE_CONTROLLER_get_dds_phase(void* base_addr, char i)
{
   PULSE_CONTROLLER_short_pulse(base_addr, 0x10006000 | (i << 4), 0);
   return PULSE_CONTROLLER_pop_result(base_addr) & 0x3fff;
}

void PULSE_CONTROLLER_config_PMT_correlation(void* base_addr, unsigned removeFromQueue, unsigned clkDiv)
{
  PULSE_CONTROLLER_write_slave_reg(base_addr, 3, 0, 0x00000040 | (removeFromQueue << 5) | ((0x0F & clkDiv) << 1));
  PULSE_CONTROLLER_write_slave_reg(base_addr, 3, 0, 0x00000000);
}

void PULSE_CONTROLLER_reset_PMT_correlation(void* base_addr)
{
  PULSE_CONTROLLER_write_slave_reg(base_addr, 3, 0, 0x00000001);
  PULSE_CONTROLLER_write_slave_reg(base_addr, 3, 0, 0x00000000);
}

void PULSE_CONTROLLER_get_PMT_correlation(void* base_addr, unsigned* histogram)
{
  unsigned k, j;
  const unsigned bins_per_word = 32/PULSE_CONTROLLER_N_PMT_BITS;
  const unsigned n_words = PULSE_CONTROLLER_N_PMT_BINS/bins_per_word;
  const unsigned max_val = (1 << PULSE_CONTROLLER_N_PMT_BITS)-1;
  
  for(j = 0; j < n_words; j++)
  {
    unsigned r = PULSE_CONTROLLER_read_slave_reg(base_addr, 4+j, 0);
    for(k=0; k<bins_per_word; k++)
      histogram[j*bins_per_word + k] += max_val & (r >> (PULSE_CONTROLLER_N_PMT_BITS*k));
  }
}
