/*****************************************************************************
 * Filename:          PULSER_v4_00_b/src/pulse_controller.h
 * Version:           4.00.b
 * Description:       pulse_controller Driver Header File
 * Date:              Sat Jul 14 14:34:13 2012 (by Create and Import Peripheral Wizard)
 *****************************************************************************/

#include <nacs-utils/utils.h>

#ifndef PULSER_IO_H
#define PULSER_IO_H

NACS_BEGIN_DECLS

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

/**
 * Software Reset Masks
 * -- SOFT_RESET : software reset
 */
#define SOFT_RESET (0x0000000A)

/**************************** Type Definitions *****************************/


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
 * @return  None.
 *
 * @note
 * C-style signature:
 * void PULSER_mWriteReg(unsigned BaseAddress, unsigned RegOffset, unsigned Data)
 *
 */
#define PULSER_mWriteReg(BaseAddress, RegOffset, Data)          \
    Xil_Out32((BaseAddress) + (RegOffset), (unsigned)(Data))

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
 * @note
 * C-style signature:
 * unsigned PULSER_mReadReg(unsigned BaseAddress, unsigned RegOffset)
 *
 */
#define PULSER_mReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))

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
 * @note
 * C-style signature:
 * void PULSER_mWriteSlaveRegn(unsigned BaseAddress, unsigned RegOffset, unsigned Value)
 * unsigned PULSER_mReadSlaveRegn(unsigned BaseAddress, unsigned RegOffset)
 *
 */
#define PULSER_mWriteSlaveReg0(BaseAddress, RegOffset, Value)           \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG0_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg1(BaseAddress, RegOffset, Value)           \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG1_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg2(BaseAddress, RegOffset, Value)           \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG2_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg3(BaseAddress, RegOffset, Value)           \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG3_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg4(BaseAddress, RegOffset, Value)           \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG4_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg5(BaseAddress, RegOffset, Value)           \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG5_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg6(BaseAddress, RegOffset, Value)           \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG6_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg7(BaseAddress, RegOffset, Value)           \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG7_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg8(BaseAddress, RegOffset, Value)           \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG8_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg9(BaseAddress, RegOffset, Value)           \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG9_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg10(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG10_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg11(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG11_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg12(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG12_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg13(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG13_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg14(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG14_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg15(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG15_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg16(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG16_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg17(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG17_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg18(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG18_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg19(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG19_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg20(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG20_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg21(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG21_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg22(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG22_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg23(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG23_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg24(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG24_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg25(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG25_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg26(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG26_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg27(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG27_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg28(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG28_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg29(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG29_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg30(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG30_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSER_mWriteSlaveReg31(BaseAddress, RegOffset, Value)          \
    Xil_Out32((BaseAddress) + (PULSER_SLV_REG31_OFFSET) + (RegOffset), (unsigned)(Value))

#define PULSER_mReadSlaveReg0(BaseAddress, RegOffset)                   \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG0_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg1(BaseAddress, RegOffset)                   \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG1_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg2(BaseAddress, RegOffset)                   \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG2_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg3(BaseAddress, RegOffset)                   \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG3_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg4(BaseAddress, RegOffset)                   \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG4_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg5(BaseAddress, RegOffset)                   \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG5_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg6(BaseAddress, RegOffset)                   \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG6_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg7(BaseAddress, RegOffset)                   \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG7_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg8(BaseAddress, RegOffset)                   \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG8_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg9(BaseAddress, RegOffset)                   \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG9_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg10(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG10_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg11(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG11_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg12(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG12_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg13(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG13_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg14(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG14_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg15(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG15_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg16(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG16_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg17(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG17_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg18(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG18_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg19(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG19_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg20(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG20_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg21(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG21_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg22(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG22_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg23(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG23_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg24(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG24_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg25(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG25_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg26(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG26_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg27(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG27_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg28(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG28_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg29(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG29_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg30(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG30_OFFSET) + (RegOffset))
#define PULSER_mReadSlaveReg31(BaseAddress, RegOffset)                  \
    Xil_In32((BaseAddress) + (PULSER_SLV_REG31_OFFSET) + (RegOffset))

/**
 *
 * Reset PULSE_CONTROLLER via software.
 *
 * @param   BaseAddress is the base address of the PULSE_CONTROLLER device.
 *
 * @return  None.
 *
 * @note
 * C-style signature:
 * void PULSER_mReset(unsigned BaseAddress)
 *
 */
#define PULSER_mReset(BaseAddress)                                      \
    Xil_Out32((BaseAddress)+(PULSER_RST_REG_OFFSET), SOFT_RESET)

/************************** Function Prototypes ****************************/


/**
 *
 * Run a self-test on the driver/device. Note this may be a destructive test if
 * resets of the device are performed.
 *
 * If the hardware system is not built correctly, this function may never
 * return to the caller.
 *
 * @param   baseaddr_p is the base address of the PULSE_CONTROLLER instance to be worked on.
 *
 * @return
 *
 *    - XST_SUCCESS   if all self-test code passed
 *    - XST_FAILURE   if any self-test code failed
 *
 * @note    Caching must be turned off for this function to work.
 * @note    Self test may fail if data memory and device are not on the same bus.
 *
 */
unsigned PULSER_SelfTest0(void * baseaddr_p);
/**
 *  Defines the number of registers available for read and write*/
#define PULSER_USER_NUM_REG 32

NACS_END_DECLS

#endif /** PULSER_IO_H */
