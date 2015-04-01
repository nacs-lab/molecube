#ifndef __MOLECUBE_INIT_SYSTEM_H__
#define __MOLECUBE_INIT_SYSTEM_H__

#define CPU_FREQ_HZ (667000000)

namespace NaCs {
namespace Pulser {
class Controller;
}

Pulser::Controller &init_system();

}

#endif
