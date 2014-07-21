#include <string>
#include <fstream>
#include <iostream>
#include <map>

#include "parseMultiPart.h"

extern FILE* gLog;

using namespace std;

// parse multipart http POST saved in file
// author: trosen


void discardSubstrs(std::string& str, const std::string& disc)
{
    size_t pos;
    while( (pos = str.find(disc)) != string::npos)
        str.erase(pos, disc.length());
}

bool parseMultiPart(std::istream& ifs, const std::string& line1, txtmap_t& params)
{
   
  /* See: http://www.w3.org/TR/html401/interact/forms.html#h-17.13.4.2
   * Line endings are \r\n
   * Example of multipart/form-data to be read:
   *
   Content-Type: multipart/form-data; boundary=AaB03x

   --AaB03x
   Content-Disposition: form-data; name="submit-name"

   Larry
   --AaB03x
   Content-Disposition: form-data; name="files"; filename="file1.txt"
   Content-Type: text/plain

   ... contents of file1.txt ...
   --AaB03x--
   *
   */

    string l;
    if(line1.length() > 0)
      l = line1;
    else
      getline(ifs, l); 
      
    discardSubstrs(l, "\r");
    
    size_t pos1 = l.find("Content-Type: multipart/form-data");
    
    if(pos1 == string::npos)
      return false;
  
    fprintf(gLog, "Parse multi-part message\n"); fflush(gLog);
    
    pos1 = l.find("boundary=", pos1);
    if(pos1 == string::npos)
      return false;
      
    string sep = l.substr(pos1+9);

    fprintf(gLog, "Separator: %s\n", sep.c_str()); fflush(gLog);

    while(!ifs.eof()) {
        string line;

        //find separator
        while(line.find(sep) == string::npos && !ifs.eof())
            getline(ifs, line);

        //next line includes param name
        getline(ifs, line);

        if(ifs.eof()) break;

        //check if this section is a file
        //extract file name
        size_t pos1 = line.find("filename=");
        
        if(line.find("filename=") != string::npos) {
            string name = line.substr(pos1+9);
            discardSubstrs(name, "\"");
            discardSubstrs(name, "\r");

            do {
                getline(ifs, line);
            } while (line.find("Content-Type") == string::npos && !ifs.eof());

            //file starts here
            string strfile;
            unsigned nLines = 0;
            while(!ifs.eof()) {
                getline(ifs, line);

                if(line.find(sep) != string::npos) //done with section
                    break;
                else {
                    nLines++;
                    strfile.append("\n");
                    strfile.append(line);
                }
            }

            params[name] = strfile;
            fprintf(gLog, "file: %s has %d lines\n", name.c_str(), nLines); fflush(gLog);
        } else {
            //extract param name
            size_t pos1 = line.find("name=");

            string name;

            if(pos1 != string::npos ) {
                name = line.substr(pos1+5);
                discardSubstrs(name, "\"");
                discardSubstrs(name, "\r");

                string value;

                //blank line
                getline(ifs, line);

                //value
                getline(ifs, value);

                params[name] = value;
                fprintf(gLog, "parameter: %s = %s\n", name.c_str(), value.c_str()); fflush(gLog);
            }
        }
    }
    
    return true;
}
