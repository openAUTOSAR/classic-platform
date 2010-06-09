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


#ifndef CANNM_INTERNAL_H_
#define CANNM_INTERNAL_H_

#if (CANNM_DEV_ERROR_DETECT == STD_ON)
#define CANNM_DET_REPORTERROR(serviceId, errorId)			\
	Det_ReportError(MODULE_ID_CANNM, 0, serviceId, errorId)

#define CANNM_VALIDATE(expression, serviceId, errorId, ...)	\
	if (!(expression)) {									\
		CANNM_DET_REPORTERROR(serviceId, errorId);			\
		return __VA_ARGS__;									\
	}

#else
#define CANNM_DET_REPORTERROR(...)
#define CANNM_VALIDATE(...)
#endif

#define CANNM_VALIDATE_INIT(serviceID, ...)					\
		CANNM_VALIDATE((CanNm_Internal.InitStatus == CANNM_INIT), serviceID, CANNM_E_NO_INIT, __VA_ARGS__)

#define CANNM_VALIDATE_CHANNEL(channel, serviceID, ...)					\
		CANNM_VALIDATE( (channel < CANNM_CHANNEL_COUNT), serviceID, CANNM_E_INVALID_CHANNEL, __VA_ARGS__)

#define CANNM_VALIDATE_NOTNULL(ptr, serviceID, ...)	\
		CANNM_VALIDATE( (ptr != NULL), serviceID, NM_E_NULL_POINTER, __VA_ARGS__)

typedef enum {
	CANNM_INIT,
	CANNM_UNINIT,
} CanNm_InitStatusType;

typedef struct {
	Nm_ModeType					Mode;
	Nm_StateType				State;
	boolean						Requested;
	uint32						TimeoutTimeLeft;
	uint32						RepeatMessageTimeLeft;
	uint32						WaitBusSleepTimeLeft;
	uint32						MessageCycleTimeLeft;
	uint32						MessageCycleOffsetTimeLeft;
	uint8						TxMessageSdu[8];
	uint8						RxMessageSdu[8];
} CanNm_Internal_ChannelType;

typedef struct {
	CanNm_InitStatusType 		InitStatus;
	CanNm_Internal_ChannelType	Channels[CANNM_CHANNEL_COUNT];
} CanNm_InternalType;

/* Timer helpers */
static inline void CanNm_Internal_TickTimeoutTime( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_TickRepeatMessageTime( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_TickWaitBusSleepTime( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_TickMessageCycleTime( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_ClearCbv( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );

/* Message helpers */
static inline void CanNm_Internal_TransmitMessage( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline uint8 CanNm_Internal_GetUserDataOffset( CanNm_ChannelType* ChannelConf );
static inline uint8* CanNm_Internal_GetUserDataPtr( CanNm_ChannelType* ChannelConf, uint8* MessageSduPtr );
static inline uint8 CanNm_Internal_GetUserDataLength( CanNm_ChannelType* ChannelConf );

/* Transition helpers */
static inline void CanNm_Internal_PrepareBusSleep_to_RepeatMessage( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_PrepareBusSleep_to_BusSleep( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );

static inline void CanNm_Internal_BusSleep_to_RepeatMessage( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_BusSleep_to_BusSleep( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );

static inline void CanNm_Internal_RepeatMessage_to_RepeatMessage( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_RepeatMessage_to_ReadySleep( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_RepeatMessage_to_NormalOperation( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );

static inline void CanNm_Internal_NormalOperation_to_RepeatMessage( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_NormalOperation_to_ReadySleep( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_NormalOperation_to_NormalOperation( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );

static inline void CanNm_Internal_ReadySleep_to_NormalOperation( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_ReadySleep_to_PrepareBusSleep( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_ReadySleep_to_RepeatMessage( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );

static inline void CanNm_Internal_NetworkMode_to_NetworkMode( CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );


#endif /* CANNM_INTERNAL_H_ */
