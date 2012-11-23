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

/** @req CANNM188  @req CANNM196  @req CANNM199    */
#if (CANNM_DEV_ERROR_DETECT == STD_ON)
#define CANNM_DET_REPORTERROR(serviceId, errorId, instanceId)			\
	Det_ReportError(MODULE_ID_CANNM, (uint8)instanceId, serviceId, errorId)

#define CANNM_VALIDATE(expression, serviceId, errorId, instanceId, ...)	\
	if (!(expression)) {									\
		CANNM_DET_REPORTERROR(serviceId, errorId, instanceId);			\
		return __VA_ARGS__;									\
	}

#define CANNM_VALIDATE_NORV(expression, serviceId, errorId, instanceId)	\
	if (!(expression)) {									\
		CANNM_DET_REPORTERROR(serviceId, errorId, instanceId);			\
		return;									\
	}

#else
#define CANNM_DET_REPORTERROR(...)
#define CANNM_VALIDATE(...)
#define CANNM_VALIDATE_NORV(...)
#endif

#define CANNM_VALIDATE_INIT(serviceID)					\
		CANNM_VALIDATE((CanNm_Internal.InitStatus == CANNM_INIT), serviceID, CANNM_E_NO_INIT, 0, NM_E_NOT_OK)

#define CANNM_VALIDATE_INIT_NORV(serviceID)					\
		CANNM_VALIDATE_NORV((CanNm_Internal.InitStatus == CANNM_INIT), serviceID, CANNM_E_NO_INIT, 0)

/** @req CANNM192 */
#define CANNM_VALIDATE_CHANNEL(channel, serviceID)					\
		CANNM_VALIDATE( (channel < CANNM_CHANNEL_COUNT), serviceID, CANNM_E_INVALID_CHANNEL, channel, NM_E_NOT_OK)

#define CANNM_VALIDATE_CHANNEL_NORV(channel, serviceID)					\
		CANNM_VALIDATE_NORV( (channel < CANNM_CHANNEL_COUNT), serviceID, CANNM_E_INVALID_CHANNEL, channel)

#define CANNM_VALIDATE_NOTNULL(ptr, serviceID)	\
		CANNM_VALIDATE_NORV( (ptr != NULL), serviceID, NM_E_NULL_POINTER, 0)

typedef enum {
	CANNM_INIT,
	CANNM_UNINIT
} CanNm_InitStatusType;

typedef struct {
	Nm_ModeType					Mode;				/**< @req CANNM092 */
	Nm_StateType				State;				/**< @req CANNM094 */
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
static inline void CanNm_Internal_TickTimeoutTime( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_TickRepeatMessageTime( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_TickWaitBusSleepTime( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_TickMessageCycleTime( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_ClearCbv( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );

/* Message helpers */
static inline void CanNm_Internal_TransmitMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline uint8 CanNm_Internal_GetUserDataOffset( const CanNm_ChannelType* ChannelConf );
static inline uint8* CanNm_Internal_GetUserDataPtr( const CanNm_ChannelType* ChannelConf, uint8* MessageSduPtr );
static inline uint8 CanNm_Internal_GetUserDataLength( const CanNm_ChannelType* ChannelConf );

/* Transition helpers */
static inline void CanNm_Internal_PrepareBusSleep_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_PrepareBusSleep_to_BusSleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );

static inline void CanNm_Internal_BusSleep_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_BusSleep_to_BusSleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );

static inline void CanNm_Internal_RepeatMessage_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_RepeatMessage_to_ReadySleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_RepeatMessage_to_NormalOperation( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );

static inline void CanNm_Internal_NormalOperation_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_NormalOperation_to_ReadySleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_NormalOperation_to_NormalOperation( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );

static inline void CanNm_Internal_ReadySleep_to_NormalOperation( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_ReadySleep_to_PrepareBusSleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_ReadySleep_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );

static inline void CanNm_Internal_NetworkMode_to_NetworkMode( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );


#endif /* CANNM_INTERNAL_H_ */
