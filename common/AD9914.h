#ifndef AD9914_H
#define AD9914_H

#include <nacs-old-pulser/pulser.h>
#include <stdio.h>

namespace NaCs {

bool init_AD9914(Pulser::OldPulser &pulser, int i, bool bForce);

bool test_AD9914(Pulser::OldPulser &pulser, int i);
void test_dds_addr(Pulser::OldPulser &pulser, int i, unsigned low_addr,
                   unsigned high_addr, unsigned ntest, FILE *f);

void print_AD9914_registers(Pulser::OldPulser &pulser, int i);

}

#endif
