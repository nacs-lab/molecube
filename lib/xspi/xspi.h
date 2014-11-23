/******************************************************************************
 *
 * (c) Copyright 2001-2013 Xilinx, Inc. All rights reserved.
 *
 * This file contains confidential and proprietary information of Xilinx, Inc.
 * and is protected under U.S. and international copyright and other
 * intellectual property laws.
 *
 * DISCLAIMER
 * This disclaimer is not a license and does not grant any rights to the
 * materials distributed herewith. Except as otherwise provided in a valid
 * license issued to you by Xilinx, and to the maximum extent permitted by
 * applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
 * FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
 * MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
 * and (2) Xilinx shall not be liable (whether in contract or tort, including
 * negligence, or under any other theory of liability) for any loss or damage
 * of any kind or nature related to, arising under or in connection with these
 * materials, including for any direct, or any indirect, special, incidental,
 * or consequential loss or damage (including loss of data, profits, goodwill,
 * or any type of loss or damage suffered as a result of any action brought by
 * a third party) even if such damage or loss was reasonably foreseeable or
 * Xilinx had been advised of the possibility of the same.
 *
 * CRITICAL APPLICATIONS
 * Xilinx products are not designed or intended to be fail-safe, or for use in
 * any application requiring fail-safe performance, such as life-support or
 * safety devices or systems, Class III medical devices, nuclear facilities,
 * applications related to the deployment of airbags, or any other applications
 * that could lead to death, personal injury, or severe property or
 * environmental damage (individually and collectively, "Critical
 * Applications"). Customer assumes the sole risk and liability of any use of
 * Xilinx products in Critical Applications, subject only to applicable laws
 * and regulations governing limitations on product liability.
 *
 * THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
 * AT ALL TIMES.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xspi.h
 *
 * This component contains the implementation of the XSpi component. It is the
 * driver for an SPI master or slave device. It supports 8-bit, 16-bit and 32-bit
 * wide data transfers.
 *
 * SPI is a 4-wire serial interface. It is a full-duplex, synchronous bus that
 * facilitates communication between one master and one slave. The device is
 * always full-duplex, which means that for every byte sent, one is received, and
 * vice-versa. The master controls the clock, so it can regulate when it wants
 * to send or receive data. The slave is under control of the master, it must
 * respond quickly since it has no control of the clock and must send/receive
 * data as fast or as slow as the master does.
 *
 * The application software between master and slave must implement a higher
 * layer protocol so that slaves know what to transmit to the master and when.
 *
 * <b>Initialization & Configuration</b>
 *
 * The XSpi_Config structure is used by the driver to configure itself. This
 * configuration structure is typically created by the tool-chain based on HW
 * build properties.
 *
 * To support multiple runtime loading and initialization strategies employed
 * by various operating systems, the driver instance can be initialized in one
 * of the following ways:
 *
 *   - XSpi_Initialize(InstancePtr, DeviceId) - The driver looks up its own
 *     configuration structure created by the tool-chain based on an ID provided
 *     by the tool-chain.
 *
 *   - XSpi_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr) - Uses a
 *     configuration structure provided by the caller. If running in a system
 *     with address translation, the provided virtual memory base address
 *     replaces the physical address present in the configuration structure.
 *
 * <b>Multiple Masters</b>
 *
 * More than one master can exist, but arbitration is the responsibility of the
 * higher layer software. The device driver does not perform any type of
 * arbitration.
 *
 * <b>Multiple Slaves</b>
 *
 * Multiple slaves are supported by adding additional slave select (SS) signals
 * to each device, one for each slave on the bus. The driver ensures that only
 * one slave can be selected at any one time.
 *
 * <b>FIFOs</b>
 *
 * The SPI hardware is parameterized such that it can be built with or without
 * FIFOs. When using FIFOs, both send and receive must have FIFOs. The driver
 * will not function correctly if one direction has a FIFO but the other
 * direction does not. The frequency of the interrupts which occur is
 * proportional to the data rate such that high data rates without the FIFOs
 * could cause the software to consume large amounts of processing time. The
 * driver is designed to work with or without the FIFOs.
 *
 * <b>Interrupts</b>
 *
 * The user must connect the interrupt handler of the driver,
 * XSpi_InterruptHandler to an interrupt system such that it will be called when
 * an interrupt occurs. This function does not save and restore the processor
 * context such that the user must provide this processing.
 *
 * The driver handles the following interrupts:
 * - Data Transmit Register/FIFO Empty
 * - Data Transmit FIFO Half Empty
 * - Data Transmit Register/FIFO Underrun
 * - Data Receive Register/FIFO Overrun
 * - Mode Fault Error
 * - Slave Mode Fault Error
 * - Slave Mode Select
 * - Data Receive FIFO not Empty
 *
 * The Data Transmit Register/FIFO Empty interrupt indicates that the SPI device
 * has transmitted all the data available to transmit, and now its data register
 * (or FIFO) is empty. The driver uses this interrupt to indicate progress while
 * sending data.  The driver may have more data to send, in which case the data
 * transmit register (or FIFO) is filled for subsequent transmission. When this
 * interrupt arrives and all the data has been sent, the driver invokes the
 * status callback with a value of XST_SPI_TRANSFER_DONE to inform the upper
 * layer software that all data has been sent.
 *
 * The Data Transmit FIFO Half Empty interrupt indicates that the SPI device has
 * transmitted half of the data available, in the FIFO, to transmit. The driver
 * uses this interrupt to indicate progress while sending data.  The driver may
 * have more data to send, in which case the data transmit FIFO is filled for
 * subsequent transmission. This interrupt is particualrly useful in slave mode,
 * while transfering more than FIFO_DEPTH number of bytes. In this case, the
 * driver ensures that the FIFO is never empty during a transfer and avoids
 * master receiving invalid data.
 *
 * The Data Transmit Register/FIFO Underrun interrupt indicates that, as slave,
 * the SPI device was required to transmit but there was no data available to
 * transmit in the transmit register (or FIFO). This may not be an error if the
 * master is not expecting data, but in the case where the master is expecting
 * data this serves as a notification of such a condition. The driver reports
 * this condition to the upper layer software through the status handler.
 *
 * The Data Receive Register/FIFO Overrun interrupt indicates that the SPI device
 * received data and subsequently dropped the data because the data receive
 * register (or FIFO) was full. The interrupt applies to both master and slave
 * operation. The driver reports this condition to the upper layer software
 * through the status handler. This likely indicates a problem with the higher
 * layer protocol, or a problem with the slave performance.
 *
 * The Mode Fault Error interrupt indicates that while configured as a master,
 * the device was selected as a slave by another master. This can be used by the
 * application for arbitration in a multimaster environment or to indicate a
 * problem with arbitration. When this interrupt occurs, the driver invokes the
 * status callback with a status value of XST_SPI_MODE_FAULT. It is up to the
 * application to resolve the conflict.
 *
 * The Slave Mode Fault Error interrupt indicates that a slave device was
 * selected as a slave by a master, but the slave device was disabled.  This can
 * be used during system debugging or by the slave application to learn when the
 * slave application has not prepared for a master operation in a timely fashion.
 * This likely indicates a problem with the higher layer protocol, or a problem
 * with the slave performance.
 *
 * The Slave Mode Select interrupt indicates that the SPI device was selected as
 * a slave by a master. The driver reports this condition to the upper layer
 * software through the status handler.
 *
 * Data Receive FIFO not Empty interrupt indicates that the SPI device, in slave
 * mode, has received a data byte in the Data Receive FIFO, after the master has
 * started a transfer. The driver reports this condition to the upper layer
 * software through the status handler.
 *
 * <b>Polled Operation</b>
 *
 * This driver operates in polled mode operation too. To put the driver in polled
 * mode the Global Interrupt must be disabled after the Spi is Initialized and
 * Spi driver is started.
 *
 * Statistics are not updated in this mode of operation.
 *
 * <b>Device Busy</b>
 *
 * Some operations are disallowed when the device is busy. The driver tracks
 * whether a device is busy. The device is considered busy when a data transfer
 * request is outstanding, and is considered not busy only when that transfer
 * completes (or is aborted with a mode fault error). This applies to both
 * master and slave devices.
 *
 * <b>Device Configuration</b>
 *
 * The device can be configured in various ways during the FPGA implementation
 * process. Configuration parameters are stored in the xspi_g.c file or passed
 * in via _CfgInitialize(). A table is defined where each entry contains
 * configuration information for an SPI device. This information includes such
 * things as the base address of the memory-mapped device, the number of slave
 * select bits in the device, and whether the device has FIFOs and is configured
 * as slave-only.
 *
 * <b>RTOS Independence</b>
 *
 * This driver is intended to be RTOS and processor independent. It works
 * with physical addresses only.  Any needs for dynamic memory management,
 * threads or thread mutual exclusion, virtual memory, or cache control must
 * be satisfied by the layer above this driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00a rpm  10/11/01 First release
 * 1.00b jhl  03/14/02 Repartitioned driver for smaller files.
 * 1.01a jvb  12/14/05 I separated dependency on the static config table and
 *                     xparameters.h from the driver initialization by moving
 *                     _Initialize and _LookupConfig to _sinit.c. I also added
 *                     the new _CfgInitialize routine.
 * 1.11a wgr  03/22/07 Converted to new coding style.
 * 1.11a  sv  02/22/08 Added the definition of LSB-MSB first option in xspi_l.h.
 * 1.12a sdm  03/22/08 Updated the code to support 16/32 bit transfer width and
 *                     polled mode of operation, removed the macros in xspi_l.h,
 *                     added macros in xspi.h file, moved the interrupt
 *                     register/bit definitions from xspi_i.h to xpsi_l.h.
 *                     Even for the polled mode of operation the Interrupt Logic
 *                     in the core should be included. The driver can be put in
 *                     polled mode of operation by disabling the Global Interrupt
 *                     after the Spi Initialization is completed and Spi is
 *                     started.
 * 2.00a sdm  07/30/08 Updated the code to support 16/32 bit transfer width and
 *                     polled mode of operation, removed the macros in xspi_l.h,
 *                     added macros in xspi.h file, moved the interrupt
 *                     register/bit definitions from xspi_i.h to xpsi_l.h.
 *                     Even for the polled mode of operation the Interrupt Logic
 *                     in the core should be included. The driver can be put in
 *                     polled mode of operation by disabling the Global Interrupt
 *                     after the Spi Initialization is completed and Spi is
 *                     started.
 * 2.01a sdm  08/22/08 Removed support for static interrupt handlers from the MDD
 *                     file
 * 2.01b sdm  04/08/09 Fixed an issue in the XSpi_Transfer function where the
 *                     Global Interrupt is being enabled in polled mode when a
 *                     slave is not selected.
 * 3.00a ktn  10/22/09 Converted all register accesses to 32 bit access.
 *                     Updated driver to use the HAL APIs/macros.
 *                     Removed the macro XSpi_mReset, XSpi_Reset API should be
 *                     used in its place.
 *                     The macros have been renamed to remove _m from the name
 *                     XSpi_mIntrGlobalEnable is renamed XSpi_IntrGlobalEnable,
 *                     XSpi_mIntrGlobalDisable is now XSpi_IntrGlobalDisable,
 *                     XSpi_mIsIntrGlobalEnabled is now XSpi_IsIntrGlobalEnabled,
 *                     XSpi_mIntrGetStatus is now XSpi_IntrGetStatus,
 *                     XSpi_mIntrClear is now XSpi_IntrClear,
 *                     XSpi_mIntrEnable is now XSpi_IntrEnable,
 *                     XSpi_mIntrDisable is now XSpi_IntrDisable,
 *                     XSpi_mIntrGetEnabled is now XSpi_IntrGetEnabled,
 *                     XSpi_mSetControlReg is now XSpi_SetControlReg,
 *                     XSpi_mGetControlReg is now XSpi_GetControlReg,
 *                     XSpi_mGetStatusReg is now XSpi_GetStatusReg,
 *                     XSpi_mSetSlaveSelectReg is now XSpi_SetSlaveSelectReg,
 *                     XSpi_mGetSlaveSelectReg is now XSpi_GetSlaveSelectReg,
 *                     XSpi_mEnable is now XSpi_Enable,
 *                     XSpi_mDisable is now XSpi_Disable.
 * 3.01a sdm  04/23/10 Updated the driver to handle new slave mode interrupts
 *                     and the DTR Half Empty interrupt.
 * 3.02a sdm  03/30/11 Updated to support axi_qspi.
 * 3.03a sdm  08/09/11 Updated the selftest to check for a correct default value
 *                     in the case of axi_qspi - CR 620502
 *                     Updated tcl to generate a config parameter for C_SPI_MODE
 * 3.04a bss  03/21/12 Updated XSpi_Config and XSpi instance structure to support
 *                     XIP Mode.
 *                     Updated XSpi_CfgInitialize to support XIP Mode
 *                     Added XIP Mode Register masks in xspi_l.h
 *                     Tcl Script changes:
 *                     Added C_TYPE_OF_AXI4_INTERFACE, C_AXI4_BASEADDR and
 *                     C_XIP_MODE to config structure.
 *                     Modified such that based on C_XIP_MODE and
 *                     C_TYPE_OF_AXI4_INTERFACE parameters C_BASEADDR will
 *                     be updated with C_AXI4_BASEADDR.
 *                     Modified such that C_FIFO_EXIST will be updated based
 *                     on C_FIFO_DEPTH for compatability of the driver with
 *                     Axi Spi.
 * 3.05a adk  18/04/13 Updated the code to avoid unused variable
 *                     warnings when compiling with the -Wextra -Wall flags
 *                     In the file xspi.c. CR:705005.
 * 3.06a adk  07/08/13 Added a dummy read in the CfgInitialize(), if startup
 *                     block is used in the h/w design (CR 721229).
 *
 * </pre>
 *
 ******************************************************************************/

#include "xstatus.h"
#include "xspi_l.h"

#ifndef XSPI_H
#define XSPI_H

NACS_BEGIN_DECLS

/************************** Constant Definitions *****************************/

/** @name Configuration options
 *
 * The following options may be specified or retrieved for the device and
 * enable/disable additional features of the SPI.  Each of the options
 * are bit fields, so more than one may be specified.
 *
 * @{
 */
/**
 * <pre>
 * The Master option configures the SPI device as a master. By default, the
 * device is a slave.
 *
 * The Active Low Clock option configures the device's clock polarity. Setting
 * this option means the clock is active low and the SCK signal idles high. By
 * default, the clock is active high and SCK idles low.
 *
 * The Clock Phase option configures the SPI device for one of two transfer
 * formats.  A clock phase of 0, the default, means data if valid on the first
 * SCK edge (rising or falling) after the slave select (SS) signal has been
 * asserted. A clock phase of 1 means data is valid on the second SCK edge
 * (rising or falling) after SS has been asserted.
 *
 * The Loopback option configures the SPI device for loopback mode.  Data is
 * looped back from the transmitter to the receiver.
 *
 * The Manual Slave Select option, which is default, causes the device not
 * to automatically drive the slave select.  The driver selects the device
 * at the start of a transfer and deselects it at the end of a transfer.
 * If this option is off, then the device automatically toggles the slave
 * select signal between bytes in a transfer.
 * </pre>
 */
#define XSP_MASTER_OPTION 0x1
#define XSP_CLK_ACTIVE_LOW_OPTION 0x2
#define XSP_CLK_PHASE_1_OPTION 0x4
#define XSP_LOOPBACK_OPTION 0x8
#define XSP_MANUAL_SSELECT_OPTION 0x10
/*@}*/

/**************************** Type Definitions *******************************/

/******************************************************************************/
/**
 * The handler data type allows the user to define a callback function to
 * handle the asynchronous processing of the SPI driver.  The application using
 * this driver is expected to define a handler of this type to support interrupt
 * driven mode.  The handler executes in an interrupt context such that minimal
 * processing should be performed.
 *
 * @param CallBackRef A callback reference passed in by the upper layer when
 *                    setting the callback functions, and passed back to the
 *                    upper layer when the callback is invoked. Its type is
 *                    unimportant to the driver component, so it is a void
 *                    pointer.
 * @param StatusEvent Indicates one or more status events that occurred. See
 *                    the XSpi_SetStatusHandler() for details on the status
 *                    events that can be passed in the callback.
 * @param ByteCount   Indicates how many bytes of data were successfully
 *                    transferred.  This may be less than the number of bytes
 *                    requested if the status event indicates an error.
 *
 *******************************************************************************/
typedef void (*XSpi_StatusHandler)(void *CallBackRef, uint32_t StatusEvent,
                                   unsigned int ByteCount);

/**
 * XSpi statistics
 */
typedef struct {
    uint32_t ModeFaults; /**< Number of mode fault errors */
    uint32_t XmitUnderruns; /**< Number of transmit underruns */
    uint32_t RecvOverruns; /**< Number of receive overruns */
    uint32_t SlaveModeFaults; /**< Num of selects as slave while disabled */
    uint32_t BytesTransferred; /**< Number of bytes transferred */
    uint32_t NumInterrupts; /**< Number of transmit/receive interrupts */
} XSpi_Stats;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
    uint16_t DeviceId; /**< Unique ID  of device */
    volatile char *BaseAddress; /**< Base address of the device */
    int HasFifos; /**< Does device have FIFOs? */
    uint32_t SlaveOnly; /**< Is the device slave only? */
    uint8_t NumSlaveBits; /**< Num of slave select bits on the device */
    uint8_t DataWidth;/**< Data transfer Width */
    uint8_t SpiMode; /**< Standard/Dual/Quad mode */
    uint8_t AxiInterface; /**< AXI-Lite/AXI Full Interface */
    uint32_t AxiFullBaseAddress; /**< AXI Full Interface Base address of
                                    the device */
    uint8_t XipMode; /**< 0 if Non-XIP, 1 if XIP Mode */
    uint8_t Use_Startup; /**< 1 if Starup block is used in h/w */
} XSpi_Config;

/**
 * The XSpi driver instance data. The user is required to allocate a
 * variable of this type for every SPI device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
    XSpi_Stats Stats; /**< Statistics */
    volatile char *BaseAddr; /**< Base address of device (IPIF) */
    volatile int IsReady; /**< Device is initialized and ready */
    volatile int IsStarted; /**< Device has been started */
    int HasFifos; /**< Device is configured with FIFOs or not */
    uint32_t SlaveOnly; /**< Device is configured to be slave only */
    uint8_t NumSlaveBits; /**< Number of slave selects for this device */
    uint8_t DataWidth; /**< Data Transfer Width 8 or 16 or 32 */
    uint8_t SpiMode; /**< Standard/Dual/Quad mode */
    uint32_t SlaveSelectMask; /**< Mask that matches the number of SS bits */
    uint32_t SlaveSelectReg; /**< Slave select register */

    uint8_t *SendBufferPtr; /**< Buffer to send  */
    uint8_t *RecvBufferPtr; /**< Buffer to receive */
    unsigned int RequestedBytes; /**< Total bytes to transfer (state) */
    unsigned int RemainingBytes; /**< Bytes left to transfer (state) */
    int IsBusy; /**< A transfer is in progress (state) */

    XSpi_StatusHandler StatusHandler; /**< Status Handler */
    void *StatusRef; /**< Callback reference for status handler */
    uint32_t FlashBaseAddr; /**< Used in XIP Mode */
    uint8_t XipMode; /**< 0 if Non-XIP, 1 if XIP Mode */
} XSpi;

#define XSPI_IS_STARTED 0x22222222 /**< component has been started */
#define XSPI_IS_READY 0x11111111 /**< component has been initialized */

/***************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This macro writes to the global interrupt enable register to enable
 * interrupts from the device.
 *
 * Interrupts enabled using XSpi_IntrEnable() will not occur until the global
 * interrupt enable bit is set by using this function.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 *
 ******************************************************************************/
static NACS_INLINE void
XSpi_IntrGlobalEnable(XSpi *self)
{
    XSpi_WriteReg(self->BaseAddr,  XSP_DGIER_OFFSET, XSP_GINTR_ENABLE_MASK);
}

/******************************************************************************/
/**
 *
 * This macro disables all interrupts for the device by writing to the Global
 * interrupt enable register.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 *
 ******************************************************************************/
static NACS_INLINE void
XSpi_IntrGlobalDisable(XSpi *self)
{
    XSpi_WriteReg(self->BaseAddr,  XSP_DGIER_OFFSET, 0);
}

/*****************************************************************************/
/**
 *
 * This function determines if interrupts are enabled at the global level by
 * reading the global interrupt register.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 *
 * @return
 *		- TRUE if global interrupts are enabled.
 *		- FALSE if global interrupts are disabled.
 *
 ******************************************************************************/
static NACS_INLINE bool
XSpi_IsIntrGlobalEnabled(XSpi *self)
{
    return XSpi_ReadReg(self->BaseAddr,
                        XSP_DGIER_OFFSET) == XSP_GINTR_ENABLE_MASK;
}

/*****************************************************************************/
/**
 *
 * This function gets the contents of the Interrupt Status Register.
 * This register indicates the status of interrupt sources for the device.
 * The status is independent of whether interrupts are enabled such
 * that the status register may also be polled when interrupts are not enabled.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 *
 * @return	A status which contains the value read from the Interrupt
 *		Status Register.
 *
 ******************************************************************************/
static NACS_INLINE uint32_t
XSpi_IntrGetStatus(XSpi *self)
{
    return XSpi_ReadReg(self->BaseAddr, XSP_IISR_OFFSET);
}

/*****************************************************************************/
/**
 *
 * This function clears the specified interrupts in the Interrupt status
 * Register. The interrupt is cleared by writing to this register with the bits
 * to be cleared set to a one and all others bits to zero. Setting a bit which
 * is zero within this register causes an interrupt to be generated.
 *
 * This function writes only the specified value to the register such that
 * some status bits may be set and others cleared.  It is the caller's
 * responsibility to get the value of the register prior to setting the value
 * to prevent an destructive behavior.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 * @param	ClearMask is the Bitmask for interrupts to be cleared.
 *		Bit positions of "1" clears the interrupt. Bit positions of 0
 *		will keep the previous setting. This mask is formed by OR'ing
 *		XSP_INTR_* bits defined in xspi_l.h.
 *
 ******************************************************************************/
static NACS_INLINE void
XSpi_IntrClear(XSpi *self, uint32_t mask)
{
    XSpi_WriteReg(self->BaseAddr,  XSP_IISR_OFFSET,
                  XSpi_IntrGetStatus(self) | mask);
}

/******************************************************************************/
/**
 *
 * This function sets the contents of the Interrupt Enable Register.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 * @param	EnableMask is the bitmask of the interrupts to be enabled.
 *		Bit positions of 1 will be enabled. Bit positions of 0 will
 *		keep the previous setting. This mask is formed by OR'ing
 *		XSP_INTR_* bits defined in xspi_l.h.
 *
 ******************************************************************************/
static NACS_INLINE void
XSpi_IntrEnable(XSpi *self, uint32_t mask)
{
    XSpi_WriteReg(self->BaseAddr, XSP_IIER_OFFSET,
                  (XSpi_ReadReg(self->BaseAddr, XSP_IIER_OFFSET)) |
                  (mask & XSP_INTR_ALL));
}

/****************************************************************************/
/**
 *
 * Disable the specified Interrupts in the Interrupt Enable Register.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 * @param	DisableMask is the bitmask of the interrupts to be disabled.
 *		Bit positions of 1 will be disabled. Bit positions of 0 will
 *		keep the previous setting. This mask is formed by OR'ing
 *		XSP_INTR_* bits defined in xspi_l.h.
 *
 ******************************************************************************/
static NACS_INLINE void
XSpi_IntrDisable(XSpi *self, uint32_t mask)
{
    XSpi_WriteReg(self->BaseAddr, XSP_IIER_OFFSET,
                  XSpi_ReadReg(self->BaseAddr, XSP_IIER_OFFSET) &
                  ~(mask & XSP_INTR_ALL));
}

/*****************************************************************************/
/**
 *
 * This function gets the contents of the Interrupt Enable Register.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 *
 * @return	The contents read from the Interrupt Enable Register.
 *
 ******************************************************************************/
static NACS_INLINE uint32_t
XSpi_IntrGetEnabled(XSpi *self)
{
    return XSpi_ReadReg(self->BaseAddr,  XSP_IIER_OFFSET);
}

/****************************************************************************/
/**
 *
 * Set the contents of the control register. Use the XSP_CR_* constants defined
 * above to create the bit-mask to be written to the register.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 * @param	Mask is the 32-bit value to write to the control register.
 *
 *****************************************************************************/
static NACS_INLINE void
XSpi_SetControlReg(XSpi *self, uint32_t mask)
{
    XSpi_WriteReg(self->BaseAddr, XSP_CR_OFFSET, mask);
}

/****************************************************************************/
/**
 *
 * Get the contents of the control register. Use the XSP_CR_* constants defined
 * above to interpret the bit-mask returned.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 *
 * @return	A 32-bit value representing the contents of the control
 *		register.
 *
 *****************************************************************************/
static NACS_INLINE uint32_t
XSpi_GetControlReg(XSpi *self)
{
    return XSpi_ReadReg(self->BaseAddr, XSP_CR_OFFSET);
}

/***************************************************************************/
/**
 *
 * Get the contents of the status register. Use the XSP_SR_* constants defined
 * above to interpret the bit-mask returned.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 *
 * @return	An 32-bit value representing the contents of the status
 *		register.
 *
 *****************************************************************************/
static NACS_INLINE uint32_t
XSpi_GetStatusReg(XSpi *self)
{
    return XSpi_ReadReg(self->BaseAddr, XSP_SR_OFFSET);
}

/****************************************************************************/
/**
 *
 * Set the contents of the XIP control register. Use the XSP_CR_XIP_* constants
 * defined above to create the bit-mask to be written to the register.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 * @param	Mask is the 32-bit value to write to the control register.
 *
 *****************************************************************************/
static NACS_INLINE void
XSpi_SetXipControlReg(XSpi *self, uint32_t mask)
{
    XSpi_WriteReg(self->BaseAddr, XSP_CR_OFFSET, mask);
}

/****************************************************************************/
/**
 *
 * Get the contents of the XIP control register. Use the XSP_CR_XIP_* constants
 * defined above to interpret the bit-mask returned.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 *
 * @return	A 32-bit value representing the contents of the control
 *		register.
 *
 *****************************************************************************/
static NACS_INLINE uint32_t
XSpi_GetXipControlReg(XSpi *self)
{
    return XSpi_ReadReg(self->BaseAddr, XSP_CR_OFFSET);
}

/****************************************************************************/
/**
 *
 * Get the contents of the status register. Use the XSP_SR_XIP_* constants
 * defined above to interpret the bit-mask returned.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 *
 * @return	An 32-bit value representing the contents of the status
 *		register.
 *
 *****************************************************************************/
static NACS_INLINE uint32_t
XSpi_GetXipStatusReg(XSpi *self)
{
    return XSpi_ReadReg(self->BaseAddr, XSP_SR_OFFSET);
}

/****************************************************************************/
/**
 *
 * Set the contents of the slave select register. Each bit in the mask
 * corresponds to a slave select line. Only one slave should be selected at
 * any one time.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 * @param	Mask is the 32-bit value to write to the slave select register.
 *
 *****************************************************************************/
static NACS_INLINE void
XSpi_SetSlaveSelectReg(XSpi *self, uint32_t mask)
{
    XSpi_WriteReg(self->BaseAddr, XSP_SSR_OFFSET, mask);
}

/****************************************************************************/
/**
 *
 * Get the contents of the slave select register. Each bit in the mask
 * corresponds to a slave select line. Only one slave should be selected at
 * any one time.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 *
 * @return	The 32-bit value in the slave select register.
 *
 *****************************************************************************/
static NACS_INLINE uint32_t
XSpi_GetSlaveSelectReg(XSpi *self)
{
    return XSpi_ReadReg(self->BaseAddr, XSP_SSR_OFFSET);
}

/****************************************************************************/
/**
 *
 * Enable the device and uninhibit master transactions. Preserves the current
 * contents of the control register.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 *
 *****************************************************************************/
static NACS_INLINE void
XSpi_Enable(XSpi *self)
{
    uint16_t ctrl;
    ctrl = XSpi_GetControlReg(self);
    ctrl |= XSP_CR_ENABLE_MASK;
    ctrl &= ~XSP_CR_TRANS_INHIBIT_MASK;
    XSpi_SetControlReg(self, ctrl);
}

/****************************************************************************/
/**
 *
 * Disable the device. Preserves the current contents of the control register.
 *
 * @param	InstancePtr is a pointer to the XSpi instance to be worked on.
 *
 * @note        C-Style signature:
 *              void XSpi_Disable(XSpi *InstancePtr);
 *
 *****************************************************************************/
static NACS_INLINE void
XSpi_Disable(XSpi *self)
{
    XSpi_SetControlReg(self, XSpi_GetControlReg(self) & ~XSP_CR_ENABLE_MASK);
}

/************************** Function Prototypes ******************************/

/*
 * Initialization functions in xspi_sinit.c
 */
int XSpi_Initialize(XSpi *InstancePtr, uint16_t DeviceId);
// XSpi_Config *XSpi_LookupConfig(uint16_t DeviceId);

/*
 * Functions, in xspi.c
 */
// Internal
int XSpi_CfgInitialize(XSpi *InstancePtr, XSpi_Config * Config,
                       volatile char *EffectiveAddr);

int XSpi_Start(XSpi *InstancePtr);
int XSpi_Stop(XSpi *InstancePtr);

// Internal
void XSpi_Reset(XSpi *InstancePtr);

// int XSpi_SetSlaveSelect(XSpi *InstancePtr, uint32_t SlaveMask);
// uint32_t XSpi_GetSlaveSelect(XSpi *InstancePtr);

int XSpi_Transfer(XSpi *InstancePtr, uint8_t *SendBufPtr, uint8_t *RecvBufPtr,
                  unsigned int ByteCount);

// void XSpi_SetStatusHandler(XSpi *InstancePtr, void *CallBackRef,
//                            XSpi_StatusHandler FuncPtr);
// void XSpi_InterruptHandler(void *InstancePtr);


/*
 * Functions for selftest, in xspi_selftest.c
 */
// int XSpi_SelfTest(XSpi *InstancePtr);

/*
 * Functions for statistics, in xspi_stats.c
 */
// void XSpi_GetStats(XSpi *InstancePtr, XSpi_Stats *StatsPtr);
// void XSpi_ClearStats(XSpi *InstancePtr);

/*
 * Functions for options, in xspi_options.c
 */
int XSpi_SetOptions(XSpi *InstancePtr, uint32_t Options);
// uint32_t XSpi_GetOptions(XSpi *InstancePtr);

NACS_END_DECLS
#endif
