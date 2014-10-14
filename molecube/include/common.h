#ifndef COMMON_H
#define COMMON_H

extern void* pulser;
extern bool g_debug_spi;
extern unsigned gDebugLevel; //control printf debugging

#ifdef ALUMINIZER_SIM
void usleep(int);
void sim_usleep(int);
#endif

//#include "config_local.h"


#endif //COMMON_H
