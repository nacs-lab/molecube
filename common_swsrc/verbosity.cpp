#include <stdarg.h>

#include "verbosity.h"

using namespace std;

int verbosity::printf(const char* format, ...)
{
    int ret = 0;

    if(f) {
        va_list vl;
        va_start(vl, format);
        ret = vfprintf(f, format, vl);
        va_end(vl);
        fflush(f);
    }

    if(pos) {
        char buff[1024];

        va_list vl;
        va_start(vl, format);
        ret = vsnprintf(buff, 1024, format, vl);
        va_end(vl);

        *pos << buff;
    }

    return ret;
}
