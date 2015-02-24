#include "pulser.h"

#include "program.h"
#include "ctrl_io.h"

#include <nacs-utils/number.h>
#include <nacs-utils/log.h>
#include <nacs-utils/timer.h>
#include <nacs-utils/fd_utils.h>
#include <nacs-xspi/xparameters.h>

#include <stdexcept>
#include <mutex>
#include <inttypes.h>

namespace NaCs {
namespace Pulser {

thread_local bool PulserBase::pulser_logging = false;

static bool
check_program(const uint32_t *prog, size_t len) noexcept
{
    if (!prog || !len || len % 2 != 0) {
        nacsError("prog: %p, len: %zu", prog, len);
        return false;
    }
    for (size_t i = 0;i < len;i += 2) {
        int32_t offset = prog[i] - PULSER_USER_SLV_SPACE_OFFSET;
        if (nacsUnlikely(offset < 0 || offset >= 32 * 4)) {
            nacsError("i: %zu, offset: %" PRIi32, i, offset);
            return false;
        }
    }
    return true;
}

static __attribute__((optimize("Ofast", "prefetch-loop-arrays"),
                      flatten, hot)) void
run_program_real(volatile void *base, const uint32_t *__restrict__ prog,
                 size_t len) noexcept
{
    // Options to benchmark
    // * With/Withtout Ofast/prefetch-loop-arrays
    // * loop with index/pointer
    // * whether to use __buildin_prefetch directly
    auto end = prog + len;
    for (auto p = prog;p < end;p++) {
        auto addr = *p;
        p++;
        auto val = *p;
        __builtin_prefetch(p + 3);
        PULSER_mWriteReg(base, addr, val);
    }
}

static int
run_program(volatile void *base, const uint32_t *prog, size_t len) noexcept
{
    if (nacsUnlikely(!check_program(prog, len))) {
        return -1;
    }
    run_program_real(base, prog, len);
    return 0;
}

// make short timed pulses
// FPGA can only handle pulse lengths up to t_max = 0x001FFFFF (about 40 ms)
void
PulserBase::short_pulse(uint32_t control, uint32_t operand)
{
    PulserLocker lock(this);
    if (log_on()) {
        nacsLog("Short Pulse control=%x, operand=%x\n", control, operand);
    }
    LogHolder holder;
    write_reg(31, operand);
    write_reg(31, control);
}

// enable / disable clock_out
// divider = 0..254 means emit clock with period 2 x (divider + 1)
// in pulse controller timing units (DT_ns)
// divider = 255 means disable
void
PulserBase::clock_out(unsigned divider)
{
    if (log_on()) {
        nacsLog("Clock out %u\n", divider);
    }
    LogHolder holder;
    short_pulse(0x50000000, divider & 0xFF);
}

// set bytes at addr + 1 and addr
// note that get_dds_two bytes also returns data at addr+1 and addr
void
PulserBase::set_dds_two_bytes(int i, uint32_t addr, uint32_t data)
{
    if (log_on()) {
        nacsLog("Set DDS(%d) two bytes addr=%x, data=%x\n", i, addr, data);
    }
    LogHolder holder;
    // put addr in bits 15...9 (maps to DDS opcode_reg[14:9] )?
    uint32_t dds_addr = (addr + 1) & 0x7F;
    // put data in bits 15...0 (maps to DDS operand_reg[15:0] )?
    uint32_t dds_data = data & 0xFFFF;
    short_pulse(0x10000002 | (i << 4) | (dds_addr << 9), dds_data);
}

// set bytes addr + 3 ... addr
void
PulserBase::set_dds_four_bytes(int i, uint32_t addr, uint32_t data)
{
    if (log_on()) {
        nacsLog("Set DDS(%d) four bytes addr=%x, data=%x\n", i, addr, data);
    }
    LogHolder holder;
    //put addr in bits 15...9 (maps to DDS opcode_reg[14:9])?
    uint32_t dds_addr = (addr + 1) & 0x7F;
    short_pulse(0x1000000F | (i << 4) | (dds_addr << 9), data);
}

//make timed pulses
//if t > t_max, subdivide into shorter pulses
//returns number of pulses made
void
PulserBase::pulse(uint64_t t, unsigned flags, unsigned operand)
{
    if (log_on()) {
        nacsLog("Long pulse t=%" PRTime ", flags=%x, operand=%x\n",
                t, flags, operand);
    }
    LogHolder holder;
    static const uint32_t t_max = 0x001FFFFF;
    PulserLocker lock(this);
    do {
        uint32_t t_step = uint32_t(min(t, t_max));
        short_pulse(t_step | flags, operand);
        t -= t_step;
    } while (t > 0);
}

// clear timing check (clear failures)
void
PulserBase::clear_timing_check()
{
    if (log_on()) {
        nacsLog("Clear timing check\n");
    }
    LogHolder holder;
    short_pulse(0x30000000, 0);
}

void
PulserBase::set_dds_freq(int i, uint32_t ftw)
{
    if (log_on()) {
        nacsLog("Set DDS(%d) frequency %x\n", i, ftw);
    }
    LogHolder holder;
    short_pulse(0x10000000 | (i << 4), ftw);
}

void
PulserBase::set_dds_amp(int i, uint32_t amp)
{
    if (log_on()) {
        nacsLog("Set DDS(%d) amplitude %x\n", i, amp);
    }
    LogHolder holder;
    set_dds_two_bytes(i, 0x32, amp);
}

// reset DDS i
void
PulserBase::dds_reset(int i)
{
    if (log_on()) {
        nacsLog("Reset DDS(%i)\n", i);
    }
    LogHolder holder;
    short_pulse(0x10000004 | (i << 4), 0);
}

void
PulserBase::set_dds_phase(int i, uint16_t phase)
{
    if (log_on()) {
        nacsLog("Set DDS(%i) phase %" PRId16 "\n", i, phase);
    }
    LogHolder holder;
    set_dds_two_bytes(i, 0x30, phase);
}

// TTL functions: pulse_io = (ttl_out | high_mask) & (~low_mask);
void
PulserBase::set_ttl_mask(uint32_t high_mask, uint32_t low_mask)
{
    if (log_on()) {
        nacsLog("Set TTL mask low=%" PRIx32 ", high=%" PRIx32 "\n",
                low_mask, high_mask);
    }
    LogHolder holder;
    PulserLocker lock(this);
    write_reg(0, high_mask);
    write_reg(1, low_mask);
}

// reset DDS selected by bitmask mask
void
PulserBase::reset_dds_sel(uint32_t mask)
{
    if (log_on()) {
        nacsLog("Reset DDS selection mask %" PRIx32 "\n", mask);
    }
    LogHolder holder;
    short_pulse(0x10000005, mask);
}

void
PulserBase::set_dds_sel(uint32_t mask)
{
    if (log_on()) {
        nacsLog("Set DDS selection mask %" PRIx32 "\n", mask);
    }
    LogHolder holder;
    short_pulse(0x10000006, mask);
}

void
Pulser::debug_regs()
{
    PulserLocker lock(this);
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
Pulser::write_reg(unsigned reg, uint32_t val)
{
    if (log_on()) {
        nacsLog("Write Register(%u), %" PRIx32 "\n", reg, val);
    }
    PULSER_mWriteSlaveReg(m_base, reg, val);
}

uint32_t
Pulser::read_reg(unsigned reg)
{
    return PULSER_mReadSlaveReg(m_base, reg);
}

void
Pulser::init(bool reset)
{
    if (log_on()) {
        LogHolder holder;
        debug_regs();
    }

    if (reset) {
        nacsInfo("PULSER_init... reset DDS\n");
        for (unsigned i = 0;i < PULSER_NDDS;i++) {
            dds_reset(i);
        }
    }
}

void
Pulser::run(const BaseProgram &prog)
{
    if (m_running.exchange(true)) {
        throw std::runtime_error("Already running.");
    }
    PulserLocker lock(this);
    if (run_program(m_base, prog.program(), prog.len()) != 0) {
        throw std::runtime_error("Invalid program.");
    }
}

bool
Pulser::is_finished()
{
    return read_reg(2) & 0x4;
}

// release hold.  pulses can run
void
Pulser::release_hold()
{
    PulserLocker lock(this);
    write_reg(3, read_reg(3) & ~0x00000080);
}

// set hold. pulses are stopped
void
Pulser::set_hold()
{
    PulserLocker lock(this);
    write_reg(3, read_reg(3) | 0x00000080);
}

void
Pulser::wait()
{
    if (!m_running.exchange(false)) {
        throw std::runtime_error("Not running.");
    }
    PulserLocker lock(this);
    release_hold();
    while (!is_finished()) {
    }
}

// TTL functions: pulse_io = (ttl_out | high_mask) & (~low_mask);
void
Pulser::get_ttl_mask(uint32_t *high_mask, uint32_t *low_mask)
{
    PulserLocker lock(this);
    *high_mask = read_reg(0);
    *low_mask = read_reg(1);
}

uint32_t
Pulser::num_results()
{
    return (read_reg(2) >> 4) & 31;
}

uint32_t
Pulser::pop_result()
{
    PulserLocker lock(this);
    while (num_results() == 0) {
    }
    return read_reg(31);
}

// were there any timing failures?
bool
Pulser::timing_ok()
{
    return !(read_reg(2) & 0x1);
}

// get byte from address on DDS i
uint32_t
Pulser::get_dds_byte(int i, uint32_t address)
{
    PulserLocker lock(this);
    short_pulse(0x10000003 | (i << 4) | (address << 9), 0);
    return (pop_result() >> 8) & 0x000000ff;
}

// get two bytes from address + 1 ... adress on DDS i
uint32_t
Pulser::get_dds_two_bytes(int i, unsigned address)
{
    PulserLocker lock(this);
    short_pulse(0x10000003 | (i << 4) | ((address + 1) << 9), 0);
    return pop_result() & 0x0000ffff;
}

// get four bytes from address + 3 ... adress on DDS i
uint32_t
Pulser::get_dds_four_bytes(int i, unsigned address)
{
    PulserLocker lock(this);
    short_pulse(0x1000000E | (i << 4) | ((address + 1) << 9), 0);
    return pop_result();
}

// toggle init. reset prior to new sequence
void
Pulser::toggle_init()
{
    PulserLocker lock(this);
    unsigned r3 = read_reg(3);
    write_reg(3, r3 | 0x00000100);
    write_reg(3, r3 & ~0x00000100);
}

// If DDS i is present return non-zero, otherwise 0.
bool
Pulser::dds_exists(int i)
{
    PulserLocker lock(this);
    unsigned addr = 0x68;
    // Check whether it's possible to set phase of profile 7 to 0 and 1
    set_dds_two_bytes(i, addr, 0);
    unsigned u0 = get_dds_two_bytes(i, addr);

    set_dds_two_bytes(i, addr, 1);
    unsigned u1 = get_dds_two_bytes(i, addr);

    return (u0 == 0) && (u1 == 1);
}

uint32_t
Pulser::get_dds_freq(int i)
{
    PulserLocker lock(this);
    // short_pulse(0x1000000E | (i << 4) | (0x2D << 9), 0);
    // return pop_result();
    uint32_t u0 = get_dds_two_bytes(i, 0x2C);
    uint32_t u2 = get_dds_two_bytes(i, 0x2E);

    return u0 | (u2 << 16);
}

uint32_t
Pulser::get_dds_phase(int i)
{
    return get_dds_two_bytes(i, 0x30);
}

uint32_t
Pulser::get_dds_amp(int i)
{
    return get_dds_two_bytes(i, 0x32);
}

bool
Pulser::test_regs()
{
    PulserLocker lock(this);
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
Pulser::self_test(int ndds, int cycle)
{
    PulserLocker lock(this);
    if (nacsUnlikely(ndds <= 0)) {
        return true;
    }
    unsigned ftw[ndds];
    unsigned nBad = 0;

    bool test_pass = test_regs();
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
            set_dds_freq(i, 0);
        }

        for (int j = 0;j < cycle;j++) {
            int i = rand() % ndds;
            unsigned ftw_read = get_dds_freq(i);

            if (ftw_read != ftw[i]) {
                test_pass = false;
                nacsError("DDS %d : wrote FTW %08X\n", i, ftw[i]);
                nacsError("          read FTW %08X\n", ftw_read);
                nBad++;
            }

            ftw[i] = rand();
            set_dds_freq(i, ftw[i]);
        }

        for (int i = 0;i < ndds;i++) {
            set_dds_freq(i, 0);
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
Pulser::test_dds(int i)
{
    PulserLocker lock(this);
    int freq_ok = 1;
    int phase_ok = 1;

    nacsLog("Testing DDS(%d) ...\n", i);
    for (int i = 0;i < 2;i++) {
        unsigned test_val = 0;

        for (int j = 0;j < 8;j++) {
            test_val = test_val + ((i * 0xF) << (j * 4));
        }

        set_dds_freq(i, test_val);
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

static inline auto
get_phys_addr()
{
    // Can also be determined by looking for
    // /proc/device-tree/amba@0/pulse-controller@73000000
    // with propery device tree file.

    return XPAR_PULSE_CONTROLLER_0_BASEADDR;
}

NACS_EXPORT Pulser&
get_pulser()
{
    // C++0x grantees that the initialization of function scope static
    // variable is thread-safe.
    static Pulser pulser = [] {
        nacsInfo("Initializing pulse controller\n");
        auto addr = mapFile("/dev/mem", get_phys_addr(), 4096);
        if (nacsUnlikely(!addr)) {
            nacsError("Can't map the memory to user space.\n");
            exit(0);
        }
        return addr;
    }();
    return pulser;
}

}
}
