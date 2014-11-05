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

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstring>

#include "ttl_pulse.h"
#include "dds_pulse.h"
#include "AD9914.h"
#include "string_func.h"

#include "parseMisc.h"
#include "saveloadmap.h"
#include "parseTxtSeq.h"
#include "verbosity.h"
#include "linux_file_util.h"

// using namespace std;

// keep track of "current" TTL state and elapsed time
// "current" refers to parsing the pulse sequence (not generating it)

bool hasTTL;
unsigned tCurr;
unsigned nextTTL = 0;
unsigned currTTL = 0;
unsigned g_lineNum = 0;

//make sure any TTLs that were spec'd previously are still active
// tNewPulse = spec'd time for new pulse
// tMinPulse = minimum duration of new pulse *before* update
static void dealWithCurrentTTL(unsigned tNewPulse, unsigned tMinPulse);

//parse text-encoded pulse sequence
static bool parseSeqTxt(unsigned reps, const std::string& seqTxt,
                        bool bForever, bool bDebugPulses);

//abstract base class for different types of pulse commands
class pulse_cmd {
public:
    virtual ~pulse_cmd() {}
    virtual void makePulse() = 0;

    static verbosity* v;
};

verbosity *pulse_cmd::v = 0;

//parsed pulse commands are stored in this vector for later playback
std::vector<pulse_cmd*> pulses;

// diable timing check command.  insert this prior to the last pulse in sequence.
// takes 0 time in the sequence.
class disable_timing_check_cmd  : public pulse_cmd {
public:
    virtual ~disable_timing_check_cmd() {}

    virtual void makePulse() {
        PULSER_disable_timing_check(pulser);
    }
};

class ttl_pulse_cmd : public pulse_cmd {
public:
    virtual ~ttl_pulse_cmd() {}
    ttl_pulse_cmd(unsigned t, unsigned ttl) : t(t), ttl(ttl)
    {
        if(t < PULSER_T_TTL_MIN) {
            gvSTDOUT.printf("TTL pulse 0x%08X too short: %.2f us\n",
                            t * PULSER_DT_us, ttl);
            throw std::runtime_error("The pulse at t = " +
                                     std::to_string(t * PULSER_DT_us) +
                                     " us is too short or early.");
        }
    }

    virtual void
    makePulse()
    {
        TTL_pulse(t, ttl, v);
    }

    unsigned t, ttl;
};

class clock_out_cmd : public pulse_cmd {
public:
    virtual ~clock_out_cmd() {}
    clock_out_cmd(unsigned divider) : divider(divider) {}

    virtual void
    makePulse()
    {
        if (v)
            v->printf("clock output divider = %u\n", divider);
        PULSER_enable_clock_out(pulser, divider);
        g_tSequence += DURATION;
    }

    unsigned divider;

    static const unsigned DURATION = 5;
};

class dds_cmd : public pulse_cmd {
public:
    virtual ~dds_cmd() {}
    dds_cmd(unsigned dds, unsigned operand) : dds(dds), operand(operand)
    {
        if (dds > NDDS - 1) {
            throw std::runtime_error("Line " + std::to_string(g_lineNum) +
                                     ", Invalid DDS: " + std::to_string(dds));
        }
    }

    unsigned dds, operand;
    static const unsigned DURATION = PULSER_T_DDS_MIN;
};

class set_freq_cmd : public dds_cmd {
public:
    virtual ~set_freq_cmd() {}
    set_freq_cmd(unsigned dds, unsigned ftw) : dds_cmd(dds, ftw) {}
    set_freq_cmd(unsigned dds, double f) : dds_cmd(dds, Hz2FTW(f, dds_clk(dds))) {}

    virtual void makePulse()
    {
        DDS_set_ftw(dds, operand, v);
    }
};

class set_amp_cmd : public dds_cmd {
public:
    virtual ~set_amp_cmd() {}
    set_amp_cmd(unsigned dds, unsigned atw) : dds_cmd(dds, atw) {}
    set_amp_cmd(unsigned dds, double A) :
        dds_cmd(dds, 0x0FFF & (int)(A * 4095 + 0.5))
    {}

    virtual void makePulse() {
        DDS_set_atw(dds, operand, v);
    }
};

class set_phase_cmd : public dds_cmd {
public:
    virtual ~set_phase_cmd() {}
    set_phase_cmd(unsigned dds, unsigned ptw) : dds_cmd(dds, ptw) {}
    set_phase_cmd(unsigned dds, double phi) : dds_cmd(dds, 0xFFFF & (int)(phi*65536/360.0 + 0.5) ) {}

    virtual void makePulse() {
        DDS_set_ptw(dds, operand, v);
    }
};

class shift_phase_cmd : public dds_cmd {
public:
    virtual ~shift_phase_cmd()
    {}
    shift_phase_cmd(unsigned dds, unsigned ptw) :
        dds_cmd(dds, ptw)
    {}
    shift_phase_cmd(unsigned dds, double phi) :
        dds_cmd(dds, 0xFFFF & (int)(phi * 65536 / 360.0 + 0.5))
    {}

    virtual void makePulse() {
        DDS_shift_ptw(dds, operand, v);
    }
};

class dds_reset_cmd : public pulse_cmd {
public:
    virtual ~dds_reset_cmd() {}
    dds_reset_cmd(unsigned dds) : dds(dds) {}

    virtual void makePulse() {
        PULSER_dds_reset(pulser, dds);

        //disable programmable modulus, enable profile 0, enable SYNC_CLK output
        PULSER_set_dds_two_bytes(pulser, dds, 0x05, 0x840B);

        //enable amplitude control (OSK)
        PULSER_set_dds_two_bytes(pulser, dds, 0x0, 0x0108);

        g_tSequence += DURATION;
    }

    static const unsigned DURATION = 90;
    unsigned dds, ftw;
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
static inline C*
parse_pulse(std::vector<pulse_cmd*>, unsigned t, std::string &arg1,
            std::istream &s)
{
    int channel = -1;
    double operand = 0;

    C *pulse = 0;

    if (get_channel_and_operand(arg1, s, &channel, &operand)) {
        dealWithCurrentTTL(t, C::DURATION);
        pulse = new C(channel, operand);
        pulses.push_back(pulse);
        tCurr = t;
    }

    return pulse;
}

//setup "current" TTL, now that the end time is known
static void
makeCurrTTL(unsigned tEnd)
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

        pulses.push_back(new ttl_pulse_cmd(tEnd - tCurr, nextTTL));
        currTTL = nextTTL;
    }
}

static void
setTTL(unsigned t, unsigned channel, unsigned value)
{
    makeCurrTTL(t);

    if (value) {
        nextTTL |= 1 << channel;
    } else {
        nextTTL &= ~(1 << channel);
    }
    tCurr = t;
    hasTTL = true;
}

static void
setTTLall(unsigned t, unsigned value)
{
    makeCurrTTL(t);

    nextTTL = value;
    tCurr = t;
    hasTTL = true;
}

static void
finishTTL()
{
    if (hasTTL) {
        //disable timing check prior to last pulse
        pulses.push_back(new disable_timing_check_cmd());
        pulses.push_back(new ttl_pulse_cmd(PULSER_T_TTL_MIN, nextTTL));
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
dealWithCurrentTTL(unsigned tNewPulse, unsigned tMinPulse)
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
            pulses.push_back(new ttl_pulse_cmd(tNewPulse-tCurr-tMinPulse, nextTTL));
            tCurr = tNewPulse - tMinPulse;
            currTTL = nextTTL;
        }
    } else {
        throw std::runtime_error("Must run a TTL pulse prior to pulse at t = " +
                                 std::to_string(tNewPulse * PULSER_DT_us));
    }
}


static bool
parseReset(unsigned t, std::string& arg1, std::istream& s)
{
    std::string line;
    getline(s, line);

    if(!line.length())
        return false;


    int channel = -1;
    if(sscanf(arg1.c_str(), " %d", &channel)) {
        dealWithCurrentTTL(t, dds_reset_cmd::DURATION);

        pulses.push_back(new dds_reset_cmd(channel));
        tCurr = t;

        return true;
    }

    return false;
}

static bool
parseTTL(unsigned t, std::string &arg1, std::istream &s)
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
        setTTL(t, channel, ttl);
        return true;
    } else {
        if (arg1.find("all") != std::string::npos) {
            setTTLall(t, ttl);
            return true;
        }
    }

    return false;
}

static bool
parseClockOut(unsigned t, std::string &arg1, std::istream&)
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

    dealWithCurrentTTL(t, 0);
    tCurr += PULSER_T_TTL_MIN;

    pulses.push_back(new clock_out_cmd(divider));

    return true;
}

static bool
parseCommand(unsigned t, std::string &cmd, std::string &arg1, std::istream &s)
{
    if (pulse_cmd::v)
        pulse_cmd::v->printf("t = %8u x 10ns,  command = %10s,  arg1 = %4s\n", t,
                             cmd.c_str(), arg1.c_str());

    if (cmd.find("TTL") != std::string::npos)
        return parseTTL(t, arg1, s);

    if (cmd.find("freq") != std::string::npos)
        return 0 != parse_pulse<set_freq_cmd>(pulses, t, arg1, s);

    if (cmd.find("amp") != std::string::npos)
        return 0 != parse_pulse<set_amp_cmd>(pulses, t, arg1, s);

    if (cmd.find("phase") != std::string::npos)
        return 0 != parse_pulse<set_phase_cmd>(pulses, t, arg1, s);

    if (cmd.find("shiftp") != std::string::npos)
        return 0 != parse_pulse<shift_phase_cmd>(pulses, t, arg1, s);

    if (cmd.find("reset") != std::string::npos)
        return parseReset(t, arg1, s);

    if (cmd.find("CLOCK_OUT") != std::string::npos)
        return parseClockOut(t, arg1, s);

    return false;
}

//parse URL-encoded pulse sequence
bool
parseSeqURL(std::string &seq)
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
    if(end_pos == std::string::npos)
        end_pos = seq.length();

    std::string seqTxt = seq.substr(start_pos + L, end_pos - start_pos - L);
    html2txt(seqTxt, 1); //this is a slow function

    parseSeqTxt(reps, seqTxt, bForever, bDebugPulses);

    return true;
}

//parse pulse sequence via CGICC
bool
parseSeqCGI(cgicc::Cgicc& cgi)
{
    unsigned reps = getUnsignedParamCGI(cgi, "reps", 1);
    bool bDebugPulses = getCheckboxParamCGI(cgi, "debugPulses", false);
    bool bForever = getCheckboxParamCGI(cgi, "forever", false);

    //look for seqtext field
    std::string seqTxt = getStringParamCGI(cgi, "seqtext", "");
    if (seqTxt.length() == 0) {
        //if missing, look for attached file (multi-part)
        fprintf(gLog, "no seqtext parameter in form, looking for seqtext file\n");
        fprintf(gLog, "%d files attached\n", cgi.getFiles().size());

        cgicc::file_iterator i =  cgi.getFile("seqtext");
        if (i != cgi.getFiles().end()) {
            seqTxt = i->getData();
        } else {
            return false;
        }
    }

    parseSeqTxt(reps, seqTxt, bForever, bDebugPulses);

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
parseSeqTxt(unsigned reps, const std::string& seqTxt, bool bForever,
            bool bDebugPulses)
{
    printPlainResponseHeader();

    if (bDebugPulses) {
        fprintf(gLog, "Parsing pulse sequence:%s\n", seqTxt.c_str());
        fflush(gLog);
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

    while (!ss0.eof()) {
        //read line
        std::string line;
        getline(ss0, line);

        g_lineNum++;

        //ignore everything after '#' comment symbol
        size_t posC = line.find("#");
        if (posC != std::string::npos)
            line = line.substr(0, posC);

        //ignore blank lines
        if (line.length() == 0)
            continue;

        // otherwise parse the line
        std::stringstream ssL(line);

        double new_t;
        std::string strPrior;

        // valid lines will start with "dt = " or "t = "

        if (!eatStreamTo(ssL, '=', strPrior)) {
            throw std::runtime_error(std::to_string(g_lineNum) +
                                     ": no time spec.");
        }

        if (strPrior.find("dt") != std::string::npos) {
            ssL >> new_t;
            new_t += tSoFar;
        } else if (strPrior.find("t") != std::string::npos) {
            ssL >> new_t;
        } else {
            throw std::runtime_error(std::to_string(g_lineNum) +
                                     ": invalid time spec.");
        }
        if (new_t <= tSoFar) {
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

        if (parseCommand(floor(tSoFar * PULSER_DT_per_us + 0.5),
                         cmd, arg1, ssL)) {
            tSoFar = new_t;
        } else {
            throw std::runtime_error(std::to_string(g_lineNum) +
                                     ": failed to parse command.");
        }
    }
    unsigned new_tcurr = floor(tSoFar * PULSER_DT_per_us + 0.5);
    dealWithCurrentTTL(new_tcurr, 0);
    tCurr = new_tcurr;
    finishTTL();

    gvSTDOUT.printf("Parsed sequence into %d pulse commands.\n", pulses.size());

    if (bForever) {
        fprintf(gLog, "Start continuous run.\n");
    } else {
        fprintf(gLog, "Run %d sequences.\n", reps);
    }

    unsigned iRep;

    // Debug on slows down the sequence by a LOOOT.
    pulse_cmd::v = bDebugPulses ? &gvSTDOUT : 0;

    clock_t tClock1 = clock();

    // now run the pulses
    g_tSequence = 0;

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
            flocker fl(g_fPulserLock);

            setProgramStatus(0, buff);

            // hold the sequnce until pulse buffer is full or
            // PULSER_wait_for_finished is called
            PULSER_set_hold(pulser);

            PULSER_toggle_init(pulser);
            PULSER_enable_timing_check(pulser);

            for (pulse_cmd *p : pulses) {
                p->makePulse();
            }

            // wait for pulses finished.
            // gcc -02 and -O3 crashes here
            // (compiler bug? or volatile keyword is needed somewhere)
            PULSER_wait_for_finished(pulser);

            if (!PULSER_timing_ok(pulser)) {
                PULSER_clear_timing_check(pulser);
                nTimingErrors++;
            }
        }

        if (g_stop_curr_seq) {
            gvSTDOUT.printf("Received stop pulse sequences signal.\n");
            g_stop_curr_seq = false;
            break;
        }

        //only log debug info for 1st sequence
        pulse_cmd::v = 0;
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
findNextDelim(FILE* f, const char* delim)
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
                fprintf(gLog, "Retrieving quote %s\n", fname);
                fflush(gLog);
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
