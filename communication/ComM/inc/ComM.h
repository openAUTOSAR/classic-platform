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





#ifndef COMM_H
#define COMM_H

/** @req COMM466 *//* ComStack_Types.h includes Std_Types.h */
#include "ComStack_Types.h" /** !req COMM820.partially */
#include "ComM_Types.h" /* @req COMM956 *//* Indirectly, ComM_Types.h includes Rte_ComM_Type.h */
#include "ComM_ConfigTypes.h"


/* @req COMM280 */
#define COMM_MODULE_ID              12u
#define COMM_VENDOR_ID              60u

#define COMM_AR_RELEASE_MAJOR_VERSION       4u
#define COMM_AR_RELEASE_MINOR_VERSION       0u
#define COMM_AR_RELEASE_REVISION_VERSION    3u

#define COMM_SW_MAJOR_VERSION       3u
#define COMM_SW_MINOR_VERSION       0u
#define COMM_SW_PATCH_VERSION       1u

/**  @req COMM456 */
#include "ComM_Cfg.h"
#include "ComM_PBcfg.h"
#if (COMM_PNC_SUPPORT == STD_ON)
#include "Com.h"
#endif

/** Function call has been successfully but mode can not
  * be granted because of mode inhibition. */
/** @req COMM649.1 */
#define COMM_E_MODE_LIMITATION          0x02u

/** ComM not initialized */
/** @req COMM649.2 */
#define COMM_E_UNINIT                   0x03u
/** @req COMM509 */
#define COMM_E_NOT_INITED               0x1u /**< API service used without module initialization */
#define COMM_E_WRONG_PARAMETERS	        0x2u /**< API service used with wrong parameters (e.g. a NULL pointer) */

#define COMM_SERVICEID_INIT                             0x01u
#define COMM_SERVICEID_DEINIT                           0x02u
#define COMM_SERVICEID_GETSTATUS                        0x03u
#define COMM_SERVICEID_GETINHIBITIONSTATUS              0x04u
#define COMM_SERVICEID_REQUESTCOMMODE                   0x05u
#define COMM_SERVICEID_GETMAXCOMMODE                    0x06u
#define COMM_SERVICEID_GETREQUESTEDCOMMODE              0x07u
#define COMM_SERVICEID_GETCURRENTCOMMODE                0x08u
#define COMM_SERVICEID_PREVENTWAKEUP                    0x09u
#define COMM_SERVICEID_LIMITCHANNELTONOCOMMODE          0x0bu
#define COMM_SERVICEID_LIMITECUTONOCOMMODE              0x0cu
#define COMM_SERVICEID_READINHIBITCOUNTER               0x0du
#define COMM_SERVICEID_RESETINHIBITCOUNTER              0x0eu
#define COMM_SERVICEID_SETECUGROUPCLASSIFICATION        0x0fu
#define COMM_SERVICEID_GETVERSIONINFO                   0x10u
#define COMM_SERVICEID_NM_NETWORKSTARTINDICATION        0x15u
#define COMM_SERVICEID_NM_NETWORKMODE                   0x18u
#define COMM_SERVICEID_NM_PREPAREBUSSLEEPMODE           0x19u
#define COMM_SERVICEID_NM_BUSSLEEPMODE                  0x1au
#define COMM_SERVICEID_NM_RESTARTINDICATION             0x1bu
#define COMM_SERVICEID_DCM_ACTIVEDIAGNOSTIC             0x1fu
#define COMM_SERVICEID_DCM_INACTIVEDIAGNOSTIC           0x20u
//#define COMM_SERVICEID_ECUM_RUNMODEINDICATION         0x29u
#define COMM_SERVICEID_ECUM_WAKEUPINDICATION            0x2au
#define COMM_SERVICEID_BUSSM_MODEINDICATION             0x33u
#define COMM_SERVICEID_MAINFUNCTION                     0x60u
#define COMM_SERVICEID_GETSTATE                         0x34u
#define COMM_SERVICEID_COMMUNICATIONALLOWED             0x35u

#define COMM_MAIN_FUNCTION_PROTOTYPE(channel) \
void ComM_MainFunction_##channel (void)

#define COMM_MAIN_FUNCTION(channel)	\
void ComM_MainFunction_##channel (void) { \
    ComM_MainFunction(COMM_NETWORK_HANDLE_##channel); \
}

void ComM_MainFunction_All_Channels(void);

/** Initializes the AUTOSAR Communication Manager and restarts the internal state machines.*/
void ComM_Init(const ComM_ConfigType* Config);  /**< @req COMM146 */

/** De-initializes (terminates) the AUTOSAR Communication Manager. */
void ComM_DeInit(void);  /**< @req COMM147 */

/** @req COMM370 */
#if (COMM_VERSION_INFO_API == STD_ON) /** @req COMM823 */
/** @req COMM824 *//** @req COMM822 */
#define ComM_GetVersionInfo(_vi) \
    ((_vi)->vendorID = COMM_VENDOR_ID);\
    ((_vi)->moduleID = COMM_MODULE_ID);\
    ((_vi)->sw_major_version = COMM_SW_MAJOR_VERSION);\
    ((_vi)->sw_minor_version = COMM_SW_MINOR_VERSION);\
    ((_vi)->sw_patch_version = COMM_SW_PATCH_VERSION);
#endif

/** Returns the initialization status of the AUTOSAR Communication Manager. */
Std_ReturnType ComM_GetState(NetworkHandleType Channel, ComM_StateType *State);
Std_ReturnType ComM_GetStatus( ComM_InitStatusType* Status );                                             /**< @req COMM242 */
Std_ReturnType ComM_GetInhibitionStatus( NetworkHandleType Channel, ComM_InhibitionStatusType* Status );  /**< @req COMM619 */

Std_ReturnType ComM_RequestComMode( ComM_UserHandleType User, ComM_ModeType ComMode );        /**< @req COMM110 */
Std_ReturnType ComM_GetMaxComMode( ComM_UserHandleType User, ComM_ModeType* ComMode );
Std_ReturnType ComM_GetRequestedComMode( ComM_UserHandleType User, ComM_ModeType* ComMode );  /**< @req COMM79 */
Std_ReturnType ComM_GetCurrentComMode( ComM_UserHandleType User, ComM_ModeType* ComMode );    /**< @req COMM83 */

Std_ReturnType ComM_PreventWakeUp( NetworkHandleType Channel, boolean Status );               /**< @req COMM156 */
Std_ReturnType ComM_LimitChannelToNoComMode( NetworkHandleType Channel, boolean Status );     /**< @req COMM163 */
Std_ReturnType ComM_LimitECUToNoComMode( boolean Status );                                    /**< @req COMM124 */
Std_ReturnType ComM_ReadInhibitCounter( uint16* CounterValue );                               /**< @req COMM224 */
Std_ReturnType ComM_ResetInhibitCounter(void);                                                /**< @req COMM108 */
Std_ReturnType ComM_SetECUGroupClassification( ComM_InhibitionStatusType Status );
void ComM_EcuM_WakeUpIndication( NetworkHandleType Channel );
/** @req COMM871 */
void ComM_CommunicationAllowed(NetworkHandleType Channel, boolean Allowed);


#if (COMM_PNC_SUPPORT == STD_ON)
/**
 * @brief Function to register new EIRA receive indication
 */
void ComM_Arc_IndicateNewRxEIRA(void);

/**
 * @brief Function to register new ERA receive indication
 */
void ComM_Arc_IndicateNewRxERA(uint8 channelIndex);

#if (HOST_TEST == STD_ON)
uint64 * readEira(void);
ComM_PncStateType checkPncStatus(uint8 idx);
ComM_PncModeType checkPncMode(uint8 idx);
#endif
#endif

#endif /*COMM_H*/
