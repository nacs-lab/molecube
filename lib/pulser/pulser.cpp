#include "pulser.h"

#include "program.h"
#include "ctrl_io.h"

#include <nacs-utils/number.h>
#include <nacs-utils/log.h>
#include <nacs-utils/timer.h>
#include <nacs-utils/fd_utils.h>
#include <nacs-xspi/xparameters.h>

#include <stdexcept>
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

void
PulserBase::short_pulse(uint32_t control, uint32_t operand)
{
    if (log_on()) {
        nacsLog("Short Pulse control=%x, operand=%x\n", control, operand);
    }
    auto holder = log_holder();
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
    auto holder = log_holder();
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
    auto holder = log_holder();
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
    auto holder = log_holder();
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
    auto holder = log_holder();
    static const unsigned t_max = 0x001FFFFF;
    do {
        unsigned t_step = nacsMin(t, t_max);
        short_pulse(t_step | flags, operand);
        t -= t_step;
    } while (t > 0);
}

// clear timing check (clear failures)
void
PulserBase::clear_timing_check()
{
    if (log_on()) {
        nacsLog("Clear timing check");
    }
    auto holder = log_holder();
    short_pulse(0x30000000, 0);
}

void
PulserBase::set_dds_freq(int i, uint32_t ftw)
{
    if (log_on()) {
        nacsLog("Set DDS(%d) frequency %x", i, ftw);
    }
    auto holder = log_holder();
    short_pulse(0x10000000 | (i << 4), ftw);
}

void
PulserBase::set_dds_amp(int i, uint32_t amp)
{
    if (log_on()) {
        nacsLog("Set DDS(%d) amplitude %x", i, amp);
    }
    auto holder = log_holder();
    set_dds_two_bytes(i, 0x32, amp);
}

// reset DDS i
void
PulserBase::dds_reset(int i)
{
    if (log_on()) {
        nacsLog("Reset DDS(%i)\n", i);
    }
    auto holder = log_holder();
    short_pulse(0x10000004 | (i << 4), 0);
}

void
PulserBase::set_dds_phase(int i, uint16_t phase)
{
    if (log_on()) {
        nacsLog("Set DDS(%i) phase %" PRId16 "\n", i, phase);
    }
    auto holder = log_holder();
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
    auto holder = log_holder();
    write_reg(0, high_mask);
    write_reg(1, low_mask);
}

void
Pulser::debug_regs()
{
    fprintf(stderr, "PULSE_CONTROLLER registers:\n");
    for (unsigned i = 0;i < 31;i++) {
        if (i % 4 == 0) {
            fprintf(stderr, "[%2d...%2d]: ", i, i + 3);
        }
        fprintf(stderr, "%08X ", read_reg(i));
        if (i % 4 == 3) {
            printf("\n");
        }
    }
    printf("\n");
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
        auto holder = log_holder();
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
    write_reg(3, read_reg(3) & ~0x00000080);
}

// set hold. pulses are stopped
void
Pulser::set_hold()
{
    write_reg(3, read_reg(3) | 0x00000080);
}

void
Pulser::wait()
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
Pulser::get_ttl_mask(uint32_t *high_mask, uint32_t *low_mask)
{
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
    short_pulse(0x10000003 | (i << 4) | (address << 9), 0);
    return (pop_result() >> 8) & 0x000000ff;
}

// get two bytes from address + 1 ... adress on DDS i
uint32_t
Pulser::get_dds_two_bytes(int i, unsigned address)
{
    short_pulse(0x10000003 | (i << 4) | ((address + 1) << 9), 0);
    return pop_result() & 0x0000ffff;
}

// get four bytes from address + 3 ... adress on DDS i
uint32_t
Pulser::get_dds_four_bytes(int i, unsigned address)
{
    short_pulse(0x1000000E | (i << 4) | ((address + 1) << 9), 0);
    return pop_result();
}

// toggle init. reset prior to new sequence
void
Pulser::toggle_init()
{
    unsigned r3 = read_reg(3);
    write_reg(3, r3 | 0x00000100);
    write_reg(3, r3 & ~0x00000100);
}

// If DDS i is present return non-zero, otherwise 0.
bool
Pulser::dds_exists(int i)
{
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

static intptr_t
get_phys_addr()
{
    // Can also be determined by looking for
    // /proc/device-tree/amba@0/pulse-controller@73000000
    // with propery device tree file.

    return XPAR_PULSE_CONTROLLER_0_BASEADDR;
}

NACS_EXPORT Pulser
get_pulser()
{
    nacsInfo("Initializing pulse controller\n");
    auto addr = nacsMapFile("/dev/mem", get_phys_addr(), 4096);
    if (nacsUnlikely(!addr)) {
        nacsError("Can't map the memory to user space.\n");
        exit(0);
    }
    return Pulser(addr);
}

}
}
