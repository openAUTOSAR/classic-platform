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
#include "Modules.h"

#define CANSM_VENDOR_ID			VENDOR_ID_ARCCORE
#define CANSM_MODULE_ID			MODULE_ID_CANSM

#define CANSM_AR_RELEASE_MAJOR_VERSION		4
#define CANSM_AR_RELEASE_MINOR_VERSION		0
#define CANSM_AR_RELEASE_REVISION_VERSION	3

#define CANSM_AR_MAJOR_VERSION	CANSM_AR_RELEASE_MAJOR_VERSION
#define CANSM_AR_MINOR_VERSION	CANSM_AR_RELEASE_MINOR_VERSION
#define CANSM_AR_PATCH_VERSION	CANSM_AR_RELEASE_REVISION_VERSION

#define CANSM_SW_MAJOR_VERSION	1
#define CANSM_SW_MINOR_VERSION	0
#define CANSM_SW_PATCH_VERSION	0

#include "CanSM_Cfg.h"             /**<  @req CANSM025.configuration */


#define CANSM_E_UNINIT 					0x01	/**< API service used without module initialization */
#define CANSM_E_PARAM_POINTER 			0x02	/**< API service called with wrong pointer */
#define CANSM_E_INVALID_NETWORK_HANDLE	0x03	/**< API service called with wrong parameter */
#define CANSM_E_PARAM_CONTROLLER		0x04	/**< API service called with wrong parameter */
//#define CANSM_E_PARAM_TRANSCEIVER		0x05
#define CANSM_E_BUSOFF_RECOVERY_ACTIVE 	0x06
#define CANSM_E_WAIT_MODE_INDICATION 	0x07
#define CANSM_E_INVALID_COMM_REQUEST 	0x08
//#define CANSM_E_PARAM_INVALID_BAUDRATE 	0x09
#define CANSM_E_MODE_REQUEST_TIMEOUT 	0x0A

/* ArcCore extra errors */
#define CANSM_E_INVALID_NETWORK_MODE 	0x10
#define CANSM_E_INVALID_INVALID_BUSOFF  0x11

#define CANSM_SERVICEID_INIT					0x00
#define CANSM_SERVICEID_GETVERSIONINFO			0x01
#define CANSM_SERVICEID_REQUESTCOMMODE			0x02
#define CANSM_SERVICEID_GETCURRENTCOMMODE		0x03
#define CANSM_SERVICEID_CONTROLLERBUSOFF		0x04
#define CANSM_SERVICEID_MAINFUNCTION			0x05
#define CANSM_SERVICEID_CONTROLLERMODEINDICATION	0x07

/* @req CANSM008 */


/** This service puts out the version information of this module */
/** @req CANSM024  @req CANSM367 @req CANSM244 @req CANSM368 @req CANSM366 */
/* !req CANSM374 */
#if (CANSM_VERSION_INFO_API == STD_ON)
#define CanSM_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,CANSM)
#endif

/** This service shall change the communication mode of a CAN network to the requested one. */
/** @req CANSM062 */
/* TODO: Move to CanSM_ComM.h? */
Std_ReturnType CanSM_RequestComMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode );

/** This service shall put out the current communication mode of a CAN network. */
/** @req CANSM063 */
/* TODO: Move to CanSM_ComM.h? */
Std_ReturnType CanSM_GetCurrentComMode( NetworkHandleType NetworkHandle, ComM_ModeType* ComM_ModePtr );

/** Init function for CanSM */
/** @req CANSM023 */
void CanSM_Init( const CanSM_ConfigType* ConfigPtr );

/** Scheduled function of the CanSM */
/** @req CANSM065 */
void CanSM_MainFunction(void);

#endif /* CANSM_H_ */
