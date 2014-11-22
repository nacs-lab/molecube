/* $Id: xbasic_types.h,v 1.19.10.4 2011/06/28 11:00:54 sadanan Exp $ */
/******************************************************************************
*
*       XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
*       SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
*       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
*       APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
*       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*       FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
*       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*       FOR A PARTICULAR PURPOSE.
*
*       (c) Copyright 2002-2007 Xilinx Inc.
*       All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xbasic_types.h
*
* This file contains basic types for Xilinx software IP.  These types do not
* follow the standard naming convention with respect to using the component
* name in front of each name because they are considered to be primitives.
*
* @note
*
* This file contains items which are architecture dependent.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date   Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a rmm  12/14/01 First release
*       rmm  05/09/03 Added "xassert always" macros to rid ourselves of diab
*                     compiler warnings
* 1.00a rpm  11/07/03 Added XNullHandler function as a stub interrupt handler
* 1.00a rpm  07/21/04 Added XExceptionHandler typedef for processor exceptions
* 1.00a xd   11/03/04 Improved support for doxygen.
* 1.00a wre  01/25/07 Added Linux style data types u32, u16, u8, TRUE, FALSE
* 1.00a rpm  04/02/07 Added ifndef KERNEL around u32, u16, u8 data types
* </pre>
*
******************************************************************************/

#ifndef XBASIC_TYPES_H	/* prevent circular inclusions */
#define XBASIC_TYPES_H	/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#include <stddef.h>

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

#define XCOMPONENT_IS_READY     0x11111111  /**< component has been initialized */
#define XCOMPONENT_IS_STARTED   0x22222222  /**< component has been started */

/* the following constants and declarations are for unit test purposes and are
 * designed to be used in test applications.
 */
#define XTEST_PASSED    0
#define XTEST_FAILED    1

#define XASSERT_NONE     0
#define XASSERT_OCCURRED 1

extern unsigned int XAssertStatus;
void XAssert(char*, int);

/**************************** Type Definitions *******************************/

/** @name Legacy types
 * Deprecated legacy types.
 * @{
 */
typedef unsigned char	Xuint8;		/**< unsigned 8-bit */
typedef char		Xint8;		/**< signed 8-bit */
typedef unsigned short	Xuint16;	/**< unsigned 16-bit */
typedef short		Xint16;		/**< signed 16-bit */
typedef unsigned long	Xuint32;	/**< unsigned 32-bit */
typedef long		Xint32;		/**< signed 32-bit */
typedef float		Xfloat32;	/**< 32-bit floating point */
typedef double		Xfloat64;	/**< 64-bit double precision FP */
typedef unsigned long	Xboolean;	/**< boolean (XTRUE or XFALSE) */

/** @name New types
 * New simple types.
 * @{
 */
#ifndef __KERNEL__
#ifndef XIL_TYPES_H
typedef Xuint32         u32;
typedef Xuint16         u16;
typedef Xuint8          u8;
#endif
#else
#include <linux/types.h>
#endif

/*@}*/

#ifdef __cplusplus
}
#endif

#endif	/* end of protection macro */
