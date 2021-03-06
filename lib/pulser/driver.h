#ifndef __NACS_PULSER_DRIVER_H__
#define __NACS_PULSER_DRIVER_H__

#include "ctrl_io.h"

namespace NaCs {
namespace Pulser {

/**
 * This is the class that does all the low level stuff
 * There's no lock to protect anything and there should only be
 * one writer and one reader for the FIFO (register 31). Other registers
 * can be read and write by multiple threads at the same time.
 */
class Driver {
    volatile void *const m_base;
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
    }
    intptr_t
    getBase() const
    {
        return intptr_t(m_base);
    }

    // Read
    inline uint32_t
    readReg(uint32_t reg) const
    {
        return mReadSlaveReg(m_base, reg);
    }
    inline uint32_t
    numResults() const
    {
        return (readReg(2) >> 4) & 31;
    }
    inline bool
    isFinished() const
    {
        return readReg(2) & 0x4;
    }
    inline uint32_t
    getTTLHighMask() const
    {
        return readReg(0);
    }
    inline uint32_t
    getTTLLowMask() const
    {
        return readReg(1);
    }
    inline uint32_t
    readResult() const
    {
        return readReg(31);
    }
    inline bool
    timingOK() const
    {
        return !(readReg(2) & 0x1);
    }
    inline uint32_t
    getCurTTL() const
    {
        return readReg(4);
    }

    // Write
    inline void
    writeReg(uint32_t reg, uint32_t val) const
    {
        mWriteSlaveReg(m_base, reg, val);
    }
    inline void
    shortPulse(uint32_t ctrl, uint32_t op) const
    {
        writeReg(31, op);
        writeReg(31, ctrl);
    }
    // TTL functions: pulse_io = (ttl_out | high_mask) & (~low_mask);
    inline void
    setTTLHighMask(uint32_t high_mask) const
    {
        writeReg(0, high_mask);
    }
    inline void
    setTTLLowMask(uint32_t low_mask) const
    {
        writeReg(1, low_mask);
    }
    // release hold.  pulses can run
    inline void
    releaseHold()
    {
        writeReg(3, readReg(3) & ~0x80);
    }
    // set hold. pulses are stopped
    inline void
    setHold()
    {
        writeReg(3, readReg(3) | 0x80);
    }
    // toggle init. reset prior to new sequence
    inline void
    toggleInit()
    {
        uint32_t r3 = readReg(3);
        writeReg(3, r3 | 0x00000100);
        writeReg(3, r3 & ~0x00000100);
    }
};

volatile void *mapPulserAddr();

}
}

#endif
