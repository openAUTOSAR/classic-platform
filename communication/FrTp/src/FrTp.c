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
/* @req FRTP200 */ /* Each code (*.c) file of the FrTp module shall provide data of version identification as defined in chapter*/
/* @req FRTP141 */ /* PduInfoType is imported by including FrTp.h which includes FrTp_Types.h which includes ComStack_Types.h */
/* @req FRTP1004 */ /* header file structure of the FrTp module */
/* @req FRTP209 */ /* The source code of the FrTp module shall be neither compiler (tool) nor platform (processor) dependent*/
/* @req FRTP1005 */ /* @req FRTP1006*/ /*If no explicit requirement is defined shall follow the specification ISO10681-2*/
/* @req FRTP1033 */ /*  FrTp specific types shall be defined in FrTp_Types.h */
/* @req FRTP1007*/ /* @req FRTP1008*/ /* @req FRTP1009*/ /* @req FRTP1010*/ /* @req FRTP1011*/ /* @req FRTP1012*/
/* @req FRTP1164*/ /* @req FRTP1166*/  /* PduInfoType is imported by including FrTp.h which includes FrTp_Types.h which includes ComStack_Types.h */
/* @req FRTP1168*/ /* @req FRTP1165*/ /* @req FRTP1167*/ /* @req FRTP1169 */ /* @req FRTP1170 */ /* @req FRTP1178 */ /* PduInfoType is imported by including FrTp.h which includes FrTp_Types.h*/
/* @req FRTP1055 */ /* Timing behaviour implemented as per requirement */
/* @req FRTP1020 */  /* Based on configuration this can be achived */
/* @req FRTP569 */ /* Configuration parameter genration is done through .xpt file*/
/* @req FRTP1158 */ /* Each header (*.h) file of the FrTp module shall provide data of version identification as defined in chapter*/
/* @req FRTP1133 */ /* FrTp specific types shall be defined in FrTp_Types.h */
/* @req FRTP1109 */ /* MODULE_ID set to 0x24 in FrTp.h */


#include "FrTp.h"
#include "Det.h"
#include "FrIf.h"
#include "SchM_FrTp.h"
#include "PduR_FrTp.h"
#include <string.h>
#include "debug.h"
#include "MemMap.h"
#include "FrTp_Internal.h"

//lint -emacro(904,FRTP_DET_REPORTERROR) //904 PC-Lint exception to MISRA 14.7 (validate DET macros).

#if (FRTP_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif

#if defined(USE_DEM)
#include "Dem.h"
#endif

#if !(((FRTP_SW_MAJOR_VERSION == 1) && (FRTP_SW_MINOR_VERSION == 0)) )
#error FrTp: Configuration file expected BSW module version to be 1.0.*
#endif
#if !(((FRTP_AR_RELEASE_MAJOR_VERSION == 4) && (FRTP_AR_RELEASE_MINOR_VERSION == 0)) )
#error FrTp: Configuration file expected AUTOSAR version to be 4.0.*
#endif

/*@req FRTP1037*/ /*@req FRTP206*/ /*@req FRTP1107*/ /*@req FRTP1108*/

#if (FRTP_DEV_ERROR_DETECT == STD_ON)
#define FRTP_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        (void)Det_ReportError(FRTP_MODULE_ID, 0, _api, _error); \
        return __VA_ARGS__; \
    }
#else
#define FRTP_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        return __VA_ARGS__; \
    }
#endif

/* Configuration of FrTp channel */
static const FrTp_ConfigType* FrTp_ConfigPtr = NULL;

/* Internal run time data */
/* @req FRTP1030*/
static FrTp_Internal_RunTimeType FrTp_Internal_RunTime = {
        .FrTpState = FRTP_OFF,
};



/* Internal function declarations  */

static void FrTp_Internal_TransmitStartFrame(FrTp_Internal_Channel_type* runtimeChnlData);
static void FrTp_Internal_WaitForFlowControl(FrTp_Internal_Channel_type* runtimeChnlData);
static Std_ReturnType FrTp_Internal_PrepareAndSendStartFrame(FrTp_Internal_Channel_type* runtimeChnlData, uint8 framePayloadLength);
static Std_ReturnType FrTp_Internal_SendFrame(FrTp_Internal_Channel_type* runtimeChnlData, PduInfoType* txPduInfo,FrtpCommType commState);
static Std_ReturnType FrTp_Internal_GetConnectionForTxSdu(const PduIdType frtpTxSduId, uint8 *connectionIndex);
static void FrTp_Internal_AssignChannelForTx(const uint8 connectionIndex,const uint8 chnlIndex,FrTp_Internal_Channel_type **runtimeChnlDataAssigned,const PduInfoType* FrTpTxSduInfoPtr);
static void FrTp_Internal_TxBufferRequestError(FrTp_Internal_Channel_type* runtimeChnlData,BufReq_ReturnType BufReqResult );
static void FrTp_Internal_FreeChannel(FrTp_Internal_Channel_type* runtimeChnlData);
static void FrTp_Internal_TransmitFinished(FrTp_Internal_Channel_type* runtimeChnlData);
static void FrTp_Internal_TransmitStatesHandle(FrTp_Internal_Channel_type* runtimeChnlData);
static void FrTp_Internal_TimeoutAs(FrTp_Internal_Channel_type* runtimeChnlData);
static void handelflowcontrolframe(FrTp_Internal_Channel_type *ChannelInternal,PduInfoType* PduInfoPtr);
static Std_ReturnType FrTp_Internal_GetNextFrameType(FrTp_Internal_Channel_type* runtimeChnlData,uint8 *txFrameType,PduLengthType *remainingTxBufferLength);
static PduLengthType FrTp_Internal_FillNextFrame(FrTp_Internal_Channel_type* runtimeChnlData, uint8 txFrameType);
static void FrTp_Internal_SendNextFrame(FrTp_Internal_Channel_type* runtimeChnlData,PduLengthType payloadIndex );
static void FrTp_Internal_HandleFrIfError(FrTp_Internal_Channel_type* runtimeChnlData,FrtpCommType commState);
static void FrTp_Internal_HandleTxCanceled(FrTp_Internal_Channel_type* runtimeChnlData);
static void FrTp_Internal_HandleRecvdFCAck(FrTp_Internal_Channel_type *ChannelInternal,PduInfoType* PduInfoPtr );
static void FrTp_Internal_HandleNextSegFrames(FrTp_Internal_Channel_type* runtimeChnlData);

static ISO10681FrameType getFrameType(const uint8 *sdudataPtr);
static boolean validateConnectionType(PduInfoType* PduInfoPtr,ISO10681FrameType frame,uint8 connection_count);
static void resetRxChannelConfiguration(FrTp_Internal_Channel_type *runtimeInternalChnllData);
static void getFrameAddress(const PduInfoType *PduInfoPtr, ISO10681PduAddressType *frameAddressInfo);
static void sendFlowControlFrame(FrTp_Internal_Channel_type *ChannelInternal, uint8 flowstatus, boolean Retry_flag);
static BufReq_ReturnType copySegmentToPduRRxBuffer(BufReq_ReturnType retun_value,FrTp_Internal_Channel_type *ChannelInternal);
static void handlePdurReturn(BufReq_ReturnType retun_value,FrTp_Internal_Channel_type *ChannelInternal,boolean stf_ack_flag);
static void handleStartFrame(FrTp_Internal_Channel_type *ChannelInternal,PduInfoType *PduInfoPtr);
static boolean handelSequenceError(FrTp_Internal_Channel_type *ChannelInternal);
static void handelconsecutiveframes(FrTp_Internal_Channel_type *ChannelInternal,ISO10681FrameType frameType);
static void handelLastFrame(FrTp_Internal_Channel_type *ChannelInternal);
static void handelpduRwaitcalls(FrTp_Internal_Channel_type *ChannelInternal);
static uint8 frtpManageStfConnections(PduInfoType* PduInfoPtr,PduIdType RxPduId,ISO10681FrameType frameTypelcl,ISO10681PduAddressType frameAddressInfo);
static void frtpManageCfConnections(PduInfoType* PduInfoPtr, ISO10681FrameType frameTypelcl, ISO10681PduAddressType frameAddressInfo);
static void frtpManageFcConnections(PduInfoType* PduInfoPtr, ISO10681PduAddressType frameAddressInfo);
static void handleReceivedFrame(FrTp_Internal_Channel_type *ChannelInternal);
static uint8 frtpRxStfAssignConnections(PduInfoType* PduInfoPtr,uint8 connection_count,ISO10681FrameType frameTypelcl);
static void handelCrTimeout(FrTp_Internal_Channel_type *ChannelInternal);
static void handelArTimeout(FrTp_Internal_Channel_type *ChannelInternal);
#if(STD_ON == FRTP_ACK_RETRY_SUPPORT)
static void updateBlockPosition(FrTp_Internal_Channel_type *ChannelInternal);
#endif


/**
 *
 * @param version info
 */
/* This service returns the version information of this module. */
/* @req FRTP215 */
#if ( FRTP_VERSION_INFO_API == STD_ON ) /* @req FRTP498 */
void FrTp_GetVersionInfo(Std_VersionInfoType* versioninfo){

    /* @req FRTP1150*/
    FRTP_DET_REPORTERROR((NULL != versioninfo),FRTP_SERVICE_ID_GET_VERSION_INFO,FRTP_E_NULL_PTR);
    /* @req FRTP202*/
    STD_GET_VERSION_INFO(versioninfo, FRTP);
    return;
}
#endif /* FRTP_VERSION_INFO_API */

/**
 *
 * @param ConfigPtr
 */
/* This service initializes all global variables of a FlexRay Transport Layer instance
and set it in the idle state */
/* @req FRTP147 */
void FrTp_Init( const FrTp_ConfigType* ConfigPtr) {
    FrTp_Internal_Channel_type* runtimeInternalChnllData;
    uint8 channel;
    uint8 conIndex;
    uint8 pduIndex;
    uint8 initCount;
    FRTP_DET_REPORTERROR((NULL != ConfigPtr),FRTP_SERVICE_ID_INIT,FRTP_E_NULL_PTR);     /* @req FRTP1151 */
    if(FrTp_Internal_RunTime.FrTpState == FRTP_ON){
        /* This a case of re-initialization and the state is set to off to disable any concurrent frtp APIs*/
           FrTp_Internal_RunTime.FrTpState = FRTP_OFF;/* @req FRTP1030*/
#if(FRTP_TRANSMIT_CANCEL_SUPPORT == STD_ON)
        /* Processing the channels previously sent TxPdus got TxConfirmation */
        for (channel = 0; channel < FRTP_CHANNEL_NUM; channel++) {
            runtimeInternalChnllData = &FrTp_Internal_RunTime.FrTpChannels[channel];
            if ((runtimeInternalChnllData->FrTpTxChannelState == FRTP_TX_SEG_ONGOING)||(runtimeInternalChnllData->FrTpTxChannelState == FRTP_WAIT_FOR_TRIGGER_TRANSMIT)){
                /* Cancel the ongoing transmission */
                if(runtimeInternalChnllData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef != NULL_PTR){
                    (void) FrIf_CancelTransmit(runtimeInternalChnllData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef->FrTpFrIfTxPduId);/* @req FRTP1120 */
                }
                PduR_FrTpTxConfirmation(runtimeInternalChnllData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId,NTFRSLT_E_CANCELATION_OK);
                FrTp_Internal_FreeChannel(runtimeInternalChnllData);
            }
        }
#endif
    }
    FrTp_ConfigPtr = ConfigPtr;
    /* @req FRTP1035*/ /* @req FRTP1034*/
    for(channel =0;channel<FRTP_CHANNEL_NUM;channel++ ) {
        runtimeInternalChnllData = &FrTp_Internal_RunTime.FrTpChannels[channel];
        FrTp_Internal_FreeChannel(runtimeInternalChnllData);
#if FRTP_FULL_DUPLEX_ENABLE == STD_ON
        resetRxChannelConfiguration(runtimeInternalChnllData);
#endif
        runtimeInternalChnllData->FrTpRxPduAvailable = FALSE;
        runtimeInternalChnllData->FrTpTxSduAvailable = FALSE;
        runtimeInternalChnllData->FrTpTransmitPduLength = 0;
        runtimeInternalChnllData->FrTpRetryInfo.TpDataState = TP_CONFPENDING;
        runtimeInternalChnllData->FrTpRetryInfo.TxTpDataCnt = 0;
        runtimeInternalChnllData->FrTpFramePayloadLength=0;
        runtimeInternalChnllData->FrTpRxChannelState = IDLE;
        runtimeInternalChnllData->FrTpTxChannelState = IDLE;
        runtimeInternalChnllData->FrTpUlReturnValue = BUFREQ_OK;
        runtimeInternalChnllData->FrTpRxSizeBuffer = 0;
        runtimeInternalChnllData->FrTpFcRecveBuffSize = 0;
        runtimeInternalChnllData->FrTpCounterRxRn = 0;
        runtimeInternalChnllData->FrTpBandwidthcontol = 0;
        runtimeInternalChnllData->FrTpTxSduUnknownMsgLength = 0;
        runtimeInternalChnllData->FrTpTcRequest = FALSE;
        for(initCount=0;initCount<FLOW_CONTROL_FRAME_PAYLOAD;initCount++){
            runtimeInternalChnllData->FrTpRxFcDataPdu[initCount] = 0;
        }
#if FRTP_ACK_RETRY_SUPPORT == STD_ON
        runtimeInternalChnllData->FrTpAckPending = FRTP_NO;
        runtimeInternalChnllData->FrTpuseCF1 = TRUE;
        runtimeInternalChnllData->FrTpChannelParams.FrTpTxChannel.FrTpTxBlockPosition = 0;
        runtimeInternalChnllData->FrTpStfAckFlag = FALSE;
        runtimeInternalChnllData->FrTpRxFcRetryFlag = FALSE;
        runtimeInternalChnllData->FrTpStfReceivedFlag = FALSE;
        runtimeInternalChnllData->FrTpEobReceivedFlag = FALSE;
#endif
      }
    for(conIndex =0;conIndex < FrTp_ConfigPtr->FrTpConnectionCount;conIndex++ ){
        FrTp_Internal_RunTime.FrTpConnectionLockStatus[conIndex]= NOT_ACTIVE;  /* To initialize connection locked by a rx or tx channel to false for half duplex*/
     }
    for(pduIndex =0;pduIndex < FrTp_ConfigPtr->FrTpTxPduCount;pduIndex++ ){
        FrTp_Internal_RunTime.FrTpTxPduStatusInfo[pduIndex].FrTpTxPduLockStatus= AVAILABLE;    /* To initialize tx pdu locked by a channel for transmission to false*/
     }
    FrTp_Internal_RunTime.FrTpState = FRTP_ON;/* @req FRTP1032*/
}
/**
 * @param void
 */
/*This service closes all pending transport protocol connections by simply stopping
operation, frees all resources and stops the FrTp Module */
/* @req FRTP148 */
void FrTp_Shutdown(void) {

 uint8 channelIndex;
 FrTp_Internal_Channel_type* chnlInternal;

    /* @req FRTP1037 */
    FRTP_DET_REPORTERROR((FRTP_ON == FrTp_Internal_RunTime.FrTpState),FRTP_SERVICE_ID_SHUTDOWN,FRTP_E_UNINIT);

    /*@req FRTP1036*/
    FrTp_Internal_RunTime.FrTpState = FRTP_OFF;


#if(FRTP_TRANSMIT_CANCEL_SUPPORT == STD_ON)
        for (uint8 i=0; i < FRTP_MAX_TXPDU_COUNT; i++){
                /* Cancel the ongoing transmission */
            /* FrTpTxPduLockStatus is not NOT_ACTIVE, it means it is awaiting TX confirmation i.e a possibility to cancel prospective transmission */
                if(FrTp_Internal_RunTime.FrTpTxPduStatusInfo[i].FrTpTxPduLockStatus != AVAILABLE)
                {
                    /* @req FRTP1120 */
                    (void) FrIf_CancelTransmit(FrTp_ConfigPtr->FrTpPduIdToFrIfPduIdMap[i]);
                }
           }
#endif
    /* Processing the channels previously sent TxPdus got TxConfirmation */
    for (channelIndex = 0; channelIndex < FRTP_CHANNEL_NUM; channelIndex++) {
        chnlInternal = &FrTp_Internal_RunTime.FrTpChannels[channelIndex];

        if (chnlInternal->FrTpTxChannelState != IDLE)
        {
#if(FRTP_TRANSMIT_CANCEL_SUPPORT == STD_ON)
            PduR_FrTpTxConfirmation(chnlInternal->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId,NTFRSLT_E_CANCELATION_OK);
#endif
            FrTp_Internal_FreeChannel(chnlInternal);
        }
        if (chnlInternal->FrTpRxChannelState != IDLE)
        {
            PduR_FrTpRxIndication(chnlInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpULRxPduId,NTFRSLT_E_CANCELATION_OK);
            resetRxChannelConfiguration(chnlInternal);
        }
    }


}

/**
 *
 * @param FrTpTxSduId
 * @param FrTpTxSduInfoPtr
 * @return
 */
/*This service is utilized to request the transfer of data*/
/* @req FRTP149 */
Std_ReturnType FrTp_Transmit(PduIdType FrTpTxSduId, const PduInfoType* FrTpTxSduInfoPtr){

    /* @req FRTP136 */
    Std_ReturnType status;
    FrTp_Internal_Channel_type* runtimeInternalChnlData;
    uint8 connectionIndex;
    uint8 txPduIndex;
    uint8 chnlIndex;
    runtimeInternalChnlData = NULL;
    FRTP_DET_REPORTERROR((FRTP_ON == FrTp_Internal_RunTime.FrTpState),FRTP_SERVICE_ID_TRANSMIT,FRTP_E_UNINIT,E_NOT_OK);     /* @req FRTP1037 */
    FRTP_DET_REPORTERROR((NULL != FrTpTxSduInfoPtr),FRTP_SERVICE_ID_TRANSMIT,FRTP_E_NULL_PTR,E_NOT_OK); /* @req FRTP1140 */
    FRTP_DET_REPORTERROR((FrTpTxSduId <= FrTp_ConfigPtr->FrTpTxSduCount),FRTP_SERVICE_ID_TRANSMIT,FRTP_E_INVALID_PDU_SDU_ID,E_NOT_OK);     /* @req FRTP1139 */
    /* Get the connection based on the TxsduId */
    status = FrTp_Internal_GetConnectionForTxSdu(FrTpTxSduId, &connectionIndex);
    /* If connection not found */
    FRTP_DET_REPORTERROR((status != E_NOT_OK),FRTP_SERVICE_ID_TRANSMIT,FRTP_E_INVALID_PDU_SDU_ID,E_NOT_OK);    /* @req FRTP1040 */
#if (FRTP_UNKOWN_MSG_LNGT == STD_OFF)
    /*For 1:n connections the unknown message length is not supported */
    /*  @req FRTP1063 */ /*  @req FRTP1187 */
    FRTP_DET_REPORTERROR(!((FrTpTxSduInfoPtr->SduLength == 0)&& (FrTp_ConfigPtr->FrTpConnectionConfig[connectionIndex].FrTpMultipleReceiverCon!= FALSE)),FRTP_SERVICE_ID_TRANSMIT,FRTP_E_SEG_ERROR,E_NOT_OK);
    /* @req FRTP1062 */
    /* Report error if unknown message length is not supported */
    /* @req FRTP1134 */ /* @req FRTP1043 */ /* @req FRTP1064 */
    FRTP_DET_REPORTERROR((FrTpTxSduInfoPtr->SduLength!=0),FRTP_SERVICE_ID_TRANSMIT,FRTP_E_UMSG_LENGTH_ERROR,E_NOT_OK);
#endif
    FRTP_DET_REPORTERROR(!((FrTpTxSduInfoPtr->SduLength > FRTP_MAX_USEG_UL_FPL)&&(FrTp_ConfigPtr->FrTpConnectionConfig[connectionIndex].FrTpMultipleReceiverCon!= FALSE)),FRTP_SERVICE_ID_TRANSMIT,FRTP_E_SEG_ERROR,E_NOT_OK);
    if(FrTp_ConfigPtr->FrTpConnectionConfig[connectionIndex].FrTpMultipleReceiverCon == TRUE){
        for(txPduIndex=0;txPduIndex<FrTp_ConfigPtr->FrTpConnectionConfig[connectionIndex].FrTpPoolTxPduCnt;txPduIndex++){
            if(FrTpTxSduInfoPtr->SduLength < (FrTp_ConfigPtr->FrTpConnectionConfig[connectionIndex].FrTpTxPduPoolRef[txPduIndex].FrTpTxPduLength-FRTP_HEADER_LENGTH_STF_LF)){
                break;
            }
        }
      FRTP_DET_REPORTERROR((txPduIndex < FrTp_ConfigPtr->FrTpConnectionConfig[connectionIndex].FrTpPoolTxPduCnt),FRTP_SERVICE_ID_TRANSMIT,FRTP_E_SEG_ERROR,E_NOT_OK);
     }
    if( FrTp_Internal_RunTime.FrTpConnectionLockStatus[connectionIndex]==ACTIVE_RX){
#if(FRTP_FULL_DUPLEX_ENABLE == STD_ON)
           for(chnlIndex=0;chnlIndex <FRTP_CHANNEL_NUM; chnlIndex++){
             if(FrTp_Internal_RunTime.FrTpChannels[chnlIndex].FrTpChannelParams.FrTpTxChannel.FrTpAssignedConnectionIndex == connectionIndex){
                if(FrTp_Internal_RunTime.FrTpChannels[chnlIndex].FrTpTxChannelState==IDLE){
                  FrTp_Internal_AssignChannelForTx(connectionIndex,chnlIndex,&runtimeInternalChnlData,FrTpTxSduInfoPtr);
                 }
               break;
              }
            }
          /* If there isn't any free channel */
          /* @req FRTP1041 */ /* @req FRTP1185 */
          FRTP_DET_REPORTERROR((runtimeInternalChnlData != NULL_PTR),FRTP_SERVICE_ID_TRANSMIT,FRTP_E_NO_CHANNEL,E_NOT_OK);
#endif
    }else if(FrTp_Internal_RunTime.FrTpConnectionLockStatus[connectionIndex]==ACTIVE_TX){
     /* @req FRTP142 */
     PduR_FrTpTxConfirmation(FrTp_ConfigPtr->FrTpConnectionConfig[connectionIndex].FrTpULTxPduId, NTFRSLT_E_FR_TX_ON);
     status = E_NOT_OK;
    }else{
       /* If there is a free channel */ /* @req FRTP228 */
       for(chnlIndex=0;chnlIndex <FRTP_CHANNEL_NUM; chnlIndex++){
           if(FrTp_Internal_RunTime.FrTpChannels[chnlIndex].FrTpTxChannelState == IDLE) {
                /* Allocate channel  for the connection */
                FrTp_Internal_AssignChannelForTx(connectionIndex,chnlIndex,&runtimeInternalChnlData,FrTpTxSduInfoPtr);
                break;
           }
       }
       /* If there isn't any free channel */
       /* @req FRTP1041 */ /* @req FRTP1185 */
       FRTP_DET_REPORTERROR((runtimeInternalChnlData != NULL_PTR),FRTP_SERVICE_ID_TRANSMIT,FRTP_E_NO_CHANNEL,E_NOT_OK);

     }
   return status;

}


/**
 *
 * @param FrTpTxPduId
 * @return
 */
/*This service primitive is used to cancel the transfer of pending Fr N-SDUs.*/
/* @req FRTP150 */
Std_ReturnType FrTp_CancelTransmit(PduIdType FrTpTxSduId) {

    /*@req FRTP384 */    /* @req FRTP1097 */
#if (FRTP_TRANSMIT_CANCEL_SUPPORT == STD_ON)
    FrTp_Internal_Channel_type* runtimeInternalChnllData;
    uint8 chnlIndex;
#endif
    Std_ReturnType status;
    uint8 connectionIndex;

    /* @req FRTP1037 *//*@req FRTP1106*/ /*@req FRTP206*/ /*@req FRTP1107*/ /*@req FRTP205*/
    FRTP_DET_REPORTERROR((FRTP_ON == FrTp_Internal_RunTime.FrTpState),FRTP_SERVICE_ID_CANCEL_TRANSMIT_REQUEST,FRTP_E_UNINIT,E_NOT_OK);
    /* @req FRTP1141 */
    FRTP_DET_REPORTERROR((FrTpTxSduId < FrTp_ConfigPtr->FrTpTxSduCount),FRTP_SERVICE_ID_CANCEL_TRANSMIT_REQUEST,FRTP_E_INVALID_PDU_SDU_ID,E_NOT_OK);

    /* Get the connection based on the TxsduId */
     status = FrTp_Internal_GetConnectionForTxSdu(FrTpTxSduId, &connectionIndex);

    /* If connection not found */
    FRTP_DET_REPORTERROR((status != E_NOT_OK),FRTP_SERVICE_ID_CANCEL_TRANSMIT_REQUEST,FRTP_E_INVALID_PDU_SDU_ID,E_NOT_OK);

#if (FRTP_TRANSMIT_CANCEL_SUPPORT == STD_ON)
    runtimeInternalChnllData = NULL_PTR;
    for(chnlIndex=0;chnlIndex <FRTP_CHANNEL_NUM; chnlIndex++){
        if((FrTp_Internal_RunTime.FrTpChannels[chnlIndex].FrTpChannelParams.FrTpTxChannel.FrTpAssignedConnectionIndex == connectionIndex)&&
             (FrTp_Internal_RunTime.FrTpChannels[chnlIndex].FrTpTxChannelState != IDLE)){
                       runtimeInternalChnllData =&FrTp_Internal_RunTime.FrTpChannels[chnlIndex];
                      break;
          }
     }

    if((chnlIndex<FRTP_CHANNEL_NUM)&&(runtimeInternalChnllData!= NULL_PTR)){
        /* If the first frame is not sent it can be canceled in the states where FrTp is before calling FrIf_Transmit and receiving FrTp_TriggerTransmit */
        /* @req FRTP1104 */
        if(runtimeInternalChnllData->FrTpTxChannelState == FRTP_TX_STARTED) {
            /* @req FRTP385 */ /* @req FRTP423*/
            runtimeInternalChnllData->FrTpTcRequest = TRUE;
        }
        else if ((runtimeInternalChnllData->FrTpTxChannelState == FRTP_TX_SEG_ONGOING)||(runtimeInternalChnllData->FrTpTxChannelState == FRTP_WAIT_FOR_TRIGGER_TRANSMIT))
        {
            runtimeInternalChnllData->FrTpTcRequest = TRUE;
            /* Try to cancel ongoing transmission */
            if(runtimeInternalChnllData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef != NULL_PTR)
            {
                (void) FrIf_CancelTransmit(runtimeInternalChnllData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef->FrTpFrIfTxPduId);
            }

        }else{
            status = E_NOT_OK;
        }

    }else{
        /* No transmit request to cancel */
        status = E_NOT_OK;
    }
#endif
    return status;

}

/**
 *
 * @param id
 * @param parameter
 * @param value
 * @return
 */
/*Request to change transport protocol parameter BandwithControl.*/
/* !req FRTP151 */
Std_ReturnType FrTp_ChangeParameter( PduIdType id, TPParameterType parameter, uint16 value){

    Std_ReturnType status;
    status = E_NOT_OK;
    /* req FRTP1037 */
    FRTP_DET_REPORTERROR((FRTP_ON == FrTp_Internal_RunTime.FrTpState),FRTP_SERVICE_ID_CHANGE_PARAMETER,FRTP_E_UNINIT,E_NOT_OK);

    /* !req FRTP1143 */
  /*  FRTP_DET_REPORTERROR((id <= FrTp_ConfigPtr->FrTpRxPduCount),FRTP_SERVICE_ID_CHANGE_PARAMETER,FRTP_E_INVALID_PDU_SDU_ID,E_NOT_OK);*/
    /* !req FRTP1144 */
   /* FRTP_DET_REPORTERROR((parameter <= TP_BC),FRTP_SERVICE_ID_CHANGE_PARAMETER,FRTP_E_INVALID_PARAMETER,E_NOT_OK);*/
    (void)parameter;
    (void)value;
    (void)id;

    return status;
}
/*lint -save -e9033 Typecasting is done from lower size to upper size uint8 to uint16 */
/**
 *
 * @param FrTpRxSduId
 * @return
 */
/*this API with the corresponding RxSduId the currently ongoing data
reception is terminated immediately */
/* @req FRTP1172 */
Std_ReturnType FrTp_CancelReceive(PduIdType FrTpRxSduId) {

    uint8 connectionCount;
   uint8 channel;
   Std_ReturnType ret;
   FrTp_Internal_Channel_type *ChannelInternal;

     ret = E_OK;
    /* @req FRTP1037 */
    FRTP_DET_REPORTERROR((FRTP_ON == FrTp_Internal_RunTime.FrTpState),FRTP_SERVICE_ID_CANCEL_RECEIVE,FRTP_E_UNINIT,E_NOT_OK);
    /* @req FRTP1180 */
    FRTP_DET_REPORTERROR((FrTpRxSduId < FrTp_ConfigPtr->FrTpRxSduCount),FRTP_SERVICE_ID_CANCEL_RECEIVE,FRTP_E_INVALID_PDU_SDU_ID,E_NOT_OK);

    for(connectionCount= 0; (connectionCount < FrTp_ConfigPtr->FrTpConnectionCount ) ;connectionCount++)
    {
        if(FrTpRxSduId == FrTp_ConfigPtr->FrTpConnectionConfig[connectionCount].FrTpRxSduId){
            break;
        }
    }

    for(channel=0;channel<FRTP_CHANNEL_NUM;channel++)
    {
        if(connectionCount == FrTp_Internal_RunTime.FrTpChannels[channel].FrTpChannelParams.FrTpRxChannel.FrTpAssignedRxConnectionIndex){
            break;
        }
    }

    if(channel<FRTP_CHANNEL_NUM){
        ChannelInternal= &FrTp_Internal_RunTime.FrTpChannels[channel];

        if((FRTP_STF_RECEIVED == ChannelInternal->FrTpRxChannelState ) ||(FRTP_RX_WAIT_STF_SDU_BUFFER == ChannelInternal->FrTpRxChannelState))
        {
            if(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxMessageLength == (PduLengthType) ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[FPL_BYTE]){
                ret = E_NOT_OK;  /* @req FRTP1182*/ /* if receiving unsegmented msg */
            }
        }
        if(RX_LAST_FRAME == ChannelInternal->FrTpRxChannelState ){
            ret = E_NOT_OK;  /* @req FRTP1182*/ /* if receiving LAST Frame */
        }
        if(ret != E_NOT_OK ){
            PduR_FrTpRxIndication(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpULRxPduId,NTFRSLT_E_CANCELATION_OK); /* @req FRTP1183*/
            resetRxChannelConfiguration(ChannelInternal);  /* @req FRTP1181*/
        }
    }else{
        ret = E_NOT_OK;   /* Execution should ideally not reach this part */
    }
    /* */

    return ret;
}



/**
 *
 * @param TxPduId
 * @param PduInfoPtr
 * @return
 */
/* @req FRTP154 */
/*lint --e{818} */
Std_ReturnType FrTp_TriggerTransmit(PduIdType TxPduId,PduInfoType* PduInfoPtr){

    /*@req FRTP1059 */
    FrTp_Internal_Channel_type* runtimeInternalChnllData;
    uint16 channelIndex;
    PduLengthType buffDiffbytes;
    PduLengthType txLen;
    boolean flag;
    Std_ReturnType status;
    txLen =0;
    buffDiffbytes =0;
    status = E_OK;
    /* @req FRTP1037 */
    FRTP_DET_REPORTERROR((FRTP_ON == FrTp_Internal_RunTime.FrTpState),FRTP_SERVICE_ID_TRIGGER_TRANSMIT,FRTP_E_UNINIT,E_NOT_OK);
    /* @req FRTP1146 */
    FRTP_DET_REPORTERROR((NULL != PduInfoPtr),FRTP_SERVICE_ID_TRIGGER_TRANSMIT,FRTP_E_NULL_PTR,E_NOT_OK);
    FRTP_DET_REPORTERROR((NULL != PduInfoPtr->SduDataPtr),FRTP_SERVICE_ID_TRIGGER_TRANSMIT,FRTP_E_NULL_PTR,E_NOT_OK);

    /* @req FRTP1145 */
    flag = (TxPduId < FrTp_ConfigPtr->FrTpTxPduCount)&&( FrTp_Internal_RunTime.FrTpTxPduStatusInfo[TxPduId].FrTpTxPduLockStatus != AVAILABLE);
    FRTP_DET_REPORTERROR((TRUE == flag),FRTP_SERVICE_ID_TRIGGER_TRANSMIT,FRTP_E_INVALID_PDU_SDU_ID,E_NOT_OK);

    channelIndex = FrTp_Internal_RunTime.FrTpTxPduStatusInfo[TxPduId].FrTpChannleHandle;
    runtimeInternalChnllData = &FrTp_Internal_RunTime.FrTpChannels[channelIndex];

    if(runtimeInternalChnllData->FrTpTcRequest == FALSE){

        if( FrTp_Internal_RunTime.FrTpTxPduStatusInfo[TxPduId].FrTpTxPduLockStatus == LOCKED_FOR_SF_CF_LF){

          /* @req FRTP1060 */ /* @req FRTP1061 */
          if(runtimeInternalChnllData->FrTpTxChannelState == FRTP_WAIT_FOR_TRIGGER_TRANSMIT){
              memcpy(PduInfoPtr->SduDataPtr,runtimeInternalChnllData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu,runtimeInternalChnllData->FrTpTransmitPduLength);
              /* Copying back Orignal state */
              runtimeInternalChnllData->FrTpTxChannelState = runtimeInternalChnllData->FrTpChannelParams.FrTpTxChannel.FrTpTxPrevChannelState;
              buffDiffbytes = PduInfoPtr->SduLength - runtimeInternalChnllData->FrTpTransmitPduLength;
              txLen = runtimeInternalChnllData->FrTpTransmitPduLength;
          }
        }else{

           if(runtimeInternalChnllData->FrTpRxChannelState == FRTP_WAIT_FOR_TRIGGER_TRANSMIT){

                 memcpy(PduInfoPtr->SduDataPtr,runtimeInternalChnllData->FrTpRxFcDataPdu,FLOW_CONTROL_FRAME_PAYLOAD);
                 /* Copying back Orignal state */
                 runtimeInternalChnllData->FrTpRxChannelState = runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpRxPrevChannelState;
                 buffDiffbytes = PduInfoPtr->SduLength - FLOW_CONTROL_FRAME_PAYLOAD;
                 txLen = FLOW_CONTROL_FRAME_PAYLOAD;
             }
        }
        if ( buffDiffbytes > 0) {
          memset(&(PduInfoPtr->SduDataPtr[txLen]), 0,buffDiffbytes);
        }
    } else {
        status = E_NOT_OK;
    }
    return status;
}


/**
 *
 * @param RxPduId
 * @param PduInfoPtr
 */
/* Indication of a received I-PDU from a lower layer communication module. */
/* @req FRTP152 */  /* @req FRTP137 */
void FrTp_RxIndication(PduIdType RxPduId,PduInfoType* PduInfoPtr) {

    uint8 count;
    static ISO10681FrameType frameTypelcl;
    ISO10681PduAddressType frameAddressInfo;

    /* @req FRTP1037 */
    FRTP_DET_REPORTERROR((FRTP_ON == FrTp_Internal_RunTime.FrTpState),FRTP_SERVICE_ID_RX_INDICATION,FRTP_E_UNINIT);
    /* @req FRTP1147 */ /* @req FRTP1069 *//* @req FRTP1070]*/
    FRTP_DET_REPORTERROR((RxPduId < FrTp_ConfigPtr->FrTpRxPducount),FRTP_SERVICE_ID_RX_INDICATION,FRTP_E_INVALID_PDU_SDU_ID);
    /* @req FRTP1148 */
    FRTP_DET_REPORTERROR((NULL != PduInfoPtr),FRTP_SERVICE_ID_RX_INDICATION,FRTP_E_NULL_PTR);
    FRTP_DET_REPORTERROR((NULL != PduInfoPtr->SduDataPtr),FRTP_SERVICE_ID_RX_INDICATION,FRTP_E_NULL_PTR);

    frameTypelcl = getFrameType(PduInfoPtr->SduDataPtr);
    getFrameAddress(PduInfoPtr, &frameAddressInfo);


    if((START_FRAME == frameTypelcl)
#if(STD_ON == FRTP_ACK_RETRY_SUPPORT)
            || (START_FRAME_ACK == frameTypelcl)
#endif
      )
    {
        count = frtpManageStfConnections(PduInfoPtr,RxPduId,frameTypelcl,frameAddressInfo);

        if(INVALID_CONNECTION != count) /* @req FRTP1072*/
        {
          /* @req FRTP1076*/ /* @req FRTP1186*/
            FRTP_DET_REPORTERROR((FRTP_CHANNEL_NUM > count),FRTP_SERVICE_ID_RX_INDICATION,FRTP_E_NO_CHANNEL);
        }
    }
    else
    {
        if(ISO10681_TPCI_FC == (PduInfoPtr->SduDataPtr[PCI_BYTE_1] & ISO10681_TPCI_MASK)){
            frtpManageFcConnections(PduInfoPtr,frameAddressInfo);
        }else{
            frtpManageCfConnections(PduInfoPtr, frameTypelcl,frameAddressInfo);
        }

    }
}
/**
 *
 * @param TxPduId
 */
/* The lower layer communication module confirms the transmission of an I-PDU.*/
/* @req FRTP153 */
void FrTp_TxConfirmation(PduIdType TxPduId){

    /* @req FRTP1052 */ /* @req FRTP1053*/ /* @req FRTP1093*/
    FrTp_Internal_Channel_type* runtimeInternalChnllData;
    uint16 channelIndex;
    boolean flag;

    /* @req FRTP1037 */
    FRTP_DET_REPORTERROR((FRTP_ON == FrTp_Internal_RunTime.FrTpState),FRTP_SERVICE_ID_TX_CONFIRMATION,FRTP_E_UNINIT);
    /* @req FRTP1149 */
    flag = (TxPduId < FrTp_ConfigPtr->FrTpTxPduCount) &&( FrTp_Internal_RunTime.FrTpTxPduStatusInfo[TxPduId].FrTpTxPduLockStatus != AVAILABLE);
    FRTP_DET_REPORTERROR((TRUE == flag),FRTP_SERVICE_ID_TX_CONFIRMATION,FRTP_E_INVALID_PDU_SDU_ID);

    channelIndex = FrTp_Internal_RunTime.FrTpTxPduStatusInfo[TxPduId].FrTpChannleHandle;

    runtimeInternalChnllData = &FrTp_Internal_RunTime.FrTpChannels[channelIndex];
    if( FrTp_Internal_RunTime.FrTpTxPduStatusInfo[TxPduId].FrTpTxPduLockStatus == LOCKED_FOR_SF_CF_LF){
      /* @req FRTP1092 */
      runtimeInternalChnllData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPendingCounter--;
      /* Stop A timer */
      runtimeInternalChnllData->FrTpChannelParams.FrTpTxChannel.FrTpCounterAs = 0u;

      if((runtimeInternalChnllData->FrTpTxChannelState == FRTP_TX_WAIT_FOR_FC)
  #if (FRTP_ACK_RETRY_SUPPORT == STD_ON)
              ||((runtimeInternalChnllData->FrTpTxChannelState == FRTP_TX_FINISHED)&&
              (runtimeInternalChnllData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpAckType != FRTP_NO))
  #endif
          )  {
             /* For Segmented Tx if Flow control is expected start the Bs timer */
             runtimeInternalChnllData->FrTpChannelParams.FrTpTxChannel.FrTpCounterBs = runtimeInternalChnllData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpTimeoutBs+1;
          }

    }else if(FrTp_Internal_RunTime.FrTpTxPduStatusInfo[TxPduId].FrTpTxPduLockStatus == LOCKED_FOR_FC){

        runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpRxConTxPduPendingCounter--;
        /* Stop Ar Timer */
        runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpCounterAr = 0u;

        if(((runtimeInternalChnllData->FrTpRxFcDataPdu[PCI_BYTE_1] & ISO10681_TPCI_FS_MASK) == ISO10681_FC_STATUS_ABORT)
                              || ((runtimeInternalChnllData->FrTpRxFcDataPdu[PCI_BYTE_1] & ISO10681_TPCI_FS_MASK) == ISO10681_FC_STATUS_OVFL)){
                      resetRxChannelConfiguration(runtimeInternalChnllData);
           }

        #if(STD_ON == FRTP_ACK_RETRY_SUPPORT)
        else if(((runtimeInternalChnllData->FrTpRxFcDataPdu[PCI_BYTE_1] & ISO10681_TPCI_FS_MASK) == ISO10681_FC_STATUS_ACK_RET)
                  &&(runtimeInternalChnllData->FrTpRxFcDataPdu[PCI_BYTE_2] == 0)){
              /* @req FRTP1083 */ /* @req FRTP1081 */
              resetRxChannelConfiguration(runtimeInternalChnllData);   /* Since FC with ACK is the final Activity of Reception */
          }
        #endif
        else if(runtimeInternalChnllData->FrTpRxChannelState == FRTP_RX_WAIT_CF){
            /* Start Cr timer */
            runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpCounterCr = runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpTimeoutCr;
        }
        else{
            /*do nothing */
        }
    }else{
        /*do nothing */
    }
      /* @req FRTP1054*/
      FrTp_Internal_RunTime.FrTpTxPduStatusInfo[TxPduId].FrTpTxPduLockStatus= AVAILABLE;

return;
}

/**
 * @param void
 */
/*Schedules the FlexRay TP. (Entry point for scheduling)*/
/* @req FRTP162 */
void FrTp_MainFunction(void) {
    /* Disable interrupts */
    SchM_Enter_FrTp_EA_0();
    /* @req FRTP203*/ /* @req FRTP580*/
    FrTp_Internal_Channel_type *ChannelInternal;
    uint8 count;
    /* @req FRTP1037 */ /*@req FRTP1106*/
    FRTP_DET_REPORTERROR((FRTP_ON == FrTp_Internal_RunTime.FrTpState),FRTP_SERVICE_ID_MAIN_FUNCTION,FRTP_E_UNINIT);

    for(count = 0; count< FRTP_CHANNEL_NUM; count++) {

        ChannelInternal =  &FrTp_Internal_RunTime.FrTpChannels[count];
        if(IDLE != ChannelInternal->FrTpRxChannelState){
            handleReceivedFrame(ChannelInternal);
        }
        FrTp_Internal_TransmitStatesHandle(ChannelInternal);
    }
    /* Enable interrupts */
    SchM_Exit_FrTp_EA_0();
}


/**
 * @brief To handle states of transmit sequence
 * @param runtimeChnlData -  Internal runtime data ptr for Channel used for transmit
 * @return void
 */
static void FrTp_Internal_TransmitStatesHandle(FrTp_Internal_Channel_type* runtimeChnlData){

if(runtimeChnlData->FrTpTxChannelState!=IDLE){
    if(runtimeChnlData->FrTpTcRequest == TRUE){
        FrTp_Internal_HandleTxCanceled(runtimeChnlData);
    }

  /* @req FRTP1094*/ /* @req FRTP1095*/
  if(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPendingCounter == 0){

      switch(runtimeChnlData->FrTpTxChannelState) {
          case FRTP_TX_STARTED :
                  FrTp_Internal_TransmitStartFrame(runtimeChnlData);
             break;
         case FRTP_TX_WAIT_FOR_FC :
                  FrTp_Internal_WaitForFlowControl(runtimeChnlData);
             break;
         case FRTP_TX_FINISHED:
             FrTp_Internal_TransmitFinished(runtimeChnlData);
             break;
         case FRTP_FRIF_ERROR:
             FrTp_Internal_HandleFrIfError(runtimeChnlData,ACTIVE_TX);
             break;
         case FRTP_TX_SEG_ONGOING:
             FrTp_Internal_HandleNextSegFrames(runtimeChnlData);
             break;
         case FRTP_WAIT_FOR_TRIGGER_TRANSMIT:
         default:
             /* Do nothing */
             break;
       }
  }else{

      if(runtimeChnlData->FrTpTxSduAvailable !=FALSE){
          /* Handle As timer and timeout */ /* @req FRTP1099 */
          FrTp_Internal_TimeoutAs(runtimeChnlData);
      }
  }
}

}


/**
 * @brief To handle Next segmented frames
 * @param runtimeChnlData -  Internal runtime data ptr for Channel used for transmit
 * @return void
 */
static void FrTp_Internal_HandleNextSegFrames(FrTp_Internal_Channel_type* runtimeChnlData){

    PduLengthType remainingTxBufferLength;
    PduLengthType payloadIndex;
    uint16 pduIndex;
    uint8 txFrameType;

    txFrameType = 0;

    /* Check if this channel has to be scheduled for transmission*/
    /* If there is a pending data in the transmit buffers or all the data is transmitted in the last CF */
    if ((runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpRemainingTxBufferLength != 0)
            ||(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpCompletedMessageLength == runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpMessageLength)
#if (FRTP_UNKOWN_MSG_LNGT == STD_ON)
            /* In case of unknown message length in the condition FrTpRemainingTxBufferLength a Last Frame has to be sent *//* @req FRTP1066 */
            || (runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpMessageLength == 0)
#endif
       ){
        for(pduIndex=0;pduIndex<runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpPoolTxPduCnt;pduIndex++){
              if(FrTp_Internal_RunTime.FrTpTxPduStatusInfo[runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpTxPduPoolRef[pduIndex].FrTpTxPduId].FrTpTxPduLockStatus== AVAILABLE){
                  runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef = (&runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpTxPduPoolRef[pduIndex]);
                 break;
              }
         }

        if(pduIndex<runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpPoolTxPduCnt){
           if(E_OK == FrTp_Internal_GetNextFrameType(runtimeChnlData,&txFrameType,&remainingTxBufferLength)){
               payloadIndex = FrTp_Internal_FillNextFrame(runtimeChnlData,txFrameType);
               FrTp_Internal_SendNextFrame(runtimeChnlData,payloadIndex);
           }
        }
      }
}




/**
 * @brief To to send next frame buffer values and fill data from upper layer data
 * @param runtimeChnlData -  Internal runtime data ptr for Channel used for transmit
 * @param txPayloadIndex
 */
static void FrTp_Internal_SendNextFrame(FrTp_Internal_Channel_type* runtimeChnlData,PduLengthType payloadIndex ){

    PduInfoType txPduInfo;
    BufReq_ReturnType bufferRetVal;
    /* Ask upper layer to copy the tx data */
    txPduInfo.SduDataPtr = &runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[payloadIndex];
    txPduInfo.SduLength = runtimeChnlData->FrTpFramePayloadLength;
#if (FRTP_ACK_RETRY_SUPPORT == STD_ON)
    if(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpAckType != FRTP_NO) {

        bufferRetVal = PduR_FrTpCopyTxData(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId, &txPduInfo,&runtimeChnlData->FrTpRetryInfo, &runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpRemainingTxBufferLength);
        /* Clear retry request or data confirmation*/
        runtimeChnlData->FrTpRetryInfo.TpDataState = TP_CONFPENDING;
    }else
#endif
    {
        bufferRetVal = PduR_FrTpCopyTxData(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId,&txPduInfo,NULL_PTR, &runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpRemainingTxBufferLength);
    }

    if (bufferRetVal == BUFREQ_OK)  {

        /* @req FRTP1105*/ /* @req FRTP1114 */
        runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxFcWt = runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpMaxFCWait;

        /* Reduce the available buffer with the already copied length */
        runtimeChnlData->FrTpFcRecveBuffSize -=  runtimeChnlData->FrTpFramePayloadLength;

        /* Transmit the complete constructed frame */
        txPduInfo.SduDataPtr = &runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[0];
        txPduInfo.SduLength += payloadIndex;
        /* Send out the next frame */
        if (FrTp_Internal_SendFrame(runtimeChnlData, &txPduInfo,ACTIVE_TX) == E_OK)  {
            /* @req FRTP1114 */
            runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpAsTimeoutRetryCounter =  runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpMaxAs;
            /* @req FRTP1067 */
            runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpCompletedMessageLength += runtimeChnlData->FrTpFramePayloadLength;

#if (FRTP_ACK_RETRY_SUPPORT == STD_ON)
            if (runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpAckType != FRTP_NO)  {
                /* Increase the counter of the unacknowledged bytes */
                runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxBlockPosition += runtimeChnlData->FrTpFramePayloadLength;
            }
#endif
        }
    }
    else
    {
        FrTp_Internal_TxBufferRequestError(runtimeChnlData, bufferRetVal);
    }
}




/**
 * @brief To to fill next frame buffer values
 * @param runtimeChnlData - Internal runtime data ptr for Channel used for transmit
 * @param txFrameType
 * @param remainingTxBufferLength
 * @return PduLengthType
 */
static PduLengthType FrTp_Internal_FillNextFrame(FrTp_Internal_Channel_type* runtimeChnlData, uint8 txFrameType ){

    PduLengthType txPayloadIndex;

    txPayloadIndex =0;

    if(FrTp_Internal_RunTime.FrTpTxPduStatusInfo[runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef->FrTpTxPduId].FrTpTxPduLockStatus== AVAILABLE){

        /* Allocate the first free available Pdu, lock the index of used pdu */  /* @req FRTP1045 */
        FrTp_Internal_RunTime.FrTpTxPduStatusInfo[runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef->FrTpTxPduId].FrTpTxPduLockStatus = LOCKED_FOR_SF_CF_LF;
        FrTp_Internal_RunTime.FrTpTxPduStatusInfo[runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef->FrTpTxPduId].FrTpChannleHandle = runtimeChnlData->FrTpChannelNumber;

    }

    switch(txFrameType){
        case ISO10681_TPCI_CF1:
#if (FRTP_ACK_RETRY_SUPPORT == STD_ON)
            /* Change CF type if retry is in progress*/
            if (runtimeChnlData->FrTpuseCF1 == FALSE) {
                runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[4] = ISO10681_TPCI_CF2 | runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpSerialNumber;
            }
            else
#endif
            {
              runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[4] = ISO10681_TPCI_CF1 | runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpSerialNumber;
            }
            txPayloadIndex = FRTP_HEADER_LENGTH_CF;
            runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpSerialNumber = (runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpSerialNumber +1) % FRTP_SN_RANGE_MODULO;
            break;
        case ISO10681_TPCI_CFEOB:
            txPayloadIndex = FRTP_HEADER_LENGTH_CF;
            runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[4] = ISO10681_TPCI_CFEOB | runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpSerialNumber;
            /* At the end of block wait for the flow control */
            runtimeChnlData->FrTpTxChannelState = FRTP_TX_WAIT_FOR_FC;
            runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpSerialNumber = (runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpSerialNumber + 1) % FRTP_SN_RANGE_MODULO;
            break;
        case ISO10681_TPCI_LF:
            txPayloadIndex = FRTP_HEADER_LENGTH_STF_LF;
            runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[4] = ISO10681_TPCI_LF;
            runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[6] = FRTP_MSB_U16(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpCompletedMessageLength + runtimeChnlData->FrTpFramePayloadLength);
            runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[7] = FRTP_LSB_U16(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpCompletedMessageLength + runtimeChnlData->FrTpFramePayloadLength);
            /* Set state to finished */
            runtimeChnlData->FrTpTxChannelState = FRTP_TX_FINISHED;
#if (FRTP_ACK_RETRY_SUPPORT == STD_ON)
            /* Wait for the acknowledge */
            runtimeChnlData->FrTpAckPending = runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpAckType;
#endif
            break;
        default:
            /* Nothing to do*/
            break;
    }

    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[5] = (uint8) runtimeChnlData->FrTpFramePayloadLength;
return txPayloadIndex;
}


/**
 * @brief To identify next frame type
 * @param runtimeChnlData Internal runtime data ptr for Channel used for transmit
 * @param txFrameType
 * @param remainingTxBufferLength
 * @return Std_ReturnType
 */
static Std_ReturnType FrTp_Internal_GetNextFrameType(FrTp_Internal_Channel_type* runtimeChnlData,uint8* txFrameType,PduLengthType* remainingTxBufferLength ){

    Std_ReturnType status;
    PduLengthType txPduLen;
    PduLengthType availableTxBufLen;
    status = E_OK;

    /* Available buffer length is the minimum of Transmitter buffer and Receiver buffer */
    if(runtimeChnlData->FrTpFcRecveBuffSize < runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpRemainingTxBufferLength ){
        availableTxBufLen = runtimeChnlData->FrTpFcRecveBuffSize;
    } else {
        availableTxBufLen = runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpRemainingTxBufferLength;
    }

    txPduLen = runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef->FrTpTxPduLength;

    /*@req FRTP1089*/  /* consider different PduLength in the pool */
    /** This condition checks if the Remaining bytes > (TxPdu length - CF header length) to decide a CF */
    if(availableTxBufLen > (txPduLen -FRTP_HEADER_LENGTH_CF)){
        *txFrameType = ISO10681_TPCI_CF1;
        runtimeChnlData->FrTpFramePayloadLength = (uint8)(txPduLen - FRTP_HEADER_LENGTH_CF);

    } else if(availableTxBufLen <= (txPduLen-FRTP_HEADER_LENGTH_STF_LF)){         /** This condition checks if the remaining bytes
    to transfer fits within selected TxPdu. Remaining bytes <= (TxPdu length - LF header length) i.e implies either a LF or CF_EOB */

        /** Check Completed Length + Remaining Length = Total Length to decide LF */
        if( ((runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpCompletedMessageLength + availableTxBufLen) ==
                runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpMessageLength)
                /* this is to decide the last frame based on the FrTpCompletedMessageLength */
#if (FRTP_UNKOWN_MSG_LNGT == STD_ON) /* @req FRTP1066 */
                || ((runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpMessageLength == 0) && (runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpRemainingTxBufferLength == 0))
#endif
        ){
            *txFrameType = ISO10681_TPCI_LF;
            runtimeChnlData->FrTpFramePayloadLength = (uint8)availableTxBufLen;
        } else if (availableTxBufLen == 0) {
            /* The available TxBuffer can not fill out the next Pdu so do not send out this data yet */
            status = E_NOT_OK;
        }else{/** If Remaining bytes to be transmitted is constrained by the receiver/Transmitter buffer then it is EOB*/

            *txFrameType = ISO10681_TPCI_CFEOB;
            runtimeChnlData->FrTpFramePayloadLength = (uint8)availableTxBufLen;
        }
    }else{ /** If we have reached here then Remaining bytes = (TxPdu Length - CF header length)    */

        if(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpRemainingTxBufferLength == availableTxBufLen){
            /* If Remaining bytes to be transmitted depends on the transmit buffer then it is CF */
            *txFrameType = ISO10681_TPCI_CF1;
            runtimeChnlData->FrTpFramePayloadLength = (uint8)availableTxBufLen;
        } else {
            /* If Remaining bytes to be transmitted depends on the receiver buffer then it is CF_EOB*/
            *txFrameType = ISO10681_TPCI_CFEOB;
            runtimeChnlData->FrTpFramePayloadLength = (uint8)availableTxBufLen;
        }
    }
    if (status == E_OK) {
        * remainingTxBufferLength = availableTxBufLen;
    }
    return status;
}


/**
 * @brief To transmit start frame to lower layers
 * @param runtimeChnlData -  Internal runtime data ptr for Channel used for transmit
 * @return void
 */
static void FrTp_Internal_TransmitStartFrame(FrTp_Internal_Channel_type* runtimeChnlData){


    BufReq_ReturnType buffRetValue;
    Std_ReturnType retVal;
    PduInfoType pduInfo;
    PduLengthType TxBufferLength;
    uint8 pdupoolIndex;
    boolean breakLoop;

    retVal = E_NOT_OK;
    buffRetValue = BUFREQ_OK;
    pduInfo.SduLength = 0;
    pduInfo.SduDataPtr = NULL;
     /* @req FRTP1042 */
    if(runtimeChnlData->FrTpTxSduAvailable==TRUE){
        /* this to get the TxBufferLength from upper layer */
        buffRetValue = PduR_FrTpCopyTxData(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId, &pduInfo, NULL_PTR, &TxBufferLength);
         if (buffRetValue==BUFREQ_OK){
             breakLoop = FALSE;
              /* @req FRTP1096 */
             for (pdupoolIndex = 0; (pdupoolIndex < runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpPoolTxPduCnt) && (FALSE == breakLoop); pdupoolIndex++) {
                 if(FrTp_Internal_RunTime.FrTpTxPduStatusInfo[runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpTxPduPoolRef[pdupoolIndex].FrTpTxPduId].FrTpTxPduLockStatus== AVAILABLE){
                    FrTp_Internal_RunTime.FrTpTxPduStatusInfo[runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpTxPduPoolRef[pdupoolIndex].FrTpTxPduId].FrTpTxPduLockStatus= LOCKED_FOR_SF_CF_LF;
                    FrTp_Internal_RunTime.FrTpTxPduStatusInfo[runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpTxPduPoolRef[pdupoolIndex].FrTpTxPduId].FrTpChannleHandle = runtimeChnlData->FrTpChannelNumber;
                    if(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpMultipleReceiverCon!=FALSE) {
                      if(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpMessageLength <= (runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpTxPduPoolRef[pdupoolIndex].FrTpTxPduLength - FRTP_HEADER_LENGTH_STF_LF)){
                            /* Assign Pdu to this channel */
                            /* @req FRTP1046 */
                            runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef = &runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpTxPduPoolRef[pdupoolIndex];
                            breakLoop = TRUE;
                        }else{

                            /* check for the next pdu with enough size to send it unsegmented
                             * shall not process for segmentation */
                             /* @req FRTP1068 */
                        }
                    }else{

                        /*@req FRTP1045*/ /*@req FRTP1046*/
                        runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef = &runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpTxPduPoolRef[pdupoolIndex];
                        breakLoop = TRUE;
                    }
                 }
             }

             if(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef != NULL_PTR){

                 if (TxBufferLength == runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpMessageLength){ /* this check for known message length during Frtp transmit call */
                     /*@req FRTP1048*/
                         /* Physical addressing - If the complete data is available try to send it unsegmented */
                     if (runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpMessageLength <= (runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef->FrTpTxPduLength - FRTP_HEADER_LENGTH_STF_LF)) {

                         /* FrTpTxChannelState will be modified to FRTP_FRIF_ERROR if FrTp_Internal_PrepareAndSendStartFrame->FrTp_Internal_SendFrame fails to send frame and gets FrIf transmit  error */
                         runtimeChnlData->FrTpTxChannelState = FRTP_TX_FINISHED;
                         retVal= FrTp_Internal_PrepareAndSendStartFrame(runtimeChnlData,(uint8)runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpMessageLength);

                     }else{
                         /* Pdu isn't large enough to transmit then Segmented transmission */
                         runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpSerialNumber = 1u;
                         /* FrTpTxChannelState will be modified to FRTP_FRIF_ERROR if FrTp_Internal_PrepareAndSendStartFrame->FrTp_Internal_SendFrame fails to send frame and gets FrIf transmit  error */
                         runtimeChnlData->FrTpTxChannelState = FRTP_TX_WAIT_FOR_FC;
                         retVal= FrTp_Internal_PrepareAndSendStartFrame(runtimeChnlData,(uint8)(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef->FrTpTxPduLength - FRTP_HEADER_LENGTH_STF_LF));
                     }
                     if(retVal == E_OK){
#if FRTP_ACK_RETRY_SUPPORT == STD_ON
                         if(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpAckType!= FRTP_NO){
                             /* Wait for the acknowledge */
                             runtimeChnlData->FrTpAckPending = FRTP_ACK_WITH_RT;
                         }
#endif
                     }

                 }else{ /* this check for unknown message length during Frtp transmit call */
                     /* FrTpTxChannelState will be modified to FRTP_FRIF_ERROR if FrTp_Internal_PrepareAndSendStartFrame->FrTp_Internal_SendFrame fails to send frame and gets FrIf transmit  error */
                     runtimeChnlData->FrTpTxChannelState = FRTP_TX_WAIT_FOR_FC;
                     runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpSerialNumber = 1u;
                     if (TxBufferLength <= (runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef->FrTpTxPduLength - FRTP_HEADER_LENGTH_STF_LF)) {

                         retVal= FrTp_Internal_PrepareAndSendStartFrame(runtimeChnlData,(uint8)TxBufferLength);
                     }else{
                         retVal = FrTp_Internal_PrepareAndSendStartFrame(runtimeChnlData,(uint8)(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef->FrTpTxPduLength - FRTP_HEADER_LENGTH_STF_LF));
                     }
                 }

                 if(retVal == E_OK){
                     /* @req FRTP1067 */
                     /* To maintain the total transmitted length */
                     runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpCompletedMessageLength += (runtimeChnlData->FrTpTransmitPduLength- FRTP_HEADER_LENGTH_STF_LF);
                 }
              }
           }else{
               /*Notify error to upper layer */
               FrTp_Internal_TxBufferRequestError(runtimeChnlData, buffRetValue);
           }
      }
}



/**
 * @brief To Prepare and send the start frame
 * @param runtimeChnlData -  Internal runtime data ptr for Channel used for transmit
 * @param framePayloadLength - Start up frame pay load length
 * @return void
 */
static Std_ReturnType FrTp_Internal_PrepareAndSendStartFrame(FrTp_Internal_Channel_type* runtimeChnlData, uint8 framePayloadLength) {

   /* @req FRTP1049*/ /* @req FRTP1006 */
    PduInfoType pduInfo;
    BufReq_ReturnType buffRetValue;
    Std_ReturnType retVal;

    retVal = E_NOT_OK;

    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[0] = FRTP_MSB_U16(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpRa);
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[1] = FRTP_LSB_U16(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpRa);
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[2] = FRTP_MSB_U16(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpLa);
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[3] = FRTP_LSB_U16(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpLa);

    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[5] = framePayloadLength;
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[6] = FRTP_MSB_U16(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpMessageLength);
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[7] = FRTP_LSB_U16(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpMessageLength);

    /* @req FRTP1123*/ /* @req FRTP1051 */
    /* request upper layer to copy the tx data */
    pduInfo.SduDataPtr = &runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[FRTP_HEADER_LENGTH_STF_LF];
    pduInfo.SduLength = framePayloadLength;
#if (FRTP_ACK_RETRY_SUPPORT == STD_ON)
    if (runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpAckType != FRTP_NO)
    {
        runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[4] = FRTP_CPCI_STFA;
        /* If acknowledge is needed the upper layer must keep the data in the buffer until acknowledge */
        runtimeChnlData->FrTpRetryInfo.TpDataState = TP_CONFPENDING;
        buffRetValue = PduR_FrTpCopyTxData(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId, &pduInfo, &runtimeChnlData->FrTpRetryInfo,&runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpRemainingTxBufferLength);
    }else
#endif
    {
       runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[4] = FRTP_CPCI_STFU;
        /* @req FRTP1065*/ /* @req FRTP1123 */
       buffRetValue = PduR_FrTpCopyTxData(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId, &pduInfo, NULL_PTR,&runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpRemainingTxBufferLength);

    }

    if (buffRetValue == BUFREQ_OK)
    {
        /* Reinitialize buffer retry counter */
        /* @req FRTP1105*/ /* @req FRTP1114 */
        runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxFcWt = runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpMaxFCWait;
        /* Transmit the complete constructed frame */
        pduInfo.SduDataPtr = &runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[0];
        pduInfo.SduLength += FRTP_HEADER_LENGTH_STF_LF;
        /* No. of attempts in case a timeout AS occurs*/
        runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpAsTimeoutRetryCounter =  runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpMaxAs;
        /* Send out the start frame */
        retVal = FrTp_Internal_SendFrame(runtimeChnlData, &pduInfo,ACTIVE_TX);

    }else{
        /*Notify error to upper layer */
        FrTp_Internal_TxBufferRequestError(runtimeChnlData, buffRetValue);
    }

    return retVal;
}

/**
 * @brief To transmit prepared data to interface layer
 * @param runtimeChnlData -  Internal runtime data ptr for Channel used for transmit
 * @param txPduInfo - pdu data and length for transmission
 * @return Std_ReturnType
 */
static Std_ReturnType FrTp_Internal_SendFrame(FrTp_Internal_Channel_type* runtimeChnlData, PduInfoType *txPduInfo, FrtpCommType commState) {

    Std_ReturnType retVal;
    retVal = E_NOT_OK;

    if(commState == ACTIVE_TX){

        runtimeChnlData->FrTpTransmitPduLength = txPduInfo->SduLength;

        /* @req FRTP1050 */ /* @req FRTP1051 */
        retVal = FrIf_Transmit(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef->FrTpFrIfTxPduId,txPduInfo);

        if(retVal == E_OK ){

            /* @req FRTP1091 */
            runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPendingCounter++;
            /* Start As timer */
            runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpCounterAs = FrTp_ConfigPtr->FrTpConnectionConfig[runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpAssignedConnectionIndex].FrTpConCtrlRef->FrTpTimeoutAs;

            if (runtimeChnlData->FrTpTxChannelState == FRTP_FRIF_ERROR)
            {
                 runtimeChnlData->FrTpTxChannelState = runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxPrevChannelState; /* copy the previous state */
                /*@req FRTP1105*/ /*@req FRTP1114 */
                runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxCounterFrIf = FrTp_ConfigPtr->FrTpConnectionConfig[runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpAssignedConnectionIndex].FrTpConCtrlRef->FrTpMaxFrIf;
            }
            /* @req FRTP1084 */ /* Decoupled Buffer Access */
            if (runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef->FrTpImmediateAccess == FALSE)
            {
                /* Copy the current stae unit trigger transmit */
                runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxPrevChannelState = runtimeChnlData->FrTpTxChannelState;
                runtimeChnlData->FrTpTxChannelState = FRTP_WAIT_FOR_TRIGGER_TRANSMIT;
            }
          }else{
              if(FrTp_ConfigPtr->FrTpConnectionConfig[runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpAssignedConnectionIndex].FrTpConCtrlRef->FrTpMaxFrIf== 0){
                  PduR_FrTpTxConfirmation(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId, NTFRSLT_E_ABORT);
                  FrTp_Internal_FreeChannel(runtimeChnlData);
              }else{
                  if(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxCounterFrIf == runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpMaxFrIf){
                      /* Take a copy of current state only for 1st call */
                      runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxPrevChannelState = runtimeChnlData->FrTpTxChannelState;
                  }
                /* load the counter with the time waiting for the next try (if retry is activated) to send via FrIf_Transmit*/
                runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpFrIfTxErrNextTryTimer = (uint8)(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpTimeFrIf);
                /* Change state to Tx error sender */
                runtimeChnlData->FrTpTxChannelState = FRTP_FRIF_ERROR;

              }
            }
     }else if(commState == ACTIVE_RX){

        /* @req FRTP1050 */ /* @req FRTP1051 */
        retVal = FrIf_Transmit(runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpRxConTxPduPoolRuntimeRef->FrTpFrIfTxPduId,txPduInfo);

        if(retVal == E_OK ){
            /* @req FRTP1091 */
            runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpRxConTxPduPendingCounter++;
            /* Start Ar timer */
            runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpCounterAr =  runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpTimeoutAr;

            if (runtimeChnlData->FrTpRxChannelState == FRTP_FRIF_ERROR)
            {
              /*@req FRTP1105*/ /*@req FRTP1114 */
               runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpRxCounterFrIf = runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpMaxFrIf;
               runtimeChnlData->FrTpRxChannelState = runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpRxPrevChannelState;/* copy the previous state */
            }
            /* @req FRTP1084 */ /* Decoupled Buffer Access */
            if (runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpRxConTxPduPoolRuntimeRef->FrTpImmediateAccess == FALSE)
            {
                /* Copy the current state unit Trigger transmit */
                runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpRxPrevChannelState = runtimeChnlData->FrTpRxChannelState;
                runtimeChnlData->FrTpRxChannelState = FRTP_WAIT_FOR_TRIGGER_TRANSMIT;
            }

          }else{
              if(FrTp_ConfigPtr->FrTpConnectionConfig[runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpAssignedRxConnectionIndex].FrTpConCtrlRef->FrTpMaxFrIf== 0){
                PduR_FrTpTxConfirmation(runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpULTxPduId, NTFRSLT_E_ABORT);
                resetRxChannelConfiguration(runtimeChnlData);
               }else{
                   if(runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpRxCounterFrIf == runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpMaxFrIf){
                       /* Take a copy of current state only for 1st time */
                       runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpRxPrevChannelState = runtimeChnlData->FrTpRxChannelState;
                   }
                 /* load the counter with the time waiting for the next try (if retry is activated) to send via FrIf_Transmit*/
                runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpFrIfRxErrNextTryTimer = (uint8)(runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpTimeFrIf);
                /* Change state to Tx error sender */
                runtimeChnlData->FrTpRxChannelState = FRTP_FRIF_ERROR;
               }
           }
      }else{
          /* do nothing */
      }

    return retVal;
}
/**
 * @brief To decrement flow control time while waiting for FC
 * @param runtimeChnlData -Internal runtime data ptr for Channel used for transmit
 */
static void FrTp_Internal_WaitForFlowControl(FrTp_Internal_Channel_type* runtimeChnlData){

   /* @req FRTP1099 */
    if(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpCounterBs !=0){

        /* Decrement the timer */
        runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpCounterBs--;
        /* If timeout expired */
        if (runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpCounterBs == 0)
        {
            /* @req FRTP1100 */
            PduR_FrTpTxConfirmation(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId, NTFRSLT_E_TIMEOUT_BS);
            FrTp_Internal_FreeChannel(runtimeChnlData);
        }
    }
}

/**
 * @brief To handle transmit completed sequence
 * @param runtimeChnlData -Internal runtime data ptr for Channel used for transmit
 */
static void FrTp_Internal_TransmitFinished(FrTp_Internal_Channel_type* runtimeChnlData){

#if (FRTP_ACK_RETRY_SUPPORT == STD_ON)
    if(runtimeChnlData->FrTpAckPending == FRTP_NO){
        /* Notify upper layer about the successful transmission */
        PduR_FrTpTxConfirmation(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId, NTFRSLT_OK);
        FrTp_Internal_FreeChannel(runtimeChnlData);
    }else{

        /* @req FRTP1099*/
        FrTp_Internal_WaitForFlowControl(runtimeChnlData);
    }
#else
    /* Notify upper layer about the successful transmission */
    /* @req FRTP557 */ /* @req FRTP1058 */
    PduR_FrTpTxConfirmation(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId, NTFRSLT_OK);
    FrTp_Internal_FreeChannel(runtimeChnlData);
#endif
}

/**
 * @brief - To get the right connection based on the FrTpTxSduId.
 * @param frtpTxSduId  - sduId request for transmit
 * @param connectionIndex - Identified connection index for sudId
 * @return Std_ReturnType
 */
static Std_ReturnType FrTp_Internal_GetConnectionForTxSdu(const PduIdType frtpTxSduId, uint8 *connectionIndex){

    uint8 loopIndex;
    Std_ReturnType retValue;
    retValue = E_NOT_OK;

    for (loopIndex = 0; loopIndex < FrTp_ConfigPtr->FrTpConnectionCount; loopIndex++) {
        if(FrTp_ConfigPtr->FrTpConnectionConfig[loopIndex].FrTpTxSduId == frtpTxSduId)
        {
            *connectionIndex = loopIndex;
            retValue = E_OK;
            break;
        }
    }
    return retValue;
}



/**
 * @brief To assign a free channel to the connection for transmission
 * @param connectionIndex - index identified for the connection with sduId
 * @param runtimeChnlDataAssigned - Channel assignment pointer
 */
static void FrTp_Internal_AssignChannelForTx(const uint8 connectionIndex, const uint8 chnlIndex, FrTp_Internal_Channel_type **runtimeChnlDataAssigned, const PduInfoType* FrTpTxSduInfoPtr){

    *runtimeChnlDataAssigned = &FrTp_Internal_RunTime.FrTpChannels[chnlIndex];

    if(FrTp_Internal_RunTime.FrTpConnectionLockStatus[connectionIndex]==NOT_ACTIVE){
        FrTp_Internal_RunTime.FrTpConnectionLockStatus[connectionIndex]=ACTIVE_TX;
    }
    (*runtimeChnlDataAssigned)->FrTpChannelNumber = chnlIndex;
    /* Attach the connection */
    (*runtimeChnlDataAssigned)->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef = &FrTp_ConfigPtr->FrTpConnectionConfig[connectionIndex];
    /* Store connection index to be able to free it and for future reference  */
    (*runtimeChnlDataAssigned)->FrTpChannelParams.FrTpTxChannel.FrTpAssignedConnectionIndex = connectionIndex;
    /* Initialize channel variables wait counter and friferror counter */
    /* @req FRTP1105 */ /* @req FRTP1114 */
    (*runtimeChnlDataAssigned)->FrTpChannelParams.FrTpTxChannel.FrTpTxFcWt = (*runtimeChnlDataAssigned)->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpMaxFCWait;
    (*runtimeChnlDataAssigned)->FrTpChannelParams.FrTpTxChannel.FrTpTxCounterFrIf =(*runtimeChnlDataAssigned)->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpMaxFrIf;
    /* Mark channel as busy if channel assignment has been finished */
    (*runtimeChnlDataAssigned)->FrTpTxChannelState = FRTP_TX_STARTED; /* @req FRTP1117*/
    (*runtimeChnlDataAssigned)->FrTpTxSduAvailable = TRUE; /* @req FRTP1038*/   /* @req FRTP1039 */  /* @req FRTP415 */  /* @req FRTP416 */
    /* @req FRTP1044 */ /* @req FRTP1064*/ /* @req FRTP1101*/ /* @req FRTP1102 */ /* @req FRTP1043 */
    if(FrTpTxSduInfoPtr->SduLength !=0) {
        (*runtimeChnlDataAssigned)->FrTpTxSduUnknownMsgLength = FALSE;
    }else{
        (*runtimeChnlDataAssigned)->FrTpTxSduUnknownMsgLength = TRUE;
    }
    /* Store the total message length */
    (*runtimeChnlDataAssigned)->FrTpChannelParams.FrTpTxChannel.FrTpMessageLength = FrTpTxSduInfoPtr->SduLength;

}

/**
 * @brief To Handle Tx buffer request errors.
 * @param runtimeChnlData -  Internal runtime data ptr for Channel used for transmit
 * @param BufReqResult - Error on the buffer request
 */
static void FrTp_Internal_TxBufferRequestError(FrTp_Internal_Channel_type* runtimeChnlData,BufReq_ReturnType BufReqResult ){

    switch (BufReqResult) {
          case BUFREQ_E_BUSY:
              /* @req FRTP402 */
              /* Buffer request shall be retried FrTpMaxFcWait times */
              if (runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxFcWt == 0)              {
                  /* @req FRTP1113*/ /* @req FRTP557*/ /* @req FRTP555 */
                  PduR_FrTpTxConfirmation(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId, NTFRSLT_E_WFT_OVRN);
                  FrTp_Internal_FreeChannel(runtimeChnlData);
              }else {
                  runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxFcWt--;
              }
              break;
          case BUFREQ_E_NOT_OK:
              /* @req FRTP1162 */ /* Abort transmission */
              PduR_FrTpTxConfirmation(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId, NTFRSLT_E_ABORT);
              FrTp_Internal_FreeChannel(runtimeChnlData);
              break;
          default:
              break;
      }
}

/**
 * @brief - resets all the channel parameters
 * @param runtimeChnlData  Internal runtime data ptr for Channel used for transmit
 */
static void FrTp_Internal_FreeChannel(FrTp_Internal_Channel_type* runtimeChnlData) {
    uint8 initCount;

    /* Remove channel assignment from the connections */
    runtimeChnlData->FrTpTxSduAvailable = FALSE;  /* @req FRTP1057 */
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxFcWt    = 0;
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxCounterFrIf = 0;
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpMessageLength  = 0;
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPendingCounter = 0;
    runtimeChnlData->FrTpTxSduUnknownMsgLength = 0;     /* @req FRTP1124 */
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpCompletedMessageLength =0;
    runtimeChnlData->FrTpTcRequest = FALSE;

    for(initCount=0; initCount < FRTP_PDUARRAY_SIZE; initCount++) {
        runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[initCount] = 0;
    }
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpCounterAs =0;
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpCounterBs =0;
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpCounterCs =0;
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxCounterFrIf =0;

    runtimeChnlData->FrTpFramePayloadLength=0;
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpRemainingTxBufferLength = 0;
    runtimeChnlData->FrTpFcRecveBuffSize=0;
    runtimeChnlData->FrTpRetryInfo.TpDataState = TP_CONFPENDING;
    runtimeChnlData->FrTpRetryInfo.TxTpDataCnt = 0;
    runtimeChnlData->FrTpTransmitPduLength = 0;
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpSerialNumber = 0;
    if(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef != NULL){
        FrTp_Internal_RunTime.FrTpTxPduStatusInfo[runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef->FrTpTxPduId].FrTpTxPduLockStatus= AVAILABLE;
    }
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef = NULL_PTR;
#if (FRTP_ACK_RETRY_SUPPORT == STD_ON)
    runtimeChnlData->FrTpAckPending = FRTP_NO;
    runtimeChnlData->FrTpuseCF1 = TRUE;
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxBlockPosition =0;
#endif

    if(FrTp_Internal_RunTime.FrTpConnectionLockStatus[runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpAssignedConnectionIndex] == ACTIVE_TX){
        if(runtimeChnlData->FrTpRxChannelState == IDLE){
            FrTp_Internal_RunTime.FrTpConnectionLockStatus[runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpAssignedConnectionIndex]=NOT_ACTIVE;
        }else{
            FrTp_Internal_RunTime.FrTpConnectionLockStatus[runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpAssignedConnectionIndex]=ACTIVE_RX;
        }
    }
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpAssignedConnectionIndex =0xFF;
    runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef = NULL;
    runtimeChnlData->FrTpTxChannelState = IDLE;    /* @req FRTP1118 */ /* @req FRTP1056 */

}

/**
 * @brief - Handles the As time out parameters and attempts to resend on time out
 * @param runtimeChnlData  Internal runtime data ptr for Channel used for transmit
 */
static void FrTp_Internal_TimeoutAs(FrTp_Internal_Channel_type* runtimeChnlData) {

    PduInfoType pduInfo;

    /* @req FRTP1100 */
    /* If timer is running*/
    if(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpCounterAs != 0) {

        runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpCounterAs--;
        /* If timeout expired */
        if(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpCounterAs == 0) {
            if(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpAsTimeoutRetryCounter == 0)   {
                PduR_FrTpTxConfirmation(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId, NTFRSLT_E_TIMEOUT_A);
                FrTp_Internal_FreeChannel(runtimeChnlData);
            } else {
                /* retry for after as time out till As max counts configured  */
                runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpAsTimeoutRetryCounter--;

                pduInfo.SduLength = runtimeChnlData->FrTpTransmitPduLength;
                pduInfo.SduDataPtr = &runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[0];
                if(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConTxPduPoolRuntimeRef != NULL_PTR) {
                    (void)FrTp_Internal_SendFrame(runtimeChnlData, &pduInfo,ACTIVE_TX);
                }
            }
        }
    }
}





/**
 * @brief : Function handles flow control frame received ack and retry.
 * @param ChannelInternal
 * @param PduInfoPtr
 */
static void FrTp_Internal_HandleRecvdFCAck(FrTp_Internal_Channel_type *ChannelInternal,PduInfoType* PduInfoPtr ) {

#if (FRTP_ACK_RETRY_SUPPORT == STD_ON)
    uint8 recvdBCorACK;
    uint16 recvdBytePos;
    uint16 calcBytepos;
    SchM_Enter_FrTp_EA_0();
  if(ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpAckType != FRTP_NO)  {
        recvdBCorACK = PduInfoPtr->SduDataPtr[PCI_BYTE_2];
        if(recvdBCorACK == FRTP_NO_RETRY){
            ChannelInternal->FrTpAckPending = FRTP_NO;
        }else if (recvdBCorACK == FRTP_ACK_WITH_RETRY){
            recvdBytePos = (((uint16)(PduInfoPtr->SduDataPtr[PCI_BYTE_3]) << SHIFT_1_BYTE)|(PduInfoPtr->SduDataPtr[PCI_BYTE_4]));
            /* If BP points to a buffer not yet transmitted */
            if(recvdBytePos > ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpTxBlockPosition) {
                /* Notify upper layer and abort transmission */
                /* @req FRTP142 */
                PduR_FrTpTxConfirmation(ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId, NTFRSLT_E_FR_WRONG_BP);
                FrTp_Internal_FreeChannel(ChannelInternal);
            }else{
                calcBytepos = ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpTxBlockPosition - recvdBytePos;
                /* Roll-back completed message length until the point of retry */
                ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpCompletedMessageLength -= calcBytepos;
                ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpRemainingTxBufferLength += calcBytepos;
                ChannelInternal->FrTpFcRecveBuffSize += calcBytepos;
                ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpTxBlockPosition = recvdBytePos;
                ChannelInternal->FrTpuseCF1 = (ChannelInternal->FrTpuseCF1 != FALSE) ?  FALSE : TRUE;
                ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpSerialNumber = 0;
                /* Store the buffer pointer to be able to retry*/
                ChannelInternal->FrTpRetryInfo.TxTpDataCnt = ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpCompletedMessageLength -1;
                ChannelInternal->FrTpRetryInfo.TpDataState = TP_DATARETRY;

                /* Clear pending acknowledge, because of retry request */
                ChannelInternal->FrTpAckPending = FRTP_NO;

                ChannelInternal->FrTpTxChannelState = FRTP_TX_SEG_ONGOING;

            }
          }
          else{
            /* do Nothing */
            }
    SchM_Exit_FrTp_EA_0();
      }
#else
  (void)*PduInfoPtr;
  (void)*ChannelInternal;
#endif


}


/**
 * @brief - Handles the frif transmit attempts till FrTpMaxFrIf on error
 * @param runtimeChnlData  Internal runtime data ptr for Channel used for transmit
 */
static void FrTp_Internal_HandleFrIfError(FrTp_Internal_Channel_type* runtimeChnlData,FrtpCommType commState) {

    PduInfoType pduInfo;

    if(commState == ACTIVE_TX){
        if( runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpFrIfTxErrNextTryTimer != 0){
            runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpFrIfTxErrNextTryTimer--;
        }
        if(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpFrIfTxErrNextTryTimer == 0){
            if(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxCounterFrIf != 0) {
                runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxCounterFrIf--;

                pduInfo.SduLength = runtimeChnlData->FrTpTransmitPduLength;
                /*@req FRTP1114 */
                pduInfo.SduDataPtr = &runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxDataPdu[0];

                (void)FrTp_Internal_SendFrame(runtimeChnlData, &pduInfo,ACTIVE_TX);
            }
            if((runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxCounterFrIf == 0) && (runtimeChnlData->FrTpTxChannelState == FRTP_FRIF_ERROR))
            {

                PduR_FrTpTxConfirmation(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId, NTFRSLT_E_ABORT);
                FrTp_Internal_FreeChannel(runtimeChnlData);
            }
        }

    }else if(commState == ACTIVE_RX){
        if(runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpFrIfRxErrNextTryTimer != 0){
            runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpFrIfRxErrNextTryTimer--;
        }
        if(runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpFrIfRxErrNextTryTimer == 0){
            if(runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpRxCounterFrIf != 0) {
                runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpRxCounterFrIf--;

                pduInfo.SduLength = FLOW_CONTROL_FRAME_PAYLOAD;
                /*@req FRTP1114 */
                pduInfo.SduDataPtr = &runtimeChnlData->FrTpRxFcDataPdu[0];

                (void)FrTp_Internal_SendFrame(runtimeChnlData, &pduInfo,ACTIVE_RX);
            }

            if((runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpRxCounterFrIf == 0) && (runtimeChnlData->FrTpRxChannelState == FRTP_FRIF_ERROR))
            {
                PduR_FrTpTxConfirmation(runtimeChnlData->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpULTxPduId, NTFRSLT_E_ABORT);
                resetRxChannelConfiguration(runtimeChnlData);
            }
        }

    }else{

    }
}



/**
 * @brief - Handles the pdur transmit cancel and resets as per state
 * @param runtimeChnlData  Internal runtime data ptr for Channel used for transmit
 */
static void FrTp_Internal_HandleTxCanceled(FrTp_Internal_Channel_type* runtimeChnlData) {

#if (FRTP_TRANSMIT_CANCEL_SUPPORT == STD_ON)
    /*@req FRTP1116 */   /*@req FRTP424 */
    /* reset of state is cleared in FrTp_Internal_FreeChannel */
    PduR_FrTpTxConfirmation(runtimeChnlData->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId,NTFRSLT_E_CANCELATION_OK);
    FrTp_Internal_FreeChannel(runtimeChnlData);
#else
    (void)* runtimeChnlData;
#endif
}



/**
 * @brief This function identifies the Frame type
 * @param sdudataPtr
 * @return
 */
static ISO10681FrameType getFrameType(const uint8 *sdudataPtr)
{
    uint8 tpci;
    ISO10681FrameType res;

    tpci = sdudataPtr[PCI_BYTE_1] & ISO10681_TPCI_MASK;
    switch(tpci)
    {
        case ISO10681_TPCI_STF:
            res =START_FRAME;
#if (STD_ON == FRTP_ACK_RETRY_SUPPORT)
            if( STF_ACK_SET == (sdudataPtr[PCI_BYTE_1] & ISO10681_TPCI_FS_MASK))
            {
                res =START_FRAME_ACK;
            }
#endif
            break;
        case ISO10681_TPCI_CF1:
            res =CONSECUTIVE_FRAME_1;
            break;
#if(STD_ON == FRTP_ACK_RETRY_SUPPORT)
        case ISO10681_TPCI_CF2:
            res =CONSECUTIVE_FRAME_2;
            break;
#endif
        case ISO10681_TPCI_CFEOB:
            res =CONSECUTIVE_FRAME_EOB;
            break;
        case ISO10681_TPCI_FC:
            res =FLOW_CONTROL_FRAME;
            break;
        case ISO10681_TPCI_LF:
            res =LAST_FRAME;
            break;
        default:
            res  = INVALID_FRAME;
    }

    return res;
}

//------------------------------------
/**
 *@brief : Function validates the message recived on functional channel
 * @param PduInfoPtr
 * @param frame
 * @return
 */
static boolean validateConnectionType(PduInfoType* PduInfoPtr,ISO10681FrameType frame,uint8 connection_count)
{
    boolean status;
    uint16 messagelength;
    status = TRUE;
    messagelength = (uint16) (PduInfoPtr->SduDataPtr[MSG_LNGTH_MSB]) << SHIFT_1_BYTE;
    messagelength |=  (uint16)PduInfoPtr->SduDataPtr[MSG_LNGTH_LSB];

    if(TRUE == FrTp_ConfigPtr->FrTpConnectionConfig[connection_count].FrTpMultipleReceiverCon) /* Check if its a Functional connection */
    {
        /* @req FRTP598 */
        if(((PduInfoPtr->SduLength - FRTP_HEADER_LENGTH_STF_LF)  != messagelength)
#if(STD_ON == FRTP_ACK_RETRY_SUPPORT)
            || (START_FRAME_ACK == frame)
#endif
            )
        {
            status = FALSE; /*Segmented Msg Reception Not Supported on Functional Connection*/
        }
    }
    if (status == TRUE) {
#if(STD_OFF == FRTP_ACK_RETRY_SUPPORT)
        (void)frame;
#endif
    }
    return status;
}
//------------------------------------
/**
 *@brief : Function resets the channel parameter
 * @param runtimeInternalChnllData
 */
static void resetRxChannelConfiguration(FrTp_Internal_Channel_type *runtimeInternalChnllData)
{
    uint8 initCount;
    runtimeInternalChnllData->FrTpRxChannelState = IDLE;  /* @req FRTP1026 */
    runtimeInternalChnllData->FrTpRxPduAvailable = FALSE;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpRxFcWt = 0;
    runtimeInternalChnllData->FrTpRxSizeBuffer = 0;

    for(initCount=0;initCount<FRTP_PDUARRAY_SIZE;initCount++)
    {
        runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[initCount] = 0;
    }
    for(initCount=0;initCount<FLOW_CONTROL_FRAME_PAYLOAD;initCount++)
    {
        runtimeInternalChnllData->FrTpRxFcDataPdu[initCount] = 0;
    }

#if FRTP_ACK_RETRY_SUPPORT == STD_ON
    runtimeInternalChnllData->FrTpStfAckFlag = FALSE;
    runtimeInternalChnllData->FrTpRxFcRetryFlag = FALSE;
    runtimeInternalChnllData->FrTpEobReceivedFlag = FALSE;
    runtimeInternalChnllData->FrTpStfReceivedFlag = FALSE;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpRxBlockPosition = 0;
#endif
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpCompletedRxMessageLength = 0;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpCounterAr = 0;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpCounterCr = 0;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpFrIfRxErrNextTryTimer = 0;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpRxConTxPduPendingCounter = 0;
    if(runtimeInternalChnllData->FrTpTxChannelState == IDLE){
        FrTp_Internal_RunTime.FrTpConnectionLockStatus[runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpAssignedRxConnectionIndex]= NOT_ACTIVE;
    }
    if(runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpRxConTxPduPoolRuntimeRef != NULL){
        FrTp_Internal_RunTime.FrTpTxPduStatusInfo[runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpRxConTxPduPoolRuntimeRef->FrTpTxPduId].FrTpTxPduLockStatus= AVAILABLE;
    }
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpRxConTxPduPoolRuntimeRef = NULL_PTR;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef = NULL_PTR;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpRxConTxPduPendingCounter = 0;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpRxMessageLength =0;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpCompletedRxMessageLength = 0;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpRemainingRxBufferLength = 0;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpCounterAr = 0;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpArTimeoutRetryCounter = 0;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpCounterBr = 0;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpCounterCr = 0;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpRxCounterFrIf = 0;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTPRxSquenceNumber = 0xFF;
    runtimeInternalChnllData->FrTpChannelParams.FrTpRxChannel.FrTpAssignedRxConnectionIndex = 0;

}
//------------------------------------

/**
 * @brief : Function extracts the Source and Target address
 * @param PduInfoPtr
 * @param frameAddressInfo
 */
static void getFrameAddress(const PduInfoType *PduInfoPtr, ISO10681PduAddressType *frameAddressInfo)
{
    uint16 address;
    address= (uint16)PduInfoPtr->SduDataPtr[TARGET_ADDR_MSB];
    frameAddressInfo->Target_Address = (uint16)((address<<SHIFT_1_BYTE) | (uint16)PduInfoPtr->SduDataPtr[TARGET_ADDR_LSB]);
    address= (uint16)PduInfoPtr->SduDataPtr[SOURCE_ADDR_MSB];
    frameAddressInfo->Source_Address = (uint16)((address<<SHIFT_1_BYTE) | (uint16)PduInfoPtr->SduDataPtr[SOURCE_ADDR_LSB]);
}

//------------------------------------
/**
 * @brief Function sends the flow control Frame as per the flowstautus provided
 * @param ChannelInternal
 * @param flowstatus
 * @param Retry_flag
 */
static void sendFlowControlFrame(FrTp_Internal_Channel_type *ChannelInternal, uint8 flowstatus, boolean Retry_flag) {
    PduInfoType pduInfo;
    uint16 buffersize ;
#if(STD_ON == FRTP_ACK_RETRY_SUPPORT)
    uint16 blockposition;
#endif
    uint8 *sduData;
    uint8 count;
    uint8 count1;
    uint8 indexcount;
    uint8 bandwidthcontrol;
    indexcount = 0;
    (void)Retry_flag;
    pduInfo.SduDataPtr = &ChannelInternal->FrTpRxFcDataPdu[0];
    sduData  = &ChannelInternal->FrTpRxFcDataPdu[0];
    /* FC Frame target Address */ /* @req FRTP1080 */
    sduData[indexcount] = (uint8)((ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpRa & MSB_BYTE_MASK)>>SHIFT_1_BYTE) ;
    indexcount++;
    sduData[indexcount] = (uint8)(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpRa & LSB_BYTE_MASK) ;
    indexcount++;/* FC frane Source address */
    sduData[indexcount] = (uint8)((ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpLa & MSB_BYTE_MASK)>>SHIFT_1_BYTE) ;
    indexcount++;
    sduData[indexcount] = (uint8)(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpLa & LSB_BYTE_MASK) ;
    switch(flowstatus) {
        case ISO10681_FC_STATUS_CTS :
            if(FALSE != ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpBandwidthLimitation){
                bandwidthcontrol =(uint8)(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpMaxNbrOfNPduPerCycle << SHIFT_3_BITS);
                bandwidthcontrol |= ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpSCexp;
            }else{
                bandwidthcontrol = 0;
            }
            buffersize = ChannelInternal->FrTpRxSizeBuffer;
            indexcount++;
            sduData[indexcount] = ISO10681_TPCI_FC | ISO10681_FC_STATUS_CTS;
            indexcount++;
            sduData[indexcount] = bandwidthcontrol;
            indexcount++;
            sduData[indexcount] = (uint8)((buffersize & MSB_BYTE_MASK)>>SHIFT_1_BYTE);
            indexcount++;
            sduData[indexcount] = (uint8)(buffersize & LSB_BYTE_MASK);
            break;
#if(STD_ON == FRTP_ACK_RETRY_SUPPORT)
        case ISO10681_FC_STATUS_ACK_RET :
            indexcount++;
            sduData[indexcount] = ISO10681_TPCI_FC | ISO10681_FC_STATUS_ACK_RET;
            indexcount++;
            sduData[indexcount] = (uint8)Retry_flag;
            if(TRUE == Retry_flag){
                blockposition = ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxBlockPosition;
                ChannelInternal->FrTpRxFcRetryFlag = TRUE;
                indexcount++;
                sduData[indexcount] = (uint8)((blockposition & MSB_BYTE_MASK)>>SHIFT_1_BYTE);
                indexcount++;
                sduData[indexcount] = (uint8)(blockposition & LSB_BYTE_MASK);
            }else{
                indexcount++;
                sduData[indexcount] = 0;
                indexcount++;
                sduData[indexcount] = 0;
            }
        break;
#endif
        case ISO10681_FC_STATUS_WAIT :
               indexcount++;
               sduData[indexcount] = ISO10681_TPCI_FC | ISO10681_FC_STATUS_WAIT;
            break;
        case ISO10681_FC_STATUS_ABORT :
               indexcount++;
               sduData[indexcount] = ISO10681_TPCI_FC | ISO10681_FC_STATUS_ABORT;
            break;
        case ISO10681_FC_STATUS_OVFL :
            indexcount++;
            sduData[indexcount] = ISO10681_TPCI_FC | ISO10681_FC_STATUS_OVFL;
            break;
        default :
            break;
    }
    pduInfo.SduLength = FLOW_CONTROL_FRAME_PAYLOAD;
    count1 = ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpPoolTxPduCnt;
    /* Implementation of FRTP1045 1st free TX PDU*/
    for(count=0; (count < count1); count++)  {
        if ( AVAILABLE == FrTp_Internal_RunTime.FrTpTxPduStatusInfo[ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpTxPduPoolRef[count].FrTpFrIfTxPduId].FrTpTxPduLockStatus){
            FrTp_Internal_RunTime.FrTpTxPduStatusInfo[ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpTxPduPoolRef[count].FrTpFrIfTxPduId].FrTpTxPduLockStatus = LOCKED_FOR_FC;
            FrTp_Internal_RunTime.FrTpTxPduStatusInfo[ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpTxPduPoolRef[count].FrTpFrIfTxPduId].FrTpChannleHandle = ChannelInternal->FrTpChannelNumber;
            ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConTxPduPoolRuntimeRef = &ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpTxPduPoolRef[count];
            FrTp_Internal_RunTime.FrTpTxPduStatusInfo[ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpTxPduPoolRef[count].FrTpFrIfTxPduId].FrTpChannleHandle
            = ChannelInternal->FrTpChannelNumber;
            break;
        }
    }
    if(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConTxPduPoolRuntimeRef != NULL_PTR){
        (void)FrTp_Internal_SendFrame(ChannelInternal, &pduInfo,ACTIVE_RX);
    }else{

        /* @req FRTP1047 */
        ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxPrevChannelState = ChannelInternal->FrTpRxChannelState;
        ChannelInternal->FrTpRxChannelState = FRTP_RX_WAIT_FC_SENDING;
        ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpCounterBr = ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpTimeBr;
    }
}

//------------------------------------

/**
 *@brief : This functions either copys the data to upper layer or terminates the connection based on retun_value provided
 * @param retun_value
 * @param ChannelInternal
 * @return
 */
static BufReq_ReturnType copySegmentToPduRRxBuffer(BufReq_ReturnType action_value,FrTp_Internal_Channel_type *ChannelInternal)
{
    PduInfoType frTpRxSduDataPtr;
    PduIdType frTpRxSduId_lcl;
    BufReq_ReturnType actionLcl;

    actionLcl = BUFREQ_E_NOT_OK;
    switch (action_value)
    {
        case BUFREQ_OK :
                if((FRTP_RX_WAIT_CF == ChannelInternal->FrTpRxChannelState)||(FRTP_EOB_RECEIVED_WAIT_SDU_BUFFER == ChannelInternal->FrTpRxChannelState))
                {     /* @req FRTP1079 */
                    frTpRxSduDataPtr.SduDataPtr = &ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[START_OF_DATA_BYTE_CF];
                }
                else
                {   /* @req FRTP1079 */
                    frTpRxSduDataPtr.SduDataPtr = &ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[START_OF_DATA_BYTE_STF_LF];
                }
                frTpRxSduDataPtr.SduLength  = (PduLengthType)ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[FPL_BYTE];
            /* @req FRTP1138*/
                frTpRxSduId_lcl = ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpULRxPduId;

                actionLcl = PduR_FrTpCopyRxData(frTpRxSduId_lcl, &frTpRxSduDataPtr,&ChannelInternal->FrTpRxSizeBuffer);
                break;

        case BUFREQ_E_NOT_OK:
            /* @req FRTP405 */
                if(FRTP_STF_RECEIVED != ChannelInternal->FrTpRxChannelState)
                {
                    PduR_FrTpRxIndication(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpULRxPduId,NTFRSLT_E_ABORT);
                }
                if(FRTP_RX_WAIT_CF != ChannelInternal->FrTpRxChannelState){
                    sendFlowControlFrame(ChannelInternal,ISO10681_FC_STATUS_ABORT,FALSE);/* Sending FC with ABORT handle  */
                    ChannelInternal->FrTpRxPduAvailable = FALSE;
                }else{
                    resetRxChannelConfiguration(ChannelInternal);
                }
                actionLcl = BUFREQ_E_NOT_OK;
                break;

        case BUFREQ_E_BUSY :
                if((FRTP_RX_WAIT_CF != ChannelInternal->FrTpRxChannelState)){
                    /* @req FRTP1160 */
                    sendFlowControlFrame(ChannelInternal,ISO10681_FC_STATUS_WAIT,FALSE);/* Sending FC with WAIT handle  */
                }else{
                    PduR_FrTpRxIndication(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpULRxPduId,NTFRSLT_E_ABORT);
                    resetRxChannelConfiguration(ChannelInternal);
                }
                actionLcl = BUFREQ_E_BUSY;
                break;

        case BUFREQ_E_OVFL :
                /* @req FRTP1161 */
                if(FRTP_STF_RECEIVED != ChannelInternal->FrTpRxChannelState)
                {
                    PduR_FrTpRxIndication(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpULRxPduId,NTFRSLT_E_ABORT);
                }
                if(FRTP_RX_WAIT_CF != ChannelInternal->FrTpRxChannelState){
                    sendFlowControlFrame(ChannelInternal,ISO10681_FC_STATUS_OVFL,FALSE);/* Sending FC with OVFL handle  */
                    ChannelInternal->FrTpRxPduAvailable = FALSE;
                }else{
                    resetRxChannelConfiguration(ChannelInternal);
                }
                actionLcl = BUFREQ_E_OVFL;
                break;
        default :
            actionLcl =  BUFREQ_E_NOT_OK;
            break;
    }

    return actionLcl;
}

//---------------------------------------------------------

/**
 * @brief This function handles the return value of copySegmentToPduRRxBuffer for STF frame
 * @param retun_value
 * @param ChannelInternal
 * @param stf_ack_flag
 */
static void handlePdurReturn(BufReq_ReturnType retun_value,FrTp_Internal_Channel_type *ChannelInternal, boolean stf_ack_flag)
{
    (void)stf_ack_flag;
    if(BUFREQ_OK == retun_value){
        if((PduLengthType)ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[FPL_BYTE] == ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxMessageLength)/* For Single frame reception*/
        {
            PduR_FrTpRxIndication(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpULRxPduId,NTFRSLT_OK); /* @req FRTP1083 */
#if (STD_ON == FRTP_ACK_RETRY_SUPPORT)
           if(TRUE == stf_ack_flag)   /* for Single Frame with ACK*/
           {
               sendFlowControlFrame(ChannelInternal,ISO10681_FC_STATUS_ACK_RET, FALSE);  /* Sending FC with ACK handle  */
           }else{
               resetRxChannelConfiguration(ChannelInternal);
           }

#else
           resetRxChannelConfiguration(ChannelInternal);
#endif
        }
        else  /* for Segmented Msg Reception frames */
        {
            ChannelInternal->FrTpRxChannelState = FRTP_RX_WAIT_CF;
            sendFlowControlFrame(ChannelInternal,ISO10681_FC_STATUS_CTS,FALSE);

            ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpCompletedRxMessageLength += (PduLengthType) ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[FPL_BYTE];   /* This will be BytePostion in case of SN Error is detected And re-try is required*/
        }

        ChannelInternal->FrTpRxPduAvailable = FALSE;
    }
    if(BUFREQ_E_BUSY == retun_value){
            ChannelInternal->FrTpRxChannelState = FRTP_RX_WAIT_STF_SDU_BUFFER;
    }

}

//------------------------------------

/**
 * @brief Function copies STF frame to local buffer and indicates the upper layer of the start of reception
 * @param ChannelInternal
 * @param PduInfoPtr
 * @param channel
 */
static void handleStartFrame(FrTp_Internal_Channel_type *ChannelInternal,PduInfoType *PduInfoPtr )
{
    ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxMessageLength = ((((PduLengthType)(PduInfoPtr->SduDataPtr[MSG_LNGTH_MSB]) << SHIFT_1_BYTE )) |(PduLengthType) (PduInfoPtr->SduDataPtr[MSG_LNGTH_LSB]) );

    if(PduInfoPtr->SduDataPtr[FPL_BYTE] > MAX_STF_PAYLOAD){
        /* FPL Error in STF , IGNORE the Frame */
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check */
        return;
    }
    /* STF with known msg length*/
    if( 0 != ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxMessageLength)
    {
         /* @req FRTP1077 */
         memcpy(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu, &PduInfoPtr->SduDataPtr[0], PduInfoPtr->SduLength);

        /* @req FRTP399 */ /* @req FRTP1078 */
         ChannelInternal->FrTpUlReturnValue = PduR_FrTpStartOfReception(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpULRxPduId, ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxMessageLength, &ChannelInternal->FrTpRxSizeBuffer );
    }
    else
    {
        /* @req FRTP1077 */ /* @req FRTP1184*/
        memcpy(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu, &PduInfoPtr->SduDataPtr[0], PduInfoPtr->SduLength);
        ChannelInternal->FrTpUlReturnValue = PduR_FrTpStartOfReception(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpULRxPduId, UKNOWN_MSG_LENGHT, &ChannelInternal->FrTpRxSizeBuffer );
    }
    if(BUFREQ_E_BUSY == ChannelInternal->FrTpUlReturnValue)
    {
        if(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpMaxFCWait == 0u){
            resetRxChannelConfiguration(ChannelInternal);
        }else{
            ChannelInternal->FrTpRxChannelState = FRTP_RX_WAIT_STF_SDU_BUFFER;
        }
    }
    else
    {
        ChannelInternal->FrTpRxChannelState = FRTP_STF_RECEIVED;
    }
    ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTPRxSquenceNumber = 0;
}


/**
 *@brief This functions detect if an Sequence error has occurred in CF reception
 * @param ChannelInternal
 * @return
 */
static boolean handelSequenceError(FrTp_Internal_Channel_type *ChannelInternal)
{
     uint8 cf_sequence_number;
     boolean sequenceCorrect;

     sequenceCorrect = TRUE;
     cf_sequence_number = ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[PCI_BYTE_1] & FRTP_MAX_SERIALNUMBER;

#if(STD_ON == FRTP_ACK_RETRY_SUPPORT)

     if(((0 != cf_sequence_number) && (TRUE == ChannelInternal->FrTpRxFcRetryFlag)) ||
             (cf_sequence_number != ((ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTPRxSquenceNumber+FRTP_ONE)%0x10)))
     {
         ChannelInternal->FrTpCounterRxRn++;
         ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTPRxSquenceNumber = 0xFu;    /* This helps in correct retry with Sequence No as 0 */
         if(ChannelInternal->FrTpCounterRxRn <= ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpMaxRn){
             ChannelInternal->FrTpRxFcRetryFlag = TRUE;
             sendFlowControlFrame(ChannelInternal,ISO10681_FC_STATUS_ACK_RET,TRUE);      /* In case of Sequence error For ACK Msg */
             ChannelInternal->FrTpRxPduAvailable = FALSE;
         }else{
             /* @req FRTP1113*/
             PduR_FrTpRxIndication(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpRxSduId, NTFRSLT_E_WRONG_SN);
             resetRxChannelConfiguration(ChannelInternal);
         }
             sequenceCorrect = FALSE;
     }
#else
     if(cf_sequence_number != ((ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTPRxSquenceNumber+FRTP_ONE)%0x10)){

         PduR_FrTpRxIndication(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpRxSduId, NTFRSLT_E_WRONG_SN);
         resetRxChannelConfiguration(ChannelInternal);
         sequenceCorrect = FALSE;
     }
#endif

return sequenceCorrect;
}
//------------------------------------
#if(STD_ON == FRTP_ACK_RETRY_SUPPORT)
static void updateBlockPosition(FrTp_Internal_Channel_type *ChannelInternal)
{

    if(ChannelInternal->FrTpEobReceivedFlag != TRUE)
    {
        ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxBlockPosition += ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[FPL_BYTE];
    }
    else{
        ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxBlockPosition = 0u;
        ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxBlockPosition += ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[FPL_BYTE];
        ChannelInternal->FrTpEobReceivedFlag = FALSE;
    }

}
#endif
//------------------------------------
/**
 * @brief : Function handels the CF frames of segmented msg transmission
 * @param ChannelInternal
 * @param frameType
 */
static void handelconsecutiveframes(FrTp_Internal_Channel_type *ChannelInternal,ISO10681FrameType frameType)
{
    BufReq_ReturnType ret_lcl = BUFREQ_E_NOT_OK;
    static PduInfoType tempPdu;
    boolean sequenceCorrect;
    boolean status;
    status = TRUE;

    sequenceCorrect = handelSequenceError(ChannelInternal);
    /* FPL validity to check */

if(TRUE == sequenceCorrect){
        if((PduLengthType)ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[FPL_BYTE] <= ChannelInternal->FrTpRxSizeBuffer ) /* When CF Payload is more than remaining Buffersize*/
        {
            ret_lcl = copySegmentToPduRRxBuffer(BUFREQ_OK,ChannelInternal);
            if(BUFREQ_OK == ret_lcl){
                ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTPRxSquenceNumber++;
                ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpCompletedRxMessageLength += (PduLengthType)ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[FPL_BYTE];   /* This will be BytePostion in case of SN Error is detected And re try is required*/
#if(STD_ON == FRTP_ACK_RETRY_SUPPORT)
                    if(FALSE != ChannelInternal->FrTpRxFcRetryFlag){
                        ChannelInternal->FrTpRxFcRetryFlag = FALSE;    /* in case this was a successful retry reset the FC retryflag */
                        ChannelInternal->FrTpCounterRxRn  = 0;
                    }
                    updateBlockPosition(ChannelInternal);
#endif
                if(CONSECUTIVE_FRAME_EOB == frameType){
                    ChannelInternal->FrTpRxChannelState = FRTP_EOB_RECEIVED_WAIT_SDU_BUFFER;
#if(STD_ON == FRTP_ACK_RETRY_SUPPORT)
                        ChannelInternal->FrTpEobReceivedFlag = TRUE;
#endif
                    tempPdu.SduLength = 0;
                    tempPdu.SduDataPtr = NULL_PTR;
                    ret_lcl = PduR_FrTpCopyRxData(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpRxSduId, &tempPdu, &ChannelInternal->FrTpRxSizeBuffer);
                    if(BUFREQ_OK == ret_lcl){
                        sendFlowControlFrame(ChannelInternal,ISO10681_FC_STATUS_CTS,FALSE);
                        ChannelInternal->FrTpRxPduAvailable = FALSE;
                        ChannelInternal->FrTpRxChannelState = FRTP_RX_WAIT_CF;
                    }else{
                        if(BUFREQ_E_BUSY== ret_lcl){
                            if(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpMaxFCWait == 0u){
                                resetRxChannelConfiguration(ChannelInternal);
                                status = FALSE;
                            }else{
                                ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxFcWt++;
                            }
                        }
                    }
                }else{
                    ChannelInternal->FrTpRxPduAvailable = FALSE;
                }
            }
        }else{
            ret_lcl = BUFREQ_E_OVFL;
        }
        if(status == TRUE) {
            if(BUFREQ_OK != ret_lcl){
                (void)copySegmentToPduRRxBuffer(ret_lcl,ChannelInternal);
            }
            else{
                /*Execution should not reach here */
            }
        }
    }else{
        ChannelInternal->FrTpRxPduAvailable = FALSE;
    }
}


static void handelLastFrame(FrTp_Internal_Channel_type *ChannelInternal)
{
    uint16 lfMsgLengthCount;
    BufReq_ReturnType ret_lcl = BUFREQ_E_NOT_OK;
    boolean status;
    status = TRUE;

    lfMsgLengthCount = (((uint16)(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[MSG_LNGTH_MSB]) << SHIFT_1_BYTE ) | (uint16)ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[MSG_LNGTH_LSB] );

    if((ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxMessageLength != lfMsgLengthCount) ||
            (lfMsgLengthCount !=  (ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpCompletedRxMessageLength + (PduLengthType) ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[FPL_BYTE])))
    {
        PduR_FrTpRxIndication(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpRxSduId, NTFRSLT_E_FR_ML_MISMATCH);
#if(STD_ON == FRTP_ACK_RETRY_SUPPORT)
           if(TRUE == ChannelInternal->FrTpStfAckFlag){
               sendFlowControlFrame(ChannelInternal,ISO10681_FC_STATUS_ABORT,FALSE);
               ChannelInternal->FrTpRxPduAvailable = FALSE;
           }else{
               resetRxChannelConfiguration(ChannelInternal);
           }
#else
        resetRxChannelConfiguration(ChannelInternal);
#endif
        status = FALSE;
    }

    else if((PduLengthType)ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[FPL_BYTE] <= ChannelInternal->FrTpRxSizeBuffer ) /* When CF Payload is more than remaining Buffersize*/
    {
        ret_lcl = copySegmentToPduRRxBuffer(BUFREQ_OK,ChannelInternal);
        if(BUFREQ_OK == ret_lcl){
            PduR_FrTpRxIndication(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpULRxPduId,NTFRSLT_OK);  /* @req FRTP1083 */
#if(STD_ON == FRTP_ACK_RETRY_SUPPORT)
           if(TRUE == ChannelInternal->FrTpStfAckFlag){
               sendFlowControlFrame(ChannelInternal,ISO10681_FC_STATUS_ACK_RET,FALSE);
               ChannelInternal->FrTpRxPduAvailable = FALSE;
            }
           else{
               resetRxChannelConfiguration(ChannelInternal);
           }
#else
          resetRxChannelConfiguration(ChannelInternal);
#endif
        }
    }else{
        ret_lcl= BUFREQ_E_OVFL;
    }

    if (status == TRUE) {
        if(BUFREQ_OK != ret_lcl){
            if(BUFREQ_E_BUSY == ret_lcl){
#if(STD_ON == FRTP_ACK_RETRY_SUPPORT)
                sendFlowControlFrame(ChannelInternal,ISO10681_FC_STATUS_ABORT,FALSE);/* This will not be sent from copySegmentToPduRRxBuffer for BUSY  */
                ChannelInternal->FrTpRxPduAvailable = FALSE;
#else
              resetRxChannelConfiguration(ChannelInternal);
#endif
            }else{
                (void)copySegmentToPduRRxBuffer(ret_lcl,ChannelInternal);
            }

        }
    }
}

//------------------------------------
/**
 * @brief : Function handles the wait scenarios when buffer is not provided by Upper layer
 * @param ChannelInternal
 */
static void handelpduRwaitcalls(FrTp_Internal_Channel_type *ChannelInternal)
{
    BufReq_ReturnType ret_lcl = BUFREQ_OK;
    static PduInfoType tempPdu;
    uint8 fplLength;

    fplLength = ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[FPL_BYTE];
    switch(ChannelInternal->FrTpRxChannelState)
    {
        case FRTP_RX_WAIT_STF_SDU_BUFFER :
            ret_lcl = PduR_FrTpStartOfReception(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpULRxPduId, ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxMessageLength, &ChannelInternal->FrTpRxSizeBuffer );
            if(BUFREQ_OK == ret_lcl)
            {
                ret_lcl = copySegmentToPduRRxBuffer(BUFREQ_OK, ChannelInternal);
                if(BUFREQ_OK == ret_lcl){
                    ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxFcWt = 0;
                    ChannelInternal->FrTpRxPduAvailable = FALSE;         /* @req FRTP421 */
                    if(fplLength == ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxMessageLength)/* For Single frame reception*/
                    {
                        PduR_FrTpRxIndication(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpRxSduId,NTFRSLT_OK);
#if(STD_ON == FRTP_ACK_RETRY_SUPPORT)
                        if(TRUE == ChannelInternal->FrTpStfAckFlag)   /* for Single Frame with ACK*/
                        {
                            sendFlowControlFrame(ChannelInternal,ISO10681_FC_STATUS_ACK_RET, FALSE);  /* Sending FC with ACK handle  */
                            ChannelInternal->FrTpRxPduAvailable = FALSE;
                        }
#else
                        resetRxChannelConfiguration(ChannelInternal);
#endif
                    }else{
                        ChannelInternal->FrTpRxChannelState = FRTP_RX_WAIT_CF;
                        sendFlowControlFrame(ChannelInternal,ISO10681_FC_STATUS_CTS,FALSE);
                        ChannelInternal->FrTpRxPduAvailable = FALSE;
                        ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpCompletedRxMessageLength += (PduLengthType)ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu[FPL_BYTE];
                    }
                }
            }
            break;
        case FRTP_EOB_RECEIVED_WAIT_SDU_BUFFER :
            tempPdu.SduLength = 0;
            tempPdu.SduDataPtr = NULL_PTR;
            ret_lcl = PduR_FrTpCopyRxData(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpULRxPduId, &tempPdu, &ChannelInternal->FrTpRxSizeBuffer);
            if(BUFREQ_OK == ret_lcl){
                ChannelInternal->FrTpRxChannelState = FRTP_RX_WAIT_CF;
                sendFlowControlFrame(ChannelInternal,ISO10681_FC_STATUS_CTS,FALSE);
                ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxFcWt = 0;
                ChannelInternal->FrTpRxPduAvailable = FALSE;  /* @req FRTP421 */
            }
            break;
        default:
            break;
    }

    if(BUFREQ_E_BUSY == ret_lcl){
        ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxFcWt++;
        if(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxFcWt > ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpMaxFCWait)
        {
            PduR_FrTpRxIndication(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpRxSduId, NTFRSLT_E_WFT_OVRN);
            resetRxChannelConfiguration(ChannelInternal);
        }
        else  /* @req FRTP1113 */
        {
            sendFlowControlFrame(ChannelInternal,ISO10681_FC_STATUS_WAIT,FALSE);
        }
    }
    else if((BUFREQ_E_OVFL == ret_lcl) || (BUFREQ_E_NOT_OK == ret_lcl)){
        (void)copySegmentToPduRRxBuffer(ret_lcl, ChannelInternal);
    }
    else{
        /* Execution should not reach here */
    }
}

/**
 *@brief The function handles Cr Timing Parameter
 * @param ChannelInternal
 */
static void handelCrTimeout(FrTp_Internal_Channel_type *ChannelInternal){
    /* @req FRTP1100 */ /* Time out for CR */
    if(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpCounterCr != 0)
    {
        ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpCounterCr --;
    }
    else
    {
        PduR_FrTpRxIndication(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpULTxPduId, NTFRSLT_E_TIMEOUT_CR);
        resetRxChannelConfiguration(ChannelInternal);
    }
}

/**
 *@brief The function handles Ar Timing Parameter
 * @param ChannelInternal
 */
static void handelArTimeout(FrTp_Internal_Channel_type *ChannelInternal){
    /* @req FRTP1100 */ /* Time out for AR */
    PduInfoType pduInfo;

    if(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpCounterAr != 0){
        ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpCounterAr --;
    }
    if(0 == ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpCounterAr){
        if(0 == ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpArTimeoutRetryCounter){
            PduR_FrTpTxConfirmation(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpULTxPduId, NTFRSLT_E_TIMEOUT_A);
            resetRxChannelConfiguration(ChannelInternal);
        }else{
            /* retry after timeout */
            ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpArTimeoutRetryCounter --;

            pduInfo.SduLength = FLOW_CONTROL_FRAME_PAYLOAD;
            pduInfo.SduDataPtr = &ChannelInternal->FrTpRxFcDataPdu[0];
            ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConTxPduPendingCounter--;
            /* Not searching new Tx Pdu_ID since we already have one locked for our FC Transmission*/
            if(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConTxPduPoolRuntimeRef != NULL_PTR) {
                (void)FrTp_Internal_SendFrame(ChannelInternal, &pduInfo,ACTIVE_RX);
            }

        }
    }
}


static void handelBrTimeout(FrTp_Internal_Channel_type *ChannelInternal){
    /* @req FRTP1100 */ /* Time out for BR */
    PduInfoType pduInfo;
    boolean fcSent;
    uint8 count;
    uint8 txPduPoolCount;
    fcSent = FALSE;

    pduInfo.SduDataPtr = &ChannelInternal->FrTpRxFcDataPdu[0];
    pduInfo.SduLength = FLOW_CONTROL_FRAME_PAYLOAD;

    txPduPoolCount = ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpPoolTxPduCnt;
    /* Implementaion of FRTP1045 1st free TX PDU*/
    for(count=0; (count < txPduPoolCount); count++)  {
        if ( AVAILABLE == FrTp_Internal_RunTime.FrTpTxPduStatusInfo[ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpTxPduPoolRef[count].FrTpFrIfTxPduId].FrTpTxPduLockStatus){
            FrTp_Internal_RunTime.FrTpTxPduStatusInfo[ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpTxPduPoolRef[count].FrTpFrIfTxPduId].FrTpTxPduLockStatus = LOCKED_FOR_FC;
            FrTp_Internal_RunTime.FrTpTxPduStatusInfo[ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpTxPduPoolRef[count].FrTpFrIfTxPduId].FrTpChannleHandle = ChannelInternal->FrTpChannelNumber;
            ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConTxPduPoolRuntimeRef = &ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpTxPduPoolRef[count];
            FrTp_Internal_RunTime.FrTpTxPduStatusInfo[ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpTxPduPoolRef[count].FrTpFrIfTxPduId].FrTpChannleHandle
            = ChannelInternal->FrTpChannelNumber;
            fcSent = TRUE;
            break;
        }
    }
    if(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConTxPduPoolRuntimeRef != NULL_PTR){
        ChannelInternal->FrTpRxChannelState = ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxPrevChannelState;
        (void)FrTp_Internal_SendFrame(ChannelInternal, &pduInfo,ACTIVE_RX);

    }

    ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpCounterBr --;

    if(fcSent == TRUE){
        ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpCounterBr = ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpTimeBr;
    }
    else if(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpCounterBr == 0)
    {
        PduR_FrTpRxIndication(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpULTxPduId, NTFRSLT_E_ABORT);
        resetRxChannelConfiguration(ChannelInternal);
    }else{
        /* do nothing */
    }
}


/**
 * @brief : Function handles flow control frame received when acting as sender.
 * @param ChannelInternal
 * @param PduInfoPtr
 */
static void handelflowcontrolframe(FrTp_Internal_Channel_type *ChannelInternal,PduInfoType* PduInfoPtr )
{

    switch((PduInfoPtr->SduDataPtr[PCI_BYTE_1] & ISO10681_TPCI_FS_MASK))    {
        case ISO10681_FC_STATUS_CTS :
            if(ChannelInternal->FrTpTxChannelState == FRTP_TX_WAIT_FOR_FC){
                ChannelInternal->FrTpFcRecveBuffSize = (((PduLengthType)(PduInfoPtr->SduDataPtr[PCI_BYTE_3]) << SHIFT_1_BYTE)|(PduLengthType)(PduInfoPtr->SduDataPtr[PCI_BYTE_4]));
                ChannelInternal->FrTpBandwidthcontol = PduInfoPtr->SduDataPtr[PCI_BYTE_2];
                if(ChannelInternal->FrTpFcRecveBuffSize == 0) {
#if (FRTP_UNKOWN_MSG_LNGT == STD_ON)
                    if(ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpMessageLength != 0) {
#endif
                        /* If BfS is 0 and remaining message can be sent in one burst so only transmit buffer limits the transmission */
                        ChannelInternal->FrTpFcRecveBuffSize = ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpMessageLength - ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpCompletedMessageLength;
#if (FRTP_UNKOWN_MSG_LNGT == STD_ON)
                    }else{
                        ChannelInternal->FrTpFcRecveBuffSize = 0xFFFF;
                    }
#endif
                }else{
                       if(
#if (FRTP_UNKOWN_MSG_LNGT == STD_ON)
                            (ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpMessageLength != 0) &&
#endif
                            (ChannelInternal->FrTpFcRecveBuffSize > (ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpMessageLength - ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpCompletedMessageLength)) ) {
                                ChannelInternal->FrTpFcRecveBuffSize = ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpMessageLength - ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpCompletedMessageLength;
                            }
                      }
#if (FRTP_ACK_RETRY_SUPPORT == STD_ON)
                /* If FlowControl CTS received the previous block is acknowledged */
                ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpTxBlockPosition = 0;
                ChannelInternal->FrTpRetryInfo.TpDataState = TP_DATACONF;
#endif
                /* Reset Wait Counter */
                ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpTxFcWt = ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpMaxFCWait;
                ChannelInternal->FrTpTxChannelState = FRTP_TX_SEG_ONGOING;
            }
            break;
         case ISO10681_FC_STATUS_ACK_RET:
             FrTp_Internal_HandleRecvdFCAck(ChannelInternal,PduInfoPtr);
          break;
        case ISO10681_FC_STATUS_WAIT:
            if(ChannelInternal->FrTpTxChannelState == FRTP_TX_WAIT_FOR_FC) {
                /* Restart Bs timer */
                ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpCounterBs =  ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpConCtrlRef->FrTpTimeoutBs;
            }
            break;
        case ISO10681_FC_STATUS_ABORT:
            PduR_FrTpTxConfirmation(ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId,NTFRSLT_E_ABORT);
            FrTp_Internal_FreeChannel(ChannelInternal);
            break;
        case ISO10681_FC_STATUS_OVFL:
            PduR_FrTpTxConfirmation(ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId,NTFRSLT_E_NO_BUFFER);
            FrTp_Internal_FreeChannel(ChannelInternal);
            break;
        default:
            /* Abort transmission if invalid FS */
            PduR_FrTpTxConfirmation(ChannelInternal->FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpULTxPduId,NTFRSLT_E_INVALID_FS);
            FrTp_Internal_FreeChannel(ChannelInternal);
            break;
    }
}

/**
 *
 * @return
 */
static uint8 frtpRxStfAssignConnections(PduInfoType* PduInfoPtr,uint8 connection_count,ISO10681FrameType frameTypelcl){

    uint8 count;
    uint8 ret;
    boolean status;
    status = TRUE;
    ret = 0u;
    FrTp_Internal_Channel_type *ChannelInternal;

    if(FrTp_Internal_RunTime.FrTpConnectionLockStatus[connection_count]== NOT_ACTIVE){
        for(count = 0; count< FRTP_CHANNEL_NUM ; count++){
            if(IDLE == FrTp_Internal_RunTime.FrTpChannels[count].FrTpRxChannelState){
                FrTp_Internal_RunTime.FrTpConnectionLockStatus[connection_count] = ACTIVE_RX;
                break;
            }
        }
    }else{

        for(count = 0;count<FRTP_CHANNEL_NUM;count++){
            if((FrTp_Internal_RunTime.FrTpChannels[count].FrTpChannelParams.FrTpRxChannel.FrTpAssignedRxConnectionIndex == connection_count )
                    ||(FrTp_Internal_RunTime.FrTpChannels[count].FrTpChannelParams.FrTpTxChannel.FrTpAssignedConnectionIndex == connection_count)){
                ChannelInternal =  &FrTp_Internal_RunTime.FrTpChannels[count];
#if(STD_OFF == FRTP_FULL_DUPLEX_ENABLE)
                if(ChannelInternal->FrTpTxChannelState != IDLE){
                    ret = FRTP_CHANNEL_NUM;     /* since returning this value will result in DET error E_NO_CHANNEL */
                    status = FALSE;
                    break;
                }
#endif
                if(ChannelInternal->FrTpRxChannelState != IDLE){
                    resetRxChannelConfiguration(ChannelInternal);
                }
                /*lint -e{9011} break is used to terminate the loop*/
                break;
            }
        }
    }
    /*lint -e{774} if evaluated to FALSE depending on FRTP_FULL_DUPLEX_ENABLE is STD_ON or STD_OFF */
    if (status == TRUE) {
        if(count < FRTP_CHANNEL_NUM){
            ChannelInternal =  &FrTp_Internal_RunTime.FrTpChannels[count];
            (ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef) = &FrTp_ConfigPtr->FrTpConnectionConfig[connection_count];

            ChannelInternal->FrTpRxChannelState = BUSY;    /* @req FRTP1025 */
            ChannelInternal->FrTpRxPduAvailable = TRUE;   /* @req FRTP1074 */
             ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpAssignedRxConnectionIndex = connection_count;

            ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxCounterFrIf = ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpMaxFrIf;
            /* Set Ar Counter To Max */
            ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpArTimeoutRetryCounter = ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpMaxAr;
#if(STD_ON == FRTP_ACK_RETRY_SUPPORT)
            if(START_FRAME_ACK == frameTypelcl)
            {
                ChannelInternal->FrTpStfAckFlag = TRUE;
            }
#else
            (void)frameTypelcl;
#endif
            handleStartFrame(ChannelInternal,PduInfoPtr);
        }
    } else {
        count = ret;
    }
    /*lint -e{438} ret is updated depending on FRTP_FULL_DUPLEX_ENABLE is STD_ON or STD_OFF */
    return count;
}


//------------------------------------
/**
 * @brief : Function Manages the connection for the STF frame received.
 * @param PduInfoPtr
 * @param RxPduId
 * @param frameTypelcl
 * @param frameAddressInfo
 * @return
 */
static uint8 frtpManageStfConnections(PduInfoType* PduInfoPtr,PduIdType RxPduId,ISO10681FrameType frameTypelcl,
                                    ISO10681PduAddressType frameAddressInfo)
{
    uint8 count;
    uint8 pduCnt;
    uint8 connection_count;
   boolean frtpconnectionfound;
   boolean frameValidity;
   connection_count = INVALID_CONNECTION;

   frtpconnectionfound = FALSE;
   for(count= 0; (count < FrTp_ConfigPtr->FrTpConnectionCount ) && (FALSE==frtpconnectionfound);count++)
   {
       if((frameAddressInfo.Target_Address == FrTp_ConfigPtr->FrTpConnectionConfig[count].FrTpLa)
           && (frameAddressInfo.Source_Address == FrTp_ConfigPtr->FrTpConnectionConfig[count].FrTpRa))
       {   /* @req FRTP1069 */
           for(pduCnt  = 0; (pduCnt  < FrTp_ConfigPtr->FrTpConnectionConfig[count].FrTpRxPduRefCount) && (FALSE==frtpconnectionfound); pduCnt ++)
           {
              if(RxPduId == FrTp_ConfigPtr->FrTpConnectionConfig[count].FrTpRxPduPoolRef[pduCnt].FrTpRxPduId){
                  frtpconnectionfound = TRUE;  /**/
                  connection_count    = count;/* @req FRTP1071*/
               }
           }
           /* @req FRTP1075*/
           FRTP_DET_REPORTERROR((frtpconnectionfound == TRUE),FRTP_SERVICE_ID_RX_INDICATION,FRTP_E_INVALID_PDU_SDU_ID,connection_count);
       }
   }


   if(frtpconnectionfound==TRUE){
       frameValidity =  validateConnectionType(PduInfoPtr,frameTypelcl,connection_count);
       if(frameValidity == TRUE){
           connection_count = frtpRxStfAssignConnections(PduInfoPtr,connection_count,frameTypelcl);
       }


   }
   return connection_count;
}
/**
 * @brief : Function Manages the connection for the CF frame received.
 * @param PduInfoPtr
 * @param frameTypelcl
 * @param frameAddressInfo
 */
static void frtpManageCfConnections(PduInfoType* PduInfoPtr, ISO10681FrameType frameTypelcl, ISO10681PduAddressType frameAddressInfo)
{
    FrTp_Internal_Channel_type *ChannelInternal;
    uint8 channel_count;

    for(channel_count = 0; channel_count< FRTP_CHANNEL_NUM; channel_count++)  {
        if(FrTp_Internal_RunTime.FrTpChannels[channel_count].FrTpChannelParams.FrTpRxChannel.FrTpRxConRef!=NULL_PTR){
            if((frameAddressInfo.Target_Address == FrTp_Internal_RunTime.FrTpChannels[channel_count].FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpLa)
               && (frameAddressInfo.Source_Address == FrTp_Internal_RunTime.FrTpChannels[channel_count].FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpRa))  {
                if((FRTP_RX_WAIT_CF == FrTp_Internal_RunTime.FrTpChannels[channel_count].FrTpRxChannelState)) {
                   ChannelInternal = &FrTp_Internal_RunTime.FrTpChannels[channel_count];
                   ChannelInternal->FrTpRxPduAvailable = TRUE;   /* @req FRTP1074*/
                   memcpy(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu, &PduInfoPtr->SduDataPtr[0], PduInfoPtr->SduLength);
                   /* Restart Cr timer */
                   ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpCounterCr = ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpTimeoutCr;
                   if(LAST_FRAME == frameTypelcl){
                       ChannelInternal->FrTpRxChannelState = RX_LAST_FRAME;  /* Assigning it here helps in FrTp_CancelReceive function */
                       /* Set Ar Counter To Max */ /*@req FRTP1114 */
                       ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpArTimeoutRetryCounter = ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpMaxAr;
                       /* Last Frame Received Stop this timer */
                       ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpCounterCr = 0u;
                   }else if(CONSECUTIVE_FRAME_EOB == frameTypelcl){
                       /* Set Ar Counter To Max */
                       ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpArTimeoutRetryCounter = ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConRef->FrTpConCtrlRef->FrTpMaxAr;
                   }else{
                       /* do nothing */
                   }
                   break;
                }
            }
         }
    }
}

/**
 * @brief : Function Manages the connection for the FC frame received.
 * @param PduInfoPtr
 * @param frameTypelcl
 * @param frameAddressInfo
 */
static void frtpManageFcConnections(PduInfoType* PduInfoPtr, ISO10681PduAddressType frameAddressInfo)
{
    FrTp_Internal_Channel_type *ChannelInternal;
    uint8 channel_count;

    for(channel_count = 0; channel_count< FRTP_CHANNEL_NUM; channel_count++)  {
        if(FrTp_Internal_RunTime.FrTpChannels[channel_count].FrTpChannelParams.FrTpTxChannel.FrTpTxConRef != NULL_PTR){
            if((frameAddressInfo.Target_Address == FrTp_Internal_RunTime.FrTpChannels[channel_count].FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpLa)
               && (frameAddressInfo.Source_Address == FrTp_Internal_RunTime.FrTpChannels[channel_count].FrTpChannelParams.FrTpTxChannel.FrTpTxConRef->FrTpRa)){
                   ChannelInternal = &FrTp_Internal_RunTime.FrTpChannels[channel_count];
                   handelflowcontrolframe(ChannelInternal,PduInfoPtr);
                   break;
            }
        }
    }
}

/**
 *@brief : This function handles the Frames received.
 */
static void handleReceivedFrame(FrTp_Internal_Channel_type *ChannelInternal)
{
    BufReq_ReturnType ret_lcl;
    ISO10681FrameType frameTypelcl;

    if(TRUE == ChannelInternal->FrTpRxPduAvailable){
        /*Check if any data copy to PDUR is Pending */
        if((ChannelInternal->FrTpRxChannelState == FRTP_RX_WAIT_STF_SDU_BUFFER )||(ChannelInternal->FrTpRxChannelState == FRTP_EOB_RECEIVED_WAIT_SDU_BUFFER)){
            handelpduRwaitcalls(ChannelInternal);    /* @req FRTP1136 */
        }else{
            frameTypelcl = getFrameType(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxDataPdu);
            switch (frameTypelcl){
                    case START_FRAME :
                            ret_lcl = copySegmentToPduRRxBuffer(ChannelInternal->FrTpUlReturnValue,ChannelInternal);
                            if((BUFREQ_OK == ret_lcl)||(BUFREQ_E_BUSY == ret_lcl)){
                                handlePdurReturn(ret_lcl,ChannelInternal,FALSE);            /*last parameter is FALSE since STF without ACK*/
                            }
                        break;
#if (STD_ON == FRTP_ACK_RETRY_SUPPORT)
                    case START_FRAME_ACK :
                            ret_lcl = copySegmentToPduRRxBuffer(ChannelInternal->FrTpUlReturnValue,ChannelInternal);
                            if((BUFREQ_OK == ret_lcl)||(BUFREQ_E_BUSY == ret_lcl)){
                                handlePdurReturn(ret_lcl,ChannelInternal,TRUE);            /*last parameter is TRUE since STF with ACK*/
                            }
                        break;
#endif
                    case CONSECUTIVE_FRAME_1:
                            handelconsecutiveframes(ChannelInternal,frameTypelcl);
                        break;
#if(STD_ON == FRTP_ACK_RETRY_SUPPORT)
                    case CONSECUTIVE_FRAME_2:
                            handelconsecutiveframes(ChannelInternal,frameTypelcl);
                        break;
#endif
                    case CONSECUTIVE_FRAME_EOB:
                            handelconsecutiveframes(ChannelInternal,frameTypelcl);
                        break;
                    case LAST_FRAME:
                            handelLastFrame(ChannelInternal);
                        break;
                    case INVALID_FRAME:
                        break;
                    default:
                        /*Can come here only for FC frame which is handled directly in Rx_indication */
                        break;
             }
        }
    } else {
    if(FRTP_RX_WAIT_CF == ChannelInternal->FrTpRxChannelState){
        if(ChannelInternal->FrTpChannelParams.FrTpRxChannel.FrTpRxConTxPduPendingCounter != 0){
            /*Manage Ar*/
            handelArTimeout(ChannelInternal);
        }else{
            /*Manage Cr*/
            handelCrTimeout(ChannelInternal);
        }
    }else if(FRTP_RX_WAIT_FC_SENDING == ChannelInternal->FrTpRxChannelState){
        handelBrTimeout(ChannelInternal);
    }else if(FRTP_FRIF_ERROR == ChannelInternal->FrTpRxChannelState){
            FrTp_Internal_HandleFrIfError(ChannelInternal,ACTIVE_RX);
    }else{
            /* do nothing */
        }
    }
}




#ifdef HOST_TEST
FrTp_Internal_RunTimeType* readInternal_FrTpChnlStatus(void);

FrTp_Internal_RunTimeType* readInternal_FrTpChnlStatus(void){

    return &FrTp_Internal_RunTime ;

}
#endif
/*lint -restore */
