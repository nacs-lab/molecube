/* Generic functions to write values to and read them from file.
 * Calls to these functions can manipulate file-mapped devices in Linux. */

#include <nacs-utils/utils.h>

#ifndef __LINUX_FILE_UTIL_H__
#define __LINUX_FILE_UTIL_H__

NACS_BEGIN_DECLS

int write_int_to_file(const char* fname, int val);
int get_int_from_file(const char* fname, int* val);
int write_str_to_file(const char* fname, const char* val);

//set status line for web interface in /tmp/progname.status
//progname is only needed at 1st call.  Later, supply 0.
void setProgramStatus(const char* progname, const char* str);

NACS_END_DECLS

#endif
