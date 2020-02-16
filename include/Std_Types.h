/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/


/** @addtogroup General General
 *  @{ */

/** @file Std_Types.h
 *  Definitions of General types.
 */

#ifndef STD_TYPES_H
#define STD_TYPES_H

// Autosar include files....
// TODO: we haven't really defined the autosar types yet.
//       the standard types are uint8, etc.

#include "Platform_Types.h" // TODO: move
#include "Compiler.h"

#ifndef 	NULL
//lint -esym(960,20.2) // PC-Lint LINT EXCEPTION
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
// PC-Lint Exception MISRA rule 19.12
//lint -save -esym(960,19.12)
#define STD_GET_VERSION_INFO(_vi,_module) \
	((_vi)->vendorID =  _module ## _VENDOR_ID);\
	((_vi)->moduleID = _module ## _MODULE_ID);\
	((_vi)->sw_major_version = _module ## _SW_MAJOR_VERSION);\
	((_vi)->sw_minor_version =  _module ## _SW_MINOR_VERSION);\
	((_vi)->sw_patch_version =  _module ## _SW_PATCH_VERSION);\
	((_vi)->ar_major_version =  _module ## _AR_MAJOR_VERSION);\
	((_vi)->ar_minor_version =  _module ## _AR_MINOR_VERSION);\
	((_vi)->ar_patch_version =  _module ## _AR_PATCH_VERSION);\
//lint -restore

#ifndef MIN
#define MIN(_x,_y) (((_x) < (_y)) ? (_x) : (_y))
#endif
#ifndef MAX
#define MAX(_x,_y) (((_x) > (_y)) ? (_x) : (_y))
#endif


typedef uint8 Std_ReturnType;

#define E_OK 					(Std_ReturnType)0
#define E_NOT_OK 				(Std_ReturnType)1

#define E_NO_DTC_AVAILABLE		(Std_ReturnType)2
#define E_SESSION_NOT_ALLOWED	(Std_ReturnType)4
#define E_PROTOCOL_NOT_ALLOWED	(Std_ReturnType)5
#define E_REQUEST_NOT_ACCEPTED	(Std_ReturnType)8
#define E_REQUEST_ENV_NOK		(Std_ReturnType)9
#define E_PENDING				(Std_ReturnType)10
#define E_COMPARE_KEY_FAILED	(Std_ReturnType)11
#define E_FORCE_RCRRP			(Std_ReturnType)12

#define STD_HIGH		0x01
#define STD_LOW			0x00

#define STD_ACTIVE		0x01
#define STD_IDLE		0x00

#define STD_ON			0x01
#define STD_OFF			0x00


#endif
/** @} */
