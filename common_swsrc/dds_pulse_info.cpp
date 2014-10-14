#include "string_func.h"


#include <math.h>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <iostream>
#include "dds_pulse_info.h"

using namespace std;
unsigned extract_flags(const std::string& s)
{
    unsigned flags = 0;
    extract_val<unsigned>(s, "flags=", &flags);

    return flags;
}

bool pulse_info::getEnabledFlag() const
{
    return !(flag_disabled & flags);
}

bool pulse_info::isEnabled() const
{
    return (t > 0.04) && !(flag_disabled & flags);
}

void pulse_info::setEnabledFlag(bool b)
{
    if(!b)
        flags |= flag_disabled;
    else
        flags &= ~flag_disabled;
}

ttl_pulse_info::ttl_pulse_info() : ttl(0)
{}

ttl_pulse_info::ttl_pulse_info(const std::string& s) : ttl(0)
{
    updateFromString(s.c_str());
}

void ttl_pulse_info::updateFromString(const char* s)
{
    sscanf(s, "%u,%lf,%X", &ttl, &t, &flags);
}

int ttl_pulse_info::to_string(char* s, size_t n) const
{
    //split time into us and ns components
    //Xilinx printf doesn't seem to work for doubles
    int us;
    unsigned ns;
    us = static_cast<int>(floor(fabs(t)));
    ns = static_cast<unsigned>(floor(0.5 + 1e3 * (fabs(t) - us)));
    if(t < 0)
        us *= -1;

    return snprintf(s, n, "%u,%d.%03u,%X", ttl, us, ns, flags);
}

bool operator==(const ttl_pulse_info& a, const ttl_pulse_info& b)
{
    return (a.t == b.t) && (a.ttl == b.ttl) && (a.flags == b.flags);
}

bool operator!=(const ttl_pulse_info& a, const ttl_pulse_info& b)
{
    return !(a==b);
}


const int dds_pulse_info::NO_SB = -999;
const unsigned dds_pulse_info::NO_AMP = 65535;

dds_pulse_info::dds_pulse_info() : iDDS(0), fOn(0), fOff(0), aOn(NO_AMP), aOff(0), sb(NO_SB)
{}

dds_pulse_info::dds_pulse_info(const std::string& s) : iDDS(0), fOn(0), fOff(0), aOn(NO_AMP), aOff(0), sb(NO_SB)
{
    updateFromString(s.c_str());
}

void dds_pulse_info::updateFromString(const char* s)
{
    if(strstr(s, "DDS") != 0) //old format
        sscanf(s, "DDS(%u),%lf,%lf,%lf,%d,%X", &iDDS, &t, &fOn, &fOff, &sb, &flags);
    else if (numOccurences(s, ",") == 5)
        sscanf(s, "%u,%lf,%lf,%lf,%d,%X", &iDDS, &t, &fOn, &fOff, &sb, &flags);
    else
        sscanf(s, "%u,%lf,%lf,%lf,%u,%u,%d,%X", &iDDS, &t, &fOn, &fOff, &aOn, &aOff, &sb, &flags);
}

int dds_pulse_info::to_string(char* s, size_t n) const
{
    //split time into us and ns components
    //Xilinx printf doesn't seem to work for doubles
    int us;
    unsigned ns;
    us = static_cast<int>(floor(fabs(t)));
    ns = static_cast<unsigned>(floor(0.5 + 1e3 * (fabs(t) - us)));
    if(t < 0)
        us *= -1;

    // return snprintf(s, n, "%u,%d.%03u,%d.%09u,%d.%09u,%d,%X", iDDS, us, ns, MHz[0], mHz[0], MHz[1], mHz[1], sb, flags);
    return snprintf(s, n, "%u,%d.%03u,%15.9f,%15.9f,%u,%u,%d,%X", iDDS, us, ns, fOn, fOff, aOn, aOff, sb, flags);

}

bool operator==(const dds_pulse_info& a, const dds_pulse_info& b)
{
    return (a.t == b.t) && (a.fOn == b.fOn) && (a.fOff == b.fOff) && (a.aOn == b.aOn) && (a.aOff == b.aOff) && (a.sb == b.sb) && (a.flags == b.flags);
}

bool operator!=(const dds_pulse_info& a, const dds_pulse_info& b)
{
    return !(a==b);
}

const int dds_pulse3_info::NO_SB = -999;

dds_pulse3_info::dds_pulse3_info() : iDDS1(0), iDDS2(0), iDDS3(0), f1On(0), f2On(0), f3On(0), f1Off(0), f2Off(0), f3Off(0), sb(NO_SB)
{}

dds_pulse3_info::dds_pulse3_info(const std::string& s) : iDDS1(0), iDDS2(0), iDDS3(0), f1On(0), f2On(0), f3On(0), f1Off(0), f2Off(0), f3Off(0), sb(NO_SB)
{
    updateFromString(s.c_str());
}

void dds_pulse3_info::updateFromString(const char* s)
{
    sscanf(s, "%u,%u,%u,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%d,%X", &iDDS1, &iDDS2, &iDDS3, &t, &f1On, &f2On, &f3On, &f1Off, &f2Off, &f3Off, &sb, &flags);
}

int dds_pulse3_info::to_string(char* s, size_t n) const
{
    //split time into us and ns components
    //Xilinx printf doesn't seem to work for doubles
    int us;
    unsigned ns;
    us = static_cast<int>(floor(fabs(t)));
    ns = static_cast<unsigned>(floor(0.5 + 1e3 * (fabs(t) - us)));
    if(t < 0)
        us *= -1;

    return snprintf(s, n, "%u,%u,%u,%d.%03u,%15.9f,%15.9f,%15.9f,%15.9f,%15.9f,%15.9f,%d,%X", iDDS1, iDDS2, iDDS3, us, ns, f1On, f2On, f3On, f1Off, f2Off, f3Off, sb, flags);

}

bool operator==(const dds_pulse3_info& a, const dds_pulse3_info& b)
{
    return (a.t == b.t) && (a.f1On == b.f1On) && (a.f2On == b.f2On) && (a.f3On == b.f3On) && (a.f1Off == b.f1Off) && (a.f2Off == b.f2Off) && (a.f3Off == b.f3Off) && (a.sb == b.sb) && (a.flags == b.flags);
}

bool operator!=(const dds_pulse3_info& a, const dds_pulse3_info& b)
{
    return !(a==b);
}

/******** Some shared functions for FPGA and PC programs that should really go somewhere else ********/
//TODO

//! returns number of occurences of string s2 in s1
unsigned numOccurences(const std::string& s1, const std::string& s2)
{
    unsigned n = 0;
    size_t i = 0;

    while(i != string::npos) {
        i = s1.find(s2, i);

        if(i != string::npos) {
            n++;
            i++;
        }
    }

    return n;
}
