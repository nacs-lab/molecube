/* $Id: xspi_sinit.c,v 1.1.2.1 2011/08/09 06:59:27 svemula Exp $ */
/******************************************************************************
*
* (c) Copyright 2005-2013 Xilinx, Inc. All rights reserved.
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
* @file xspi_sinit.c
*
* The implementation of the XSpi component's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.01a jvb  10/13/05 First release
* 1.11a wgr  03/22/07 Converted to new coding style.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <nacs-utils/log.h>
#include <nacs-utils/fd_utils.h>
#include "xparameters.h"
#include "xspi.h"
#include "xspi_i.h"
#include <assert.h>

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param	DeviceId contains the ID of the device to look up the
*		configuration for.
*
* @return
*
* A pointer to the configuration found or NULL if the specified device ID was
* not found. See xspi.h for the definition of XSpi_Config.
*
* @note		None.
*
******************************************************************************/
XSpi_Config*
XSpi_LookupConfig(uint16_t DeviceId)
{
    XSpi_Config *CfgPtr = NULL;

    for (unsigned i = 0;i < XPAR_XSPI_NUM_INSTANCES;i++) {
        if (XSpi_ConfigTable[i].DeviceId == DeviceId) {
            CfgPtr = &XSpi_ConfigTable[i];
            break;
        }
    }
    //remap physical address to virtual one
    CfgPtr->BaseAddress = nacsMapFile("/dev/mem",
                                      (intptr_t)CfgPtr->BaseAddress, 4096);
    if (nacsUnlikely(!CfgPtr->BaseAddress)) {
        nacsError("Can't map the memory to user space.\n");
        exit(0);
    }
    return CfgPtr;
}

/*****************************************************************************/
/**
*
* Initializes a specific XSpi instance such that the driver is ready to use.
*
* The state of the device after initialization is:
*	- Device is disabled
*	- Slave mode
*	- Active high clock polarity
*	- Clock phase 0
*
* @param	InstancePtr is a pointer to the XSpi instance to be worked on.
* @param	DeviceId is the unique id of the device controlled by this XSpi
*		instance. Passing in a device id associates the generic XSpi
*		instance to a specific device, as chosen by the caller or
*		application developer.
*
* @return
*
*		- XST_SUCCESS if successful.
*		- XST_DEVICE_IS_STARTED if the device is started. It must be
*		  stopped to re-initialize.
*		- XST_DEVICE_NOT_FOUND if the device was not found in the
*		  configuration such that initialization could not be
*		  accomplished.
*
* @note		None.
*
******************************************************************************/
int XSpi_Initialize(XSpi *InstancePtr, uint16_t DeviceId)
{
    XSpi_Config *ConfigPtr; /* Pointer to Configuration ROM data */

    assert(InstancePtr != NULL);

    /*
     * Lookup the device configuration in the temporary CROM table. Use this
     * configuration info down below when initializing this component.
     */
    ConfigPtr = XSpi_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        return XST_DEVICE_NOT_FOUND;
    }

    return XSpi_CfgInitialize(InstancePtr, ConfigPtr,
                              ConfigPtr->BaseAddress);
}
