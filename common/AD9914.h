#ifndef AD9914_H
#define AD9914_H

#include <nacs-old-pulser/pulser.h>
#include <stdio.h>

namespace NaCs {
namespace Pulser {
class Controller;
}

namespace AD9914 {
bool init(Pulser::Controller &ctrl, int i, bool force);
void print_registers(Pulser::Controller &ctrl, int i);
}

bool init_AD9914(Pulser::OldPulser &pulser, int i, bool bForce);
void print_AD9914_registers(Pulser::OldPulser &pulser, int i);

}

#endif
