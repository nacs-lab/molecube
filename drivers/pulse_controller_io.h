/*****************************************************************************
 * Filename:          PULSER_v4_00_b/src/pulse_controller.h
 * Version:           4.00.b
 * Description:       pulse_controller Driver Header File
 * Date:              Sat Jul 14 14:34:13 2012 (by Create and Import Peripheral Wizard)
 *****************************************************************************/

#include <nacs-utils/utils.h>
#include <nacs-utils/mem.h>

#ifndef PULSER_IO_H
#define PULSER_IO_H

/************************** Constant Definitions ***************************/

/**
 * User Logic Slave Space Offsets
 * -- SLV_REG0 : user logic slave module register 0
 * -- SLV_REG1 : user logic slave module register 1
 * -- SLV_REG2 : user logic slave module register 2
 * -- SLV_REG3 : user logic slave module register 3
 * -- SLV_REG4 : user logic slave module register 4
 * -- SLV_REG5 : user logic slave module register 5
 * -- SLV_REG6 : user logic slave module register 6
 * -- SLV_REG7 : user logic slave module register 7
 * -- SLV_REG8 : user logic slave module register 8
 * -- SLV_REG9 : user logic slave module register 9
 * -- SLV_REG10 : user logic slave module register 10
 * -- SLV_REG11 : user logic slave module register 11
 * -- SLV_REG12 : user logic slave module register 12
 * -- SLV_REG13 : user logic slave module register 13
 * -- SLV_REG14 : user logic slave module register 14
 * -- SLV_REG15 : user logic slave module register 15
 * -- SLV_REG16 : user logic slave module register 16
 * -- SLV_REG17 : user logic slave module register 17
 * -- SLV_REG18 : user logic slave module register 18
 * -- SLV_REG19 : user logic slave module register 19
 * -- SLV_REG20 : user logic slave module register 20
 * -- SLV_REG21 : user logic slave module register 21
 * -- SLV_REG22 : user logic slave module register 22
 * -- SLV_REG23 : user logic slave module register 23
 * -- SLV_REG24 : user logic slave module register 24
 * -- SLV_REG25 : user logic slave module register 25
 * -- SLV_REG26 : user logic slave module register 26
 * -- SLV_REG27 : user logic slave module register 27
 * -- SLV_REG28 : user logic slave module register 28
 * -- SLV_REG29 : user logic slave module register 29
 * -- SLV_REG30 : user logic slave module register 30
 * -- SLV_REG31 : user logic slave module register 31
 */
#define PULSER_USER_SLV_SPACE_OFFSET (0x00000000)
#define PULSER_SLV_REG0_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000000)
#define PULSER_SLV_REG1_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000004)
#define PULSER_SLV_REG2_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000008)
#define PULSER_SLV_REG3_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x0000000C)
#define PULSER_SLV_REG4_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000010)
#define PULSER_SLV_REG5_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000014)
#define PULSER_SLV_REG6_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000018)
#define PULSER_SLV_REG7_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x0000001C)
#define PULSER_SLV_REG8_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000020)
#define PULSER_SLV_REG9_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000024)
#define PULSER_SLV_REG10_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000028)
#define PULSER_SLV_REG11_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x0000002C)
#define PULSER_SLV_REG12_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000030)
#define PULSER_SLV_REG13_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000034)
#define PULSER_SLV_REG14_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000038)
#define PULSER_SLV_REG15_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x0000003C)
#define PULSER_SLV_REG16_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000040)
#define PULSER_SLV_REG17_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000044)
#define PULSER_SLV_REG18_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000048)
#define PULSER_SLV_REG19_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x0000004C)
#define PULSER_SLV_REG20_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000050)
#define PULSER_SLV_REG21_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000054)
#define PULSER_SLV_REG22_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000058)
#define PULSER_SLV_REG23_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x0000005C)
#define PULSER_SLV_REG24_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000060)
#define PULSER_SLV_REG25_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000064)
#define PULSER_SLV_REG26_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000068)
#define PULSER_SLV_REG27_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x0000006C)
#define PULSER_SLV_REG28_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000070)
#define PULSER_SLV_REG29_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000074)
#define PULSER_SLV_REG30_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x00000078)
#define PULSER_SLV_REG31_OFFSET (PULSER_USER_SLV_SPACE_OFFSET + 0x0000007C)

/**
 * Software Reset Space Register Offsets
 * -- RST : software reset register
 */
#define PULSER_SOFT_RST_SPACE_OFFSET (0x00000100)
#define PULSER_RST_REG_OFFSET (PULSER_SOFT_RST_SPACE_OFFSET + 0x00000000)

/***************** Macros (Inline Functions) Definitions *******************/

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
 *
 * Write/Read 32 bit value to/from PULSE_CONTROLLER user logic slave registers.
 *
 * @param   BaseAddress is the base address of the PULSE_CONTROLLER device.
 * @param   RegOffset is the offset from the slave register to write to or read from.
 * @param   Value is the data written to the register.
 *
 * @return  Data is the data from the user logic slave register.
 *
 */
#define DEF_PULSER_RW_SLAVE(n)                                  \
    static NACS_INLINE void                                     \
    PULSER_mWriteSlaveReg##n(volatile void *base, off_t offset, \
                             uint32_t data)                     \
    {                                                           \
        mem_write32((volatile char*)base +                      \
                    PULSER_SLV_REG##n##_OFFSET + offset, data); \
    }                                                           \
    static NACS_INLINE uint32_t                                 \
    PULSER_mReadSlaveReg##n(volatile void *base, off_t offset)  \
    {                                                           \
        return mem_read32((volatile char*)base +                \
                          PULSER_SLV_REG##n##_OFFSET + offset); \
    }
DEF_PULSER_RW_SLAVE(0)
DEF_PULSER_RW_SLAVE(1)
DEF_PULSER_RW_SLAVE(2)
DEF_PULSER_RW_SLAVE(3)
DEF_PULSER_RW_SLAVE(4)
DEF_PULSER_RW_SLAVE(5)
DEF_PULSER_RW_SLAVE(6)
DEF_PULSER_RW_SLAVE(7)
DEF_PULSER_RW_SLAVE(8)
DEF_PULSER_RW_SLAVE(9)
DEF_PULSER_RW_SLAVE(10)
DEF_PULSER_RW_SLAVE(11)
DEF_PULSER_RW_SLAVE(12)
DEF_PULSER_RW_SLAVE(13)
DEF_PULSER_RW_SLAVE(14)
DEF_PULSER_RW_SLAVE(15)
DEF_PULSER_RW_SLAVE(16)
DEF_PULSER_RW_SLAVE(17)
DEF_PULSER_RW_SLAVE(18)
DEF_PULSER_RW_SLAVE(19)
DEF_PULSER_RW_SLAVE(20)
DEF_PULSER_RW_SLAVE(21)
DEF_PULSER_RW_SLAVE(22)
DEF_PULSER_RW_SLAVE(23)
DEF_PULSER_RW_SLAVE(24)
DEF_PULSER_RW_SLAVE(25)
DEF_PULSER_RW_SLAVE(26)
DEF_PULSER_RW_SLAVE(27)
DEF_PULSER_RW_SLAVE(28)
DEF_PULSER_RW_SLAVE(29)
DEF_PULSER_RW_SLAVE(30)
DEF_PULSER_RW_SLAVE(31)

#undef DEF_PULSER_RW_SLAVE

/**
 *
 * Reset PULSE_CONTROLLER via software.
 *
 * @param   BaseAddress is the base address of the PULSE_CONTROLLER device.
 *
 * @return  None.
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
