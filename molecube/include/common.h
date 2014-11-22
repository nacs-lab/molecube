#ifndef COMMON_H
#define COMMON_H

#ifndef __cplusplus
#  include <stdbool.h>
#endif

#include <stdio.h>
#include <stddef.h>

extern void *pulser;
extern bool g_debug_spi;
extern bool g_stop_curr_seq;
extern unsigned gDebugLevel; //control printf debugging
extern FILE *gLog;

#endif //COMMON_H
