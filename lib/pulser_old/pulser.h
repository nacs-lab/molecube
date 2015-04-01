#ifndef __NACS_PULSER_PULSER_H__
#define __NACS_PULSER_PULSER_H__
#include <nacs-pulser/driver.h>
#include <nacs-pulser/commands.h>

#include <nacs-utils/utils.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <chrono>

namespace NaCs {
namespace Pulser {

using namespace std::literals;

class BaseProgram;

class NACS_EXPORT OldPulserBase {
    std::unique_ptr<std::recursive_timed_mutex> m_lock;
    OldPulserBase(const OldPulserBase&) = delete;
    void operator=(const OldPulserBase&) = delete;
public:
    OldPulserBase();
    OldPulserBase(OldPulserBase &&other);

    virtual ~OldPulserBase() {}
    virtual void set_dds_phase(int i, uint16_t phase);
    void dds_reset(int i);
    void set_ttl_mask(uint32_t high_mask, uint32_t low_mask);
    virtual void shortPulse(uint32_t control, uint32_t operand);

    void set_dds_phase_f(int i, double f);
    template<typename Cmd>
    inline std::enable_if_t<isBaseCmd<Cmd>>
    add(Cmd &&cmd)
    {
        cmd.run(*this);
    }
private:
    virtual void write_reg(unsigned reg, uint32_t val) = 0;
};

NACS_INLINE
OldPulserBase::OldPulserBase()
    : m_lock(new std::recursive_timed_mutex())
{
}

NACS_INLINE
OldPulserBase::OldPulserBase(OldPulserBase &&other)
    : m_lock(std::move(other.m_lock))
{
}

NACS_INLINE void
OldPulserBase::set_dds_phase_f(int i, double p)
{
    set_dds_phase(i, DDSCvt::phase2num(p));
}

class NACS_EXPORT OldPulser : public OldPulserBase {
    Driver m_driver;
    std::atomic_bool m_running;

    OldPulser() = delete;
    OldPulser(const OldPulser&) = delete;
    void operator=(const OldPulser&) = delete;
public:
    OldPulser(OldPulser &&other);
    OldPulser(volatile void *base);

    ~OldPulser() {}
    // TODO
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
     * OldPulser are considered time-critical.  If a pulse finishes, and there is
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
    intptr_t get_base() const;

    double get_dds_freq_f(int i);
    double get_dds_phase_f(int i);
    double get_dds_amp_f(int i);

    bool self_test(int ndds, int cycle=1);
    bool test_regs();
    bool test_dds(int i);
    uint32_t num_results();
    bool is_finished();
    void release_hold();
};

NACS_INLINE
OldPulser::OldPulser(OldPulser &&other)
    : OldPulserBase(static_cast<OldPulserBase&&>(other)),
      m_driver(std::move(other.m_driver)),
      m_running(other.m_running.exchange(false))
{
}

NACS_INLINE
OldPulser::OldPulser(volatile void *base)
    : OldPulserBase(),
      m_driver(base),
      m_running(false)
{
}

NACS_INLINE intptr_t
OldPulser::get_base() const
{
    return m_driver.getBase();
}

OldPulser &get_pulser();

NACS_INLINE double
OldPulser::get_dds_freq_f(int i)
{
    return DDSCvt::num2freq(get_dds_freq(i), PULSER_AD9914_CLK);
}

NACS_INLINE double
OldPulser::get_dds_phase_f(int i)
{
    return DDSCvt::num2phase(get_dds_phase(i));
}

NACS_INLINE double
OldPulser::get_dds_amp_f(int i)
{
    return DDSCvt::num2amp(get_dds_amp(i));
}

}
}

#endif
