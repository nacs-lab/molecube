#ifndef TTL_PULSE_H_
#define TTL_PULSE_H_

#include <stdio.h>
//#include "common.h"
#include "fpga.h"

#ifdef WIN32
#define scalb _scalb
#define snprintf _snprintf
#endif

extern bool bDebugPulses;
extern FILE* gLog;

#define TIME_UNIT    (1e-8)

#define TTL_NOTHING     (0)

#ifdef CONFIG_AL

#ifndef PLATFORM_ZYNQ
//For 1H116 lab add 12 to switch label in GUI to obtain left-shift value of pulse.  
// #define TTL_GUI_N  (1 << (GUI_N+12))
// GUI_N is the label on the Switch panel GUI
#define N_SHIFT_SWITCHES 12
#else
#define N_SHIFT_SWITCHES 0
#endif

#define TTL_SHUTTER     (1 << 9 << N_SHIFT_SWITCHES)
#define TTL_RAMAN_90    ((1 << 4 << N_SHIFT_SWITCHES) | (1 << 5 << N_SHIFT_SWITCHES))
#define TTL_RAMAN_CO    ((1 << 4 << N_SHIFT_SWITCHES) | (1 << 3 << N_SHIFT_SWITCHES))
#define TTL_START_EXP   (1 << N_SHIFT_SWITCHES)
#define TTL_3P1_LO     (1 << 8 << N_SHIFT_SWITCHES)
#define TTL_3P1_V     ((1 << 16 << N_SHIFT_SWITCHES) | (1 << 19 << N_SHIFT_SWITCHES))
#define TTL_3P1_PI     ((1 << 16 << N_SHIFT_SWITCHES) | (1 << 18 << N_SHIFT_SWITCHES))
#define TTL_3P1_SIGMA   ((1 << 16 << N_SHIFT_SWITCHES) | (1 << 17 << N_SHIFT_SWITCHES))
#define TTL_3P0          (1 << 14 << N_SHIFT_SWITCHES)
#define TTL_RF_HEAT      (1 << 15 << N_SHIFT_SWITCHES)
#define TTL_HF_RF       (1 << 11 << N_SHIFT_SWITCHES)
#define TTL_REPUMP      (1 << 13 << N_SHIFT_SWITCHES)
#define TTL_PRECOOL     (1 << 2 << N_SHIFT_SWITCHES)
#define TTL_DETECT      (1 << 1 << N_SHIFT_SWITCHES)
#define TTL_BLUE_RED_DETECT_MON  (1 << 12 << N_SHIFT_SWITCHES) //Turn on repump along with the Blue-red detection pulses CWC 03092011
#define TTL_ZEEMAN (1 << 4 << N_SHIFT_SWITCHES) //TTL pulse controlling Agilent 33521A arbitrary waveform generator for B field modulation CWC 03232011
#define TTL_MG_SHUTTER_CLOSE (1 << 5 << N_SHIFT_SWITCHES)
#define TTL_MG_SHUTTER_OPEN  (0)

// #define TTL_MOTOR_0      (1 << 6 << N_SHIFT_SWITCHES)
// #define TTL_MOTOR_1      (1 << 7 << N_SHIFT_SWITCHES)

//pulses for new 24Mg+
#define TTL_ABLATION_TRIG (1 << 11 << N_SHIFT_SWITCHES)
#define TTL_266_TRIG (1 << 12 << N_SHIFT_SWITCHES)
#define TTL_TRAP_RF_OFF (1 << 10 << N_SHIFT_SWITCHES)

inline const char* TTL_name(unsigned ttl)
{
  switch (ttl)
  {
  case TTL_NOTHING:
    return "  Nothing   ";
  case TTL_START_EXP:
    return "Start exp   ";
  case TTL_SHUTTER:
    return "  Shutter   ";
  case TTL_RAMAN_90:
    return " Raman 90   ";
  case TTL_RAMAN_CO:
    return " Raman Co   ";
  case TTL_REPUMP:
    return "   Repump   ";
  case TTL_3P1_PI:
    return "3P1    pi (hi)";
  case TTL_3P1_V:
    return "3P1     V (hi)";
  case TTL_3P1_SIGMA:
    return "3P1 sigma (hi)";
  case TTL_3P1_PI | TTL_3P1_LO:
    return "3P1    pi (lo)";
  case TTL_3P1_V | TTL_3P1_LO:
    return "3P1     V (lo)";
  case TTL_3P1_SIGMA | TTL_3P1_LO:
    return "3P1 sigma (lo)";
  case TTL_3P0:
    return "3P0         ";
  case TTL_RF_HEAT:
    return "RF Heat     ";
  case TTL_DETECT:
    return " Detect    ";
  case TTL_PRECOOL:
    return "Precool    ";
  case TTL_ABLATION_TRIG:
    return "Ablation   ";
  case TTL_266_TRIG:
    return "266nm pulse";
  default:
    return "  unknown   ";
  }
}

#endif

#ifdef CONFIG_HG
#define N_SHIFT_SWITCHES 12

#define TTL_NOTHING     (0)
#define TTL_START_EXP   (1 << 12)
#define TTL_DETECT_MON  (1 << 13)
#define TTL_PRECOOL_MON (1 << 14)

#define TTL_MOTOR_0      (1 << 18)
#define TTL_MOTOR_1      (1 << 19)

inline const char* TTL_name(unsigned ttl)
{
  switch (ttl)
  {
    case TTL_NOTHING: return "  Nothing";
    case TTL_START_EXP: return "Start exp";
    case TTL_DETECT_MON: return " Detect_M";
    case TTL_PRECOOL_MON: return "Precool_M";
    default: return "  unknown";
  }
}

#endif

#ifdef CONFIG_SHB
#define N_SHIFT_SWITCHES 0

#define TTL_NOTHING      (0)
#define TTL_START_EXP    (1 << 12)

#define TTL_RF_SWITCH_1  (1 << 0 )
#define TTL_INT_HOLD_1   (1 << 1 )
#define TTL_DATA_SYNC    (1 << 2 )
#define TTL_RF_SWITCH_2  (1 << 3 )
#define TTL_INT_HOLD_2   (1 << 4 )
#define TTL_RESET_ADC    (1 << 5 )
#define TTL_SAMPLE       (1 << 6 )
#define TTL_SHUTTER      (1 << 8 )
#define TTL_INT_HOLD_ALL (TTL_INT_HOLD_1 | TTL_INT_HOLD_2)

inline const char* TTL_name(unsigned ttl)
{
  switch (ttl)
  {
    case TTL_NOTHING: return "  Nothing";
    case TTL_START_EXP: return "Start exp";
    case TTL_RF_SWITCH_1: return "RFswitch1";
    case TTL_INT_HOLD_1: return " IntHold1";
    case TTL_DATA_SYNC: return " DataSync";
    case TTL_RF_SWITCH_2: return "RFswitch2";
    case TTL_INT_HOLD_2: return " IntHold2";
    case TTL_RESET_ADC: return " ResetADC";
    case TTL_SAMPLE: return "   Sample";
    case TTL_SHUTTER: return "  Shutter";
    case TTL_INT_HOLD_ALL: return "  IntHold";

    default: return "  unknown";
  }
}

#endif

#ifdef CONFIG_LMS

#define TTL_START_EXP   (1 << 12)
inline const char* TTL_name(unsigned ttl)
{
  switch (ttl)
  {
    default: return "  unknown";
  }
}
#endif

#ifdef CONFIG_BB

#define TTL_START_EXP   (1 << 12)
inline const char* TTL_name(unsigned ttl)
{
  switch (ttl)
  {
    default: return "  unknown";
  }
}

#endif

inline unsigned int us2TW(double t)
{
  return static_cast<unsigned int>(t * 100);
}

inline unsigned int ms2TW(double t)
{
  return us2TW(t * 1e3);
}

inline void print_pulse_info(unsigned t, unsigned ttl, const char* info = 0)
{

  if (info)
    fprintf(gLog, "%12s %36s t = %8.2f us TTL=%08X (%s)\n", TTL_name(ttl), "",
        0.01 * (double) t, ttl, info);
  else
    fprintf(gLog, "%12s %36s t = %8.2f us TTL=%08X\n", TTL_name(ttl), "",
        0.01 * (double) t, ttl);
}

//make an RF pulse of specified frequency and duration
inline void TTL_pulse(unsigned t, unsigned ttl = 0)
{
  if (t > 4)
  {
    PULSER_pulse(pulser, t, 0, ttl);

    if (bDebugPulses)
      print_pulse_info(t, ttl);
  }
}

#endif /*TTL_PULSE_H_*/

