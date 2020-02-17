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
/** @req LINIF691 */

#ifndef LINIF_H_
#define LINIF_H_

#define LINIF_MODULE_ID            62u
#define LINIF_VENDOR_ID            60u

#define LINIF_SW_MAJOR_VERSION              4u
#define LINIF_SW_MINOR_VERSION              0u
#define LINIF_SW_PATCH_VERSION              0u
#define LINIF_AR_RELEASE_MAJOR_VERSION      4u
#define LINIF_AR_RELEASE_MINOR_VERSION      0u
#define LINIF_AR_RELEASE_PATCH_VERSION      3u
/* @req LINIF499 */
#include "Std_Types.h"
/** @req LINIF638 */
#include "Lin_GeneralTypes.h"
#include "LinIf_Cfg.h"
#include "LinIf_Types.h"
#include "ComM_Types.h"


/** @name Service id's */
//@{
#define LINIF_INIT_SERVICE_ID               0x00u
#define LINIF_GETVERSIONINFO_SERVICE_ID     0x03u
#define LINIF_SCHEDULEREQUEST_SERVICE_ID    0x05u
#define LINIF_GOTOSLEEP_SERVICE_ID    		0x06u
#define LINIF_WAKEUP_SERVICE_ID     		0x07u
#define LINIF_MAINFUNCTION_SERVICE_ID       0x80u
//@}
/**@req LINIF267 */
/** @name Error Codes */
//@{
#define LINIF_E_UNINIT                      (uint8) 0x00u
#define LINIF_E_ALREADY_INITIALIZED         (uint8) 0x10u
#define LINIF_E_NONEXISTENT_CHANNEL         (uint8) 0x20u
#define LINIF_E_PARAMETER                   (uint8) 0x30u
#define LINIF_E_PARAMETER_POINTER           (uint8) 0x40u
#define LINIF_E_SCHEDULE_OVERFLOW           (uint8) 0x50u
#define LINIF_E_SCHEDULE_REQUEST_ERROR      (uint8) 0x51u
#define LINIF_E_RESPONSE                    (uint8) 0x60u
#define LINIF_E_NC_NO_RESPONSE              (uint8) 0x61u
// Following error codes are added by ArcCore
#define LINIF_E_UNEXPECTED_EXECUTION        0x70u
//@}

/**@req LINIF278 */ /**@req LINIF487 *//**@req LINIF340 */
#if (LINIF_VERSION_INFO_API == STD_ON)
void LinIf_GetVersionInfo( Std_VersionInfoType *versionInfo );
#define LinIf_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,LINIF)
#endif

void LinIf_Init( const LinIf_ConfigType* ConfigPtr );

void LinIf_DeInit(void);

Std_ReturnType LinIf_Transmit(PduIdType LinTxPduId,const PduInfoType* PduInfoPtr);

Std_ReturnType LinIf_ScheduleRequest(NetworkHandleType Channel,LinIf_SchHandleType Schedule);

Std_ReturnType LinIf_GotoSleep(NetworkHandleType Channel);

Std_ReturnType LinIf_WakeUp(NetworkHandleType Channel);

void LinIf_MainFunction(void);

#endif
/** @} */
