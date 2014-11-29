#include "verbosity.h"

#include <stdarg.h>
#include <nacs-utils/log.h>

namespace NaCs {

int
verbosity::printf(const char *format, ...) const
{
    int ret = 0;

    va_list ap;
    va_start(ap, format);
    nacsLogV(format, ap);
    va_end(ap);

    if (m_pos) {
        char buff[1024];

        va_list vl;
        va_start(vl, format);
        ret = vsnprintf(buff, 1024, format, vl);
        va_end(vl);

        *m_pos << buff;
    }
    return ret;
}

}
