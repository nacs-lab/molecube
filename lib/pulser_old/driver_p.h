#ifndef __NACS_PULSER_DRIVER_P_H__
#define __NACS_PULSER_DRIVER_P_H__

#include <nacs-pulser/ctrl_io.h>

#include <type_traits>
#include <atomic>

namespace NaCs {
namespace Pulser {

/**
 * This is the class that does all the low level stuff
 * There's no lock to protect anything and there should only be
 * one writer and one reader.
 */
class Driver {
    volatile void *m_base;
    Driver() = delete;
    Driver(const Driver&) = delete;
    void operator=(const Driver&) = delete;
public:
    Driver(volatile void *base)
        : m_base(base)
    {}
    Driver(Driver &&other)
        : m_base(other.m_base)
    {
        other.m_base = nullptr;
    }
    intptr_t
    getBase() const
    {
        return intptr_t(m_base);
    }

    // Read
    inline uint32_t
    readReg(uint32_t reg)
    {
        return mReadSlaveReg(m_base, reg);
    }
    inline uint32_t
    numResults()
    {
        return (readReg(2) >> 4) & 31;
    }
    inline bool
    isFinished()
    {
        return readReg(2) & 0x4;
    }
    inline uint32_t
    getTTLHighMask()
    {
        return readReg(0);
    }
    inline uint32_t
    getTTLLowMask()
    {
        return readReg(1);
    }
    inline uint32_t
    readResult()
    {
        return readReg(31);
    }
    inline bool
    timingOK()
    {
        return !(readReg(2) & 0x1);
    }

    // Write
    inline void
    writeReg(uint32_t reg, uint32_t val)
    {
        mWriteSlaveReg(m_base, reg, val);
    }
    inline void
    shortPulse(uint32_t ctrl, uint32_t op)
    {
        writeReg(31, op);
        writeReg(31, ctrl);
    }
    // TTL functions: pulse_io = (ttl_out | high_mask) & (~low_mask);
    inline void
    setTTLHighMask(uint32_t high_mask)
    {
        writeReg(0, high_mask);
    }
    inline void
    setTTLLowMask(uint32_t low_mask)
    {
        writeReg(1, low_mask);
    }
};

}
}

#endif
