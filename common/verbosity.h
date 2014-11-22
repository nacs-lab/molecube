#ifndef VERBOSITY_H
#define VERBOSITY_H

#include <stdio.h>
#include <ostream>

// Flexible logging via C++ ostream and printf
// if ostream* pos != 0, any printf calls are echoed to *pos
// if FILE* f != 0 , any printf calls are echoed to f
class verbosity {
public:
    verbosity(std::ostream* pos, FILE* f) : pos(pos), f(f) {}
    verbosity(const verbosity& v) : pos(v.pos), f(v.f) {}
    int printf(const char* format, ...);
private:
    std::ostream* pos;
    FILE* f;
};

extern verbosity gvSTDOUT; //printf goes to log and stdout
extern verbosity gvLog;    //printf goes to log only

#endif //VERBOSITY_H
