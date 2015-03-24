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

#include <nacs-old-pulser/sequence.h>
#include <nacs-old-pulser/commands.h>
#include <nacs-utils/log.h>
#include <nacs-utils/timer.h>

#include <sstream>
#include <string>
#include <stdexcept>
#include <mutex>
#include <random>

#include "AD9914.h"
#include "string_func.h"

#include "parseMisc.h"
#include "saveloadmap.h"
#include "linux_file_util.h"

#include <common.h>

// keep track of "current" TTL state and elapsed time
// "current" refers to parsing the pulse sequence (not generating it)

namespace NaCs {

//parse text-encoded pulse sequence
static bool parseSeqTxt(Pulser::Pulser &pulser, unsigned reps,
                        const std::string &seqTxt, bool bForever,
                        bool debugPulses, const verbosity &reply);

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

static bool
parseReset(Pulser::SequenceBuilder &builder, uint64_t t, std::string &arg1,
           std::istream &s)
{
    std::string line;
    getline(s, line);

    if (!line.length())
        return false;

    int channel = -1;
    if (sscanf(arg1.c_str(), " %d", &channel)) {
        // Duration of the reset command
        builder.handle_curr_ttl(t, 90);
        builder.dds_reset(channel);

        // disable programmable modulus, enable profile 0,
        // enable SYNC_CLK output
        Pulser::setDDSTwoBytes(builder, channel, 0x05, 0x840B);

        // enable amplitude control (OSK)
        Pulser::setDDSTwoBytes(builder, channel, 0x0, 0x0108);
        builder.curr_t() = t;
        return true;
    }
    return false;
}

static bool
parseClockOut(Pulser::SequenceBuilder &builder, uint64_t t, std::string &arg1)
{
    int divider = 0;

    if (arg1.find("off") == 0) {
        divider = 255;
    } else {
        divider = atoi(arg1.c_str()) - 1;
    }

    if (divider < 0 || divider > 255) {
        nacsError("Error at t = %6.3Lf us.  "
                  "CLOCK_OUT accepts parameter off or 1 ... 255\n",
                  (long double)t * PULSER_DT_us);
        throw std::runtime_error("Bad CLOCK_OUT parameter.");
    }
    builder.handle_curr_ttl(t, 0);
    builder.curr_t() += PULSER_ENABLE_CLK_DURATION;
    Pulser::clockOut(builder, divider);
    return true;
}

static bool
parseTTL(Pulser::SequenceBuilder &builder, uint64_t t,
         std::string &arg1, std::istream &s)
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
        builder.push_ttl(t, channel, ttl);
        return true;
    } else {
        if (arg1.find("all") != std::string::npos) {
            builder.push_ttl_all(t, ttl);
            return true;
        }
    }

    return false;
}

template<typename Func>
static NACS_INLINE bool
parseDDS(Pulser::SequenceBuilder &builder, uint64_t t, std::string &arg1,
         std::istream &s, Func &&cb)
{
    int chn = -1;
    double operand = 0;

    if (get_channel_and_operand(arg1, s, &chn, &operand)) {
        if (chn > PULSER_NDDS - 1) {
            throw std::runtime_error("Line " +
                                     std::to_string(builder.line_num()) +
                                     ", Invalid DDS: " +
                                     std::to_string(chn));
        }
        builder.handle_curr_ttl(t, PULSER_T_DDS_MIN);
        cb(chn, operand);
        builder.curr_t() = t;
        return true;
    }
    return false;
}

static bool
parseCommand(Pulser::SequenceBuilder &builder, uint64_t t, std::string &cmd,
             std::string &arg1, std::istream &s)
{
    if (cmd.find("TTL") != std::string::npos)
        return parseTTL(builder, t, arg1, s);

    if (cmd.find("freq") != std::string::npos) {
        return parseDDS(builder, t, arg1, s, [&] (int chn, double freq) {
                setDDSFreqF(builder, chn, freq);
            });
    }

    if (cmd.find("amp") != std::string::npos) {
        return parseDDS(builder, t, arg1, s, [&] (int chn, double amp) {
                Pulser::setDDSAmpF(builder, chn, amp);
            });
    }

    if (cmd.find("phase") != std::string::npos) {
        return parseDDS(builder, t, arg1, s, [&] (int chn, double phase) {
                builder.set_dds_phase_f(chn, phase);
            });
    }

    if (cmd.find("shiftp") != std::string::npos) {
        return parseDDS(builder, t, arg1, s, [&] (int chn, double phase) {
                builder.shift_dds_phase_f(chn, phase);
            });
    }

    if (cmd.find("reset") != std::string::npos)
        return parseReset(builder, t, arg1, s);

    if (cmd.find("CLOCK_OUT") != std::string::npos)
        return parseClockOut(builder, t, arg1);

    return false;
}

//parse URL-encoded pulse sequence
bool
parseSeqURL(Pulser::Pulser &pulser, std::string &seq, const verbosity &reply)
{
    unsigned reps = getUnsignedParam(seq, "reps=", 1);
    bool debugPulses = getCheckboxParam(seq, "debugPulses=", false);
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

    parseSeqTxt(pulser, reps, seqTxt, bForever, debugPulses, reply);

    return true;
}

//parse pulse sequence via CGICC
bool
parseSeqCGI(Pulser::Pulser &pulser, cgicc::Cgicc &cgi, const verbosity &reply)
{
    unsigned reps = getUnsignedParamCGI(cgi, "reps", 1);
    bool debugPulses = getCheckboxParamCGI(cgi, "debugPulses", false);
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

    parseSeqTxt(pulser, reps, seqTxt, bForever, debugPulses, reply);

    return true;
}

// parse text-encoded pulse sequence
static bool
parseSeqTxt(Pulser::Pulser &pulser, unsigned reps,
            const std::string &seqTxt, bool bForever, bool debugPulses,
            const verbosity &reply)
{
    printPlainResponseHeader(reply);

    if (debugPulses) {
        nacsLog("Parsing pulse sequence:%s\n", seqTxt.c_str());
    }

    tic();

    Pulser::SequenceBuilder builder;
    builder.enable_timing_check();

    // first parse and load up the pulses vector
    std::stringstream ss0(seqTxt);

    // long double and double are the same on ARM
    // Both should be precise enough (2^-52 or ~1 ps in 1 hour)
    long double tSoFar = 0;
    bool use_dt = true;

    while (!ss0.eof()) {
        // read line
        std::string line;
        getline(ss0, line);

        builder.line_num()++;

        // ignore everything after '#' comment symbol
        size_t posC = line.find("#");
        if (posC != std::string::npos)
            line = line.substr(0, posC);

        // ignore blank lines
        if (line.length() == 0)
            continue;

        // otherwise parse the line
        std::stringstream ssL(line);

        long double new_t;
        std::string strPrior;

        // valid lines will start with "dt = " or "t = "

        if (!eatStreamTo(ssL, '=', strPrior)) {
            continue;
            // throw std::runtime_error(std::to_string(builder.line_num()) +
            //                          ": no time spec.");
        }

        if (strPrior.find("dt") != std::string::npos) {
            use_dt = true;
            ssL >> new_t;
            new_t += tSoFar;
        } else if (strPrior.find("t") != std::string::npos) {
            use_dt = false;
            ssL >> new_t;
        } else {
            throw std::runtime_error(std::to_string(builder.line_num()) +
                                     ": invalid time spec.");
        }
        if (new_t <= tSoFar && tSoFar > 0) {
            throw std::runtime_error(std::to_string(builder.line_num()) +
                                     ": going back in time.");
        }

        // next comes the time unit (not used), then a comma
        std::string timeunit;
        getline(ssL, timeunit, ',');

        if (ssL.eof()) {
            throw std::runtime_error(std::to_string(builder.line_num()) +
                                     ": no action.");
        }

        // then comes the command name, followed by '(arg1)'
        std::string cmd;
        getline(ssL, cmd, '(');
        if (ssL.eof()) {
            throw std::runtime_error(std::to_string(builder.line_num()) +
                                     ": incomplete action.");
        }

        std::string arg1;
        getline(ssL, arg1, ')');
        if (ssL.eof()) {
            throw std::runtime_error(std::to_string(builder.line_num()) +
                                     ": incomplete action (2).");
        }

        if (!use_dt) {
            tSoFar = new_t;
        }
        uint64_t t_count =
            static_cast<uint64_t>(tSoFar * PULSER_DT_per_us + 0.5);
        if (parseCommand(builder, t_count, cmd, arg1, ssL)) {
            tSoFar = new_t;
        } else {
            throw std::runtime_error(std::to_string(builder.line_num()) +
                                     ": failed to parse command.");
        }
    }
    if (use_dt) {
        uint64_t t_count =
            static_cast<uint64_t>(tSoFar * PULSER_DT_per_us + 0.5);
        builder.handle_curr_ttl(t_count, 0);
    }
    builder.finish_ttl();

    auto parse_time = toc();

    reply.printf("Parsed sequence into a %zu bytes program.\n",
                 builder.len() * sizeof(uint32_t));

    if (bForever) {
        nacsLog("Start continuous run.\n");
    } else {
        nacsLog("Run %d sequences.\n", reps);
    }

    tic();

    // now run the pulses
    // update status string every 500 ms
    auto seq_len_ms = (long double)builder.curr_t() * PULSER_DT_us * 1e-3l;
    unsigned nTimingErrors = 0;
    unsigned iRep;
    char buff[64] = {'\0'};
    for (iRep = 0;iRep < reps || bForever;iRep++) {
        if (bForever) {
            snprintf(buff, 64, "Running sequence %d", iRep);
        } else {
            snprintf(buff, 64, "Running sequence %d / %d", iRep, reps);
        }
        setProgramStatus(buff);

        std::lock_guard<FLock> fl(g_fPulserLock);
        // hold the sequnce until pulse buffer is full or
        // pulser.wait() is called
        pulser.set_hold();
        pulser.toggle_init();
        pulser.run(builder);

        // wait for pulses finished.
        pulser.wait();

        if (!pulser.timing_ok()) {
            Pulser::clearTimingCheck(pulser);
            nTimingErrors++;
        }

        if (g_stop_curr_seq) {
            reply.printf("Received stop pulse sequences signal.\n");
            g_stop_curr_seq = false;
            break;
        }
    }

    auto run_time = toc();

    reply.printf("Finished %d/%d pulse sequences.\n", iRep, reps);

    if (nTimingErrors == 0) {
        reply.printf("Timing OK\n");
    } else {
        reply.printf("Warning: %d timing failures.\n", nTimingErrors);
    }

    reply.printf("          Parser time: %9.3Lf ms\n",
                 (long double)parse_time * 1e-6)
        .printf("       Execution time: %9.3Lf ms\n",
                (long double)run_time * 1e-6)
        .printf("Duration of sequences: %9.3Lf ms\n", iRep * seq_len_ms);
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
    static std::random_device rd;
    static std::mutex lock;
    std::unique_lock<std::mutex> locker(lock, std::defer_lock);
    NACS_RET_IF_FAIL(locker.try_lock(), "");

    static time_t tLastQuote = time(0);
    time_t tNow = time(0);

    if (tNow > tLastQuote + 30) {
        //no wasting time in the lab
        if (std::uniform_int_distribution<int>(0, 1)(rd)) {
            tLastQuote = tNow;
            std::string s;

            FILE *f = fopen(fname, "r");

            if (f) {
                nacsLog("Retrieving quote %s\n", fname);
                //get file length
                fseek(f, 0, SEEK_END);
                size_t len = ftell(f);
                auto pos = std::uniform_int_distribution<size_t>(0, len)(rd);

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
