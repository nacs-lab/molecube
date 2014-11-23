#include "parseMisc.h"

#include <nacs-utils/number.h>
#include <nacs-utils/log.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <mutex>

#include "parseTxtSeq.h"
#include "saveloadmap.h"
#include "AD9914.h"
#include "dds_pulse.h"
#include "verbosity.h"
#include "pulse_controller.h"

#include <string_func.h>
#include <common.h>

NaCs::FLock g_fPulserLock("/tmp/pulser.lock");
std::vector<unsigned> active_dds; // all DDS that are available

void printPlainResponseHeader()
{
    // std::cout << "HTTP/1.1 200 OK\n";
    std::cout << "Content-type: text/plain; charset=UTF-8\r\n\r\n";
}

void printJSONResponseHeader()
{
    // std::cout << "HTTP/1.1 200 OK\n";
    std::cout << "Content-type: application/json; charset=UTF-8\r\n\r\n";
}


static void
removeNonAlphaNum(std::string &s)
{
    size_t i=0;

    while (i < s.length()) {
        if (isalpha(s[i]) || isdigit(s[i])) {
            i++;
        } else {
            s.erase(i, 1);
        }
    }
}

static bool
parseParamsCGI(txtmap_t& params, cgicc::Cgicc& cgi)
{
    cgicc::const_form_iterator i = cgi.getElements().begin();

    while(i != cgi.getElements().end()) {
        params[i->getName()] = i->getValue();
        i++;
    }

    return true;
}

static void
getDeviceParams(const std::string& page, txtmap_t& params)
{
    std::lock_guard<NaCs::FLock> fl(g_fPulserLock);

    if (page == "dds") {
        char key[32];
        char val[32];



        for(unsigned iDDS=0; iDDS<NDDS; iDDS++) {
            double f = 1e-6*DDS_get_freqHz(iDDS);
            snprintf(key, 32, "freq%d", iDDS);
            snprintf(val, 32, "%.6f MHz", f);
            params[key] = val;

            double A = DDS_get_amp(iDDS);
            snprintf(key, 32, "tude%d", iDDS);
            snprintf(val, 32, "%.4f", A);
            params[key] = val;

            double phase = DDS_get_phase_deg(iDDS);
            snprintf(key, 32, "phase%d", iDDS);
            snprintf(val, 32, "%.3f deg", phase);
            params[key] = val;
        }
    }
}

static void
setDeviceParams(const std::string& page, const txtmap_t& params)
{
    std::lock_guard<NaCs::FLock> fl(g_fPulserLock);

    if(page == "dds") {
        // dumpMap(params, gLog);

        txtmap_t::const_iterator pos;
        char buff[32];

        for (unsigned iDDS = 0;iDDS < NDDS;iDDS++) {
            sprintf(buff, "freq%d", iDDS);

            pos = params.find(buff);
            if (pos != params.end()) {
                double f = 1e6 * atof(pos->second.c_str());
                nacsLog("DDS setfreq(%d): %12.3f\n", iDDS, f);
                DDS_set_freqHz(iDDS, f);
                unsigned ftw = DDS_get_ftw(iDDS);
                nacsLog("DDS getfreq(%d): %12.3f  (ftw = %08X)\n",
                        iDDS, FTW2HzD(ftw, dds_clk(iDDS)), ftw);
            }

            // "amp" doesn't work with jQuery (special meaning? jQuery bug?)
            sprintf(buff, "tude%d", iDDS);
            pos = params.find(buff);
            if(pos != params.end()) {
                double A = atof(pos->second.c_str());
                A = nacsBound(0, A, 1);
                nacsLog("DDS setamp (%d): %6.3f %%\n", iDDS, A*100);
                DDS_set_amp(iDDS, A);
            }

            sprintf(buff, "phase%d", iDDS);
            pos = params.find(buff);
            if(pos != params.end()) {
                double phase = atof(pos->second.c_str());
                nacsLog("DDS setphase(%d): %9.3f degrees\n", iDDS, phase);
                DDS_set_phase_deg(iDDS, phase);
            }

            sprintf(buff, "reset%d", iDDS);
            pos = params.find(buff);
            if(pos != params.end()) {
                nacsLog("DDS reset/init (%d)\n", iDDS);
                init_AD9914(pulser, iDDS, true);
                //fprintf(gLog, "DDS test (%d)\n", iDDS);
                //print_AD9914_registers(pulser, iDDS, gLog);
            }
        }
    }

    if(page == "ttl") {
        txtmap_t::const_iterator posHi = params.find("ttlHiMask");
        txtmap_t::const_iterator posLo = params.find("ttlLoMask");

        if(posHi != params.end() && posLo != params.end()) {
            unsigned hi = 0;
            sscanf(posHi->second.c_str(), "%x", &hi);

            unsigned lo = 0;
            sscanf(posLo->second.c_str(), "%x", &lo);

            PULSER_set_ttl(pulser, hi, lo);
            nacsLog("set TTL ttlHiMask=%08X  ttlLoMask=%08X\n", hi, lo);
        }
    }
}

template<class V>
static NACS_INLINE void
stream_vect_to_JSON_array(std::ostream& os, const V& v)
{
    os << "[";
    for (unsigned i = 0;i < v.size();i++) {
        if (i > 0) {
            os << ", ";
        }
        os << v[i];
    }
    os << "]";
}

bool
parseQueryCGI(cgicc::Cgicc &cgi)
{
    cgicc::form_iterator cmd = cgi.getElement("command");
    cgicc::form_iterator page = cgi.getElement("page");

    if (cmd != cgi.getElements().end()) {
        gvSTDOUT.printf("Command = %s\n", (**cmd).c_str());

        if ((**cmd) == "getTTL") {
            printJSONResponseHeader();
            unsigned lo, hi;
            PULSER_get_ttl(pulser, &hi, &lo);

            char buff[64];
            sprintf(buff, "{\"lo\":%u, \"hi\":%u}", lo, hi);
            std::cout << buff;

            nacsLog("%s\n", buff);
            return true;
        }

        if ((**cmd) == "getActiveDDS") {
            printJSONResponseHeader();
            stream_vect_to_JSON_array(std::cout, active_dds);
            return true;
        }

        if ((**cmd) == "setParams" &&  page != cgi.getElements().end()) {
            printPlainResponseHeader();
            std::string sPage = **page;
            removeNonAlphaNum(sPage);

            if (sPage.length()) {
                txtmap_t params;
                std::string fname = "/home/www/userdata/params_" + sPage;

                //load existing params
                loadMap(params, fname);

                if (parseParamsCGI(params, cgi)) {
                    //save
                    saveMap(params, fname);
                    return true;
                }
            } else {
                return false;
            }
        }

        if ((**cmd) == "getParams" &&  page != cgi.getElements().end() ) {
            printPlainResponseHeader();
            std::string sPage = **page;
            removeNonAlphaNum(sPage);
            if (sPage.length()) {
                txtmap_t params;
                loadMap(params, "/home/www/userdata/params_" + sPage);
                getDeviceParams(sPage, params);
                //dumpMap(params, gLog); fflush(gLog);
                dumpMapHTML(params, std::cout);
                return true;
            } else {
                return false;
            }
        }

        if ((**cmd) == "setDeviceParams" &&  page != cgi.getElements().end()) {
            printPlainResponseHeader();

            std::string sPage = **page;
            removeNonAlphaNum(sPage);

            if(sPage.length()) {
                txtmap_t params;
                if(parseParamsCGI(params, cgi)) {
                    setDeviceParams(sPage, params);
                    return true;
                }
            } else {
                return false;
            }
        }

        if ((**cmd) == "runseq") {
            parseSeqCGI(cgi);
            return true;
        }
        return false;
    } else {
        gvSTDOUT.printf("No Command\n");
        return false;
    }

}

unsigned
getUnsignedParam(const std::string& seq, const std::string& name,
                 unsigned defaultVal)
{
    size_t pos = seq.find(name);

    if(pos != std::string::npos) {
        unsigned val;

        if(sscanf(seq.substr(pos).c_str()+name.length(), "%u", &val))
            return val;
    }

    return defaultVal;
}

unsigned
getHexParam(const std::string &seq, const std::string &name,
            unsigned defaultVal)
{
    size_t pos = seq.find(name);

    if(pos != std::string::npos) {
        unsigned val;

        if(sscanf(seq.substr(pos).c_str()+name.length(), "%x", &val))
            return val;
    }

    return defaultVal;
}

double
getDoubleParam(const std::string& seq, const std::string& name,
               double defaultVal)
{
    size_t pos = seq.find(name);

    if (pos != std::string::npos) {
        double val;

        //this may not be working.  glibc bug?
        if(sscanf(seq.substr(pos).c_str()+name.length(), "%lf", &val))
            return val;
    }

    return defaultVal;
}

bool
getCheckboxParam(const std::string& seq, const std::string& name,
                 bool defaultVal)
{
    size_t pos = seq.find(name+"on");
    if (pos != std::string::npos) {
        return true;
    }
    return defaultVal;
}

std::string
getStringParam(const std::string &seq, const std::string &token,
               const std::string &defaultVal, const std::string&)
{
    size_t pos = seq.find(token);
    if (pos != std::string::npos) {
        pos += token.length();
        std::string val;

        size_t pos2 = seq.substr(pos).find("&");
        if (pos2 != std::string::npos) {
            return seq.substr(pos).substr(0, pos2);
        } else {
            return seq.substr(pos);
        }
    }
    return defaultVal;
}

bool
getCheckboxParamCGI(cgicc::Cgicc& cgi, const std::string& name,
                    bool defaultVal)
{
    cgicc::form_iterator i = cgi.getElement(name);

    if (i != cgi.getElements().end()) {
        return i->getValue() == "on";
    } else {
        return defaultVal;
    }
}

unsigned
getUnsignedParamCGI(cgicc::Cgicc& cgi, const std::string& name,
                    unsigned defaultVal)
{
    cgicc::form_iterator i = cgi.getElement(name);

    if(i != cgi.getElements().end())
        return i->getIntegerValue(0);
    else
        return defaultVal;
}

std::string
getStringParamCGI(cgicc::Cgicc& cgi, const std::string& name,
                  const std::string& defaultVal)
{
    cgicc::form_iterator i = cgi.getElement(name);

    if(i != cgi.getElements().end())
        return i->getValue();
    else
        return defaultVal;
}
