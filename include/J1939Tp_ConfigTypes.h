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


#ifndef J1939TP_CONFIGTYPES_H_
#define J1939TP_CONFIGTYPES_H_

#include "ComStack_Types.h"

/**
 * @brief Protocol variant used when transferring data larger than 8 bytes.
 * @note  This information is stored inside a channel structure of type
 *        J1939Tp_ChannelType.
 */
typedef enum {
    J1939TP_PROTOCOL_BAM,  /**< Broadcast Announce Message */
    J1939TP_PROTOCOL_CMDT  /**< Connection Mode Data Transfer (peer-to-peer) */
} J1939Tp_ProtocolType;


/** @brief Transfer direction of a channel.
 *  @note  A channel can have only one direction: transmitting or receiving.
 *         This direction is stored in J1939Tp_ChannelType
 */
typedef enum {
    J1939TP_RX,  /**< Receiving    */
    J1939TP_TX   /**< Transmitting */
} J1939Tp_DirectionType;



/** @brief State of a transmission channel.
 *  @note  Used in J1939Tp_Internal_TxChannelInfoType.
 */
typedef enum {
    /** @brief Idle state. */
    J1939TP_TX_IDLE,

    /** @brief Waiting for CanIf to confirm the transmission
     *         of a BAM Connection Management PDU. */
    J1939TP_TX_WAIT_BAM_CANIF_CONFIRM,

    /** @brief Waiting for CanIf to confirm a sent RTS message. */
    J1939TP_TX_WAIT_RTS_CANIF_CONFIRM,

    /** @brief Waiting for Clear To Send command from receiver. */
    J1939TP_TX_WAITING_FOR_CTS,

    /** @brief Wait for CanIf to confirm sending a TP.DT packet
     *         which we send using CMDT. */
    J1939TP_TX_WAIT_DT_CANIF_CONFIRM,

    /** @brief Wait for CanIf to confirm sending a TP.DT packet
     *         which we send by means of BAM. */
    J1939TP_TX_WAIT_DT_BAM_CANIF_CONFIRM,

    /** @brief Waiting for the BAM interval to expire before we
     *         send another data packet.
     *  @note  When using BAM, data packets are sent in intervals
     *         of time with no other conection management */
    J1939TP_TX_WAITING_FOR_BAM_DT_SEND_TIMEOUT,

    /* Waiting for End of Message Aknowledge*/
    J1939TP_TX_WAITING_FOR_END_OF_MSG_ACK
} J1939Tp_Internal_TxChannelStateType;


/** @brief State of a reception channel. */
typedef enum {
    /** @brief Idle state. */
    J1939TP_RX_IDLE,

    /** @brief Waiting for CanIf to confirm the transmission of a CTS message. */
    J1939TP_RX_WAIT_CTS_CANIF_CONFIRM,

    /** @brief Receiving data. */
    J1939TP_RX_RECEIVING_DT,

    /** @brief Waiting for CanIf to confirm the transmission of a
     *        'End Of Message Aknowledge' message. */
    J1939TP_RX_WAIT_ENDOFMSGACK_CANIF_CONFIRM,
} J1939Tp_Internal_RxChannelStateType;

/** @brief Timer structure. */
typedef struct {
    /** @brief Timer counter */
    uint32 Timer;

    /** @brief Expire value.
     *
     *  The timer expires when Timer reaches a value => TimerExpire. */
    uint32 TimerExpire;
} J1939Tp_Internal_TimerType;


/** @brief Represents a Parameter Group Number. */
typedef uint32 J1939Tp_Internal_PgnType;

/** @brief Represents the size in bytes of a data payload. */
typedef uint32 J1939Tp_Internal_DtPayloadSizeType;

/** @see J1939Tp_ChannelType_ */
typedef struct J1939Tp_ChannelType_ J1939Tp_ChannelType;

/** @see J1939Tp_PgType_*/
typedef struct J1939Tp_PgType_      J1939Tp_PgType;

/** @brief Parameter Group Number */
typedef uint32                      J1939Tp_PgnType;

/**
 * @brief Represents a received (Rx) or transmitted (Tx) Parameter Group which
 *        was associated to a channel.
 *
 * The content of such a structure is setup with BSW Builder when allocating
 * PGs to the single channels during configuration of the J1939 module.
 */
struct J1939Tp_PgType_
{
    /** @brief   True when this PG has dynamic length.
     *  @details Dynamic length means that the Sdu data can have any length in
     *           range [0, 8]. This also means, that the Sdu has to be
     *           tranmitted or received as direct frame (with no connection
     *           management). */
    const boolean                   DynLength;

    /** @brief Parameter Group Number (PGN) of this Parameter Group.
     *  @note  The PGNs are listed in the J1939 specification. */
    const J1939Tp_Internal_PgnType  Pgn;

    /** @brief ID of the PDU frame used for a PG with payload size
     *         equal or less than 8 bytes.
     *  @note  This ID is used for calls to CanIf when transmitting PGs
     *         with data size equal less 8 bytes (direct frames).
     *  @note  Set only when DynLength is set to true. */
    const PduIdType                 DirectNPdu;

    /** @brief ID of the Service Data Unit (SDU) used for this parameter group.
     *  @note  This ID is used for calls to PduR functions. */
    const PduIdType                 NSdu;

    /** @brief Channel used to transfer this PG.
     *  @note  The channel also has a pointer to this PG structure. */
    const J1939Tp_ChannelType*      Channel;

    /** @brief Value of the parameter Metadatalength setup for the NSdu
     *         associated with this parameter group */
    const uint8 NSdu_MetaDataLength;
};

/**
 * @brief Description of a communication channel.
 *
 * A single channel is used to transfer PDUs containing the same sender and
 * receiver addresses.
 *
 * @note The contents of a channel structure are setup when configuring a Tx or
 *       Rx channel of the J1939Tp module with BSW Builder.
 * @note The PDUs are setup when configuring the EcuC module.
 */
struct J1939Tp_ChannelType_ {
    /** @brief  Protocol used by this channel (BAM or CMDT).
     * @details This is the protocol setup by the user and does not change at
     *          runtime.
     */
    const J1939Tp_ProtocolType              Protocol;

    /** @brief ID of the TP.DT frame used by BAM and CMDT to transfer
     *  the contents of an N-SDU. */
    const PduIdType                         DtNPdu;

    /** @brief ID of the Connection Management (TP.CM) PDU used
     *         with BAM and CMDT. */
    const PduIdType                         CmNPdu;

    /** @brief ID of the Flow Control PDU used for Abort, CTS, EndOfMsgAck
     *         messages (reverse connection management).
     *  @note: Only set when Protocol is J1939TP_PROTOCOL_CMDT.
     */
    const PduIdType                         FcNPdu;

    /** @brief Receive or transmit direction of this channel. */
    const J1939Tp_DirectionType             Direction;

    /** @brief Length of the array Pgs.
     *
     * How many PGs are transferred through this channel. */
    const uint16                            PgCount;

    /** @brief Array of pointers to Parameter Group structures.
     *
     * This is the list of PGs transferred through this channel, be it received
     * or transmitted. */
    const J1939Tp_PgType**                  Pgs;

    /*--------[Setup MetaDataLength]------------------------------------------*/

    /** @brief Value of the parameter MetaDataLength setup for the
     *         Connection Management Pdu */
    const uint8 CmNPdu_MetaDataLength;

    /** @brief Value of the parameter MetaDataLength setup for the
     *         Data NPdu */
    const uint8 DtNPdu_MetaDataLength;

    /** @brief Value of the parameter MetaDataLength setup for the
     *         Flow Control NPdu */
    const uint8 FcNPdu_MetaDataLength;
};


/**
 * @brief Types of PDUs.
 *
 * @note  This type is checked in J1939Tp_RxIndication and
 *        J1939Tp_TxConfirmation.
 */
typedef enum {
    /** @brief Reverse Connection Management.
     *
     *  Connection Management Messages in reverse direction are sent by the
     *  receiving note to the sending node. Indicates an Abort, CTS or
     *  EndOfMsgAck message (Flow Control messages).
     */
    J1939TP_REVERSE_CM,

    /** @brief  Connection Management frame (TP.CM).
     *
     * TP.CM is used by the BAM and CMDT protocols to initialize a connection
     * (BAM and RTS messages) or when the sender aborts transmission. */
    J1939TP_CM,

    /** @brief Data Transfer frame (TP.DT). */
    J1939TP_DT,

    /** @brief Direct transfer of PDUs with data (payload) less equal 8 bytes.
     *
     * For this kind of messages, there is no segmentation and no
     * connection management. They are forwarded to CanIf without further
     * logic. */
    J1939TP_DIRECT
} J1939Tp_RxPduType;


/** @brief Description of a transferred PDU from the point of view of the
 *         J1939 protocol.
 */
typedef struct {
    /** @brief Type of PDU.
     *  @see   J1939Tp_RxPduType */
    const J1939Tp_RxPduType     PacketType;

    /** @brief Channel number (index). */
    const uint8                 ChannelIndex;

    /** @brief Pointer to received Parameter Group structure. */
    const J1939Tp_PgType*       PgPtr;
} J1939Tp_RxPduInfoType;


/**
 * @brief Contains an array of J1939Tp_RxPduInfoType structures
 *        (PDU descriptions) and the corresponding array size.
 *
 * Every used PDU ID is associated with a structure of this type, that is with
 * a list of PDU descriptions.
 */
typedef struct {
    /** @brief Array of pointers to PDU descriptions. */
    const J1939Tp_RxPduInfoType** RxPdus;

    /** @brief Length of array RxPdus */
    const PduIdType               RxPduCount;
} J1939Tp_RxPduInfoRelationsType;


/** @req J1939TP0175 */
/**
 * @brief Stores the configuration of module J1939Tp.
 *
 * The array 'RxPduRelations' is meant to be indexed by PDU ID: to each PDU ID
 * being used in the module J1939 there is a list of PDU description structures,
 * where every structure contains information about a channel index, parameter
 * group transmitted and message type (connection management, data etc).
 * This is necessary because a certain PDU might be used on more than one
 * channel and with more than one role (for instance direct transmission and
 * segmented data transmission), depending on the user configuration.
 *
 * Here is an example of how to iterate over all channel objects associated with
 * a certain PDU ID:
 * @code
 *     PduIdType pduid              = ...;
 *     J1939Tp_ConfigType*  config  = ...;
 *
 *     J1939Tp_RxPduInfoRelationsType *pduList  = config->RxPduRelations[pduid];
 *     J1939Tp_RxPduInfoType          *pduInfo = 0;
 *     J1939Tp_ChannelType            *channel = 0;
 *     uint8                           channelIndex = 0;
 *     PduIdType i;
 *     for (i = 0; i < pduList->RxPduCount; i++)
 *     {
 *         pduInfo = pduList->RxPdus[i];
 *         channelIndex = pduInfo->ChannelIndex;
 *         channel = config -> Channels[channelIndex];
 *     }
 * @endcode
 */
typedef struct {
    const J1939Tp_RxPduInfoRelationsType* RxPduRelations;
    const J1939Tp_ChannelType*            Channels;
    const J1939Tp_PgType*                 Pgs;
} J1939Tp_ConfigType;

#endif
