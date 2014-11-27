#include <nacs-utils/utils.h>
#include <atomic>

#ifndef __NACS_PULSER_PULSER_H__
#define __NACS_PULSER_PULSER_H__

namespace NaCs {
namespace Pulser {

class NACS_EXPORT PulserBase {
public:
    virtual ~PulserBase() {};
    void clock_out(unsigned divider);
    void set_dds_two_bytes(int i, uint32_t addr, uint32_t data);
    void set_dds_four_bytes(int i, uint32_t addr, uint32_t data);
    void pulse(unsigned t, unsigned flags, unsigned operand);
    void clear_timing_check();
    void set_dds_freq(int i, uint32_t ftw);
    void set_dds_amp(int i, uint32_t amp);
    void dds_reset(int i);
    void set_dds_phase(int i, uint16_t phase);
    void set_ttl_mask(uint32_t high_mask, uint32_t low_mask);
    virtual void short_pulse(uint32_t control, uint32_t operand);
protected:
    void raw_pulse(uint32_t control, uint32_t operand);
    void dds_reset(unsigned i);
private:
    virtual void write_reg(unsigned reg, uint32_t val) = 0;
};

class BaseProgram {
    uint32_t *m_prog;
    size_t m_len;
    size_t m_alloc_len;
public:
    BaseProgram()
        : m_prog(nullptr),
          m_len(0),
          m_alloc_len(0)
    {
    }
    ~BaseProgram();
    inline const uint32_t*
    program() const
    {
        return m_prog;
    }
    inline size_t
    len() const
    {
        return m_len;
    }
protected:
    template <typename... ArgTypes>
    inline void
    writes(ArgTypes&&... args)
    {
        const size_t nargs = sizeof...(args);
        uint32_t data[] = {args...};
        reserve_space(nargs);
        memcpy(m_prog + m_len, data, nargs * sizeof(uint32_t));
        m_len += nargs;
    }
private:
    void reserve_space(size_t len);
};

class NACS_EXPORT Pulser : public PulserBase {
    volatile void *m_base;
    std::atomic_bool m_running;
    Pulser() = delete;
    Pulser(const Pulser&) = delete;
    Pulser &operator=(const Pulser&) = delete;
public:
    Pulser(Pulser &&other)
        : m_base(other.m_base),
          m_running(other.m_running.exchange(false))
    {
    }
    Pulser(volatile void *base)
        : m_base(base),
          m_running(false)
    {
    }
    ~Pulser() {}
    void init(unsigned ndds, bool reset);
    void run(const BaseProgram &prog);
    void wait();
    void get_ttl_mask(uint32_t *high_mask, uint32_t *low_mask);
    void write_reg(unsigned reg, uint32_t val) override;
    uint32_t read_reg(unsigned reg);
    uint32_t pop_result();
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
    volatile void*
    get_base() const
    {
        return m_base;
    }
private:
    uint32_t num_results();
    void debug_regs();
    bool is_finished();
    void release_hold();
};

class Program : public BaseProgram, public PulserBase {
    uint32_t m_flags;
    uint16_t m_phases[32];
public:
    Program()
        : BaseProgram(),
          m_flags(0)
    {
    }
    void short_pulse(uint32_t control, uint32_t operand) override;
    void enable_timing_check();
    void disable_timing_check();
    void dds_reset(int i);
    void set_dds_phase(int i, uint16_t phase);
    void shift_dds_phase(int i, uint16_t phase);
private:
    void write_reg(unsigned reg, uint32_t val) override;
};

}
}

#endif
