#include <stdio.h>

#include "init_platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>

void init_system();


#include <cstdio>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <fstream>
#include <map>

#include "fpga.h"
#include "timing.h"
#include "linux_file_util.h"

#include "../../parser/parseTxtSeq.h"
#include "../../parser/parseMisc.h"
#include <verbosity.h>
#include <CmdLineArgs.h>

#include <fcgi/fcgio.h>
#include <fcgi/fcgi_config.h>

using namespace cgicc;

#include "fcgi/fcgio.h"

#include "cgicc/CgiInput.h"

// TR: Linker failed when FCgiIO was in a separate header (undefined ref to vtable),
//     so I'm pasting it here.  Removed pointers to stderr and stdout streams.
//      This came from cgicc/contrib/FCgiIO.h , .cpp

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
class CGICC_API FCgiIO : public cgicc::CgiInput
{
public:

    // ============================================================

    /*! \name Constructor and Destructor */
    //@{

    /*!
     * \brief Constructor
     *
     * Create a new FCgiIO object
     */
    FCgiIO(FCGX_Request& request) :
        fRequest(request)
    {
        // Parse environment
        for(char **e = fRequest.envp; *e != NULL; ++e) {
            std::string s(*e);
            std::string::size_type i = s.find('=');
            if(i == std::string::npos)
                throw std::runtime_error("Illegally formed environment");
            fEnv[s.substr(0, i)] = s.substr(i + 1);
        }
    }

    /*!
     * \brief Copy constructor
     *
     */
    FCgiIO(const FCgiIO& io) :
        CgiInput(io),
        fRequest(io.fRequest)
        {
        }
    /*!
     * \brief Destructor
     *
     * Delete this FCgiIO object
     */
    virtual ~FCgiIO() {}
    //@}

    // ============================================================

    /*! \name Data Sources */
    //@{

    /*!
     * \brief Read data from the request's input stream.
     *
     * \param data The target buffer
     * \param length The number of characters to read
     * \return The number of characters read
     */
    size_t read(char *data, size_t length)
    {
        return FCGX_GetStr(data, length, fRequest.in);
    }

    /*!
     * \brief Query the value of an environment variable stored in the request.
     *
     * \param varName The name of an environment variable
     * \return The value of the requested environment variable, or an empty
     * string if not found.
     */
    virtual std::string getenv(const char *varName)
    {
        return fEnv[varName];
    }
    //@}


protected:
    FCGX_Request& fRequest;
    std::map<std::string, std::string> fEnv;
};


using namespace std;


int nSig = 0;
bool g_stop = false;

#ifdef LINUX_OS
void* pulser = 0;
#else
void* pulser = (void*)XPAR_PULSE_CONTROLLER_0_BASEADDR;
#endif

bool g_debug_spi = false;
bool g_stop_curr_seq = false;

FILE* gLog = 0;
verbosity gvSTDOUT(0, 0);
verbosity gvLog(0, 0);

const char PROG_NAME[]="molecube";

unsigned gDebugLevel = 0;

//atexit handler
void bye(void)
{
    fprintf(gLog, "bye\n\n");
}

void handleINT(int)
{
    g_stop = true;
    printf("received signal to exit\n");
    exit(0);
}

void handleUSR1(int)
{
    g_stop_curr_seq = true;

    fprintf(gLog, "Received USR1 signal\n");
    fprintf(gLog, "Stopping pulse sequence\n");
}

void printHeader(FILE* fLog)
{
    fprintf(fLog, "\n\nMolecube 1.09 (FastCGI)\n");
    fprintf(fLog, "Built: %s %s  ", __DATE__, __TIME__);
    fprintf(fLog, "with GCC %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
}
void printUsage()
{
    printHeader(stdout);
    printf("Command line options:\n");
    printf(" -l log_file_name : Set log file.  If none specified, use stdout.\n");
    printf(" -s startup_file_name : If specified, run startup pulse sequence from file.\n");
    printf(" -h or --help : Print help / usage info.\n");
    printf("\n\n");
}

int main(int argc, char *argv[])
{
    signal(SIGINT, &handleINT); //handler gets called for ctrl-C or similar kill signals
    signal(SIGUSR1, &handleUSR1); //USR1: stop current pulse sequence
    atexit(bye); //bye gets called from exit()

    //get command line args
    CmdLineArgs cla(argc, argv);

    g_fPulserLock = fopen("/tmp/pulser.lock", "w");

    if( (cla.FindString("-h") >= 0) || (cla.FindString("--help") >= 0) ) {
        gLog = stdout;
        printUsage();
        return 0;
    }

    string sLogFileName = cla.GetStringAfter("-l", "stdout");
    if(sLogFileName == "stdout")
        gLog =stdout;
    else
        gLog = fopen(sLogFileName.c_str(), "a");

    gvSTDOUT = verbosity(&cout, gLog);
    gvLog = verbosity(0, gLog);

    printHeader(gLog);

    setProgramStatus(PROG_NAME, "Initializing");

    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    fprintf(gLog, "Current time: (UTC) %s\n", asctime(timeinfo));
    fflush(gLog);

    init_system();

    FCGX_Request request;

    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    int nAccept = 0;

    time_t srandT = time(0);
    srand(srandT + nAccept);
    fprintf(gLog, "Random seed = %u.  2 random numbers: %u, %u\n", (unsigned)srandT, rand(), rand());

    // run startup sequence
    string fnameStartup = cla.GetStringAfter("-s", "");
    if(fnameStartup.length()) {
        fprintf(gLog, "Read startup sequence from: %s\n", fnameStartup.c_str());
        ifstream ifs(fnameStartup);

        if(ifs.is_open()) {
            string sStartupSeq;
            while(!ifs.eof()) {
                string line;
                getline(ifs, line);
                sStartupSeq.append(line);
                sStartupSeq.append("\n");
            }

            try {
                parseSeqURL(sStartupSeq);
            } catch (runtime_error e) {
                fprintf(gLog, "Startup sequence error:   %s\n", e.what());
            }
        }
        else
            fprintf(gLog, "Could not open file.\n");
    }

    fprintf(gLog, "Waiting for network connections...\n\n");
    fflush(gLog);

    setProgramStatus(0, "Idle");

    while (FCGX_Accept_r(&request) == 0) {
        setProgramStatus(0, "Processing request");
        fprintf(gLog, "================ Accept FastCGI request %d ================\n", nAccept);

        // Replace stdio streambufs.
        // Note that the default bufsize (0) will cause the use of iostream
        // methods that require positioning (such as peek(), seek(),
        // unget() and putback()) to fail (in favour of more efficient IO).
        //     fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);

        //   cin.rdbuf(&cin_fcgi_streambuf);
        cout.rdbuf(&cout_fcgi_streambuf);
        cerr.rdbuf(&cerr_fcgi_streambuf);


        FCgiIO IO(request);
        Cgicc cgi(&IO);

        gvSTDOUT = verbosity(&cout, gLog);

        try {
            if(!parseQueryCGI(cgi))
                fprintf(gLog, "Couldn't understand HTTP request.\n");
        } catch (runtime_error e) {
            gvSTDOUT.printf("Oh noes! \n   %s\n", e.what());
            gvSTDOUT.printf("%s", getQuote("/usr/local/quotes.frt", "%%").c_str());
        }

        fprintf(gLog, "================ Finish FastCGI request %d ================\n\n", nAccept++);
        fflush(gLog);
        cout << endl;
        setProgramStatus(0, "Idle");
    }

    fprintf(gLog, "Exit, return 0\n");
    setProgramStatus(0, "Finished / Quit");
    return 0;
}
