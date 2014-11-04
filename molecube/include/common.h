#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

extern void *pulser;
extern bool g_debug_spi;
extern bool g_stop_curr_seq;
extern unsigned gDebugLevel; //control printf debugging
extern FILE *gLog;

#ifdef ALUMINIZER_SIM
void usleep(int);
void sim_usleep(int);
#endif

//#include "config_local.h"


#endif //COMMON_H
