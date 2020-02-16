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

/** @req COMM612  @req COMM511  @req COMM512  @req COMM270 */
#if (COMM_DEV_ERROR_DETECT == STD_ON)
#define COMM_DET_REPORTERROR(serviceId, errorId)			\
	Det_ReportError(MODULE_ID_COMM, 0, serviceId, errorId)

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

typedef struct {
	ComM_ModeType				Mode;
	ComM_StateType				SubMode;
	uint32						UserRequestMask;
	ComM_InhibitionStatusType	InhibitionStatus;
	uint32						FullComMinDurationTimeLeft;
	uint32						LightTimeoutTimeLeft;
	uint8						NmIndicationMask;
	boolean						CommunicationAllowed;
	boolean						DCM_Requested;
	ComM_StateType				RequestedSubMode;
} ComM_Internal_ChannelType;

typedef struct {
	ComM_ModeType				RequestedMode;
} ComM_Internal_UserType;

typedef struct {
	ComM_InitStatusType 		InitStatus;
	ComM_Internal_ChannelType	Channels[COMM_CHANNEL_COUNT];
	ComM_Internal_UserType		Users[COMM_USER_COUNT];
	boolean 					NoCommunication;
	uint16						InhibitCounter;  /**< @req COMM138  @req COMM141 */
} ComM_InternalType;

#define COMM_NM_INDICATION_NONE					(uint8)(0u)
#define COMM_NM_INDICATION_NETWORK_MODE			(uint8)(1u)
#define COMM_NM_INDICATION_PREPARE_BUS_SLEEP	(uint8)(1u << 1)
#define COMM_NM_INDICATION_BUS_SLEEP			(uint8)(1u << 2)
#define COMM_NM_INDICATION_RESTART				(uint8)(1u << 3)

/* Delegate request to users channels and call ComM_Internal_UpdateChannelState */
static Std_ReturnType ComM_Internal_RequestComMode(
				ComM_UserHandleType User, ComM_ModeType ComMode );

/* Looks at stored requests for Channel and updates state accordingly */
static Std_ReturnType ComM_Internal_UpdateChannelState( const ComM_ChannelType* ChannelConf, boolean isRequest );

static inline Std_ReturnType ComM_Internal_UpdateFromNoCom(const ComM_ChannelType* ChannelConf,
					ComM_Internal_ChannelType* ChannelInternal, boolean isRequest);

static inline Std_ReturnType ComM_Internal_UpdateFromSilentCom(const ComM_ChannelType* ChannelConf,
					ComM_Internal_ChannelType* ChannelInternal,	boolean isRequest);

static inline Std_ReturnType ComM_Internal_UpdateFromFullCom(const ComM_ChannelType* ChannelConf,
					ComM_Internal_ChannelType* ChannelInternal, boolean isRequest);

static inline Std_ReturnType ComM_Internal_Enter_NoCom(const ComM_ChannelType* ChannelConf,
													ComM_Internal_ChannelType* ChannelInternal);

static inline Std_ReturnType ComM_Internal_Enter_SilentCom(const ComM_ChannelType* ChannelConf,
														ComM_Internal_ChannelType* ChannelInternal);

static inline Std_ReturnType ComM_Internal_Enter_NetworkRequested(const ComM_ChannelType* ChannelConf,
																ComM_Internal_ChannelType* ChannelInternal);

static inline Std_ReturnType ComM_Internal_Enter_ReadySleep(const ComM_ChannelType* ChannelConf,
															ComM_Internal_ChannelType* ChannelInternal);

/* Propagates channel mode to BusSM */
static Std_ReturnType ComM_Internal_PropagateComMode( const ComM_ChannelType* ChannelConf, ComM_ModeType ComMode );

/* Requests/releases Nm network according to channel mode */
static Std_ReturnType ComM_Internal_NotifyNm( const ComM_ChannelType* ChannelConf);

/* Propagate query to channel Bus SMs. Collect overall mode and status */
static Std_ReturnType ComM_Internal_PropagateGetCurrentComMode(
		ComM_UserHandleType User, ComM_ModeType* ComMode );

/* Tick 'Min full com duration' timeout, and update state if needed */
static inline Std_ReturnType ComM_Internal_TickFullComMinTime(const ComM_ChannelType* ChannelConf,
		ComM_Internal_ChannelType* ChannelInternal);

static inline boolean ComM_Internal_FullComMinTime_AllowsExit(const ComM_ChannelType* ChannelConf,
		const ComM_Internal_ChannelType* ChannelInternal);

/* Tick 'Light nm' timeout, and update state if needed */
static inline Std_ReturnType ComM_Internal_TickLightTime(const ComM_ChannelType* ChannelConf,
		ComM_Internal_ChannelType* ChannelInternal);

#endif /* COMM_INTERNAL_H */
