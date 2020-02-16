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

#ifndef J1939TP_INTERNAL_H_
#define J1939TP_INTERNAL_H_
#include "J1939Tp_Internal_Packets.h"
#include "J1939Tp_ConfigTypes.h"
#include "ComStack_Types.h"
#define PGN_BYTE_COUNT          3
/** Service Ids */
#define J1939TP_TRANSMIT_ID     0x05
#define J1939TP_INIT_ID         0x01

/** Error Codes */
#define J1939TP_E_PARAM_ID      0x01
#define J1939TP_E_UNINIT        0x20
#define J1939TP_E_REINIT        0x21

#define J1939TP_PACKET_SIZE     7

/** @req J1939TP0019 */
/** @req SWS_J1939Tp_00019 */
typedef enum {
    J1939TP_ON,
    J1939TP_OFF
} J1939Tp_Internal_GlobalStateType;

/** @brief State of a timer of type J1939Tp_Internal_TimerType:
 *         expired or not expired */
typedef enum {
    J1939TP_EXPIRED,
    J1939TP_NOT_EXPIRED
} J1939Tp_Internal_TimerStatusType;

/**
 * @brief State description of a transmission channel.
 */
typedef struct {
    /** @brief Channel state (idle, waiting etc).
     *  @note  The type of this field is defined in J1939Tp_ConfigTypes.h. */
    J1939Tp_Internal_TxChannelStateType State;

    /** @brief Timer used to detect timeouts. */
    J1939Tp_Internal_TimerType          TimerInfo;

    /** @brief   How many data frames did we already sent?
     *  @details A message has been fully transmitted when
     *           SentDtCount * 'bytes per message' equals TotalMessageSize. */
    uint8                               SentDtCount;

    /** @brief   How many DT packets to send before waiting for another CTS
     *           message.
     *  @details A CTS message specifies how many packets (TP.DT frames) can the
     *           receiver node receive at a time. This is the number we store in
     *           this field. After sending that many packets, we wait for
     *           another CTS. */
    uint8                               DtToSendBeforeCtsCount;

    /** @brief Total message size in bytes. */
    J1939Tp_Internal_DtPayloadSizeType  TotalMessageSize;

    /** @brief   ID of the SDU transmitted through the channel.
     *  @details This ID points to some PDU inside the PduR module. */
    PduIdType                           PduRPdu;

    /** @brief Parameter Group transmitted through this channel. */
    const J1939Tp_PgType*               CurrentPgPtr;
} J1939Tp_Internal_TxChannelInfoType;



/**
 * @brief State description of a reception channel and related metadata
 *        like timer, number of received data frames and expected message size.
 */
typedef struct {
    /** @brief State of the reception channel.
     *  @note  The type of this field is defined in J1939Tp_ConfigTypes.h.
     */
    J1939Tp_Internal_RxChannelStateType State;

    /** @brief Timer used to detect timeouts. */
    J1939Tp_Internal_TimerType          TimerInfo;

    /** @brief Number of received Data Transfer (DT) messages.
     *  @details A message is fully received when ReceivedDtCount equals
     *           DtToReceiveCount.
     *  @note    The number of packets to receive is specified in the RTS message.
     */
    uint8                               ReceivedDtCount;

    /** @brief Number of expected Data Transfer (DT) messages */
    uint8                               DtToReceiveCount;

    /** @brief Total size of message to receive. */
    J1939Tp_Internal_DtPayloadSizeType  TotalMessageSize;

    /** @brief Parameter Group received through the channel. */
    const J1939Tp_PgType*               CurrentPgPtr;
} J1939Tp_Internal_RxChannelInfoType;

/**
 * @brief Runtime information about a communication channel.
 *
 * This structure stores the runtime information about a communication channel.
 *
 * This structure adds to a channel structure information about its transmission
 * or reception state, like number of packets to send resp. receive, total
 * message size and how many packets were received resp. transmitted.
 * In order to be able to store generic state information about a channel, two
 * state-structures were added: one for the transmission state and one for the
 * reception state. If this is transmission channel, its state is stored ar the
 * address pointed to by the field TxState, otherwise at the address pointed to
 * by RxState.
 *
 * @note In order to know if this is a transmission or reception channel, you
 *       inquire the field Direction of the channel pointer. Example: to read
 *       the total message size transmitted through this channel no matter if
 *       this is a transmission or reception channel:
 * @code
 *       J1939Tp_Internal_ChannelInfoType* channelInfo = ...;
 *       J1939Tp_Internal_DtPayloadSizeType messageSize = 0;
 *
 *       if (channelInfo->Direction == J1939TP_RX) {
 *           // reception channel
 *           messageSize = channelInfo->RxState->TotalMessageSize;
 *       } else if (channelInfo->Direction == J1939TP_TX) {
 *           // trasmission channel
 *           messageSize = channelInfo->TxState->TotalMessageSize;
 *       }
 * @endcode
 *
 */
typedef struct {
    /** @brief Setup channel information. */
    const J1939Tp_ChannelType*              ChannelConfPtr;

    /** @brief Channel state if this is a transmission channel.
     *  @note  setup in init */
    J1939Tp_Internal_TxChannelInfoType*     TxState;

    /** @brief Channel state if this is a reception channel.
     *  @note  setup in init */
    J1939Tp_Internal_RxChannelInfoType*     RxState;

    /*--------[Metadata support]--------------------------*/
    /* Each of the following fields store one entry in the metadata buffer.
     * The metadata, which is appended to the data of a PDU or SDU contains
     * parts of or the entire CAN Id used by the underlying CanIf.
     * These fields are changed at runtime based on the provided metadata.
     */

    /**
     * @brief   Dynamic protocol, setup at runtime.
     * @details For Sdus with metadata, J1939Tp inspects at runtime the
     *          destination address stored in the metadata and, based on this
     *          address, chooses the appropriate protocol version (BAM for 0xFF,
     *          CMDT otherwise). This is the dynamic protocol stored in this
     *          field. The initial value of this field is the same as that of
     *          field ChannelConfPtr->Protocol.
     */
     J1939Tp_ProtocolType                   Protocol;

     /** @brief Source address provided in the metadata. */
     uint8                                  SourceAddress;

     /** @brief Destination address provided in the metadata. */
     uint8                                  DestinationAddress;

     /** @brief Priority provided in the metadata. */
     uint8                                  Priority;

} J1939Tp_Internal_ChannelInfoType;

typedef enum {
    J1939TP_PG_TX_IDLE,
    J1939TP_PG_TX_WAIT_DIRECT_SEND_CANIF_CONFIRM,
} J1939Tp_Internal_PgDirectState;


/** @brief Metadata describing a transferred parameter group (PG).
 *
 * This structure adds more information to a J1939Tp_PgType structure
 * (Parameter Group description): on which channel is the PG transmitted resp.
 * received, transmission state and a timer used in case of direct transmission.
 */
typedef struct {
    /** @brief Channel used to transmit the PG. */
    J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr;

    /** @brief Transferred parameter group. */
    const J1939Tp_PgType*             PgConfPtr;

    /** @brief   State when direct sending.
     *  @details For direct transmission of PGs with at most 8 bytes of data, the transmission
     *           state is stored directly in the PG info and not in the enclosing channel.
     *  @note    Direct sending occurs when the PG data is less equal 8 bytes. */
    J1939Tp_Internal_PgDirectState    TxState;

    /** @brief   Timer structure used to monitor timeout of direct transmissions.
     *  @details When using direct transmission, we use this timer to monitor timeout.
     *           When using segmented transmission (BAM or CMDT) we use the timer in the
     *           transmission or reception state of the channel. */
    J1939Tp_Internal_TimerType        TimerInfo;

} J1939Tp_Internal_PgInfoType;


typedef struct {
    J1939Tp_Internal_GlobalStateType State;
} J1939Tp_Internal_GlobalStateInfoType;


typedef uint8 J1939Tp_Internal_ControlByteType;


static inline uint8 J1939Tp_Internal_GetPf(J1939Tp_PgnType pgn);
static inline Std_ReturnType J1939Tp_Internal_ValidatePacketType(const J1939Tp_RxPduInfoType* RxPduInfo);
static inline J1939Tp_Internal_ChannelInfoType* J1939Tp_Internal_GetChannelState(const J1939Tp_RxPduInfoType* RxPduInfo);
static inline const J1939Tp_ChannelType* J1939Tp_Internal_GetChannel(const J1939Tp_RxPduInfoType* RxPduInfo);
static inline void J1939Tp_Internal_RxIndication_Dt(PduInfoType* PduInfoPtr, J1939Tp_Internal_ChannelInfoType* Channel);
static inline void J1939Tp_Internal_RxIndication_Cm(PduInfoType* PduInfoPtr, J1939Tp_Internal_ChannelInfoType* Channel);
static inline void J1939Tp_Internal_RxIndication_ReverseCm(PduInfoType* PduInfoPtr, J1939Tp_Internal_ChannelInfoType* Channel);
static inline void J1939Tp_Internal_RxIndication_Direct(PduInfoType* PduInfoPtr, const J1939Tp_RxPduInfoType* RxPduInfoPtr);
Std_ReturnType J1939Tp_ChangeParameterRequest(PduIdType SduId, TPParameterType Parameter, uint16 value);
static inline void J1939Tp_Internal_TxConfirmation_RxChannel(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr, const J1939Tp_RxPduInfoType* RxPduInfo);
static inline boolean J1939Tp_Internal_IsDtPacketAlreadySent(uint8 nextPacket, uint8 totalPacketsSent);
static inline Std_ReturnType J1939Tp_Internal_GetRxPduRelationsInfo(PduIdType RxPdu,const J1939Tp_RxPduInfoRelationsType** RxPduInfo);

static inline Std_ReturnType J1939Tp_Internal_GetPgFromPgn(const J1939Tp_ChannelType* channel, J1939Tp_Internal_PgnType Pgn, const J1939Tp_PgType** Pg);
static inline boolean J1939Tp_Internal_IsLastDt(J1939Tp_Internal_RxChannelInfoType* rxPgInfo);
static inline void J1939Tp_Internal_TxConfirmation_TxChannel(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr,const J1939Tp_RxPduInfoType* RxPduInfo);
static inline boolean J1939Tp_Internal_IsLastDtBeforeNextCts(J1939Tp_Internal_RxChannelInfoType* rxChannelInfo);

static inline uint16 J1939Tp_Internal_GetRtsMessageSize(PduInfoType* pduInfo);
static inline boolean J1939Tp_Internal_WaitForCts(J1939Tp_Internal_TxChannelInfoType* TxChannelState);
static inline boolean J1939Tp_Internal_LastDtSent(J1939Tp_Internal_TxChannelInfoType* TxPgState);
static inline Std_ReturnType J1939Tp_Internal_ConfGetPg(PduIdType NSduId, const J1939Tp_PgType* Pg);
static inline J1939Tp_Internal_TimerStatusType J1939Tp_Internal_IncAndCheckTimer(J1939Tp_Internal_TimerType* TimerInfo);
static inline uint8 J1939Tp_Internal_GetDtDataSize(uint8 currentSeqNum, uint8 totalSize);

static inline Std_ReturnType J1939Tp_Internal_SendBam(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr,const PduInfoType* TxInfoPtr);
static inline Std_ReturnType J1939Tp_Internal_SendDt(J1939Tp_Internal_ChannelInfoType* Channel);
static inline Std_ReturnType J1939Tp_Internal_SendRts(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr, const PduInfoType* TxInfoPtr, uint8 MaxNumPackets);
static inline void J1939Tp_Internal_SendEndOfMsgAck(J1939Tp_Internal_ChannelInfoType* Channel);
static inline void J1939Tp_Internal_SendCts(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr, J1939Tp_PgnType Pgn, uint8 NextPacketSeqNum,uint8 NumPackets);
static inline void J1939Tp_Internal_SendConnectionAbort(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr, J1939Tp_RxPduType packetType, J1939Tp_PgnType Pgn, uint8 Reason);
static inline void J1939Tp_Internal_StartTimer(J1939Tp_Internal_TimerType* TimerInfo,uint16 TimerExpire);
static inline void J1939Tp_Internal_StopTimer(J1939Tp_Internal_TimerType* TimerInfo);
static inline void J1939Tp_Internal_SetPgn(uint8* PgnBytes,J1939Tp_PgnType pgn );
static inline J1939Tp_PgnType J1939Tp_Internal_GetPgn(uint8 PgnBytes[3]);
static inline Std_ReturnType J1939Tp_Internal_GetPg(PduIdType SduId, J1939Tp_Internal_PgInfoType** PgInfo);
static inline uint8 J1939TP_Internal_GetNumDtPacketsToSend(PduLengthType messageSize);
static inline Std_ReturnType J1939Tp_Internal_DirectTransmit(const PduInfoType* TxInfoPtr, J1939Tp_Internal_PgInfoType* PgInfo);
static inline J1939Tp_Internal_PgInfoType* J1939Tp_GetPgInfo(const J1939Tp_PgType* Pg);

static inline void J1939Tp_Internal_ReportError(uint8 ApiId, uint8 ErrorId);

static inline void J1939Tp_Internal_Reset_Channel(J1939Tp_Internal_ChannelInfoType * channel);


static inline PduIdType J1939Tp_Internal_Get_Pdu(const J1939Tp_ChannelType* ChannelConfPtr, J1939Tp_RxPduType PacketType);

/*------------[Functions related to metadata support]---------------------------------------------*/
static inline void J1939Tp_Internal_Reset_Channel_Metadata(J1939Tp_Internal_ChannelInfoType* channelInfo);
static void J1939Tp_Internal_Set_Channel_Metadata(J1939Tp_Internal_ChannelInfoType* channelInfo, uint8 metadata[], uint8 metadatalength);
static inline void J1939Tp_Internal_Copy_Channel_Metadata(J1939Tp_Internal_ChannelInfoType* channelInfo, uint8 metadata[METADATALENGTH_MAX], uint8 metadatalength);
static Std_ReturnType J1939Tp_Internal_Filter_RxPdu_Metadata
(PduIdType RxPduId, PduInfoType* PduInfoPtr, J1939Tp_RxPduType packetType, J1939Tp_Internal_ChannelInfoType* channel);
static inline uint8 J1939Tp_Internal_Get_Pdu_MetaDataLength(const J1939Tp_ChannelType* ChannelConfPtr, J1939Tp_RxPduType packetType);
static boolean J1939Tp_Internal_Metadata_Matches_Channel(uint8 metadata[], uint8 metadatalength, J1939Tp_RxPduType packetType, J1939Tp_Internal_ChannelInfoType* channel);

#endif
