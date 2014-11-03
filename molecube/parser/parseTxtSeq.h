#ifndef PARSE_TXT_SEQ_H
#define PARSE_TXT_SEQ_H

#include <istream>
#include <cgicc/Cgicc.h>

//parse URL-encoded pulse sequence in string
// should only be used for shorter sequence (< 100 pulses)
bool parseSeqURL(std::string& seq);

bool parseSeqCGI(cgicc::Cgicc& cgi);

//parse sequence in multipart format
// more efficient for long sequences because no decoding from URL format is needed
bool parseSeqMultiPart(std::istream& is, const std::string& line1);

std::string getQuote(const char* fname, const char* divider);

#endif //PARSE_TXT_SEQ_H
