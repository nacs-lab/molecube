#ifndef PARSE_TXT_SEQ_H
#define PARSE_TXT_SEQ_H

#include <fcgio.h>
#include <cgicc/Cgicc.h>
#include <functional>
#include <ostream>

namespace NaCs {
namespace Pulser {
class Controller;
}

// parse URL-encoded pulse sequence in string
// should only be used for shorter sequence (< 100 pulses)
bool parseSeqURL(Pulser::Controller &ctrl, std::string &seq, std::ostream &reply);
bool parseSeqCGI(Pulser::Controller &ctrl, cgicc::Cgicc &cgi,
                 std::ostream &reply, FCGX_Request *request);

void handleRunByteCode(Pulser::Controller &ctrl, uint64_t seq_len_ns,
                       const uint8_t *code, size_t code_len,
                       const std::function<void()> &send_reply, uint32_t ttl_mask);

}

#endif
