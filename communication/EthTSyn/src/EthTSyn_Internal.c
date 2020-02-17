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

#include "EthTSyn.h"
#include "EthTSyn_Internal.h"
#include <string.h>

/* @req 4.2.2/SWS_EthTSyn_00028 */ /* Message format derived from IEEE 802.1As */

extern EthTSyn_InternalType EthTSyn_Internal;
#ifdef HOST_TEST
/*lint -esym(9003, testBuffers) */
EthTsyn_Internal_MsgSync testBuffers[2];
#endif

#define TWO_STEP_CLOCK                2u         /* Follow up msg supported */
#define ANNOUNCE_LEAP59               4u         /* if the last minute of the current UTC day,
                                                    relative to the current grandmaster, contains 59 s, */
#define ANNOUNCE_UTC_OFFSET_VALID     8u         /* if currentUtcOffset relative to the current grandmaster,
                                                    is known to be correct */
#define TRANSPORT_SPECIFIC  (1u << 4u) /* Set transportSpecific = 1 for 802.1AS */

extern const EthTSyn_ConfigType* EthTSyn_ConfigPointer;
/*lint -save -e572 -e778 -e9032 */
/**
 *
 * @param bufPtr
 * @param timedomain
 */
void EthTSyn_Internal_MsgPackPdelayReqMsg(EthTsyn_Internal_MsgPDelayReq *bufPtr, uint8 timedomain)
{
    /* PTP Header */
    memset(bufPtr, 0, V2_PDELAY_REQ_LENGTH); /* Reset packet to all zeros */
    /* Message type, length, flags, Sequence, Control, log mean message interval */
    bufPtr->headerMsg.transportSpecificAndMessageType = TRANSPORT_SPECIFIC;     /* Set transportSpecific = 1 and Clear previous Message type */
    bufPtr->headerMsg.transportSpecificAndMessageType |= V2_PDELAY_REQ_MESSAGE;
    bufPtr->headerMsg.reserved1AndVersionPTP = V2_VERSION_PTP;
    bufPtr->headerMsg.messageLength = ETHTSYN_HEADER_PDELAY_REQ_LENGTH_BE;

    memcpy(bufPtr->headerMsg.sourcePortId.clockIdentity, EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.port_clock_identity, CLOCK_IDENTITY_LENGTH);
    bufPtr->headerMsg.sourcePortId.portNumber = ETHTSYN_HEADER_DEFAULT_PORT_NUMBER_BE;   /* Ordinary clock so its 1 */
    bufPtr->headerMsg.sequenceId = bswap16(EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.last_pdelay_req_tx_sequence_number) ;
    bufPtr->headerMsg.control = V2_ALL_OTHERS_CONTROL;
    bufPtr->headerMsg.logMeanMessageInterval = LOGMEAN_PDELAY_REQ;
#ifdef HOST_TEST
    memcpy(&testBuffers[0], bufPtr, V2_PDELAY_REQ_LENGTH);
#endif
}
/**
 *
 * @param bufPtr
 * @param timedomain
 * @param req_index
 */
void EthTSyn_Internal_MsgPackPdelayRespMsg(EthTsyn_Internal_MsgPDelayResp *bufPtr, uint8 timedomain, uint8 req_index)
{
    /* PTP Header */
    memset(bufPtr, 0, V2_PDELAY_RESP_LENGTH);/* Reset packet to all zeros */
    /* Message type, length, flags, Sequence, Control, log mean message interval */
    bufPtr->headerMsg.transportSpecificAndMessageType = TRANSPORT_SPECIFIC;                   /* Set transportSpecific = 1 and Clear previous Message type */
    bufPtr->headerMsg.transportSpecificAndMessageType |= V2_PDELAY_RESP_MESSAGE;
    bufPtr->headerMsg.reserved1AndVersionPTP = V2_VERSION_PTP;
    bufPtr->headerMsg.messageLength = ETHTSYN_HEADER_PDELAY_RESP_LENGTH_BE;
    bufPtr->headerMsg.flags[0] = TWO_STEP_CLOCK;
    memcpy(bufPtr->headerMsg.sourcePortId.clockIdentity, EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.port_clock_identity, CLOCK_IDENTITY_LENGTH);
    bufPtr->headerMsg.sourcePortId.portNumber = ETHTSYN_HEADER_DEFAULT_PORT_NUMBER_BE; /* Ordinary clock so its 1 */

    bufPtr->headerMsg.sequenceId = bswap16(EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.last_pdelay_resp_tx_sequence_number[req_index]);
    bufPtr->headerMsg.control = V2_ALL_OTHERS_CONTROL;
    bufPtr->headerMsg.logMeanMessageInterval = LOGMEAN_PDELAY_RESP;

    bufPtr->receiveTimestamp.nanoseconds = bswap32(EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.t2PdelayReqRxTime[req_index].nanoseconds);
    bufPtr->receiveTimestamp.seconds = bswap32(EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.t2PdelayReqRxTime[req_index].seconds);
    bufPtr->receiveTimestamp.secondsHi = bswap16(EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.t2PdelayReqRxTime[req_index].secondsHi);

    memcpy(bufPtr->requestingPortId.clockIdentity, EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.requestingPortIdentity[req_index].clockIdentity,CLOCK_IDENTITY_LENGTH);
    bufPtr->requestingPortId.portNumber = bswap16(EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.requestingPortIdentity[req_index].portNumber);
#ifdef HOST_TEST
    memcpy(&testBuffers[0], bufPtr, V2_PDELAY_RESP_LENGTH);
#endif
}
/**
 *
 * @param bufPtr
 * @param timedomain
 * @param req_index
 */
void EthTSyn_Internal_MsgPackPdelayRespFollowUpMsg(EthTsyn_Internal_MsgPDelayRespFollowUp *bufPtr, uint8 timedomain, uint8 req_index)
{
    /* PTP Header */
    memset(bufPtr, 0, V2_PDELAY_RESP_FOLLOWUP_LENGTH);/* Reset packet to all zeros */
    /* Message type, length, flags, Sequence, Control, log mean message interval */
    bufPtr->headerMsg.transportSpecificAndMessageType = TRANSPORT_SPECIFIC;                   /* Set transportSpecific = 1 and Clear previous Message type */
    bufPtr->headerMsg.transportSpecificAndMessageType |= V2_PDELAY_RESP_FOLLOWUP_MESSAGE;
    bufPtr->headerMsg.reserved1AndVersionPTP = V2_VERSION_PTP;
    bufPtr->headerMsg.messageLength = ETHTSYN_HEADER_PDELAY_RESP_FOLLOWUP_LENGTH_BE;

    memcpy(bufPtr->headerMsg.sourcePortId.clockIdentity, EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.port_clock_identity, CLOCK_IDENTITY_LENGTH);
    bufPtr->headerMsg.sourcePortId.portNumber = ETHTSYN_HEADER_DEFAULT_PORT_NUMBER_BE;  /* Ordinary clock so its 1 */

    bufPtr->headerMsg.sequenceId = bswap16(EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.last_pdelay_resp_tx_sequence_number[req_index]);
    bufPtr->headerMsg.control = V2_ALL_OTHERS_CONTROL;
    bufPtr->headerMsg.logMeanMessageInterval = LOGMEAN_PDELAY_RESP_FOLLOWUP;

    bufPtr->responseOriginTimestamp.nanoseconds = bswap32(EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.t3PdelayRespTxTime[req_index].nanoseconds);
    bufPtr->responseOriginTimestamp.seconds = bswap32(EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.t3PdelayRespTxTime[req_index].seconds);
    bufPtr->responseOriginTimestamp.secondsHi = bswap16(EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.t3PdelayRespTxTime[req_index].secondsHi);

    memcpy(bufPtr->requestingPortId.clockIdentity, EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.requestingPortIdentity[req_index].clockIdentity,CLOCK_IDENTITY_LENGTH);
    bufPtr->requestingPortId.portNumber = bswap16(EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.requestingPortIdentity[req_index].portNumber);

#ifdef HOST_TEST
    memcpy(&testBuffers[0], bufPtr, V2_PDELAY_RESP_FOLLOWUP_LENGTH);
#endif
}
/**
 *
 * @param bufPtr
 * @param timedomain
 */
void EthTSyn_Internal_MsgPackSyncMsg( EthTsyn_Internal_MsgSync *bufPtr, uint8 timedomain)
{
    /* PTP Header */
    memset(bufPtr, 0, V2_SYNC_LENGTH);/* Reset packet to all zeros */
    /* Message type, length, flags, Sequence, Control, log mean message interval */
    bufPtr->headerMsg.transportSpecificAndMessageType = TRANSPORT_SPECIFIC;   /* Set transportSpecific = 1 and Clear previous Message type */
    bufPtr->headerMsg.transportSpecificAndMessageType |= V2_SYNC_MESSAGE;
    bufPtr->headerMsg.reserved1AndVersionPTP = V2_VERSION_PTP;
    bufPtr->headerMsg.messageLength = ETHTSYN_HEADER_SYNC_LENGTH_BE;

    bufPtr->headerMsg.flags[0] = TWO_STEP_CLOCK;

    memcpy(bufPtr->headerMsg.sourcePortId.clockIdentity, EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.port_clock_identity, CLOCK_IDENTITY_LENGTH);
    bufPtr->headerMsg.sourcePortId.portNumber = ETHTSYN_HEADER_DEFAULT_PORT_NUMBER_BE;  /* Ordinary clock so its 1 */

    bufPtr->headerMsg.sequenceId = bswap16(EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.last_sync_tx_sequence_number);
    bufPtr->headerMsg.control = V2_SYNC_CONTROL;
    /* bufPtr->headerMsg.logMeanMessageInterval = (sint8)log2f((float32)((float32)(EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[i].EthTSynTimeDomain->EthTSynGlobalTimeTxPeriod) * ETHTSYN_MAIN_FUNCTION_PERIOD_LOG)); */ /* Need to check */
    bufPtr->headerMsg.logMeanMessageInterval = (sint8)EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timedomain].EthTSynTimeDomain->EthTSynGlobalTimeTxPeriod * ETHTSYN_MAIN_FUNCTION_PERIOD_LOG;  /* Need to check */

#ifdef HOST_TEST
    memcpy(&testBuffers[0], bufPtr, V2_SYNC_LENGTH);
#endif
}
#if (ETHTSYN_SEND_ANNOUNCE_SUPPORT == STD_ON)
/**
 *
 * @param bufPtr
 * @param timedomain
 */
void EthTSyn_Internal_MsgPackAnnounceMsg( EthTsyn_Internal_MsgAnnounce *bufPtr, uint8 timedomain)
{
    /* PTP Header */
    memset(bufPtr, 0, V2_ANNOUNCE_LENGTH);/* Reset packet to all zeros */
    /* Message type, length, flags, Sequence, Control, log mean message interval */
    bufPtr->headerMsg.transportSpecificAndMessageType = TRANSPORT_SPECIFIC;   /* Set transportSpecific = 1 and Clear previous Message type */
    bufPtr->headerMsg.transportSpecificAndMessageType |= V2_ANNOUNCE_MESSAGE;
    bufPtr->headerMsg.reserved1AndVersionPTP = V2_VERSION_PTP;
    bufPtr->headerMsg.messageLength = bswap16(V2_ANNOUNCE_LENGTH + 4 + V2_CLOCKPATH_TLV_LEN);
    bufPtr->headerMsg.flags[1] = (ANNOUNCE_LEAP59 | ANNOUNCE_UTC_OFFSET_VALID);
    /*bufPtr->headerMsg.correctionField is left as 0 */
    memcpy(bufPtr->headerMsg.sourcePortId.clockIdentity, EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.port_clock_identity, CLOCK_IDENTITY_LENGTH);
    bufPtr->headerMsg.sourcePortId.portNumber = ETHTSYN_HEADER_DEFAULT_PORT_NUMBER_BE;  /* Ordinary clock so its 1 */
    bufPtr->headerMsg.sequenceId = bswap16(EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.last_announce_tx_sequence_number);
    bufPtr->headerMsg.control = V2_ALL_OTHERS_CONTROL;
    bufPtr->headerMsg.logMeanMessageInterval = (sint8)EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timedomain].EthTSynTimeDomain->EthTSynGlobalTimeTxPeriod * ETHTSYN_MAIN_FUNCTION_PERIOD_LOG * 2; /* */

    bufPtr->currentUtcOffset = 0;
    bufPtr->grandmasterPriority1 = 10; /* May be set 1-255, 255 for a not grandmaster capable system, 0 is for management use. */
    bufPtr->grandmasterClockQuality = 0xF8FE4100u; /* Class - 248, Accuracy - 254 and Variance - 16640 */
    bufPtr->grandmasterPriority2 = 12; /* May be set 1-255, 255 for a not grandmaster capable system, 0 is for management use. */
    memcpy(bufPtr->grandmasterIdentity, EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.port_clock_identity, CLOCK_IDENTITY_LENGTH);
    bufPtr->stepsRemoved = 0; /* We assume we are grandmaster capable,  else this is the number of links in the path from the root to our network */
    bufPtr->timeSource = 0xA0; /* Clock derived from internal oscillator */
    bufPtr->clockpath.tlv = bswap16(V2_CLOCKPATH_TLV_ID);
    bufPtr->clockpath.length = bswap16(V2_CLOCKPATH_TLV_LEN * 1);
    memcpy(bufPtr->clockpath.clockIdentity, EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.port_clock_identity, CLOCK_IDENTITY_LENGTH);
#ifdef HOST_TEST
    memcpy(&testBuffers[0], bufPtr, V2_ANNOUNCE_LENGTH);
#endif
}
#endif
/**
 *
 * @param bufPtr
 * @param timedomain
 */
void EthTSyn_Internal_MsgPackSyncFollow(EthTsyn_Internal_MsgFollowUp *bufPtr, uint8 timedomain)
{
    /* PTP Header */
    memset(bufPtr, 0, V2_FOLLOWUP_LENGTH);/* Reset packet to all zeros */
    /* Message type, length, flags, Sequence, Control, log mean message interval */
    bufPtr->headerMsg.transportSpecificAndMessageType = TRANSPORT_SPECIFIC;                   /* Clear previous Message type */
    bufPtr->headerMsg.transportSpecificAndMessageType |= V2_FOLLOWUP_MESSAGE;
    bufPtr->headerMsg.reserved1AndVersionPTP = V2_VERSION_PTP;
    bufPtr->headerMsg.messageLength =  ETHTSYN_HEADER_FOLLOWUP_LENGTH_BE;

    memcpy(bufPtr->headerMsg.sourcePortId.clockIdentity, EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.port_clock_identity, 8);
    bufPtr->headerMsg.sourcePortId.portNumber = ETHTSYN_HEADER_DEFAULT_PORT_NUMBER_BE;  /* Ordinary clock so its 1 */

    bufPtr->headerMsg.sequenceId = bswap16(EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.last_sync_tx_sequence_number);
    bufPtr->headerMsg.control = V2_FOLLOWUP_CONTROL;

 /* bufPtr->headerMsg.logMeanMessageInterval = (sint8)log2f((float32)(((float32)EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[i].EthTSynTimeDomain->EthTSynGlobalTimeTxFollowUpOffset) * ETHTSYN_MAIN_FUNCTION_PERIOD_LOG) ); */ /* Need to check */;
    bufPtr->headerMsg.logMeanMessageInterval = (sint8)EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timedomain].EthTSynTimeDomain->EthTSynGlobalTimeTxFollowUpOffset * ETHTSYN_MAIN_FUNCTION_PERIOD_LOG;
    bufPtr->preciseOriginTimestamp.nanoseconds = bswap32(EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.t1SyncTxTime.nanoseconds) ;
    bufPtr->preciseOriginTimestamp.seconds = bswap32(EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.t1SyncTxTime.seconds);
    bufPtr->preciseOriginTimestamp.secondsHi = bswap16(EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.t1SyncTxTime.secondsHi);
#ifdef HOST_TEST
    memcpy(&testBuffers[0], bufPtr, V2_FOLLOWUP_LENGTH);
#endif
}

/**
 *
 * @param X
 * @return
 */
uint64 convert_To_NanoSec(Eth_TimeStampType X)
{
    uint64 result;
    uint64 second48;

    second48 = ((((uint64)X.secondsHi << 32u)) | X.seconds);
    result = (1000000000 * second48) + X.nanoseconds;
    return (result);
}

/**
 *
 * @param X
 * @return
 */
uint64 convert_To_NanoSec_Stbm(StbM_TimeStampType X)
{
    uint64 result;
    uint64 second48;

    second48 = ((((uint64)X.secondsHi << 32u)) | X.seconds);
    result = (1000000000 * second48) + X.nanoseconds;
    return (result);
}

/**
 *
 * @param value
 * @return
 */
EthTSyn_Internal_TimeNanoType absolute(sint64 value) {
    EthTSyn_Internal_TimeNanoType tmp_val;
  if (value < 0) {
      tmp_val.master_to_slave_delay = (uint64)-value;
      tmp_val.sign = TRUE;
  }
  else {
      tmp_val.master_to_slave_delay = (uint64)value;
      tmp_val.sign = FALSE;
  }
  return tmp_val;
}
/**
 *
 * @param timedomain
 */
void EthTSyn_Internal_UpdateTimer(uint8 timedomain)
{
	uint8 i;
    boolean timerExpire;
    /* For the timers started in EthTSyn_MainFunction() context (except RxFollowUpTimer):
     * Timeout status is set one main function before than configured threshold by this function.This enables timeout status to be recognized in the next main function.
     * Note: EthTSyn_Internal_UpdateTimer() is called at the end of EthTSyn_MainFunction() */
#if (ETHTSYN_SEND_ANNOUNCE_SUPPORT == STD_ON)
    /* Announce Interval time counter */
    if(TRUE == EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.announceIntervalTimerStarted){
        timerExpire = ((EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timedomain].EthTSynTimeDomain->EthTSynGlobalTimeTxPeriod == 0) ||
                      (EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.announceIntervalTimer >= (EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timedomain].EthTSynTimeDomain->EthTSynGlobalTimeTxPeriod -1)));
        if (FALSE == timerExpire){
            EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.announceIntervalTimer++;
            EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.announceIntervalTimeout = FALSE;
        }else {
            EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.announceIntervalTimeout = TRUE;
        }
    }
#endif
    /* Follow Up Timer counter */
	if(TRUE == EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.syncfollowUpTimerStarted){
		timerExpire = ((EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timedomain].EthTSynTimeDomain->EthTSynGlobalTimeTxFollowUpOffset == 0) ||
						  (EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.syncfollowUpTimer >= (EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timedomain].EthTSynTimeDomain->EthTSynGlobalTimeTxFollowUpOffset -1)));
		if (FALSE == timerExpire){
			EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.syncfollowUpTimer++;
			EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.syncfollowUpTimerTimeout = FALSE;
		} else {
			EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.syncfollowUpTimerTimeout = TRUE;
		}
	}

	/* Sync Interval time counter */
    if(TRUE == EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.syncIntervalTimerStarted){
        timerExpire = ((EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timedomain].EthTSynTimeDomain->EthTSynGlobalTimeTxPeriod == 0) ||
                      (EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.syncIntervalTimer >= (EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timedomain].EthTSynTimeDomain->EthTSynGlobalTimeTxPeriod -1)));
        if (FALSE == timerExpire){
            EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.syncIntervalTimer++;
            EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.syncIntervalTimeout = FALSE;
        }else {
            EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.syncIntervalTimeout = TRUE;
        }
    }

    /* Reception Timeout counter */
    if(TRUE == EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.RxFollowUpTimerStarted){
        timerExpire = ((EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timedomain].EthTSynGlobalTimeFollowUpTimeout == 0) ||
                        (EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.RxFollowUpTimer > (EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timedomain].EthTSynGlobalTimeFollowUpTimeout - 1)));
        if(FALSE == timerExpire){
            EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.RxFollowUpTimer++;
            EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.receptionTimeout = FALSE;
        }else{
            EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.receptionTimeout = TRUE;
        }
    }

    /* Pdelay Req interval counter */
    if(TRUE == EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.pdelayTimerStarted){
        timerExpire = ((EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timedomain].EthTSynTimeDomain->EthTSynGlobalTimeTxPdelayReqPeriod == 0) ||
                        (EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.pdelayIntervalTimer >= (EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timedomain].EthTSynTimeDomain->EthTSynGlobalTimeTxPdelayReqPeriod - 1)));
        if( FALSE == timerExpire){
            EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.pdelayIntervalTimer++;
            EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.pdelayIntervalTimeout = FALSE;
        }else {
            EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.pdelayIntervalTimeout = TRUE;
        }
    }

    for (i = 0; i < ETHTSYN_MAX_PDELAY_REQUEST; i++) {
        /* Pdelay Resp interval counter */
        if((TRUE == EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.pdelayRespTimerStarted[i])){
            timerExpire = ((EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timedomain].EthTSynTimeDomain->EthTSynGlobalTimeTxFollowUpOffset == 0)||
                            (EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.pdelayRespIntervalTimer[i] >= (EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timedomain].EthTSynTimeDomain->EthTSynGlobalTimeTxFollowUpOffset-1)));
            if(FALSE == timerExpire){
                EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.pdelayRespIntervalTimer[i]++;
                EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.pdelayRespTimeout[i] = FALSE;
            }else {
                EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.pdelayRespTimeout[i] = TRUE;
            }
        }
        /* Follow Up Timer counter */
        if((TRUE == EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.pdelayRespfollowUpTimerStarted[i])){
            timerExpire = ((EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timedomain].EthTSynTimeDomain->EthTSynGlobalTimeTxFollowUpOffset == 0) ||
                             (EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.pdelayRespfollowUpTimer[i] >= (EthTSyn_ConfigPointer->EthTSynGlobalTimeDomain[timedomain].EthTSynTimeDomain->EthTSynGlobalTimeTxFollowUpOffset -1)));
            if (FALSE == timerExpire){
                EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.pdelayRespfollowUpTimer[i]++;
                EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.pdelayRespfollowUpTimerTimeout[i] = FALSE;
            } else {
                EthTSyn_Internal.timeDomain[timedomain].ptpCfgData.pdelayRespfollowUpTimerTimeout[i] = TRUE;
            }
        }
    }
}

/*lint -restore */
