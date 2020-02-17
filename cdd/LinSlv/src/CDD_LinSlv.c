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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */
/** @tagSettings DEFAULT_ARCHITECTURE=MPC5645S */

/* @req CDD_LinSlv_00001 The sum of all functionality of the module fulfills the LIN 2.1 specification */
/* @req CDD_LinSlv_00003 Supports the LinFlex hardware */
/* @req CDD_LinSlv_00008 If an error is found it will return */
/* @req CDD_LinSlv_00010 Input parameters of public functions have DET validation */


/* ----------------------------[includes]------------------------------------*/
#include "Lin.h"
#include "LinIf.h"
#include "CDD_LinSlv.h"
#include "CDD_LinSlv_Internal.h"
#include "PduR_CDD_LinSlvIf.h"

#if defined(USE_DET)
#include "Det.h"
#endif
/* ----------------------------[private define]------------------------------*/
#if defined(CFG_MPC5645S)
#define LIN_FLEX
#endif

#define LINSLV_MAX_MSG_LENGTH 8

#define NAD_IDX     0u
#define PCI_IDX     1u
#define SID_IDX     2u
#define ID_IDX      3u

#define MASK_PCI_SF_MAX    (uint8)0x06u

#define MASK_GO_TO_SLEEP    (uint8)0x00u
#define MASK_WILDCARD_NAD   (uint8)0x7Fu
#define MASK_FUNCTIONAL_NAD (uint8)0x7E
#define MASK_BROADCAST_NAD  MASK_WILDCARD_NAD

#define MASK_SID_ASSIGN_FRAME_ID_RANGE  (uint8)0xB7u
#define MASK_SID_READ_BY_IDENTIFIER     (uint8)0xB2u

#define WILDCARD_SUPPLIER_LSB   (uint8)0xFFu
#define WILDCARD_SUPPLIER_MSB   (uint8)0x7Fu
#define WILDCARD_FUNCTION_LSB   (uint8)0xFFu
#define WILDCARD_FUNCTION_MSB   (uint8)0xFFu

#define ID_MSB_BYTE(_x) ((uint8)(((_x) >> 8u) & 0xFFu))
#define ID_LSB_BYTE(_x) ((uint8)((_x) & 0xFFu))
#define ID_SUPPLIER_LSB_IDX 4u
#define ID_SUPPLIER_MSB_IDX 5u
#define ID_FUNCTION_LSB_IDX 6u
#define ID_FUNCTION_MSB_IDX 7u

#define AFBI_IDX_IDX                3u
#define AFBI_DONT_CARE              (uint8)0xFFu
#define AFBI_MAX_NBR_ASSIGNMENTS    4u
#define AFBI_PID_START_IDX          4u
/* ----------------------------[private macro]-------------------------------*/
/* Development error macros. */
/* @req CDD_LinSlv_00009 */
#if ( LINSLV_DEV_ERROR_DETECT == STD_ON )
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(CDD_LINSLV_MODULE_ID,0,_api,_err); \
          return; \
        }

#define VALIDATE_W_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(CDD_LINSLV_MODULE_ID,0,_api,_err); \
          return (_rv); \
        }
#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_W_RV(_exp,_api,_err,_rv )
#endif
/* ----------------------------[private typedef]-----------------------------*/
typedef enum {
    CONFIGURED_NAD,
    FUNCTIONAL_NAD,
    BROADCAST_NAD,
    GO_TO_SLEEP_NAD,
    INVALID_NAD
}LinSlvNadType;

typedef enum {
    SID_ASSIGN_FRAME_BY_ID,
    SID_READ_BY_IDENTIFIER,
    SID_GO_TO_SLEEP,
    SID_INVALD,
}LinSlvSidType;
/* ----------------------------[private function prototypes]-----------------*/
static void timeOutMonitor(uint8 Channel);
static void sleepCalloutCheck(uint8 Channel);

void receivedNewTpRequest(uint8 Channel, const uint8 *buffer);
void safeCopyData(uint8 Channel);
boolean handleTpRequest(uint8 Channel);
void parseTpRequest(uint8 Channel, const uint8 *tpFrame, LinSlvNadType *nadCaller, boolean *pciOk, LinSlvSidType *reqService);
Std_ReturnType handleTpReadById(uint8 Channel, const uint8 *tpFrame);
Std_ReturnType handleTpAssignFrameById(uint8 Channel, const uint8 *tpFrame);
void setSafeTpResponse(uint8 Channel, const uint8 *tpFrame);
void handleSrfTimeOut(uint8 Channel, boolean newSrfSet);
void handleWakeUpNotification(uint8 Channel);
/* ----------------------------[private variables]---------------------------*/
static LinSlv_ContainerType LinSlv_Cfg = {0}; /*lint !e910 Set all members to 0*/
static Lin_DriverStatusType LinSlvDriverStatus = LIN_UNINIT;
static boolean txConfIsExpected[LINSLV_CHANNEL_CNT] = {0};
static const LinIf_FrameType *lastTransmittedFrame[LINSLV_CHANNEL_CNT] = {0}; /*lint !e910 Set all members to 0*/
static uint8 Response_error[LINSLV_CHANNEL_CNT] = {0}; /* As named in the Lin specification */
static boolean wakeUpSet[LINSLV_CHANNEL_CNT] = {0};
static boolean wakeUpNotif[LINSLV_CHANNEL_CNT] = {0};

static boolean busActivity[LINSLV_CHANNEL_CNT] = {0};
static boolean sleepNotificationReq[LINSLV_CHANNEL_CNT] = {0};
static uint32 busTimeOutCnt[LINSLV_CHANNEL_CNT] = {0};
static boolean oldSleepNotificationReq[LINSLV_CHANNEL_CNT] = {0};
static uint32 srfTimeCnt[LINSLV_CHANNEL_CNT];
static uint32 wakeUpCnt[LINSLV_CHANNEL_CNT] = {0};

uint8 mrfBuffer[LINSLV_CHANNEL_CNT][LINSLV_MAX_MSG_LENGTH];
uint8 mrfShadowBuffer[LINSLV_CHANNEL_CNT][LINSLV_MAX_MSG_LENGTH];
uint8 srfBuffer[LINSLV_CHANNEL_CNT][LINSLV_MAX_MSG_LENGTH];
boolean mrfReceived[LINSLV_CHANNEL_CNT] = {0};
boolean srfPending[LINSLV_CHANNEL_CNT] = {0};

#if (LINSLV_TRCV_SUPPORT == STD_ON)
LinTrcv_TrcvModeType lastSetTrcvState[LINSLV_CHANNEL_CNT];
#endif
/* ----------------------------[functions]-----------------------------------*/

/**
 * Initialize the Lin slave driver
 * @param Config
 */
void LinSlv_Init(const LinSlv_ConfigType* Config) {
    Std_ReturnType retval;
    uint8 i;

    VALIDATE( (LinSlvDriverStatus == LIN_UNINIT), LINSLV_INIT_SERVICE_ID, LINSLV_E_STATE_TRANSITION );
    VALIDATE( ( Config != NULL ), LINSLV_INIT_SERVICE_ID, LINSLV_E_INVALID_POINTER);
    // Validate sub-pointers
    VALIDATE( ( Config->LinSlvChannelConfig      != NULL ), LINSLV_INIT_SERVICE_ID, LINSLV_E_INVALID_POINTER);
    VALIDATE( ( Config->LinSlv_HwId2ChannelMap   != NULL ), LINSLV_INIT_SERVICE_ID, LINSLV_E_INVALID_POINTER);
    VALIDATE( ( Config->LinSlvAttrib             != NULL ), LINSLV_INIT_SERVICE_ID, LINSLV_E_INVALID_POINTER);
    VALIDATE( ( Config->LinSlvChannelFrameConfig != NULL ), LINSLV_INIT_SERVICE_ID, LINSLV_E_INVALID_POINTER);
    VALIDATE( ( Config->LinSlvPDUFrameCollection != NULL ), LINSLV_INIT_SERVICE_ID, LINSLV_E_INVALID_POINTER);
    VALIDATE( ( Config->LinSlvChnTrcvConfig      != NULL ), LINSLV_INIT_SERVICE_ID, LINSLV_E_INVALID_POINTER);

    LinSlv_Cfg.LinSlvCfgPtr = Config;

    retval = LinSlv_Internal_Init(&LinSlv_Cfg);

    // Set DET error if it could not finish the initialization
    VALIDATE( (retval != E_NOT_OK ), LINSLV_INIT_SERVICE_ID, LINSLV_E_INVALID_CONFIG);

#if (LINSLV_TRCV_SUPPORT == STD_ON)
    for (i = 0; i < LINSLV_CHANNEL_CNT; i++) {
        if (LinSlv_Cfg.LinSlvCfgPtr->LinSlvChnTrcvConfig[i].LinSlvTrcvIsConfigured) {
            retval = LinSlv_Internal_SetTrcvMode(&LinSlv_Cfg, i, LinSlv_Cfg.LinSlvCfgPtr->LinSlvChnTrcvConfig[i].LinSlvTrcvInitState);
            // Set DET error if it could not finish the initialization
            VALIDATE( (retval != E_NOT_OK ), LINSLV_INIT_SERVICE_ID, LINSLV_E_INVALID_CONFIG);

            lastSetTrcvState[i] = LinSlv_Cfg.LinSlvCfgPtr->LinSlvChnTrcvConfig[i].LinSlvTrcvInitState;
        }
    }
#endif

    // Clear internal variables
    for (i = 0; i < LINSLV_CHANNEL_CNT; i++) {
        txConfIsExpected[i] = 0;
        lastTransmittedFrame[i] = NULL;
        Response_error[i] = 0;
        wakeUpSet[i] = FALSE;
        wakeUpNotif[i] = FALSE;
        busActivity[i] = FALSE;
        sleepNotificationReq[i] = FALSE;
        busTimeOutCnt[i] = 0;
        oldSleepNotificationReq[i] = FALSE;
        srfPending[i] = FALSE;
        srfTimeCnt[i] = 0;
        wakeUpCnt[i] = 0;
    }

    if (E_OK == retval) {
        LinSlvDriverStatus = LIN_INIT;
    }
}

/**
 * DeInit the driver.
 */
void LinSlv_DeInit(void) {

    VALIDATE( (LinSlvDriverStatus == LIN_INIT), LINSLV_DEINIT_SERVICE_ID, LINSLV_E_STATE_TRANSITION );

    LinSlv_Internal_Deinit(&LinSlv_Cfg);

    LinSlvDriverStatus = LIN_UNINIT;
}

/**
 * Set the channel in sleep.
 * @param Channel
 * @return
 */
Std_ReturnType LinSlv_GoToSleepInternal(uint8 Channel) {

    VALIDATE_W_RV( (LinSlvDriverStatus != LIN_UNINIT), LINSLV_GOTOSLEEPINTERNAL_SERVICE_ID, LINSLV_E_UNINIT, E_NOT_OK);
    VALIDATE_W_RV( (Channel < LINSLV_CHANNEL_CNT) , LINSLV_GOTOSLEEPINTERNAL_SERVICE_ID, LINSLV_E_INVALID_CHANNEL, E_NOT_OK);

    LinSlv_Internal_GoToSleepInternal(&LinSlv_Cfg, Channel);

    LinSlv_Cfg.LinSlvChannelStatus[Channel] = LIN_CH_SLEEP;

    return E_OK;
}

/**
 * Set the Lin channel to operational state
 * @param Channel
 * @return
 */
Std_ReturnType LinSlv_WakeupInternal(uint8 Channel) {

    VALIDATE_W_RV( (LinSlvDriverStatus != LIN_UNINIT), LINSLV_WAKEUPINTERNAL_SERVICE_ID, LINSLV_E_UNINIT, E_NOT_OK);
    VALIDATE_W_RV( (Channel < LINSLV_CHANNEL_CNT)      , LINSLV_WAKEUPINTERNAL_SERVICE_ID, LINSLV_E_INVALID_CHANNEL, E_NOT_OK);
    VALIDATE_W_RV( (LinSlv_Cfg.LinSlvChannelStatus[Channel] == LIN_CH_SLEEP), LINSLV_WAKEUPINTERNAL_SERVICE_ID, LINSLV_E_STATE_TRANSITION, E_NOT_OK);

    LinSlv_Internal_WakeupInternal(&LinSlv_Cfg, Channel);

    LinSlv_Cfg.LinSlvChannelStatus[Channel] = LIN_OPERATIONAL;
    wakeUpSet[Channel] = FALSE;

    return E_OK;
}

/**
 * Generates a wake up pulse and sets the channel state to operational.
 * @param Channel
 * @return
 */
Std_ReturnType LinSlv_Wakeup(uint8 Channel) {
    Std_ReturnType retval;

    VALIDATE_W_RV( (LinSlvDriverStatus != LIN_UNINIT), LINSLV_WAKEUP_SERVICE_ID, LINSLV_E_UNINIT, E_NOT_OK);
    VALIDATE_W_RV( (Channel < LINSLV_CHANNEL_CNT)    , LINSLV_WAKEUP_SERVICE_ID, LINSLV_E_INVALID_CHANNEL, E_NOT_OK);
    VALIDATE_W_RV( (LinSlv_Cfg.LinSlvChannelStatus[Channel] == LIN_CH_SLEEP), LINSLV_WAKEUP_SERVICE_ID, LINSLV_E_STATE_TRANSITION, E_NOT_OK);

    retval = LinSlv_Internal_Wakeup(&LinSlv_Cfg, Channel);

    if (E_OK == retval) {
        LinSlv_Cfg.LinSlvChannelStatus[Channel] = LIN_OPERATIONAL;
        wakeUpSet[Channel] = FALSE;
        wakeUpNotif[Channel] = TRUE;
    };

    return retval;
}

/**
 * Checks if the wakeup source have caused the wakeup. If it has it will
 * call EcuM_SetWakeupEvent.
 * @param WakeupSource
 * @return
 */
Std_ReturnType LinSlv_CheckWakeup(EcuM_WakeupSourceType WakeupSource) {
    const Lin_ChannelConfigType *chnCfg;
    boolean wakeupSourceFound = FALSE;
    uint8 chnCnt;

    VALIDATE_W_RV( (LinSlvDriverStatus != LIN_UNINIT), LINSLV_CHECKWAKEUP_SERVICE_ID, LINSLV_E_UNINIT, E_NOT_OK);

    for (chnCnt = 0; chnCnt < LINSLV_CHANNEL_CNT; chnCnt++) {
        chnCfg = &LinSlv_Cfg.LinSlvCfgPtr->LinSlvChannelConfig[chnCnt].LinSlvChnConfig;

        if (wakeUpSet[chnCnt])
        {
            // Always set to false after it has been read out.
            wakeUpSet[chnCnt] = FALSE;

            wakeupSourceFound = TRUE;
        }

#if (LINSLV_TRCV_SUPPORT == STD_ON)
        if (LinSlv_Cfg.LinSlvCfgPtr->LinSlvChnTrcvConfig[chnCnt].LinSlvTrcvIsConfigured) {
            LinTrcv_TrcvModeType currentTrcvState;
            Std_ReturnType status;

            status = LinSlv_Internal_GetTrcvMode(&LinSlv_Cfg, chnCnt, &currentTrcvState);
            if ((E_OK == status) && (LINTRCV_TRCV_MODE_SLEEP == lastSetTrcvState[chnCnt]) &&
                    (LINTRCV_TRCV_MODE_STANDBY == currentTrcvState))
            {
                //Set lastSetTrcvState to standby so that no wakeup event can be reported twice
                lastSetTrcvState[chnCnt] = LINTRCV_TRCV_MODE_STANDBY;
                wakeupSourceFound = TRUE;
            }
        }
#endif

        if (chnCfg->LinChannelWakeUpSupport && (chnCfg->LinChannelEcuMWakeUpSource == WakeupSource) && wakeupSourceFound) {
            EcuM_SetWakeupEvent(WakeupSource);
            break;
        }

    }
    return E_OK;
}

/**
 * Returns the LinSlv peripheral status.
 * @param Channel
 * @return
 */
Lin_StatusType LinSlv_GetStatus(uint8 Channel) {

    VALIDATE_W_RV( (LinSlvDriverStatus != LIN_UNINIT), LINSLV_GETSTATUS_SERVICE_ID, LINSLV_E_UNINIT, LIN_NOT_OK);
    VALIDATE_W_RV( (Channel < LINSLV_CHANNEL_CNT)    , LINSLV_GETSTATUS_SERVICE_ID, LINSLV_E_INVALID_CHANNEL, LIN_NOT_OK);

    return LinSlv_Internal_GetStatus(&LinSlv_Cfg, Channel);
}


/**
 * Receive interrupt routine. The routine is filter based and assumes that the full frame has been received.
 * @param HwChannelId
 */
void LinSlv_Internal_Rx_Isr(uint8 hwChannelId) {
    Std_ReturnType retval;
    PduInfoType outgoingPdu;
    uint8 sduBuffer[LINSLV_MAX_MSG_LENGTH];
    const LinIf_FrameType *frameCfgPtr = NULL;
    uint8 channel = LinSlv_Cfg.LinSlvCfgPtr->LinSlv_HwId2ChannelMap[hwChannelId];

    busActivity[channel] = TRUE;

    if (LIN_CH_SLEEP == LinSlv_Cfg.LinSlvChannelStatus[channel])
    {
        wakeUpSet[channel] = LinSlv_Internal_IsWakeUpSet(&LinSlv_Cfg, channel);

        if (wakeUpSet[channel] && LinSlv_Cfg.LinSlvCfgPtr->LinSlvChannelConfig[channel].LinSlvChnConfig.LinChannelWakeUpSupport) {
            /* Report wakeup to EcuM */
            /* @req CDD_LinSlv_00011 */
            EcuM_CheckWakeup(LinSlv_Cfg.LinSlvCfgPtr->LinSlvChannelConfig[channel].LinSlvChnConfig.LinChannelEcuMWakeUpSource);
        }
        return;
    }


    retval = LinSlv_Hw_Rx(&LinSlv_Cfg, channel, &frameCfgPtr, sduBuffer);

    /* If E_NOT_OK it could not find a valid configured frame for the received identifier */
    if (E_OK == retval) {

        outgoingPdu.SduDataPtr = sduBuffer;
        outgoingPdu.SduLength = (PduLengthType)frameCfgPtr->LinIfLength;

        if (MRF == frameCfgPtr->LinIfFrameType) {
            receivedNewTpRequest(channel, sduBuffer);
        } else {
            /* @req CDD_LinSlv_00007 */
            PduR_LinSlvIfRxIndication((PduIdType)frameCfgPtr->LinIfTxTargetPduId,&outgoingPdu);
        }
    }

}

/**
 * Parse and depending on header set response data.
 * @param hwChannelId
 */
static Std_ReturnType transmitResponse(uint8 channel) {
    const LinSlv_ChannelConfigType *chnCfg = &LinSlv_Cfg.LinSlvCfgPtr->LinSlvChannelConfig[channel];
    const LinIf_FrameType *frameCfgPtr = NULL;
    uint8 sduBuffer[LINSLV_MAX_MSG_LENGTH];
    PduInfoType outgoingPdu;
    Std_ReturnType retval;
    const uint8 *bufPtr;

    /* Based on id in the received header, get the corresponding frame ptr */
    retval = LinSlv_Hw_Tx_GetFramePtr(&LinSlv_Cfg, channel, &frameCfgPtr);

    /* If E_NOT_OK it could not find a valid configured frame for the received identifier */
    if (E_OK == retval) {

        if (SRF ==  frameCfgPtr->LinIfFrameType) {
            bufPtr = srfBuffer[channel];
            retval = (TRUE == srfPending[channel]) ? E_OK : E_NOT_OK;
        } else { /* Unconditional frame */
            outgoingPdu.SduDataPtr = sduBuffer;
            outgoingPdu.SduLength = (PduLengthType)frameCfgPtr->LinIfLength;

            /* @req CDD_LinSlv_00007 */
            retval = PduR_LinSlvIfTriggerTransmit((PduIdType)frameCfgPtr->LinIfTxTargetPduId,&outgoingPdu);

            /* If it is the frame containing the signal Response error */
            /* @req CDD_LinSlv_00006 */
            if (chnCfg->LinSlvRespErrorFrameRef == frameCfgPtr) {
                /* Clear and set the Response error */
                sduBuffer[chnCfg->LinSlvRespErrByteIdx] &= ~(1u << chnCfg->LinSlvRespErrBitIdxInByte);
                /*lint -e{701, 734} False positive detected, it is not signed. All types are of uint8, no loss of precision. */
                sduBuffer[chnCfg->LinSlvRespErrByteIdx] |= (Response_error[channel] << chnCfg->LinSlvRespErrBitIdxInByte);
            }
            bufPtr = sduBuffer;
        }

        if (E_OK == retval) {
            /* Set data for transmission */
            LinSlv_Internal_SetResponse(&LinSlv_Cfg, channel, frameCfgPtr->LinIfLength, bufPtr);
            lastTransmittedFrame[channel] = frameCfgPtr;
        }
    }

    return retval;
}

/**
 * Handle tx interrupts
 * @param hwChannelId
 */
void LinSlv_Internal_Tx_Isr(uint8 hwChannelId) {
    uint8 channel = LinSlv_Cfg.LinSlvCfgPtr->LinSlv_HwId2ChannelMap[hwChannelId];
    const LinSlv_ChannelConfigType *chnCfg = &LinSlv_Cfg.LinSlvCfgPtr->LinSlvChannelConfig[channel];
    LinSlv_TxIsrSourceType source;
    Std_ReturnType retval;

    source = LinSlv_Hw_Tx_GetIsrSource(&LinSlv_Cfg, channel);

    if (LINSLV_HEADER == source) {
        retval = transmitResponse(channel);
        txConfIsExpected[channel] = (E_OK == retval) ? TRUE : FALSE;
    } else if ((LINSLV_TRANSMIT_DONE == source) && txConfIsExpected[channel]) {

        if ((lastTransmittedFrame[channel] != NULL) && (UNCONDITIONAL == lastTransmittedFrame[channel]->LinIfFrameType)) {
            /* @req CDD_LinSlv_00007 */
            PduR_LinSlvIfTxConfirmation((PduIdType)lastTransmittedFrame[channel]->LinIfTxTargetPduId);

            /* After successful transmit of Response error signal it shall be cleared */
            /* @req CDD_LinSlv_00006 */
            if (chnCfg->LinSlvRespErrorFrameRef == lastTransmittedFrame[channel]) {
                Response_error[channel] = 0u;
            }
        } else if ((lastTransmittedFrame[channel] != NULL) && (SRF == lastTransmittedFrame[channel]->LinIfFrameType)) {
            srfPending[channel] = FALSE;
        } else {
            ;
        }
        txConfIsExpected[channel] = FALSE;
    } else {
        ;
    }

    LinSlv_Hw_ClearTxIsrFlags(&LinSlv_Cfg, channel);
    busActivity[channel] = TRUE;
}


/**
 * Handle the error interrupts
 * @param hwChannelId
 */
void LinSlv_Internal_Err_Isr(uint8 hwChannelId) {
    uint8 channel = LinSlv_Cfg.LinSlvCfgPtr->LinSlv_HwId2ChannelMap[hwChannelId];
    Std_ReturnType retval;

    txConfIsExpected[channel] = FALSE;

    retval = LinSlv_Hw_Err(&LinSlv_Cfg, channel);

    if (E_NOT_OK == retval) {
        /* @req CDD_LinSlv_00006 */
        Response_error[channel] = 1u;
    }
}

#if (LINSLV_VERSION_INFO_API == STD_ON)
/**
 * Returns the version info of the module
 * @param versioninfo
 */
void LinSlv_GetVersionInfo(Std_VersionInfoType* versioninfo)
{
    VALIDATE( !(versioninfo == NULL), LINSLV_GETVERSIONINFO_SERVICE_ID, LINSLV_E_PARAM_POINTER );

    versioninfo->vendorID = CDD_LINSLV_VENDOR_ID;
    versioninfo->moduleID = CDD_LINSLV_MODULE_ID;
    versioninfo->sw_major_version = CDD_LINSLV_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = CDD_LINSLV_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = CDD_LINSLV_SW_PATCH_VERSION;
}
#endif


/**
 * The main processing function of the LinSlv Interface
 * The LinSlv_MainFunction shall be called by the environment every LinSlvTimeBase second.
 */
void LinSlv_MainFunction(void) {
    boolean srfNewSet;
    uint8 chnIdx;

    VALIDATE( (LinSlvDriverStatus != LIN_UNINIT), LINSLV_MAINFUNCTION_SERVICE_ID, LINSLV_E_UNINIT);

    for (chnIdx = 0; chnIdx < LINSLV_CHANNEL_CNT; chnIdx++) {
        srfNewSet = FALSE;

        if (LIN_OPERATIONAL == LinSlv_Cfg.LinSlvChannelStatus[chnIdx]) {

            /* Check if a requested wakeup has been transmitted - send wakeup confirmation */
            handleWakeUpNotification(chnIdx);

            /* Check bus communication timeout */
            timeOutMonitor(chnIdx);

            /* Check if any new MRF frame has been received */
#if defined(LIN_FLEX)
            // IMPROVEMENT add this check on the internal driver level.
            // This check of driver status is important as one of the Tp services requires
            // setting the peripheral into init mode for reconfiguration,
            // which means that we to temporary disable frame tx/rx.
            Lin_StatusType driverStatus = LinSlv_Internal_GetStatus(&LinSlv_Cfg, chnIdx);

            if (mrfReceived[chnIdx] && LinSlv_Cfg.LinSlvCfgPtr->LinSlvAttrib[chnIdx].LinSlvAttribIsConfigured &&
                ((LIN_OPERATIONAL == driverStatus) || (LIN_CH_SLEEP == driverStatus)))
            {
                mrfReceived[chnIdx] = FALSE;
                srfNewSet = handleTpRequest(chnIdx);
            }
#else
            if (mrfReceived[chnIdx] && LinSlv_Cfg.LinSlvCfgPtr->LinSlvAttrib[chnIdx].LinSlvAttribIsConfigured) {
                mrfReceived[chnIdx] = FALSE;
                srfNewSet = handleTpRequest(chnIdx);
            }
#endif
            /* Check if we have any pending srf and check timeout for discard */
            handleSrfTimeOut(chnIdx, srfNewSet);


            /* Check if busTimeOut or GoToSleep - Send sleep notification */
            sleepCalloutCheck(chnIdx);
        }

    }
}

/**
 * If no bus activity is detected the counter starts and continuous to count until the limit is reached.
 * @param Channel
 */
static void timeOutMonitor(uint8 Channel) {
    const LinSlv_ChannelConfigType *chnCfg = &LinSlv_Cfg.LinSlvCfgPtr->LinSlvChannelConfig[Channel];

    if (0uL < chnCfg->LinSlvBusTimeOutCnt) {

        if (!busActivity[Channel]) {

            if (busTimeOutCnt[Channel] < (chnCfg->LinSlvBusTimeOutCnt -1uL)) {
                busTimeOutCnt[Channel]++;
            } else {
                /* @req CDD_LinSlv_00013 */
                sleepNotificationReq[Channel] = TRUE;
            }
        } else {
            busTimeOutCnt[Channel] = 0;
            busActivity[Channel] = FALSE;
        }
    }
}

/**
 * Calls the sleep callout functions.
 * @param Channel
 */
static void sleepCalloutCheck(uint8 Channel) {
    const LinSlv_ChannelConfigType *chnCfg = &LinSlv_Cfg.LinSlvCfgPtr->LinSlvChannelConfig[Channel];

    /* Only at value change will sleepNotificationRequest be evaluated*/
    if (oldSleepNotificationReq[Channel] != sleepNotificationReq[Channel]) {
        oldSleepNotificationReq[Channel] = sleepNotificationReq[Channel];

        /* @req CDD_LinSlv_00012 */
        /* @req CDD_LinSlv_00013 */
        if (sleepNotificationReq[Channel] && (chnCfg->LinSlvSleepCallback != NULL)) {
            //Sleep callback function.
            chnCfg->LinSlvSleepCallback(LINSLV_BUS_TIMEOUT);
        }
    }
    sleepNotificationReq[Channel] = FALSE;
}





/**
 * Copy of data in interrupt to the MRF buffer
 * @param Channel
 * @param buffer
 */
void receivedNewTpRequest(uint8 Channel, const uint8 *buffer) {
    uint8 i;

    VALIDATE( ( NULL !=  buffer), LINSLV_MAINFUNCTION_SERVICE_ID, LINSLV_E_INVALID_POINTER);

    for (i = 0; i <LINSLV_MAX_MSG_LENGTH; i++) {
        mrfBuffer[Channel][i] = buffer[i];
    }

    mrfReceived[Channel] = TRUE;
}

/**
 * Safe copy of MRF data to a shadow buffer to ensure data consistency
 * @param Channel
 */
void safeCopyData(uint8 Channel) {
    uint8 i;

    LinSlv_Internal_StoreAndDisableIsr(&LinSlv_Cfg, Channel);

    for (i = 0; i <LINSLV_MAX_MSG_LENGTH; i++) {
        mrfShadowBuffer[Channel][i] = mrfBuffer[Channel][i];
    }

    LinSlv_Internal_RestoreAndEnableIsr(&LinSlv_Cfg, Channel);
}


/**
 * The main function for handling and processing of TP frames.
 * @param Channel
 * @return Set to true if a new slave response frame was set.
 */
/* @req CDD_LinSlv_00002 */
boolean handleTpRequest(uint8 Channel) {
    Std_ReturnType retval;
    const uint8 *dataPtr;
    LinSlvNadType nadCallType = INVALID_NAD;
    LinSlvSidType reqService = SID_INVALD;
    boolean pciOk = FALSE;

    safeCopyData(Channel);

    dataPtr = mrfShadowBuffer[Channel];

    parseTpRequest(Channel, dataPtr, &nadCallType, &pciOk, &reqService);

    if (((FUNCTIONAL_NAD == nadCallType) && srfPending[Channel]) || (!pciOk && (GO_TO_SLEEP_NAD != nadCallType))) {
        /* Ignore the request */
        return FALSE;
    } else if (INVALID_NAD == nadCallType) {
        /* New request on the bus, but not for this node. Still have to cancel any pending SRF we might have */
        srfPending[Channel] = FALSE;
        return FALSE;
    } else {
        // Cancel any pending SRF
        srfPending[Channel] = FALSE;
    }

    switch(reqService) {
        case SID_GO_TO_SLEEP:
            /* @req CDD_LinSlv_00012 */
            if (LinSlv_Cfg.LinSlvCfgPtr->LinSlvChannelConfig[Channel].LinSlvSleepCallback != NULL) {
                //Sleep callback function.
                LinSlv_Cfg.LinSlvCfgPtr->LinSlvChannelConfig[Channel].LinSlvSleepCallback(LINSLV_GO_TO_SLEEP_REQ);
            }
            break;
        case SID_ASSIGN_FRAME_BY_ID:
            retval = handleTpAssignFrameById(Channel, dataPtr);
            if (E_OK == retval) {
                srfPending[Channel] = TRUE;
            }
            break;
        case SID_READ_BY_IDENTIFIER:
            retval = handleTpReadById(Channel, dataPtr);
            if (E_OK == retval) {
                srfPending[Channel] = TRUE;
            }
            break;
        default:
            break;
    }

    return srfPending[Channel];
}

/**
 * Parses the NAD and SID of the MRF frame to determine node addressing type
 * and requested service.
 * @param Channel
 * @param tpFrame
 * @return
 */
void parseTpRequest(uint8 Channel, const uint8 *tpFrame, LinSlvNadType *nadCaller, boolean *pciOk, LinSlvSidType *reqService) {

    VALIDATE( ( NULL !=  tpFrame), LINSLV_MAINFUNCTION_SERVICE_ID, LINSLV_E_INVALID_POINTER);
    VALIDATE( ( NULL !=  nadCaller), LINSLV_MAINFUNCTION_SERVICE_ID, LINSLV_E_INVALID_POINTER);
    VALIDATE( ( NULL !=  pciOk), LINSLV_MAINFUNCTION_SERVICE_ID, LINSLV_E_INVALID_POINTER);
    VALIDATE( ( NULL !=  reqService), LINSLV_MAINFUNCTION_SERVICE_ID, LINSLV_E_INVALID_POINTER);

    /* Validate Node address */
    if (LinSlv_Cfg.LinSlvCfgPtr->LinSlvAttrib[Channel].LinSlvConfiguredNad == tpFrame[NAD_IDX]) {
        *nadCaller = CONFIGURED_NAD;
    } else if (MASK_FUNCTIONAL_NAD == tpFrame[NAD_IDX]) {
        *nadCaller = FUNCTIONAL_NAD;
    } else if (MASK_BROADCAST_NAD == tpFrame[NAD_IDX]) {
        *nadCaller = BROADCAST_NAD;
    } else if (MASK_GO_TO_SLEEP == tpFrame[NAD_IDX]) {
        *nadCaller = GO_TO_SLEEP_NAD;
    } else {
        *nadCaller = INVALID_NAD;
    }

    // A Single Frame PDU (SF) with a length value greater than six (6) bytes shall be
    // ignored by the receiver.
    if (MASK_PCI_SF_MAX < tpFrame[PCI_IDX]) {
        *pciOk = FALSE;
    } else {
        *pciOk = TRUE;
    }

    /* Sid */
    if (GO_TO_SLEEP_NAD == *nadCaller) {
        *reqService = SID_GO_TO_SLEEP;
    } else if (MASK_SID_ASSIGN_FRAME_ID_RANGE == tpFrame[SID_IDX]) {
        *reqService = SID_ASSIGN_FRAME_BY_ID;
    } else if (MASK_SID_READ_BY_IDENTIFIER == tpFrame[SID_IDX]) {
        *reqService = SID_READ_BY_IDENTIFIER;
    } else {
        *reqService = SID_INVALD;
    }
}

/**
 * Handle Tp request Read By Identifier
 * @param Channel
 * @param tpFrame
 */
Std_ReturnType handleTpReadById(uint8 Channel, const uint8 *tpFrame) {
    const LinSlv_ReadByIdentifierFrameType *readByIdCfg = &LinSlv_Cfg.LinSlvCfgPtr->LinSlvChannelFrameConfig[Channel].LinSlvReadByIdFrameCfg[0];
    uint8 negativResp[LINSLV_MAX_MSG_LENGTH] = {LinSlv_Cfg.LinSlvCfgPtr->LinSlvAttrib->LinSlvConfiguredNad, 0x03, 0x7F, 0xB2, 0x12, 0xFF, 0xFF, 0xFF};
    const uint32 nbrCfgIds = LinSlv_Cfg.LinSlvCfgPtr->LinSlvChannelFrameConfig[Channel].LinSlvChnReadByIdCnt;
    const uint16 functionIdCfg = LinSlv_Cfg.LinSlvCfgPtr->LinSlvAttrib->LinSlvFunctionId;
    const uint16 supplierIdCfg = LinSlv_Cfg.LinSlvCfgPtr->LinSlvAttrib->LinSlvSupplierId;
    boolean isFuncIdValid = FALSE;
    boolean isSupIdValid = FALSE;
    boolean isIdValid = FALSE;
    boolean isLengthValid = FALSE;
    uint32 i;
    uint8 *rspFrame = NULL;
    uint8 rspFrameLen = 0;

    VALIDATE_W_RV( ( NULL !=  tpFrame), LINSLV_MAINFUNCTION_SERVICE_ID, LINSLV_E_INVALID_POINTER, E_NOT_OK);

    /* Validate identifier */
    for (i = 0; i < nbrCfgIds; i++) {
        if (readByIdCfg[i].LinSlvIdentifier == tpFrame[ID_IDX]) {
            isIdValid = TRUE;
            break;
        }
    }

    /* Validate Supplier Id */
    if ((ID_LSB_BYTE(supplierIdCfg) == tpFrame[ID_SUPPLIER_LSB_IDX]) &&
        (ID_MSB_BYTE(supplierIdCfg) == tpFrame[ID_SUPPLIER_MSB_IDX]))
    {
        isSupIdValid = TRUE;
    }
    else if ((WILDCARD_SUPPLIER_LSB == tpFrame[ID_SUPPLIER_LSB_IDX]) &&
        (WILDCARD_SUPPLIER_MSB == tpFrame[ID_SUPPLIER_MSB_IDX]))
    {
        isSupIdValid = TRUE;
    } else {
        ; // Misra compliance
    }

    /* Validate Function Id */
    if ((ID_LSB_BYTE(functionIdCfg) == tpFrame[ID_FUNCTION_LSB_IDX]) &&
        (ID_MSB_BYTE(functionIdCfg) == tpFrame[ID_FUNCTION_MSB_IDX]))
    {
        isFuncIdValid = TRUE;
    }
    else if ((WILDCARD_FUNCTION_LSB == tpFrame[ID_FUNCTION_LSB_IDX]) &&
            (WILDCARD_FUNCTION_MSB == tpFrame[ID_FUNCTION_MSB_IDX]))
    {
        isFuncIdValid = TRUE;
    } else {
        ; // Misra compliance
    }

    /* Validate PCI length and type */
    if (MASK_PCI_SF_MAX == tpFrame[PCI_IDX]) {
        isLengthValid = TRUE;
    }

    /* Set response */
    if (isIdValid && isSupIdValid && isFuncIdValid && isLengthValid) {
        const LinSlv_ReadByIdentifierFrameType *readByIdFrame = &readByIdCfg[i];

        /* @req CDD_LinSlv_00014 */
        if (NULL != readByIdFrame->LinSlvResponseCallback) {
            readByIdFrame->LinSlvResponseCallback(&rspFrame, &rspFrameLen);
        }

        /* Validate response frame length */
        if ((NULL != rspFrame) && (LINSLV_MAX_MSG_LENGTH != rspFrameLen)) {
            /* Invalid request, set negative response */
            setSafeTpResponse(Channel, negativResp);
        }
        else {
            setSafeTpResponse(Channel, rspFrame);
        }
    } else {
        /* Invalid request, set negative response */
        setSafeTpResponse(Channel, negativResp);
    }

    return E_OK;
}

/**
 * Handles the assign frame by id service.
 * @param Channel
 * @param tpFrame
 */
Std_ReturnType handleTpAssignFrameById(uint8 Channel, const uint8 *tpFrame) {
    uint8 positiveResp[LINSLV_MAX_MSG_LENGTH] = {LinSlv_Cfg.LinSlvCfgPtr->LinSlvAttrib->LinSlvConfiguredNad, 0x01, 0xF7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8 chnMaxCnt = LinSlv_Cfg.LinSlvCfgPtr->LinSlvChannelFrameConfig[Channel].LinSlvUnconditionalFrameCnt;
    uint8 nbrAllowedFrameReAssignments;
    boolean isAssignmentInRange = TRUE;
    Std_ReturnType retval = E_NOT_OK;
    boolean isLengthValid = FALSE;
    uint8 currentPidAssignment;
    uint8 startIndex;
    uint8 i;

    VALIDATE_W_RV( ( NULL !=  tpFrame), LINSLV_MAINFUNCTION_SERVICE_ID, LINSLV_E_INVALID_POINTER, E_NOT_OK);

    /* Validate PCI length and type */
    if (MASK_PCI_SF_MAX == tpFrame[PCI_IDX]) {
        isLengthValid = TRUE;
    }

    startIndex = tpFrame[AFBI_IDX_IDX];

    /* Validate if all PID (index + n) can be reconfigured */
    nbrAllowedFrameReAssignments = (chnMaxCnt >= startIndex) ? (chnMaxCnt-startIndex) : (uint8)0u;

    for (i = 0; i < AFBI_MAX_NBR_ASSIGNMENTS; i++) {
        currentPidAssignment = tpFrame[AFBI_PID_START_IDX + i];

        // Lin 2.1 specification: "frames with frame identifiers 60 (0x3C) to 63(0x3F) can not be changed"
        if ((0x3Cu <= currentPidAssignment) && (0x3Fu >= currentPidAssignment)) {
            isAssignmentInRange = FALSE;
        }

        if ((i >= nbrAllowedFrameReAssignments) && (AFBI_DONT_CARE != currentPidAssignment)) {
            isAssignmentInRange = FALSE;
        }
    }

    // The frame is valid, update filter rules and set positive response
    if (isLengthValid && isAssignmentInRange) {

        for (i = 0; i < AFBI_MAX_NBR_ASSIGNMENTS; i++) {
            if (AFBI_DONT_CARE != tpFrame[AFBI_PID_START_IDX + i]) {
                LinSlv_Internal_UpdatePid(&LinSlv_Cfg, Channel, (startIndex + i), tpFrame[AFBI_PID_START_IDX + i]);
            }
        }

        setSafeTpResponse(Channel, positiveResp);
        retval = E_OK;
    }

    return retval;
}

/**
 * Safely copies the data to the buffer used the the interrupt routine.
 * @param Channel
 * @param tpFrame
 */
void setSafeTpResponse(uint8 Channel, const uint8 *tpFrame) {
    uint8 i;

    VALIDATE( ( NULL !=  tpFrame), LINSLV_MAINFUNCTION_SERVICE_ID, LINSLV_E_INVALID_POINTER);

    LinSlv_Internal_StoreAndDisableIsr(&LinSlv_Cfg, Channel);

    for (i = 0; i <LINSLV_MAX_MSG_LENGTH; i++) {
        srfBuffer[Channel][i] = tpFrame[i];
    }

    LinSlv_Internal_RestoreAndEnableIsr(&LinSlv_Cfg, Channel);
}


/**
 * Handles the SRF timeout
 * @param Channel
 * @param newSrfSet
 */
void handleSrfTimeOut(uint8 Channel, boolean newSrfSet) {

    if (newSrfSet || !srfPending[Channel]) {
        srfTimeCnt[Channel] = 0;
    } else {
        srfTimeCnt[Channel]++;
    }

    //Timeout, discard the response
    if (srfTimeCnt[Channel] >= (LINSLV_SRF_DISCARD_CNT-1uL)) {
        srfPending[Channel] = FALSE;
    }
}

/**
 * Checks the peripheral status if a wakeup notification shall be sent.
 * @param Channel
 */
void handleWakeUpNotification(uint8 Channel) {
    const LinSlv_ChannelConfigType *chnCfg;
    boolean retval;
    boolean forceSetFailure = FALSE;

    chnCfg = &LinSlv_Cfg.LinSlvCfgPtr->LinSlvChannelConfig[Channel];

    // If no wakeup notification is configured for the channel
    // there is no point doing anything more in here.
    if (NULL == chnCfg->LinSlvWakeupConfirmation) {
        return;
    }

    // If a call to LinSlv_Wakeup has been done
    if (wakeUpNotif[Channel]) {
        retval = LinSlv_Internal_IsWakeUpDoneTransmitting(&LinSlv_Cfg, Channel);

        /*lint -e{685,778} It will depend on the parameter LinSlvTimeBase */
        if (wakeUpCnt[Channel] < (LINSLV_WAKEUP_TIMEOUT_CNT-1uL)) {
            wakeUpCnt[Channel]++;
        } else {
            forceSetFailure = TRUE;
        }

        if (retval) {
            chnCfg->LinSlvWakeupConfirmation(TRUE);
            wakeUpNotif[Channel] = FALSE;
        } else if (forceSetFailure) {
            chnCfg->LinSlvWakeupConfirmation(FALSE);
            wakeUpNotif[Channel] = FALSE;
        } else {
            ;
        }
    } else {
        wakeUpCnt[Channel] = 0;
    }

}

#if (LINSLV_TRCV_SUPPORT == STD_ON)
/**
 * Set transceiver mode for the specified channel.
 * @param Channel
 * @param TransceiverMode
 * @return
 */
Std_ReturnType LinSlv_SetTrcvMode(uint8 Channel, LinTrcv_TrcvModeType TransceiverMode) {
    Std_ReturnType retval = E_NOT_OK;

    VALIDATE_W_RV( (LinSlvDriverStatus != LIN_UNINIT), LINSLV_SETTRCVMODE_SERVICE_ID, LINSLV_E_UNINIT, E_NOT_OK);
    VALIDATE_W_RV( (Channel < LINSLV_CHANNEL_CNT)    , LINSLV_SETTRCVMODE_SERVICE_ID, LINSLV_E_INVALID_CHANNEL, E_NOT_OK);
    VALIDATE_W_RV( ((TransceiverMode == LINTRCV_TRCV_MODE_NORMAL)  ||
                    (TransceiverMode == LINTRCV_TRCV_MODE_STANDBY) ||
                    (TransceiverMode == LINTRCV_TRCV_MODE_SLEEP)), LINSLV_SETTRCVMODE_SERVICE_ID, LINSLV_E_STATE_TRANSITION, E_NOT_OK);

    if (LinSlv_Cfg.LinSlvCfgPtr->LinSlvChnTrcvConfig->LinSlvTrcvIsConfigured) {
        retval = LinSlv_Internal_SetTrcvMode(&LinSlv_Cfg, Channel, TransceiverMode);

        if (E_OK == retval) {
            lastSetTrcvState[Channel] = TransceiverMode;
        }
    }

    return retval;
}

/**
 * Returns the transceiver mode for the specified channel.
 * @param Channel
 * @param TransceiverModePtr
 * @return
 */
Std_ReturnType LinSlv_GetTrcvMode(uint8 Channel, LinTrcv_TrcvModeType* TransceiverModePtr) {
    Std_ReturnType retval = E_NOT_OK;

    VALIDATE_W_RV( (LinSlvDriverStatus != LIN_UNINIT), LINSLV_GETTRCVMODE_SERVICE_ID, LINSLV_E_UNINIT, E_NOT_OK);
    VALIDATE_W_RV( (Channel < LINSLV_CHANNEL_CNT)    , LINSLV_GETTRCVMODE_SERVICE_ID, LINSLV_E_INVALID_CHANNEL, E_NOT_OK);
    VALIDATE_W_RV( ( TransceiverModePtr != NULL )    , LINSLV_GETTRCVMODE_SERVICE_ID, LINSLV_E_INVALID_POINTER, E_NOT_OK);

    if (LinSlv_Cfg.LinSlvCfgPtr->LinSlvChnTrcvConfig->LinSlvTrcvIsConfigured) {
        retval = LinSlv_Internal_GetTrcvMode(&LinSlv_Cfg, Channel, TransceiverModePtr);
    }

    return retval;
}
#endif
