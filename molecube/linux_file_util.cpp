#include "linux_file_util.h"

#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>

#include <mutex>

/* Generic functions to write values to and read them from file.
 * Calls to these functions can manipulate file-mapped devices in Linux. */

namespace NaCs {

// Record status string in file.
// Keep the file open, and for each status update, lock it exclusively,
// then rewrite and unlock it.  Readers should acquire a lock,
// to prevent reading of partially written or empty files.
void
setProgramStatus(const char *str)
{
    static FILE *f = fopen("/var/run/molecube/molecube.status", "w");
    static std::mutex lock;

    if (f) {
        std::lock_guard<std::mutex> locker(lock);
        flock(fileno(f), LOCK_EX);
        ftruncate(fileno(f), 0);
        rewind(f);
        fprintf(f, "%s\n", str);
        fflush(f);
        flock(fileno(f), LOCK_UN);
    }
}

}
