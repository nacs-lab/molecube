#ifndef AD9914_H
#define AD9914_H

#include <stdio.h>

namespace NaCs {
namespace Pulser {
class Controller;
}
namespace AD9914 {
enum InitFlags : int {
    Force = 1 << 0,
    LogAction = 1 << 1,
    LogVerbose = 1 << 2,
};
bool init(Pulser::Controller &ctrl, int i, InitFlags flags=LogVerbose);
void print_registers(Pulser::Controller &ctrl, int i);
}
}

#endif
