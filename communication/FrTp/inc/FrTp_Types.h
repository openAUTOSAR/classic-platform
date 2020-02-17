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
#ifndef FRTP_TYPES_H_
#define FRTP_TYPES_H_

#include "ComStack_Types.h"

/* @req FRTP1028 */
typedef enum {
    FRTP_OFF = 0,
    FRTP_ON
} FrTp_StateType;

typedef enum {
    FRTP_NO,
    FRTP_ACK_WITH_RT
}FrTp_AckType;

typedef enum {
    NOT_ACTIVE = 0,
    ACTIVE_RX,
    ACTIVE_TX,
}FrtpCommType;



typedef enum {
    AVAILABLE = 0,
    LOCKED_FOR_FC,
    LOCKED_FOR_SF_CF_LF,
}FrtpTxPduLockType;


typedef enum{
    IDLE =0,
    BUSY,
    FRTP_STF_RECEIVED,
    FRTP_RX_WAIT_STF_SDU_BUFFER,
    FRTP_RX_WAIT_CF,
    FRTP_RX_WAIT_FC_SENDING,
    FRTP_EOB_RECEIVED_WAIT_SDU_BUFFER,
    RX_LAST_FRAME,
    FRTP_TX_STARTED,
    FRTP_TX_WAIT_FOR_FC,
    FRTP_TX_SEG_ONGOING,
    FRTP_TX_FINISHED,
    FRTP_WAIT_FOR_TRIGGER_TRANSMIT,
    FRTP_FRIF_ERROR,
}FrTpChnlStateType;

typedef enum {
    INVALID_FRAME, /* Not specified by ISO10681 - used as error return type when decoding frame. */
    START_FRAME,
    START_FRAME_ACK,
    CONSECUTIVE_FRAME_1,
    CONSECUTIVE_FRAME_2,
    CONSECUTIVE_FRAME_EOB,
    FLOW_CONTROL_FRAME,
    LAST_FRAME
} ISO10681FrameType;               /* Different types of Flexray Frames as per ISO10681-2 section 7.4 Table 8 */

typedef struct {
    uint16 Source_Address;
    uint16 Target_Address;
} ISO10681PduAddressType;


typedef struct {
   PduIdType FrTpTxPduId;
   PduIdType FrTpFrIfTxPduId;
   PduLengthType FrTpTxPduLength;
   boolean FrTpImmediateAccess;
}FrTp_TxPduPoolType;



typedef struct {
    PduIdType FrTpRxPduId;
    PduLengthType FrTpRxPduLength;
}FrTp_RxPduPoolType;


typedef struct {

     uint16 FrTpTimeoutCr;
     uint16 FrTpTimeoutBs;
     uint16 FrTpTimeoutAs;
     uint16 FrTpTimeoutAr;
     uint16 FrTpTimeFrIf;
     uint16 FrTpTimeBuffer;
     uint16 FrTpTimeBr;
     uint16 FrTpMaxBufferSize;
     uint8 FrTpSCexp;
     uint8 FrTpMaxRn;
     uint8 FrTpMaxNbrOfNPduPerCycle;
     uint8 FrTpMaxFrIf;
     uint8 FrTpMaxFCWait;
     uint8 FrTpMaxAs;
     uint8 FrTpMaxAr;
     FrTp_AckType FrTpAckType;
}FrTp_ConnectionControlType;


typedef struct {
    const FrTp_TxPduPoolType *FrTpTxPduPoolRef;
    const FrTp_RxPduPoolType *FrTpRxPduPoolRef;
    const FrTp_ConnectionControlType  *FrTpConCtrlRef;
    uint16  FrTpRa;
    uint16  FrTpLa;
    PduIdType FrTpRxSduId;                      /*Reference to a PDU in the global PDU structure.*/
    PduIdType FrTpTxSduId;                      /*Reference to a PDU in the global PDU structure.*/
    PduIdType FrTpULRxPduId;
    PduIdType FrTpULTxPduId;
    uint8 FrTpRxPduRefCount;
    uint8 FrTpPoolTxPduCnt;
    boolean FrTpBandwidthLimitation;
    boolean FrTpMultipleReceiverCon;
}FrTp_ConnectionConfigType;

/** Top level config container for FRTP implementation. */
typedef struct {
    const uint8 * FrTpPduIdToFrIfPduIdMap;
    const FrTp_ConnectionConfigType  *FrTpConnectionConfig;
    const PduIdType                   FrTpTxSduCount;
    const PduIdType                   FrTpRxSduCount;
    const PduIdType                   FrTpRxPducount;
    const PduIdType                   FrTpTxPduCount;
    uint8 FrTpConnectionCount;

}FrTp_ConfigType;



#endif /* FRTP_TYPES_H_ */
