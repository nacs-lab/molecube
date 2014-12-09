#include <nacs-utils/utils.h>

#ifndef __NACS_PULSER_PULSER_H__
#define __NACS_PULSER_PULSER_H__

#include "converter.h"
#include <nacs-pulser/pulser-config.h>

#include <atomic>
#include <memory>
#include <mutex>

namespace NaCs {
namespace Pulser {

class BaseProgram;

class NACS_EXPORT PulserBase {
    std::unique_ptr<std::recursive_timed_mutex> m_lock;
    PulserBase(const PulserBase&) = delete;
protected:
    bool m_debug;
public:
    PulserBase(bool debug=false);
    PulserBase(PulserBase &&other);

    virtual ~PulserBase() {}
    void clock_out(unsigned divider);
    void set_dds_two_bytes(int i, uint32_t addr, uint32_t data);
    void set_dds_four_bytes(int i, uint32_t addr, uint32_t data);
    void pulse(uint64_t t, unsigned flags, unsigned operand);
    void clear_timing_check();
    void set_dds_freq(int i, uint32_t ftw);
    void set_dds_amp(int i, uint32_t amp);
    virtual void set_dds_phase(int i, uint16_t phase);
    void dds_reset(int i);
    void set_ttl_mask(uint32_t high_mask, uint32_t low_mask);
    virtual void short_pulse(uint32_t control, uint32_t operand);

    void set_dds_freq_f(int i, double f);
    void set_dds_amp_f(int i, double f);
    void set_dds_phase_f(int i, double f);

    void reset_dds_sel(uint32_t mask);
    void set_dds_sel(uint32_t mask);
private:
    static thread_local bool pulser_logging;
    virtual void write_reg(unsigned reg, uint32_t val) = 0;
protected:
    bool log_on();
    ScopeSwap<bool> log_holder();
};

NACS_INLINE ScopeSwap<bool>
PulserBase::log_holder()
{
    return make_scope_swap(pulser_logging, true);
}

NACS_INLINE bool
PulserBase::log_on()
{
    return m_debug && !pulser_logging;
}

NACS_INLINE
PulserBase::PulserBase(bool debug)
    : m_lock(new std::recursive_timed_mutex()),
      m_debug(debug)
{
}

NACS_INLINE
PulserBase::PulserBase(PulserBase &&other)
    : m_lock(std::move(other.m_lock)),
      m_debug(other.m_debug)
{
}


NACS_INLINE void
PulserBase::set_dds_freq_f(int i, double f)
{
    set_dds_freq(i, DDSConverter::freq2num(f, PULSER_AD9914_CLK));
}

NACS_INLINE void
PulserBase::set_dds_amp_f(int i, double amp)
{
    set_dds_amp(i, DDSConverter::amp2num(amp));
}

NACS_INLINE void
PulserBase::set_dds_phase_f(int i, double p)
{
    set_dds_phase(i, DDSConverter::phase2num(p));
}

class NACS_EXPORT Pulser : public PulserBase {
    volatile void *m_base;
    std::atomic_bool m_running;

    Pulser() = delete;
    Pulser(const Pulser&) = delete;
    void operator=(const Pulser&) = delete;
public:
    Pulser(Pulser &&other);
    Pulser(volatile void *base, bool debug=false);
    ~Pulser() {}
    void init(bool reset);
    void run(const BaseProgram &prog);
    void wait();
    void get_ttl_mask(uint32_t *high_mask, uint32_t *low_mask);
    void write_reg(unsigned reg, uint32_t val) override;
    uint32_t read_reg(unsigned reg);
    uint32_t pop_result();

    /**
     * Timing-check functions to help figure out if experiment timing is being
     * met. Timing failure will occur if the pulse buffer underflows.
     *
     * While timing_check flag is enabled, all pulses that are sent to the
     * Pulser are considered time-critical.  If a pulse finishes, and there is
     * not another pulse waiting in the buffer, a timing error is stored in one
     * of the status registers. This can be detected by calling timing_ok
     * (returns false if a timing error occured). The error status can be
     * cleared by calling clear_timing_check.
     */
    bool timing_ok();
    void set_hold();
    uint32_t get_dds_byte(int i, uint32_t address);
    uint32_t get_dds_two_bytes(int i, unsigned address);
    uint32_t get_dds_four_bytes(int i, unsigned address);
    void toggle_init();
    bool dds_exists(int i);
    uint32_t get_dds_freq(int i);
    uint32_t get_dds_phase(int i);
    uint32_t get_dds_amp(int i);
    volatile void *get_base() const;

    double get_dds_freq_f(int i);
    double get_dds_phase_f(int i);
    double get_dds_amp_f(int i);

    bool &debug();

    bool self_test(int ndds, int cycle=1);
private:
    bool test_regs();
    bool test_dds(int i);
    uint32_t num_results();
    void debug_regs();
    bool is_finished();
    void release_hold();
};

NACS_INLINE bool&
Pulser::debug()
{
    return m_debug;
}

NACS_INLINE
Pulser::Pulser(Pulser &&other)
    : PulserBase(static_cast<PulserBase&&>(other)),
      m_base(other.m_base),
      m_running(other.m_running.exchange(false))
{
}

NACS_INLINE
Pulser::Pulser(volatile void *base, bool debug)
    : PulserBase(debug),
      m_base(base),
      m_running(false)
{
}

NACS_INLINE volatile void*
Pulser::get_base() const
{
    return m_base;
}

Pulser get_pulser();

NACS_INLINE double
Pulser::get_dds_freq_f(int i)
{
    return DDSConverter::num2freq(get_dds_freq(i), PULSER_AD9914_CLK);
}

NACS_INLINE double
Pulser::get_dds_phase_f(int i)
{
    return DDSConverter::num2phase(get_dds_phase(i));
}

NACS_INLINE double
Pulser::get_dds_amp_f(int i)
{
    return DDSConverter::num2amp(get_dds_amp(i));
}

}
}

#endif
