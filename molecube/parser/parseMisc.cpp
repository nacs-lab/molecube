#include "parseMisc.h"

#include <nacs-utils/number.h>
#include <nacs-utils/log.h>

#include <iostream>
#include <mutex>

#include "parseTxtSeq.h"
#include "saveloadmap.h"
#include "AD9914.h"
#include "common.h"

namespace NaCs {

void
printPlainResponseHeader(std::ostream &out)
{
    out << "Content-type: text/plain; charset=UTF-8\r\n\r\n";
}

static void
printJSONResponseHeader(std::ostream &out)
{
    out << "Content-type: application/json; charset=UTF-8\r\n\r\n";
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
getDeviceParams(NaCs::Pulser::Pulser &pulser, const std::string &page,
                txtmap_t &params)
{
    std::lock_guard<FLock> fl(g_fPulserLock);

    if (page == "dds") {
        char key[32];
        char val[32];



        for (unsigned iDDS = 0;iDDS < PULSER_NDDS;iDDS++) {
            double f = 1e-6 * pulser.get_dds_freq_f(iDDS);
            snprintf(key, 32, "freq%d", iDDS);
            snprintf(val, 32, "%.6f MHz", f);
            params[key] = val;

            double A = pulser.get_dds_amp_f(iDDS);
            snprintf(key, 32, "tude%d", iDDS);
            snprintf(val, 32, "%.4f", A);
            params[key] = val;

            double phase = pulser.get_dds_phase_f(iDDS);
            snprintf(key, 32, "phase%d", iDDS);
            snprintf(val, 32, "%.3f deg", phase);
            params[key] = val;
        }
    }
}

static void
setDeviceParams(NaCs::Pulser::Pulser &pulser, const std::string &page,
                const txtmap_t &params)
{
    std::lock_guard<FLock> fl(g_fPulserLock);

    if (page == "dds") {
        // dumpMap(params, gLog);

        txtmap_t::const_iterator pos;
        char buff[32];

        for (unsigned iDDS = 0;iDDS < PULSER_NDDS;iDDS++) {
            sprintf(buff, "freq%d", iDDS);

            pos = params.find(buff);
            if (pos != params.end()) {
                double f = 1e6 * atof(pos->second.c_str());
                nacsLog("DDS setfreq(%d): %12.3f\n", iDDS, f);
                pulser.set_dds_freq_f(iDDS, f);
                unsigned ftw = pulser.get_dds_freq(iDDS);
                double freq_get =
                    Pulser::DDSConverter::num2freq(ftw, PULSER_AD9914_CLK);
                nacsLog("DDS getfreq(%d): %12.3f  (ftw = %08X)\n",
                        iDDS, freq_get, ftw);
            }

            // "amp" doesn't work with jQuery (special meaning? jQuery bug?)
            sprintf(buff, "tude%d", iDDS);
            pos = params.find(buff);
            if (pos != params.end()) {
                double amp = limit(atof(pos->second.c_str()), 1);
                nacsLog("DDS setamp (%d): %6.3f %%\n", iDDS, amp * 100);
                pulser.set_dds_amp_f(iDDS, amp);
            }

            sprintf(buff, "phase%d", iDDS);
            pos = params.find(buff);
            if (pos != params.end()) {
                double phase = atof(pos->second.c_str());
                nacsLog("DDS setphase(%d): %9.3f degrees\n", iDDS, phase);
                pulser.set_dds_phase_f(iDDS, phase);
            }

            sprintf(buff, "reset%d", iDDS);
            pos = params.find(buff);
            if(pos != params.end()) {
                nacsLog("DDS reset/init (%d)\n", iDDS);
                NaCs::init_AD9914(pulser, iDDS, true);
                // fprintf(gLog, "DDS test (%d)\n", iDDS);
                // print_AD9914_registers(pulser, iDDS, gLog);
            }
        }
    }

    if (page == "ttl") {
        txtmap_t::const_iterator posHi = params.find("ttlHiMask");
        txtmap_t::const_iterator posLo = params.find("ttlLoMask");

        if(posHi != params.end() && posLo != params.end()) {
            unsigned hi = 0;
            sscanf(posHi->second.c_str(), "%x", &hi);

            unsigned lo = 0;
            sscanf(posLo->second.c_str(), "%x", &lo);

            pulser.set_ttl_mask(hi, lo);
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
parseQueryCGI(NaCs::Pulser::Pulser &pulser, cgicc::Cgicc &cgi,
              const verbosity &reply)
{
    cgicc::form_iterator cmd = cgi.getElement("command");
    cgicc::form_iterator page = cgi.getElement("page");
    if (cmd != cgi.getElements().end()) {
        nacsLog("Command = %s\n", (**cmd).c_str());
        LockGPL lock;
        if ((**cmd) == "getTTL") {
            printJSONResponseHeader(reply);
            unsigned lo, hi;
            pulser.get_ttl_mask(&hi, &lo);

            char buff[64];
            sprintf(buff, "{\"lo\":%u, \"hi\":%u}", lo, hi);
            reply << buff;

            nacsLog("%s\n", buff);
            return true;
        }

        if ((**cmd) == "getActiveDDS") {
            printJSONResponseHeader(reply);
            stream_vect_to_JSON_array(reply, active_dds);
            return true;
        }

        if ((**cmd) == "setParams" &&  page != cgi.getElements().end()) {
            printPlainResponseHeader(reply);
            std::string sPage = **page;
            removeNonAlphaNum(sPage);

            if (sPage.length()) {
                txtmap_t params;
                // TODO FIX absolute path
                std::string fname = "/srv/http/userdata/params_" + sPage;

                // load existing params
                loadMap(params, fname);

                if (parseParamsCGI(params, cgi)) {
                    saveMap(params, fname);
                    return true;
                }
            } else {
                return false;
            }
        }

        if ((**cmd) == "getParams" &&  page != cgi.getElements().end() ) {
            printPlainResponseHeader(reply);
            std::string sPage = **page;
            removeNonAlphaNum(sPage);
            if (sPage.length()) {
                txtmap_t params;
                // TODO FIX absolute path
                loadMap(params, "/srv/http/userdata/params_" + sPage);
                getDeviceParams(pulser, sPage, params);
                dumpMapHTML(params, reply);
                return true;
            } else {
                return false;
            }
        }

        if ((**cmd) == "setDeviceParams" && page != cgi.getElements().end()) {
            printPlainResponseHeader(reply);

            std::string sPage = **page;
            removeNonAlphaNum(sPage);

            if (sPage.length()) {
                txtmap_t params;
                if (parseParamsCGI(params, cgi)) {
                    setDeviceParams(pulser, sPage, params);
                    return true;
                }
            } else {
                return false;
            }
        }

        if ((**cmd) == "runseq") {
            parseSeqCGI(pulser, cgi, reply);
            return true;
        }
        return false;
    } else {
        nacsLog("No Command\n");
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

    if (i != cgi.getElements().end()) {
        return i->getIntegerValue(0);
    } else {
        return defaultVal;
    }
}

std::string
getStringParamCGI(cgicc::Cgicc& cgi, const std::string& name,
                  const std::string& defaultVal)
{
    cgicc::form_iterator i = cgi.getElement(name);

    if (i != cgi.getElements().end()) {
        return i->getValue();
    } else {
        return defaultVal;
    }
}

}
