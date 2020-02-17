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
#ifndef ETHTSYN_INTERNAL_H_
#define ETHTSYN_INTERNAL_H_

#include "EthTSyn_Types.h"
#include "Eth_GeneralTypes.h"
/*lint -e9045 */
#include "math.h"

/* Version 2 control field values */
#define V2_SYNC_CONTROL                 0x00u
#define V2_FOLLOWUP_CONTROL             0x02u
#define V2_ALL_OTHERS_CONTROL           0x05u

/* General messages */
#define V2_PDELAY_REQ_LENGTH            54u
#define V2_ANNOUNCE_LENGTH              (44u+20u)
#define V2_SYNC_LENGTH                  44u
#define V2_FOLLOWUP_LENGTH              44u
#define V2_PDELAY_RESP_LENGTH           54u
#define V2_PDELAY_RESP_FOLLOWUP_LENGTH  54u

/* Event messages */
#define V2_SYNC_MESSAGE                 0x0u
#define V2_PDELAY_REQ_MESSAGE           0x2u
#define V2_PDELAY_RESP_MESSAGE          0x3u

/* General messages */
#define V2_FOLLOWUP_MESSAGE             0x8u
#define V2_PDELAY_RESP_FOLLOWUP_MESSAGE 0xAu
#define V2_ANNOUNCE_MESSAGE             0xBu

#define V2_VERSION_PTP                  0x02u

#define V2_CLOCKPATH_TLV_ID             0x08u
#define V2_CLOCKPATH_TLV_LEN            0x08u


#define TIME_DOMAIN_0                   0u
#define LOGMEAN_PDELAY_REQ              0x7fu
#define LOGMEAN_PDELAY_RESP             0x7fu
#define LOGMEAN_PDELAY_RESP_FOLLOWUP    0x7fu
#define ETH_FRAME_TYPE_TSYN             0x88F7u
#define PTP_UUID_LENGTH                 6u

#define CLOCK_IDENTITY_LENGTH           8u
#define V2_TWO_STEP_FLAG                0x02u  /* Bit 1 */
#define DEFAULT_PORT_NUMBER             1u

#define ETHTSYN_MAX_PDELAY_REQUEST      6 /* The max number of peer delay requests this implementation will handle */

#if (CPU_BYTE_ORDER == LOW_BYTE_FIRST)
#define bswap32(x)      (((uint32)(x) >> 24u) | (((uint32)(x) >> 8u) & 0xff00u) | (((uint32)(x) & 0xff00uL ) << 8u) | (((uint32)(x) & 0xffu) << 24u) )
#define bswap16(x)      (((uint16)(x) >> 8u) | (((uint16)(x) & 0xffu) << 8u))
/* General messages */
#define ETHTSYN_HEADER_PDELAY_REQ_LENGTH_BE            bswap16(V2_PDELAY_REQ_LENGTH)
#define ETHTSYN_HEADER_SYNC_LENGTH_BE                  bswap16(V2_SYNC_LENGTH)
#define ETHTSYN_HEADER_ANNOUNCE_LENGTH_BE              bswap16(V2_ANNOUNCE_LENGTH)
#define ETHTSYN_HEADER_FOLLOWUP_LENGTH_BE              bswap16(V2_FOLLOWUP_LENGTH)
#define ETHTSYN_HEADER_PDELAY_RESP_LENGTH_BE           bswap16(V2_PDELAY_RESP_LENGTH)
#define ETHTSYN_HEADER_PDELAY_RESP_FOLLOWUP_LENGTH_BE  bswap16(V2_PDELAY_RESP_FOLLOWUP_LENGTH)
#define ETHTSYN_HEADER_DEFAULT_PORT_NUMBER_BE          bswap16(DEFAULT_PORT_NUMBER)
#else
#define bswap32(x)      (x)
#define bswap16(x)      (x)
#define ETHTSYN_HEADER_PDELAY_REQ_LENGTH_BE            V2_PDELAY_REQ_LENGTH
#define ETHTSYN_HEADER_SYNC_LENGTH_BE                  V2_SYNC_LENGTH
#define ETHTSYN_HEADER_ANNOUNCE_LENGTH_BE              V2_ANNOUNCE_LENGTH
#define ETHTSYN_HEADER_FOLLOWUP_LENGTH_BE              V2_FOLLOWUP_LENGTH
#define ETHTSYN_HEADER_PDELAY_RESP_LENGTH_BE           V2_PDELAY_RESP_LENGTH
#define ETHTSYN_HEADER_PDELAY_RESP_FOLLOWUP_LENGTH_BE  V2_PDELAY_RESP_FOLLOWUP_LENGTH
#define ETHTSYN_HEADER_DEFAULT_PORT_NUMBER_BE          DEFAULT_PORT_NUMBER

#endif

/* Internal type defined for  Time Domain */
typedef uint16 EthTSyn_TimeDomainType;

typedef enum {
    ETHTSYN_STATE_UNINIT,   /* Status of EthTSyn module before EthIf_Init function*/
    ETHTSYN_STATE_INIT,     /* Status of EthTSyn module after EthIf_Init function called*/
}EthTSyn_Internal_StateType;

typedef struct
{
    uint64 master_to_slave_delay;  /* path delay */
    boolean sign;                  /* +ve or -ve sign of delay */
}EthTSyn_Internal_TimeNanoType;
typedef struct __attribute__ ((packed))
{
  uint8    clockIdentity[CLOCK_IDENTITY_LENGTH];  /* array formed by mapping an IEEE EUI-48 assigned to the time-aware system to IEEE EUI-64 format*/
  uint16   portNumber;        /* value = 1 for port on a time-aware end station and value = 1.2...n for multiport(bridge)*/
} PortIdentity;

/** IEEE 802.1AS PTP Version 2 common Message header structure */
/* @req 4.2.2/SWS_EthTSyn_00028 */
typedef struct __attribute__ ((packed))
{                                                      // Offset  Length (bytes)
  uint8        transportSpecificAndMessageType;       // 00       1 (2 4-bit fields)
  uint8        reserved1AndVersionPTP;                // 01       1 (2 4-bit fields)
  uint16       messageLength;                         // 02       2
  uint8        domainNumber;                          // 04       1
  uint8        reserved2;                             // 05       1
  uint8        flags[2];                              // 06       2
  sint64       correctionField;                       // 08       8
  uint32       reserved3;                             // 16       4
  PortIdentity sourcePortId;                          // 20      10
  uint16       sequenceId;                            // 30       2
  uint8        control;                               // 32       1
  sint8        logMeanMessageInterval;                // 33       1
} EthTsyn_Internal_MsgHeader;

/** PTP Version 2 Sync message structure */
typedef struct __attribute__ ((packed))
{
   EthTsyn_Internal_MsgHeader headerMsg;
   uint8 reserved[10];
} EthTsyn_Internal_MsgSync;

/** PTP Version 2 clock path message structure */
typedef struct __attribute__ ((packed))
{
   uint16 tlv;
   uint16 length;
   uint8 clockIdentity[CLOCK_IDENTITY_LENGTH][1];
} EthTsyn_Internal_TlvClockPath;

/** PTP Version 2 Announce message structure */
typedef struct __attribute__ ((packed))
{
   EthTsyn_Internal_MsgHeader headerMsg;
   uint8 reserved[10];
   uint16 currentUtcOffset;
   uint8 reserved2;
   uint8 grandmasterPriority1;
   uint32 grandmasterClockQuality;
   uint8 grandmasterPriority2;
   uint8 grandmasterIdentity[CLOCK_IDENTITY_LENGTH];
   uint16 stepsRemoved;
   uint8 timeSource;
   EthTsyn_Internal_TlvClockPath clockpath;
} EthTsyn_Internal_MsgAnnounce;

/** PTP Version 2 Follow Up message structure */
typedef struct __attribute__ ((packed))
{
  EthTsyn_Internal_MsgHeader headerMsg;
  Eth_TimeStampType preciseOriginTimestamp;
} EthTsyn_Internal_MsgFollowUp;

/** PTP Version 2 PDelay Resp message structure */
typedef struct __attribute__ ((packed))
{
  EthTsyn_Internal_MsgHeader headerMsg;
  Eth_TimeStampType receiveTimestamp;
  PortIdentity      requestingPortId;
} EthTsyn_Internal_MsgPDelayResp;

/** PTP Version 2 PDelay Req message structure */
typedef struct __attribute__ ((packed))
{
  EthTsyn_Internal_MsgHeader headerMsg;
  Eth_TimeStampType originTimestamp;
  uint8                reserved[10];
} EthTsyn_Internal_MsgPDelayReq;

typedef struct __attribute__ ((packed))
{
  EthTsyn_Internal_MsgHeader headerMsg;
  Eth_TimeStampType responseOriginTimestamp;
  PortIdentity      requestingPortId;
} EthTsyn_Internal_MsgPDelayRespFollowUp;

typedef enum{
    TXSYNC,
    RXSYNC,
    TXPDELAY_REQ,
    RXPDELAY_REQ,
    TXPDELAY_RESP,
    RXPDELAY_RESP,
}EthTSyn_Internal_MessageType;

/* State for Sync Transmit */
typedef enum{
	SEND_ANNOUNCE_MSG,
    SEND_SYNC_MSG,
    SEND_FOLLOW_UP,
}EthTSyn_Internal_SyncSend_StateMachineType;
/* states of Pdelay Req */
typedef enum{
    INITIAL_SEND_PDELAY_REQ,
    SEND_PDELAY_REQ,
    WAITING_FOR_PDELAY_RESP,
    WAITING_FOR_PDELAY_RESP_FOLLOW_UP,
    WAITING_FOR_PDELAY_INTERVAL_TIMER,
}EthTSyn_Internal_Pdelay_StateMachineType;

/* states of Pdelay Resp and FollowUp */
typedef enum{
    INITIAL_WAITING_FOR_PDELAY_REQ,
    WAITING_FOR_PDELAY_REQ,
    SENT_PDELAY_RESP_WAITING,
    SENT_PDELAY_RESP_FOLLOWUP_WAITING,
}EthTSyn_Internal_Pdelay_Resp_StateMachineType;

/* states of Sync Receive */
typedef enum{
    DISCARD,
    WAITING_FOR_FOLLOW_UP,
    WAITING_FOR_SYNC,
}EthTSyn_Internal_SyncReceive_StateMachineType;

/** Main program data structure for ptp msgs */
typedef struct {
    /* Delay Request mechanism, version 1 and version 2 PTP */
    /* Announce, Sync and Follow Up sent related Variable */
    EthTSyn_Internal_SyncSend_StateMachineType syncSend_State;
    boolean sentSync;
    boolean txSyncTimePending;
    uint16 last_sync_tx_sequence_number;
    uint32  syncIntervalTimer;
    boolean syncIntervalTimerStarted;
    boolean syncIntervalTimeout;
    uint32  syncfollowUpTimer;
    boolean syncfollowUpTimerStarted;
    boolean syncfollowUpTimerTimeout;
    uint16  last_announce_tx_sequence_number;
    uint32  announceIntervalTimer;
    boolean announceIntervalTimerStarted;
    boolean announceIntervalTimeout;
    Eth_TimeStampType  t1SyncTxTime;

    /* Sync and FUP Receive related variable */
    EthTSyn_Internal_SyncReceive_StateMachineType syncReceive_State;
    boolean rcvdSync;
    boolean rcvdFollowUp;
    boolean receptionTimeout;
    uint16  parent_last_sync_sequence_number;
    sint64  syncCorrection;         /* Receive Sync correction time  */
    sint64  followupCorrection;     /* Receive Follow up correction time  */
    boolean waitingForFollow; /* Indicates two step Sync received, waiting for RX follow up */
    boolean RxFollowUpTimerStarted;
    uint32  RxFollowUpTimer;
    Eth_TimeStampType  t2SyncReceiptTime;     /* Time at slave of inbound SYNC message */
    Eth_TimeStampType  t2SyncReceiptTime_old;     /* Time at slave of inbound SYNC message */
    Eth_TimeStampType  t1SyncOriginTime;      /* Time from master from SYNC  */
    Eth_TimeStampType  t1SyncOriginTime_old;      /* Time from master from SYNC  */
    boolean            initSyncFollowReceived;

    /* Pdelay Req sent/Receive related variable */
    uint8   numPdelay_req;
    EthTSyn_Internal_Pdelay_StateMachineType pDelayReq_State;
    boolean sentPdelayReq;
    uint16  lostResponses;
    boolean isMeasuringDelay;
    boolean pdelayTimerStarted;
    boolean pdelayIntervalTimeout;
    uint32  pdelayIntervalTimer;
    boolean initPdelayRespReceived;
    uint16  last_pdelay_req_tx_sequence_number;
    boolean txPdelayReqTimePending;
    Eth_TimeStampType t1PdelayReqTxTime;     /* Time at requester of outbound PDELAY REQ */
    Eth_TimeStampType  t2PdelayReqRxTime[ETHTSYN_MAX_PDELAY_REQUEST];     /* Time from responder, reported in PDELAY RESP */

    /* Pdelay Resp and FUP sent related variable */
    EthTSyn_Internal_Pdelay_Resp_StateMachineType pDelayResp_State[ETHTSYN_MAX_PDELAY_REQUEST];
    boolean rcvdPdelayReq[ETHTSYN_MAX_PDELAY_REQUEST];
    boolean pdelayRespTimerStarted[ETHTSYN_MAX_PDELAY_REQUEST];
    boolean pdelayRespTimeout[ETHTSYN_MAX_PDELAY_REQUEST];
    uint32  pdelayRespIntervalTimer[ETHTSYN_MAX_PDELAY_REQUEST];
    boolean txPdelayRespTimePending[ETHTSYN_MAX_PDELAY_REQUEST];
    Eth_TimeStampType  t3PdelayRespTxTime[ETHTSYN_MAX_PDELAY_REQUEST];    /* Time from responder, reported in PDELAY RESP FOLLOWUP */
    uint32  pdelayRespfollowUpTimer[ETHTSYN_MAX_PDELAY_REQUEST];
    boolean pdelayRespfollowUpTimerStarted[ETHTSYN_MAX_PDELAY_REQUEST];
    boolean pdelayRespfollowUpTimerTimeout[ETHTSYN_MAX_PDELAY_REQUEST];

    /* Pdelay Resp and FUP receive related variable */
    boolean rcvdPdelayResp;
    boolean rcvdPdelayRespFollowUp;
    uint16  last_pdelay_resp_tx_sequence_number[ETHTSYN_MAX_PDELAY_REQUEST];
    uint16  last_pdelay_resp_follow_tx_sequence_number[ETHTSYN_MAX_PDELAY_REQUEST];
    PortIdentity requestingPortIdentity[ETHTSYN_MAX_PDELAY_REQUEST];
    Eth_TimeStampType  t2PdelayRespTime[ETHTSYN_MAX_PDELAY_REQUEST];      /* check! Time at responder of inbound of PDELAY REQ*/
    Eth_TimeStampType  t3PdelayRespTime;      /* Time from responder, reported in PDELAY RESP FOLLOWUP */
    Eth_TimeStampType  t3PdelayRespRxTime_old; /* Time from responder, reported in PDELAY RESP FOLLOWUP */
    Eth_TimeStampType  t4PdelayRespRxTime;     /* Time at requester */
    Eth_TimeStampType  t4PdelayRespRxTime_old; /* Time at requester */

    /* General Variables */
    sint64             peer_mean_path_delay;
    boolean            txTime_pending;         /* Added to poll for HW transmit time */
    uint8               msg_type;
    boolean             neighborRateRatioValid;
    sint64              neighborRateRatio;
    Eth_RateRatioType   rateratio;
    uint8               port_clock_identity[8];  /* V2 uses EUI-64 */
    uint8               port_uuid_field[PTP_UUID_LENGTH];

    boolean syncReceivedOnceFlag;
}EthTSyn_Internal_Ptp_Cfg;

typedef struct {
    EthTSyn_TransmissionModeType transmissionMode;    /* var to hold on/off of transmission */
    boolean trcvActiveStateFlag;
    EthTSyn_Internal_Ptp_Cfg  ptpCfgData;
}EthTSyn_Internal_DomainType;

typedef struct {
    EthTSyn_Internal_StateType initStatus;  /* variable to hold EthTSyn module status */
    EthTSyn_Internal_DomainType* timeDomain;
}EthTSyn_InternalType;

void EthTSyn_Internal_MsgPackSyncMsg(EthTsyn_Internal_MsgSync *bufPtr, uint8 timedomain);
void EthTSyn_Internal_MsgPackAnnounceMsg(EthTsyn_Internal_MsgAnnounce *bufPtr, uint8 timedomain);
void EthTSyn_Internal_MsgPackSyncFollow(EthTsyn_Internal_MsgFollowUp *bufPtr, uint8 timedomain);
void EthTSyn_Internal_MsgPackPdelayReqMsg(EthTsyn_Internal_MsgPDelayReq *bufPtr, uint8 timedomain);
void EthTSyn_Internal_MsgPackPdelayRespMsg(EthTsyn_Internal_MsgPDelayResp *bufPtr, uint8 timedomain, uint8 index);
void EthTSyn_Internal_MsgPackPdelayRespFollowUpMsg(EthTsyn_Internal_MsgPDelayRespFollowUp *bufPtr, uint8 timedomain, uint8 index);
void EthTSyn_Internal_UpdateTimer(uint8 timedomain);
uint64 convert_To_NanoSec(Eth_TimeStampType X);
uint64 convert_To_NanoSec_Stbm(StbM_TimeStampType X);
EthTSyn_Internal_TimeNanoType absolute(sint64 value);
#endif /* ETHTSYN_INTERNAL_H_ */
