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

#include <nacs-pulser/instruction.h>
#include <nacs-utils/log.h>
#include <nacs-utils/timer.h>
#include <nacs-utils/base64.h>
#include <nacs-seq/pulser.h>

#include <sstream>
#include <string>
#include <stdexcept>
#include <mutex>
#include <random>

#include "AD9914.h"

#include "parseMisc.h"
#include "saveloadmap.h"
#include "linux_file_util.h"

#include "molecube.h"

namespace NaCs {

using Inst = Pulser::InstWriter;

//parse text-encoded pulse sequence
static bool parseSeqTxt(Pulser::Controller &pulser, unsigned reps,
                        const std::string &seqTxt, bool bForever,
                        const verbosity &reply, FCGX_Request *request);

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

static auto
parseError(Pulser::BlockBuilder &builder, std::string &&text)
{
    return std::runtime_error("L" + std::to_string(builder.lineNum) +
                              ": " + text);
}

static Pulser::Instruction
parseReset(Pulser::BlockBuilder &builder, std::string &arg1, std::istream &s,
           uint64_t *tp)
{
    std::string line;
    getline(s, line);
    int channel = -1;

    if (!line.length() || !sscanf(arg1.c_str(), " %d", &channel)) {
        throw parseError(builder, "Failed to parse reset command.");
    }
    return Inst::DDS::reset(channel, tp);
}

static Pulser::Instruction
parseClockOut(Pulser::BlockBuilder &builder, std::string &arg1, uint64_t *tp)
{
    int divider = 0;

    if (arg1.find("off") == 0) {
        divider = 255;
    } else {
        divider = atoi(arg1.c_str()) - 1;
    }

    if (divider < 0 || divider > 255) {
        throw parseError(builder, "Bad CLOCK_OUT parameter");
    }
    return Inst::clockOut(divider, tp);
}

static Pulser::Instruction
parseDACSetVolt(Pulser::BlockBuilder &builder, std::string &arg1,
                std::istream &s, uint64_t *tp)
{
    int chn = -1;
    double operand = 0;

    if (get_channel_and_operand(arg1, s, &chn, &operand)) {
        if (chn > 3)
            throw parseError(builder,
                             "Invalid DAC (" + std::to_string(chn) + ")");
        return Inst::dacSetVolt(uint8_t(chn), operand, tp);
    }
    throw parseError(builder, "Failed to parse DAC command.");
}

static Pulser::Instruction
parseTTL(Pulser::BlockBuilder &builder, std::string &arg1, std::istream &s,
         uint64_t *tp)
{
    std::string line;
    getline(s, line);
    unsigned ttl;

    if (!line.length() || !sscanf(line.c_str(), " = %x", &ttl)) {
        throw parseError(builder, "Failed to parse TTL command.");
    }

    int channel = -1;
    if (sscanf(arg1.c_str(), " %d", &channel)) {
        return Inst::ttl(uint8_t(channel), ttl, tp);
    } else {
        if (arg1.find("all") != std::string::npos) {
            return Inst::ttlAll(ttl, tp);
        }
    }
    throw parseError(builder, "Failed to parse TTL command.");
}

template<typename Func>
static Pulser::Instruction
parseDDS(Pulser::BlockBuilder &builder, std::string &arg1, std::istream &s,
         Func &&cb, uint64_t *tp)
{
    int chn = -1;
    double operand = 0;

    if (get_channel_and_operand(arg1, s, &chn, &operand)) {
        if (chn > PULSER_NDDS - 1) {
            throw parseError(builder,
                             "Invalid DDS (" + std::to_string(chn) + ")");
        }
        return cb(chn, operand, tp);
    }
    throw parseError(builder, "Failed to parse DDS command.");
}

static Pulser::Instruction
parseCommand(Pulser::BlockBuilder &builder, std::string &cmd,
             std::string &arg1, std::istream &s, uint64_t *tp)
{
    if (cmd.find("TTL") != std::string::npos)
        return parseTTL(builder, arg1, s, tp);

    if (cmd.find("freq") != std::string::npos)
        return parseDDS(builder, arg1, s, Inst::DDS::setFreq, tp);

    if (cmd.find("amp") != std::string::npos)
        return parseDDS(builder, arg1, s, Inst::DDS::setAmp, tp);

    if (cmd.find("phase") != std::string::npos)
        return parseDDS(builder, arg1, s, Inst::DDS::setPhase, tp);

    if (cmd.find("shiftp") != std::string::npos)
        return parseDDS(builder, arg1, s, Inst::DDS::shiftPhase, tp);

    if (cmd.find("reset") != std::string::npos)
        return parseReset(builder, arg1, s, tp);

    if (cmd.find("CLOCK_OUT") != std::string::npos)
        return parseClockOut(builder, arg1, tp);

    if (cmd.find("dac") != std::string::npos)
        return parseDACSetVolt(builder, arg1, s, tp);

    throw parseError(builder, "Unknown command.");
}

//parse URL-encoded pulse sequence
bool
parseSeqURL(Pulser::Controller &ctrl, std::string &seq, const verbosity &reply)
{
    unsigned reps = getUnsignedParam(seq, "reps=", 1);
    bool bForever = getCheckboxParam(seq, "forever=", false);

    size_t start_pos = seq.find("seqtext=");
    size_t L = std::string("seqtext=").length();

    if (start_pos == std::string::npos) { // not a URLENCODEd sequence
        return false;
    }

    size_t end_pos = seq.find("&", start_pos + L);
    if (end_pos == std::string::npos)
        end_pos = seq.length();

    std::string seqTxt = seq.substr(start_pos + L, end_pos - start_pos - L);
    html2txt(seqTxt, 1); //this is a slow function

    parseSeqTxt(ctrl, reps, seqTxt, bForever, reply, nullptr);

    return true;
}

// parse pulse sequence via CGICC
bool
parseSeqCGI(Pulser::Controller &ctrl, cgicc::Cgicc &cgi, const verbosity &reply,
            FCGX_Request *request)
{
    unsigned reps = getUnsignedParamCGI(cgi, "reps", 1);
    bool bForever = getCheckboxParamCGI(cgi, "forever", false);

    // look for seqtext field
    std::string seqTxt = getStringParamCGI(cgi, "seqtext", "");
    if (seqTxt.length() == 0) {
        // if missing, look for attached file (multi-part)
        nacsLog("%d files attached\n", cgi.getFiles().size());

        cgicc::file_iterator i = cgi.getFile("seqtext");
        if (i != cgi.getFiles().end()) {
            seqTxt = i->getData();
        } else {
            return false;
        }
    }

    parseSeqTxt(ctrl, reps, seqTxt, bForever, reply, request);

    return true;
}

static void parsePlainTxt(const std::string &seqTxt,
                          Pulser::BlockBuilder &builder)
{
    // first parse and load up the pulses vector
    std::stringstream ss0(seqTxt);

    while (!ss0.eof()) {
        // read line
        std::string line;
        getline(ss0, line);

        builder.lineNum++;

        // ignore everything after '#' comment symbol
        size_t posC = line.find("#");
        if (posC != std::string::npos)
            line = line.substr(0, posC);

        // ignore blank lines
        if (line.length() == 0)
            continue;

        // otherwise parse the line
        std::stringstream ssL(line);

        std::string strPrior;

        // valid lines will start with "dt = " or "t = "

        if (!eatStreamTo(ssL, '=', strPrior)) {
            continue;
        }

        bool use_dt;
        double __new_t;
        ssL >> __new_t;
        uint64_t new_t = uint64_t(__new_t * PULSER_DT_per_us);
        if (strPrior.find("dt") != std::string::npos) {
            use_dt = true;
        } else if (strPrior.find("t") != std::string::npos) {
            use_dt = false;
        } else {
            throw parseError(builder, "Invalid time spec.");
        }

        // next comes the time unit (not used), then a comma
        std::string timeunit;
        getline(ssL, timeunit, ',');

        if (ssL.eof()) {
            throw parseError(builder, "No action.");
        }

        // then comes the command name, followed by '(arg1)'
        std::string cmd;
        getline(ssL, cmd, '(');
        if (ssL.eof()) {
            throw parseError(builder, "Incomplete action.");
        }

        std::string arg1;
        getline(ssL, arg1, ')');
        if (ssL.eof()) {
            throw parseError(builder, "Incomplete action (2).");
        }

        if (use_dt) {
            builder.pulseDT(new_t, parseCommand, builder, cmd, arg1, ssL);
        } else {
            builder.pulseAbsT(new_t, parseCommand, builder, cmd, arg1, ssL);
        }
    }
}

static void parseBase64Txt(const std::string &seqTxt,
                           Pulser::BlockBuilder &builder)
{
    const uint8_t *data = ((const uint8_t*)seqTxt.data()) + 1;
    size_t data_len = seqTxt.size() - 1;
    if (!Base64::validate(data, data_len))
        throw parseError(builder, "Invalid Base64 encoding");
    builder.fromSeq(Seq::Sequence::fromBase64(data, data_len));
}

// 256MB cache
static StrCache<Pulser::BlockBuilder> seq_cache((size_t)256e6);

// parse text-encoded pulse sequence
static bool
parseSeqTxt(Pulser::Controller &ctrl, unsigned reps,
            const std::string &seqTxt, bool bForever,
            const verbosity &reply, FCGX_Request *request)
{
    printPlainResponseHeader(reply);
    if (bForever)
        reps = UINT_MAX;

    nacsLog("Parsing pulse sequence: %s\n", seqTxt.c_str());

    for (auto i: active_dds) {
        if (AD9914::init(ctrl, i, AD9914::LogAction)) {
            nacsLog("DDS %d reinit\n", i);
            AD9914::print_registers(ctrl, i);
        }
    }

    tic();

    Pulser::BlockBuilder _builder;

    const Pulser::BlockBuilder *builder_p = seq_cache.get(seqTxt);

    uint64_t parse_time;
    const bool is_b64 = seqTxt[0] == '=';
    if (!builder_p) {
        if (is_b64) {
            parseBase64Txt(seqTxt, _builder);
        }
        else {
            parsePlainTxt(seqTxt, _builder);
        }
        parse_time = toc();
        // Only cache the sequence if it takes longer than 50ms and
        // 20% of the sequence length to parse.
        if (is_b64 && parse_time > (uint64_t)50e6 &&
            parse_time > _builder.currT * 2)
            builder_p = seq_cache.set(seqTxt, std::move(_builder));
        if (!builder_p) {
            builder_p = &_builder;
        }
    } else {
        parse_time = toc();
    }

    reply.printf("Parsed into %zu pulses.\n", builder_p->size());

    if (bForever) {
        nacsLog("Start continuous run.\n");
    } else if (reps != 1) {
        nacsLog("Run %d sequences.\n", reps);
    }

    tic();

    // now run the pulses
    // update status string every 500 ms
    auto seq_len_ms = double(builder_p->currT) * PULSER_DT_us * 1e-3;
    bool terminate_request = reps == 1 && seq_len_ms <= 1000 && request;

    unsigned nTimingErrors = 0;
    unsigned iRep;
    Pulser::CtrlLocker locker(ctrl);
    for (iRep = 0;iRep < reps || bForever;iRep++) {
        if (reps != 1 || seq_len_ms > 500) {
            char buff[64] = {'\0'};
            if (bForever) {
                snprintf(buff, 64, "Running sequence %d", iRep);
            } else {
                snprintf(buff, 64, "Running sequence %d / %d", iRep, reps);
            }
            setProgramStatus(buff);
        }

        // hold the sequnce until pulse buffer is full or
        // ctrl.waitFinish() is called
        ctrl.setHold();
        ctrl.toggleInit();
        Pulser::CtrlState state;
        Pulser::runInstructionList(&ctrl, &state, *builder_p);

        if (terminate_request) {
            ctrl.releaseHold();
            // If the sequence is short and we are only running it once,
            // reply as soon as possible.
            reply.printf("Sequence started\n");
            // This might be causing memory issues...
            // FCGX_Finish_r(request);
            nacsLog("Parse time: %9.3f ms\n", (double)parse_time * 1e-6);
            nacsLog("   Seq len: %9.3f ms\n", iRep * seq_len_ms);

            ctrl.waitFinish();
            auto run_time = toc();
            if (!ctrl.timingOK()) {
                ctrl.run(Pulser::ClearTimingCheck());
                nacsLog("Warning: timing failures.\n");
            }
            setProgramStatus("Idle");
            nacsLog("  Exe time: %9.3f ms\n", (double)run_time * 1e-6);
            return true;
        }

        // wait for pulses finished.
        ctrl.waitFinish();

        if (!ctrl.timingOK()) {
            ctrl.run(Pulser::ClearTimingCheck());
            nTimingErrors++;
        }

        if (g_stop_curr_seq) {
            reply.printf("Received stop pulse sequences signal.\n");
            g_stop_curr_seq = false;
            break;
        }
    }

    auto run_time = toc();

    if (reps == 1) {
        reply.printf("Finished 1 pulse sequences.\n");
    }
    else {
        reply.printf("Finished %d/%d pulse sequences.\n", iRep, reps);
    }

    if (nTimingErrors) {
        reply.printf("Warning: %d timing failures.\n", nTimingErrors);
    } else if (reps != 1) {
        reply.printf("Timing OK\n");
    }
    setProgramStatus("Idle");

    reply.printf("Parse time: %9.3f ms\n", (double)parse_time * 1e-6)
        .printf("  Exe time: %9.3f ms\n", (double)run_time * 1e-6)
        .printf("   Seq len: %9.3f ms\n", iRep * seq_len_ms);
    return true;
}

void handleRunByteCode(Pulser::Controller &ctrl, uint64_t seq_len_ns,
                       const uint8_t *code, size_t code_len,
                       const std::function<void()> &send_reply)
{
    tic();
    nacsLog("Start sequence %" PRIu64 " ns.\n", seq_len_ns);

    // less than 1s
    bool short_seq = seq_len_ns <= 1000 * 1000 * 1000;

    if (!short_seq)
        setProgramStatus("Running sequence 1 / 1");

    Pulser::CtrlLocker locker(ctrl);
    // hold the sequnce until pulse buffer is full or
    // ctrl.waitFinish() is called
    ctrl.setHold();
    ctrl.toggleInit();
    Pulser::runByteCode(&ctrl, code, code_len);
    ctrl.releaseHold();

    if (short_seq) {
        // If the sequence is short and we are only running it once,
        // reply as soon as possible.
        send_reply();
    }
    // wait for pulses finished.
    ctrl.waitFinish();
    if (!short_seq)
        send_reply();

    auto run_time = toc();
    if (!ctrl.timingOK())
        nacsLog("Warning: timing failures.\n");

    Pulser::runEpilogue(&ctrl);
    nacsLog("Exe time: %9.3f ms\n", (double)run_time * 1e-6);

    // Doing this check before this sequence will make the current sequence
    // more likely to work. However, that increase the latency and the DDS
    // reset only happen very infrequently so let's do it after the sequence
    // for better efficiency.
    for (auto i: active_dds) {
        if (AD9914::init(ctrl, i, AD9914::LogAction)) {
            nacsLog("DDS %d reinit\n", i);
            AD9914::print_registers(ctrl, i);
        }
    }
    setProgramStatus("Idle");
}

}
