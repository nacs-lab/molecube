#ifndef PARSE_MISC_H
#define PARSE_MISC_H

#include <nacs-utils/fd_utils.h>

#include <string>
#include <vector>
#include <cgicc/Cgicc.h>

//thrown when the token does not exist
class noSuchToken {
public:
    noSuchToken(const std::string& token) : token(token) {}

    std::string token;
};

void printPlainResponseHeader();
void printJSONResponseHeader();

bool parseQueryCGI(cgicc::Cgicc& cgi);

unsigned getUnsignedParam(const std::string& seq, const std::string& name,
                          unsigned defaultVal);

unsigned getHexParam(const std::string& seq, const std::string& name,
                     unsigned defaultVal);

double getDoubleParam(const std::string& seq, const std::string& name,
                      double defaultVal);

bool getCheckboxParam(const std::string& seq, const std::string& name,
                      bool defaultVal);

std::string getStringParam(const std::string& seq, const std::string& name,
                           const std::string& defaultVal, const std::string& sep);

template<class C> const C& getParamCGI(cgicc::Cgicc& cgi, const std::string& name,
                                       C defaultVal);

bool getCheckboxParamCGI(cgicc::Cgicc& cgi, const std::string& name,
                         bool defaultVal);

unsigned getUnsignedParamCGI(cgicc::Cgicc& cgi, const std::string& name,
                             unsigned defaultVal);

std::string getStringParamCGI(cgicc::Cgicc& cgi, const std::string& name,
                              const std::string& defaultVal);

// lock file. Set the lock when performing PULSER operations that
// should not be interrupted by other PULSER operations.
extern NaCs::FLock g_fPulserLock;
extern std::vector<unsigned> active_dds; // all DDS that are available

#endif //PARSE_MISC_H
