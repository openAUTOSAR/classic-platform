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

/** @req COMM466 */
#include "ComStack_Types.h" /** @req COMM820.partially */
#include "ComM_Types.h"
#include "ComM_ConfigTypes.h"
#include "Modules.h"

#define COMM_MODULE_ID			MODULE_ID_COMM
#define COMM_VENDOR_ID			VENDOR_ID_ARCCORE

/** @req COMM469 */
#define COMM_AR_MAJOR_VERSION	4
#define COMM_AR_MINOR_VERSION	0
#define COMM_AR_PATCH_VERSION	3

#define COMM_SW_MAJOR_VERSION	1
#define COMM_SW_MINOR_VERSION	0
#define COMM_SW_PATCH_VERSION	0

/**  @req COMM456 */
#include "ComM_Cfg.h"

/** Function call has been successfully but mode can not
  * be granted because of mode inhibition. */
/** @req COMM649.1 */
#define COMM_E_MODE_LIMITATION			0x02

/** ComM not initialized */
/** @req COMM649.2 */
#define COMM_E_UNINIT					0x03

#define COMM_E_NOT_INITED				0x1 /**< API service used without module initialization */
#define COMM_E_WRONG_PARAMETERS			0x2 /**< API service used with wrong parameters (e.g. a NULL pointer) */
#define COMM_E_ERROR_IN_PROV_SERVICE	0x3 /**< Provided API services of other modules returned with an error. */

#define COMM_SERVICEID_INIT								0x01
#define COMM_SERVICEID_DEINIT							0x02
#define COMM_SERVICEID_GETSTATUS						0x03
#define COMM_SERVICEID_GETINHIBITIONSTATUS				0x04
#define COMM_SERVICEID_REQUESTCOMMODE					0x05
#define COMM_SERVICEID_GETMAXCOMMODE					0x06
#define COMM_SERVICEID_GETREQUESTEDCOMMODE				0x07
#define COMM_SERVICEID_GETCURRENTCOMMODE				0x08
#define COMM_SERVICEID_PREVENTWAKEUP					0x09
#define COMM_SERVICEID_LIMITCHANNELTONOCOMMODE			0x0b
#define COMM_SERVICEID_LIMITECUTONOCOMMODE				0x0c
#define COMM_SERVICEID_READINHIBITCOUNTER				0x0d
#define COMM_SERVICEID_RESETINHIBITCOUNTER				0x0e
#define COMM_SERVICEID_SETECUGROUPCLASSIFICATION		0x0f
#define COMM_SERVICEID_GETVERSIONINFO					0x10
#define COMM_SERVICEID_NM_NETWORKSTARTINDICATION		0x15
#define COMM_SERVICEID_NM_NETWORKMODE					0x18
#define COMM_SERVICEID_NM_PREPAREBUSSLEEPMODE			0x19
#define COMM_SERVICEID_NM_BUSSLEEPMODE					0x1a
#define COMM_SERVICEID_NM_RESTARTINDICATION				0x1b
#define COMM_SERVICEID_DCM_ACTIVEDIAGNOSTIC				0x1f
#define COMM_SERVICEID_DCM_INACTIVEDIAGNOSTIC			0x20
//#define COMM_SERVICEID_ECUM_RUNMODEINDICATION			0x29
#define COMM_SERVICEID_ECUM_WAKEUPINDICATION			0x2a
#define COMM_SERVICEID_BUSSM_MODEINDICATION				0x33
#define COMM_SERVICEID_MAINFUNCTION						0x60
#define COMM_SERVICEID_GETSTATE							0x34
#define COMM_SERVICEID_COMMUNICATIONALLOWED				0x35

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
#define ComM_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,COMM) /** @req COMM824 */
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

/** @req COMM871 */
void ComM_CommunicationAllowed(NetworkHandleType Channel, boolean Allowed);

#ifdef COMM_TESTS
void ComM_MainFunction(NetworkHandleType Channel);
#endif
#endif /*COMM_H*/
