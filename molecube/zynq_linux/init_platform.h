#include <nacs-pulser/pulser.h>

#ifndef __MOLECUBE_INIT_PLATFORM_H__
#define __MOLECUBE_INIT_PLATFORM_H__

namespace NaCs {

//Platform specific initialization
//Linux: get device addresses

Pulser::Pulser get_pulser();

}

#endif
