/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/


#ifndef COMM_INTERNAL_H
#define COMM_INTERNAL_H

#include "ComM_Types.h"

/** @req COMM612  @req COMM511  @req COMM512  @req COMM270  @req COMM523 */
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
		COMM_VALIDATE((ComM_Internal.InitStatus == COMM_INIT), serviceID, COMM_E_NOT_INITED, COMM_E_UNINIT)

#define COMM_VALIDATE_INIT_NORV(serviceID)					\
		COMM_VALIDATE_NORV((ComM_Internal.InitStatus == COMM_INIT), serviceID, COMM_E_NOT_INITED)

#define COMM_VALIDATE_PARAMETER(expression, serviceID)					\
		COMM_VALIDATE(expression, serviceID, COMM_E_WRONG_PARAMETERS, E_NOT_OK)

#define COMM_VALIDATE_PARAMETER_NORV(expression, serviceID)					\
		COMM_VALIDATE_NORV(expression, serviceID, COMM_E_WRONG_PARAMETERS)

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
	ComM_Internal_SubModeType	SubMode;
	uint32						UserRequestMask;
	ComM_InhibitionStatusType	InhibitionStatus;
	uint32						FullComMinDurationTimeLeft;
	uint32						LightTimeoutTimeLeft;
	uint8						NmIndicationMask;
	boolean						RunModeIndication;
	boolean						WakeUp;
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
static Std_ReturnType ComM_Internal_PropagateComMode( const ComM_ChannelType* ChannelConf );

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

/* Perform a Run Request through EcuM */
static void ComM_Internal_RequestRUN(NetworkHandleType Channel);

/* Perform a Release Request through EcuM */
static void ComM_Internal_ReleaseRUN(NetworkHandleType Channel);

/* Private function to check if EcuM has indicated Run Mode */
static boolean ComM_Internal_RunModeIndication( NetworkHandleType Channel );

#endif /* COMM_INTERNAL_H */
