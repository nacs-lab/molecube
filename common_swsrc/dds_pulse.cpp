#include "ttl_pulse.h"
#include "dds_pulse.h"

unsigned FTW2Hz(unsigned ftw, double fClock)
{
	return (unsigned)floor(ftw * fClock * pow(2., -32) + 0.5);
}

int FTW2HzI(int ftw)
{
	return (int)floor(ftw * 1.0e9 * pow(2., -32) + 0.5);
}

double FTW2HzD(unsigned ftw, double fClock)
{
	return ftw * fClock * pow(2., -32);
}

unsigned int Hz2FTW(double f, double fClock)
{
	return static_cast<unsigned int>(floor(0.5 + (f * pow(2., 32.) / fClock)));
}

int Hz2FTWI(double f)
{
	return static_cast<int>(floor(0.5 + (f * pow(2., 32.) / 1e9)));
}

unsigned int MHz2FTW(double f, double fClock)
{
	return Hz2FTW(f * 1e6, fClock);
}

int MHz2FTWI(double f)
{
	return Hz2FTWI(f * 1e6);
}

double dds_clk(int iDDS)
{
	return iDDS < NDDS ? 1e9 : AD9914_CLK;
}

void print_pulse_info(unsigned iDDS, unsigned ftwOn, unsigned ftwOff, unsigned t, unsigned ttl, const char* info)
{
	double fClock = 1e9;
	
	if(iDDS >= NDDS)
		fClock = 3.5e9;
		
	if (info)
		fprintf(gLog, "%s DDS(%u) fOn = %9u, fOff = %9u, t = %8.2f us TTL=%08X, (%s)\n", 
      DDS_name(iDDS), iDDS, FTW2Hz(ftwOn, fClock), FTW2Hz(ftwOff, fClock), 0.01 * (double)t, ttl, info);
	else
		fprintf(gLog, "%s DDS(%u) fOn = %9u, fOff = %9u, t = %8.2f us TTL=%08X\n", 
      DDS_name(iDDS), iDDS, FTW2Hz(ftwOn, fClock), FTW2Hz(ftwOff, fClock), 0.01 * (double)t, ttl);

}

void print_pulse_info(unsigned iDDS, unsigned ftwOn, unsigned ftwOff, unsigned aOn, unsigned aOff, unsigned t, unsigned ttl, const char* info)
{
	double fClock = 1e9;
	
	if(iDDS >= NDDS)
		fClock = 3.5e9;
		
	if (info)
		fprintf(gLog, "%s DDS(%u) fOn = %9u, fOff = %9u, aOn = %u, aOff = %u, t = %8.2f us TTL=%08X, (%s)\n", 
      DDS_name(iDDS), iDDS, FTW2Hz(ftwOn, fClock), FTW2Hz(ftwOff, fClock), aOn, aOff, 0.01 * (double)t, ttl, info);
	else
		fprintf(gLog, "%s DDS(%u) fOn = %9u, fOff = %9u, aOn = %u, aOff = %u, t = %8.2f us TTL=%08X\n", 
      DDS_name(iDDS), iDDS, FTW2Hz(ftwOn, fClock), FTW2Hz(ftwOff, fClock), aOn, aOff, 0.01 * (double)t, ttl);
}
