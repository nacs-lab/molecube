#include "ttl_pulse.h"
#include <stdio.h>
#include "fpga.h"

unsigned g_tSequence = 0; //accumulated sequence duration in PULSER units
