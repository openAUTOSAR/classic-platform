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


#ifndef COMM_INTERNAL_H
#define COMM_INTERNAL_H

#include "ComM_Types.h"

#if (COMM_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif

#define COMM_PNC_MAX_NUM    48u

/*lint -emacro(904,COMM_VALIDATE_PARAMETER_NORV,COMM_VALIDATE_INIT_NORV,COMM_VALIDATE_INIT,COMM_VALIDATE_PARAMETER) 
 * ARGUMENT_CHECK Macros used for checking arguments before performing any functionality [MISRA 2004 Rule 14.7, required] */

/** @req COMM612  @req COMM511  @req COMM512  @req COMM270 */
#if (COMM_DEV_ERROR_DETECT == STD_ON)
#define COMM_DET_REPORTERROR(serviceId, errorId)			\
    (void)Det_ReportError(COMM_MODULE_ID, 0, serviceId, errorId)

#define COMM_VALIDATE(expression, serviceId, errorId, ret)	\
    if (!(expression)) {									\
        COMM_DET_REPORTERROR(serviceId, errorId);			\
        return ret;									\
    }

#define COMM_VALIDATE_NORV(expression, serviceId, errorId)	\
    if (!(expression)) {									\
        COMM_DET_REPORTERROR(serviceId, errorId);			\
        return;									\
    }


#else
#define COMM_DET_REPORTERROR(...)
#define COMM_VALIDATE(...)
#define COMM_VALIDATE_NORV(...)
#endif

#define COMM_VALIDATE_INIT(serviceID)					\
        COMM_VALIDATE((ComM_Internal.InitStatus == COMM_INIT), serviceID, COMM_E_NOT_INITED, COMM_E_UNINIT) /** @req COMM234 */ /** @req COMM858 */

#define COMM_VALIDATE_INIT_NORV(serviceID)					\
        COMM_VALIDATE_NORV((ComM_Internal.InitStatus == COMM_INIT), serviceID, COMM_E_NOT_INITED)

#define COMM_VALIDATE_PARAMETER(expression, serviceID)					\
        COMM_VALIDATE(expression, serviceID, COMM_E_WRONG_PARAMETERS, E_NOT_OK)

#define COMM_VALIDATE_PARAMETER_NORV(expression, serviceID)					\
        COMM_VALIDATE_NORV(expression, serviceID, COMM_E_WRONG_PARAMETERS) /** @req COMM234 */

#define COMM_VALIDATE_CHANNEL(channel, serviceID)					\
        COMM_VALIDATE_PARAMETER( (channel < COMM_CHANNEL_COUNT), serviceID)

#define COMM_VALIDATE_CHANNEL_NORV(channel, serviceID)					\
        COMM_VALIDATE_PARAMETER_NORV( (channel < COMM_CHANNEL_COUNT), serviceID)

#define COMM_VALIDATE_USER(user, serviceID)					\
        COMM_VALIDATE_PARAMETER( (user < COMM_USER_COUNT), serviceID )


typedef enum {
    COMM_SUBMODE_NETWORK_REQUESTED,
    COMM_SUBMODE_READY_SLEEP,
    COMM_SUBMODE_NONE
} ComM_Internal_SubModeType;

typedef enum {
    TRIGGER_SEND_PNC_VAL_0,
    TRIGGER_SEND_PNC_VAL_1,
    INVALID_TRIGGER_SEND
}ComM_Internal_TriggerSendType;

typedef struct {
    ComM_ModeType				RequestedMode;
    ComM_ModeType               CurrentMode;
} ComM_Internal_UserType;

#if (COMM_PNC_SUPPORT == STD_ON)
#define MAX_PNC_NUM_BYTES     8u
typedef union {
    uint64 data;
    uint8  bytes[MAX_PNC_NUM_BYTES];
}ComM_Internal_RAType; /* Same type for EIRA and ERA. */

typedef struct {
    uint32                  prepareSleepTimer;
    ComM_PncStateType       pncState;
    ComM_PncModeType        pncSubState;
    ComM_ModeType           pncRequestedState;
    boolean                 pncNewUserRequest;
}ComM_PncInternalType;
#endif /* (COMM_PNC_SUPPORT == STD_ON) */

typedef struct {
    ComM_ModeType				Mode;
    ComM_StateType				SubMode;
    uint64						UserRequestMask;
    ComM_InhibitionStatusType	InhibitionStatus;
    uint32						FullComMinDurationTimeLeft;
    uint32						LightTimeoutTimeLeft;
    ComM_ModeType               userOrPncReqMode;
    ComM_ModeType               lastRequestedMode;
    uint8                       NmIndicationMask;
    boolean                     CommunicationAllowed;
    boolean                     DCM_Requested;
    boolean                     requestPending;
    boolean						EcuMWkUpIndication;
    boolean                     userOrPncReqPending;
    boolean                     internalRequest;
    boolean                     nwStartIndication;
    boolean                     fullComMinDurationTimerStopped;
    boolean                     nmLightTimeoutTimerStopped;

#if (COMM_PNC_SUPPORT == STD_ON)
    ComM_Internal_RAType		TxComSignal;
#if (COMM_PNC_GATEWAY_ENABLED == STD_ON)
    ComM_Internal_RAType		pnERA;
    boolean                     newERARxSignal;
#endif
#endif
} ComM_Internal_ChannelType;

typedef struct {
    ComM_InitStatusType 		InitStatus;
    ComM_Internal_ChannelType	Channels[COMM_CHANNEL_COUNT];
    ComM_Internal_UserType		Users[COMM_USER_COUNT];
    boolean 					NoCommunication;
    uint16						InhibitCounter;  /**< @req COMM138  @req COMM141 */
#if (COMM_PNC_SUPPORT == STD_ON)
    ComM_PncInternalType        pncRunTimeData[COMM_PNC_NUM];
    ComM_Internal_RAType		pnEIRA;
    boolean                     newEIRARxSignal;
#endif
} ComM_InternalType;


#define COMM_NM_INDICATION_NONE					(uint8)(0u)
#define COMM_NM_INDICATION_NETWORK_MODE			(uint8)(1u)
#define COMM_NM_INDICATION_PREPARE_BUS_SLEEP	(uint8)(1u << 1)
#define COMM_NM_INDICATION_BUS_SLEEP			(uint8)(1u << 2)
#define COMM_NM_INDICATION_RESTART				(uint8)(1u << 3)

/*lint -esym(9003,ComM_Internal) FALSE_POSITIVE ComM_Internal cannot be defined at block scope because it is used in ComM.c and ComM_ASIL.c */
extern ComM_InternalType ComM_Internal;

#endif /* COMM_INTERNAL_H */
