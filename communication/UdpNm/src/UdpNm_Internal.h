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
/*lint -emacro(904,UDPNM_VALIDATE,UDPNM_VALIDATE_INIT,UDPNM_VALIDATE_CHANNEL,UDPNM_VALIDATE_NOTNULL,UDPNM_VALIDATE_PDUID) 904 PC-Lint exception to MISRA 14.7 (validate DET macros) */
/** @req SWS_UdpNm_00223 @req SWS_UdpNm_00189 @req SWS_UdpNm_00190 @req SWS_UdpNm_00210 @req SWS_UdpNm_00196 */
#if (UDPNM_DEV_ERROR_DETECT == STD_ON)
#define UDPNM_DET_REPORTERROR(serviceId, errorId, instanceId)			\
    (void)Det_ReportError(UDPNM_MODULE_ID, (uint8)instanceId, serviceId, errorId)

#define UDPNM_VALIDATE(expression, serviceId, errorId, instanceId, ...)	\
    if (!(expression)) {									\
        UDPNM_DET_REPORTERROR(serviceId, errorId, instanceId);			\
        return __VA_ARGS__;									\
    }

#else
#define UDPNM_DET_REPORTERROR(...)
#define UDPNM_VALIDATE(...)
#endif

/** @req SWS_UdpNm_00191 */
#define UDPNM_VALIDATE_INIT(serviceID, ...)					\
        UDPNM_VALIDATE((UdpNm_Internal.InitStatus == UDPNM_INIT), serviceID, UDPNM_E_NO_INIT, 0, __VA_ARGS__)

/** @req SWS_UdpNm_00192 */
#define UDPNM_VALIDATE_CHANNEL(channel, serviceID, ...)					\
        UDPNM_VALIDATE( ((channel < UdpNm_ConfigPtr->ChannelMapSize) && ( UdpNm_ConfigPtr->ChannelMap[channel] < UDPNM_CHANNEL_COUNT )), serviceID, UDPNM_E_INVALID_CHANNEL, channel, __VA_ARGS__)

#define UDPNM_VALIDATE_NOTNULL(ptr, serviceID, ...)	\
        UDPNM_VALIDATE( (ptr != NULL), serviceID, UDPNM_E_PARAM_POINTER, 0, __VA_ARGS__)

#define UDPNM_VALIDATE_PDUID(pduid, serviceID, ...)                 \
        UDPNM_VALIDATE( (pduid < UDPNM_CHANNEL_COUNT), serviceID, UDPNM_E_INVALID_CHANNEL, pduid, __VA_ARGS__)


#define UDPNM_CBV_PNI				0x40u
#define UDPNM_LSBIT_MASK			0x1u

typedef enum {
    UDPNM_INIT,
    UDPNM_UNINIT
} UdpNm_InitStatusType;

#if (UDPNM_PNC_COUNT > 0)
typedef struct{
    uint32 							resetTimer;
    boolean 						timerRunning;
} UdpNm_Internal_PnTimerType;
#endif

typedef struct {
    Nm_ModeType						Mode;
    Nm_StateType					State;
    boolean							Requested;
    uint32							TimeoutTimeLeft;
    uint32							RepeatMessageTimeLeft;
    uint32							WaitBusSleepTimeLeft;
    uint32							MessageCycleTimeLeft;
    uint32							MessageTimeoutTimeLeft;
    uint8							TxMessageSdu[8];
    uint8							RxMessageSdu[8];
#if (UDPNM_PNC_COUNT > 0)
    /* @req SWS_UdpNm_00358 */
    /* @req SWS_UdpNm_00371 */
    UdpNm_Internal_PnTimerType		pnERATimers[UDPNM_PNC_COUNT];
    uint8							pnERABytes[UDPNM_PNC_INFO_LEN];
#endif
} UdpNm_Internal_ChannelType;

typedef struct {
    UdpNm_InitStatusType 			InitStatus;
    UdpNm_Internal_ChannelType		Channels[UDPNM_CHANNEL_COUNT];
#if (UDPNM_PNC_COUNT > 0)
    /* @req SWS_UdpNm_00345 */
    /* @req SWS_UdpNm_00372 */
    UdpNm_Internal_PnTimerType		pnEIRATimers[UDPNM_PNC_COUNT];
    uint8							pnEIRABytes[UDPNM_PNC_INFO_LEN];
#endif
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
static inline void UdpNm_Internal_BusSleep_to_BusSleep( const UdpNm_ChannelType* ChannelConf );

static inline void UdpNm_Internal_RepeatMessage_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_RepeatMessage_to_ReadySleep( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_RepeatMessage_to_NormalOperation( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );

static inline void UdpNm_Internal_NormalOperation_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_NormalOperation_to_ReadySleep( UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_NormalOperation_to_NormalOperation( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );

static inline void UdpNm_Internal_ReadySleep_to_NormalOperation( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_ReadySleep_to_PrepareBusSleep( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );
static inline void UdpNm_Internal_ReadySleep_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );

static inline void UdpNm_Internal_NetworkMode_to_NetworkMode( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal );

#if (UDPNM_PNC_COUNT > 0)
/**
 * @brief UdpNm_Internal_UpdateRequestArray A helper function to update the request array.
 * @param updateEira TRUE if EIRA should be updated, FALSE if ERA should be updated.
 * @param calculatedRa The data filtered with the pn filter mask.
 * @param ChannelConf
 * @param ChannelInternal
 */
static void UdpNm_Internal_UpdateRequestArray(boolean updateEira,
        const uint8 *calculatedRa,
        const UdpNm_ChannelType *ChannelConf,
        UdpNm_Internal_ChannelType *ChannelInternal);

/**
 * @brief NM filtering process done for each reception
 * @param ChannelConf - Channel configuration
 * @param ChannelInternal - Channel internal runtime data
 * @param pni - PNI bit set in CBV?
 * @return Reception valid or not
 */
static Std_ReturnType UdpNm_Internal_RxProcess(
        const UdpNm_ChannelType* ChannelConf,
        UdpNm_Internal_ChannelType* ChannelInternal,
        boolean pni );

/**
 * @brief UdpNm_Internal_ProcessTxPdu Pn filtering done for each send
 * @param ChannelConf - Channel configuration
 * @param ChannelInternal - Channel internal runtime data
 * @return void
 */
static void UdpNm_Internal_ProcessTxPdu(const uint8 * const pnInfo,
        const UdpNm_ChannelType* ChannelConf,
        UdpNm_Internal_ChannelType* ChannelInternal);

/**
 * @brief Set the PNI bit in CBV byte of Nm Pdu
 * @param ChannelConf - channel configuration
 * @param ChannelInternal - channel internal runtime data
 * @return void
 */
static void UdpNm_Internal_SetPniCbv(
        const UdpNm_ChannelType* ChannelConf,
        UdpNm_Internal_ChannelType* ChannelInternal);

#if (UDPNM_PNC_EIRA_CALC_ENABLED == STD_ON)
/**
 * @brief UdpNm_Internal_TickPnEiraResetTime Run PN reset timers every main function cycle
 * @param void
 * @return void
 */
static void UdpNm_Internal_TickPnEiraResetTime(uint32 period);
#endif /* (UDPNM_PNC_EIRA_CALC_ENABLED == STD_ON) */

#if (UDPNM_PNC_ERA_CALC_ENABLED == STD_ON)
/**
 * @brief UdpNm_Internal_TickPnEraResetTime Run PN reset timers every main function cycle
 * @param chanIdx
 * @return
 */
static void UdpNm_Internal_TickPnEraResetTime(const uint8 chanIdx);
#endif /* (UDPNM_PNC_ERA_CALC_ENABLED == STD_ON) */
#endif /* (UDPNM_PNC_COUNT > 0) */

#endif /* UDPNM_INTERNAL_H_ */
