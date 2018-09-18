//

#include "molecube.h"

#include "init_system.h"
#include "linux_file_util.h"
#include "parseTxtSeq.h"
#include "parseMisc.h"

#include "CmdLineArgs.h"

#include <nacs-utils/timer.h>
#include <nacs-utils/log.h>
#include <nacs-utils/zmq_utils.h>
#include <nacs-pulser/controller.h>

#include <stdexcept>
#include <fstream>
#include <map>
#include <atomic>

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>

#include <fcgio.h>
#include <cgicc/CgiInput.h>

using namespace NaCs;

// TR: Linker failed when FCgiIO was in a separate header (undefined ref to vtable),
//     so I'm pasting it here.  Removed pointers to stderr and stdout streams.
//      This came from cgicc/contrib/FCgiIO.h , .cpp

// Yichao Yu: this effectively makes the whole program LGPL LOOOOOOL

// ============================================================
// Class FCgiIO
// ============================================================

/*! \class FCgiIO FCgiIO.h FCgiIO.h
 * \brief Class that implements input and output through a FastCGI request.
 *
 * This class provides access to the input byte-stream and environment
 * variable interfaces of a FastCGI request.  It is fully compatible with the
 * Cgicc input API.
 *
 * It also provides access to the request's output and error streams, using a
 * similar interface.
 */
class FCgiIO: public cgicc::CgiInput {
    FCgiIO(const FCgiIO &io) = delete;
public:
    /*! \name Constructor */
    /*!
     * \brief Constructor
     *
     * Create a new FCgiIO object
     */
    FCgiIO(FCGX_Request &request) :
        cgicc::CgiInput(),
        m_request(request)
    {
        // Parse environment
        for(char **e = m_request.envp; *e != nullptr;e++) {
            std::string s(*e);
            std::string::size_type i = s.find('=');
            if (i == std::string::npos)
                throw std::runtime_error("Illegally formed environment");
            m_env[s.substr(0, i)] = s.substr(i + 1);
        }
    }

    /*! \name Data Sources */
    /*!
     * \brief Read data from the request's input stream.
     *
     * \param data The target buffer
     * \param length The number of characters to read
     * \return The number of characters read
     */
    size_t
    read(char *data, size_t length) override
    {
        return FCGX_GetStr(data, (int)length, m_request.in);
    }

    /*!
     * \brief Query the value of an environment variable stored in the request.
     *
     * \param varName The name of an environment variable
     * \return The value of the requested environment variable, or an empty
     * string if not found.
     */
    std::string
    getenv(const char *varName) override
    {
        return m_env[varName];
    }
protected:
    FCGX_Request &m_request;
    std::map<std::string, std::string> m_env;
};

namespace NaCs {

volatile bool g_stop_curr_seq = false;
std::vector<unsigned> active_dds; // all DDS that are available

static void
handleINT(int)
{
    exit(0);
}

static void
handleUSR1(int)
{
    g_stop_curr_seq = true;
}

static void
printHeader(FILE *fLog)
{
    fprintf(fLog, "\n\nMolecube 1.09 (FastCGI)\n");
    fprintf(fLog, "Built: %s %s  ", __DATE__, __TIME__);
#ifdef __clang__
    fprintf(fLog, "with Clang %d.%d.%d\n", __clang_major__,
            __clang_minor__, __clang_patchlevel__);
#else
    fprintf(fLog, "with GCC %d.%d.%d\n", __GNUC__,
            __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif
}

static void
printUsage()
{
    printHeader(stdout);
    printf("Command line options:\n");
    printf(" -l log_file_name : Set log file.  "
           "If none specified, use stdout.\n");
    printf(" -s startup_file_name : "
           "If specified, run startup pulse sequence from file.\n");
    printf(" -s zmq address to listen to.\n");
    printf(" -h or --help : Print help / usage info.\n");
    printf("\n\n");
}

static inline int
getRequestId()
{
    static std::atomic<int> id(0);
    return id.fetch_add(1, std::memory_order_relaxed);
}

}

using namespace NaCs;

int
main(int argc, char *argv[])
{
    // handler gets called for ctrl-C or similar kill signals
    signal(SIGINT, &handleINT);
    // USR1: stop current pulse sequence
    signal(SIGUSR1, &handleUSR1);

    //get command line args
    CmdLineArgs cla(argc, argv);

    if (cla.FindString("-h") >= 0 || cla.FindString("--help") >= 0) {
        printUsage();
        return 0;
    }

    std::string sLogFileName = cla.GetStringAfter("-l", "stdout");
    if (sLogFileName == "stdout") {
        Log::setLog(stdout);
    } else {
        Log::setLog(fopen(sLogFileName.c_str(), "a"));
    }

    printHeader(Log::getLog());

    setProgramStatus("Initializing");

    auto &ctrl = init_system();
    FCGX_Init();

    // run startup sequence
    std::string fnameStartup = cla.GetStringAfter("-s", "");
    if (fnameStartup.length()) {
        Log::log("Read startup sequence from: %s\n", fnameStartup.c_str());
        std::ifstream ifs(fnameStartup);

        if (ifs.is_open()) {
            std::string sStartupSeq;
            while (!ifs.eof()) {
                std::string line;
                getline(ifs, line);
                sStartupSeq.append(line);
                sStartupSeq.append("\n");
            }

            try {
                parseSeqURL(ctrl, sStartupSeq, std::cout);
            } catch (const std::runtime_error &e) {
                Log::error("Startup sequence error:   %s\n", e.what());
            }
        } else {
            Log::error("Could not open file.\n");
        }
    }

    Log::info("Waiting for network connections...\n\n");

    setProgramStatus("Idle");

    auto processRequests = [&] {
        FCGX_Request request;
        FCGX_InitRequest(&request, 0, 0);
        while (FCGX_Accept_r(&request) == 0) {
            auto request_id = getRequestId();
            Log::log("==== Accept FastCGI request %d ====\n", request_id);
            fcgi_streambuf out_fcgi_streambuf(request.out);
            std::ostream out(&out_fcgi_streambuf);
            FCgiIO IO(request);
            cgicc::Cgicc cgi(&IO);
            try {
                // May finish the requesst when no error happens
                if (!parseQueryCGI(ctrl, cgi, out)) {
                    Log::error("Couldn't understand HTTP request.\n");
                }
            } catch (const std::runtime_error &e) {
                out << "Oh noes! \n   " << e.what() << std::endl;
            }

            Log::log("==== Finish FastCGI request %d ====\n\n", request_id);
            out << std::endl;
        }
    };

    static constexpr size_t numWorkers = 8;
    std::vector<std::thread> workers;
    for (size_t i = 0;i < numWorkers;i++) {
        workers.emplace_back(processRequests);
    }
    std::string zmqaddr = cla.GetStringAfter("-z", "");
    if (!zmqaddr.empty()) {
        auto processZMQ = [&] {
            zmq::context_t ctx;
            zmq::socket_t sock(ctx, ZMQ_ROUTER);
            sock.bind(zmqaddr);
            zmq::message_t empty(0);
            auto send_reply_header = [&] (zmq::message_t &addr) {
                sock.send(addr, ZMQ_SNDMORE);
                sock.send(empty, ZMQ_SNDMORE);
            };
            auto send_reply = [&] (zmq::message_t &addr, auto &&msg) {
                send_reply_header(addr);
                sock.send(msg);
            };
            while (true) {
                zmq::message_t addr;
                sock.recv(&addr);

                auto request_id = getRequestId();
                Log::log("==== Accept ZMQ request %d ====\n", request_id);

                zmq::message_t msg;
                sock.recv(&msg);
                assert(msg.size() == 0);

                if (!ZMQ::recv_more(sock, msg)) {
                    Log::log("Empty request %d\n", request_id);
                    send_reply(addr, ZMQ::bits_msg(uint64_t(0)));
                    goto out;
                }
                else if (ZMQ::match(msg, "run_seq")) {
                    if (!ZMQ::recv_more(sock, msg) || msg.size() != 4) {
                        // No version
                        send_reply(addr, ZMQ::bits_msg(uint64_t(0)));
                        goto out;
                    }
                    uint32_t ttl_mask = 0;
                    uint32_t ver;
                    memcpy(&ver, msg.data(), 4);
                    size_t min_len;
                    if (ver == 0) {
                        min_len = 8;
                    }
                    else if (ver == 1) {
                        min_len = 12;
                    }
                    else {
                        // Wrong version
                        send_reply(addr, ZMQ::bits_msg(uint64_t(0)));
                        goto out;
                    }
                    if (!ZMQ::recv_more(sock, msg) || msg.size() <= min_len) {
                        // Not long enough
                        send_reply(addr, ZMQ::bits_msg(uint64_t(0)));
                        goto out;
                    }
                    uint64_t len_ns;
                    auto msg_data = (const uint8_t*)msg.data();
                    auto msg_sz = msg.size();
                    memcpy(&len_ns, msg_data, 8);
                    msg_data += 8;
                    msg_sz -= 8;
                    if (ver >= 1) {
                        memcpy(&ttl_mask, msg_data, 4);
                        msg_data += 4;
                        msg_sz -= 4;
                    }
                    handleRunByteCode(ctrl, len_ns, msg_data, msg_sz, [&] {
                            send_reply(addr, ZMQ::bits_msg(uint64_t(1)));
                        }, ttl_mask);
                }
                else {
                    Log::log("Unknown request %d\n", request_id);
                    send_reply(addr, ZMQ::bits_msg(uint64_t(0)));
                }
            out:
                ZMQ::readall(sock);
                Log::log("==== Finish ZMQ request %d ====\n\n", request_id);
            }
        };
        workers.emplace_back(std::move(processZMQ));
    }
    for (auto &t: workers) {
        t.join();
    }

    Log::log("Exit, return 0\n");
    setProgramStatus("Finished / Quit");
    return 0;
}
