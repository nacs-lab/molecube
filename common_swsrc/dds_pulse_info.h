#ifndef DDS_PULSE_INFO_H
#define DDS_PULSE_INFO_H

#include <string>

unsigned extract_flags(const std::string& s);


class pulse_info {
public:
    pulse_info() : t(0), flags(0) {}

    bool getEnabledFlag() const;
    bool isEnabled() const;
    void setEnabledFlag(bool b);

    double t;
    unsigned flags;

    static const unsigned flag_disabled		= 0x00000001;
    static const unsigned flag_scan			= 0x00000002;
    static const unsigned flag_ramsey		= 0x00000004;
    static const unsigned flag_composite   = 0x00000008;

};

class ttl_pulse_info : public pulse_info {
public:
    ttl_pulse_info();
    ttl_pulse_info(const std::string& s);

    void updateFromString(const char* s);

    int to_string(char* s, size_t n) const;

    unsigned ttl;
};

bool operator==(const ttl_pulse_info&, const ttl_pulse_info&);
bool operator!=(const ttl_pulse_info&, const ttl_pulse_info&);

class dds_pulse_info : public pulse_info {
public:
    dds_pulse_info();
    dds_pulse_info(const std::string& s);

    void updateFromString(const char* s);

    int to_string(char* s, size_t n) const;

    bool hasAMP() const {
        return aOn != NO_AMP;
    }
    bool hasSB() const {
        return sb != NO_SB;
    }

    const static int NO_SB;
    const static unsigned NO_AMP;

    unsigned iDDS;
    double fOn, fOff;
    unsigned aOn, aOff;
    int sb;
};

bool operator==(const dds_pulse_info&, const dds_pulse_info&);
bool operator!=(const dds_pulse_info&, const dds_pulse_info&);

class dds_pulse3_info : public pulse_info {
public:
    dds_pulse3_info();
    dds_pulse3_info(const std::string& s);

    void updateFromString(const char* s);

    int to_string(char* s, size_t n) const;

    bool hasSB() const {
        return sb != NO_SB;
    }

    const static int NO_SB;

    unsigned iDDS1, iDDS2, iDDS3;
    double f1On, f2On, f3On, f1Off, f2Off, f3Off;
    int sb;
};

bool operator==(const dds_pulse3_info&, const dds_pulse3_info&);
bool operator!=(const dds_pulse3_info&, const dds_pulse3_info&);


/******** Some shared functions for FPGA and PC programs that should really go somewhere else ********/
//TODO

//! returns number of occurences of string s2 in s1
unsigned numOccurences(const std::string& s1, const std::string& s2);

#endif // DDS_PULSE_INFO_H

