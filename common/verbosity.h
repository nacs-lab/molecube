#ifndef VERBOSITY_H
#define VERBOSITY_H

#include <ostream>

namespace NaCs {

// Flexible logging via C++ ostream and printf
// if ostream *pos != 0, any printf calls are echoed to *pos
class verbosity {
    verbosity(const verbosity&) = delete;
public:
    verbosity(std::ostream *pos) : m_pos(pos) {}
    int printf(const char *format, ...);
private:
    std::ostream *m_pos;
};

extern verbosity gvSTDOUT; //printf goes to log and stdout

}

#endif //VERBOSITY_H
