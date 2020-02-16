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


#ifndef UDPNM_INTERNAL_H_
#define UDPNM_INTERNAL_H_

/** @req UDPNM223 @req UDPNM189 @req UDPNM190 @req UDPNM210 @req UDPNM196 */
#if (UDPNM_DEV_ERROR_DETECT == STD_ON)
#define UDPNM_DET_REPORTERROR(serviceId, errorId, instanceId)			\
	Det_ReportError(MODULE_ID_UDPNM, (uint8)instanceId, serviceId, errorId)

#define UDPNM_VALIDATE(expression, serviceId, errorId, instanceId, ...)	\
	if (!(expression)) {									\
		UDPNM_DET_REPORTERROR(serviceId, errorId, instanceId);			\
		return __VA_ARGS__;									\
	}

#else
#define UDPNM_DET_REPORTERROR(...)
#define UDPNM_VALIDATE(...)
#endif

/** @req UDPNM020 @req UDPNM191 */
#define UDPNM_VALIDATE_INIT(serviceID, ...)					\
		UDPNM_VALIDATE((UdpNm_Internal.InitStatus == UDPNM_INIT), serviceID, UDPNM_E_NO_INIT, 0, __VA_ARGS__)

/** @req UDPNM192 */
#define UDPNM_VALIDATE_CHANNEL(channel, serviceID, ...)					\
		UDPNM_VALIDATE( (channel < UDPNM_CHANNEL_COUNT), serviceID, UDPNM_E_INVALID_CHANNEL, channel, __VA_ARGS__)

/** @req UDPNM292 */
#define UDPNM_VALIDATE_NOTNULL(ptr, serviceID, ...)	\
		UDPNM_VALIDATE( (ptr != NULL), serviceID, UDPNM_E_NULL_POINTER, 0, __VA_ARGS__)

typedef enum {
	UDPNM_INIT,
	UDPNM_UNINIT
} UdpNm_InitStatusType;

typedef struct {
	Nm_ModeType					Mode;
	Nm_StateType				State;
	boolean						Requested;
	uint32						TimeoutTimeLeft;
	uint32						RepeatMessageTimeLeft;
	uint32						WaitBusSleepTimeLeft;
	uint32						MessageCycleTimeLeft;
	uint32						MessageTimeoutTimeLeft;
	uint8						TxMessageSdu[8];
	uint8						RxMessageSdu[8];
} UdpNm_Internal_ChannelType;

typedef struct {
	UdpNm_InitStatusType 		InitStatus;
	UdpNm_Internal_ChannelType	Channels[UDPNM_CHANNEL_COUNT];
} UdpNm_InternalType;

/* Timer helpers */
static inline void UdpNm_Internal_TickTimeoutTime( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_TickMessageTimeoutTime( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_TickRepeatMessageTime( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_TickWaitBusSleepTime( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_TickMessageCycleTime( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );

/* Message helpers */
static inline void UdpNm_Internal_TransmitMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline uint8 UdpNm_Internal_GetUserDataOffset( const UdpNm_ChannelType* ChannelConf );
static inline uint8* UdpNm_Internal_GetUserDataPtr( const UdpNm_ChannelType* ChannelConf, uint8* MessageSduPtr );
static inline uint8 UdpNm_Internal_GetUserDataLength( const UdpNm_ChannelType* ChannelConf );
static inline void UdpNm_Internal_ClearCbv( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );

/* Transition helpers */
static inline void UdpNm_Internal_PrepareBusSleep_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_PrepareBusSleep_to_BusSleep( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );

static inline void UdpNm_Internal_BusSleep_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_BusSleep_to_BusSleep( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );

static inline void UdpNm_Internal_RepeatMessage_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_RepeatMessage_to_ReadySleep( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_RepeatMessage_to_NormalOperation( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );

static inline void UdpNm_Internal_NormalOperation_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_NormalOperation_to_ReadySleep( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_NormalOperation_to_NormalOperation( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );

static inline void UdpNm_Internal_ReadySleep_to_NormalOperation( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_ReadySleep_to_PrepareBusSleep( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_ReadySleep_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );

static inline void UdpNm_Internal_NetworkMode_to_NetworkMode( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );


#endif /* UDPNM_INTERNAL_H_ */
