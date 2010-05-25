/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/


/** @addtogroup General General
 *  @{ */

/** @file Std_Types.h
 *  Definitions of General types.
 */

#ifndef _STD_TYPES_H
#define _STD_TYPES_H

// Autosar include files....
// TODO: we haven't really defined the autosar types yet.
//       the standard types are uint8, etc.

#include <stdint.h>
#include "Platform_Types.h" // TODO: move
#include "Compiler.h"

#ifndef 	NULL
#define	NULL	0
#endif

//typedef uint8_t uint8;
//typedef uint16_t uint16;
//typedef uint32_t uint32;


typedef struct {
	// TODO: not done!!
	uint16 vendorID;
	uint16 moduleID;
	uint8  instanceID;

	uint8 sw_major_version;    /**< Vendor numbers */
	uint8 sw_minor_version;    /**< Vendor numbers */
	uint8 sw_patch_version;    /**< Vendor numbers */

	uint8 ar_major_version;    /**< Autosar spec. numbers */
	uint8 ar_minor_version;    /**< Autosar spec. numbers */
	uint8 ar_patch_version;    /**< Autosar spec. numbers */
} Std_VersionInfoType;

/** make compare number... #if version > 10203  ( 1.2.3 ) */
#define STD_GET_VERSION (_major,_minor,_patch) (_major * 10000 + _minor * 100 + _patch)

/** Create Std_VersionInfoType */
#define STD_GET_VERSION_INFO(_vi,_module) \
	((_vi)->vendorID =  _module ## _VENDOR_ID);\
	((_vi)->moduleID = _module ## _MODULE_ID);\
	((_vi)->sw_major_version = _module ## _SW_MAJOR_VERSION);\
	((_vi)->sw_minor_version =  _module ## _SW_MINOR_VERSION);\
	((_vi)->sw_patch_version =  _module ## _SW_PATCH_VERSION);\
	((_vi)->ar_major_version =  _module ## _AR_MAJOR_VERSION);\
	((_vi)->ar_minor_version =  _module ## _AR_MINOR_VERSION);\
	((_vi)->ar_patch_version =  _module ## _AR_PATCH_VERSION);


// TODO: Move to OSEK implementation, See 8.2 in SWS_StandardTypes

#define STATUSTYPEDEFINED
typedef enum {
	E_OK = 0,
	E_OS_ACCESS = 1,               /**< STD OSEK */
	E_OS_CALLEVEL = 2,             /**< STD OSEK */
	E_OS_ID	= 3,                   /**< STD OSEK */
	E_OS_LIMIT = 4,                /**< STD OSEK */
	E_OS_NOFUNC = 5,               /**< STD OSEK */
	E_OS_RESOURCE = 6,             /**< STD OSEK */
	E_OS_STATE = 7,                /**< STD OSEK */
	E_OS_VALUE = 8,                /**< STD OSEK */

	E_OS_SERVICEID,                /**< AUTOSAR, see 7.10 */
	E_OS_RATE ,                    /**< AUTOSAR, see 7.10 */
	E_OS_ILLEGAL_ADDRESS ,         /**< AUTOSAR, see 7.10 */
	E_OS_MISSINGEND ,              /**< AUTOSAR, see 7.10 */
	E_OS_DISABLEDINT ,             /**< AUTOSAR, see 7.10 */
	E_OS_STACKFAULT ,              /**< AUTOSAR, see 7.10 */
	E_OS_PROTECTION_MEMORY ,       /**< AUTOSAR, see 7.10 */
	E_OS_PROTECTION_TIME ,         /**< AUTOSAR, see 7.10 */
	E_OS_PROTECTION_LOCKED ,       /**< AUTOSAR, see 7.10 */
	E_OS_PROTECTION_EXCEPTION ,    /**< AUTOSAR, see 7.10 */
	E_OS_PROTECTION_RATE,          /**< AUTOSAR, see 7.10 */

	/* COM.. TODO: move ?? */
	E_COM_ID,


	/** Implementation specific */
	E_OS_SYS_APA,

	E_NOT_OK,
} StatusType;

typedef uint8 Std_ReturnType;


#ifndef STATUSTYPEDEFINED
#define STATUSTYPEDEFINED
#define E_OK 			0
typedef unsigned char StatusType;
#endif

#define E_NOT_OK				1

#define E_NO_DTC_AVAILABLE		2
#define E_SESSION_NOT_ALLOWED	4
#define E_PROTOCOL_NOT_ALLOWED	5
#define E_REQUEST_NOT_ACCEPTED	8
#define E_REQUEST_ENV_NOK		9
#define E_PENDING				10
#define E_COMPARE_KEY_FAILED	11
#define E_FORCE_RCRRP			12

#define STD_HIGH		0x01
#define STD_LOW			0x00

#define STD_ACTIVE		0x01
#define STD_IDLE		0x00

#define STD_ON			0x01
#define STD_OFF			0x00


#endif
/** @} */
