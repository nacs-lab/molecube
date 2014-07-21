#ifndef PARSE_MULTI_PART_H
#define PARSE_MULTI_PART_H

#include <map>
#include <string>
#include <istream>

typedef std::map<std::string, std::string> txtmap_t;

// parse multipart http POST from input stream
bool parseMultiPart(std::istream& is, const std::string& line1, txtmap_t& params);

#endif //PARSE_MULTI_PART_H
