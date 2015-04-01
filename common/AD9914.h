#ifndef AD9914_H
#define AD9914_H

#include <nacs-old-pulser/pulser.h>
#include <stdio.h>

namespace NaCs {

bool init_AD9914(Pulser::OldPulser &pulser, int i, bool bForce);
void print_AD9914_registers(Pulser::OldPulser &pulser, int i);

}

#endif
