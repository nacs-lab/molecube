/*****************************************************************************
 * Filename:          PULSER_v4_00_b/src/pulse_controller.h
 * Version:           4.00.b
 * Description:       pulse_controller Driver Header File
 * Date:              Sat Jul 14 14:34:13 2012 (by Create and Import Peripheral Wizard)
 *****************************************************************************/

#include <nacs-utils/mem.h>

#ifndef __NACS_PULSER_CTRL_IO_H__
#define __NACS_PULSER_CTRL_IO_H__

namespace NaCs {
namespace Pulser {

/**
 *
 * Write a value to a PULSE_CONTROLLER register. A 32 bit write is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is written.
 *
 * @param   BaseAddress is the base address of the PULSE_CONTROLLER device.
 * @param   RegOffset is the register offset from the base to write to.
 * @param   Data is the data written to the register.
 *
 */
static NACS_INLINE void
mWriteReg(volatile void *base, off_t offset, uint32_t data)
{
    mem_write32((volatile char*)base + offset, data);
}

/**
 *
 * Read a value from a PULSE_CONTROLLER register. A 32 bit read is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is read from the register. The most significant data
 * will be read as 0.
 *
 * @param   BaseAddress is the base address of the PULSE_CONTROLLER device.
 * @param   RegOffset is the register offset from the base to write to.
 *
 * @return  Data is the data from the register.
 *
 */
static NACS_INLINE uint32_t
mReadReg(volatile void *base, off_t offset)
{
    return mem_read32((volatile char*)base + offset);
}

/**
 * User Logic Slave Space Offsets
 * -- SLV_REG(n) : user logic slave module register n
 */
static constexpr uint32_t usr_slv_space_offset = 0x0;
static inline constexpr uint32_t
slvRegOffset(uint32_t n)
{
    return usr_slv_space_offset + n * 4;
}

/**
 *
 * Write/Read 32 bit value to/from PULSE_CONTROLLER user logic slave registers.
 *
 * @param   base is the base address of the PULSE_CONTROLLER device.
 * @param   offset is the offset from the slave register to write to or read from.
 * @param   val is the data written to the register.
 *
 * @return  The data from the user logic slave register.
 *
 */
static NACS_INLINE void
mWriteSlaveReg(volatile void *base, int n, uint32_t val)
{
    mWriteReg(base, slvRegOffset(n), val);
}

static NACS_INLINE uint32_t
mReadSlaveReg(volatile void *base, int n)
{
    return mReadReg(base, slvRegOffset(n));
}

/**
 * Software Reset Space Register Offsets
 * -- RST : software reset register
 */
static constexpr uint32_t soft_rst_space_offset = 0x100;
static constexpr uint32_t rst_reg_offset = soft_rst_space_offset + 0x0;

/**
 *
 * Reset PULSE_CONTROLLER via software.
 *
 * @param   BaseAddress is the base address of the PULSE_CONTROLLER device.
 *
 */
static NACS_INLINE void
mReset(volatile void *base)
{
    /**
     * Software Reset Masks
     * -- SOFT_RESET : software reset
     */
    static constexpr uint32_t soft_reset = 0x0000000A;
    mem_write32((volatile char*)base + rst_reg_offset, soft_reset);
}

/* Defines the number of registers available for read and write */
static constexpr uint32_t usr_num_reg = 32;

}
}

#endif
