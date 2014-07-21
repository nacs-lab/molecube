/* save and load string / string maps
 * author: trosen
 *
 * File format:
 * {name1} = {value1};
 * {name2} = {value2};
 * ...
 *
 * */

#include <sstream>
#include <iostream>
#include <iomanip>

#include "saveloadmap.h"

extern FILE* gLog;

using namespace std;

void saveMap(const txtmap_t& params, const std::string& fname)
{
    ofstream os(fname.c_str());

    for (txtmap_t::const_iterator i = params.begin(); i != params.end(); i++) {
        //printf("%s = %s\n", i->first.c_str(), i->second.c_str());

        if (i->second.length() > 0) {
            os << "{" << i->first.c_str() << "} = {";
            os << i->second.c_str() << "}";
            os << ";" << endl;
        }
    }
}

bool processLine(const std::string& sLine, std::string& sName, std::string& sValue)
{
    string sWhite("\r\n ");

    //find left/right brace surrounding the name
    size_t lbN = sLine.find('{');
    size_t rbN = sLine.find('}');

    if (lbN == string::npos || rbN == string::npos)
        return false; //can't process

    //copy the bracketed part into sName
    sName = string(sLine, lbN + 1, rbN - lbN - 1);

    //find left/right brace surrounding the value
    size_t lbV = sLine.find('{', rbN + 1);
    size_t rbV = sLine.rfind('}');

    if (lbV == string::npos || rbV == string::npos || lbV > rbV)
        return false; //can't process

    sValue = string(sLine, lbV + 1, rbV - lbV - 1);

    return true;
}

void loadMap(txtmap_t& m, const std::string& fname)
{
    ifstream is(fname.c_str());

    if(! is.good() )
        fprintf(gLog, "failed to open parameters file: %s\n", fname.c_str());
    else
        fprintf(gLog, "opened parameters file: %s\n", fname.c_str());
        
    //loop through the file
    while (is.good() && !is.eof()) {
        string sLine("");

        //read until ';'
        getline(is, sLine, ';');

        string sName, sValue;

        if (!processLine(sLine, sName, sValue))
            continue;

        //add the pair sName, sValue to the map
        m[sName] = sValue;
    }
}

//merge maps.  copy all entries from new to old
void mergeMaps(txtmap_t& mOld, const txtmap_t& mNew)
{
    for (txtmap_t::const_iterator i = mNew.begin(); i != mNew.end(); i++)
        mOld[ i->first ] = i->second;
}

void dumpMapHTML(const txtmap_t& m, ostream& os)
{
    bool notFirst = false;
    for (txtmap_t::const_iterator i = m.begin(); i != m.end(); i++) {
        //printf("%s = %s\n", i->first.c_str(), i->second.c_str());

        if (i->second.length() > 0) {
            string name = i->first;
            string value = i->second;

            html2txt(name, -1);
            html2txt(value, -1);

            if(notFirst)
                os << "&";

            os << name << "=" << value;
            notFirst = true;
        }
    }
}

void dumpMap(const txtmap_t& m, FILE* f)
{
    bool notFirst = false;
    for (txtmap_t::const_iterator i = m.begin(); i != m.end(); i++) {
        fprintf(f, "%s = %s\n", i->first.c_str(), i->second.c_str());
    }
}

//replace next string in str matching from with to.  start at next
//return next positon after replacment or string::npos if no match
size_t replace(std::string& str, const std::string& from, const std::string& to, size_t next)
{
    size_t start_pos = str.find(from, next);

    if(start_pos == std::string::npos)
        return start_pos;

    str.replace(start_pos, from.length(), to);
    return start_pos+to.length();
}

void replaceAll(std::string& str, const std::string& from, const std::string& to, int dir)
{
    size_t next = 0;

    if(dir == 1)
        while((next = replace(str, from, to, next)) != std::string::npos) {}
    else
        while((next = replace(str, to, from, next)) != std::string::npos) {}
}

void replaceAllR(std::string& str, const std::string& from, const std::string& to)
{
    while(replace(str, to, from, 0)) {}
}

//url_encode is copied from stackexchange: 
// http://stackoverflow.com/questions/154536/encode-decode-urls-in-c
string url_encode(const string &value) 
{
    ostringstream escaped;
    escaped.fill('0'); //pad numbers w/ 0 to reach width
    escaped << hex; //numbers will be in hex format

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << '%' << setw(2) << int((unsigned char) c);
    }

    return escaped.str();
}

//url_decode is copied from stackexchange: 
// http://stackoverflow.com/questions/154536/encode-decode-urls-in-c
string url_decode(string &SRC) 
{
    string ret;
    char ch;
    int i, ii;
    for (i=0; i<SRC.length(); i++) {
        if (int(SRC[i])=='+') {
            ret += ' ';
        } else {
            if (int(SRC[i])=='%') {
                sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
                ch=static_cast<char>(ii);
                ret+=ch;
                i=i+2;
            } else {
                ret+=SRC[i];
            }
        }
    }
    return (ret);
}

// convert html to text or visa-versa
// dir=1 is html to text
// dir=-1 is text to html
// libcgicc has standard functions for this and might be a better choice
void html2txt(std::string& seq, int dir)
{
  //see: http://www.w3schools.com/tags/ref_urlencode.asp
  
    if(dir == 1)
      seq = url_decode(seq);
    else
      seq = url_encode(seq);
}




