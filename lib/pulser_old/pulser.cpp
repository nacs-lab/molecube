#include "pulser.h"

#include "program.h"
#include <nacs-pulser/ctrl_io.h>
#include <nacs-pulser/commands.h>

#include <nacs-utils/container.h>

#include <nacs-utils/number.h>
#include <nacs-utils/log.h>
#include <nacs-utils/timer.h>
#include <nacs-utils/fd_utils.h>
#include <nacs-xspi/xparameters.h>

#include <stdexcept>
#include <mutex>
#include <random>
#include <limits>
#include <inttypes.h>

namespace NaCs {
namespace Pulser {

static bool
check_program(const uint32_t *prog, size_t len) noexcept
{
    if (!prog || !len || len % 2 != 0) {
        nacsError("prog: %p, len: %zu", prog, len);
        return false;
    }
    for (size_t i = 0;i < len;i += 2) {
        int32_t offset = prog[i] - usr_slv_space_offset;
        if (nacsUnlikely(offset < 0 || offset >= 32 * 4)) {
            nacsError("i: %zu, offset: %" PRIi32, i, offset);
            return false;
        }
    }
    return true;
}

// optimize("Ofast", "prefetch-loop-arrays")
static __attribute__((flatten, hot)) void
run_program_real(Driver &driver, const uint32_t *__restrict__ prog,
                 size_t len) noexcept
{
    // Options to benchmark
    // * With/Withtout Ofast/prefetch-loop-arrays
    // * loop with index/pointer
    // * whether to use __buildin_prefetch directly
    auto end = prog + len;
#pragma unroll(16)
    for (auto p = prog;p < end;p++) {
        auto addr = *p;
        p++;
        auto val = *p;
        __builtin_prefetch(p + 3);
        driver.writeReg(addr, val);
    }
}

static int
run_program(Driver &driver, const uint32_t *prog, size_t len) noexcept
{
    if (nacsUnlikely(!check_program(prog, len))) {
        return -1;
    }
    run_program_real(driver, prog, len);
    return 0;
}

// make short timed pulses
// FPGA can only handle pulse lengths up to t_max = 0x00FFFFFF (about 40 ms)
void
OldPulserBase::shortPulse(uint32_t control, uint32_t operand)
{
    write_reg(31, operand);
    write_reg(31, control);
}

// reset DDS i
void
OldPulserBase::dds_reset(int i)
{
    add(DDSReset(i));
}

void
OldPulserBase::set_dds_phase(int i, uint16_t phase)
{
    add(DDSSetPhase(i, phase));
}

// TTL functions: pulse_io = (ttl_out | high_mask) & (~low_mask);
void
OldPulserBase::set_ttl_mask(uint32_t high_mask, uint32_t low_mask)
{
    write_reg(0, high_mask);
    write_reg(1, low_mask);
}

void
OldPulser::debug_regs()
{
    FILE *log_f = nacsGetLog();
    fprintf(log_f, "PULSE_CONTROLLER registers:\n");
    for (unsigned i = 0;i < 31;i++) {
        if (i % 4 == 0) {
            fprintf(log_f, "[%2d...%2d]: ", i, i + 3);
        }
        fprintf(log_f, "%08X ", read_reg(i));
        if (i % 4 == 3) {
            fprintf(log_f, "\n");
        }
    }
    fprintf(log_f, "\n");
}

void
OldPulser::write_reg(unsigned reg, uint32_t val)
{
    m_driver.writeReg(reg, val);
}

uint32_t
OldPulser::read_reg(unsigned reg)
{
    return m_driver.readReg(reg);
}

void
OldPulser::init(bool reset)
{
    release_hold();
    debug_regs();
    release_hold();
    if (reset) {
        nacsInfo("PULSER_init... reset DDS\n");
        for (unsigned i = 0;i < PULSER_NDDS;i++) {
            dds_reset(i);
        }
    }
}

void
OldPulser::run(const BaseProgram &prog)
{
    if (m_running.exchange(true)) {
        throw std::runtime_error("Already running.");
    }
    if (run_program(m_driver, prog.program(), prog.len()) != 0) {
        throw std::runtime_error("Invalid program.");
    }
}

bool
OldPulser::is_finished()
{
    return read_reg(2) & 0x4;
}

// release hold.  pulses can run
void
OldPulser::release_hold()
{
    write_reg(3, read_reg(3) & ~0x80);
}

// set hold. pulses are stopped
void
OldPulser::set_hold()
{
    write_reg(3, read_reg(3) | 0x80);
}

void
OldPulser::wait()
{
    if (!m_running.exchange(false)) {
        throw std::runtime_error("Not running.");
    }
    release_hold();
    while (!is_finished()) {
    }
}

// TTL functions: pulse_io = (ttl_out | high_mask) & (~low_mask);
void
OldPulser::get_ttl_mask(uint32_t *high_mask, uint32_t *low_mask)
{
    *high_mask = read_reg(0);
    *low_mask = read_reg(1);
}

uint32_t
OldPulser::num_results()
{
    return (read_reg(2) >> 4) & 31;
}

uint32_t
OldPulser::pop_result()
{
    while (num_results() == 0) {
    }
    return read_reg(31);
}

// were there any timing failures?
bool
OldPulser::timing_ok()
{
    return !(read_reg(2) & 0x1);
}

uint32_t
OldPulser::get_dds_byte(int i, uint32_t address)
{
    add(DDSGetByte(i, address));
    return (pop_result() >> 8) & 0x000000ff;
}

uint32_t
OldPulser::get_dds_two_bytes(int i, unsigned address)
{
    add(DDSGetTwoBytes(i, address));
    return pop_result() & 0x0000ffff;
}

uint32_t
OldPulser::get_dds_four_bytes(int i, unsigned address)
{
    add(DDSGetFourBytes(i, address));
    return pop_result();
}

// toggle init. reset prior to new sequence
void
OldPulser::toggle_init()
{
    unsigned r3 = read_reg(3);
    write_reg(3, r3 | 0x00000100);
    write_reg(3, r3 & ~0x00000100);
}

// If DDS i is present return non-zero, otherwise 0.
bool
OldPulser::dds_exists(int i)
{
    unsigned addr = 0x68;
    // Check whether it's possible to set phase of profile 7 to 0 and 1
    add(DDSSetTwoBytes(i, addr, 0));
    unsigned u0 = get_dds_two_bytes(i, addr);

    add(DDSSetTwoBytes(i, addr, 1));
    unsigned u1 = get_dds_two_bytes(i, addr);

    return (u0 == 0) && (u1 == 1);
}

uint32_t
OldPulser::get_dds_freq(int i)
{
    // shortPulse(0x1000000E | (i << 4) | (0x2D << 9), 0);
    // return pop_result();
    uint32_t u0 = get_dds_two_bytes(i, 0x2c);
    uint32_t u2 = get_dds_two_bytes(i, 0x2e);

    return u0 | (u2 << 16);
}

uint32_t
OldPulser::get_dds_phase(int i)
{
    return get_dds_two_bytes(i, 0x30);
}

uint32_t
OldPulser::get_dds_amp(int i)
{
    return get_dds_two_bytes(i, 0x32);
}

bool
OldPulser::test_regs()
{
    bool sr_ok = 1;
    for (int i = 0;i < 2;i++) {
        unsigned test_val = i ? unsigned(-1) : 0;

        nacsLog("Testing %08X ...\n", test_val);
        for (int k = 0;k < 8;k++) {
            write_reg(k, test_val);
            unsigned read = read_reg(k);
            nacsLog("    SR%d = %08X\n", k, read);
            sr_ok = sr_ok && (read == test_val);
        }

        if (sr_ok) {
            nacsLog("OK\n");
        } else {
            nacsError("FAILED\n");
        }
    }
    return sr_ok;
}

bool
OldPulser::self_test(int ndds, int cycle)
{
    if (nacsUnlikely(ndds <= 0)) {
        return true;
    }
    unsigned ftw[ndds];
    unsigned nBad = 0;

    bool test_pass = test_regs();
    static std::random_device rd;
    std::uniform_int_distribution<int> dds_dist(0, ndds);
    std::uniform_int_distribution<unsigned>
        val_dist(0, std::numeric_limits<unsigned>::max());
    nacsLog("\n");

    if (cycle > 0) {
        for (int i = 0;i < ndds;i++) {
            test_pass = test_pass && test_dds(i);
        }

        nacsLog("Testing %d random read/writes on DDS boards 0-%d ...\n",
                cycle, ndds - 1);

        // initialize to 0 Hz
        for (int i = 0;i < ndds;i++) {
            ftw[i] = 0;
            add(DDSSetFreq(i, 0));
        }

        for (int j = 0;j < cycle;j++) {
            int i = dds_dist(rd);
            unsigned ftw_read = get_dds_freq(i);

            if (ftw_read != ftw[i]) {
                test_pass = false;
                nacsError("DDS %d : wrote FTW %08X\n", i, ftw[i]);
                nacsError("          read FTW %08X\n", ftw_read);
                nBad++;
            }

            ftw[i] = val_dist(rd);
            add(DDSSetFreq(i, ftw[i]));
        }

        for (int i = 0;i < ndds;i++) {
            add(DDSSetFreq(i, 0));
        }

        if (nBad == 0) {
            nacsLog("OK\n");
        } else {
            nacsError("FAILURE: %d errors\n", nBad);
        }
    }
    return test_pass;
}

bool
OldPulser::test_dds(int i)
{
    int freq_ok = 1;
    int phase_ok = 1;

    nacsLog("Testing DDS(%d) ...\n", i);
    for (int i = 0;i < 2;i++) {
        unsigned test_val = 0;

        for (int j = 0;j < 8;j++) {
            test_val = test_val + ((i * 0xF) << (j * 4));
        }

        add(DDSSetFreq(i, test_val));
        unsigned read = get_dds_freq(i);
        freq_ok = freq_ok && (read == test_val);

        if (read != test_val) {
            nacsError("DDS(%d) wrote FTW %08X\n", i, test_val);
            nacsError("DDS(%d)  read FTW %08X\n", i, read);
        }
    }

    for (uint16_t phase = 0;phase < 0x4000;phase++) {
        set_dds_phase(i, phase);
        unsigned read = get_dds_phase(i);

        phase_ok = phase_ok && (read == phase);

        if (read != phase) {
            nacsError("DDS(%d) wrote PHASE %04X\n", i, (unsigned)phase);
            nacsError("DDS(%d)  read PHASE %04X\n", i, read);
        }
    }

    if (freq_ok && phase_ok) {
        nacsLog("DDS %d OK\n", i);
    } else {
        nacsError("DDS %d FAILED\n", i);
    }

    return freq_ok && phase_ok;
}

NACS_EXPORT OldPulser&
get_pulser()
{
    // C++0x grantees that the initialization of function scope static
    // variable is thread-safe.
    static OldPulser pulser = mapPulserAddr();
    return pulser;
}

}
}
