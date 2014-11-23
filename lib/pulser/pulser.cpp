#include "pulser_p.h"
#include "ctrl_io.h"

namespace NaCs {
namespace Pulser {

static bool
check_program(const uint32_t *prog, size_t len)
{
    if (len % 2 != 0) {
        return false;
    }
    for (size_t i = 0;i < len;i++) {
        int32_t offset = prog[i] - PULSER_USER_SLV_SPACE_OFFSET;
        if (nacsUnlikely(offset < 0 || offset >= 32 * 4)) {
            return false;
        }
    }
    return true;
}

static __attribute__((optimize("Ofast", "prefetch-loop-arrays"),
                      flatten, hot)) void
run_program_real(volatile void *base, const uint32_t *__restrict__ prog,
                 size_t len)
{
    // Options to benchmark
    // * With/Withtout Ofast/prefetch-loop-arrays
    // * loop with index/pointer
    // * whether to use __buildin_prefetch directly
    auto end = prog + len;
    for (auto p = prog;p < end;p++) {
        auto addr = *p;
        p++;
        auto val = *p;
        __builtin_prefetch(p + 1);
        PULSER_mWriteReg(base, addr, val);
    }
}

NACS_EXPORT int
run_program(volatile void *base, const uint32_t *prog, size_t len)
{
    if (nacsUnlikely(!check_program(prog, len))) {
        return -1;
    }
    run_program_real(base, prog, len);
    return 0;
}

}
}
