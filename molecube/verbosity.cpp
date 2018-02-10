#include "verbosity.h"

#include <stdarg.h>
#include <nacs-utils/log.h>

namespace NaCs {

const verbosity&
verbosity::printf(const char *format, ...) const
{
    va_list ap;
    va_start(ap, format);
    nacsLogV(format, ap);
    va_end(ap);

    if (m_pos) {
        char buff[1024];

        va_list vl;
        va_start(vl, format);
        vsnprintf(buff, 1024, format, vl);
        va_end(vl);

        *m_pos << buff;
    }
    return *this;
}

}
