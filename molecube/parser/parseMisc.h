#ifndef PARSE_MISC_H
#define PARSE_MISC_H

#include <string>
#include <vector>

//thrown when the tokewn does not exist
class noSuchToken {
public:
    noSuchToken(const std::string& token) : token(token) {}

    std::string token;
};

void printPlainResponseHeader();
void printJSONResponseHeader();

bool parseQuery(std::string& doc);
bool parseTTL(std::string& ttl);

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

//lock file.  Set the lock when performing PULSER operations that
//should not be interrupted by other PULSER operations.
//Use flocker stack objects to automatically release the lock
//as the stack variable goes out of scope.
extern FILE* g_fPulserLock;

//acquire exclusive file lock at creation and release at destruction
class flocker {
public:
    flocker(FILE* f);
    ~flocker();
    
    int fd;
};

extern FILE* gLog; //log file
  
extern std::vector<unsigned> active_dds; // all DDS that are available
#endif //PARSE_MISC_H
