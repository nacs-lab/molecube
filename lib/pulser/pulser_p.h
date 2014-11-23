#include <nacs-utils/utils.h>

#ifndef __NACS_PULSER_PULSER_H__
#define __NACS_PULSER_PULSER_H__

namespace NaCs {
namespace Pulser {

int run_program(volatile void *base, const uint32_t *prog, size_t len);

}
}

#endif
