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

/** @req CANSM008 */

#ifndef CANSM_H_
#define CANSM_H_

#include "ComStack_Types.h"       	/* @req CANSM238 */
#include "ComM.h"					/* @req CANSM174 */
#include "CanSM_ConfigTypes.h"


/* @req CANSM559 */
#define CANSM_VENDOR_ID                     60u
#define CANSM_MODULE_ID                     140u

#define CANSM_AR_RELEASE_MAJOR_VERSION      4u
#define CANSM_AR_RELEASE_MINOR_VERSION      0u
#define CANSM_AR_RELEASE_REVISION_VERSION   3u

#define CANSM_AR_MAJOR_VERSION              CANSM_AR_RELEASE_MAJOR_VERSION
#define CANSM_AR_MINOR_VERSION              CANSM_AR_RELEASE_MINOR_VERSION
#define CANSM_AR_PATCH_VERSION              CANSM_AR_RELEASE_REVISION_VERSION

#define CANSM_SW_MAJOR_VERSION              3u
#define CANSM_SW_MINOR_VERSION              2u
#define CANSM_SW_PATCH_VERSION              0u

#include "CanSM_Cfg.h"
/* @req CANSM548 */
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON) && defined(USE_CANNM)
#include "CanNm.h"
#endif

#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
#include "CanTrcv.h"
#endif

/** @req CANSM069 */
#define CANSM_E_UNINIT                              0x01u   /**< API service used without module initialization */
#define CANSM_E_PARAM_POINTER                       0x02u   /**< API service called with wrong pointer */
#define CANSM_E_INVALID_NETWORK_HANDLE              0x03u   /**< API service called with wrong parameter */
#define CANSM_E_PARAM_CONTROLLER                    0x04u   /**< API service called with wrong parameter */
#define CANSM_E_PARAM_TRANSCEIVER                   0x05u
#define CANSM_E_BUSOFF_RECOVERY_ACTIVE              0x06u
#define CANSM_E_WAIT_MODE_INDICATION                0x07u
#define CANSM_E_INVALID_COMM_REQUEST                0x08u
//#define CANSM_E_PARAM_INVALID_BAUDRATE            0x09u
#define CANSM_E_MODE_REQUEST_TIMEOUT                0x0Au

/* ArcCore extra errors */
#define CANSM_E_INVALID_NETWORK_MODE                0x10u
#define CANSM_E_INVALID_ACTION                      0x11u
#define CANSM_E_UNEXPECTED_EXECUTION                0x12u
#define CANSM_E_INVALID_INVALID_BUSOFF              0x13u

#define CANSM_SERVICEID_INIT                        0x00u
#define CANSM_SERVICEID_GETVERSIONINFO              0x01u
#define CANSM_SERVICEID_REQUESTCOMMODE              0x02u
#define CANSM_SERVICEID_GETCURRENTCOMMODE           0x03u
#define CANSM_SERVICEID_CONTROLLERBUSOFF            0x04u
#define CANSM_SERVICEID_MAINFUNCTION                0x05u
#define CANSM_SERVICEID_CONFIRMPNAVAILABILITY       0x06u
#define CANSM_SERVICEID_CONTROLLERMODEINDICATION    0x07u
#define CANSM_SERVICEID_CLEARTRCVWUFINDICATION      0x08u
#define CANSM_SERVICEID_TRANSCEIVERMODEINDICATION   0x09u
#define CANSM_SERVICEID_CHECKTRCVWUFINDICATION      0x0au


/* @req CANSM008 */


/** This service puts out the version information of this module */
/** @req CANSM024  @req CANSM367 @req CANSM244 @req CANSM368 @req CANSM366 */
/* !req CANSM374 */
#if (CANSM_VERSION_INFO_API == STD_ON)
#define CanSM_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,CANSM)
#endif

/** This service shall change the communication mode of a CAN network to the requested one. */
/** @req CANSM062 */
/* IMPROVEMENT: Move to CanSM_ComM.h? */
Std_ReturnType CanSM_RequestComMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode );

/** This service shall put out the current communication mode of a CAN network. */
/** @req CANSM063 */
/* IMPROVEMENT: Move to CanSM_ComM.h? */
Std_ReturnType CanSM_GetCurrentComMode( NetworkHandleType NetworkHandle, ComM_ModeType* ComM_ModePtr );

/** Init function for CanSM */
/** @req CANSM023 */
void CanSM_Init( const CanSM_ConfigType* ConfigPtr );

/** Scheduled function of the CanSM */
/** @req CANSM065 */
void CanSM_MainFunction(void);

#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
/* @req CANSM399 */
void CanSM_TransceiverModeIndication(uint8 TransceiverId, CanTrcv_TrcvModeType TransceiverMode);
/* @req CANSM413 */
void CanSM_ClearTrcvWufFlagIndication( uint8 Transceiver );
/* @req CANSM416 */
void CanSM_CheckTransceiverWakeFlagIndication( uint8 Transceiver );
/* @req CANSM419 */
void CanSM_ConfirmPnAvailability( uint8 Transceiver );
#endif

#endif /* CANSM_H_ */
