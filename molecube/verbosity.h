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
    const verbosity &printf(const char *format, ...) const;
    operator std::ostream&() const
    {
        return *m_pos;
    }
    template<typename T>
    decltype(auto) operator<<(T &&v) const
    {
        return *m_pos << std::forward<T>(v);
    }
private:
    std::ostream *m_pos;
};

}

#endif
