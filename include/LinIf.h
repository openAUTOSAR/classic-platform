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

/** @addtogroup LinIf LIN Interface
 *  @{ */

/** @file LinIf.h
 * API and type definitions for LIN Interface.
 */

#ifndef LINIF_H_
#define LINIF_H_

#define LINIF_MODULE_ID            MODULE_ID_LINIF
#define LINIF_VENDOR_ID            VENDOR_ID_ARCCORE

#define LINIF_SW_MAJOR_VERSION              2
#define LINIF_SW_MINOR_VERSION              0
#define LINIF_SW_PATCH_VERSION              0
#define LINIF_AR_RELEASE_MAJOR_VERSION      4
#define LINIF_AR_RELEASE_MINOR_VERSION      0
#define LINIF_AR_RELEASE_PATCH_VERSION      3

#include "Std_Types.h"
#include "LinIf_Cfg.h"
#include "LinIf_Types.h"
#include "ComStack_Types.h"
#include "ComM_Types.h"
#include "Modules.h"


/** @name Service id's */
//@{
#define LINIF_INIT_SERVICE_ID               0x00
#define LINIF_GETVERSIONINFO_SERVICE_ID     0x03
#define LINIF_SCHEDULEREQUEST_SERVICE_ID    0x05
#define LINIF_GOTOSLEEP_SERVICE_ID    		0x06
#define LINIF_WAKEUP_SERVICE_ID     		0x07
#define LINIF_MAINFUNCTION_SERVICE_ID       0x80
//@}

/** @name Error Codes */
//@{
#define LINIF_E_UNINIT                      0x00
#define LINIF_E_ALREADY_INITIALIZED         0x10
#define LINIF_E_NONEXISTENT_CHANNEL         0x20
#define LINIF_E_PARAMETER                   0x30
#define LINIF_E_PARAMETER_POINTER           0x40
#define LINIF_E_SCHEDULE_OVERFLOW           0x50
#define LINIF_E_SCHEDULE_REQUEST_ERROR      0x51
#define LINIF_E_RESPONSE                    0x60
#define LINIF_E_NC_NO_RESPONSE              0x61
//@}

#if (LINIF_VERSION_INFO_API == STD_ON)
void LinIf_GetVersionInfo( Std_VersionInfoType *versionInfo );
#define LinIf_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,LINIF)
#endif

void LinIf_Init( const LinIf_ConfigType* ConfigPtr );

void LinIf_DeInit();

Std_ReturnType LinIf_Transmit(PduIdType LinTxPduId,const PduInfoType* PduInfoPtr);

Std_ReturnType LinIf_ScheduleRequest(NetworkHandleType Channel,LinIf_SchHandleType Schedule);

Std_ReturnType LinIf_GotoSleep(NetworkHandleType Channel);

Std_ReturnType LinIf_WakeUp(NetworkHandleType Channel);

void LinIf_MainFunction();

#endif
/** @} */
