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

#ifndef FRNM_INTERNAL_H_
#define FRNM_INTERNAL_H_

//lint -emacro(904,FRNM_DET_REPORTERROR) //904 PC-Lint exception to MISRA 14.7 (validate DET macros).
#define FRNM_CBV_PNI                                       0x40u //CRI bit

typedef enum {
    FRNM_STATUS_UNINIT,
    FRNM_STATUS_INIT
}FrNm_Internal_InitStatusType;

/* @req FRNM425 */
typedef struct{
    uint32 resetTimer;
    boolean timerRunning;
}FrNm_Internal_PnTimerType;

/* Type used for both EIRA and ERA. */
typedef union {
    uint64 data;
    uint8  bytes[sizeof(uint64)];
}FrNm_Internal_RAType;

typedef struct{

    Nm_ModeType  Mode;               /* @req FRNM105 */
    Nm_StateType State;              /* @req FRNM107 */
    uint32       RepeatMessageTimeLeft;
    uint32       MessageTimeoutTimeLeft;
    uint32       syncLossTimerLeft;
    uint16       readySleepCntLeft;
    uint8        FrNmTxVotePdu;/* @req FRNM205 */ /* @req FRNM215 */
    uint8        FrNmTxDataPdu[8];/* @req FRNM205 */ /* @req FRNM006 */
    uint8        FrNmRxDataPdu[8]; /* @req FRNM205 */
    uint8        FrNmTxUserdataMessagePdu[8];
    uint8        RepetitionCyclesLeft;
    uint8        DataCyclesLeft;
    boolean      repetitionCycleCompleted;
    boolean      CommunicationEnabled;
    boolean      activeWakeup;
    boolean      FrNm_NetworkRequested; /* @req FRNM167 */
    boolean      FrNm_RepeatMessage;  /* @req FRNM118 */
    boolean      setUserDataEnable;
}FrNm_Internal_ChannelType;


typedef struct {
#if (FRNM_PNC_COUNT > 0)
    FrNm_Internal_PnTimerType      pnEIRATimers[FRNM_PNC_COUNT];
    FrNm_Internal_RAType           pnEIRA;
#endif
    FrNm_Internal_InitStatusType InitStatus;
    /*lint -e9038 Macro is well defined */
    FrNm_Internal_ChannelType  FrNmChannels[FRNM_NUMBER_OF_CLUSTERS];
}FrNm_InternalType;



#if (FRNM_DEV_ERROR_DETECT == STD_ON)/* @req FRNM022 */ /* @req FRNM049 */
#define FRNM_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        (void)Det_ReportError(FRNM_MODULE_ID, 0, _api, _error); \
        return __VA_ARGS__; \
    }
#else
#define FRNM_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        return __VA_ARGS__; \
    }
#endif

#define FRNM_VALIDATE_CHANNEL(NetworkHandle, _api, ...)                 \
        FRNM_DET_REPORTERROR(((NetworkHandle<COMM_CHANNEL_COUNT)&&(FrNm_ConfigPtr->FrNmChannelLookups[NetworkHandle]!=FRNM_UNUSED_CHANNEL)), _api, FRNM_E_INVALID_CHANNEL,__VA_ARGS__)

#if (FRNM_SOURCE_NODE_INDENTIFIER_ENABLED == STD_OFF)
#define FRNM_INTERNAL_GET_USER_DATA_OFFSET FRNM_USRDATA_WITHOUT_SOURCE_NODE_ID /* @req FRNM381 */
#else
#define FRNM_INTERNAL_GET_USER_DATA_OFFSET FRNM_USRDATA_WITH_SOURCE_NODE_ID /* @req FRNM222 */
#endif
void FrNm_Internal_Synchronize_to_RepeatMessage( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal );
void FrNm_Internal_TickRepeatMessageTime(const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal );
void FrNm_internal_CheckRepetitionCycles(const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal,uint8 CurrentFlexrayCycle);
void FrNm_Internal_Globaltime_Error_Handling( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal ) ;
#if (FRNM_PASSIVE_MODE_ENABLED == STD_OFF)
//inline void FrNm_Internal_NormalOperation_to_RepeatMessage( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal );
//inline void FrNm_Internal_NormalOperation_to_ReadySleep( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal );
Std_ReturnType FrNm_Internal_TransmitMessage( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal );
Std_ReturnType FrNm_Internal_Transmit_Nmpdudata(const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal,PduInfoType* dataPduPtr, PduIdType* dataPduId);
Std_ReturnType FrNm_Internal_Get_pduVote_ForTx( const FrNm_ChannelInfoType* ChannelConf, PduIdType* dataPduId );
Std_ReturnType FrNm_Internal_Get_Userdata_ForTx( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal,const PduInfoType* dataPduPtr );
Std_ReturnType FrNm_Internal_Get_pdudataPtr_ForTx( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal, PduInfoType* dataPduPtr,PduIdType* dataPduId );
void FrNm_Internal_TickTxTimeout( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal );
//inline void FrNm_Internal_ReadySleep_to_RepeatMessage( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal );

#if (FRNM_HW_VOTE_ENABLE == STD_ON)
void FrNm_Internal_Hardware_VectorData_Handling( const uint8* HwvectorInfoPtr, const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal );
#endif
#endif /* FRNM_PASSIVE_MODE_ENABLED */

void FrNm_Internal_Resetvotedata(FrNm_Internal_ChannelType* ChannelInternal);
void FrNm_Internal_ReadySleepState_Handling(const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal);

#if  ((FRNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (FRNM_PNC_COUNT > 0))
void FrNm_Internal_RxProcess(FrNm_Internal_ChannelType* ChannelInternal );
void FrNm_Internal_TickPnEIRAResetTime(uint32 mainFuncPeriod);
void FrNm_Internal_resetEIRAPNbits(uint8 *pnIndex, uint8 indexCount);
void FrNm_Internal_ProcessTxPdu(uint8 *pnInfo);
#endif




#if (FRNM_PASSIVE_MODE_ENABLED == STD_OFF)

/**
 * @brief Transit from Normal operation state to Repeat message state in Network mode
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal - Channel Internal runtime data ptr
 * @return void
 */
static inline void FrNm_Internal_NormalOperation_to_RepeatMessage( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal ) {
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
    /* @req FRNM117*/
    ChannelInternal->RepeatMessageTimeLeft = ChannelConf->FrNmTimingConfig->FrNmRepeatMessageTime;
#if (FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON) /* @req FRNM106 */
    Nm_StateChangeNotification(ChannelConf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef, NM_STATE_NORMAL_OPERATION, NM_STATE_REPEAT_MESSAGE);
#endif
}

/**
 * @brief Transition from Normal operation state to Ready sleep in Network mode
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal - Channel Internal runtime data ptr
 * @return void
 */
static inline void FrNm_Internal_NormalOperation_to_ReadySleep( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal ) {
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_READY_SLEEP;
    FrNm_Internal_Resetvotedata(ChannelInternal);
    /* @req FRNM127 */
    ChannelInternal->readySleepCntLeft = ChannelConf->FrNmTimingConfig->FrNmReadySleepCnt;
    ChannelInternal->MessageTimeoutTimeLeft = 0 ;
#if (FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON) /* @req FRNM106 */
    Nm_StateChangeNotification(ChannelConf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef, NM_STATE_NORMAL_OPERATION, NM_STATE_READY_SLEEP);
#endif
}

/**
 * @brief Transition from Ready sleep state to Repeat message state in Network mode
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal - Channel Internal runtime data ptr
 * @return void
 */
static inline void FrNm_Internal_ReadySleep_to_RepeatMessage( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal ) {
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
    /* @req FRNM117*/
    ChannelInternal->RepeatMessageTimeLeft = ChannelConf->FrNmTimingConfig->FrNmRepeatMessageTime;
    ChannelInternal->MessageTimeoutTimeLeft = ChannelConf->FrNmTimingConfig->FrNmMsgTimeoutTime ;
#if (FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON) /* @req FRNM106 */
    Nm_StateChangeNotification(ChannelConf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef, NM_STATE_READY_SLEEP,  NM_STATE_REPEAT_MESSAGE);
#endif
}





#endif



#endif /* FRNM_INTERNAL_H_ */
