#ifndef PARSE_TXT_SEQ_H
#define PARSE_TXT_SEQ_H

#include <verbosity.h>

#include <istream>
#include <cgicc/Cgicc.h>

namespace NaCs {
namespace Pulser {
class Controller;
}

//parse URL-encoded pulse sequence in string
// should only be used for shorter sequence (< 100 pulses)
bool parseSeqURL(Pulser::Controller &ctrl, std::string &seq,
                 const verbosity &reply);
bool parseSeqCGI(Pulser::Controller &ctrl, cgicc::Cgicc &cgi,
                 const verbosity &reply);

// parse sequence in multipart format
// more efficient for long sequences because no decoding
// from URL format is needed
// bool parseSeqMultiPart(std::istream &is, const std::string &line1);

std::string getQuote(const char *fname, const char *divider);

}

#endif
