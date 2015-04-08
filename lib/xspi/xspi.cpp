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
 * @file xspi.c
 *
 * Contains required functions of the XSpi driver component.  See xspi.h for
 * a detailed description of the device and driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00a rpm  10/11/01 First release
 * 1.00b jhl  03/14/02 Repartitioned driver for smaller files.
 * 1.00b rpm  04/25/02 Collapsed IPIF and reg base addresses into one
 * 1.00b rmm  05/14/03 Fixed diab compiler warnings relating to asserts
 * 1.01a jvb  12/13/05 Changed Initialize() into CfgInitialize(), and made
 *                     CfgInitialize() take a pointer to a config structure
 *                     instead of a device id. Moved Initialize() into
 *                     xspi_sinit.c, and had Initialize() call CfgInitialize()
 *                     after it retrieved the config structure using the device
 *                     id. Removed include of xparameters.h along with any
 *                     dependencies on xparameters.h and the _g.c config table.
 * 1.11a wgr  03/22/07 Converted to new coding style.
 * 1.11a rpm  01/22/08 Updated comment on Transfer regarding needing interrupts.
 * 1.12a sdm  03/27/08 Updated the code to support 16/32 bit transfer width and
 *                     polled mode of operation. Even for the polled mode of
 *                     operation the Interrupt Logic in the core should be
 *                     included. The driver can be put in polled mode of
 *                     operation by disabling the Global Interrupt after the
 *                     Spi Initialization is completed.
 * 2.00a sdm  07/30/08 Updated the code to support 16/32 bit transfer width and
 *                     polled mode of operation. Even for the polled mode of
 *                     operation the Interrupt Logic in the core should be
 *                     included. The driver can be put in polled mode of
 *                     operation by disabling the Global Interrupt after the
 *                     Spi Initialization is completed.
 * 2.01b sdm  04/08/09 Fixed an issue in the XSpi_Transfer function where the
 *                     Global Interrupt is being enabled in polled mode when a
 *                     slave is not selected.
 * 3.00a ktn  10/28/09 Updated all the register accesses as 32 bit access.
 *                     Updated to use the HAL APIs/macros.
 *                     Removed the macro XSpi_mReset, XSpi_Reset API should be
 *                     used in its place.
 *                     The macros have been renamed to remove _m from the name.
 *                     Removed an unnecessary read to the core register in the
 *                     XSpi_GetSlaveSelect API.
 * 3.01a sdm  04/23/10 Updated the driver to handle new slave mode interrupts
 *                     and the DTR Half Empty interrupt.
 * 3.04a bss  03/21/12 Updated XSpi_CfgInitialize to support XIP Mode
 * 3.05a adk  18/04/13 Updated the code to avoid unused variable
 *                     warnings when compiling with the -Wextra -Wall flags
 *                     In the file xspi.c. CR:705005.
 * 3.06a adk  07/08/13 Added a dummy read in the CfgInitialize(), if startup
 *                     block is used in the h/w design (CR 721229).
 * </pre>
 *
 ******************************************************************************/

#include "xspi.h"
#include "xspi_i.h"

/**
 *
 * Initializes a specific XSpi instance such that the driver is ready to use.
 *
 * The state of the device after initialization is:
 *        - Device is disabled
 *        - Slave mode
 *        - Active high clock polarity
 *        - Clock phase 0
 *
 * @param        InstancePtr is a pointer to the XSpi instance to be worked on.
 * @param        Config is a reference to a structure containing information
 *                about a specific SPI device. This function initializes an
 *                InstancePtr object for a specific device specified by the
 *                contents of Config. This function can initialize multiple
 *                instance objects with the use of multiple calls giving
 different Config information on each call.
 * @param        EffectiveAddr is the device base address in the virtual memory
 *                address space. The caller is responsible for keeping the
 *                address mapping from EffectiveAddr to the device physical base
 *                address unchanged once this function is invoked. Unexpected
 *                errors may occur if the address mapping changes after this
 *                function is called. If address translation is not used, use
 *                Config->BaseAddress for this parameters, passing the physical
 *                address instead.
 *
 * @return
 *                - XST_SUCCESS if successful.
 *                - XST_DEVICE_IS_STARTED if the device is started. It must be
 *                stopped to re-initialize.
 *
 * @note                None.
 *
 ******************************************************************************/
int
XSpi_CfgInitialize(XSpi *InstancePtr, XSpi_Config *Config,
                   volatile char *EffectiveAddr)
{
    InstancePtr->BaseAddr = EffectiveAddr;

    /*
     * Create a slave select mask based on the number of bits that can
     * be used to deselect all slaves, initialize the value to put into
     * the slave select register to this value.
     */
    InstancePtr->SlaveSelectMask = (1u << Config->NumSlaveBits) - 1;

    /*
     * Reset the SPI device to get it into its initial state. It is expected
     * that device configuration will take place after this initialization
     * is done, but before the device is started.
     */
    XSpi_Reset(InstancePtr);

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function enables interrupts for the SPI device. If the Spi driver is used
 * in interrupt mode, it is up to the user to connect the SPI interrupt handler
 * to the interrupt controller before this function is called. If the Spi driver
 * is used in polled mode the user has to disable the Global Interrupts after
 * this function is called. If the device is configured with FIFOs, the FIFOs are
 * reset at this time.
 *
 * @param        InstancePtr is a pointer to the XSpi instance to be worked on.
 *
 * @return
 *                - XST_SUCCESS if the device is successfully started
 *                - XST_DEVICE_IS_STARTED if the device was already started.
 *
 * @note                None.
 *
 ******************************************************************************/
NACS_EXPORT int
XSpi_Start(XSpi *InstancePtr)
{
    /*
     * Enable the interrupts.
     */
    XSpi_IntrEnable(InstancePtr, XSP_INTR_DFT_MASK);

    /*
     * Reset the transmit and receive FIFOs if present. There is a critical
     * section here since this register is also modified during interrupt
     * context. So we wait until after the r/m/w of the control register to
     * enable the Global Interrupt Enable.
     */
    uint32_t ControlReg = XSpi_GetControlReg(InstancePtr);
    ControlReg |= (XSP_CR_TXFIFO_RESET_MASK | XSP_CR_RXFIFO_RESET_MASK |
                   XSP_CR_ENABLE_MASK);
    XSpi_SetControlReg(InstancePtr, ControlReg);

    /*
     * Enable the Global Interrupt Enable just after we start.
     */
    XSpi_IntrGlobalEnable(InstancePtr);

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * Resets the SPI device by writing to the Software Reset register. Reset must
 * only be called after the driver has been initialized. The configuration of the
 * device after reset is the same as its configuration after initialization.
 * Refer to the XSpi_Initialize function for more details. This is a hard reset
 * of the device. Any data transfer that is in progress is aborted.
 *
 * The upper layer software is responsible for re-configuring (if necessary)
 * and restarting the SPI device after the reset.
 *
 * @param        InstancePtr is a pointer to the XSpi instance to be worked on.
 *
 * @return        None.
 *
 * @note                None.
 *
 ******************************************************************************/
void XSpi_Reset(XSpi *InstancePtr)
{
    /*
     * Abort any transfer that is in progress.
     */
    XSpi_Abort(InstancePtr);

    /*
     * Reset the device.
     */
    XSpi_WriteReg(InstancePtr->BaseAddr, XSP_SRR_OFFSET,
                  XSP_SRR_RESET_MASK);
}

/*****************************************************************************/
/**
 *
 * Aborts a transfer in progress by setting the stop bit in the control register,
 * then resetting the FIFOs if present. The byte counts are cleared and the
 * busy flag is set to false.
 *
 * @param        InstancePtr is a pointer to the XSpi instance to be worked on.
 *
 * @return        None.
 *
 * @note
 *
 * This function does a read/modify/write of the control register. The user of
 * this function needs to take care of critical sections.
 *
 ******************************************************************************/
void XSpi_Abort(XSpi *InstancePtr)
{
    /*
     * Deselect the slave on the SPI bus to abort a transfer, this must be
     * done before the device is disabled such that the signals which are
     * driven by the device are changed without the device enabled.
     */
    XSpi_SetSlaveSelectReg(InstancePtr,
                           InstancePtr->SlaveSelectMask);
    /*
     * Abort the operation currently in progress. Clear the mode
     * fault condition by reading the status register (done) then
     * writing the control register.
     */
    uint32_t ControlReg = XSpi_GetControlReg(InstancePtr);
    /*
     * Stop any transmit in progress and reset the FIFOs if they exist,
     * don't disable the device just inhibit any data from being sent.
     */
    ControlReg |= XSP_CR_TRANS_INHIBIT_MASK;
    XSpi_SetControlReg(InstancePtr, ControlReg);
}
