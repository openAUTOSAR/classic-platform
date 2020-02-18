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

/*
 * NB! This file is for DOIP internal use only and may only be included from DOIP C-files!
 */
#ifndef DOIP_INTERNAL_H_
#define DOIP_INTERNAL_H_

#include "DoIP_Cfg.h"

/* Buffer Size */
#define UDP_RX_BUFF_SIZE                80
#define UDP_TX_BUFF_SIZE                40
#define TCP_TX_BUFF_SIZE                100u
#define TCP_RX_BUFF_SIZE                0x4010// 16 k buffer for reception of tcp message
#define TX_QUEUE_DEPTH                  10

#define PROTOCOL_VERSION                0x02u
#define PROTOCOL_VERSION_VID            0xFFu

/* Payload types */
#define PL_TYPE_GENERIC_N_ACK           0x0000u
#define PL_TYPE_VID_RES                 0x0004u
#define PL_TYPE_ROUT_ACTIV_RES          0x0006u
#define PL_TYPE_ALIVE_CHK_REQ           0x0007u
#define PL_TYPE_ENT_STATUS_RES          0x4002u
#define PL_TYPE_POWER_MODE_RES          0x4004u
#define PL_TYPE_DIAG_MSG_P_ACK          0x8002u
#define PL_TYPE_DIAG_MSG_N_ACK          0x8003u
#define PL_TYPE_VID_REQ                 0x0001u
#define PL_TYPE_VID_EID_REQ             0x0002u
#define PL_TYPE_VID_VIN_REQ             0x0003u
#define PL_TYPE_ROUT_ACTIV_REQ          0x0005u
#define PL_TYPE_ALIVE_CHK_RES           0x0008u
#define PL_TYPE_ENT_STATUS_REQ          0x4001u
#define PL_TYPE_POWER_MODE_REQ          0x4003u
#define PL_TYPE_DIAG_MSG                0x8001u

/* Payload lengths */
#define PL_LEN_VID_REQ                  0u  
#define PL_LEN_VID_EID_REQ_RES          6u  
#define PL_LEN_GID                      6u  
#define PL_LEN_VID_VIN_REQ_RES          17u 
#define PL_LEN_ROUT_ACTIV_REQ           7u  
#define PL_LEN_ROUT_ACTIV_OEM_REQ       11u 
#define PL_LEN_DIAG_MIN_REQ             5u  
#define PL_LEN_ALIVE_CHK_RES            2u  
#define PL_LEN_ENT_STATUS_REQ           0u  
#define PL_LEN_POWER_MODE_REQ           0u  
#define PL_LEN_GENERIC_N_ACK            1u
#define PL_LEN_VID_RES                  32u
#define PL_LEN_ROUT_ACTIV_RES           9u
#define PL_LEN_ALIVE_CHK_REQ            0u
#if(DOIP_ENTITY_MAX_BYTE_USE == STD_ON) 
#define PL_LEN_ENT_STATUS_RES           7u
#else
#define PL_LEN_ENT_STATUS_RES           3u
#endif
#define PL_LEN_POWER_MODE_RES           1u
#define PL_LEN_DIAG_MSG_ACK             5u
#define MSG_LEN_INCL_PL_LEN_FIELD       8u

/* Index of the DoIP message fields */
#define PL_TYPE_INDEX                   2u
#define PL_LEN_INDEX                    4u
#define SA_INDEX                        8u
#define TA_INDEX                        10u
#define ROUT_ACTIV_TYPE_INDEX           10u
#define SA_AND_TA_LEN                   4u
#define REQ_PAYLOAD_INDEX               8u

#define VID_VIN_INDEX                   8u
#define VID_LA_INDEX                    25u
#define VID_EID_INDEX                   27u
#define VID_GID_INDEX                   33u
#define VID_FUR_ACT_REQ_INDEX           39u
#define VID_VIN_GID_STS_INDEX           40u

#define SHIFT_BY_ONE_BYTE               8u
#define SHIFT_BY_TW0_BYTES              16u
#define SHIFT_BY_THREE_BYTES            24u  
#define SHIFT_BY_FOUR_BYTES             32u 
#define SHIFT_BY_FIVE_BYTES             40u 

/* Diagnostic message negative acknowledge codes */
/* 0x00 and 0x01  Reserved by document */
#define ERROR_DIAG_INVALID_SA           0x02u    /* Invalid Source Address */
#define ERROR_DIAG_UNKNOWN_TA           0x03u    /* Unknown Target Address */
#define ERROR_DIAG_MESSAGE_TO_LARGE     0x04u    /* Diagnostic Message too large */
#define ERROR_DIAG_OUT_OF_MEMORY        0x05u    /* Out of memory */
#define ERROR_DIAG_TARGET_UNREACHABLE   0x06u    /* Target unreachable */
#define ERROR_DIAG_UNKNOWN_NETWORK      0x07u    /* Unknown network */
#define ERROR_DIAG_TP_ERROR             0x08u    /* Transport protocol error */

/* Generic DoIP header negative acknowledge codes */
/** @req SWS_DoIP_00014 */
#define ERROR_INCORRECT_PATTERN_FORMAT  0x00u
/** @req SWS_DoIP_00016 */
#define ERROR_UNKNOWN_PAYLOAD_TYPE      0x01u
/** @req SWS_DoIP_00017 */
#define ERROR_MESSAGE_TOO_LARGE         0x02u
/** @req SWS_DoIP_00018 */
#define ERROR_OUT_OF_MEMORY             0x03u
/** @req SWS_DoIP_00019 */
#define ERROR_INVALID_PAYLOAD_LENGTH    0x04u

#define INVALID_PDU_ID                  DOIP_INVALID_PDU_ID
#define INVALID_SOCKET_NUMBER           0xFFFFu
#define INVALID_SOURCE_ADDRESS          0xFFu
#define INVALID_CHANNEL_INDEX           0xFFu
#define INVALID_CONNECTION_INDEX        0xFFFFu
#define INVALID_ROUTING_ACTIV_INDEX     0xFFFFu
#define INVALID_PENDING_ROUT_ACTIV      0xFFFFu
#define UDP_RX_BUFFER_RESET_INDEX       0u

#define DOIP_PORT_NUM                   13400uL
typedef enum {
    ID_REQUEST_ALL,
    ID_REQUEST_BY_EID,
    ID_REQUEST_BY_VIN,
} DoIP_Internal_VehReqType;

typedef enum {
    TCP_TYPE,
    TCP_FORCE_CLOSE_TYPE,
    UDP_TYPE
} DoIP_Internal_ConType;

typedef enum {
    SOCKET_ASSIGNMENT_FAILED,
    SOCKET_ASSIGNMENT_SUCCESSFUL,
    SOCKET_ASSIGNMENT_PENDING,
} DoIP_Internal_SockAssigResType;

typedef enum {
    CONNECTION_INVALID,
    CONNECTION_INITIALIZED,
    CONNECTION_REGISTERED,
} DoIP_Internal_SocketStateType;

typedef enum {
    BUFFER_IDLE,
    BUFFER_LOCK_START,
    BUFFER_LOCK,
} DoIP_Internal_BufferStateType;

typedef enum {
    ACTIVATION_LINE_INACTIVE,
    ACTIVATION_LINE_ACTIVE,
} DoIP_Internal_ActnLineStsType;

typedef enum {
    LOOKUP_SA_TA_OK,
    LOOKUP_SA_TA_TAUNKNOWN,
    LOOKUP_SA_TA_SAERR,
    LOOKUP_SA_TA_ROUTING_ERR,
} DoIP_Internal_LookupResType;

typedef enum {
    DOIP_UNINIT,
    DOIP_INIT
} DoIP_Internal_StatusType;

/** @req SWS_DoIP_00002 */  /** @req SWS_DoIP_00142 */
typedef struct {
    uint32                          initialInactivityTimer;
    uint32                          generalInactivityTimer;
    uint32                          aliveCheckTimer;
    uint32                          txBytesToTransmit;
    uint32                          txBytesCopied;
    uint32                          txBytesTransmitted;
    PduIdType                       txPduIdUnderProgress;
    SoAd_SoConIdType                sockNr;
    uint16                          sa;
    uint16                           activationType;
    DoIP_Internal_BufferStateType   txBufferState;
    DoIP_Internal_SocketStateType   socketState;
    uint8                           txBuffer[TCP_TX_BUFF_SIZE];
    uint8                           channelIndex;
    boolean                         authenticated;
    boolean                         confirmed;
    boolean                         awaitingAliveCheckResponse;
    boolean                         closeSocketIndication;
    boolean                         uLMsgTxInProgress;
} DoIP_Internal_TcpConAdminType;

typedef struct {
    uint8                           buffer[TCP_RX_BUFF_SIZE];
    DoIP_Internal_BufferStateType   bufferState;
    PduIdType                       pduIdUnderProgress;    
} DoIP_Internal_TcpConRxBufAdType;

typedef struct {
    uint8                          *SduDataPtr;
    PduLengthType                   SduLength;
    PduIdType                       txPduId;
    uint16                          sa;                  
    uint16                          ta;
    boolean                         diagAckQueueActive;
    boolean                         tpTransmitQueueActive;
} DoIP_Internal_TcpQueueAdminType;


/** @req SWS_DoIP_00001 */
typedef struct {
    SoAd_SoConIdType                sockNr;
    PduIdType                       rxPduIdUnderProgress;
    DoIP_Internal_SocketStateType   socketState;
    uint32                          vehicleAnnounceInitialTime;
    uint32                          vehicleAnnounceTimeInterval;
    DoIP_Internal_BufferStateType   txBufferState;
    uint8                           vehicleAnnounceRepetition;
    uint8                           txBuffer[UDP_TX_BUFF_SIZE];
    uint8                           rxBuffer[DOIP_MAX_UDP_REQUEST_MESSAGE][UDP_RX_BUFF_SIZE];
    uint8                           rxBufferStartIndex;
    uint8                           rxBufferEndIndex;
    boolean                         vehicleAnnounceTgr;
    boolean                         vehicleAnnounceInProgress;
    boolean                         rxBufferPresent;
} DoIP_Internal_UdpConAdminType;

#endif /* DOIP_INTERNAL_H_ */
