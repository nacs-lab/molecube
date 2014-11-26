#include "pulser.h"
#include "ctrl_io.h"

#include <nacs-utils/number.h>
#include <nacs-utils/log.h>

#include <stdexcept>

namespace NaCs {
namespace Pulser {

static bool
check_program(const uint32_t *prog, size_t len) noexcept
{
    if (!prog || !len || len % 2 != 0) {
        return false;
    }
    for (size_t i = 0;i < len;i++) {
        int32_t offset = prog[i] - PULSER_USER_SLV_SPACE_OFFSET;
        if (nacsUnlikely(offset < 0 || offset >= 32 * 4)) {
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
        __builtin_prefetch(p + 1);
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

NACS_EXPORT void
PulserBase::raw_pulse(uint32_t control, uint32_t operand)
{
    write_reg(31, operand);
    write_reg(31, control);
}

NACS_EXPORT void
PulserBase::dds_reset(unsigned i)
{
    short_pulse(0x10000004 | (i << 4), 0);
}

NACS_EXPORT void
PulserBase::short_pulse(uint32_t control, uint32_t operand)
{
    raw_pulse(control, operand);
}

// enable / disable clock_out
// divider = 0..254 means emit clock with period 2 x (divider + 1)
// in pulse controller timing units (DT_ns)
// divider = 255 means disable
NACS_EXPORT void
PulserBase::clock_out(unsigned divider)
{
    short_pulse(0x50000000, divider & 0xFF);
}

// set bytes at addr + 1 and addr
// note that get_dds_two bytes also returns data at addr+1 and addr
NACS_EXPORT void
PulserBase::set_dds_two_bytes(int i, uint32_t addr, uint32_t data)
{
    // put addr in bits 15...9 (maps to DDS opcode_reg[14:9] )?
    uint32_t dds_addr = (addr + 1) & 0x7F;
    // put data in bits 15...0 (maps to DDS operand_reg[15:0] )?
    uint32_t dds_data = data & 0xFFFF;
    short_pulse(0x10000002 | (i << 4) | (dds_addr << 9), dds_data);
}

// set bytes addr + 3 ... addr
NACS_EXPORT void
PulserBase::set_dds_four_bytes(int i, uint32_t addr, uint32_t data)
{
    //put addr in bits 15...9 (maps to DDS opcode_reg[14:9])?
    uint32_t dds_addr = (addr + 1) & 0x7F;
    short_pulse(0x1000000F | (i << 4) | (dds_addr << 9), data);
}

//make timed pulses
//if t > t_max, subdivide into shorter pulses
//returns number of pulses made
NACS_EXPORT void
PulserBase::pulse(unsigned t, unsigned flags, unsigned operand)
{
    static const unsigned t_max = 0x001FFFFF;
    do {
        unsigned t_step = nacsMin(t, t_max);
        short_pulse(t_step | flags, operand);
        t -= t_step;
    } while (t > 0);
}

// clear timing check (clear failures)
NACS_EXPORT void
PulserBase::clear_timing_check()
{
    short_pulse(0x30000000, 0);
}

NACS_EXPORT void
PulserBase::set_dds_freq(int i, uint32_t ftw)
{
    short_pulse(0x10000000 | (i << 4), ftw);
}

NACS_EXPORT void
PulserBase::set_dds_amp(int i, uint32_t amp)
{
    set_dds_two_bytes(i, 0x32, amp);
}

// reset DDS i
NACS_EXPORT void
PulserBase::dds_reset(int i)
{
    short_pulse(0x10000004 | (i << 4), 0);
}

NACS_EXPORT void
PulserBase::set_dds_phase(int i, uint16_t phase)
{
    set_dds_two_bytes(i, 0x30, phase);
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
    PULSER_mWriteSlaveReg(m_base, reg, val);
}

NACS_EXPORT uint32_t
Pulser::read_reg(unsigned reg)
{
    return PULSER_mReadSlaveReg(m_base, reg);
}

NACS_EXPORT void
Pulser::init(unsigned ndds, bool reset)
{
    if (nacsCheckLogLevel(NACS_LOG_INFO)) {
        debug_regs();
    }

    if (reset) {
        nacsInfo("PULSER_init... reset DDS\n");
        for (unsigned i = 0;i < ndds;i++) {
            dds_reset(i);
        }
    }
}

NACS_EXPORT void
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

//! release hold.  pulses can run
void
Pulser::release_hold()
{
    write_reg(3, read_reg(3) & ~0x00000080);
}

//! set hold. pulses are stopped
NACS_EXPORT void
Pulser::set_hold()
{
    write_reg(3, read_reg(3) | 0x00000080);
}

NACS_EXPORT void
Pulser::wait()
{
    if (!m_running.exchange(false)) {
        throw std::runtime_error("Not running.");
    }
    release_hold();
    while (!is_finished()) {
    }
}

//! TTL functions: pulse_io = (ttl_out | high_mask) & (~low_mask);
NACS_EXPORT void
Pulser::set_ttl_mask(uint32_t high_mask, uint32_t low_mask)
{
    write_reg(0, high_mask);
    write_reg(1, low_mask);
}

//! TTL functions: pulse_io = (ttl_out | high_mask) & (~low_mask);
NACS_EXPORT void
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

NACS_EXPORT uint32_t
Pulser::pop_result()
{
    while (num_results() == 0) {
    }
    return read_reg(31);
}

// were there any timing failures?
NACS_EXPORT bool
Pulser::timing_ok()
{
    return !(read_reg(2) & 0x1);
}

// get byte from address on DDS i
NACS_EXPORT uint32_t
Pulser::get_dds_byte(int i, uint32_t address)
{
    short_pulse(0x10000003 | (i << 4) | (address << 9), 0);
    return (pop_result() >> 8) & 0x000000ff;
}

// get two bytes from address + 1 ... adress on DDS i
NACS_EXPORT uint32_t
Pulser::get_dds_two_bytes(int i, unsigned address)
{
    short_pulse(0x10000003 | (i << 4) | ((address + 1) << 9), 0);
    return pop_result() & 0x0000ffff;
}

// get four bytes from address + 3 ... adress on DDS i
NACS_EXPORT uint32_t
Pulser::get_dds_four_bytes(int i, unsigned address)
{
    short_pulse(0x1000000E | (i << 4) | ((address + 1) << 9), 0);
    return pop_result();
}

// toggle init. reset prior to new sequence
NACS_EXPORT void
Pulser::toggle_init()
{
    unsigned r3 = read_reg(3);
    write_reg(3, r3 | 0x00000100);
    write_reg(3, r3 & ~0x00000100);
}

// If DDS i is present return non-zero, otherwise 0.
NACS_EXPORT bool
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

NACS_EXPORT uint32_t
Pulser::get_dds_freq(int i)
{
    // short_pulse(0x1000000E | (i << 4) | (0x2D << 9), 0);
    // return pop_result();
    uint32_t u0 = get_dds_two_bytes(i, 0x2C);
    uint32_t u2 = get_dds_two_bytes(i, 0x2E);

    return u0 | (u2 << 16);
}

NACS_EXPORT uint32_t
Pulser::get_dds_phase(int i)
{
    return get_dds_two_bytes(i, 0x30);
}

NACS_EXPORT uint32_t
Pulser::get_dds_amp(int i)
{
    return get_dds_two_bytes(i, 0x32);
}

}
}