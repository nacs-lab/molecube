#ifndef PARSE_MISC_H
#define PARSE_MISC_H

#include <nacs-old-pulser/pulser.h>

#include <verbosity.h>

#include <string>
#include <cgicc/Cgicc.h>

#include <ostream>

namespace NaCs {

void printPlainResponseHeader(std::ostream&);

bool parseQueryCGI(Pulser::OldPulser &pulser, cgicc::Cgicc& cgi,
                   const verbosity &reply);

unsigned getUnsignedParam(const std::string &seq, const std::string &name,
                          unsigned defaultVal);

bool getCheckboxParam(const std::string &seq, const std::string &name,
                      bool defaultVal);

bool getCheckboxParamCGI(cgicc::Cgicc &cgi, const std::string &name,
                         bool defaultVal);

unsigned getUnsignedParamCGI(cgicc::Cgicc &cgi, const std::string &name,
                             unsigned defaultVal);

std::string getStringParamCGI(cgicc::Cgicc &cgi, const std::string &name,
                              const std::string &defaultVal);

}

#endif
