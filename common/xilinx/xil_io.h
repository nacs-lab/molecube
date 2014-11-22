/******************************************************************************
*
* (c) Copyright 2009-13  Xilinx, Inc. All rights reserved.
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
* @file xil_io.h
*
* This file contains the interface for the general IO component, which
* encapsulates the Input/Output functions for processors that do not
* require any special I/O handling.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 1.00a ecm/sdm  10/24/09 First release
* 1.00a sdm      07/21/10 Added Xil_Htonl/s, Xil_Ntohl/s
* 3.07a asa      08/31/12 Added xil_printf.h include
* 3.08a sgd      11/05/12 Reverted SYNC macros definitions
* </pre>
******************************************************************************/

#include <nacs-utils/utils.h>
#include "xil_types.h"

#ifndef XIL_IO_H
#define XIL_IO_H

NACS_BEGIN_DECLS

/* The following functions allow the software to be transportable across
 * processors which may use memory mapped I/O or I/O which is mapped into a
 * seperate address space.
 */
#ifndef Xil_Out32
#define Xil_Out8(addr, data) *((volatile unsigned char*)(addr)) = data
#define Xil_In8(addr) *((volatile unsigned char*)(addr))
#endif

#ifndef Xil_Out16
#define Xil_Out16(addr, data) *((volatile unsigned short*)(addr)) = data
#define Xil_In16(addr) *((volatile unsigned short*)(addr))

#define Xil_Out32(addr, data) *((volatile unsigned *)(addr)) = data
#define Xil_In32(addr) *((volatile unsigned*)(addr))
#endif

NACS_END_DECLS

#endif
