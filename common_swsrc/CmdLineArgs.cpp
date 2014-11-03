#include "CmdLineArgs.h"
#include <sstream>
#include <cstring>

using namespace std;

CmdLineArgs::CmdLineArgs(int argc, char* argv[])
{
    m_vArgv = std::vector<std::string>(argc);

    for (int i=0; i<argc; i++) {
        m_vArgv[i] = argv[i];
    }
}

CmdLineArgs::CmdLineArgs(const char* szCommandLine)
{
    std::istringstream ss(szCommandLine);

    do {
        std::string s;
        ss >> s;
        m_vArgv.push_back(s);
    } while(!(ss.rdstate() & ios::eofbit));
}

int CmdLineArgs::FindString(const string& s, int iAfter /*=0*/) const
{
    for(int i=iAfter; i<(int)m_vArgv.size(); i++)
        if(strcmp(m_vArgv[i].c_str(), s.c_str()) == 0)
            return i;

    return -1;
}

int CmdLineArgs::GetString(std::string& s, int index) const
{
    if((index < 0) || (index >= (int)m_vArgv.size()))
        return -1;

    s = m_vArgv[index];
    return (int)s.length();
}

std::string
CmdLineArgs::GetStringAfter(const std::string &s,
                            const std::string &sDefault) const
{
    int i = FindString(s);

    if (i < 0)
        return sDefault;

    std::string sReturn;

    if (GetString(sReturn, i + 1) < 0)
        return sDefault;
    else
        return sReturn;
}

std::string
CmdLineArgs::GetStringAfter(const std::string &s) const
{
    int i = FindString(s);
    if (i < 0)
        throw non_existing_string(s);

    std::string sReturn;

    if (GetString(sReturn, i+1) < 0)
        throw non_existing_string(s);
    else
        return sReturn;
}
