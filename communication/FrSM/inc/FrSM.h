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

#ifndef FRSM_H_
#define FRSM_H_

#include "ComStack_Types.h"
#include "ComM.h"
#include "FrSM_Types.h" /* @req FrSm121 */
#if (USE_DEM)
#include "Dem.h"
#endif


/* @req FrSm139 */
#define FRSM_VENDOR_ID                     60u
#define FRSM_MODULE_ID                     142u

#define FRSM_AR_RELEASE_MAJOR_VERSION      4u
#define FRSM_AR_RELEASE_MINOR_VERSION      0u
#define FRSM_AR_RELEASE_REVISION_VERSION   3u

#define FRSM_AR_MAJOR_VERSION              FRSM_AR_RELEASE_MAJOR_VERSION
#define FRSM_AR_MINOR_VERSION              FRSM_AR_RELEASE_MINOR_VERSION
#define FRSM_AR_PATCH_VERSION              FRSM_AR_RELEASE_REVISION_VERSION

#define FRSM_SW_MAJOR_VERSION              1u
#define FRSM_SW_MINOR_VERSION              0u
#define FRSM_SW_PATCH_VERSION              0u

/** Error IDs  */
/* @req FrSm042 */
#define FRSM_E_NULL_PTR		      1u
#define FRSM_E_INV_HANDLE	      2u
#define FRSM_E_UNINIT 		      3u
#define FRSM_E_INV_MODE		      4u
#define FRSM_E_CLUSTER_STARTUP    5u
#define FRSM_E_CLUSTER_SYNC_LOSS  6u

/** Service IDs */
#define FRSM_SERVICE_ID_INIT						1u
#define FRSM_SERVICE_ID_REQUEST_COM_MODE			2u
#define FRSM_SERVICE_ID_GET_CURRENT_COM_MODE		3u
#define FRSM_SERVICE_ID_GET_VERSION_INFO			4u
#define FRSM_SERVICE_ID_ALL_SLOTS					5u
#define FRSM_SERVICE_ID_SET_ECU_PASSIVE				6u
#define FRSM_SERVICE_ID_MAIN_FUNCTION				0x80u

/* @req FrSm121 */
#include "FrSM_Cfg.h"

 /* Publish the configuration */
extern const FrSM_ConfigType FrSMConfigData;

/* @req FrSm120 */
/** FrSM.h shall export the API of the FrSM module */
/** Init function for FrSM */
/* @req FrSm013 */
void FrSM_Init( const FrSM_ConfigType* FrSM_ConfigPtr );

/** This service shall change the communication mode of a CAN network to the requested one. **/
/* @req FrSm020 */
Std_ReturnType FrSM_RequestComMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode );

/** This service shall put out the current communication mode of a CAN network. */
/* @req FrSm024 */
Std_ReturnType FrSM_GetCurrentComMode( NetworkHandleType NetworkHandle, ComM_ModeType* ComM_ModePtr );

/** This service puts out the version information of this module */
/* @req FrSm029 */
/* @req FrSm129 */
/* @req FrSm130 */
#if (FRSM_VERSION_INFO_API == STD_ON)
#define FrSM_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,FRSM)
#endif

/* @req FrSm172 */
Std_ReturnType FrSM_AllSlots( NetworkHandleType NetworkHandle );

/* @req FrSm174 */
Std_ReturnType FrSM_SetEcuPassive( boolean FrSM_Passive );

#ifdef HOST_TEST
extern void ReportErrorStatus(void);
#endif

#endif /* FRSM_H_ */
