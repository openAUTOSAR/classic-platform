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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.2.2 */

#ifndef STBM_H_
#define STBM_H_

#include "Std_Types.h"
#include "StbM_Types.h"
#include "Rte_StbM_Type.h"
#if defined(USE_ETHTSYN)
#include "EthTSyn.h"
#endif

#define STBM_VENDOR_ID                     60u
#define STBM_MODULE_ID                     160u

#define STBM_AR_RELEASE_MAJOR_VERSION      4u
#define STBM_AR_RELEASE_MINOR_VERSION      2u
#define STBM_AR_RELEASE_REVISION_VERSION   2u

#define STBM_AR_MAJOR_VERSION              STBM_AR_RELEASE_MAJOR_VERSION
#define STBM_AR_MINOR_VERSION              STBM_AR_RELEASE_MINOR_VERSION
#define STBM_AR_PATCH_VERSION              STBM_AR_RELEASE_REVISION_VERSION

#define STBM_SW_MAJOR_VERSION              1u
#define STBM_SW_MINOR_VERSION              0u
#define STBM_SW_PATCH_VERSION              0u


#include "StbM_Cfg.h"


/*
 * Errors described by StbM 7.4.1 Error classification.
 *****************************************************/
/** @req SWS_StbM_00041 */
#define STBM_E_PARAM               0x0Au
#define STBM_E_NOT_INITIALIZED     0x0Bu
#define STBM_E_PARAM_POINTER       0x10u
#define STBM_E_INIT_FAILED         0x11u



#define STBM_INVALID_OS_COUNTER      0xFFFFFFFFu
#define STBM_INVALID_ETH_TIMEDOMAIN  0xFFFFFFFFu
#define STBM_INVALID_TIMEBASE        0xFFFFu
#define STBM_INVALID_NVM_HANDLE      0xFFu

#define STBM_NANOSEC_MAX_VALUE       1000000000ULL

/*
 * Service IDs for StbM function definitions.
 */
/* @req SWS_StbM_00041 */
#define STBM_SERVICE_ID_INIT                       0x00u
#define STBM_SERVICE_ID_MAIN_FUNCTION              0x04u
#define STBM_SERVICE_ID_GET_VERSION_INFO           0x05u
#define STBM_SERVICE_ID_GET_CURRENT_TIME           0x07u
#define STBM_SERVICE_ID_GET_CURRENT_TIME_EXTENDED  0x08u
#define STBM_SERVICE_ID_GET_CURRENT_RAW            0x09u
#define STBM_SERVICE_ID_GET_CURRENT_DIFF           0x0Au
#define STBM_SERVICE_ID_SET_GLOBAL_TIME            0x0Bu
#define STBM_SERVICE_ID_SET_USER_DATA              0x0Cu
#define STBM_SERVICE_ID_SET_OFFSET                 0x0Du
#define STBM_SERVICE_ID_GET_OFFSET                 0x0Eu
#define STBM_SERVICE_ID_BUS_SET_GLOBAL_TIME        0x0Fu

/*lint -esym(9003, StbMConfigData) Could define variable at block scope. OK, false positive.  */
extern const StbM_ConfigType StbMConfigData;
/* 8.1.3 Standard functions */
/* This service returns the version information of this module. */
/* @req SWS_StbM_00066 */
#if ( STBM_VERSION_INFO_API == STD_ON )
void StbM_GetVersionInfo(Std_VersionInfoType* versioninfo);
#endif /* STBM_VERSION_INFO_API */


/* Initializes the Synchronized Time-base Manager  */
/* @req SWS_StbM_00052 */
void StbM_Init(const StbM_ConfigType* ConfigPtr);

/* @req SWS_StbM_00195 */
Std_ReturnType StbM_GetCurrentTime(StbM_SynchronizedTimeBaseType timeBaseId, StbM_TimeStampType* timeStampPtr, StbM_UserDataType* userDataPtr );

/* !req SWS_StbM_00200 */
#if (STBM_GET_CURRENT_TIME_EXT_AVIALBLE == STD_ON)
Std_ReturnType StbM_GetCurrentTimeExtended(StbM_SynchronizedTimeBaseType timeBaseId, StbM_TimeStampExtendedType* timeStampPtr, StbM_UserDataType* userDataPtr );
#endif

/* @req SWS_StbM_00205 */
Std_ReturnType StbM_GetCurrentTimeRaw(StbM_TimeStampRawType* timeStampRawPtr);

/* @req SWS_StbM_00209 */
Std_ReturnType StbM_GetCurrentTimeDiff(StbM_TimeStampRawType givenTimeStamp, StbM_TimeStampRawType* timeStampDiffPtr );

/* @req SWS_StbM_00213 */
Std_ReturnType StbM_SetGlobalTime(StbM_SynchronizedTimeBaseType timeBaseId, const StbM_TimeStampType* timeStampPtr, const StbM_UserDataType* userDataPtr );

/* @req SWS_StbM_00218 */
Std_ReturnType StbM_SetUserData(StbM_SynchronizedTimeBaseType timeBaseId, const StbM_UserDataType* userDataPtr );

/* @req SWS_StbM_00223 */
Std_ReturnType StbM_SetOffset(StbM_SynchronizedTimeBaseType timeBaseId, const StbM_TimeStampType* timeStampPtr );

/* @req SWS_StbM_00228 */
Std_ReturnType StbM_GetOffset(StbM_SynchronizedTimeBaseType timeBaseId, StbM_TimeStampType* timeStampPtr );

/* @req SWS_StbM_00233 */
Std_ReturnType StbM_BusSetGlobalTime(StbM_SynchronizedTimeBaseType timeBaseId, const StbM_TimeStampType* timeStampPtr, const StbM_UserDataType* userDataPtr, boolean syncToTimeBase );

/* @req SWS_StbM_00057 */
void StbM_MainFunction(void);

extern const StbM_ConfigType StbMConfigData;
#endif /* STBM_H_ */

