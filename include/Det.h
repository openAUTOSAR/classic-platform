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

/*
 * Development Error Tracer driver
 *
 */

/*
 *  General requirements
 */
/* @req 4.0.3/DET004 */
/* @req 4.0.3/DET037 */
/* @req 4.0.3/DET029 */
/* @req 4.0.3/DET031 */

#ifndef DET_H
#define DET_H

#include "Std_Types.h" /* @req 4.0.3/DET102 */
#include "Modules.h"

/* @req 4.0.3/DET208 */

#define DET_MODULE_ID            MODULE_ID_DET
#define DET_VENDOR_ID            VENDOR_ID_ARCCORE

/* Implementation version */
#define DET_SW_MAJOR_VERSION     1
#define DET_SW_MINOR_VERSION     0
#define DET_SW_PATCH_VERSION     0

/* AUTOSAR specification document version */
#define DET_AR_MAJOR_VERSION                4
#define DET_AR_MINOR_VERSION                0
#define DET_AR_RELEASE_REVISION_VERSION     3


/* Type needed by config file. */
typedef void (*detCbk_t)( uint16 ModuleId, uint8 InstanceId , uint8 ApiId, uint8 ErrorId);

#include "Det_Cfg.h"


// Error codes
#define DET_E_PARAM_POINTER           0x01
#define DET_E_CBK_REGISTRATION_FAILED 0x02
#define DET_E_INDEX_OUT_OF_RANGE      0x03

#define DET_CALLBACK_API              0xFF

#define DET_CBK_REGISTRATION_FAILED_INDEX	0xFF

#define DET_INIT_SERVICE_ID 0x00
#define DET_REPORTERROR_SERVICE_ID 0x01
#define DET_START_SERVICE_ID 0x02
#define DET_GETVERSIONINFO_SERVICE_ID 0x03

// Type used to store errors
typedef struct
{
  uint16 moduleId;
  uint8 instanceId;
  uint8 apiId;
  uint8 errorId;
} Det_EntryType;

#if ( DET_ENABLE_CALLBACKS == STD_ON )

/*
 * Add a callback function to the array of callback. After a call to Det_ReportError the callback
 * is called. This can be used in for instance unit tests to verify that correct errors are
 * reported when sending invalid parameters to a function.
 * This function returns the index of the callback in the array when registration is successful. If
 * not DET_CBK_REGISTRATION_FAILED_INDEX is returned.
 * The index can be used to remove a callback with the Det_RemoveCbk.
 */
uint8 Det_AddCbk ( detCbk_t detCbk);
void Det_RemoveCbk ( uint8 detCbkIndex);
#endif

/* @req 4.0.3/DET008 */
void Det_Init( void );
#if (DET_DEINIT_API == STD_ON)
void Det_DeInit( void );
#endif
/* @req 4.0.3/DET009 */
void Det_ReportError( uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId);
/* @req 4.0.3/DET010 */
void Det_Start( void );
/* @req 4.0.3/DET011 */
/* @req 4.0.3/DET012 */
/* @req 4.0.3/DET022 */
#if (DET_VERSIONINFO_API == STD_ON)
void Det_GetVersionInfo(Std_VersionInfoType* vi);
#endif

#endif /*DET_H*/
