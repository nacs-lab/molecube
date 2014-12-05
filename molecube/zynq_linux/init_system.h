#ifndef __MOLECUBE_INIT_SYSTEM_H__
#define __MOLECUBE_INIT_SYSTEM_H__

#include <nacs-pulser/pulser.h>

#define CPU_FREQ_HZ (667000000)

namespace NaCs {

Pulser::Pulser &init_system();

}

#endif
