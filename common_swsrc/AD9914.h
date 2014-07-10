#ifndef AD9914_H
#define AD9914_H

//pulse controller cycles needed to upate the DDS frequency
//this will be reduced in the new HW
#define T_MIN_SET_FREQ_AD9914 (50)

#include <stdio.h>

void init_AD9914(void* base_addr, char i);

void set_freq_AD9914PM(void* base_addr, char i, unsigned ftw, unsigned A, unsigned B, FILE* f=0);
void set_phase_AD9914(void* base_addr, char i, unsigned ptw, FILE* f=0);

double get_freq_AD9914(void* base_addr, char i); //get freq in Hz
unsigned get_ftw_AD9914(void* base_addr, char i); //get freq. tuning word (FTW)
double get_amp_AD9914(void* base_addr, char i); //get amp in % (0...100)
double get_phase_AD9914(void* base_addr, char i); //get phase in deg (0...360)

bool test_AD9914(void* base_addr, char i);
void test_dds_addr(void* base_addr, char i, unsigned low_addr, unsigned high_addr,
                   unsigned ntest, FILE* f=0);

void print_AD9914_registers(void* base_addr, char i, FILE* f);
//void set_dds_byte_AD9914(void* base_addr, char i, unsigned addr, unsigned data);
void set_dds_four_bytes(void* base_addr, char i, unsigned addr, unsigned data);
void set_freq_AD9914(void* base_addr, char i, double Hz, FILE* f=0);
void set_ftw_AD9914(void* base_addr, char i, unsigned ftw, FILE* f=0);
void set_amp_AD9914(void* base_addr, char i, unsigned A, FILE* f=0);
unsigned gcd(unsigned x, unsigned y);

#endif // AD9914_H
