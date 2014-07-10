#include <stdio.h>

/* Generic functions to write values to and read them from file.
 * Calls to these functions can manipulate file-mapped devices in Linux. */

int write_int_to_file(const char* fname, int val)
{
    FILE* f = fopen(fname, "w");
    if(f) {
        fprintf(f, "%d", val);
        fclose(f);
        return 0;
    } else
        printf("Unable to write value %d to file %s\n", val, fname);

    return -1;
}

int get_int_from_file(const char* fname, int* val)
{
    FILE* f = fopen(fname, "r");

    if(f) {
        fscanf(f, "%d", val);
        fclose(f);
        return 0;
    } else
        printf("Unable to get value from file %s\n", fname);

    return -1;
}

int write_str_to_file(const char* fname, const char* val)
{
    FILE* f = fopen(fname, "w");
    if(f) {
        fprintf(f, "%s", val);
        fclose(f);
        return 0;
    } else
        printf("Unable to write value %s to file %s\n", val, fname);

    return -1;
}
