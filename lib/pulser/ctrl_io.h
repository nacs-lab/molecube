#include <nacs-utils/mem.h>

#ifndef __NACS_PULSER_CTRL_IO_H__
#define __NACS_PULSER_CTRL_IO_H__

// TODO: generate the parameters in this file from the hardware design
/**
 * This is the file generated with parameters (register offsets) from the
 * hardware design. The pulse_controller has 32x 32bits slave registers which
 * are used to communicate between the PS and PL.
 */

namespace NaCs {
namespace Pulser {

/**
 * Write a value to a PULSE_CONTROLLER register. A 32 bit write is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is written.
 *
 * @param base is the base address of the PULSE_CONTROLLER device.
 * @param offset is the register offset from the base to write to.
 * @param data is the data written to the register.
 */
static NACS_INLINE void
mWriteReg(volatile void *base, off_t offset, uint32_t data)
{
    Mem::write<uint32_t>((volatile char*)base + offset, data);
}

/**
 * Read a value from a PULSE_CONTROLLER register. A 32 bit read is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is read from the register. The most significant data
 * will be read as 0.
 *
 * @param base is the base address of the PULSE_CONTROLLER device.
 * @param offset is the register offset from the base to write to.
 *
 * @return data is the data from the register.
 */
static NACS_INLINE uint32_t
mReadReg(volatile void *base, off_t offset)
{
    return Mem::read<uint32_t>((volatile char*)base + offset);
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

}
}

#endif
