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
#ifndef FRTP_INTERNAL_TYPES_H_
#define FRTP_INTERNAL_TYPES_H_


#include "FrTp_Types.h"
#include "FrTp_Cfg.h"

#define FLOW_CONTROL_FRAME_PAYLOAD    8u
#define FR_PDU_LENGTH                 254u  /* Flexray Frame length */
#define SHIFT_1_BYTE                  8u
#define SHIFT_3_BITS                  3u
#define ISO10681_TPCI_MASK            0xF0u
#define ISO10681_TPCI_STF             0x40u
#define ISO10681_TPCI_STFA            0x41u
#define ISO10681_TPCI_CF1             0x50u
#define ISO10681_TPCI_CF2             0x60u
#define ISO10681_TPCI_CFEOB           0x70u
#define ISO10681_TPCI_FC              0x80u
#define ISO10681_TPCI_LF              0x90u
#define ISO10681_TPCI_FS_MASK         0x0Fu /* Flow control status mask */
#define ISO10681_FC_STATUS_CTS        0x03u
#define ISO10681_FC_STATUS_ACK_RET    0x04u
#define ISO10681_FC_STATUS_WAIT       0x05u
#define ISO10681_FC_STATUS_ABORT      0x06u
#define ISO10681_FC_STATUS_OVFL       0x07u
#define CF_LAST_INDEX                 0xfu

#define STF_ACK_SET                   1u
#define TARGET_ADDR_MSB               0u
#define TARGET_ADDR_LSB               1u
#define SOURCE_ADDR_MSB               2u
#define SOURCE_ADDR_LSB               3u
#define PCI_BYTE_1                    4u
#define PCI_BYTE_2                    5u
#define PCI_BYTE_3                    6u
#define PCI_BYTE_4                    7u
#define START_OF_DATA_BYTE            8u
#define FPL_BYTE                      PCI_BYTE_2
#define MSG_LNGTH_MSB                 PCI_BYTE_3
#define MSG_LNGTH_LSB                 PCI_BYTE_4
#define UKNOWN_MSG_LENGHT             0u
#define MAX_STF_PAYLOAD               246u /* 254 - 4 byte address info - 4 byte PCI*/
#define MSB_BYTE_MASK                 0xFF00u
#define LSB_BYTE_MASK                 0x00FFu
#define START_OF_DATA_BYTE_CF         6u
#define START_OF_DATA_BYTE_STF_LF     8u

#define INVALID_CONNECTION            0xFF

#define FRTP_PDUARRAY_SIZE  254
#define FRTP_ONE    1
#define FRTP_MAX_USEG_UL_FPL 246  /*max FPL from upper layer not included C_PCI and C_AI 8 Bytes */
#define FRTP_MAX_SERIALNUMBER   0x0Fu
#define FRTP_SN_RANGE_MODULO    0x10u


#define FRTP_HEADER_LENGTH_STF_LF 8u
#define FRTP_HEADER_LENGTH_CF     6u
#define FRTP_CPCI_STFA            0x41u /* Start Frame with acknowledge */
#define FRTP_CPCI_STFU            0x40u /* Start Frame - Normal */


#define FRTP_MSB_U16(x) (uint8)((x)>>8u)
#define FRTP_LSB_U16(x) (uint8)((x) & 0x00FFu)

typedef struct{
    const FrTp_ConnectionConfigType  *FrTpTxConRef;
    const FrTp_TxPduPoolType    *FrTpTxConTxPduPoolRuntimeRef;
    FrTpChnlStateType           FrTpTxPrevChannelState;   /* holds the Previous state of TX channel */
    uint16                      FrTpTxConTxPduPendingCounter;        /* @req FRTP1090*/
    PduLengthType               FrTpMessageLength;
    PduLengthType               FrTpCompletedMessageLength;
    PduLengthType               FrTpRemainingTxBufferLength;
    uint16                      FrTpCounterAs; /* As */
    uint16                      FrTpAsTimeoutRetryCounter;
    uint16                      FrTpCounterBs; /* Bs */
    uint16                      FrTpCounterCs; /* Cs */
#if FRTP_ACK_RETRY_SUPPORT == STD_ON
    uint16                      FrTpTxBlockPosition; /* Block position in case a retry is recived as FC status */
#endif
    uint8                       FrTpFrIfTxErrNextTryTimer;
    uint8                       FrTpTxCounterFrIf;
    uint8                       FrTpSerialNumber;
    uint8                       FrTpTxFcWt;
    uint8                       FrTpAssignedConnectionIndex;
    uint8                       FrTpTxDataPdu[FR_PDU_LENGTH];
}FrTpTxType;

typedef struct{
    const FrTp_ConnectionConfigType  *FrTpRxConRef;
    const FrTp_TxPduPoolType         *FrTpRxConTxPduPoolRuntimeRef;
    FrTpChnlStateType                 FrTpRxPrevChannelState;   /* holds the Previous state of RX channel*/
    uint16                            FrTpRxConTxPduPendingCounter;
    PduLengthType                     FrTpRxMessageLength;   /* This is the ML*/
    PduLengthType                     FrTpCompletedRxMessageLength;
    PduLengthType                     FrTpRemainingRxBufferLength;
    uint16                            FrTpCounterAr; /* Ar  */
    uint16                            FrTpArTimeoutRetryCounter;
    uint16                      FrTpCounterBr; /* Br  */
    uint16                      FrTpCounterCr; /* Cr */
#if FRTP_ACK_RETRY_SUPPORT == STD_ON
    uint16                      FrTpRxBlockPosition;
#endif
    uint8                       FrTpFrIfRxErrNextTryTimer;
    uint8                       FrTpRxCounterFrIf;
    uint8                       FrTPRxSquenceNumber;
    uint8                       FrTpRxFcWt;
    uint8                       FrTpAssignedRxConnectionIndex;
    uint8                       FrTpRxDataPdu[FR_PDU_LENGTH];     /*local Rx buffer*/
}FrTpRxType;

#if(FRTP_FULL_DUPLEX_ENABLE == STD_OFF)
    typedef union
    {
        FrTpTxType               FrTpTxChannel;
        FrTpRxType               FrTpRxChannel;
    } FrTpChannelType;
#else
    typedef struct
    {
        FrTpTxType               FrTpTxChannel;
        FrTpRxType               FrTpRxChannel;
    } FrTpChannelType;
#endif


typedef struct{
   /* Need to Add description for all the parameters  */
    FrTpChannelType             FrTpChannelParams;
    FrTpChnlStateType           FrTpTxChannelState;
    FrTpChnlStateType           FrTpRxChannelState;   /* holds the current processing state of RX channel*/
    RetryInfoType               FrTpRetryInfo;
    BufReq_ReturnType           FrTpUlReturnValue;    /* holds the upper layer return value*/
    PduLengthType               FrTpTransmitPduLength; /* for keep track of trigger transmit */
    PduLengthType               FrTpRxSizeBuffer;    /* This value is provided by the upper layer*/
    PduLengthType               FrTpFcRecveBuffSize;    /* This value is provided by the Receiver via FC When we r transmitting */
    uint8                       FrTpFramePayloadLength;
    uint8                       FrTpCounterRxRn;
    uint8                       FrTpChannelNumber;
    uint8                       FrTpBandwidthcontol;
    uint8                       FrTpRxFcDataPdu[FLOW_CONTROL_FRAME_PAYLOAD];
    boolean                     FrTpTxSduUnknownMsgLength; /* @req FRTP1101 */ /* FRTP1102 */
    boolean                     FrTpRxPduAvailable;      /* RX Pdu Available flag */
    boolean                     FrTpTxSduAvailable; /* @req FRTP415  */
    boolean                     FrTpTcRequest;
#if FRTP_ACK_RETRY_SUPPORT == STD_ON
    FrTp_AckType                FrTpAckPending;      /* Flag to identify ack is received */
    boolean                     FrTpuseCF1;
    boolean                     FrTpStfAckFlag;         /* flag indicates if STF was recieved with Ack */
    boolean                     FrTpRxFcRetryFlag;      /* Set this flag when sending FC with Retry Flag Set*/
    boolean                     FrTpStfReceivedFlag;
    boolean                     FrTpEobReceivedFlag;
#endif

}FrTp_Internal_Channel_type;


typedef struct{

    FrtpTxPduLockType FrTpTxPduLockStatus;
    uint8 FrTpChannleHandle;
}FrTp_TxPduStatusInfoType;



typedef struct {
    FrTp_StateType FrTpState; /* @req FRTP1029*/
    FrtpCommType FrTpConnectionLockStatus[FRTP_CONFIGURED_CONNECTION_CNT];
    FrTp_TxPduStatusInfoType FrTpTxPduStatusInfo[FRTP_MAX_TXPDU_COUNT]; /* this show the tx pdu locked by a channel for transmission*/
    FrTp_Internal_Channel_type FrTpChannels[FRTP_CHANNEL_NUM]; /* @req FRTP088*/ /* @req FRTP228*/
}FrTp_Internal_RunTimeType;

#ifdef HOST_TEST
FrTp_Internal_RunTimeType* readInternal_ChnlStatus(void);
#endif

#endif /* FRTP_INTERNAL_TYPES_H_ */
