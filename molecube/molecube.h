#ifndef COMMON_H
#define COMMON_H

#include <nacs-utils/fd_utils.h>

#include <vector>
#include <mutex>

namespace NaCs {

extern volatile bool g_stop_curr_seq;
extern std::vector<unsigned> active_dds; // all DDS that are available

}

#endif
