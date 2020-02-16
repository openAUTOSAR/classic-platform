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
		CANNM_VALIDATE((CanNm_Internal.InitStatus == CANNM_INIT), serviceID, CANNM_E_NO_INIT, 0, E_NOT_OK)

#define CANNM_VALIDATE_INIT_NORV(serviceID, instanceID)					\
		CANNM_VALIDATE_NORV((CanNm_Internal.InitStatus == CANNM_INIT), serviceID, CANNM_E_NO_INIT, instanceID)

/** @req CANNM192 */
#define CANNM_VALIDATE_CHANNEL(nmHandle, serviceID)					\
		CANNM_VALIDATE( ((nmHandle < CANNM_NM_CHANNELS_COUNT) && (CanNm_ConfigPtr->ChannelLookups[nmHandle] != CANNM_UNUSED_CHANNEL)), serviceID, CANNM_E_INVALID_CHANNEL, nmHandle, E_NOT_OK)

#define CANNM_VALIDATE_NOTNULL(ptr, serviceID, instanceID)	\
		CANNM_VALIDATE((ptr != NULL), serviceID, NM_E_NULL_POINTER, instanceID, E_NOT_OK)

#define CANNM_VALIDATE_NOTNULL_NORV(ptr, serviceID, instanceID)	\
		CANNM_VALIDATE_NORV( (ptr != NULL), serviceID, NM_E_NULL_POINTER, instanceID)

#define CANNM_VALIDATE_NOTNULL_INIT(ptr, serviceID, instanceID)	\
		CANNM_VALIDATE_NORV( (ptr != NULL), serviceID, CANNM_E_INIT_FAILED, instanceID)

#define CANNM_VALIDATE_PDUID_NORV(pduID, serviceID) \
		CANNM_VALIDATE_NORV( (pduID < CANNM_CHANNEL_COUNT), serviceID, CANNM_E_INVALID_PDUID, pduID)

#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
#define CANNM_SPONTANEOUS_TRANSMISSION 3
#define CANNM_ONGOING_TRANSMISSION 1
#define CANNM_NO_TRANSMISSION 0
#endif

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
	uint32                      MessageTimeoutTimeLeft;
	uint8						TxMessageSdu[8];
	uint8						RxMessageSdu[8];
	uint8						immediateNmTransmissionsSent;
	boolean						immediateModeActive;
#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
	uint8						TransmissionStatus;
	uint8						SpontaneousTxMessageSdu[8];
#elif (CANNM_USER_DATA_ENABLED == STD_ON) /* and com_user_data... == OFF */
	boolean						IsUserDataSet;
#endif
	boolean 	                CommunicationEnabled;
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
static inline void CanNm_Internal_TickTxTimeout(const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline void CanNm_Internal_ClearCbv( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );

/* Message helpers */
static inline void CanNm_Internal_TransmitMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );
static inline uint8 CanNm_Internal_GetUserDataOffset( const CanNm_ChannelType* ChannelConf );
static inline uint8* CanNm_Internal_GetUserDataPtr( const CanNm_ChannelType* ChannelConf, uint8* MessageSduPtr );
static inline uint8 CanNm_Internal_GetUserDataLength( const CanNm_ChannelType* ChannelConf );

/* Transition helpers */
static inline void CanNm_Internal_PrepareBusSleep_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ,boolean isNwReq );
static inline void CanNm_Internal_PrepareBusSleep_to_BusSleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal );

static inline void CanNm_Internal_BusSleep_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal, boolean isNwReq );
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
