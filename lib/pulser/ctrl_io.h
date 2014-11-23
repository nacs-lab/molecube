/*****************************************************************************
 * Filename:          PULSER_v4_00_b/src/pulse_controller.h
 * Version:           4.00.b
 * Description:       pulse_controller Driver Header File
 * Date:              Sat Jul 14 14:34:13 2012 (by Create and Import Peripheral Wizard)
 *****************************************************************************/

#include <nacs-utils/mem.h>

#ifndef PULSER_IO_H
#define PULSER_IO_H

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
PULSER_mWriteReg(volatile void *base, off_t offset, uint32_t data)
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
PULSER_mReadReg(volatile void *base, off_t offset)
{
    return mem_read32((volatile char*)base + offset);
}

/**
 * User Logic Slave Space Offsets
 * -- SLV_REG(n) : user logic slave module register n
 */
#define PULSER_USER_SLV_SPACE_OFFSET (0x00000000)
#define PULSER_SLV_REG_OFFSET(n) (PULSER_USER_SLV_SPACE_OFFSET + n * 4)

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
PULSER_mWriteSlaveReg(volatile void *base, int n, uint32_t val)
{
    PULSER_mWriteReg(base, PULSER_SLV_REG_OFFSET(n), val);
}

static NACS_INLINE uint32_t
PULSER_mReadSlaveReg(volatile void *base, int n)
{
    return PULSER_mReadReg(base, PULSER_SLV_REG_OFFSET(n));
}

/**
 * Software Reset Space Register Offsets
 * -- RST : software reset register
 */
#define PULSER_SOFT_RST_SPACE_OFFSET (0x00000100)
#define PULSER_RST_REG_OFFSET (PULSER_SOFT_RST_SPACE_OFFSET + 0x00000000)

/**
 *
 * Reset PULSE_CONTROLLER via software.
 *
 * @param   BaseAddress is the base address of the PULSE_CONTROLLER device.
 *
 */
static NACS_INLINE void
PULSER_mReset(volatile void *base)
{
    /**
     * Software Reset Masks
     * -- SOFT_RESET : software reset
     */
    static const uint32_t SOFT_RESET = 0x0000000A;
    mem_write32((volatile char*)base + PULSER_RST_REG_OFFSET, SOFT_RESET);
}

/* Defines the number of registers available for read and write */
#define PULSER_USER_NUM_REG 32

#endif /** PULSER_IO_H */
