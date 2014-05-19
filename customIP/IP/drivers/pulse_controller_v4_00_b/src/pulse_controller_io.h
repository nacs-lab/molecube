/*****************************************************************************
* Filename:          pulse_controller_v4_00_b/src/pulse_controller.h
* Version:           4.00.b
* Description:       pulse_controller Driver Header File
* Date:              Sat Jul 14 14:34:13 2012 (by Create and Import Peripheral Wizard)
*****************************************************************************/

#ifndef PULSE_CONTROLLER_IO_H
#define PULSE_CONTROLLER_IO_H

#ifdef LINUX_OS
#define Xil_Out32(addr, data) *((volatile unsigned *)(addr)) = data
#define Xil_In32(addr) *((volatile unsigned*)(addr))
#endif

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
#define PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET (0x00000000)
#define PULSE_CONTROLLER_SLV_REG0_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000000)
#define PULSE_CONTROLLER_SLV_REG1_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000004)
#define PULSE_CONTROLLER_SLV_REG2_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000008)
#define PULSE_CONTROLLER_SLV_REG3_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x0000000C)
#define PULSE_CONTROLLER_SLV_REG4_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000010)
#define PULSE_CONTROLLER_SLV_REG5_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000014)
#define PULSE_CONTROLLER_SLV_REG6_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000018)
#define PULSE_CONTROLLER_SLV_REG7_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x0000001C)
#define PULSE_CONTROLLER_SLV_REG8_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000020)
#define PULSE_CONTROLLER_SLV_REG9_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000024)
#define PULSE_CONTROLLER_SLV_REG10_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000028)
#define PULSE_CONTROLLER_SLV_REG11_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x0000002C)
#define PULSE_CONTROLLER_SLV_REG12_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000030)
#define PULSE_CONTROLLER_SLV_REG13_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000034)
#define PULSE_CONTROLLER_SLV_REG14_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000038)
#define PULSE_CONTROLLER_SLV_REG15_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x0000003C)
#define PULSE_CONTROLLER_SLV_REG16_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000040)
#define PULSE_CONTROLLER_SLV_REG17_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000044)
#define PULSE_CONTROLLER_SLV_REG18_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000048)
#define PULSE_CONTROLLER_SLV_REG19_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x0000004C)
#define PULSE_CONTROLLER_SLV_REG20_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000050)
#define PULSE_CONTROLLER_SLV_REG21_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000054)
#define PULSE_CONTROLLER_SLV_REG22_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000058)
#define PULSE_CONTROLLER_SLV_REG23_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x0000005C)
#define PULSE_CONTROLLER_SLV_REG24_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000060)
#define PULSE_CONTROLLER_SLV_REG25_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000064)
#define PULSE_CONTROLLER_SLV_REG26_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000068)
#define PULSE_CONTROLLER_SLV_REG27_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x0000006C)
#define PULSE_CONTROLLER_SLV_REG28_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000070)
#define PULSE_CONTROLLER_SLV_REG29_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000074)
#define PULSE_CONTROLLER_SLV_REG30_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000078)
#define PULSE_CONTROLLER_SLV_REG31_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x0000007C)

/**
 * Software Reset Space Register Offsets
 * -- RST : software reset register
 */
#define PULSE_CONTROLLER_SOFT_RST_SPACE_OFFSET (0x00000100)
#define PULSE_CONTROLLER_RST_REG_OFFSET (PULSE_CONTROLLER_SOFT_RST_SPACE_OFFSET + 0x00000000)

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
 * 	void PULSE_CONTROLLER_mWriteReg(unsigned BaseAddress, unsigned RegOffset, unsigned Data)
 *
 */
#define PULSE_CONTROLLER_mWriteReg(BaseAddress, RegOffset, Data) \
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
 * 	unsigned PULSE_CONTROLLER_mReadReg(unsigned BaseAddress, unsigned RegOffset)
 *
 */
#define PULSE_CONTROLLER_mReadReg(BaseAddress, RegOffset) \
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
 * 	void PULSE_CONTROLLER_mWriteSlaveRegn(unsigned BaseAddress, unsigned RegOffset, unsigned Value)
 * 	unsigned PULSE_CONTROLLER_mReadSlaveRegn(unsigned BaseAddress, unsigned RegOffset)
 *
 */
#define PULSE_CONTROLLER_mWriteSlaveReg0(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG0_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg1(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG1_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg2(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG2_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg3(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG3_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg4(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG4_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg5(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG5_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg6(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG6_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg7(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG7_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg8(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG8_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg9(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG9_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg10(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG10_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg11(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG11_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg12(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG12_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg13(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG13_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg14(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG14_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg15(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG15_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg16(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG16_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg17(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG17_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg18(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG18_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg19(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG19_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg20(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG20_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg21(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG21_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg22(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG22_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg23(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG23_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg24(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG24_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg25(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG25_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg26(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG26_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg27(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG27_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg28(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG28_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg29(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG29_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg30(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG30_OFFSET) + (RegOffset), (unsigned)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg31(BaseAddress, RegOffset, Value) \
 	Xil_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG31_OFFSET) + (RegOffset), (unsigned)(Value))

#define PULSE_CONTROLLER_mReadSlaveReg0(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG0_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg1(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG1_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg2(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG2_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg3(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG3_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg4(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG4_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg5(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG5_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg6(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG6_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg7(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG7_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg8(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG8_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg9(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG9_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg10(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG10_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg11(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG11_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg12(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG12_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg13(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG13_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg14(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG14_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg15(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG15_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg16(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG16_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg17(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG17_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg18(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG18_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg19(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG19_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg20(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG20_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg21(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG21_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg22(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG22_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg23(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG23_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg24(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG24_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg25(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG25_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg26(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG26_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg27(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG27_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg28(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG28_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg29(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG29_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg30(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG30_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg31(BaseAddress, RegOffset) \
 	Xil_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG31_OFFSET) + (RegOffset))

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
 * 	void PULSE_CONTROLLER_mReset(unsigned BaseAddress)
 *
 */
#define PULSE_CONTROLLER_mReset(BaseAddress) \
 	Xil_Out32((BaseAddress)+(PULSE_CONTROLLER_RST_REG_OFFSET), SOFT_RESET)

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
unsigned PULSE_CONTROLLER_SelfTest0(void * baseaddr_p);
/**
*  Defines the number of registers available for read and write*/
#define PULSE_CONTROLLER_USER_NUM_REG 32


#endif /** PULSE_CONTROLLER_IO_H */
