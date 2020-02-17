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
/*-------------------------------- Note ------------------------------------
 * Some additions that are outside the Autosar standard has been added.
 * 1) We are able to send the announce message.
 * This is regulated with a switch in EthTSyn_Cfg.h.
 * The switch is ETHTSYN_SEND_ANNOUNCE_SUPPORT set it to STD_ON for the announce
 * message to be transmitted.
 * 2) We are able to receive multiple peer delay requests.
 * If a switch is used that do not have support for peer delay request handling
 * there will be multiple pdelay_req messages received to the master. Functionality
 * has been added so these are identified as different requests and handled.
 * Really this should not be necessary because this would essentially be a end to
 * end system and we should instead accept multiple delay_req.
 * Improvement: In future alter acceptance of multiple pdelay_req to accept multiple delay_req.
 *-------------------------------- Note ------------------------------------*/

#include "EthTSyn.h"
#include "StbM.h"

#if (ETHTSYN_DEV_ERROR_DETECT == STD_ON)
#if defined(USE_DET)
#include "Det.h"
#else
#error "EthTSyn: DET must be used when Default error detect is enabled"
#endif
#endif
#include "EthIf.h"
#include "EthTSyn_Internal.h"
#include <string.h>

//lint -emacro(904,ETHTSYN_DET_REPORTERROR) //904 PC-Lint exception to MISRA 14.7 (validate DET macros).

#define ETHTSYN_INVALID_TIMEDOMAIN   0xFFu
#define ETHTSYN_INVALID_CTRLID       0xFFu
#define ETHTSYN_MESSAGE_TYPE         0x0Fu

const EthTSyn_ConfigType* EthTSyn_ConfigPointer;

/** Static declarations */
static EthTSyn_Internal_DomainType EthTSyn_InternalDomains[ETHTSYN_TIMEDOMAIN_COUNT];
/* @req 4.2.2/SWS_EthTSyn_00002 */
const uint8 EthTSyn_Internal_MulitCast_DestMACAddrs[6] = {0x01,0x80,0xc2,0x00,0x00,0x0E};/* Destination MAC address for PTP */

EthTSyn_InternalType EthTSyn_Internal = {
        .initStatus = ETHTSYN_STATE_UNINIT,
        .timeDomain = EthTSyn_InternalDomains,
};

/* @req 4.2.2/SWS_EthTSyn_00007 */
#if (ETHTSYN_DEV_ERROR_DETECT == STD_ON)
#define ETHTSYN_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        (void)Det_ReportError(ETHTSYN_MODULE_ID, 0, _api, _error); \
        return __VA_ARGS__; \
    }
#else
#define ETHTSYN_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        return __VA_ARGS__; \
    }
#endif

#define ETHTSYN_GET_TIMEDOMAINID(_timedomain)               \
    (EthTSynConfigData.EthTSynGlobalTimeDomain[_timedomain].EthTSynSynchronizedTimeBaseRef)

#define ETHTSYN_GET_CTRL_IDX(_timedomain)                \
    (EthTSynConfigData.EthTSynGlobalTimeDomain[_timedomain].EthTSynTimeDomain->EthTSynGlobalTimeEthIfId)

#if (ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON)
/**
 * Gives back with mapped Time Domain ID
 * @param TimeDomainId
 * @return uint16
 */
static uint16 EthTSyn_Internal_GetTimeDomainStbm ( uint16 TimeDomainId){

    uint16 status;
    status = ETHTSYN_INVALID_TIMEDOMAIN;
    for (uint8 i = 0; i < ETHTSYN_TIMEDOMAIN_COUNT; i++) {
        if(TimeDomainId == ETHTSYN_GET_TIMEDOMAINID(i)){
            status = TimeDomainId;
            break;
        }
    }
    return status;
}
#endif

/**
 * Gives back with mapped Ethernet interface controller index
 * @param CtrlIdx
 * @return controller Id
 */
static uint8 EthTSyn_Internal_CheckEthIfCtrl ( uint8 CtrlIdx){

    uint8 status;
    status = (uint8)ETHTSYN_INVALID_CTRLID;
    for (uint8 i = 0; i < ETHTSYN_TIMEDOMAIN_COUNT; i++) {
        if(CtrlIdx == ETHTSYN_GET_CTRL_IDX(i) ) {
            status = CtrlIdx;
            break;
        }
    }
    return status;
}

/**
 * Gives back with mapped Time Domain ID of Ethernet interface controller index
 * @param CtrlIdx
 * @return uint16
 */
static uint16 EthTsyn_Internal_GetTimeDomainId ( uint8 CtrlIdx){

    uint16 status;
    status = ETHTSYN_INVALID_TIMEDOMAIN;
    for (uint8 i = 0; i < ETHTSYN_TIMEDOMAIN_COUNT; i++) {
        if(CtrlIdx == ETHTSYN_GET_CTRL_IDX(i) ) {
            status = (uint16)i;
            break;
        }
    }
    return status;
}

/** AUTOSAR APIs */
/**
 * Init function for EthTSyn
 * @param configPtr
 */
/* @req 4.2.2/SWS_EthIf_00024 */
void EthTSyn_Init( const EthTSyn_ConfigType* configPtr )
{
    uint8 i,j;
    uint8 ctrlIdx;
    ETHTSYN_DET_REPORTERROR((NULL != configPtr),ETHTSYN_SERVICE_ID_INIT, ETHTSYN_E_INIT_FAILED);

    EthTSyn_ConfigPointer = configPtr;

    /* @req 4.2.2/SWS_EthTSyn_00006 */ /* @req 4.2.2/SWS_EthTSyn_00008 */ /*@req 4.2.2/SWS_EthTSyn_00010 */
    for(i =0 ; i <  EthTSyn_ConfigPointer->EhtTSyn_GlobalTimeDomainCount; i ++){
        /* Sync and Follow Up msg transmission related Variable init */
        EthTSyn_Internal.timeDomain[i].ptpCfgData.syncSend_State = SEND_SYNC_MSG;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.sentSync = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.txSyncTimePending = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.last_sync_tx_sequence_number = 0;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.syncIntervalTimer = 0;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.syncIntervalTimerStarted = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.syncIntervalTimeout = TRUE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.syncfollowUpTimer = 0;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.syncfollowUpTimerStarted = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.syncfollowUpTimerTimeout = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.t1SyncTxTime = (Eth_TimeStampType){0};
#if (ETHTSYN_SEND_ANNOUNCE_SUPPORT == STD_ON)
        EthTSyn_Internal.timeDomain[i].ptpCfgData.announceIntervalTimer = 0;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.announceIntervalTimerStarted = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.announceIntervalTimeout = TRUE;
#endif
        /* Sync and FUP Receive related variable init */
        EthTSyn_Internal.timeDomain[i].ptpCfgData.syncReceive_State = DISCARD;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.rcvdSync = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.rcvdFollowUp = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.receptionTimeout = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.parent_last_sync_sequence_number = 0;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.syncCorrection = 0;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.followupCorrection = 0;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.waitingForFollow = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.RxFollowUpTimerStarted = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.initSyncFollowReceived = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.RxFollowUpTimer = 0;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.t2SyncReceiptTime = (Eth_TimeStampType){0};
        EthTSyn_Internal.timeDomain[i].ptpCfgData.t2SyncReceiptTime_old = (Eth_TimeStampType){0};
        EthTSyn_Internal.timeDomain[i].ptpCfgData.t1SyncOriginTime = (Eth_TimeStampType){0};
        EthTSyn_Internal.timeDomain[i].ptpCfgData.t1SyncOriginTime_old = (Eth_TimeStampType){0};

        /* Pdelay Req sent related variable init */
        EthTSyn_Internal.timeDomain[i].ptpCfgData.pDelayReq_State = INITIAL_SEND_PDELAY_REQ;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.sentPdelayReq = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.isMeasuringDelay = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.pdelayTimerStarted = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.pdelayIntervalTimer = 0;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.pdelayIntervalTimeout = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.txPdelayReqTimePending = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.initPdelayRespReceived = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.t1PdelayReqTxTime = (Eth_TimeStampType){0};
        EthTSyn_Internal.timeDomain[i].ptpCfgData.numPdelay_req = 0;
        for (j=0; j<ETHTSYN_MAX_PDELAY_REQUEST; j++) {
            EthTSyn_Internal.timeDomain[i].ptpCfgData.t2PdelayReqRxTime[j] = (Eth_TimeStampType){0};
        }
        /* Pdelay Resp and FUP sent related variable init */
        for (j=0; j<ETHTSYN_MAX_PDELAY_REQUEST; j++) {
            EthTSyn_Internal.timeDomain[i].ptpCfgData.pDelayResp_State[j] = INITIAL_WAITING_FOR_PDELAY_REQ;
            EthTSyn_Internal.timeDomain[i].ptpCfgData.rcvdPdelayReq[j] = FALSE;
            EthTSyn_Internal.timeDomain[i].ptpCfgData.pdelayRespTimerStarted[j] = FALSE;
            EthTSyn_Internal.timeDomain[i].ptpCfgData.pdelayRespTimeout[j] = FALSE;
            EthTSyn_Internal.timeDomain[i].ptpCfgData.pdelayRespIntervalTimer[j] = 0;
            EthTSyn_Internal.timeDomain[i].ptpCfgData.txPdelayRespTimePending[j] = FALSE;
            EthTSyn_Internal.timeDomain[i].ptpCfgData.t3PdelayRespTxTime[j] = (Eth_TimeStampType){0};
            EthTSyn_Internal.timeDomain[i].ptpCfgData.pdelayRespfollowUpTimer[j] = 0;
            EthTSyn_Internal.timeDomain[i].ptpCfgData.pdelayRespfollowUpTimerStarted[j] = FALSE;
            EthTSyn_Internal.timeDomain[i].ptpCfgData.pdelayRespfollowUpTimerTimeout[j] = FALSE;
        }

        /* Pdelay Resp and FUP receive related variable init*/
        for (j=0; j<ETHTSYN_MAX_PDELAY_REQUEST; j++) {
            EthTSyn_Internal.timeDomain[i].ptpCfgData.rcvdPdelayResp = FALSE;
            EthTSyn_Internal.timeDomain[i].ptpCfgData.rcvdPdelayRespFollowUp = FALSE;
            EthTSyn_Internal.timeDomain[i].ptpCfgData.last_pdelay_resp_tx_sequence_number[j] = 0;
            EthTSyn_Internal.timeDomain[i].ptpCfgData.requestingPortIdentity[j].portNumber = 1;
            EthTSyn_Internal.timeDomain[i].ptpCfgData.t2PdelayRespTime[j] = (Eth_TimeStampType){0};
        }
        EthTSyn_Internal.timeDomain[i].ptpCfgData.last_pdelay_req_tx_sequence_number = 0; /* Any random Number */
        EthTSyn_Internal.timeDomain[i].ptpCfgData.t3PdelayRespTime = (Eth_TimeStampType){0};
        EthTSyn_Internal.timeDomain[i].ptpCfgData.t3PdelayRespRxTime_old = (Eth_TimeStampType){0};
        EthTSyn_Internal.timeDomain[i].ptpCfgData.t4PdelayRespRxTime = (Eth_TimeStampType){0};
        EthTSyn_Internal.timeDomain[i].ptpCfgData.t4PdelayRespRxTime_old = (Eth_TimeStampType){0};

        /* General variable init */
        EthTSyn_Internal.timeDomain[i].ptpCfgData.neighborRateRatioValid = FALSE;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.neighborRateRatio = 0;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.rateratio = (Eth_RateRatioType){{{0},0},{{0},0}}; /* @req 4.2.2/SWS_EthTSyn_00009 */

        EthTSyn_Internal.timeDomain[i].ptpCfgData.syncReceivedOnceFlag=FALSE;

        if (TRUE == EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[i].EthTSynTimeDomain->EthTSynEthTrcvEnable) {
            EthTSyn_Internal.timeDomain[i].trcvActiveStateFlag = FALSE;
        } else {
            EthTSyn_Internal.timeDomain[i].trcvActiveStateFlag = TRUE; /* If transceiver is not supported assume Transceiver Active state is reached */
        }

        EthTSyn_Internal.timeDomain[i].transmissionMode = ETHTSYN_TX_OFF;

        /* Generation of clock identity */
        /* Get MAC Addres */
        ctrlIdx = ETHTSYN_GET_CTRL_IDX(i);
        EthIf_GetPhysAddr(ctrlIdx, EthTSyn_Internal.timeDomain[i].ptpCfgData.port_uuid_field);
        /* Copy OUI field */
        memcpy(EthTSyn_Internal.timeDomain[i].ptpCfgData.port_clock_identity, EthTSyn_Internal.timeDomain[i].ptpCfgData.port_uuid_field, 3);
        /* IEEE EUI-48 (MAC address) to EUI-64 */
        EthTSyn_Internal.timeDomain[i].ptpCfgData.port_clock_identity[3] = 0xFF;
        EthTSyn_Internal.timeDomain[i].ptpCfgData.port_clock_identity[4] = 0xFE;
        /* Copy remaining 3 bytes of MAC address */
        memcpy(&EthTSyn_Internal.timeDomain[i].ptpCfgData.port_clock_identity[5], &EthTSyn_Internal.timeDomain[i].ptpCfgData.port_uuid_field[3], 3);
    }
    EthTSyn_Internal.initStatus = ETHTSYN_STATE_INIT;
}

#if(ETHTSYN_GET_VERSION_INFO == STD_ON)
/**
 * This service puts out the version information of this module
 * @param versioninfo
 */
void EthTSyn_GetVersionInfo( Std_VersionInfoType* versioninfo )
{
    /* @req 4.2.2/SWS_EthIf_00127 */
    ETHTSYN_DET_REPORTERROR((NULL != versioninfo),ETHTSYN_GETVERSIONINFO_ID,ETHTSYN_E_PARAM_POINTER);

    versioninfo->moduleID = ETHTSYN_MODULE_ID;  /* Module ID of ETHIF */
    versioninfo->vendorID = ETHTSYN_VENDOR_ID;  /* Vendor Id (ARCCORE) */

    /* return the Software Version numbers */
    versioninfo->sw_major_version = ETHTSYN_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = ETHTSYN_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = ETHTSYN_SW_PATCH_VERSION;
}
#endif

#if (ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON)
/**
 *
 * @param timeBaseId
 * @param timeStampPtr
 * @param syncState
 * @return
 */
Std_ReturnType EthTSyn_GetCurrentTime( StbM_SynchronizedTimeBaseType timeBaseId, StbM_TimeStampType* timeStampPtr, EthTSyn_SyncStateType* syncState )
{
    uint8 ctrlIdx;
    Eth_TimeStampQualType timeEthIfQualPtr;
    Eth_TimeStampType timeEthIfStampPtr;
    Std_ReturnType retValue;
    uint16 timeDomainId;

    /* @req 4.2.2/SWS_EthTSyn_00007 */
    ETHTSYN_DET_REPORTERROR((ETHTSYN_STATE_INIT == EthTSyn_Internal.initStatus), ETHTSYN_GETCURRENTTIME_ID, ETHTSYN_E_NOT_INITIALIZED, E_NOT_OK);

    timeDomainId = EthTSyn_Internal_GetTimeDomainStbm(timeBaseId);

    ETHTSYN_DET_REPORTERROR((ETHTSYN_INVALID_TIMEDOMAIN != timeDomainId),ETHTSYN_GETCURRENTTIME_ID,ETHTSYN_E_INVALID_TIMEBASE_ID,E_NOT_OK);

    ETHTSYN_DET_REPORTERROR((NULL != timeStampPtr),ETHTSYN_GETCURRENTTIME_ID, ETHTSYN_E_PARAM_POINTER, E_NOT_OK);

    ETHTSYN_DET_REPORTERROR((NULL != syncState),ETHTSYN_GETCURRENTTIME_ID, ETHTSYN_E_PARAM_POINTER, E_NOT_OK);

    ctrlIdx = ETHTSYN_GET_CTRL_IDX(timeDomainId);
    retValue = EthIf_GetCurrentTime(ctrlIdx, &timeEthIfQualPtr, &timeEthIfStampPtr);

    if(E_OK == retValue){
        if((EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.syncReceivedOnceFlag == FALSE)&&((PTP_SLAVE == EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomainId].EthTSynMasterFlag))){
               *syncState = ETHTSYN_NEVERSYNC;
         }else{
            if(ETH_VALID == timeEthIfQualPtr){
                *syncState = ETHTSYN_SYNC;
            }else if(ETH_INVALID == timeEthIfQualPtr){
                *syncState = ETHTSYN_UNSYNC;
            }else {
                *syncState = ETHTSYN_UNCERTAIN;
            }
         }
        timeStampPtr->nanoseconds = timeEthIfStampPtr.nanoseconds;
        timeStampPtr->seconds     = timeEthIfStampPtr.seconds;
        timeStampPtr->secondsHi   =  timeEthIfStampPtr.secondsHi;
    }
    return retValue;
}

/**
 *
 * @param timeBaseId
 * @param timeStampPtr
 * @return
 */
Std_ReturnType EthTSyn_SetGlobalTime( StbM_SynchronizedTimeBaseType timeBaseId, const StbM_TimeStampType* timeStampPtr )
{
    uint8 ctrlIdx;
    Eth_TimeStampType timeEthIfStampPtr;
    Std_ReturnType retValue;
    uint16 timeDomainId;

    /* @req 4.2.2/SWS_EthTSyn_00007 */
    ETHTSYN_DET_REPORTERROR((ETHTSYN_STATE_INIT == EthTSyn_Internal.initStatus), ETHTSYN_SETGLOBALTIME_ID, ETHTSYN_E_NOT_INITIALIZED, E_NOT_OK);

    timeDomainId = EthTSyn_Internal_GetTimeDomainStbm(timeBaseId);

    ETHTSYN_DET_REPORTERROR((ETHTSYN_INVALID_TIMEDOMAIN != timeDomainId), ETHTSYN_SETGLOBALTIME_ID,ETHTSYN_E_INVALID_TIMEBASE_ID,E_NOT_OK);

    ETHTSYN_DET_REPORTERROR((NULL != timeStampPtr), ETHTSYN_SETGLOBALTIME_ID, ETHTSYN_E_PARAM_POINTER, E_NOT_OK);

    timeEthIfStampPtr.nanoseconds = (*timeStampPtr).nanoseconds;
    timeEthIfStampPtr.seconds     = (*timeStampPtr).seconds;
    timeEthIfStampPtr.secondsHi   = (*timeStampPtr).secondsHi;

    ctrlIdx = ETHTSYN_GET_CTRL_IDX(timeDomainId);
    /* @req 4.2.2/SWS_EthTSyn_00015 */
    retValue = EthIf_SetGlobalTime(ctrlIdx, &timeEthIfStampPtr);

    return retValue;
}
#endif
/**
 * API to turn on and off the TX capabilities of the EthTSyn
 * @param CtrlIdx
 * @param Mode
 */
void EthTSyn_SetTransmissionMode( uint8 CtrlIdx, EthTSyn_TransmissionModeType Mode )
{
    EthTSyn_TimeDomainType timeDomainId;
    timeDomainId = EthTsyn_Internal_GetTimeDomainId(CtrlIdx);

    /* @req 4.2.2/SWS_EthTSyn_00007 */
    ETHTSYN_DET_REPORTERROR((ETHTSYN_STATE_INIT == EthTSyn_Internal.initStatus), ETHTSYN_SETTRANSMISSIONMODE_ID, ETHTSYN_E_NOT_INITIALIZED);

    ETHTSYN_DET_REPORTERROR((ETHTSYN_INVALID_CTRLID != EthTSyn_Internal_CheckEthIfCtrl(CtrlIdx)), ETHTSYN_SETTRANSMISSIONMODE_ID, ETHTSYN_E_INVALID_CTRL_ID);

    ETHTSYN_DET_REPORTERROR(((ETHTSYN_TX_ON == Mode) ||(ETHTSYN_TX_OFF == Mode) ), ETHTSYN_SETTRANSMISSIONMODE_ID, ETHTSYN_E_INV_MODE);

    EthTSyn_Internal.timeDomain[timeDomainId].transmissionMode = Mode;
}

/**
 *
 * @param CtrlIdx
 * @param TrcvLinkState
 * @return
 */
/* @req 4.2.2/SWS_EthTSyn_00043 */
Std_ReturnType EthTSyn_TrcvLinkStateChg( uint8 CtrlIdx, EthTrcv_LinkStateType TrcvLinkState )
{
    Std_ReturnType retvalue;
    uint16 timeDomainId;
    uint8 i;

    retvalue = E_OK;
    timeDomainId = EthTsyn_Internal_GetTimeDomainId(CtrlIdx);
    ETHTSYN_DET_REPORTERROR((ETHTSYN_STATE_INIT == EthTSyn_Internal.initStatus), ETHTSYN_TRCVLINKSTATECHG_ID, ETHTSYN_E_NOT_INITIALIZED, E_NOT_OK);

    ETHTSYN_DET_REPORTERROR((ETHTSYN_INVALID_CTRLID != EthTSyn_Internal_CheckEthIfCtrl(CtrlIdx)), ETHTSYN_TRCVLINKSTATECHG_ID, ETHTSYN_E_INVALID_CTRL_ID, E_NOT_OK);

    /* @req 4.2.2/SWS_EthTSyn_00019 */
    if(ETHTRCV_LINK_STATE_DOWN == TrcvLinkState){
        EthTSyn_Internal.timeDomain[timeDomainId].trcvActiveStateFlag = FALSE;
        EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.txTime_pending = FALSE;
        if(PTP_MASTER == EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomainId].EthTSynMasterFlag){
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.syncSend_State = SEND_SYNC_MSG;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.sentSync = FALSE;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.last_sync_tx_sequence_number = 0;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.txSyncTimePending = FALSE;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.syncIntervalTimerStarted = FALSE;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.syncIntervalTimeout = FALSE;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.syncfollowUpTimerStarted = FALSE;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.syncfollowUpTimerTimeout = FALSE;
            for (i=0; i<ETHTSYN_MAX_PDELAY_REQUEST; i++) {
                EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.pDelayResp_State[i] = INITIAL_WAITING_FOR_PDELAY_REQ;
                EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.rcvdPdelayReq[i] = FALSE;
                EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.pdelayRespfollowUpTimerStarted[i] = FALSE;
                EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.pdelayRespfollowUpTimerTimeout[i] = FALSE;
            }
            EthTSyn_Internal.timeDomain[timeDomainId].transmissionMode = ETHTSYN_TX_OFF;
#if (ETHTSYN_SEND_ANNOUNCE_SUPPORT == STD_ON)
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.announceIntervalTimerStarted = FALSE;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.announceIntervalTimeout = FALSE;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.last_announce_tx_sequence_number = 0;
#endif
        }else{
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.syncReceive_State = DISCARD;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.rcvdSync = FALSE;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.rcvdFollowUp = FALSE;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.parent_last_sync_sequence_number = 0;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.sentPdelayReq = FALSE;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.pDelayReq_State = INITIAL_SEND_PDELAY_REQ;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.pdelayIntervalTimeout = FALSE;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.rcvdPdelayResp = FALSE;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.rcvdPdelayRespFollowUp = FALSE;
            for ( i=0; i<ETHTSYN_MAX_PDELAY_REQUEST; i++) {
                EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.last_pdelay_resp_tx_sequence_number[i] = 0;
            }
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.initPdelayRespReceived = FALSE;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.last_pdelay_req_tx_sequence_number = 0;
            EthTSyn_Internal.timeDomain[timeDomainId].transmissionMode = ETHTSYN_TX_OFF;
        }
    }else{/* @req 4.2.2/SWS_EthTSyn_00020 */
        EthTSyn_Internal.timeDomain[timeDomainId].trcvActiveStateFlag = TRUE;
    }
    return retvalue;
}
/**
 * function to capture Ingress TimeStamp
 * @param CtrlIdx
 * @param timeDomainId
 * @param timeStamp
 * @param req_index
 */
/*lint -e{818} */
static void EthTSyn_Internal_IngressTimeStamp(uint8 CtrlIdx, EthTSyn_TimeDomainType timeDomainId, Eth_DataType* DataPtr,EthTSyn_Internal_MessageType messageType, uint8 req_index)
{

#if ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON
    Eth_TimeStampQualType timeQual;
    Eth_TimeStampType timeStamp ;
#else
    StbM_TimeStampType timeStampStbM;
    StbM_UserDataType userData;
    Std_ReturnType stbmretvalue;

    userData = (StbM_UserDataType){0};
    timeStampStbM = (StbM_TimeStampType){0,0,0,0};
    (void)CtrlIdx;
    (void)*DataPtr;
#endif

#if ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON
    EthIf_GetIngressTimeStamp(CtrlIdx, DataPtr, &timeQual, &timeStamp);
#else
    uint8 timeBaseId =    EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomainId].EthTSynSynchronizedTimeBaseRef;
    stbmretvalue = StbM_GetCurrentTime(timeBaseId, &timeStampStbM, &userData );
#endif

    /* Timestamp the received Sync msg */
    if(RXSYNC == messageType){
#if ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON
        if(ETH_VALID == timeQual){
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t2SyncReceiptTime.nanoseconds = timeStamp.nanoseconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t2SyncReceiptTime.seconds = timeStamp.seconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t2SyncReceiptTime.secondsHi = timeStamp.secondsHi;
        }
#else
        if(E_OK == stbmretvalue){
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t2SyncReceiptTime.nanoseconds = timeStampStbM.nanoseconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t2SyncReceiptTime.seconds = timeStampStbM.seconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t2SyncReceiptTime.secondsHi = timeStampStbM.secondsHi;
        }
#endif
    }else if(RXPDELAY_REQ == messageType){ /* Timestamp the received Pdelay msg */
#if ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON
        if(ETH_VALID == timeQual){
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t2PdelayReqRxTime[req_index].nanoseconds = timeStamp.nanoseconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t2PdelayReqRxTime[req_index].seconds = timeStamp.seconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t2PdelayReqRxTime[req_index].secondsHi = timeStamp.secondsHi;
        }
#else
        if(E_OK == stbmretvalue){
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t2PdelayReqRxTime[req_index].nanoseconds = timeStampStbM.nanoseconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t2PdelayReqRxTime[req_index].seconds = timeStampStbM.seconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t2PdelayReqRxTime[req_index].secondsHi = timeStampStbM.secondsHi;
        }else{

        }
#endif
    }else if (RXPDELAY_RESP == messageType){/* Timestamp the received Pdelay Resp msg */
#if ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON
        if(ETH_VALID == timeQual){
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t4PdelayRespRxTime.nanoseconds = timeStamp.nanoseconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t4PdelayRespRxTime.seconds = timeStamp.seconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t4PdelayRespRxTime.secondsHi = timeStamp.secondsHi;
        }
#else
        if(E_OK == stbmretvalue){
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t4PdelayRespRxTime.nanoseconds = timeStampStbM.nanoseconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t4PdelayRespRxTime.seconds = timeStampStbM.seconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t4PdelayRespRxTime.secondsHi = timeStampStbM.secondsHi;
        }
#endif
    }else {}
}

/**
 * function to capture Egress TimeStamp
 * @param CtrlIdx
 * @param BufIdx
 * @param timeDomainId
 * @param timeStamp
 * @param req_index
 */
static Std_ReturnType EthTSyn_Internal_EgressTimeStamp(uint8 CtrlIdx, Eth_BufIdxType BufIdx, EthTSyn_TimeDomainType timeDomainId, EthTSyn_Internal_MessageType messageType, uint8 req_index)
{
    Std_ReturnType retvalue;
#if ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON
    Eth_TimeStampQualType timeQual;
    Eth_TimeStampType timeStamp;
#else
    StbM_TimeStampType timeStampStbM;
    StbM_UserDataType userData;
    Std_ReturnType stbmretvalue;

    userData = (StbM_UserDataType){0};
    timeStampStbM = (StbM_TimeStampType){0,0,0,0};
    (void)CtrlIdx;
    (void)BufIdx;
#endif
    retvalue = E_NOT_OK;

#if ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON
    EthIf_GetEgressTimeStamp(CtrlIdx, BufIdx, &timeQual, &timeStamp);
#else
    uint8 timeBaseId =    EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomainId].EthTSynSynchronizedTimeBaseRef;
    stbmretvalue = StbM_GetCurrentTime(timeBaseId, &timeStampStbM, &userData );
#endif
    /* Timestamp the sent Sync msg */
    if(TXSYNC == messageType){
#if ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON
        if(ETH_VALID == timeQual){
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t1SyncTxTime.nanoseconds = timeStamp.nanoseconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t1SyncTxTime.seconds = timeStamp.seconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t1SyncTxTime.secondsHi = timeStamp.secondsHi;
            retvalue = E_OK;
        }else{
            retvalue = E_NOT_OK;
        }

#else
        if(E_OK == stbmretvalue){
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t1SyncTxTime.nanoseconds = timeStampStbM.nanoseconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t1SyncTxTime.seconds = timeStampStbM.seconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t1SyncTxTime.secondsHi = timeStampStbM.secondsHi;
            retvalue = E_OK;
        }else{
            retvalue = E_NOT_OK;
        }
#endif
    }else if(TXPDELAY_REQ == messageType){/* Timestamp the sent Pdelay Req msg */
#if ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON
        if(ETH_VALID == timeQual){
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t1PdelayReqTxTime.nanoseconds = timeStamp.nanoseconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t1PdelayReqTxTime.seconds = timeStamp.seconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t1PdelayReqTxTime.secondsHi = timeStamp.secondsHi;
            retvalue = E_OK;
        }else{
            retvalue = E_NOT_OK;
        }
#else
        if(E_OK == stbmretvalue){
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t1PdelayReqTxTime.nanoseconds = timeStampStbM.nanoseconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t1PdelayReqTxTime.seconds = timeStampStbM.seconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t1PdelayReqTxTime.secondsHi = timeStampStbM.secondsHi;
            retvalue = E_OK;
        }else{
            retvalue = E_NOT_OK;
        }
#endif
    }else if(TXPDELAY_RESP == messageType){ /* Timestamp the sent Pdelay Resp msg */
#if ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON
        if(ETH_VALID == timeQual){
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t3PdelayRespTxTime[req_index].nanoseconds = timeStamp.nanoseconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t3PdelayRespTxTime[req_index].seconds = timeStamp.seconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t3PdelayRespTxTime[req_index].secondsHi = timeStamp.secondsHi;
        }
#else
        if(E_OK == stbmretvalue){
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t3PdelayRespTxTime[req_index].nanoseconds = timeStampStbM.nanoseconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t3PdelayRespTxTime[req_index].seconds = timeStampStbM.seconds;
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t3PdelayRespTxTime[req_index].secondsHi = timeStampStbM.secondsHi;
            retvalue = E_OK;
        }else{
            retvalue = E_NOT_OK;
        }
#endif
    }else{}

    return retvalue;
}

/**
 *
 * @param CtrlIdx
 * @param BufIdx
 */
void EthTSyn_TxConfirmation( uint8 CtrlIdx, Eth_BufIdxType BufIdx )
{
    uint16 timeDomainId;
    Std_ReturnType retval;
    uint8 i;

    timeDomainId = EthTsyn_Internal_GetTimeDomainId(CtrlIdx);

    ETHTSYN_DET_REPORTERROR((ETHTSYN_STATE_INIT == EthTSyn_Internal.initStatus), ETHTSYN_TXCONFIRMATION_ID, ETHTSYN_E_NOT_INITIALIZED);

    ETHTSYN_DET_REPORTERROR((ETHTSYN_INVALID_CTRLID != EthTSyn_Internal_CheckEthIfCtrl(CtrlIdx)), ETHTSYN_TXCONFIRMATION_ID, ETHTSYN_E_INVALID_CTRL_ID);

    if (TRUE == EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.txTime_pending){

        /* at least one tx timestamp pending */
        /* Sync pending */ /* @req 4.2.2/SWS_EthTSyn_00017 */
        if (TRUE == EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.txSyncTimePending){
            /* If port is Master timestamp it */
            if(PTP_MASTER == EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomainId].EthTSynMasterFlag){
                retval = EthTSyn_Internal_EgressTimeStamp(CtrlIdx,BufIdx,timeDomainId,TXSYNC,0); /*@req 4.2.2/SWS_EthTSyn_00017 */
                if(E_OK == retval){
                    EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.sentSync = TRUE;
                    EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.txSyncTimePending = FALSE;
                }else{
                    EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.syncSend_State = SEND_SYNC_MSG;
                }
            }/*else {
                 Sync Tx pending is true, but we are no longer master time pending Flags
                EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.txSyncTimePending = FALSE;
            }*/
        }
        /* @req 4.2.2/SWS_EthTSyn_00013 */
        /* PDelay request pending */
        if(TRUE == EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.txPdelayReqTimePending){
            /* Pdelay Req sent and Time stamp T1 */
            retval = EthTSyn_Internal_EgressTimeStamp(CtrlIdx,BufIdx,timeDomainId,TXPDELAY_REQ,0);
            if(E_OK == retval){
                EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.sentPdelayReq = TRUE;
                EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.txPdelayReqTimePending = FALSE;
            }else{
                EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.pDelayReq_State = SEND_PDELAY_REQ;
            }
        }
        /* PDelay response pending */
        for (i=0; i < ETHTSYN_MAX_PDELAY_REQUEST; i++) {
            if(TRUE == EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.txPdelayRespTimePending[i]){
                /* Pdelay Resp sent and Time stamp T3 */
                retval = EthTSyn_Internal_EgressTimeStamp(CtrlIdx,BufIdx,timeDomainId,TXPDELAY_RESP,i);
                (void)retval;
                EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.txPdelayRespTimePending[i] = FALSE;
            }
        }
    }
}

/**
 * function process received Sync message and time stamp ingress time
 * @param CtrlIdx
 * @param timeDomainId
 * @param DataPtr
 * @param LenByte
 */
static void EthTSyn_Internal_SyncReceive(uint8 CtrlIdx, EthTSyn_TimeDomainType timeDomainId, uint8* DataPtr, uint16 LenByte)
{
    uint16  sequence_delta;
    /*lint -e927 -e826 LenByte check below ensures addresses referenced by is pktPtr valid*/
    const EthTsyn_Internal_MsgSync  * pktPtr = (EthTsyn_Internal_MsgSync *)DataPtr;

    sequence_delta = bswap16(pktPtr->headerMsg.sequenceId) - EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.parent_last_sync_sequence_number;

    if((sequence_delta == 0) || (LenByte < V2_SYNC_LENGTH)){
        /* Possible duplicate sequence received/short sync message, ignoring */
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check */
        return;
    }

    if  (sequence_delta != 1)
    {
        /* 1 or more sync Messages may have been lost/discarded */
    }
    /* Time stamp T2 */
    EthTSyn_Internal_IngressTimeStamp(CtrlIdx,timeDomainId, DataPtr, RXSYNC, 0);

    /* Store sequence id to check next time with followup seq id */
    EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.parent_last_sync_sequence_number = bswap16(pktPtr->headerMsg.sequenceId);

    if(V2_TWO_STEP_FLAG == (pktPtr->headerMsg.flags[0] & V2_TWO_STEP_FLAG)){
        EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.waitingForFollow = TRUE;
        /*Start Follow Up Timer  */
        EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.RxFollowUpTimerStarted = TRUE;
        EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.RxFollowUpTimer = 0;
        EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.rcvdSync = TRUE;
    }else
    {
        EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.rcvdSync = FALSE;
    }
}

/**
 * function process received Sync FUP message and time stamp ingress time
 * @param CtrlIdx
 * @param timeDomainId
 * @param DataPtr
 * @param LenByte
 */
static void EthTSyn_Internal_SyncFollowUpReceive(uint8 CtrlIdx, EthTSyn_TimeDomainType timeDomainId, const uint8* DataPtr, uint16 LenByte)
{
    /*lint -e927 -e826 LenByte check below ensures addresses referenced by is pktPtr valid*/
    const EthTsyn_Internal_MsgFollowUp * pktPtr = (const EthTsyn_Internal_MsgFollowUp *)DataPtr;

    if(LenByte < V2_FOLLOWUP_LENGTH){
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check */
        return;
    }
    if((TRUE == EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.waitingForFollow)){

        if(EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.parent_last_sync_sequence_number == bswap16(pktPtr->headerMsg.sequenceId)){
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t1SyncOriginTime.nanoseconds = bswap32(pktPtr->preciseOriginTimestamp.nanoseconds);
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t1SyncOriginTime.seconds = bswap32(pktPtr->preciseOriginTimestamp.seconds);
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t1SyncOriginTime.secondsHi = bswap16(pktPtr->preciseOriginTimestamp.secondsHi);
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.waitingForFollow = FALSE;

            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.rcvdFollowUp = TRUE;
            /* Reset Sync interval timer */
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.RxFollowUpTimerStarted = FALSE;
        }
    }/* short sync FollowUp message */
    (void)CtrlIdx;
}

/**
 * function process received Pdelay Req message and time stamp ingress time
 * @param CtrlIdx
 * @param timeDomainId
 * @param DataPtr
 * @param LenByte
 */
static void EthTSyn_Internal_PdelayReqReceive(uint8 CtrlIdx, EthTSyn_TimeDomainType timeDomainId, uint8* DataPtr, uint16 LenByte)
{
	uint8 i;
	/*lint -e927 -e826 LenByte check below ensures addresses referenced by is pktPtr valid*/
    const EthTsyn_Internal_MsgPDelayReq * pktPtr = (EthTsyn_Internal_MsgPDelayReq *)DataPtr;
    if(LenByte < V2_PDELAY_REQ_LENGTH){
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check */
        return;
    }
    /* Find index to requestingPortIdentity */
    for (i=0; i<ETHTSYN_MAX_PDELAY_REQUEST; i++) {
    	if (memcmp(EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.requestingPortIdentity[i].clockIdentity,
    			pktPtr->headerMsg.sourcePortId.clockIdentity, CLOCK_IDENTITY_LENGTH) == 0) {
            break;
    	}
    }
    if (i == ETHTSYN_MAX_PDELAY_REQUEST) {
    	/* First pdelay_req that is received take new index! */
    	if (EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.numPdelay_req < ETHTSYN_MAX_PDELAY_REQUEST ) {
            i = EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.numPdelay_req;
            memcpy(EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.requestingPortIdentity[i].clockIdentity, pktPtr->headerMsg.sourcePortId.clockIdentity, CLOCK_IDENTITY_LENGTH);
            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.numPdelay_req++;
        } else {
        	i = ETHTSYN_MAX_PDELAY_REQUEST-1; /* reuse the last index!!! */
            memcpy(EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.requestingPortIdentity[i].clockIdentity, pktPtr->headerMsg.sourcePortId.clockIdentity, CLOCK_IDENTITY_LENGTH);
        }
    }
    /* Time stamp T2 */
    EthTSyn_Internal_IngressTimeStamp(CtrlIdx,timeDomainId,DataPtr, RXPDELAY_REQ, i);
    EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.last_pdelay_resp_tx_sequence_number[i] = bswap16(pktPtr->headerMsg.sequenceId);
    EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.requestingPortIdentity[i].portNumber = bswap16(pktPtr->headerMsg.sourcePortId.portNumber);
    EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.rcvdPdelayReq[i] = TRUE;
}

/**
 * function process received Pdelay Resp message and time stamp ingress time
 * @param CtrlIdx
 * @param timeDomainId
 * @param DataPtr
 * @param LenByte
 */
static void EthTSyn_Internal_PdelayRespReceive(uint8 CtrlIdx, EthTSyn_TimeDomainType timeDomainId, uint8* DataPtr, uint16 LenByte)
{
    /*lint -e927 -e826 LenByte check below ensures addresses referenced by is pktPtr valid*/
    const EthTsyn_Internal_MsgPDelayResp * pktPtr = (EthTsyn_Internal_MsgPDelayResp *) DataPtr;
    if(LenByte < V2_PDELAY_RESP_LENGTH){
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check */
        return;
    }
    if (EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomainId].EthTSynMasterFlag == PTP_SLAVE) {
        /* Time stamp T4 */
        EthTSyn_Internal_IngressTimeStamp(CtrlIdx,timeDomainId,DataPtr,RXPDELAY_RESP,0);
        /* Take T2 Copy from receiveTimestamp  */
        EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t2PdelayRespTime[0].nanoseconds = bswap32(pktPtr->receiveTimestamp.nanoseconds);
        EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t2PdelayRespTime[0].seconds = bswap32(pktPtr->receiveTimestamp.seconds);
        EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t2PdelayRespTime[0].secondsHi = bswap16(pktPtr->receiveTimestamp.secondsHi);
        EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.last_pdelay_resp_tx_sequence_number[0] = bswap16(pktPtr->headerMsg.sequenceId);

        if((memcmp(EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.port_clock_identity, pktPtr->requestingPortId.clockIdentity, CLOCK_IDENTITY_LENGTH) == 0)
            && (bswap16(pktPtr->requestingPortId.portNumber) == DEFAULT_PORT_NUMBER)){

            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.rcvdPdelayResp = TRUE;
        }
    }
}
/**
 * function process received Pdelay Resp FUP message and time stamp ingress time
 * @param CtrlIdx
 * @param timeDomainId
 * @param DataPtr
 * @param LenByte
 */
static void EthTSyn_Internal_PdelayRespFollowUpReceive(uint8 CtrlIdx, EthTSyn_TimeDomainType timeDomainId, const uint8* DataPtr, uint16 LenByte)
{
    /*lint -e927 -e826 LenByte check below ensures addresses referenced by is pktPtr valid*/
    const EthTsyn_Internal_MsgPDelayRespFollowUp * pktPtr = (const EthTsyn_Internal_MsgPDelayRespFollowUp *) DataPtr;
    if(LenByte < V2_PDELAY_RESP_FOLLOWUP_LENGTH){
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check */
        return;
    }
    if (EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomainId].EthTSynMasterFlag == PTP_SLAVE) {
        /* Time stamp T3 */
        EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t3PdelayRespTime.nanoseconds = bswap32(pktPtr->responseOriginTimestamp.nanoseconds);
        EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t3PdelayRespTime.seconds = bswap32(pktPtr->responseOriginTimestamp.seconds);
        EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.t3PdelayRespTime.secondsHi = bswap16(pktPtr->responseOriginTimestamp.secondsHi);
        EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.last_pdelay_resp_follow_tx_sequence_number[0] = bswap16(pktPtr->headerMsg.sequenceId);

        if((memcmp(EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.port_clock_identity, pktPtr->requestingPortId.clockIdentity, CLOCK_IDENTITY_LENGTH) == 0)
            && (bswap16(pktPtr->requestingPortId.portNumber) == DEFAULT_PORT_NUMBER)){

            EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.rcvdPdelayRespFollowUp = TRUE;
        }
    }

    (void)CtrlIdx;
}

/**
 * function process received message and time stamp ingress time
 * @param CtrlIdx
 * @param FrameType
 * @param IsBroadcast
 * @param PhysAddrPtr
 * @param DataPtr
 * @param LenByte
 */
void EthTSyn_RxIndication( uint8 CtrlIdx, Eth_FrameType FrameType, boolean IsBroadcast, const uint8* PhysAddrPtr, uint8* DataPtr, uint16 LenByte )
{
    EthTSyn_TimeDomainType timeDomainId;
    timeDomainId = EthTsyn_Internal_GetTimeDomainId(CtrlIdx);

    /*@req 4.2.2/SWS_EthTSyn_00041 */
    ETHTSYN_DET_REPORTERROR((ETHTSYN_STATE_INIT == EthTSyn_Internal.initStatus), ETHTSYN_RXINDICATION_ID, ETHTSYN_E_NOT_INITIALIZED);

    ETHTSYN_DET_REPORTERROR((ETHTSYN_INVALID_CTRLID != EthTSyn_Internal_CheckEthIfCtrl(CtrlIdx)), ETHTSYN_RXINDICATION_ID, ETHTSYN_E_INVALID_CTRL_ID);

    if(ETH_FRAME_TYPE_TSYN == FrameType){
        /*lint -e{9016,926} */
        EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.msg_type  = *(uint8*) (DataPtr + 0) & ETHTSYN_MESSAGE_TYPE ;
        switch(EthTSyn_Internal.timeDomain[timeDomainId].ptpCfgData.msg_type){
            case V2_SYNC_MESSAGE :
                EthTSyn_Internal_SyncReceive(CtrlIdx, timeDomainId, DataPtr, LenByte); /* @req 4.2.2/SWS_EthTSyn_00024 */
                break;

            case V2_FOLLOWUP_MESSAGE :
                EthTSyn_Internal_SyncFollowUpReceive(CtrlIdx, timeDomainId, DataPtr, LenByte);
                break;

            case V2_PDELAY_REQ_MESSAGE :
                EthTSyn_Internal_PdelayReqReceive(CtrlIdx, timeDomainId, DataPtr, LenByte);  /* @req 4.2.2/SWS_EthTSyn_00049 */
                break;

            case V2_PDELAY_RESP_MESSAGE:
                EthTSyn_Internal_PdelayRespReceive(CtrlIdx, timeDomainId, DataPtr, LenByte); /* @req 4.2.2/SWS_EthTSyn_00049 */
                break;

            case V2_PDELAY_RESP_FOLLOWUP_MESSAGE:
                EthTSyn_Internal_PdelayRespFollowUpReceive(CtrlIdx, timeDomainId, DataPtr, LenByte);
                break;

            default : /*@req 4.2.2/SWS_EthTSyn_00005 */
                break;
        }

    }/* Frame invalid Ethertype (not 802.1AS PTP), ignore */
    (void)IsBroadcast;
    /*lint -e{920} */
    (void)PhysAddrPtr;
    /*lint -e{818} */
}


#if (ETHTSYN_SEND_ANNOUNCE_SUPPORT == STD_ON)
/**
 * function transmit packet Announce message
 * @param timeDomain
 */
static Std_ReturnType EthTSyn_Internal_SendAnnounce(uint8 timeDomain)
{
    uint8 ctrlIdx;
    Eth_BufIdxType bufIndex;
    uint16 lenByte;
    void* address;
    EthTsyn_Internal_MsgAnnounce *announceMsg;
    Std_ReturnType retValue;
    BufReq_ReturnType retBufValue;

    retValue = E_NOT_OK;
    lenByte = V2_ANNOUNCE_LENGTH + 4 + V2_CLOCKPATH_TLV_LEN*1;
    bufIndex = 0;
    ctrlIdx = EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomain].EthTSynTimeDomain->EthTSynGlobalTimeEthIfId;
    /*lint -e{929}*/
    retBufValue = EthIf_ProvideTxBuffer(ctrlIdx, ETH_FRAME_TYPE_TSYN, 0, &bufIndex, (uint8 **)&address, &lenByte);
    if(BUFREQ_OK == retBufValue){
        /* Sequence Id increment */
        ++(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.last_announce_tx_sequence_number);
        announceMsg = (EthTsyn_Internal_MsgAnnounce *)address;
        /* Prepare Sync Msg packet */
        EthTSyn_Internal_MsgPackAnnounceMsg(announceMsg, timeDomain);

        retValue = EthIf_Transmit(ctrlIdx, bufIndex, ETH_FRAME_TYPE_TSYN, TRUE, lenByte, EthTSyn_Internal_MulitCast_DestMACAddrs);
        if(E_OK == retValue){
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.txSyncTimePending = FALSE;
        }else{
            /* Since Transmission is failed decrement Sequence Id */
            --(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.last_announce_tx_sequence_number);
        }
    }
    return retValue;
}
#endif

/**
 * function transmit packed Sync message and timestamp the egress time
 * @param timeDomain
 */
static Std_ReturnType EthTSyn_Internal_SendSync(uint8 timeDomain)
{
    uint8 ctrlIdx;
    Eth_BufIdxType bufIndex;
    uint16 lenByte;
    void* address;
    EthTsyn_Internal_MsgSync *syncMsg;
    Std_ReturnType retValue;
    BufReq_ReturnType retBufValue;

    retValue = E_NOT_OK;
    lenByte = V2_SYNC_LENGTH;
    bufIndex = 0;
    ctrlIdx = EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomain].EthTSynTimeDomain->EthTSynGlobalTimeEthIfId;
    /*lint -e{929}*/
    retBufValue = EthIf_ProvideTxBuffer(ctrlIdx, ETH_FRAME_TYPE_TSYN, 0, &bufIndex, (uint8 **)&address, &lenByte);
    if(BUFREQ_OK == retBufValue){
#if (ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON)
        EthIf_EnableEgressTimeStamp(ctrlIdx, bufIndex);
#endif
        /* Sequence Id increment */
        ++(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.last_sync_tx_sequence_number);
        syncMsg = (EthTsyn_Internal_MsgSync *)address;
        /* Prepare Sync Msg packet */
        EthTSyn_Internal_MsgPackSyncMsg(syncMsg, timeDomain);

        retValue = EthIf_Transmit(ctrlIdx, bufIndex, ETH_FRAME_TYPE_TSYN, TRUE, lenByte, EthTSyn_Internal_MulitCast_DestMACAddrs);
        if(E_OK == retValue){
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.txTime_pending = TRUE;
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.txSyncTimePending = TRUE;
        }else{
            /* Since Transmission is failed decrement Sequence Id */
            --(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.last_sync_tx_sequence_number);
        }
    }
    return retValue;
}

/**
 * function transmit packed Sync FUP message and timestamp the egress time
 * @param timeDomain
 */
/* @req 4.2.2/SWS_EthTSyn_00018 */
static Std_ReturnType EthTSyn_Internal_SendSyncFollowUp(uint8 timeDomain)
{
    Eth_BufIdxType bufIndex;
    uint16 lenByte;
    void* address;
    EthTsyn_Internal_MsgFollowUp *syncFollowMsg;
    uint8 ctrlIdx;
    Std_ReturnType retValue;
    BufReq_ReturnType retBufValue;

    retValue = E_NOT_OK;
    lenByte = V2_FOLLOWUP_LENGTH;
    bufIndex = 0;
    ctrlIdx = EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomain].EthTSynTimeDomain->EthTSynGlobalTimeEthIfId;

    /*lint -e{929} */
    retBufValue = EthIf_ProvideTxBuffer(ctrlIdx, ETH_FRAME_TYPE_TSYN, 0, &bufIndex, (uint8**)&address,   &lenByte);
    if(BUFREQ_OK == retBufValue){
        syncFollowMsg = (EthTsyn_Internal_MsgFollowUp*)address;
        EthTSyn_Internal_MsgPackSyncFollow(syncFollowMsg, timeDomain);

        retValue = EthIf_Transmit(ctrlIdx, bufIndex, ETH_FRAME_TYPE_TSYN, TRUE, lenByte, EthTSyn_Internal_MulitCast_DestMACAddrs);
        if(E_OK == retValue){
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.txSyncTimePending = FALSE;
        }
    }
    return retValue;
}

/**
 * function transmit packed Pdelay Req message and timestamp the egress time
 * @param timeDomain
 */
static Std_ReturnType EthTSyn_Internal_SendPdelayReq(uint8 timeDomain)
{
    uint8 ctrlIdx;
    Eth_BufIdxType bufIndex;
    uint16 lenByte;
    void* address;
    EthTsyn_Internal_MsgPDelayReq *PdelayReqMsg;
    Std_ReturnType retValue;
    BufReq_ReturnType retBufValue;

    retValue =  E_NOT_OK;
    lenByte = V2_PDELAY_REQ_LENGTH;
    bufIndex = 0;
    ctrlIdx = EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomain].EthTSynTimeDomain->EthTSynGlobalTimeEthIfId;

    /*lint -e{929} */
    retBufValue = EthIf_ProvideTxBuffer(ctrlIdx, ETH_FRAME_TYPE_TSYN, 0, &bufIndex, (uint8**)&address, &lenByte);
    if(BUFREQ_OK == retBufValue){
#if (ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON)
        EthIf_EnableEgressTimeStamp(ctrlIdx, bufIndex);
#endif
        /* Sequence Id increment */
        ++(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.last_pdelay_req_tx_sequence_number);
        PdelayReqMsg = (EthTsyn_Internal_MsgPDelayReq*)address;
        EthTSyn_Internal_MsgPackPdelayReqMsg(PdelayReqMsg, timeDomain);

        retValue = EthIf_Transmit(ctrlIdx, bufIndex, ETH_FRAME_TYPE_TSYN, TRUE, lenByte, EthTSyn_Internal_MulitCast_DestMACAddrs);
        if(E_OK == retValue){
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.txTime_pending = TRUE;
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.txPdelayReqTimePending = TRUE;
        }else{
            /* Since Transmission is failed decrement Sequence Id */
            --(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.last_pdelay_req_tx_sequence_number);
        }
    }
    return retValue;
}

/**
 * function transmit packed Pdelay Resp message and timestamp the egress time
 * @param timeDomain
 * @param req_index
 */
static Std_ReturnType EthTSyn_Internal_SendPdelayResp(uint8 timeDomain, uint8 req_index)
{
    uint8 ctrlIdx;
    Eth_BufIdxType bufIndex;
    uint16 lenByte;
    void* address;
    EthTsyn_Internal_MsgPDelayResp *PdelayRespMsg;
    Std_ReturnType retValue;
    BufReq_ReturnType retBufValue;

    retValue =  E_NOT_OK;
    lenByte = V2_PDELAY_RESP_LENGTH;
    bufIndex = 0;
    ctrlIdx = EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomain].EthTSynTimeDomain->EthTSynGlobalTimeEthIfId;

    /*lint -e{929} */
    retBufValue = EthIf_ProvideTxBuffer(ctrlIdx, ETH_FRAME_TYPE_TSYN, 0, &bufIndex, (uint8**)&address, &lenByte);
    if(BUFREQ_OK == retBufValue){
#if (ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON)
        EthIf_EnableEgressTimeStamp(ctrlIdx, bufIndex);
#endif
        PdelayRespMsg = (EthTsyn_Internal_MsgPDelayResp*)address;
        EthTSyn_Internal_MsgPackPdelayRespMsg(PdelayRespMsg, timeDomain,req_index);

        retValue = EthIf_Transmit(ctrlIdx, bufIndex, ETH_FRAME_TYPE_TSYN, TRUE, lenByte, EthTSyn_Internal_MulitCast_DestMACAddrs);
        if(E_OK == retValue){
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.txTime_pending = TRUE;
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.txPdelayRespTimePending[req_index] = TRUE;
        }
    }
    return retValue;
}

/**
 * function transmit packed Pdelay Resp FUP message and timestamp the egress time
 * @param timeDomain
 * @param req_index
 */
/* @req 4.2.2/SWS_EthTSyn_00014 */
static void EthTSyn_Internal_SendPdelayRespFollowUp(uint8 timeDomain, uint8 req_index)
{
    Eth_BufIdxType bufIndex;
    uint16 lenByte;
    void* address;
    EthTsyn_Internal_MsgPDelayRespFollowUp *PdelayRespFollowMsg;
    uint8 ctrlIdx;
    Std_ReturnType retValue;
    BufReq_ReturnType retBufValue;

    lenByte = V2_PDELAY_RESP_FOLLOWUP_LENGTH;
    bufIndex = 0;
    ctrlIdx = EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomain].EthTSynTimeDomain->EthTSynGlobalTimeEthIfId;

    /*lint -e{929} */
    retBufValue = EthIf_ProvideTxBuffer(ctrlIdx, ETH_FRAME_TYPE_TSYN, 0, &bufIndex, (uint8**)&address, &lenByte);
    if(BUFREQ_OK == retBufValue){
        PdelayRespFollowMsg = (EthTsyn_Internal_MsgPDelayRespFollowUp*)address;
        EthTSyn_Internal_MsgPackPdelayRespFollowUpMsg(PdelayRespFollowMsg, timeDomain, req_index);

        retValue = EthIf_Transmit(ctrlIdx, bufIndex, ETH_FRAME_TYPE_TSYN, TRUE, lenByte, EthTSyn_Internal_MulitCast_DestMACAddrs);
        if(E_OK == retValue){
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pdelayRespTimerStarted[req_index] = FALSE;
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pdelayRespTimeout[req_index] = FALSE;
        }
    }
}

/**
 * function calculate neighbor RateRatio
 * @param timeDomain
 */
/* @req 4.2.2/SWS_EthTSyn_00003 */
static void computePdelayRateRatio(uint8 timeDomain)
{
    sint64 correctedResponderEventTimestamp_N;
    sint64 correctedResponderEventTimestamp_0;
    sint64 pdelayRespEventIngressTimestamp_N;
    sint64 pdelayRespEventIngressTimestamp_0;
    sint64 result_1;
    sint64 result_2;

    /* Store Pdelay Resp receipt timestamp */
    pdelayRespEventIngressTimestamp_N = (sint64)convert_To_NanoSec(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t4PdelayRespRxTime);
    pdelayRespEventIngressTimestamp_0 = (sint64)convert_To_NanoSec(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t4PdelayRespRxTime_old);

    /* Store Pdelay Resp origin timestamp */
    correctedResponderEventTimestamp_N = (sint64)convert_To_NanoSec(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t3PdelayRespTime);
    correctedResponderEventTimestamp_0 = (sint64)convert_To_NanoSec(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t3PdelayRespRxTime_old);

    /*Get Ingress time and Engress Time stamp of Pdelay Resp */
    result_1 = (pdelayRespEventIngressTimestamp_N - pdelayRespEventIngressTimestamp_0);
    result_2 = (correctedResponderEventTimestamp_N - correctedResponderEventTimestamp_0);

    /* neighborRateRatio */
    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.neighborRateRatio = (result_2 / result_1);
}

/**
 * function to calculate peer mean path delay(link delay)
 * @param timeDomain
 */
static void computePropTime(uint8 timeDomain)
{
    sint64 pdelayRespEventIngressTimestamp;
    sint64 pdelayReqEventEgressTimestamp;
    sint64 requestReceiptTimestamp;
    sint64 responseOriginTimestamp;

    /* convert T1, T2, T3 and T4 into nanoseconds */
    pdelayReqEventEgressTimestamp = (sint64)(convert_To_NanoSec(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t1PdelayReqTxTime));
    requestReceiptTimestamp = (sint64)(convert_To_NanoSec(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t2PdelayRespTime[0]));
    responseOriginTimestamp = (sint64)(convert_To_NanoSec(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t3PdelayRespTime));
    pdelayRespEventIngressTimestamp = (sint64)(convert_To_NanoSec(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t4PdelayRespRxTime));

    /* peer mean path delay value */
    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.peer_mean_path_delay = (((pdelayRespEventIngressTimestamp - pdelayReqEventEgressTimestamp) - (responseOriginTimestamp - requestReceiptTimestamp))/2);
    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.peer_mean_path_delay = EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.peer_mean_path_delay * EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.neighborRateRatio;
}

/**
 * function to calculate offset from master to slave and sync with master
 * @param timeDomain
 */
static void synchronize_clock(uint8 timeDomain)
{
    sint64 t2Recv_time;
    sint64 t1Send_time;
    sint64 master_to_slave_delay;
#if (ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON)
    uint8 ctrlIdx;
    Eth_TimeIntDiffType offset_From_Master;
    EthTSyn_Internal_TimeNanoType tmp_Master_to_slave_delay;
#else
    StbM_TimeStampType stbmTimeStamp;
    uint64 updatedStbmTimeStamp;
    StbM_UserDataType userData;
    Std_ReturnType retVal;
    sint64 stbmTime;
#endif
    /* convert origintime at master port of sync msg into nano secs */
    t1Send_time = (sint64)convert_To_NanoSec(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t1SyncOriginTime);
    /* convert Receipt time of syncmsg at slave port into nano secs */
    t2Recv_time = (sint64)convert_To_NanoSec(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t2SyncReceiptTime);

    /* Calculate offset from master to slave port */
    master_to_slave_delay = ((t2Recv_time - t1Send_time) - EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.peer_mean_path_delay);
    /* master_to_slave_delay = (((((t2Recv_time - t1Send_time) - EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncCorrection) -
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.followupCorrection) - EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.peer_mean_path_delay ));*/
#if (ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON)
    /* Convert Offset calculation as per autosar req to send it as function parameter in EthIf_SetCorrectionTime and StbM_BusSetGlobalTime */
    tmp_Master_to_slave_delay = absolute(master_to_slave_delay);
    offset_From_Master.diff.secondsHi = (uint16)(((uint64)(tmp_Master_to_slave_delay.master_to_slave_delay/1000000000ULL)) >> 32u);
    offset_From_Master.diff.seconds = (uint32)(tmp_Master_to_slave_delay.master_to_slave_delay/1000000000ULL);
    offset_From_Master.diff.nanoseconds = (uint32)(tmp_Master_to_slave_delay.master_to_slave_delay % 1000000000ULL);
    offset_From_Master.sign = tmp_Master_to_slave_delay.sign;

    ctrlIdx = EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomain].EthTSynTimeDomain->EthTSynGlobalTimeEthIfId;
    /* @req 4.2.2/SWS_EthTSyn_00026 */
    /* Check Offset delay is exceeds config thresold delay */
    if((0 == EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomain].EthTSynTimeDomain->EthTSynTimeHardwareCorrectionThreshold) ||
            (tmp_Master_to_slave_delay.master_to_slave_delay > (EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomain].EthTSynTimeDomain->EthTSynTimeHardwareCorrectionThreshold))){
        /* Sync port w.r.t Master */
        EthIf_SetCorrectionTime(ctrlIdx, &offset_From_Master, &EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rateratio);
    }
#else

    uint8 timeBaseId =    EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomain].EthTSynSynchronizedTimeBaseRef;
    retVal = StbM_GetCurrentTime(timeBaseId, &stbmTimeStamp, &userData );
    if(E_OK == retVal){
        stbmTime = (sint64)convert_To_NanoSec_Stbm(stbmTimeStamp);
        /*lint -e{9033} */
        updatedStbmTimeStamp = (uint64)(stbmTime + master_to_slave_delay);
        /* Convert as per autosar std format to send it in StbM_BusSetGlobalTime function */
        stbmTimeStamp.secondsHi = (uint16)(((uint64)(updatedStbmTimeStamp/1000000000ULL)) >> 32u);
        stbmTimeStamp.seconds = (uint32)(updatedStbmTimeStamp/1000000000ULL);
        stbmTimeStamp.nanoseconds = (uint32)(updatedStbmTimeStamp % 1000000000ULL);
        /* @req 4.2.2/SWS_EthTSyn_00052 */
        retVal = StbM_BusSetGlobalTime(timeDomain, &stbmTimeStamp, NULL, TRUE);
        (void)retVal;
    }
#endif
}


/**
 * function to calculate Link delay rate ratio
 * @param timeDomain
 */
static void EthTSyn_Internal_ComputeDelay(uint8 timeDomain)
{
    /* compute rate ratio */
    if(TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.initPdelayRespReceived){
        computePdelayRateRatio(timeDomain);
        computePropTime(timeDomain);
        EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.isMeasuringDelay = TRUE;
    }
    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t4PdelayRespRxTime_old = EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t4PdelayRespRxTime;
    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t3PdelayRespRxTime_old = EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t3PdelayRespTime;
    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.initPdelayRespReceived = TRUE;
    /* compute Link Delay ratio */

    /* Check Pdelay Interval */
    if(TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pdelayIntervalTimeout){
        EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pDelayReq_State = SEND_PDELAY_REQ;
        EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pdelayTimerStarted = FALSE;
    }
}

static Std_ReturnType EthTSyn_Internal_Transmit_PdelayReq(uint8 timeDomain)
{
    Std_ReturnType retValue;
    retValue = E_NOT_OK;
    if(FALSE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.sentPdelayReq){
        /* @req 4.2.2/SWS_EthTSyn_00022 */
        if(ETHTSYN_TX_ON == EthTSyn_Internal.timeDomain[timeDomain].transmissionMode){
            retValue = EthTSyn_Internal_SendPdelayReq(timeDomain);
            if(E_OK == retValue){
                /* Pdelay Timer has started */
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pdelayTimerStarted = TRUE;
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pdelayIntervalTimer = 0;

            }
        }/* @req 4.2.2/SWS_EthTSyn_00021 */
    }else{
        EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pDelayReq_State = WAITING_FOR_PDELAY_RESP;
    }
    return retValue;
}
/**
 * state machine to handle Pdelay Req sent and received Pdelay Resp, FUP msg
 * @param timeDomain
 */
static void EthTSyn_Internal_PdelayReqStateMachine(uint8 timeDomain)
{
    Std_ReturnType retValue;

    //retValue = E_NOT_OK;
    switch(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pDelayReq_State){
        case INITIAL_SEND_PDELAY_REQ:
            retValue = EthTSyn_Internal_Transmit_PdelayReq(timeDomain);
            if(E_OK == retValue){
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.isMeasuringDelay = FALSE;
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.lostResponses = 0;
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.initPdelayRespReceived = FALSE;
            }
            break;

        case SEND_PDELAY_REQ:
            retValue = EthTSyn_Internal_Transmit_PdelayReq(timeDomain);
            (void)retValue;
            break;

        case WAITING_FOR_PDELAY_RESP:
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.sentPdelayReq = FALSE;  /* reset Pdelay req flag */
            if((TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rcvdPdelayResp) &&
                    (EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.last_pdelay_resp_tx_sequence_number[0] == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.last_pdelay_req_tx_sequence_number) ){

                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pDelayReq_State = WAITING_FOR_PDELAY_RESP_FOLLOW_UP;
            }else if((TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pdelayIntervalTimeout) ||
                    ((TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rcvdPdelayResp) && (EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.last_pdelay_resp_tx_sequence_number[0] != EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.last_pdelay_req_tx_sequence_number) )){

                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pdelayTimerStarted = FALSE;
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.initPdelayRespReceived = FALSE;
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.lostResponses += 1;
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pDelayReq_State = SEND_PDELAY_REQ;
            }else{}
            break;

        case WAITING_FOR_PDELAY_RESP_FOLLOW_UP:
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rcvdPdelayResp = FALSE;
            if((TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rcvdPdelayRespFollowUp) &&
                    ((EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.last_pdelay_resp_follow_tx_sequence_number[0] == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.last_pdelay_req_tx_sequence_number))){

                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pDelayReq_State = WAITING_FOR_PDELAY_INTERVAL_TIMER;
            }else if((TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pdelayIntervalTimeout) ||
                    ((TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rcvdPdelayRespFollowUp) && ((EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.last_pdelay_resp_follow_tx_sequence_number[0] != EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.last_pdelay_req_tx_sequence_number)))){

                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pdelayTimerStarted = FALSE;
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.initPdelayRespReceived = FALSE;
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.lostResponses += 1;
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pDelayReq_State = SEND_PDELAY_REQ;
            }else{}
            break;

        case WAITING_FOR_PDELAY_INTERVAL_TIMER: /*@req 4.2.2/SWS_EthTSyn_00004 */
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rcvdPdelayRespFollowUp = FALSE;
            if(TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pdelayIntervalTimeout){
                /* compute rate ratio */
                EthTSyn_Internal_ComputeDelay(timeDomain);
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pDelayReq_State = SEND_PDELAY_REQ;
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pdelayTimerStarted = FALSE;
            }
            break;

        default:
            break;
    }
}

/**
 * state machine to handle Pdelay Resp and follow up msg transimission
 * @param timeDomain
 */
static void EthTSyn_Internal_PdelayRespStateMachine(uint8 timeDomain)
{
  Std_ReturnType retValue;
  uint8 i;

  for (i=0; i < ETHTSYN_MAX_PDELAY_REQUEST; i++) {
    switch(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pDelayResp_State[i]){
        case INITIAL_WAITING_FOR_PDELAY_REQ:
            if(TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rcvdPdelayReq[i]){
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pDelayResp_State[i] = SENT_PDELAY_RESP_WAITING;
            }
            break;

        case WAITING_FOR_PDELAY_REQ:
            if(TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rcvdPdelayReq[i]){
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pDelayResp_State[i] = SENT_PDELAY_RESP_WAITING;
            }
            break;

        case SENT_PDELAY_RESP_WAITING:
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rcvdPdelayReq[i] = FALSE;
            if(ETHTSYN_TX_ON == EthTSyn_Internal.timeDomain[timeDomain].transmissionMode){
                /* Send Pdelay Resp since Pdelay Req is received */
                retValue = EthTSyn_Internal_SendPdelayResp(timeDomain, i);
                if(E_OK == retValue){
                    /* Pdelay Resp FollowUp offset Timer has started */
                    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pdelayRespTimerStarted[i] = TRUE;
                    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pdelayRespIntervalTimer[i] = 0;
                    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pDelayResp_State[i] = SENT_PDELAY_RESP_FOLLOWUP_WAITING;
                }else{
                    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pDelayResp_State[i] = WAITING_FOR_PDELAY_REQ;
                }
            }else{
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pDelayResp_State[i] = WAITING_FOR_PDELAY_REQ;
            }
            break;

        case SENT_PDELAY_RESP_FOLLOWUP_WAITING:
            if(ETHTSYN_TX_ON == EthTSyn_Internal.timeDomain[timeDomain].transmissionMode){
                if(TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pdelayRespTimeout[i]){
                    EthTSyn_Internal_SendPdelayRespFollowUp(timeDomain, i);
                    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pdelayRespTimeout[i] = FALSE;
                    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pDelayResp_State[i] = WAITING_FOR_PDELAY_REQ;
                }
            }else{
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.pDelayResp_State[i] = WAITING_FOR_PDELAY_REQ;
            }
            break;
        default:
            break;
    }
  }
}

/**
 * state machine to handle Sync and Sync follow Up msg transmission
 * @param timeDomain
 */
static void EthTSyn_Internal_SyncSendStateMachine(uint8 timeDomain)
{
    Std_ReturnType retValue;

    switch(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncSend_State){
        case SEND_SYNC_MSG:
            if(TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncIntervalTimeout){
                retValue = EthTSyn_Internal_SendSync(timeDomain);
                if(E_OK == retValue){
                    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.sentSync = FALSE;
                    /* Sync Interval timer started */
                    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncIntervalTimerStarted = TRUE;
                    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncIntervalTimer = 0;
                    /* Follow Up offset started */
                    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncfollowUpTimerStarted = TRUE;
                    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncfollowUpTimer = 0;
                    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncSend_State = SEND_FOLLOW_UP;
                }
            }
            break;

        case SEND_FOLLOW_UP:
            /* In case of no successful Tx confirmation for sync message and sync interval time out expires
             * reset the state to SEND_SYNC_MSG
             */
            if ((FALSE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.sentSync) &&
                    (TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncIntervalTimeout)) {
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.sentSync = FALSE;
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncfollowUpTimerStarted = FALSE;
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncfollowUpTimerTimeout = FALSE;
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncSend_State = SEND_SYNC_MSG;
            }
            /*@req 4.2.2/SWS_EthTSyn_00018 */
            else if((TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.sentSync) && (TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncfollowUpTimerTimeout)){
                retValue = EthTSyn_Internal_SendSyncFollowUp(timeDomain);
                if(E_OK == retValue){
                    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.sentSync = FALSE;
                    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncfollowUpTimerStarted = FALSE;
                    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncfollowUpTimerTimeout = FALSE;
                    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncSend_State = SEND_SYNC_MSG;
                }else{
                    if(TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncIntervalTimeout){
                        EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncSend_State = SEND_SYNC_MSG;
                        EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.sentSync = FALSE;
                    }
                }
            } else {
                /* Do nothing */
            }
            break;
        default:
            break;
    }
}

/**
 * state machine to handle Reception of Sync and Sync follow Up msg
 * @param timeDomain
 */
static void EthTSyn_Internal_SyncReceiveStateMachine(uint8 timeDomain)
{
    switch(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncReceive_State){
        case DISCARD:
            if(TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rcvdSync){
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncReceive_State = WAITING_FOR_FOLLOW_UP;
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncReceivedOnceFlag = TRUE;
            }
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rcvdSync = FALSE;
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rcvdFollowUp = FALSE;
            break;

        case WAITING_FOR_FOLLOW_UP:
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rcvdSync = FALSE;
            if((TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rcvdFollowUp)){
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncReceive_State = WAITING_FOR_SYNC;
            }else if(TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.receptionTimeout){
                /*@req 4.2.2/SWS_EthTSyn_00025 */
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncReceive_State = DISCARD;
            }else{}
            break;

        case WAITING_FOR_SYNC:
            EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rcvdFollowUp = FALSE;
            if(TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rcvdSync){
                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.syncReceive_State = WAITING_FOR_FOLLOW_UP;
            }
            break;

        default:
            break;
    }
}
/**
 * function to calculate rate ratio w.r.t master clock
 * @param timeDomain
 */
static void computeRateRatio(uint8 timeDomain)
{
    sint64 syncOriginTime_N;
    sint64 syncOriginTime_0;
    sint64 syncReceiptTime_N;
    sint64 syncReceiptTime_0;
    sint64 result_1;
    sint64 result_2;
    EthTSyn_Internal_TimeNanoType IngressTimeStamp;
    EthTSyn_Internal_TimeNanoType OriginTimeStamp;

    /* convert reception timestamp of Sync msg at slave port into nanoseconds */
    syncReceiptTime_N = (sint64)convert_To_NanoSec(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t2SyncReceiptTime);
    syncReceiptTime_0 = (sint64)convert_To_NanoSec(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t2SyncReceiptTime_old);
    /* convert origin timestamp of Sync msg at master port into nanoseconds */
    syncOriginTime_N = (sint64)convert_To_NanoSec(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t1SyncOriginTime);
    syncOriginTime_0 = (sint64)convert_To_NanoSec(EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t1SyncOriginTime_old);

    /*Get Ingress time and Engress Time stamp delta*/
    result_1 = (syncReceiptTime_N - syncReceiptTime_0);
    result_2 = (syncOriginTime_N - syncOriginTime_0);

    /* Represent IngressTimeStamp Delta as per AutoSar format */
    IngressTimeStamp = absolute(result_1);
    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rateratio.IngressTimeStampDelta.diff.secondsHi = (uint16)(((uint64)(IngressTimeStamp.master_to_slave_delay/1000000000ULL)) >> 32u);
    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rateratio.IngressTimeStampDelta.diff.seconds = (uint32)(IngressTimeStamp.master_to_slave_delay/1000000000ULL);
    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rateratio.IngressTimeStampDelta.diff.nanoseconds = (uint32)(IngressTimeStamp.master_to_slave_delay % 1000000000ULL);
    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rateratio.IngressTimeStampDelta.sign = IngressTimeStamp.sign;

    /* Represent OriginTimeStampDelta Delta as per AutoSar format */
    OriginTimeStamp = absolute(result_2);
    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rateratio.OriginTimeStampDelta.diff.secondsHi = (uint16)(((uint64)(OriginTimeStamp.master_to_slave_delay/1000000000ULL)) >> 32u);
    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rateratio.OriginTimeStampDelta.diff.seconds = (uint32)(OriginTimeStamp.master_to_slave_delay/1000000000ULL);
    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rateratio.OriginTimeStampDelta.diff.nanoseconds = (uint32)(OriginTimeStamp.master_to_slave_delay % 1000000000ULL);
    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rateratio.OriginTimeStampDelta.sign = OriginTimeStamp.sign;
}

/**
 * function to calculate rate ratio w.r.t master clock
 * @param timeDomain
 */
static void EthTSyn_Internal_Rate_Ratio(uint8 timeDomain)
{
    if(TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.initSyncFollowReceived){
        computeRateRatio(timeDomain);
    }
    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.initSyncFollowReceived = TRUE;
    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t1SyncOriginTime_old = EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t1SyncOriginTime;
    EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t2SyncReceiptTime_old = EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.t2SyncReceiptTime;
}
/**
 *
 */
/* @req 4.2.2/SWS_EthTSyn_00050 */
void EthTSyn_MainFunction(void){
    uint8 timeDomain;

    /* @req 4.2.2/SWS_EthTSyn_00046 */
    ETHTSYN_DET_REPORTERROR((ETHTSYN_STATE_INIT == EthTSyn_Internal.initStatus), ETHTSYN_MAINFUNCTION_ID, ETHTSYN_E_NOT_INITIALIZED);

    for(timeDomain =0 ; timeDomain <  EthTSyn_ConfigPointer->EhtTSyn_GlobalTimeDomainCount; timeDomain ++){
        if(TRUE == EthTSyn_Internal.timeDomain[timeDomain].trcvActiveStateFlag){
            switch(EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timeDomain].EthTSynMasterFlag){
                case PTP_MASTER:
                    if(ETHTSYN_TX_ON == EthTSyn_Internal.timeDomain[timeDomain].transmissionMode){
#if (ETHTSYN_SEND_ANNOUNCE_SUPPORT == STD_ON)
                	    Std_ReturnType retValue;
                		/* Check if it is time to transmit announce message */
                        if(TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.announceIntervalTimeout){
                            retValue = EthTSyn_Internal_SendAnnounce(timeDomain);
                            if(E_OK == retValue){
                                /* Announce Interval timer started */
                                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.announceIntervalTimerStarted = TRUE;
                                EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.announceIntervalTimer = 0;
                            }
                        } else
#endif
                        {

                        	/* Transmission - Send Sync and Follow Up msgs */
                            EthTSyn_Internal_SyncSendStateMachine(timeDomain); /* @req 4.2.2/SWS_EthTSyn_00016 */
                        }
                    }
                    /* Transmission - Pdelay Resp and Follow Up msg*/
                    EthTSyn_Internal_PdelayRespStateMachine(timeDomain); /* @req 4.2.2/SWS_EthTSyn_00012 */
                    break;

                case PTP_SLAVE: /* @req 4.2.2/SWS_EthTSyn_00023 */
                    /* Reception of Sync and Follow Up msg */
                    EthTSyn_Internal_SyncReceiveStateMachine(timeDomain);
                    if(TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.rcvdFollowUp){
                        EthTSyn_Internal_Rate_Ratio(timeDomain);
                    }
                    /* Transmission of Pdelay_Req and Reception of Pdelay_Resp and Pdelay_Resp_Follow_Up msg */
                    EthTSyn_Internal_PdelayReqStateMachine(timeDomain);  /*@req 4.2.2/SWS_EthTSyn_00011 */

                    /* Sync Correction Calculation */
                    if(TRUE == EthTSyn_Internal.timeDomain[timeDomain].ptpCfgData.isMeasuringDelay){
                        synchronize_clock(timeDomain);
                    }
                    break;

                default:
                    break;
            }
            EthTSyn_Internal_UpdateTimer(timeDomain);
        }
    }
}

#ifdef HOST_TEST
EthTSyn_InternalType* readinternal_EthTsynstatus(void );
EthTSyn_InternalType* readinternal_EthTsynstatus(void)
{
    return &EthTSyn_Internal;
}
#endif
