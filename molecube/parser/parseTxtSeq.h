#ifndef PARSE_TXT_SEQ_H
#define PARSE_TXT_SEQ_H

#include <verbosity.h>

#include <istream>
#include <fcgio.h>
#include <fcgi_config.h>
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
                 const verbosity &reply, FCGX_Request *request);

// parse sequence in multipart format
// more efficient for long sequences because no decoding
// from URL format is needed
// bool parseSeqMultiPart(std::istream &is, const std::string &line1);

std::string getQuote(const char *fname, const char *divider);

void handleRunByteCode(Pulser::Controller &ctrl, uint64_t seq_len_ns,
                       const uint8_t *code, size_t code_len,
                       const std::function<void()> &send_reply);

}

#endif
