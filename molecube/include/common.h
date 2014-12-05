#ifndef COMMON_H
#define COMMON_H

#include <nacs-utils/fd_utils.h>

#include <vector>

namespace NaCs {

extern volatile bool g_stop_curr_seq;
// lock file. Set the lock when performing PULSER operations that
// should not be interrupted by other PULSER operations.
extern FLock g_fPulserLock;
extern std::vector<unsigned> active_dds; // all DDS that are available

}

#endif
