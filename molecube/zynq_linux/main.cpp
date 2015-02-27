#include <nacs-utils/timer.h>
#include <nacs-utils/log.h>

#include "init_system.h"

#include "linux_file_util.h"

#include "../parser/parseTxtSeq.h"
#include "../parser/parseMisc.h"

#include <verbosity.h>
#include <CmdLineArgs.h>
#include <common.h>

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
#include <fcgi_config.h>
#include <cgicc/CgiInput.h>

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
            if(i == std::string::npos)
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
        return FCGX_GetStr(data, length, m_request.in);
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
FLock g_fPulserLock("/run/molecube/pulser.lock");
std::vector<unsigned> active_dds; // all DDS that are available
std::mutex GPL;

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
    fprintf(fLog, "with GCC %d.%d.%d\n", __GNUC__,
            __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
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
    printf(" -h or --help : Print help / usage info.\n");
    printf("\n\n");
}

static inline int
getRequestId()
{
    static std::atomic<int> id(0);
    return ++id;
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
        nacsSetLog(stdout);
    } else {
        nacsSetLog(fopen(sLogFileName.c_str(), "a"));
    }

    printHeader(nacsGetLog());

    setProgramStatus("Initializing");

    auto &pulser = init_system();

    time_t srandT = time(0);
    srand(srandT);
    nacsLog("Random seed = %u.  2 random numbers: %u, %u\n",
            (unsigned)srandT, rand(), rand());

    FCGX_Init();

    // run startup sequence
    std::string fnameStartup = cla.GetStringAfter("-s", "");
    if (fnameStartup.length()) {
        nacsLog("Read startup sequence from: %s\n", fnameStartup.c_str());
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
                parseSeqURL(pulser, sStartupSeq, verbosity(&std::cout));
            } catch (std::runtime_error e) {
                nacsError("Startup sequence error:   %s\n", e.what());
            }
        } else {
            nacsError("Could not open file.\n");
        }
    }

    nacsInfo("Waiting for network connections...\n\n");

    setProgramStatus("Idle");

    FCGX_Request request;
    FCGX_InitRequest(&request, 0, 0);
    while (FCGX_Accept_r(&request) == 0) {
        auto request_id = getRequestId();
        setProgramStatus("Processing request");
        nacsLog("================ Accept FastCGI request %d "
                "================\n", request_id);

        // Replace stdio streambufs.
        // Note that the default bufsize (0) will cause the use of iostream
        // methods that require positioning (such as peek(), seek(),
        // unget() and putback()) to fail (in favour of more efficient IO).
        fcgi_streambuf out_fcgi_streambuf(request.out);
        std::ostream out(&out_fcgi_streambuf);
        FCgiIO IO(request);
        cgicc::Cgicc cgi(&IO);
        verbosity reply(&out);
        LockGPL lock;
        try {
            if (!parseQueryCGI(pulser, cgi, reply)) {
                nacsError("Couldn't understand HTTP request.\n");
            }
        } catch (std::runtime_error e) {
            reply.printf("Oh noes! \n   %s\n", e.what())
                .printf("%s", getQuote("/usr/share/molecube/quotes.frt",
                                       "%%").c_str());
        }

        nacsLog("================ Finish FastCGI request %d "
                "================\n\n", request_id);
        out << std::endl;
        setProgramStatus("Idle");
    }

    nacsLog("Exit, return 0\n");
    setProgramStatus("Finished / Quit");
    return 0;
}
