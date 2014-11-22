#ifndef AD9914_H
#define AD9914_H

#include <stdio.h>

bool init_AD9914(volatile void *base_addr, char i, bool bForce);

void set_freq_AD9914PM(volatile void *base_addr, char i, unsigned ftw,
                       unsigned A, unsigned B, FILE *f);

bool test_AD9914(volatile void *base_addr, char i);
void test_dds_addr(volatile void *base_addr, char i, unsigned low_addr,
                   unsigned high_addr, unsigned ntest, FILE *f);

void print_AD9914_registers(volatile void *base_addr, char i);
void set_dds_four_bytes(volatile void *base_addr, char i, unsigned addr,
                        unsigned data);
unsigned gcd(unsigned x, unsigned y);

#endif // AD9914_H
