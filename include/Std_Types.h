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









#ifndef _STD_TYPES_H
#define _STD_TYPES_H

// Autosar include files....
// TODO: we haven't really defined the autosar types yet.
//       the standard types are uint8, etc.

#ifndef NULL
#define	NULL	0
#endif

#if !defined(USE_KERNEL)
#include "typedefs.h"
#else
#include <stdint.h>
#endif

#include "Platform_Types.h" // TODO: move
#include "Compiler.h"

//typedef uint8_t uint8;
//typedef uint16_t uint16;
//typedef uint32_t uint32;


typedef struct {
	// TODO: not done!!
	uint16 vendorID;
	uint16 moduleID;
	uint8  instanceID;
	/* Vendor numbers */
	uint8 sw_major_version;
	uint8 sw_minor_version;
	uint8 sw_patch_version;
	/* Autosar spec. numbers */
	uint8 ar_major_version;
	uint8 ar_minor_version;
	uint8 ar_patch_version;
} Std_VersionInfoType;

/* make compare number... #if blabla > 10203  ( 1.2.3 ) */
#define STD_GET_VERSION (_major,_minor,_patch) (_major * 10000 + _minor * 100 + _patch)

/* Non-standard macro */
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
	/* STD OSEK */
	E_OS_ACCESS = 1,
	E_OS_CALLEVEL = 2,
	E_OS_ID	= 3,
	E_OS_LIMIT = 4,
	E_OS_NOFUNC = 5,
	E_OS_RESOURCE = 6,
	E_OS_STATE = 7,
	E_OS_VALUE = 8,

	/* AUTOSAR, see 7.10 */
	E_OS_SERVICEID,
	E_OS_RATE ,
	E_OS_ILLEGAL_ADDRESS ,
	E_OS_MISSINGEND ,
	E_OS_DISABLEDINT ,
	E_OS_STACKFAULT ,
	E_OS_PROTECTION_MEMORY ,
	E_OS_PROTECTION_TIME ,
	E_OS_PROTECTION_LOCKED ,
	E_OS_PROTECTION_EXCEPTION ,
	E_OS_PROTECTION_RATE,

	/* COM.. TODO: move ?? */
	E_COM_ID,


	/* Implementation specific */
	E_OS_SYS_APA,

	E_NOT_OK,
} StatusType;


// TODO: really ???
typedef uint8 Std_ReturnType;


#ifndef STATUSTYPEDEFINED
#define STATUSTYPEDEFINED
#define E_OK 			0
typedef unsigned char StatusType;
#endif

#define E_NOT_OK				1

#define E_SESSION_NOT_ALLOWED	4
#define E_PROTOCOL_NOT_ALLOWED	5
#define E_REQUEST_NOT_ACCEPTED	8
#define E_REQUEST_ENV_NOK		9
#define E_PENDING				10
#define E_COMPARE_KEY_FAILED	11
#define E_FORCE_RCRRP			12

#define STD_HIGH		0x01
#define STD_LOW		0x00

#define STD_ACTIVE		0x01
#define STD_IDLE		0x00

#define STD_ON			0x01
#define STD_OFF		0x00


#endif
