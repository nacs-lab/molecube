#ifndef PARSE_MISC_H
#define PARSE_MISC_H

#include <string>
#include <fcgio.h>
#include <cgicc/Cgicc.h>

#include <ostream>

namespace NaCs {
namespace Pulser {
class Controller;
}

void printPlainResponseHeader(std::ostream&);
bool parseQueryCGI(Pulser::Controller &ctrl, cgicc::Cgicc &cgi,
                   std::ostream &reply, FCGX_Request *request);

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
