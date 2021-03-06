/* save and load string / string maps
 * author: trosen
 *
 * File format:
 * {name1} = {value1};
 * {name2} = {value2};
 * ...
 *
 **/

#include "saveloadmap.h"

#include <nacs-utils/log.h>

#include <sstream>
#include <iostream>
#include <iomanip>

void saveMap(const txtmap_t &params, const std::string &fname)
{
    std::ofstream os(fname.c_str());

    for (txtmap_t::const_iterator i = params.begin(); i != params.end(); i++) {
        //printf("%s = %s\n", i->first.c_str(), i->second.c_str());

        if (i->second.length() > 0) {
            os << "{" << i->first.c_str() << "} = {";
            os << i->second.c_str() << "}";
            os << ";" << std::endl;
        }
    }
}

static bool
processLine(const std::string &sLine, std::string &sName, std::string &sValue)
{
    std::string sWhite("\r\n ");

    //find left/right brace surrounding the name
    size_t lbN = sLine.find('{');
    size_t rbN = sLine.find('}');

    if (lbN == std::string::npos || rbN == std::string::npos)
        return false; //can't process

    //copy the bracketed part into sName
    sName = std::string(sLine, lbN + 1, rbN - lbN - 1);

    //find left/right brace surrounding the value
    size_t lbV = sLine.find('{', rbN + 1);
    size_t rbV = sLine.rfind('}');

    if (lbV == std::string::npos || rbV == std::string::npos || lbV > rbV)
        return false; //can't process

    sValue = std::string(sLine, lbV + 1, rbV - lbV - 1);

    return true;
}

void loadMap(txtmap_t &m, const std::string &fname)
{
    std::ifstream is(fname.c_str());

    if (!is.good()) {
        NaCs::Log::error("failed to open parameters file: %s\n", fname.c_str());
    } else {
        NaCs::Log::log("opened parameters file: %s\n", fname.c_str());
    }

    //loop through the file
    while (is.good() && !is.eof()) {
        std::string sLine("");

        //read until ';'
        getline(is, sLine, ';');

        std::string sName, sValue;

        if (!processLine(sLine, sName, sValue))
            continue;

        //add the pair sName, sValue to the map
        m[sName] = sValue;
    }
}

void dumpMapHTML(const txtmap_t &m, std::ostream &os)
{
    bool notFirst = false;
    for (txtmap_t::const_iterator i = m.begin(); i != m.end(); i++) {
        //printf("%s = %s\n", i->first.c_str(), i->second.c_str());

        if (i->second.length() > 0) {
            std::string name = i->first;
            std::string value = i->second;

            html2txt(name, -1);
            html2txt(value, -1);

            if (notFirst)
                os << "&";

            os << name << "=" << value;
            notFirst = true;
        }
    }
}

//url_encode is copied from stackexchange:
// http://stackoverflow.com/questions/154536/encode-decode-urls-in-c
static std::string
url_encode(const std::string &value)
{
    std::ostringstream escaped;
    escaped.fill('0'); //pad numbers w/ 0 to reach width
    escaped << std::hex; //numbers will be in hex format

    for (std::string::const_iterator i = value.begin(), n = value.end();
         i != n; ++i) {

        std::string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << '%' << std::setw(2) << int((unsigned char) c);
    }

    return escaped.str();
}

// url_decode is copied from stackexchange:
// http://stackoverflow.com/questions/154536/encode-decode-urls-in-c
static std::string
url_decode(std::string &SRC)
{
    std::string ret;
    char ch;
    for (unsigned i = 0;i < SRC.length();i++) {
        if (SRC[i] == '+') {
            ret += ' ';
        } else {
            if (SRC[i] == '%') {
                int ii;
                sscanf(SRC.substr(i + 1, 2).c_str(), "%x", &ii);
                ch = static_cast<char>(ii);
                ret += ch;
                i = i + 2;
            } else {
                ret += SRC[i];
            }
        }
    }
    return ret;
}

// convert html to text or visa-versa
// dir=1 is html to text
// dir=-1 is text to html
// libcgicc has standard functions for this and might be a better choice
void
html2txt(std::string &seq, int dir)
{
    //see: http://www.w3schools.com/tags/ref_urlencode.asp

    if (dir == 1) {
        seq = url_decode(seq);
    } else {
        seq = url_encode(seq);
    }
}
