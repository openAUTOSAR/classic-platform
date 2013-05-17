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








/*
 * Development Error Tracer driver
 *
 */


/*
 *  General requirements
 */
/** @req DET004 */

#ifndef DET_H
#define DET_H

#include "Std_Types.h"
#include "Modules.h"

#define DET_MODULE_ID            MODULE_ID_DET
#define DET_VENDOR_ID            VENDOR_ID_ARCCORE

/* Implementation version */
#define DET_SW_MAJOR_VERSION     1
#define DET_SW_MINOR_VERSION     0
#define DET_SW_PATCH_VERSION     0

/* AUTOSAR specification document version */
#define DET_AR_MAJOR_VERSION     3
#define DET_AR_MINOR_VERSION     1
#define DET_AR_PATCH_VERSION     5

#include "Det_Cfg.h"


// Error codes
#define DET_E_CBK_REGISTRATION_FAILED 0x01
#define DET_E_INDEX_OUT_OF_RANGE      0x02

#define DET_CALLBACK_API              0xFF

#define DET_CBK_REGISTRATION_FAILED_INDEX	0xFF

// Type used to store errors
typedef struct
{
  uint16 moduleId;
  uint8 instanceId;
  uint8 apiId;
  uint8 errorId;
} Det_EntryType;

#if ( DET_ENABLE_CALLBACKS == STD_ON )
typedef void (*detCbk_t)( uint16 ModuleId, uint8 InstanceId , uint8 ApiId, uint8 ErrorId);

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

void Det_Init( void ); /** @req DET008 */
#if DET_DEINIT_API == STD_ON
void Det_DeInit( void );
#endif
void Det_ReportError( uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId); /** @req DET009 */
void Det_Start( void ); /** @req DET010 */
#define Det_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,DET) /** @req DET011 */ /** @req DET012 */

#endif /*DET_H*/
