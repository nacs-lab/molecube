/* Parse pulse sequence descriptions (text)
 *
 * This is where the sausage gets eaten.  The goal is to read a simple text
 * description of the desired pulse sequence, and then produce it on the FPGA.
 * Ideally, the text description is both easily readable by humans (physicists?)
 * and computers.
 *
 * The text description format is documented in the Wiki (Zyki).
 * It is not really satisfactory.  A better approach might be to use Lex & Yacc
 * to build a proper parser.
 *  ___
 *   |R
 */
#include "parseTxtSeq.h"

#include <nacs-utils/log.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <mutex>
#include <algorithm>

#include "dds_pulse.h"
#include "AD9914.h"
#include "string_func.h"

#include "parseMisc.h"
#include "saveloadmap.h"
#include "verbosity.h"
#include "linux_file_util.h"

#include <common.h>

// keep track of "current" TTL state and elapsed time
// "current" refers to parsing the pulse sequence (not generating it)

namespace NaCs {

static bool hasTTL;
static unsigned tCurr;
static unsigned nextTTL = 0;
static unsigned currTTL = 0;
static unsigned g_lineNum = 0;

// make sure any TTLs that were spec'd previously are still active
// tNewPulse = spec'd time for new pulse
// tMinPulse = minimum duration of new pulse *before* update
static void dealWithCurrentTTL(volatile void *pulse_addr, unsigned tNewPulse,
                               unsigned tMinPulse);

//parse text-encoded pulse sequence
static bool parseSeqTxt(volatile void *pulse_addr, unsigned reps,
                        const std::string &seqTxt, bool bForever,
                        bool bDebugPulses);

//abstract base class for different types of pulse commands
class pulse_cmd {
protected:
    volatile void *m_pulse_addr;
    pulse_cmd() = delete;
public:
    pulse_cmd(volatile void *pulse_addr) : m_pulse_addr(pulse_addr) {}
    virtual ~pulse_cmd() {}
    virtual void makePulse() = 0;
};

//parsed pulse commands are stored in this vector for later playback
std::vector<pulse_cmd*> pulses;

// diable timing check command.  insert this prior to the last pulse in sequence.
// takes 0 time in the sequence.
class disable_timing_check_cmd  : public pulse_cmd {
public:
    using pulse_cmd::pulse_cmd;
    virtual ~disable_timing_check_cmd() {}

    virtual void
    makePulse()
    {
        // Change extra_flags
        PULSER_disable_timing_check();
    }
};

class ttl_pulse_cmd : public pulse_cmd {
    unsigned m_t, m_ttl;
public:
    virtual ~ttl_pulse_cmd() {}
    ttl_pulse_cmd(volatile void *pulse_addr, unsigned t, unsigned ttl) :
        pulse_cmd(pulse_addr), m_t(t), m_ttl(ttl)
    {
        if (m_t < PULSER_T_TTL_MIN) {
            gvSTDOUT.printf("TTL pulse 0x%08X too short: %.2f us\n",
                            m_t * PULSER_DT_us, m_ttl);
            throw std::runtime_error("The pulse at t = " +
                                     std::to_string(m_t * PULSER_DT_us) +
                                     " us is too short or early.");
        }
    }

    virtual void
    makePulse()
    {
        // multiple PULSER_short_pulse
        PULSER_pulse(m_pulse_addr, m_t, 0, m_ttl);
    }
};

class clock_out_cmd : public pulse_cmd {
    unsigned m_divider;
public:
    virtual ~clock_out_cmd() {}
    clock_out_cmd(volatile void *pulse_addr, unsigned divider)
        : pulse_cmd(pulse_addr),
          m_divider(divider)
    {}

    virtual void
    makePulse()
    {
        // PULSER_short_pulse
        PULSER_enable_clock_out(m_pulse_addr, m_divider);
    }
    static const unsigned DURATION = 5;
};

class dds_cmd : public pulse_cmd {
protected:
    unsigned m_dds, m_operand;
public:
    virtual ~dds_cmd() {}
    dds_cmd(volatile void *pulse_addr, unsigned dds, unsigned operand)
        : pulse_cmd(pulse_addr), m_dds(dds), m_operand(operand)
    {
        if (dds > NDDS - 1) {
            throw std::runtime_error("Line " + std::to_string(g_lineNum) +
                                     ", Invalid DDS: " +
                                     std::to_string(m_dds));
        }
    }
    static const unsigned DURATION = PULSER_T_DDS_MIN;
};

class set_freq_cmd : public dds_cmd {
public:
    virtual ~set_freq_cmd() {}
    set_freq_cmd(volatile void *pulse_addr, unsigned dds, unsigned ftw) :
        dds_cmd(pulse_addr, dds, ftw) {}
    set_freq_cmd(volatile void *pulse_addr, unsigned dds, double f) :
        dds_cmd(pulse_addr, dds, Hz2FTW(f, dds_clk(dds)))
    {
    }

    virtual void
    makePulse()
    {
        // PULSER_short_pulse
        DDS_set_ftw(m_pulse_addr, m_dds, m_operand);
    }
};

class set_amp_cmd : public dds_cmd {
public:
    virtual ~set_amp_cmd() {}
    set_amp_cmd(volatile void *pulse_addr, unsigned dds, unsigned atw) :
        dds_cmd(pulse_addr, dds, atw) {}
    set_amp_cmd(volatile void *pulse_addr, unsigned dds, double A) :
        dds_cmd(pulse_addr, dds, 0x0FFF & (int)(A * 4095 + 0.5))
    {}

    virtual void
    makePulse()
    {
        // PULSER_short_pulse
        DDS_set_atw(m_pulse_addr, m_dds, m_operand);
    }
};

class set_phase_cmd : public dds_cmd {
public:
    virtual ~set_phase_cmd() {}
    set_phase_cmd(volatile void *pulse_addr, unsigned dds, unsigned ptw) :
        dds_cmd(pulse_addr, dds, ptw) {}
    set_phase_cmd(volatile void *pulse_addr, unsigned dds, double phi) :
        dds_cmd(pulse_addr, dds, 0xFFFF & (int)(phi * 65536 / 360.0 + 0.5))
    {}

    virtual void
    makePulse()
    {
        // PULSER_short_pulse
        // Set ddsPhase
        DDS_set_ptw(m_pulse_addr, m_dds, m_operand);
    }
};

class shift_phase_cmd : public dds_cmd {
public:
    virtual ~shift_phase_cmd()
    {}
    shift_phase_cmd(volatile void *pulse_addr, unsigned dds, unsigned ptw) :
        dds_cmd(pulse_addr, dds, ptw)
    {}
    shift_phase_cmd(volatile void *pulse_addr, unsigned dds, double phi) :
        dds_cmd(pulse_addr, dds, 0xFFFF & (int)(phi * 65536 / 360.0 + 0.5))
    {}

    virtual void
    makePulse()
    {
        // PULSER_short_pulse
        // use ddsPhase
        DDS_shift_ptw(m_pulse_addr, m_dds, m_operand);
    }
};

class dds_reset_cmd : public pulse_cmd {
    unsigned m_dds;
public:
    virtual ~dds_reset_cmd() {}
    dds_reset_cmd(volatile void *pulse_addr, unsigned dds)
        : pulse_cmd(pulse_addr), m_dds(dds)
    {}

    virtual void
    makePulse()
    {
        // PULSER_short_pulse
        // Set ddsPhase
        PULSER_dds_reset(m_pulse_addr, m_dds);

        //disable programmable modulus, enable profile 0, enable SYNC_CLK output
        PULSER_set_dds_two_bytes(m_pulse_addr, m_dds, 0x05, 0x840B);

        //enable amplitude control (OSK)
        PULSER_set_dds_two_bytes(m_pulse_addr, m_dds, 0x0, 0x0108);
    }

    static const unsigned DURATION = 90;
};


static bool
get_channel_and_operand(std::string &arg1, std::istream &s, int *channel,
                        double *operand)
{
    std::string line;
    getline(s, line);

    if (!line.length())
        return false;

    if (operand)
        if (!sscanf(line.c_str(), " = %le", operand))
            return false;

    if (sscanf(arg1.c_str(), " %d", channel))
        return true;

    return false;
}

template <class C>
static NACS_INLINE C*
parse_pulse(volatile void *pulse_addr, std::vector<pulse_cmd*>, unsigned t,
            std::string &arg1, std::istream &s)
{
    int channel = -1;
    double operand = 0;

    C *pulse = 0;

    if (get_channel_and_operand(arg1, s, &channel, &operand)) {
        dealWithCurrentTTL(pulse_addr, t, C::DURATION);
        pulse = new C(pulse_addr, channel, operand);
        pulses.push_back(pulse);
        tCurr = t;
    }

    return pulse;
}

//setup "current" TTL, now that the end time is known
static void
makeCurrTTL(volatile void *pulse_addr, unsigned tEnd)
{
    if (hasTTL) {
        if (tEnd < tCurr + PULSER_T_TTL_MIN) {
            gvSTDOUT.printf("TTL pulse too short at tEnd = %.2f us.\n",
                            tEnd * PULSER_DT_us);
            gvSTDOUT.printf("Previous t = %.2f us.  Earliest is %.2f us.\n",
                            tCurr * PULSER_DT_us,
                            (PULSER_T_TTL_MIN + tCurr) * PULSER_DT_us);

            throw std::runtime_error("The pulse at t = " +
                                     std::to_string(tEnd * PULSER_DT_us) +
                                     " us is too early.  What's the big hurry?");
        }

        pulses.push_back(new ttl_pulse_cmd(pulse_addr, tEnd - tCurr, nextTTL));
        currTTL = nextTTL;
    }
}

static void
setTTL(volatile void *pulse_addr, unsigned t, unsigned channel, unsigned value)
{
    makeCurrTTL(pulse_addr, t);

    if (value) {
        nextTTL |= 1 << channel;
    } else {
        nextTTL &= ~(1 << channel);
    }
    tCurr = t;
    hasTTL = true;
}

static void
setTTLall(volatile void *pulse_addr, unsigned t, unsigned value)
{
    makeCurrTTL(pulse_addr, t);

    nextTTL = value;
    tCurr = t;
    hasTTL = true;
}

static void
finishTTL(volatile void *pulse_addr)
{
    if (hasTTL) {
        //disable timing check prior to last pulse
        pulses.push_back(new disable_timing_check_cmd(pulse_addr));
        pulses.push_back(new ttl_pulse_cmd(pulse_addr,
                                           PULSER_T_TTL_MIN, nextTTL));
    }

    currTTL = nextTTL;
}

// eat stream up to character == to.  put prior chars into strPrior
static bool
eatStreamTo(std::istream &is, char to, std::string &strPrior)
{
    while (!is.eof()) {
        char c;
        is.get(c);

        if (!is.eof()) {
            strPrior.push_back(c);

            if (c == to) {
                return true;
            }
        }
    }

    return false;
}

//make sure any TTLs that were spec'd previously are still active
// tNewPulse = spec'd time for new pulse
// tMinPulse = minimum duration of new pulse *before* update
static void
dealWithCurrentTTL(volatile void *pulse_addr, unsigned tNewPulse,
                   unsigned tMinPulse)
{
    if (hasTTL) {
        unsigned tMin = tCurr + tMinPulse;

        if (currTTL != nextTTL)
            tMin += PULSER_T_TTL_MIN;

        if (tNewPulse == tMin && currTTL == nextTTL)
            return; // nothing to do.

        if (tNewPulse < tMin) {
            gvSTDOUT.printf("ERROR: Pulse too short at t = %.2f us. \n",
                            tNewPulse*PULSER_DT_us);
            gvSTDOUT.printf("Previous t = %.2f us.  Earliest is t = %.2f us.\n",
                            tCurr*PULSER_DT_us, tMin*PULSER_DT_us);

            throw std::runtime_error("The pulse at t = " +
                                     std::to_string(tNewPulse * PULSER_DT_us) +
                                     " us is too early.  What's the big hurry?");
        } else {
            pulses.push_back(new ttl_pulse_cmd(pulse_addr,
                                               tNewPulse - tCurr - tMinPulse,
                                               nextTTL));
            tCurr = tNewPulse - tMinPulse;
            currTTL = nextTTL;
        }
    } else {
        throw std::runtime_error("Must run a TTL pulse prior to pulse at t = " +
                                 std::to_string(tNewPulse * PULSER_DT_us));
    }
}


static bool
parseReset(volatile void *pulse_addr, unsigned t, std::string &arg1,
           std::istream &s)
{
    std::string line;
    getline(s, line);

    if(!line.length())
        return false;


    int channel = -1;
    if (sscanf(arg1.c_str(), " %d", &channel)) {
        dealWithCurrentTTL(pulse_addr, t, dds_reset_cmd::DURATION);

        pulses.push_back(new dds_reset_cmd(pulse_addr, channel));
        tCurr = t;

        return true;
    }

    return false;
}

static bool
parseTTL(volatile void *pulse_addr, unsigned t, std::string &arg1,
         std::istream &s)
{
    std::string line;
    getline(s, line);

    if (!line.length())
        return false;

    unsigned ttl;
    if (!sscanf(line.c_str(), " = %x", &ttl))
        return false;

    int channel = -1;
    if (sscanf(arg1.c_str(), " %d", &channel)) {
        setTTL(pulse_addr, t, channel, ttl);
        return true;
    } else {
        if (arg1.find("all") != std::string::npos) {
            setTTLall(pulse_addr, t, ttl);
            return true;
        }
    }

    return false;
}

static bool
parseClockOut(volatile void *pulse_addr, unsigned t, std::string &arg1,
              std::istream&)
{
    int divider = 0;

    if (arg1.find("off") == 0) {
        divider = 255;
    } else {
        divider = atoi(arg1.c_str()) - 1;
    }

    if (divider < 0 || divider > 255) {
        gvSTDOUT.printf("Error at t = %6.3f us.  "
                        "CLOCK_OUT accepts parameter off or 1 ... 255\n",
                        t * PULSER_DT_us);
        throw std::runtime_error("There was a very bad parameter.");
    }

    dealWithCurrentTTL(pulse_addr, t, 0);
    tCurr += PULSER_T_TTL_MIN;

    pulses.push_back(new clock_out_cmd(pulse_addr, divider));

    return true;
}

static bool
parseCommand(volatile void *pulse_addr, unsigned t, std::string &cmd,
             std::string &arg1, std::istream &s)
{
    if (cmd.find("TTL") != std::string::npos)
        return parseTTL(pulse_addr, t, arg1, s);

    if (cmd.find("freq") != std::string::npos)
        return 0 != parse_pulse<set_freq_cmd>(pulse_addr, pulses, t, arg1, s);

    if (cmd.find("amp") != std::string::npos)
        return 0 != parse_pulse<set_amp_cmd>(pulse_addr, pulses, t, arg1, s);

    if (cmd.find("phase") != std::string::npos)
        return 0 != parse_pulse<set_phase_cmd>(pulse_addr, pulses, t, arg1, s);

    if (cmd.find("shiftp") != std::string::npos)
        return 0 != parse_pulse<shift_phase_cmd>(pulse_addr, pulses, t, arg1, s);

    if (cmd.find("reset") != std::string::npos)
        return parseReset(pulse_addr, t, arg1, s);

    if (cmd.find("CLOCK_OUT") != std::string::npos)
        return parseClockOut(pulse_addr, t, arg1, s);

    return false;
}

//parse URL-encoded pulse sequence
bool
parseSeqURL(volatile void *pulse_addr, std::string &seq)
{
    unsigned reps = getUnsignedParam(seq, "reps=", 1);
    bool bDebugPulses = getCheckboxParam(seq, "debugPulses=", false);
    bool bForever = getCheckboxParam(seq, "forever=", false);

    size_t start_pos = seq.find("seqtext=");
    size_t L = std::string("seqtext=").length();

    if (start_pos == std::string::npos) { // not a URLENCODEd sequence
        return false;
    }

    size_t end_pos = seq.find("&", start_pos+L);
    if (end_pos == std::string::npos)
        end_pos = seq.length();

    std::string seqTxt = seq.substr(start_pos + L, end_pos - start_pos - L);
    html2txt(seqTxt, 1); //this is a slow function

    parseSeqTxt(pulse_addr, reps, seqTxt, bForever, bDebugPulses);

    return true;
}

//parse pulse sequence via CGICC
bool
parseSeqCGI(volatile void *pulse_addr, cgicc::Cgicc& cgi)
{
    unsigned reps = getUnsignedParamCGI(cgi, "reps", 1);
    bool bDebugPulses = getCheckboxParamCGI(cgi, "debugPulses", false);
    bool bForever = getCheckboxParamCGI(cgi, "forever", false);

    //look for seqtext field
    std::string seqTxt = getStringParamCGI(cgi, "seqtext", "");
    if (seqTxt.length() == 0) {
        //if missing, look for attached file (multi-part)
        nacsLog("no seqtext parameter in form, looking for seqtext file\n");
        nacsLog("%d files attached\n", cgi.getFiles().size());

        cgicc::file_iterator i =  cgi.getFile("seqtext");
        if (i != cgi.getFiles().end()) {
            seqTxt = i->getData();
        } else {
            return false;
        }
    }

    parseSeqTxt(pulse_addr, reps, seqTxt, bForever, bDebugPulses);

    return true;
}

// 7/21/2014: no longer using this code
// keeping it in case CGICC causes trouble

// #include "parseMultiPart.h"

// bool parseSeqMultiPart(std::istream& is, const std::string& line1)
// {
//     txtmap_t m;
//     if (!parseMultiPart(is, line1, m))
//         return false;

//     unsigned reps = atoi(m["reps"].c_str());
//     bool bDebugPulses = (string::npos != m["debugPulses"].find("on"));
//     bool bForever = string::npos != m["forever"].find("on");

//     parseSeqTxt(reps, m["sequence.txt"], bForever, bDebugPulses);

//     return true;
// }

//parse text-encoded pulse sequence
static bool
parseSeqTxt(volatile void *pulse_addr, unsigned reps,
            const std::string& seqTxt, bool bForever, bool bDebugPulses)
{
    printPlainResponseHeader();

    if (bDebugPulses) {
        nacsLog("Parsing pulse sequence:%s\n", seqTxt.c_str());
    }

    unsigned nTimingErrors = 0;

    clock_t tClock0 = clock();

    //first parse and load up the pulses vector
    std::stringstream ss0(seqTxt);

    hasTTL = false;
    tCurr = 0;

    pulses.clear();

    g_lineNum = 0;
    double tSoFar = 0;
    bool use_dt = true;

    while (!ss0.eof()) {
        //read line
        std::string line;
        getline(ss0, line);

        g_lineNum++;

        // ignore everything after '#' comment symbol
        size_t posC = line.find("#");
        if (posC != std::string::npos)
            line = line.substr(0, posC);

        // ignore blank lines
        if (line.length() == 0)
            continue;

        // otherwise parse the line
        std::stringstream ssL(line);

        double new_t;
        std::string strPrior;

        // valid lines will start with "dt = " or "t = "

        if (!eatStreamTo(ssL, '=', strPrior)) {
            continue;
            throw std::runtime_error(std::to_string(g_lineNum) +
                                     ": no time spec.");
        }

        if (strPrior.find("dt") != std::string::npos) {
            use_dt = true;
            ssL >> new_t;
            new_t += tSoFar;
        } else if (strPrior.find("t") != std::string::npos) {
            use_dt = false;
            ssL >> new_t;
        } else {
            throw std::runtime_error(std::to_string(g_lineNum) +
                                     ": invalid time spec.");
        }
        if (new_t <= tSoFar && tSoFar > 0) {
            throw std::runtime_error(std::to_string(g_lineNum) +
                                     ": going back in time.");
        }

        // next comes the time unit, then a comma
        std::string timeunit;
        getline(ssL, timeunit, ',');

        if (ssL.eof()) {
            throw std::runtime_error(std::to_string(g_lineNum) +
                                     ": no action.");
        }

        // then comes the command name, followed by '(arg1)'
        std::string cmd;
        getline(ssL, cmd, '(');
        if (ssL.eof()) {
            throw std::runtime_error(std::to_string(g_lineNum) +
                                     ": incomplete action.");
        }

        std::string arg1;
        getline(ssL, arg1, ')');
        if (ssL.eof()) {
            throw std::runtime_error(std::to_string(g_lineNum) +
                                     ": incomplete action (2).");
        }

        if (!use_dt) {
            tSoFar = new_t;
        }
        if (parseCommand(pulse_addr, floor(tSoFar * PULSER_DT_per_us + 0.5),
                         cmd, arg1, ssL)) {
            tSoFar = new_t;
        } else {
            throw std::runtime_error(std::to_string(g_lineNum) +
                                     ": failed to parse command.");
        }
    }
    if (use_dt) {
        unsigned new_tcurr = floor(tSoFar * PULSER_DT_per_us + 0.5);
        dealWithCurrentTTL(pulse_addr, new_tcurr, 0);
        tCurr = new_tcurr;
    }
    finishTTL(pulse_addr);

    gvSTDOUT.printf("Parsed sequence into %d pulse commands.\n", pulses.size());

    if (bForever) {
        nacsLog("Start continuous run.\n");
    } else {
        nacsLog("Run %d sequences.\n", reps);
    }

    unsigned iRep;

    clock_t tClock1 = clock();

    // now run the pulses
    // update status string every 500 ms
    unsigned updateStatusModulo = 500000000 / (tCurr * PULSER_DT_ns);

    for (iRep = 0;iRep < reps || bForever;iRep++) {
        char buff[64];

        // updateStatusModulo can be 0 if the sequence is longer than 0.5s.
        if (!updateStatusModulo || iRep % updateStatusModulo == 0) {
            if (bForever) {
                snprintf(buff, 64, "Running sequence %d", iRep);
            } else {
                snprintf(buff, 64, "Running sequence %d / %d", iRep, reps);
            }
        }

        {
            // new scope to automatically release file lock at scope
            // exit or exception
            std::lock_guard<FLock> fl(g_fPulserLock);

            setProgramStatus(0, buff);

            // hold the sequnce until pulse buffer is full or
            // PULSER_wait_for_finished is called
            PULSER_set_hold(pulse_addr);

            PULSER_toggle_init(pulse_addr);
            PULSER_enable_timing_check();

            for (pulse_cmd *p : pulses) {
                p->makePulse();
            }

            // wait for pulses finished.
            PULSER_wait_for_finished(pulse_addr);

            if (!PULSER_timing_ok(pulse_addr)) {
                PULSER_clear_timing_check(pulse_addr);
                nTimingErrors++;
            }
        }

        if (g_stop_curr_seq) {
            gvSTDOUT.printf("Received stop pulse sequences signal.\n");
            g_stop_curr_seq = false;
            break;
        }
    }

    clock_t tClock2 = clock();

    gvSTDOUT.printf("Finished %d/%d pulse sequences.\n", iRep, reps);

    if (nTimingErrors == 0) {
        gvSTDOUT.printf("Timing OK\n");
    } else {
        gvSTDOUT.printf("Warning: %d timing failures.\n", nTimingErrors);
        gvSTDOUT.printf("Consider inserting a 10-100 us spulse to start the sequence.\n");
    }

    gvSTDOUT.printf("          Parser time: %9.3f ms\n",
                    (tClock1 - tClock0) * 0.001);
    gvSTDOUT.printf("       Execution time: %9.3f ms\n",
                    (tClock2 - tClock1) * 0.001);
    gvSTDOUT.printf("Duration of sequences: %9.3f ms\n",
                    iRep * (tCurr * PULSER_DT_ns) * 1e-6);
    return true;
}

//advance file to end of next delimeter
//return false if EOF
static bool
findNextDelim(FILE *f, const char *delim)
{
    int nMatch = 0;
    int delimLen = strlen(delim);

    while (!feof(f)) {
        int c = fgetc(f);

        if (c == delim[nMatch]) {
            nMatch++;
            if (nMatch == delimLen) {
                return true;
            }
        } else {
            nMatch = 0;
        }
    }
    return false;
}

// something fun
std::string
getQuote(const char *fname, const char *delim)
{
    static time_t tLastQuote = time(0);
    time_t tNow = time(0);

    if (tNow > tLastQuote + 30) {
        //no wasting time in the lab
        if (rand() % 2) {
            tLastQuote = tNow;
            std::string s;

            FILE *f = fopen(fname, "r");

            if (f) {
                nacsLog("Retrieving quote %s\n", fname);
                //get file length
                fseek(f, 0, SEEK_END);
                size_t len = ftell(f);
                size_t pos = rand() % len; // RAND_MAX >> len;

                fseek(f, pos, SEEK_SET);
                if (findNextDelim(f, delim)) {
                    size_t pos1 = ftell(f);
                    if (findNextDelim(f, delim)) {
                        size_t pos2 = ftell(f);
                        pos2 -= strlen(delim);

                        fseek(f, pos1, SEEK_SET);
                        s.resize(pos2-pos1, ' ');

                        if (s.size()) {
                            size_t __attribute__((unused)) unused =
                                fread(&(s[0]), 1, s.size(), f);
                        }
                    }
                }
                fclose(f);
            }

            return "\n\n===\n" + s + "\n===\n";
        }
    }
    return "";
}

}
