#ifndef PARSE_MISC_H
#define PARSE_MISC_H

#include <nacs-utils/fd_utils.h>
#include <nacs-pulser/pulser.h>

#include <verbosity.h>

#include <string>
#include <vector>
#include <cgicc/Cgicc.h>

namespace NaCs {

void printPlainResponseHeader();
void printJSONResponseHeader();

bool parseQueryCGI(NaCs::Pulser::Pulser &pulser, cgicc::Cgicc& cgi,
                   const verbosity &reply);

unsigned getUnsignedParam(const std::string &seq, const std::string &name,
                          unsigned defaultVal);

unsigned getHexParam(const std::string &seq, const std::string &name,
                     unsigned defaultVal);

double getDoubleParam(const std::string &seq, const std::string &name,
                      double defaultVal);

bool getCheckboxParam(const std::string &seq, const std::string &name,
                      bool defaultVal);

std::string getStringParam(const std::string &seq, const std::string &name,
                           const std::string &defaultVal,
                           const std::string &sep);

template<class C> const C &getParamCGI(cgicc::Cgicc &cgi,
                                       const std::string &name,
                                       C defaultVal);

bool getCheckboxParamCGI(cgicc::Cgicc &cgi, const std::string &name,
                         bool defaultVal);

unsigned getUnsignedParamCGI(cgicc::Cgicc &cgi, const std::string &name,
                             unsigned defaultVal);

std::string getStringParamCGI(cgicc::Cgicc &cgi, const std::string &name,
                              const std::string &defaultVal);

// lock file. Set the lock when performing PULSER operations that
// should not be interrupted by other PULSER operations.
extern FLock g_fPulserLock;
extern std::vector<unsigned> active_dds; // all DDS that are available

}

#endif //PARSE_MISC_H
