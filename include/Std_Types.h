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


/** @file Std_Types.h
 *  Definitions of General types.
 */

#ifndef STD_TYPES_H
#define STD_TYPES_H

#include "Platform_Types.h"
#include "Compiler.h"

#ifndef 	NULL
//lint -esym(960,20.2) // PC-Lint LINT EXCEPTION
#define	NULL	0
#endif


typedef struct {
    uint16 vendorID;
    uint16 moduleID;

    uint8 sw_major_version;    /**< Vendor numbers */
    uint8 sw_minor_version;    /**< Vendor numbers */
    uint8 sw_patch_version;    /**< Vendor numbers */
} Std_VersionInfoType;

/** make compare number... #if version > 10203  ( 1.2.3 ) */
#define STD_GET_VERSION(_major,_minor,_patch) (_major * 10000 + _minor * 100 + _patch)

/** Create Std_VersionInfoType */
// PC-Lint Exception MISRA rule 19.12
//lint -save -esym(960,19.12)
#define STD_GET_VERSION_INFO(_vi,_module) \
    ((_vi)->vendorID =  _module ## _VENDOR_ID);\
    ((_vi)->moduleID =  _module ## _MODULE_ID);\
    ((_vi)->sw_major_version =  _module ## _SW_MAJOR_VERSION);\
    ((_vi)->sw_minor_version =  _module ## _SW_MINOR_VERSION);\
    ((_vi)->sw_patch_version =  _module ## _SW_PATCH_VERSION);\

//lint -restore

#ifndef MIN
#define MIN(_x,_y) (((_x) < (_y)) ? (_x) : (_y))
#endif
#ifndef MAX
#define MAX(_x,_y) (((_x) > (_y)) ? (_x) : (_y))
#endif


typedef uint8 Std_ReturnType;

#define E_OK 					0u
#define E_NOT_OK 				(Std_ReturnType)1u

#define E_NO_DTC_AVAILABLE		(Std_ReturnType)2u
#define E_SESSION_NOT_ALLOWED	(Std_ReturnType)4u
#define E_PROTOCOL_NOT_ALLOWED	(Std_ReturnType)5u
#define E_REQUEST_NOT_ACCEPTED	(Std_ReturnType)8u
#define E_REQUEST_ENV_NOK		(Std_ReturnType)9u
#define E_PENDING				(Std_ReturnType)10u
#define E_COMPARE_KEY_FAILED	(Std_ReturnType)11u
#define E_FORCE_RCRRP			(Std_ReturnType)12u

#define STD_HIGH		0x01u
#define STD_LOW			0x00u

#define STD_ACTIVE		0x01u
#define STD_IDLE		0x00u

#define STD_ON			0x01u
#define STD_OFF			0x00u


#endif
