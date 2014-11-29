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
    __attribute__((format(printf, 2, 3)))
    int printf(const char *format, ...) const;
private:
    std::ostream *m_pos;
};

}

#endif
