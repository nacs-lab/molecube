#ifndef AD9914_H
#define AD9914_H

#include <stdio.h>

namespace NaCs {
namespace Pulser {
class Controller;
}
namespace AD9914 {
bool init(Pulser::Controller &ctrl, int i, bool force);
void print_registers(Pulser::Controller &ctrl, int i);
}
}

#endif
