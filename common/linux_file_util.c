#include "linux_file_util.h"

#include <sys/file.h>
#include <unistd.h>

/* Generic functions to write values to and read them from file.
 * Calls to these functions can manipulate file-mapped devices in Linux. */

int
write_int_to_file(const char* fname, int val)
{
    FILE *f = fopen(fname, "w");
    if (f) {
        fprintf(f, "%d", val);
        fclose(f);
        return 0;
    } else {
        printf("Unable to write value %d to file %s\n", val, fname);
    }
    return -1;
}

int
get_int_from_file(const char *fname, int *val)
{
    FILE *f = fopen(fname, "r");

    if (f) {
        fscanf(f, "%d", val);
        fclose(f);
        return 0;
    } else {
        printf("Unable to get value from file %s\n", fname);
    }
    return -1;
}

int
write_str_to_file(const char *fname, const char *val)
{
    FILE *f = fopen(fname, "w");
    if (f) {
        fprintf(f, "%s", val);
        fclose(f);
        return 0;
    } else {
        printf("Unable to write value %s to file %s\n", val, fname);
    }
    return -1;
}

// Record status string in file.
// Keep the file open, and for each status update, lock it exclusively,
// then rewrite and unlock it.  Readers should acquire a lock,
// to prevent reading of partially written or empty files.
void
setProgramStatus(const char *progname, const char *str)
{
    static FILE *f = 0;

    if (f == 0) {
        char buff[256];

        if (progname) {
            snprintf(buff, 256, "/tmp/%s.status", progname);
        } else {
            snprintf(buff, 256, "/tmp/unnamedprog.status");
        }
        f = fopen(buff, "w");
    }

    if (f) {
        flock(fileno(f), LOCK_EX);
        ftruncate(fileno(f), 0);
        rewind(f);
        fprintf(f, "%s\n", str);
        fflush(f);
        flock(fileno(f), LOCK_UN);
    }
}
