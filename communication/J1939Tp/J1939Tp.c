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

/** @req J1939TP0003 **/
#include "J1939Tp.h"
/** @req J1939TP0172 */
#include "CanIf.h"
/** @req J1939TP0013 */
#include "J1939Tp_Cbk.h"
#include "J1939Tp_Internal.h"
#include "PduR.h"
/** @req J1939TP0015 */
#include "PduR_J1939Tp.h"
/** @req J1939TP0193 */
/** @req SWS_J1939Tp_00193 */
#if (defined(USE_DET))
#include "Det.h"
#endif

/* Globally fulfilled requirements */
/** @req J1939TP0123 */
/** @req J1939TP0165 */
/** @req J1939TP0097 */
/** @req J1939TP0019 */
/** @req J1939TP0184 */
/** @req J1939TP0007 */
/** @req J1939TP0152 */
/** @req J1939TP0018 */
/** @req J1939TP0036 */
/** @req J1939TP0155 */
/** @req J1939TP0152 */
/** @req J1939TP0125 */
/** @req J1939TP0174 */
/** @req J1939TP0041 */
/** @req J1939TP0189 */
/** @req J1939TP0192 */
/** @req J1939TP0156 */

/** @req SWS_J1939Tp_00121 */
/** @req SWS_J1939Tp_00226 */
/** @req SWS_J1939Tp_00035 */

/** @req J1939TP0020 */

/** @brief State of this module.
 *  @details The state is set to J1939TP_ON when the J1939Tp_Init function is
 *           called and to J1939TP_OFF when the function J1939Tp_Shutdown is
 *           called. */
static J1939Tp_Internal_GlobalStateInfoType globalState = {
        .State = J1939TP_OFF,
};


/*==================[internal data]===============================================================*/

/* Module Configuration. The content of the pointed struct is being generated. */
static const J1939Tp_ConfigType*          J1939Tp_ConfigPtr = NULL_PTR;
/* Runtime channel information */
static J1939Tp_Internal_ChannelInfoType   channelInfos   [J1939TP_CHANNEL_COUNT];
/* Transmission states of the transmission channels. */
static J1939Tp_Internal_TxChannelInfoType txChannelInfos [J1939TP_TX_CHANNEL_COUNT];
/* Reception states of the reception channels. */
static J1939Tp_Internal_RxChannelInfoType rxChannelInfos [J1939TP_RX_CHANNEL_COUNT];
/* Array of PG descriptions indexed by SDU ID. */
static J1939Tp_Internal_PgInfoType        pgInfos        [J1939TP_PG_COUNT];

/** Block size declared in CTS and RTS messages.
 *
 * To support to function J1939Tp_ChangeParameter, which is supposed to change the parameter
 * J1939TpPacketsPerBlock, we define this internal variable and
 * initialize it with the value of J1939TP_PACKETS_PER_BLOCK.
 * @see J1939Tp_ChangeParameter */
#if defined (J1939TP_PACKETS_PER_BLOCK)
static uint16 Internal_J1939TpPacketsPerBlock = J1939TP_PACKETS_PER_BLOCK;
#else
/* The configuration parameter J1939TpPacketsPerBlock has the default value 16 */
static uint16 Internal_J1939TpPacketsPerBlock = 16;
#endif

/*==================[functions]===================================================================*/

/*------------------[Initialize the J1939Tp module]-----------------------------------------------*/
/** @req J1939TP0087 */
void J1939Tp_Init(const J1939Tp_ConfigType* ConfigPtr) {
    #if (J1939TP_DEV_ERROR_DETECT == STD_ON)
    if (globalState.State == J1939TP_ON) {
        /** @req J1939TP0026 */
        J1939Tp_Internal_ReportError(J1939TP_INIT_ID, J1939TP_E_REINIT);
    }
    #endif
    J1939Tp_ConfigPtr = ConfigPtr;
    int rxCount = 0;
    int txCount = 0;
    for (int i = 0; i < J1939TP_CHANNEL_COUNT; i++)
    {
        if (ConfigPtr->Channels[i].Direction == J1939TP_TX)
        {
            /* channelInfos[i] points to a channel configuration */
            channelInfos[i].ChannelConfPtr = &(ConfigPtr->Channels[i]);
            channelInfos[i].TxState        = &(txChannelInfos[txCount]);
            channelInfos[i].TxState->State = J1939TP_TX_IDLE;
            channelInfos[i].RxState        = 0;
            txCount++;
        }
        else if (ConfigPtr->Channels[i].Direction == J1939TP_RX)
        {
            channelInfos[i].ChannelConfPtr = &(ConfigPtr->Channels[i]);
            channelInfos[i].TxState = 0;
            channelInfos[i].RxState = &(rxChannelInfos[rxCount]);
            channelInfos[i].RxState->State = J1939TP_RX_IDLE;
            rxCount++;
        }
        else
        {
            return; // unexpected
        }

        /* Reset metadata information */
        J1939Tp_Internal_Reset_Channel_Metadata(&(channelInfos[i]));
    }

    for (int i = 0; i < J1939TP_PG_COUNT; i++) {
        pgInfos[i].PgConfPtr = &(ConfigPtr->Pgs[i]);
        uint8 channelIndex   = ConfigPtr->Pgs[i].Channel - ConfigPtr->Channels;
        pgInfos[i].ChannelInfoPtr = &(channelInfos[channelIndex]);
        pgInfos[i].TxState = J1939TP_PG_TX_IDLE;
    }

    /* Reinitialize packets per block in case someone decides for some
     * reason to reinitialize the module. */
#if defined (J1939TP_PACKETS_PER_BLOCK)
    Internal_J1939TpPacketsPerBlock = J1939TP_PACKETS_PER_BLOCK;
#else
    /* The configuration parameter J1939TpPacketsPerBlock has the
     * default value 16 */
    Internal_J1939TpPacketsPerBlock = 16;
#endif

    globalState.State = J1939TP_ON; /** @req J1939TP0022 */
}
/*------------------------------------------------------------------------------------------------*/

/*------------------[Shutdown the J1939Tp module]-------------------------------------------------*/

/** @req SWS_J1939Tp_00093 */
/**
 * @brief Shutdown the J1939Tp module.
 *
 * The shutdown function puts all communication channels into idle state and stops all timers.
 * It does not reset any internal pointers.
 *
 *
 * @see Items SWS_J1939Tp_00093, SWS_J1939Tp_00094 and SWS_J1939Tp_00094
 *      of the Autosar specification of the J1939Tp module, R4.1 Rev 1.
 */
void J1939Tp_Shutdown(void)
{
    int rxCount = 0;
    int txCount = 0;

    /** @req SWS_J1939Tp_00094 */

    /* Put every channel into idle state and stop it's timer. */
    for (int i = 0; i < J1939TP_CHANNEL_COUNT; i++)
    {
        J1939Tp_Internal_Reset_Channel(&(channelInfos[i]));

        switch (channelInfos[i].ChannelConfPtr->Direction)
        {
            case J1939TP_TX:
                txCount++;
                break;
            case J1939TP_RX:
                rxCount++;
                break;
            default:
                break;
        }
    }

    /* Put the PGs in idle state and stop their timers */
    for (int i = 0; i < J1939TP_PG_COUNT; i++)
    {
        pgInfos[i].TxState = J1939TP_PG_TX_IDLE;
        J1939Tp_Internal_StopTimer(&(pgInfos[i].TimerInfo));
    }

    /* Shutdown */
    globalState.State = J1939TP_OFF;

    /** @req SWS_J1939Tp_00095 */
}
/*------------------------------------------------------------------------------------------------*/

/**
 * Callback function used by the underlying layer (CanIf) to inform J1939Tp
 * when a Pdu has been received.
 *
 * @param    RxPduId        ID of the received I-PDU.
 *                          This may be the PduId of a CM, FC, DT or direct
 *                          frame. Note: this number should not be an Sdu Id.
 * @param    PduInfoPtr     Contains the length (SduLength) of the received
 *                          I-PDU and a pointer to a buffer (SduDataPtr)
 *                          containing the I-PDU.
 * @returns  none
 * @note     The argument RxPduId should be a PduId in the J1939Tp namespace.
 */
void J1939Tp_RxIndication(PduIdType RxPduId, PduInfoType* PduInfoPtr)
{
    Std_ReturnType metadata_match = E_NOT_OK;

    if (globalState.State == J1939TP_ON)
    {
        const J1939Tp_RxPduInfoRelationsType* RxPduRelationsInfo = NULL_PTR;

        /* Get the usages of this PDU */
        Std_ReturnType status =
        J1939Tp_Internal_GetRxPduRelationsInfo(RxPduId, &RxPduRelationsInfo);

        if (status == E_OK)
        {
            /* For all usages (relations to some channel) of this PduId check
             * the usage type and handle accordingly. */
            for (PduIdType i = 0; i < RxPduRelationsInfo->RxPduCount; i++)
            {
                imask_t state;
                Irq_Save(state);

                const J1939Tp_RxPduInfoType*
                    RxPduInfo = RxPduRelationsInfo->RxPdus[i];

                J1939Tp_Internal_ChannelInfoType*
                    ChannelInfoPtr = J1939Tp_Internal_GetChannelState(RxPduInfo);

                /* Use eventual metadata to filter incoming PDUs. For BAM and
                 * RTS messages the channel's addressing information is setup,
                 * otherwise, if there is any metadata, it is matched against
                 * the current channel addressing information. Non matching PDUs
                 * are ignored. */
                metadata_match =
                    J1939Tp_Internal_Filter_RxPdu_Metadata
                    (RxPduId, PduInfoPtr, RxPduInfo->PacketType, ChannelInfoPtr);

                if (metadata_match == E_OK)
                {
                    switch (RxPduInfo->PacketType)
                    {
                        /* CTS, Connection Abort, End of Msg Ack */
                        case J1939TP_REVERSE_CM:
                            J1939Tp_Internal_RxIndication_ReverseCm
                                (PduInfoPtr, ChannelInfoPtr);
                            break;
                        /* data transmission*/
                        case J1939TP_DT:
                            J1939Tp_Internal_RxIndication_Dt
                                (PduInfoPtr, ChannelInfoPtr);
                            break;
                        /* BAM, RTS, Connection Abort */
                        case J1939TP_CM:
                            J1939Tp_Internal_RxIndication_Cm
                                (PduInfoPtr, ChannelInfoPtr);
                            break;
                        /* direct transmission of data less equal 8 bytes */
                        case J1939TP_DIRECT:
                            J1939Tp_Internal_RxIndication_Direct
                                (PduInfoPtr,RxPduInfo);
                            break;
                        default:
                            break;
                    }
                }
                Irq_Restore(state);
            }
        }
    }
}

/* This function is being called periodically J1939TP_MAIN_FUNCTION_PERIOD
 * times in a second.  */

void J1939Tp_MainFunction(void) {
    /** @req J1939TP0030 */
    if (globalState.State == J1939TP_ON)
    {
        /*                 Handle timeout of segmented transmission                               */

        /* Check every communication channel if its timer has expired.
         * The PGs transferred through these channels are either BAM or CMDT frames. */
        for (uint16 i = 0; i < J1939TP_CHANNEL_COUNT; i++)
        {

            J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr = &(channelInfos[i]);
            const J1939Tp_ChannelType*        Channel        = ChannelInfoPtr->ChannelConfPtr;
            J1939Tp_Internal_TimerStatusType  timer          = J1939TP_NOT_EXPIRED;


            if (Channel->Direction == J1939TP_TX)
            {
                /*                 Transmission channel                   */
                imask_t state;
                Irq_Save(state);
                timer = J1939Tp_Internal_IncAndCheckTimer(&(ChannelInfoPtr->TxState->TimerInfo));

                if (timer == J1939TP_EXPIRED)
                {
                    /*       transmission timer has timed out, reset it   */
                    J1939Tp_Internal_StopTimer(&(ChannelInfoPtr->TxState->TimerInfo));

                    /* transmission state machine */

                    switch (ChannelInfoPtr->TxState->State)
                    {
                        case J1939TP_TX_WAIT_BAM_CANIF_CONFIRM:
                        case J1939TP_TX_WAIT_RTS_CANIF_CONFIRM:
                            /* CanIf failed to send the initial connection management frame. */
                            ChannelInfoPtr->TxState->State = J1939TP_TX_IDLE;
                            PduR_J1939TpTxConfirmation
                            (ChannelInfoPtr->TxState->CurrentPgPtr->NSdu, NTFRSLT_E_NOT_OK);
                            break;
                        case J1939TP_TX_WAITING_FOR_CTS:
                        case J1939TP_TX_WAIT_DT_CANIF_CONFIRM:
                        case J1939TP_TX_WAITING_FOR_END_OF_MSG_ACK:
                            ChannelInfoPtr->TxState->State = J1939TP_TX_IDLE;
                             /** @req SWS_J1939Tp_00100 */
                             J1939Tp_Internal_SendConnectionAbort
                             (ChannelInfoPtr, J1939TP_CM, Channel->CmNPdu, CONNABORT_REASON_TIMEOUT);

                             PduR_J1939TpTxConfirmation
                             (ChannelInfoPtr->TxState->CurrentPgPtr->NSdu,NTFRSLT_E_NOT_OK);
                            break;
                        case J1939TP_TX_WAITING_FOR_BAM_DT_SEND_TIMEOUT:
                            /* It's time to send the next BAM data packet */

                            ChannelInfoPtr->TxState->State = J1939TP_TX_WAIT_DT_BAM_CANIF_CONFIRM;

                            J1939Tp_Internal_StartTimer
                            (&(ChannelInfoPtr->TxState->TimerInfo), J1939TP_TX_CONF_TIMEOUT);

                            if (J1939Tp_Internal_SendDt(ChannelInfoPtr) == E_NOT_OK)
                            {
                                /* CanIf failed to send next data packet */
                                J1939Tp_Internal_Reset_Channel(ChannelInfoPtr);

                                /** @req SWS_J1939Tp_00048 */
                                /** @req SWS_J1939Tp_00032 */
                                PduR_J1939TpTxConfirmation
                                (ChannelInfoPtr->TxState->CurrentPgPtr->NSdu, NTFRSLT_E_NOT_OK);

                                J1939Tp_Internal_SendConnectionAbort
                                (ChannelInfoPtr, J1939TP_CM,
                                 ChannelInfoPtr->TxState->CurrentPgPtr->Pgn,
                                 CONNABORT_REASON_TIMEOUT);
                            }
                            break;
                        default:
                            break;
                    }
                }
                Irq_Restore(state);
            }
            else if (Channel->Direction == J1939TP_RX)
            {
                /*                 Reception channel                   */
                imask_t state;
                Irq_Save(state);

                /* Switch on reception status */

                switch (ChannelInfoPtr->RxState->State)
                {
                    case J1939TP_RX_WAIT_CTS_CANIF_CONFIRM:
                    case J1939TP_RX_RECEIVING_DT:
                    case J1939TP_RX_WAIT_ENDOFMSGACK_CANIF_CONFIRM:
                        timer = J1939Tp_Internal_IncAndCheckTimer
                        (&(ChannelInfoPtr->RxState->TimerInfo));
                        break;
                    default:
                        break;
                }
                if (timer == J1939TP_EXPIRED)
                {
                    /* Reception timer has expired: stop timer and set channel in idle state */
                    J1939Tp_Internal_Reset_Channel(ChannelInfoPtr);

                    if (ChannelInfoPtr->Protocol == J1939TP_PROTOCOL_CMDT)
                    {
                        /** @req SWS_J1939Tp_00159 */
                        J1939Tp_Internal_SendConnectionAbort
                        (ChannelInfoPtr, J1939TP_REVERSE_CM,
                         ChannelInfoPtr->RxState->CurrentPgPtr->Pgn,
                         CONNABORT_REASON_TIMEOUT);
                    }
                    /** @req SWS_J1939Tp_00031 */
                    PduR_J1939TpRxIndication
                    (ChannelInfoPtr->RxState->CurrentPgPtr->NSdu,NTFRSLT_E_NOT_OK);
                }
                Irq_Restore(state);
            }
        }

        /*                 Handle timeout of direct transmission                                  */

        /* Check timer expiration for all parameter groups (PG) being transferred directly. */
        for (uint16 i = 0; i < J1939TP_PG_COUNT; i++)
        {
            imask_t state;
            Irq_Save(state);
            if (pgInfos[i].TxState != J1939TP_PG_TX_IDLE)
            {
                if (J1939Tp_Internal_IncAndCheckTimer(&(pgInfos[i].TimerInfo)) == J1939TP_EXPIRED)
                {
                    /* Timer expired, abort direct transmission. */
                    pgInfos[i].TxState = J1939TP_PG_TX_IDLE;
                    PduR_J1939TpTxConfirmation(pgInfos[i].PgConfPtr->NSdu, NTFRSLT_E_NOT_OK);
                }
            }
            Irq_Restore(state);
        }
    }
}

/**
 * @brief CanIf calls this function to confirm the successful transmission
 *        of a N-PDU, be it a CM, DT or direct frame.
 *
 * @param   RxPdu ID of the sent N-PDU.
 * @returns nothing
 */
void J1939Tp_TxConfirmation(PduIdType RxPdu)
{
    /** @req J1939TP0030 */
    if (globalState.State == J1939TP_ON)
    {
        const J1939Tp_RxPduInfoRelationsType* RxPduRelationsInfo = 0;

        if (J1939Tp_Internal_GetRxPduRelationsInfo(RxPdu, &RxPduRelationsInfo) == E_OK)
        {
            /* RxPdu is a valid PDU ID */
            imask_t state;
            Irq_Save(state);

            /* Handle all PGs associated with this PDU ID
             * (a SDU/PDU ID can be used in more than one role) */
            for (PduIdType i = 0; i < RxPduRelationsInfo->RxPduCount; i++)
            {
                const J1939Tp_RxPduInfoType* RxPduInfo = RxPduRelationsInfo->RxPdus[i];
                const J1939Tp_ChannelType*   Channel = J1939Tp_Internal_GetChannel(RxPduInfo);

                J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr = J1939Tp_Internal_GetChannelState(RxPduInfo);
                switch (Channel->Direction)
                {
                    case J1939TP_TX:
                        J1939Tp_Internal_TxConfirmation_TxChannel(ChannelInfoPtr, RxPduInfo);
                        break;
                    case J1939TP_RX:
                        J1939Tp_Internal_TxConfirmation_RxChannel(ChannelInfoPtr, RxPduInfo);
                        break;
                    default:
                        break;
                }
            }
            Irq_Restore(state);
        }
    }
}

static inline const J1939Tp_ChannelType* J1939Tp_Internal_GetChannel
(const J1939Tp_RxPduInfoType* RxPduInfo)
{
    return &(J1939Tp_ConfigPtr->Channels[RxPduInfo->ChannelIndex]);
}

static inline J1939Tp_Internal_ChannelInfoType* J1939Tp_Internal_GetChannelState
(const J1939Tp_RxPduInfoType* RxPduInfo)
{
    return &(channelInfos[RxPduInfo->ChannelIndex]);
}

static inline Std_ReturnType J1939Tp_Internal_ValidatePacketType(const J1939Tp_RxPduInfoType* RxPduInfo) {
    const J1939Tp_ChannelType* ChannelPtr = J1939Tp_Internal_GetChannel(RxPduInfo);
    switch (RxPduInfo->PacketType) {
        case J1939TP_REVERSE_CM:
            if (ChannelPtr->Direction != J1939TP_TX ) {
                return E_NOT_OK;
            }
            break;
        case J1939TP_DT:
            if (ChannelPtr->Direction != J1939TP_RX) {
                return E_NOT_OK;
            }
            break;
        case J1939TP_CM:
            if (ChannelPtr->Direction != J1939TP_RX) {
                return E_NOT_OK;
            }
            break;
        default:
            return E_NOT_OK;
    }
    return E_OK;
}


/*------------------[Handle reception of Direct Frames]-------------------------------------------*/

/**
 * @brief Handle reception of direct frames.
 *
 * Direct frames are used for messages (PGs) with data less or equal 8 bytes,
 * where no segmentation is necessary.
 *
 * @param PduInfoPtr    Pointer to the received PDU structure
 * @param RxPduInfoPtr  Pointer to the associated PG meta information holding the packet type,
 *                      channel index and the actual PG description.
 */
static inline void J1939Tp_Internal_RxIndication_Direct
(
    PduInfoType* PduInfoPtr, const J1939Tp_RxPduInfoType* RxPduInfoPtr
)
{
    PduLengthType messageSize      = PduInfoPtr->SduLength;
    const J1939Tp_PgType* Pg       = 0;
    PduLengthType remainingBuffer  = 0;
    BufReq_ReturnType PduR_startOK = BUFREQ_NOT_OK;
    BufReq_ReturnType r            = BUFREQ_NOT_OK;

    if (messageSize <= DIRECT_TRANSMIT_SIZE)
    {
        Pg = RxPduInfoPtr->PgPtr;

        PduR_startOK = PduR_J1939TpStartOfReception
                (Pg->NSdu, PduInfoPtr, messageSize, &remainingBuffer);

        /** @req SWS_J1939Tp_00040 */
        if (PduR_startOK == BUFREQ_OK && remainingBuffer >= messageSize)
        {
            r = PduR_J1939TpCopyRxData(Pg->NSdu, PduInfoPtr, &remainingBuffer);
            if (r == BUFREQ_OK)
            {
                PduR_J1939TpRxIndication(Pg->NSdu, NTFRSLT_OK);
            }
            else
            {
                PduR_J1939TpRxIndication(Pg->NSdu, NTFRSLT_E_NOT_OK);
                /* SWS_J1939Tp_00040 sais we should abort connection if
                 * "the PduR_J1939TpCopyRxData function returns BUFREQ_E_NOT_OK"
                 * but here we cannot send the connection abort message because
                 * direct frames are not connection managed. */
            }
        }
   }
   /* No need to abort connection for reception of direct frames */
}
/*------------------------------------------------------------------------------------------------*/

/*------------------[Get number of DT packets for next CTS]---------------------------------------*/
/**
 * @brief Get the number of TP.DT packets to declare in the next CTS message.
 *
 * This function computes how many packets to declare in the next Clear To Send
 * (CTS) message based on how many packets were received and the total amount of
 * packets to receive.
 *
 * This function is used before sending a CTS message.
 *
 * @param receivedDtPackets        how many packets received currently
 * @param totalDtPacketsToReceive  total amount of packets to receive
 *
 * @returns how many packets to declare in the next CTS message
 */
static inline uint8 J1939Tp_Internal_GetDtPacketsInNextCts
(
    uint8 receivedDtPackets, uint8 totalDtPacketsToReceive
)
{
    /* Packets left until we get all of them */
    uint8 packetsLeft = totalDtPacketsToReceive - receivedDtPackets;

    /* J1939TP_PACKETS_PER_BLOCK is how many packets maximal we declare in a
     * CTS message. A block is a sequence of packets we receive between two CTS
     * messages.
     * We don't use the symbolic value J1939TP_PACKETS_PER_BLOCK but the variable
     * Internal_J1939TpPacketsPerBlock, because the function J1939Tp_ChangeParameter
     * might change this parameter. */
    if (packetsLeft < (Internal_J1939TpPacketsPerBlock)) {
        return packetsLeft;
    } else {
        return Internal_J1939TpPacketsPerBlock;
    }
}
/*------------------------------------------------------------------------------------------------*/

/*------------------[Handle reception of Data frames]---------------------------------------------*/
/**
 * Handle reception of data packet.
 *
 * @param PduInfoPtr       received packet
 * @param ChannelInfoPtr   reception channel
 */
static inline void J1939Tp_Internal_RxIndication_Dt
(PduInfoType* PduInfoPtr, J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr)
{
    if
    ((ChannelInfoPtr == NULL_PTR)                                ||
     (ChannelInfoPtr->RxState == NULL_PTR)                       ||
     (ChannelInfoPtr->RxState->CurrentPgPtr == NULL_PTR)         ||
     (ChannelInfoPtr->RxState->State != J1939TP_RX_RECEIVING_DT)
    )
    {
        return;
    }

    /* Get the SDU ID of the received data packet */
    PduIdType PduRSdu = ChannelInfoPtr->RxState->CurrentPgPtr->NSdu;

    /* Get the protocol used to transfer the packet
     * (there is a protocol because direct frames are handled in another function) */
    const J1939Tp_ProtocolType
        protocol = ChannelInfoPtr->Protocol;

    /* Increment number of received packets through this channel */
    ChannelInfoPtr->RxState->ReceivedDtCount++;

    /* Which packet number we expect */
    uint8 expectedSeqNumber = ChannelInfoPtr->RxState->ReceivedDtCount;

    /* Which packet number is has been effectively received.
     * The packet number is stored in the first data byte */
    uint8 seqNumber = PduInfoPtr->SduDataPtr[DT_BYTE_SEQ_NUM];

    if (seqNumber != expectedSeqNumber) {
        /* The received packet number does not match the expected packet number.
         * See item SWS_J1939Tp_00216 of the Autosar J1939 specification v4.1 */
        ChannelInfoPtr->RxState->State = J1939TP_RX_IDLE;
        if (protocol == J1939TP_PROTOCOL_CMDT) {

            J1939Tp_PgnType pgn = ChannelInfoPtr->RxState->CurrentPgPtr->Pgn;
            /** @req SWS_J1939Tp_00216 */
            /** @req SWS_J1939Tp_00192 */
            J1939Tp_Internal_SendConnectionAbort
            (ChannelInfoPtr, J1939TP_REVERSE_CM, pgn, CONNABORT_REASON_SEQUENCE_ERR);
        }
        PduR_J1939TpRxIndication(ChannelInfoPtr->RxState->CurrentPgPtr->NSdu, NTFRSLT_E_NOT_OK);
        return;
    }

    PduLengthType remainingBytesInRxBuffer = 0;

    PduInfoType PduInfoRxCopy;
    PduInfoRxCopy.SduLength = J1939Tp_Internal_GetDtDataSize(seqNumber,ChannelInfoPtr->RxState->TotalMessageSize);
    PduInfoRxCopy.SduDataPtr = &(PduInfoPtr->SduDataPtr[DT_BYTE_DATA_1]);

    /** @req SWS_J1939Tp_00162 */
    BufReq_ReturnType r =
            PduR_J1939TpCopyRxData
            (ChannelInfoPtr->RxState->CurrentPgPtr->NSdu, &PduInfoRxCopy, &remainingBytesInRxBuffer);

    if (r != BUFREQ_OK)
    {
        /* PduR could not copy the received data into the buffers of the upper layer.
         * Abort connection if CMDT is in usage. */

        /* Note: J1939Tp does not use internal PDU buffers, but calls PduR_J1939TpCopyTxData and
         * PduR_J1939TpCopyRxData to transfer data to upper layers.
         * See Autosar specification v4.1 of the J1939Tp module,
         * chapter 7.5.5 N-SDU Buffer Management. */

        if (protocol == J1939TP_PROTOCOL_CMDT) {
            J1939Tp_PgnType pgn = ChannelInfoPtr->RxState->CurrentPgPtr->Pgn;

            /** @req SWS_J1939Tp_00040 */

            J1939Tp_Internal_SendConnectionAbort
            (ChannelInfoPtr, J1939TP_REVERSE_CM, pgn, CONNABORT_REASON_NO_RESOURCES);
        }
        /* set reception state of the channel to idle */
        ChannelInfoPtr->RxState->State = J1939TP_RX_IDLE;
        return;
    }

    /* If this is the last data packet before we should send another CTS message,
     * then send the new CTS message */
    if (protocol == J1939TP_PROTOCOL_CMDT &&
        J1939Tp_Internal_IsLastDtBeforeNextCts(ChannelInfoPtr->RxState)
       )
    {
        J1939Tp_PgnType pgn = ChannelInfoPtr->RxState->CurrentPgPtr->Pgn;

        /* How many DT packets should we request in the CTS message */
        uint8 requestPackets =
                J1939Tp_Internal_GetDtPacketsInNextCts
                (ChannelInfoPtr->RxState->ReceivedDtCount, ChannelInfoPtr->RxState->DtToReceiveCount);

        if (Internal_J1939TpPacketsPerBlock < requestPackets)
        {
            requestPackets = Internal_J1939TpPacketsPerBlock;
        }

        /* Change reception channel state to 'wait for CanIf to transmit the CTS' */
        ChannelInfoPtr->RxState->State = J1939TP_RX_WAIT_CTS_CANIF_CONFIRM;

        /* Transmit the CTS message */
        J1939Tp_Internal_SendCts
            (ChannelInfoPtr, pgn, ChannelInfoPtr->RxState->ReceivedDtCount+1, requestPackets);
    }
    /* If this the last DT packet (no new CTS needed) */
    else if (J1939Tp_Internal_IsLastDt(ChannelInfoPtr->RxState))
    {
        /* Send the Aknowledge message if were using CMTD, not needed for BAM */
        if (ChannelInfoPtr->Protocol == J1939TP_PROTOCOL_CMDT)
        {
            J1939Tp_Internal_SendEndOfMsgAck(ChannelInfoPtr);
        }

        /* Change state to idle */
        ChannelInfoPtr->RxState->State = J1939TP_RX_IDLE;
        /* Tell PduR we're ready  */
        PduR_J1939TpRxIndication(PduRSdu, NTFRSLT_OK);
    } else {
        /* Wait until we send the next packet */
        J1939Tp_Internal_StartTimer(&(ChannelInfoPtr->RxState->TimerInfo), J1939TP_T1_TIMEOUT_MS);
    }
}
/*------------------------------------------------------------------------------------------------*/

static inline uint8 J1939Tp_Internal_GetDtDataSize(uint8 currentSeqNum, uint8 totalSize) {
    if (currentSeqNum*DT_DATA_SIZE <= totalSize) {
        return DT_DATA_SIZE;
    } else {
        return DT_DATA_SIZE - ((currentSeqNum*DT_DATA_SIZE) - totalSize);
    }
}

/*------------------[Handle reception of RTS, BAM and ConnAbort]----------------------------------*/
/**
 * @brief Handle reception of Connection Management frames.
 *
 * This function handles the reception of TP.CM frames,
 * i.e. of RTS, BAM and ConnAbort messages.
 *
 *
 * @param PduInfoPtr       Pointer to received Pdu Info structure.
 * @param ChannelInfoPtr   Reception channel
 */
static inline void J1939Tp_Internal_RxIndication_Cm
(
    PduInfoType                      *PduInfoPtr,
    J1939Tp_Internal_ChannelInfoType *ChannelInfoPtr
)
{
    /* Do nothing if the reception channel is not in idle state */
    if (ChannelInfoPtr->RxState        == NULL_PTR       ||
        ChannelInfoPtr->RxState->State != J1939TP_RX_IDLE)
    {
        return;
    }

    /* Get the PGN from the PDU data buffer */
    J1939Tp_PgnType pgn =
            J1939Tp_Internal_GetPgn(&(PduInfoPtr->SduDataPtr[CM_PGN_BYTE_1]));

    const J1939Tp_PgType* pg = NULL_PTR;
    /* Get the PG structure correspoding to the received PGN */
    if (J1939Tp_Internal_GetPgFromPgn(ChannelInfoPtr->ChannelConfPtr, pgn ,&pg)
            != E_OK)
    {
        /* No such PG, wrong PGN? */
        return;
    }

    /* The command byte is the first one in the received data.
     * It makes the difference between the different messages. */
    uint8 Command = PduInfoPtr->SduDataPtr[CM_BYTE_CONTROL];

    /* The message size contained in BAM or RTS */
    J1939Tp_Internal_DtPayloadSizeType
        messageSize = J1939Tp_Internal_GetRtsMessageSize(PduInfoPtr);

    /** @req J1939TP0043**/
    /** @req SWS_J1939Tp_00043 */
    if (Command == RTS_CONTROL_VALUE || Command == BAM_CONTROL_VALUE)
    {
        /* We have received a BAM or RTS message       */
        /* Setup the reception metadata of the channel */
        ChannelInfoPtr->RxState->ReceivedDtCount  = 0;
        ChannelInfoPtr->RxState->DtToReceiveCount = PduInfoPtr->SduDataPtr[BAM_BYTE_NUM_PACKETS];
        ChannelInfoPtr->RxState->TotalMessageSize = messageSize;
        ChannelInfoPtr->RxState->CurrentPgPtr     = pg;

        /* Protocol used on the channel */
        J1939Tp_ProtocolType
            channelProtocol = ChannelInfoPtr->Protocol;

        if (Command == RTS_CONTROL_VALUE)
        {
        /*                       RTS - Request To Send                          */

            /** @req SWS_J1939Tp_00173 */
            /* According to SWS_J1939Tp_00173:
             * abort the reception silently if the protocol chosen based on
             * Command does not match the transport protocol configured for the
             * transported PGN */
            if (channelProtocol == J1939TP_PROTOCOL_CMDT)
            {
                /*                  CMDT && RTS                     */
                PduLengthType remainingBuffer = 0;
                /** @req SWS_J1939Tp_00038 */
                BufReq_ReturnType PduR_startOK = PduR_J1939TpStartOfReception
                            (pg->NSdu, PduInfoPtr, messageSize, &remainingBuffer);

                /** @req SWS_J1939Tp_00040 */
                /* Abort reception if
                 * "The value returned by PduR_J1939TpStartOfReception via bufferSizePtr
                 * is smaller than the total data length of the N-SDU"*/

                if (PduR_startOK != BUFREQ_OK || remainingBuffer < messageSize)
                {
                    /* PduR failed start of reception or the available buffer
                     * in the upper layer is not large enough so abort transfer */

                    /** @req SWS_J1939Tp_00173*/
                    J1939Tp_Internal_SendConnectionAbort
                    (ChannelInfoPtr, J1939TP_CM, pgn, CONNABORT_REASON_BUSY);
                }
                else
                {
                    /* Buffer ok and PduR did not fail,
                     * so send the CTS message in response to the RTS message*/

                    /* how many packets to declare in the CTS message i.e. how many packets
                     * can we receive in the next block.
                     * We intitialize this value to the number of packets the sender wants to send. */
                    uint8 receive_packets = PduInfoPtr->SduDataPtr[RTS_BYTE_NUM_PACKETS];

                    /* Add support for dynamic block size calculation */
#if (J1939TP_DYNAMIC_BLOCK_CALCULATION == STD_ON)
/* dynamic block size calculation is enabled */
  #if defined(J1939TP_PACKETS_PER_BLOCK)
                    /** @req SWS_J1939Tp_00210 */
                    receive_packets = J1939TP_Internal_GetNumDtPacketsToSend(remainingBuffer);
                    if (Internal_J1939TpPacketsPerBlock < receive_packets)
                    {
                        receive_packets = Internal_J1939TpPacketsPerBlock;
                    }
  #else
                    /** @req SWS_J1939Tp_00211 */
                    receive_packets = J1939TP_Internal_GetNumDtPacketsToSend(remainingBuffer);
  #endif
#else
/* dynamic block size calculation is disabled */
  #if defined(J1939TP_PACKETS_PER_BLOCK)
                    /** @req SWS_J1939Tp_00213 */
                    if (Internal_J1939TpPacketsPerBlock < receive_packets)
                    {
                        receive_packets = Internal_J1939TpPacketsPerBlock;
                    }
  #endif
#endif
                    /* Set the reception state to
                     * 'waiting for CanIf to confirm sending of CTS message' */
                    ChannelInfoPtr->RxState->State = J1939TP_RX_WAIT_CTS_CANIF_CONFIRM;

                    J1939Tp_Internal_StartTimer
                        (&(ChannelInfoPtr->RxState->TimerInfo), J1939TP_TX_CONF_TIMEOUT);

                    /* Send the Clear To Send message i.e.
                     * instruct the sending node to start sending */

                    J1939Tp_Internal_SendCts(ChannelInfoPtr, pgn, CTS_START_SEQ_NUM, receive_packets);
                }
            }
            else
            {
                /*                  Protocols do not match                     */
                /*                  RTS not setup with CMDT                    */
                /*                  see SWS_J1939Tp_00173                      */
                J1939Tp_Internal_SendConnectionAbort
                (ChannelInfoPtr, J1939TP_CM, pgn, CONNABORT_REASON_BUSY);
            }

        }
        else if (Command == BAM_CONTROL_VALUE && channelProtocol == J1939TP_PROTOCOL_BAM)
        {
        /*                        BAM                                         */

            PduLengthType remainingBuffer = 0;
            /** @req SWS_J1939Tp_00038 */
            BufReq_ReturnType PduR_startOK =
                    PduR_J1939TpStartOfReception
                        (pg->NSdu, PduInfoPtr, messageSize, &remainingBuffer);

            if (PduR_startOK == BUFREQ_OK)
            {
                J1939Tp_Internal_StartTimer(&(ChannelInfoPtr->RxState->TimerInfo),J1939TP_T2_TIMEOUT_MS);

                /* Set the reception channel to state 'Receiving data'
                 * because after the BAM announce message the sending node starts sending. */
                ChannelInfoPtr->RxState->State = J1939TP_RX_RECEIVING_DT;
            }
        }
    } /* No BAM or RTS message */
    else if (Command == CONNABORT_CONTROL_VALUE)
    {
        /* Connection Abort message, set the channel into idle state */
        ChannelInfoPtr->RxState->State = J1939TP_RX_IDLE;
    }
}
/*------------------------------------------------------------------------------------------------*/

/*------------------[Handle CTS, EndOfMsgAck and ConnAbort]---------------------------------------*/
/**
 * @brief Handle control flow messages CTS, EndOfMsgAck and ConnAbort.
 *
 * @details J1939Tp_RxIndication branches to this function when the receiver of CMDT data transfer
 *          sends control flow messages. Within the message, the receiver sends a command byte,
 *          which is one of the following (defined in J1939Tp_Internal_Packets.h):
 * - CTS_CONTROL_VALUE          : Clear To Send, the acknowledging party is
 *                                ready to receive more data.
 * - ENDOFMSGACK_CONTROL_VALUE  : Aknowledge (confirm) the End of Message
 * - CONNABORT_CONTROL_VALUE    : Connection Abort
 * Other command values are ignored.
 */
static inline void J1939Tp_Internal_RxIndication_ReverseCm
(PduInfoType* PduInfoPtr, J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr)
{
    const J1939Tp_PgType* pg = NULL_PTR;
    J1939Tp_PgnType pgn = J1939Tp_Internal_GetPgn(&(PduInfoPtr->SduDataPtr[CM_PGN_BYTE_1]));

    if (J1939Tp_Internal_GetPgFromPgn(ChannelInfoPtr->ChannelConfPtr,pgn,&pg)
            != E_OK)
    {
        return;
    }

    /* the command byte is the first byte of the data */
    uint8 Command = PduInfoPtr->SduDataPtr[CM_BYTE_CONTROL];
    switch (Command) {
        case CTS_CONTROL_VALUE:
            /* Clear To Send: other party is ready to receive more data */
            if (ChannelInfoPtr->TxState->State == J1939TP_TX_WAITING_FOR_CTS)
            {
                /* how many packets can the other party receive */
                uint8 NumPacketsToSend = PduInfoPtr->SduDataPtr[CTS_BYTE_NUM_PACKETS];
                /* next packet number the receiver expects */
                uint8 NextPacket = PduInfoPtr->SduDataPtr[CTS_BYTE_NEXT_PACKET];

                /* Handle wait frame */
                if (NumPacketsToSend == 0)
                {
                    // Receiver wants to keep the connection open but cant receive packets
                    /** @req J1939TP0195 */
                    /** @req SWS_J1939Tp_00195 */
                    J1939Tp_Internal_StartTimer(&(ChannelInfoPtr->TxState->TimerInfo),
                            J1939TP_T4_TIMEOUT_MS);
                }
                else if(J1939Tp_Internal_IsDtPacketAlreadySent(NextPacket,ChannelInfoPtr->TxState->SentDtCount))
                {
                    /** @req J1939TP0190 */
                    /** @req SWS_J1939Tp_00194 */

                    /** @req SWS_J1939Tp_00032 */
                    /* receiver expects a packet we already sent */
                    PduR_J1939TpTxConfirmation(pg->NSdu,NTFRSLT_E_NOT_OK);


                    /** @req SWS_J1939Tp_00097 */
                    J1939Tp_Internal_SendConnectionAbort
                    (ChannelInfoPtr, J1939TP_CM, pgn, CONNABORT_REASON_MAX_RETRY_LIMIT);

                    ChannelInfoPtr->TxState->State = J1939TP_TX_IDLE;
                }
                else
                {
                    ChannelInfoPtr->TxState->DtToSendBeforeCtsCount = NumPacketsToSend;
                    ChannelInfoPtr->TxState->State = J1939TP_TX_WAIT_DT_CANIF_CONFIRM;
                    if (J1939Tp_Internal_SendDt(ChannelInfoPtr) == E_NOT_OK)
                    {
                        ChannelInfoPtr->TxState->State = J1939TP_TX_IDLE;

                        /** @req SWS_J1939Tp_00048 */
                        /** @req SWS_J1939Tp_00032 */
                        PduR_J1939TpTxConfirmation
                        (ChannelInfoPtr->TxState->CurrentPgPtr->NSdu, NTFRSLT_E_NOT_OK);

                        J1939Tp_Internal_SendConnectionAbort
                        (ChannelInfoPtr, J1939TP_CM,
                         ChannelInfoPtr->TxState->CurrentPgPtr->Pgn,
                         CONNABORT_REASON_NO_RESOURCES);
                    } else {
                        J1939Tp_Internal_StartTimer(&(ChannelInfoPtr->TxState->TimerInfo),J1939TP_TX_CONF_TIMEOUT);
                    }
                }
            }
            break;
        case ENDOFMSGACK_CONTROL_VALUE:
            if (ChannelInfoPtr->TxState->State == J1939TP_TX_WAITING_FOR_END_OF_MSG_ACK)
            {
                /** @req SWS_J1939Tp_00119 */
                PduR_J1939TpTxConfirmation(pg->NSdu,NTFRSLT_OK);
                ChannelInfoPtr->TxState->State = J1939TP_TX_IDLE;
            }
            break;
        case CONNABORT_CONTROL_VALUE:
            if (ChannelInfoPtr->TxState->State != J1939TP_TX_IDLE) {
                ChannelInfoPtr->TxState->State = J1939TP_TX_IDLE;
                PduR_J1939TpTxConfirmation(pg->NSdu,NTFRSLT_E_NOT_OK);
            }
            break;
        default:
            break;
    }
}



/*------------------[Change Parameter]------------------------------------------------------------*/
/** @req SWS_J1939Tp_00180 */
/**
 * @brief  Change reception or transmission parameters for a specific N-SDU.
 *
 *
 * @param  SduId      SDU ID for which parameters should be changed.
 *                    Currently this parameter is only checked to be valid but not used.
 * @param  Parameter  Parameter-ID.
 *                    Currently only the parameter TP_BS is supported (defined in ComStack_Types.h).
 * @param  Value      New value for changed parameter.
 *
 *
 * @retval E_OK       The request has been accepted.
 * @retval E_NOT_OK   The request failed.
 *
 * @see Specification of a Transport Layer for SAE J1939, V1.3.0, R4.1 Rev 1
 *      Items SWS_J1939Tp_00180, SWS_J1939Tp_00206, SWS_J1939Tp_00205 and ECUC_J1939Tp_00061.
 */
Std_ReturnType J1939Tp_ChangeParameter
(
    PduIdType       SduId,
    TPParameterType Parameter,
    uint16          Value
)
{
    Std_ReturnType status = E_NOT_OK;
    J1939Tp_Internal_PgInfoType* PgInfo = NULL_PTR;

    /** @req SWS_J1939Tp_00206 */
    if (globalState.State == J1939TP_ON)
    {
        /* This sets status to E_OK if SduId is a valid ID,
         * otherwise to E_NOT_OK */
        status = J1939Tp_Internal_GetPg(SduId, &PgInfo);
        if (status == E_OK)
        {
            if (Parameter != TP_BS)
            {
                /* v4.1 of Autosar supports only changing the block size,
                 * see SWS_J1939Tp_00206 */
                status = E_NOT_OK;
            }
            else if (Value > (uint16) 255u)
            {
                /* The new value has to be no larger than 255 because this value takes in
                 * CTS and RTS messages a single byte. */
                status = E_NOT_OK;
            }
            else if (status ==  E_OK)
            {
                /* Item SWS_J1939Tp_00180 sais we should change the parameter
                 * for a specific N-SDU but at the same time the item SWS_J1939Tp_00205
                 * specifies we should change the value of J1939TpPacketsPerBlock,
                 * which is a global value and not specific to a N-SDU.
                 * So at this point we effectively ignore SduId and change a global variable. */
                /** @req SWS_J1939Tp_00205 */
                Internal_J1939TpPacketsPerBlock = Value;
            }
        }
    }

    return status;
}
/*------------------------------------------------------------------------------------------------*/


/**
 * @brief Handle transmission confirmation on a reception channel.
 *
 * CTS messages are sent using a reception channel for flow control.
 *
 * @param ChannelInfoPtr reception channel
 * @param RxPduInfo      PDU description, containing the packet type,
 *                       channel index and a pointer to a PG description.
 */
static inline void J1939Tp_Internal_TxConfirmation_RxChannel
(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr, const J1939Tp_RxPduInfoType* RxPduInfo)
{
    switch (RxPduInfo->PacketType) {
        case J1939TP_REVERSE_CM:
            if (ChannelInfoPtr->RxState->State == J1939TP_RX_WAIT_CTS_CANIF_CONFIRM) {

                /* the transmitted PDU is used for reverse direction
                 * flow control and we are waiting for CanIf to confirm that
                 * it sent a CTS message */

                /* Start the channel timer with value T2.
                 * If the timer expires, then we didn't get any data frames
                 * althought we sent the CTS message. */
                J1939Tp_Internal_StartTimer(&(ChannelInfoPtr->RxState->TimerInfo),J1939TP_T2_TIMEOUT_MS);

                /* Put the reception channel into receiving state */
                ChannelInfoPtr->RxState->State = J1939TP_RX_RECEIVING_DT;
            }
            break;
        default:
            break;
    }
}
static inline boolean J1939Tp_Internal_IsDtPacketAlreadySent(uint8 nextPacket, uint8 totalPacketsSent) {
    return nextPacket < totalPacketsSent;
}

/*------------------[Get PDU relations info]------------------------------------------------------*/
/**
 * Get a pointer to the J1939Tp_RxPduInfoRelationsType structure
 * with index RxPdu.
 *
 * @param    PduId          ID of a received or transmitted I-PDU.
 * @param    RxPduInfo      Address where to store the found PDU relations
 *                          structure.
 * @retval   E_OK           PduId is a valid PDU ID
 * @retval   E_NOT_OK       PduId is not a valid PDU ID
 *
 * @note     The returned J1939Tp_RxPduInfoRelationsType holds a list of
 *           J1939Tp_RxPduInfoType describing the relation of the PDU with ID
 *           PduId with a channel, PG and packet type.
 */
static inline Std_ReturnType J1939Tp_Internal_GetRxPduRelationsInfo
(PduIdType PduId, const J1939Tp_RxPduInfoRelationsType** RxPduInfo)
{
    Std_ReturnType r = E_NOT_OK;

    if (PduId < J1939TP_RX_PDU_COUNT) {
        /* J1939Tp_ConfigPtr is the one passed to the init function */
        *RxPduInfo = &(J1939Tp_ConfigPtr->RxPduRelations[PduId]);
        r = E_OK;
    }
    return r;
}
/*------------------------------------------------------------------------------------------------*/

/*------------------[Get PG with PGN on channel]--------------------------------------------------*/
/**
 * @brief Get the PG associated with the PGN on a certain channel.
 *
 * This function searches for a Parameter Group (PG) structure with the
 * specified Parameter Group Number (PGN) in the list of PGs of the specified
 * channel. If it finds such a PG, it stores its address at address *Pg and
 * returns E_OK. If it finds no such PG structure, it returns E_NOT_OK without
 * writing to *Pg.
 *
 * @param   channel   Channel storing the PG list.
 * @param   Pgn       PGN to search for.
 * @param   Pg        Address where to store a pointer to PG if the PGN was
 *                    found.
 *
 * @retval  E_OK      The PG with the specified PGN has been found and its
 *                    address is stored in *Pg.
 * @retval  E_NOT_OK  The requested PGN has not been found.
 */
static inline Std_ReturnType J1939Tp_Internal_GetPgFromPgn
(
    const J1939Tp_ChannelType       *channel,
          J1939Tp_Internal_PgnType   Pgn,
    const J1939Tp_PgType           **Pg
)
{
    Std_ReturnType found = E_NOT_OK;

    /* Current PG struct while searching, here we read the PGN from */
    const J1939Tp_PgType   *current_PG   = 0;

    /* Current PGN while searching, is read fro current_PG */
    J1939Tp_Internal_PgnType current_PGN = 0;

    /* Index in the PG list of channel, loop control */
    uint16 pg_index = 0;

    /* List size i.e. how many PGs are there on the channel */
    uint16 pg_count = channel->PgCount;

    /* Search the list */
    while (pg_index < pg_count && found != E_OK)
    {
        current_PG  = channel->Pgs[pg_index];
        current_PGN = current_PG->Pgn;
        if (current_PGN == Pgn)
        {
            /* Found! */
            *Pg = current_PG;
            found = E_OK;
        }
        pg_index++;
    }
    return found;

#if 0
    /* Old code for reference.
     * Deleted because not MISRA conform: only one return */
    for (int i = 0; i < channel->PgCount; i++) {
        if (channel->Pgs[i]->Pgn == Pgn) {
            *Pg = channel->Pgs[i];
            return E_OK;
        }
    }
    return E_NOT_OK;
#endif
}
/*------------------------------------------------------------------------------------------------*/

/*
 * Returns true, if the last received Data Transfer message is the last one
 * before we send a new CTS message.
 * This implies that when this functions returns true, a new CTS message should
 * be sent to the data sender.
 * See AUTOSAR_SWS_SAEJ1939TransportLayer.pdf, "7.5.4.2 Data Flow using CMDT"
 * for more information about the Clear To Send (CTS) frame.
 */
static inline boolean J1939Tp_Internal_IsLastDtBeforeNextCts
(J1939Tp_Internal_RxChannelInfoType* rxChannelInfo)
{
    /* if Data Transfer (DT) Messages received until now
     * equals expected Data Transfer messages, that is if we got all expected DTs */
    boolean finalDt    = rxChannelInfo->ReceivedDtCount == rxChannelInfo->DtToReceiveCount;
    /* if number of DTs fills one packet block */
    boolean sendNewCts = rxChannelInfo->ReceivedDtCount % Internal_J1939TpPacketsPerBlock == 0;
    return ( (!finalDt) && sendNewCts );
}

/*------------------[Is it the last received DT?]-------------------------------------------------*/

/**
 * Returns true, if the last received Data Transfer message (DT) is the last one
 * we expect.
 *
 * After this last frame we don't expect any new data messages.
 */
static inline boolean J1939Tp_Internal_IsLastDt
(
    J1939Tp_Internal_RxChannelInfoType* rxChannelInfo
)
{
    return (rxChannelInfo->DtToReceiveCount == rxChannelInfo->ReceivedDtCount);
}
/*------------------------------------------------------------------------------------------------*/

static inline void J1939Tp_Internal_TxConfirmation_TxChannel
(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr, const J1939Tp_RxPduInfoType* RxPduInfo)
{
    J1939Tp_Internal_TxChannelStateType State = ChannelInfoPtr->TxState->State;
    J1939Tp_Internal_PgInfoType* PgInfo = J1939Tp_GetPgInfo(RxPduInfo->PgPtr);

    switch (RxPduInfo->PacketType)
    {
        case J1939TP_REVERSE_CM:
            break;
        case J1939TP_CM:
            if (State == J1939TP_TX_WAIT_RTS_CANIF_CONFIRM)
            {/* The RTS message has been sent. Begin to wait for the CTS message */
                ChannelInfoPtr->TxState->State = J1939TP_TX_WAITING_FOR_CTS;
                J1939Tp_Internal_StartTimer(&(ChannelInfoPtr->TxState->TimerInfo),J1939TP_T3_TIMEOUT_MS);
            }
            else if (State == J1939TP_TX_WAIT_BAM_CANIF_CONFIRM)
            {
                ChannelInfoPtr->TxState->State = J1939TP_TX_WAITING_FOR_BAM_DT_SEND_TIMEOUT;
                J1939Tp_Internal_StartTimer(&(ChannelInfoPtr->TxState->TimerInfo),J1939TP_DT_BROADCAST_MIN_INTERVAL);
            }
            break;
        case J1939TP_DT:
            if (State == J1939TP_TX_WAIT_DT_CANIF_CONFIRM)
            {
                /* Data packet was sent with CMDT */
                ChannelInfoPtr->TxState->SentDtCount++;
                if (J1939Tp_Internal_LastDtSent(ChannelInfoPtr->TxState))
                {
                    J1939Tp_Internal_StartTimer(&(ChannelInfoPtr->TxState->TimerInfo),J1939TP_T3_TIMEOUT_MS);
                    ChannelInfoPtr->TxState->State = J1939TP_TX_WAITING_FOR_END_OF_MSG_ACK;
                }
                else if (J1939Tp_Internal_WaitForCts(ChannelInfoPtr->TxState))
                {
                    J1939Tp_Internal_StartTimer(&(ChannelInfoPtr->TxState->TimerInfo),J1939TP_T3_TIMEOUT_MS);
                    ChannelInfoPtr->TxState->State = J1939TP_TX_WAITING_FOR_CTS;
                }
                else
                {
                    J1939Tp_Internal_StartTimer(&(ChannelInfoPtr->TxState->TimerInfo),J1939TP_TX_CONF_TIMEOUT);
                    J1939Tp_Internal_SendDt(ChannelInfoPtr);
                }
            }
            else if (State == J1939TP_TX_WAIT_DT_BAM_CANIF_CONFIRM)
            {
                /* We get the confirmation of a TP.DT frame transmitted
                 * during BAM transmission */

                ChannelInfoPtr->TxState->SentDtCount++;
                if (J1939Tp_Internal_LastDtSent(ChannelInfoPtr->TxState)) {
                    /* The confirmed message was the last one we had to send.
                     * Put the channel into idle state. */
                    ChannelInfoPtr->TxState->State = J1939TP_TX_IDLE;
                    /* Tell PduR the transmission was ok */
                    /** @req SWS_J1939Tp_00119 */
                    PduR_J1939TpTxConfirmation(ChannelInfoPtr->TxState->CurrentPgPtr->NSdu, NTFRSLT_OK);
                } else {
                    /* There is more BAM data, so wait for the next cycle
                     * to send the next packet. Note: as BAM is not connection
                     * managed, data must be send in intervals of time.
                     * This is where we wait for this interval to expire. */
                    ChannelInfoPtr->TxState->State = J1939TP_TX_WAITING_FOR_BAM_DT_SEND_TIMEOUT;
                    /* Start the channel timer with the minimal BAM interval.
                     * When this timer expires, we send the next data frame. */
                    J1939Tp_Internal_StartTimer(&(ChannelInfoPtr->TxState->TimerInfo),J1939TP_DT_BROADCAST_MIN_INTERVAL);
                }
            }
            break;
        case J1939TP_DIRECT:
            if (PgInfo->TxState == J1939TP_PG_TX_WAIT_DIRECT_SEND_CANIF_CONFIRM)
            {
                PgInfo->TxState = J1939TP_PG_TX_IDLE;
                /** @req SWS_J1939Tp_00119 */
                PduR_J1939TpTxConfirmation(RxPduInfo->PgPtr->NSdu, NTFRSLT_OK);
            }
            /* PgInfo->TxState is already in idle state */
            break;
        default:
            break;
    }
}

/*------------------[Get Parameter Group]---------------------------------------------------------*/
/**
 * @brief Get the Parameter Group structure associated with the specified SDU ID.
 *
 * The returned pointer points to a structure containing information about the channel used to
 * transfer the PG, the PG itself, transmission state and a timer.
 *
 * @param  SduId      SDU ID
 * @param  PgInfo     PG structure where to store the associated PG.
 *
 * @retval E_OK       SduId is a valid SDU ID
 * @retval E_NOT_OK   SduId is not a valid SDU ID
 */
static inline Std_ReturnType J1939Tp_Internal_GetPg
(PduIdType SduId, J1939Tp_Internal_PgInfoType** PgInfo)
{
    Std_ReturnType ret = E_NOT_OK;

    if (SduId < J1939TP_PG_COUNT) {
        *PgInfo = &(pgInfos[SduId]);
        ret = E_OK;
    }

    return ret;
}
/*------------------------------------------------------------------------------------------------*/

/*------------------[Cancel Transmission]---------------------------------------------------------*/
/** @req SWS_J1939Tp_00214 */
#if (J1939TP_CANCELLATION_SUPPORT == STD_ON)
/** @req SWS_J1939Tp_00177 */
/**
 * @brief Cancel the transmission of the SDU with ID TxSduId.
 *
 * This function will fail and return E_NOT_OK under the following conditions:
 * - the J1939Tp module was not initialized (function J1939Tp_Init)
 * - TxSduId is not a valid SDU ID
 * - The PG associated with this SDU is not being used on a transmission channel
 *   which means, TxSduId points to reception SDU.
 * - The transmission state of the used channel is not set (internal or configuration error).
 * - TxSduId points to a direct frame. Direct frames are simple, not segmented 8 bytes frames.
 * - The last message was has already been sent through BAM.

 *
 * @param  TxSduId   The SDU ID of the J1939Tp N-SDU to be canceled.
 * @retval E_OK      Cancelation ok.
 * @retval E_NOT_OK  Cancelation failed.
 *
 * @note   The requirements to this function are specified in item SWS_J1939Tp_00203 of the
 *         Autosar R4.1 Rev 1.
 */
Std_ReturnType J1939Tp_CancelTransmit(PduIdType TxSduId)
{
    /* Control variable used to monitor the correct functioning.
     * We also return the value of it. */
    Std_ReturnType status = E_OK;

    /* Meta information about the PG. Also contains information about the used channel. */
    J1939Tp_Internal_PgInfoType* PgInfo = 0;


    if (globalState.State == J1939TP_OFF)
    {
        status = E_NOT_OK;
        /* Module was not initialised */
    }

    /** @req SWS_J1939Tp_00203 */

    /* Check if TxSduId is a valid SDU ID.
     * If TxSduId is invalid, the following call returns E_NOT_OK. */

    if (status == E_OK)
    {
        status = J1939Tp_Internal_GetPg(TxSduId, &PgInfo);
    }


    /* Check if "currently a direct frame is transmitted".
     * We check if the specified SDU ID is used to transmit a direct frame.
     * Direct frames cannot be canceled because they are not segmented.
     * Once the Can controller has been instructed to transmit the direct frame,
     * we cannot take the message back from the network. */
    if (status == E_OK)
    {
        if (PgInfo->PgConfPtr->DynLength  == true ||
            PgInfo->PgConfPtr->DirectNPdu != ((PduIdType) -1))
        {
            status = E_NOT_OK;
        }
    }


    /* Check that the used channel is indeed a transmission channel
     * (and not a reception one) */
    if (status == E_OK)
    {
        if (PgInfo->ChannelInfoPtr->ChannelConfPtr->Direction != J1939TP_TX)
        {
            status = E_NOT_OK;
        }
        if (! PgInfo->ChannelInfoPtr->TxState)
        {
            /* For some reason the transmission state pointer is null
             * and we don't want to dereference the null pointer later */
            status = E_NOT_OK;
        }
    }

    /* Check "if TxSduId is currently not active".
     * I interpret this as "if the channel used to transmit the associated PG
     * is idle", meaning the message was already transmitted so we cannot cancel
     * it anylonger. */
    if (status == E_OK)
    {
        if (PgInfo->ChannelInfoPtr->TxState->State == J1939TP_TX_IDLE)
        { /* Nothing going on on the channel, so nothing to cancel. */
            status = E_NOT_OK;
        }
    }

    /* Check if "the last TP.DT frame has already been transmitted during BAM
     * transmission". Here we have a point where the specification is not very
     * clear. If the last BAM data frame has been successfully transmitted, then
     * the channel is idle and the SduId is not active - which we already
     * checked. I interpret the specification as: if the last BAM data frame
     * was already commited to CanIf, which didn't yet confirmed sending. So the
     * last data frame was commited but we cannot say for sure, that the message
     * was entirely sent, because the channel is not yet idle and CanIf didn't
     * confirm the last packet yet.
     */
    if (status == E_OK)
    {
        boolean isBAM =
                (PgInfo->ChannelInfoPtr->ChannelConfPtr->Protocol == J1939TP_PROTOCOL_BAM);

        J1939Tp_Internal_DtPayloadSizeType totalMessageSize =
                PgInfo->ChannelInfoPtr->TxState->TotalMessageSize;
        J1939Tp_Internal_DtPayloadSizeType sentDtCount =
                PgInfo->ChannelInfoPtr->TxState->SentDtCount;

        /* If the next confirmed packet with number sentDtCount + 1 would be
         * the last data packet, then we must be waiting for the confirmation
         * of the last data packet. */
        boolean waitingForLastDtConfirm =
                (sentDtCount + 1) * DT_DATA_SIZE >= totalMessageSize;

        if (isBAM && waitingForLastDtConfirm)
        {
            status = E_NOT_OK;
            /* Cannot cancel transmission after having commited to CanIf the
             * last data packet. */
        }
    }

    /* Here we should check
     * "if the TP.CM_EOMAck frame has already been received during CMDT transmission".
     * However, when we receive the TP.CM_EOMAck message, we set the transmission channel into
     * idle mode and we already have checked this mode.
     * See J1939Tp_Internal_RxIndication_ReverseCm, where we check the command byte to be
     * ENDOFMSGACK_CONTROL_VALUE.
     */


    /** @req SWS_J1939Tp_00048 */

    if (status == E_OK)
    {
        /* According to SWS_J1939Tp_00032 in v4.1, we should use the value NTFRSLT_E_NOT_OK
         * when aborting transmission. NTFRSLT_E_CANCELATION_NOT_OK was used in
         * version 4.0 to indicate that the implementation has chosen to do nothing. The item
         * J1939TP0179 however, which allowed J1939Tp_CancelTransmit to do nothing, has been
         * removed and replaced with SWS_J1939Tp_00203. */

        /** @req SWS_J1939Tp_00032 */
        PduR_J1939TpTxConfirmation(PgInfo->PgConfPtr->NSdu, NTFRSLT_E_NOT_OK);
        //PduR_J1939TpTxConfirmation(PgInfo->PgConfPtr->NSdu, NTFRSLT_E_CANCELATION_NOT_OK);

        /* SWS_J1939Tp_00048 wants us to send connection abort when
         * J1939Tp_CancelTransmit is called.
         * NOTE: In R4.1 Rev 1 there is a bug in SWS_J1939Tp_00048:
         * it is J1939Tp_CancelTransmit (this function) and not J1939Tp_CancelReceive. */
        J1939Tp_ProtocolType proto = PgInfo->ChannelInfoPtr->ChannelConfPtr->Protocol;
        if (proto == J1939TP_PROTOCOL_CMDT)
        {
            /* Get the Parameter Group Number PGN transfered through this PDU */
            J1939Tp_PgnType PGN   = PgInfo->PgConfPtr->Pgn;
            /* Send connection abort message */
            J1939Tp_Internal_SendConnectionAbort
            (PgInfo->ChannelInfoPtr, J1939TP_CM, PGN, CONNABORT_REASON_NO_RESOURCES);
        }

        /* Put the transmission channel into idle mode. */
        PgInfo->ChannelInfoPtr->TxState->State = J1939TP_TX_IDLE;
    }

    return status;
}
#endif /*J1939TP_CANCELLATION_SUPPORT */
/*------------------------------------------------------------------------------------------------*/

/*------------------[Cancel Reception]------------------------------------------------------------*/
/** @req SWS_J1939Tp_00215 */
#if (J1939TP_CANCELLATION_SUPPORT == STD_ON)
/** @req SWS_J1939Tp_00176 */
/**
 * @brief   Cancel the ongoing reception of a J1939Tp N-SDU.
 *
 * This function will fail and return E_NOT_OK if one of the following conditions are met:
 * - The module was not initialized by calling J1939Tp_Init.
 * - RxSduId is not a valid SDU ID.
 * - The channel use to recept the PG associated with the SDU RxSduId is not a reception channel.
 * - The reception state of the channel used to recept the SDU RxSduId is not setup (configuration
 *   or logic error).
 * - The channel used to recept RxSduId is in idle state (not receiving).
 * - The SDU with ID RxSduId is used to recept a direct frame. Direct reception consists of single
 *   frames with no connection management and cannot be canceled after reception.
 * - The received PDU is the last frame received using BAM. The reception cannot be canceled
 *   because there are no following frames to receive.
 *
 * In all other cases this function indication PduR about the canceled reception and if the
 * protocol used to recept the SDU with ID RxSduId is CMDT, it transmits a connection abort message
 * to the sending node.
 *
 * @param   RxSduId ID of the J1939Tp N-SDU whose reception to cancel.
 *
 * @retval  E_OK      Cancelation request accepted.
 * @retval  E_NOT_OK  Cancelation request failed.
 *
 * @see     AUTOSAR specification for J1939Tp module, item SWS_J1939Tp_00176.
 */
Std_ReturnType J1939Tp_CancelReceive(PduIdType RxSduId)
{

    J1939Tp_Internal_PgInfoType* PgInfo = NULL_PTR;

    /* What we return from this function.
     * MISRA C2012 rule 15.5 requieres one single return statement.
     * This is why the following code seems so convoluted. */
    Std_ReturnType status = E_OK;

    if (globalState.State == J1939TP_OFF)
    {
        status = E_NOT_OK;
        /* Module was not initialised */
    }

    /** @req SWS_J1939Tp_00204 */

    /* Get the PG structure corresponding to the Sdu Id RxSduId.
     * If this fails, then RxSduId is an invalid Sdu ID. */

    if (status == E_OK)
    {
        status = J1939Tp_Internal_GetPg(RxSduId, &PgInfo);
    }

    /* Check if RxSduId corresponds to a direct frame:
     * We cannot cancel direct frames because they are single frames and there
     * is no session to cancel.
     * Normally, the generator checks that every SduId is used only once, so
     * RxSduId should not be used as Pdu Id of a direct frame. But we check
     * anyway. We can see that RxSduId is used as index of a parameter group
     * whose Sdu is <= 8 bytes long if for the pointed PG was dynamic length
     * enabled or - in case of an older generator - if the direct Pdu is not
     * -1, which is a symbolic value we use for Pdu and Sdu Ids which were not
     * setup.
     */
    if (status == E_OK)
    {
        if (PgInfo->PgConfPtr->DynLength  == true ||
            PgInfo->PgConfPtr->DirectNPdu != ((PduIdType) -1))
        {
            status = E_NOT_OK;
        }
    }

    /* Check that we are speaking about a reception channel after all. */
    if (status == E_OK)
    {
        if (PgInfo->ChannelInfoPtr->ChannelConfPtr->Direction != J1939TP_RX)
        {
            /* not a reception channel so we cannot cancel reception on a
             * transmission channel */
            status = E_NOT_OK;
        }
        if (! PgInfo->ChannelInfoPtr->RxState)
        {/* The reception pointer is null! */
            status = E_NOT_OK;
        }
    }
    /* OK, the PDU is associated with a reception channel. */

    /* The specification of the J1939Tp module v4.1 (item SWS_J1939Tp_00204)
     * sais that we "shall return E_NOT_OK [...] if RxSduId is currently not
     * active".
     * I interpret this as "if the channel use to transmit the PDU with ID
     * RxSduId is in idle state". */

    if (status == E_OK)
    {
        if (PgInfo->ChannelInfoPtr->RxState->State == J1939TP_RX_IDLE)
        {
            /* channel used for PG with index RxSduId is idle, nothing to
             * cancel. */
            status = E_NOT_OK;
        }
    }

    /* Check if the last TP.DT frame has already been received during BAM
     * reception. */
    if (status == E_OK)
    {
        /* Do we use BAM to receive this PG? */
        boolean isBAM    = (PgInfo->ChannelInfoPtr->ChannelConfPtr->Protocol == J1939TP_PROTOCOL_BAM);
        /* Did we already received the last data packet? */
        boolean isLastDt = J1939Tp_Internal_IsLastDt(PgInfo->ChannelInfoPtr->RxState);
        if (isBAM && isLastDt)
        {
            status = E_NOT_OK;
        }
    }

    /* The next check required by item SWS_J1939Tp_00204 in the Autosar
     * specification v4.1 of the J1939Tp module is to prove if we already sent
     * the TP.CM_EOMAck. However, when we send this message, we also put the
     * channel used to receive the pdu into idle mode and we
     * already have checked this mode. So we can ommit this requirement.
     *
     * See J1939Tp_Internal_RxIndication_Dt, where we call
     * J1939Tp_Internal_SendEndOfMsgAck.
     */

    if (status == E_OK)/* nothing wrong until now */
    {
        /** @req SWS_J1939Tp_00031*/
        PduR_J1939TpRxIndication(PgInfo->PgConfPtr->NSdu, NTFRSLT_E_NOT_OK);

        /** @req SWS_J1939Tp_00040 */

        /* SWS_J1939Tp_00040 specifies a Connection Abort message in case of
         * a CMDT connection. */

        /* Get the protocol used to receive this PG */
        J1939Tp_ProtocolType proto = PgInfo->ChannelInfoPtr->ChannelConfPtr->Protocol;
        if (proto == J1939TP_PROTOCOL_CMDT)
        {
            /* Get the Parameter Group Number PGN transfered through this PDU */
            J1939Tp_PgnType PGN   = PgInfo->PgConfPtr->Pgn;
            /* Send connection abort message */
            J1939Tp_Internal_SendConnectionAbort
            (PgInfo->ChannelInfoPtr, J1939TP_CM, PGN, CONNABORT_REASON_NO_RESOURCES);
        }

        /* Put the channel into idle mode because after cancelation we do not
         * expect data anymore. */
        PgInfo->ChannelInfoPtr->RxState->State = J1939TP_RX_IDLE;
    }

    return status;
}
#endif /* J1939TP_CANCELLATION_SUPPORT */
/*------------------------------------------------------------------------------------------------*/

/*------------------[Transmit Parameter Group]----------------------------------------------------*/

/** @req J1939TP0096 */
/** @req J1939TP0045 */
/**
 * @brief Transmit a J1939Tp N-SDU.
 *
 * @param TxSduId   ID of the SDU to be transmitted.
 * @param TxInfoPtr Pointer to struture containing the data length of the SDU to
 *                  transmit and the metadata information.
 *                  If TxInfoPtr->SduDataPtr is NULL_PTR, then the
 *                  SDU was setup with no metadata. Otherwise it is assumed that
 *                  it contains only the addressing information (metadata) of
 *                  the SDU. The size of this buffer is the setup parameter
 *                  MetaDataLength.
 * @retval E_OK     Transmission request has been accepted.
 * @retval E_NOT_OK Transmission failed.
 */
Std_ReturnType J1939Tp_Transmit(PduIdType TxSduId, const PduInfoType* TxInfoPtr)
{
    Std_ReturnType status               = E_NOT_OK;
    uint8 metadatalength                = 0;

    J1939Tp_Internal_PgInfoType* PgInfo = NULL_PTR;
    const J1939Tp_PgType* Pg            = NULL_PTR;
    uint8 MaxNumPackets                 = RTS_MAX_NUM_PACKETS_VALUE;

    J1939Tp_Internal_ChannelInfoType*   ChannelInfoPtr = NULL_PTR;
    J1939Tp_Internal_TxChannelInfoType* TxState        = NULL_PTR;

    if (globalState.State == J1939TP_ON)
    {
        status = E_OK;
    }
#if (J1939TP_DEV_ERROR_DETECT == STD_ON)
    else
    {
        J1939Tp_Internal_ReportError(J1939TP_TRANSMIT_ID, J1939TP_E_UNINIT);
    }
#endif

    if (status == E_OK)
    {
        /* Get the information about the parameter group to transmit: this
         * contains the PG and the channel. The PG is associated with
         * the Sdu Id.
         * If this call returns E_NOT_OK, then the SduId is not valid. */
        status = J1939Tp_Internal_GetPg(TxSduId, &PgInfo);
    }

    if (status == E_OK)
    {
        /* Sdu Id is valid, decide whether we do direct or segmented
         * transmission based on the data length of the Sdu. */
        if (TxInfoPtr->SduLength <= DIRECT_TRANSMIT_SIZE)
        {
            /* data length is less equal 8 bytes, do direct transmission */
            status = J1939Tp_Internal_DirectTransmit(TxInfoPtr, PgInfo);
        }
        else
        {
            /* Segmented transmission, extract the PG, used channel,
             * transmission state and protocol */
            Pg             = PgInfo         -> PgConfPtr;
            ChannelInfoPtr = PgInfo         -> ChannelInfoPtr;
            TxState        = ChannelInfoPtr -> TxState;
            metadatalength = Pg             -> NSdu_MetaDataLength;

            imask_t state;
            Irq_Save(state); /* save interrupts mask */
            if (TxState->State == J1939TP_TX_IDLE)
            {
                /* Transmission channel is idle */
                /* Setup transmission state */
                TxState->TotalMessageSize = TxInfoPtr->SduLength;
                TxState->PduRPdu          = TxSduId;
                TxState->CurrentPgPtr     = Pg;
                TxState->SentDtCount      = 0;

                /* Set the channel addressing information */
                J1939Tp_Internal_Set_Channel_Metadata(ChannelInfoPtr,
                        TxInfoPtr->SduDataPtr, metadatalength);

                switch (ChannelInfoPtr->Protocol)
                {
                    case J1939TP_PROTOCOL_BAM:
                        TxState->State = J1939TP_TX_WAIT_BAM_CANIF_CONFIRM;
                        J1939Tp_Internal_StartTimer
                            (&(TxState->TimerInfo), J1939TP_TX_CONF_TIMEOUT);
                        status = J1939Tp_Internal_SendBam(ChannelInfoPtr, TxInfoPtr);
                        if (status != E_OK)
                        {
                            /* Cannot sent the BAM message, abort */
                            J1939Tp_Internal_Reset_Channel(ChannelInfoPtr);
                            PduR_J1939TpTxConfirmation
                            (TxState->CurrentPgPtr->NSdu, NTFRSLT_E_NOT_OK);
                        }
                        break;

                    case J1939TP_PROTOCOL_CMDT:
                        TxState->State = J1939TP_TX_WAIT_RTS_CANIF_CONFIRM;

                        /** @req SWS_J1939Tp_00165 */
#if (J1939TP_DYNAMIC_BLOCK_CALCULATION == STD_ON)
/* dynamic block size calculation is enabled */

                        /* Dummy PDU with no data and zero data length used to request
                         * the buffer size of a specific SDU from PduR. */
                        PduInfoType zeroSDU = {
                            .SduDataPtr = NULL_PTR,
                            .SduLength  = 0
                        };

                        /* Available buffer size of the data to be transmitted.
                         * This size is requested from PduR before sending the
                         * RTS message. */
                        PduLengthType     availableBufferSize = 0;

                        /* Request from PduR the buffer size of the SDU we transmit */
                        BufReq_ReturnType size_request_ret =
                            PduR_J1939TpCopyTxData
                            (TxSduId, &zeroSDU, NULL_PTR, &availableBufferSize);

                        if (size_request_ret == BUFREQ_OK)
                        {
#if defined(J1939TP_MAX_PACKETS_PER_BLOCK)
                            /** @req SWS_J1939Tp_00207 */
                            /* "compare the available amount of data returned
                             * by PduR_J1939TpCopyTxData to J1939TpMaxPacketsPerBlock
                             * and use use the smaller of these two values to calculate
                             * the maximum number of packets field of the
                             * TP.CM_RTS message". */
                            MaxNumPackets =
                                J1939TP_Internal_GetNumDtPacketsToSend(availableBufferSize);
                            if (MaxNumPackets > J1939TP_MAX_PACKETS_PER_BLOCK)
                            {
                                MaxNumPackets = J1939TP_MAX_PACKETS_PER_BLOCK;
                            }
#else
                            /** @req SWS_J1939Tp_00208 */
                            MaxNumPackets =
                                J1939TP_Internal_GetNumDtPacketsToSend(availableBufferSize);
#endif
                        }
#else
/* dynamic block size calculation is disabled */
#if defined(J1939TP_MAX_PACKETS_PER_BLOCK)
                        /** @req SWS_J1939Tp_00209 */
                        MaxNumPackets = J1939TP_MAX_PACKETS_PER_BLOCK;
#endif
#endif
                        J1939Tp_Internal_StartTimer
                            (&(TxState->TimerInfo), J1939TP_TX_CONF_TIMEOUT);
                        status = J1939Tp_Internal_SendRts
                            (ChannelInfoPtr, TxInfoPtr, MaxNumPackets);

                        if (status != E_OK)
                        {
                            J1939Tp_Internal_Reset_Channel(ChannelInfoPtr);
                            PduR_J1939TpTxConfirmation
                            (TxState->CurrentPgPtr->NSdu, NTFRSLT_E_NOT_OK);
                        }

                        break;
                    default:
                        status = E_NOT_OK; /* no such protocol */
                        break;
                }
            }
            else
            {
                /* Transmission channel is not idle */
                status = E_NOT_OK;
            }
            Irq_Restore(state); /* restore interrupts mask */
        }
    }
    return status;
}

/*------------------[Direct transmission]---------------------------------------------------------*/
/**
 * @brief Perform direct transmission of SDU.
 *
 * Direct transmission (without segmentation) is performed with PGs having less
 * equal 8 bytes data.
 *
 * @param   TxInfoPtr  SDU to be transmitted. The pointed structure contains the size of the
 *                     transmitted data. The actual data content is requested from PduR.
 * @param   PgInfo     PG information about the SDU to be transmitted.
 *
 * @retval  E_OK
 * @retval  E_NOT_OK
 */
static inline Std_ReturnType J1939Tp_Internal_DirectTransmit
(
    const PduInfoType*           TxInfoPtr,
    J1939Tp_Internal_PgInfoType* PgInfo
)
{
    Std_ReturnType        status         = E_NOT_OK;
    const J1939Tp_PgType* Pg             = PgInfo->PgConfPtr;
    PduIdType             CanIfPdu       = Pg->DirectNPdu;
    PduIdType             NSdu           = Pg->NSdu;
    PduLengthType         remainingBytes = 0;
    imask_t               irq_state      = 0;

    /* check if transmission state is idle (not already sending)
     * for direct frames the transmission state is stored directy in the PgInfo
     * structure, not in the channel */
    Irq_Save(irq_state);
    if (PgInfo->TxState == J1939TP_PG_TX_IDLE)
    {
        /* transmit state is idle, so we can transmit */
        status = E_OK;
    }
    Irq_Restore(irq_state);

    /** @req SWS_J1939Tp_00068 */
    /* Data buffer used to copy the transmitted data from PduR */
    uint8 dataPtr[DIRECT_TRANSMIT_SIZE + METADATALENGTH_MAX] =
    { J1939TP_RESERVED, J1939TP_RESERVED, J1939TP_RESERVED, J1939TP_RESERVED,
      J1939TP_RESERVED, J1939TP_RESERVED, J1939TP_RESERVED, J1939TP_RESERVED
    };

    /* Pdu information we provide to CanIf for transmission.
     * CanIf should use the PduId to recognize if this Pdu has metadata and
     * subtract the metadatalength from the total length we provide.
     * The metadata bytes themselves are provided by PduR itself when we call
     * PduR_J1939TpCopyTxData below. */
    PduInfoType directPduInfo =
    {
        .SduDataPtr = dataPtr,
        .SduLength  = TxInfoPtr->SduLength
    };

    if (status == E_OK)
    {
        /* Get the data from PduR. This may include metadata bytes */
        if (PduR_J1939TpCopyTxData
             (NSdu, &directPduInfo, NULL_PTR, &remainingBytes) != BUFREQ_OK)
        {
            status = E_NOT_OK;
        }
    }

    if (status == E_OK)
    {
        Irq_Save(irq_state);
        /* Change transmission state to waiting for CanIf to confirm
         * transmission */
        PgInfo->TxState = J1939TP_PG_TX_WAIT_DIRECT_SEND_CANIF_CONFIRM;
        Irq_Restore(irq_state);
        /* Timeout if CanIf takes to long */
        J1939Tp_Internal_StartTimer(&(PgInfo->TimerInfo), J1939TP_TX_CONF_TIMEOUT);
        /* Transmit! */
        status = CanIf_Transmit(CanIfPdu, &directPduInfo);
    }

    if (status != E_OK)
    {
        /* CanIf failed */
        Irq_Save(irq_state);
        /* Reset state to idle */
        PgInfo->TxState = J1939TP_PG_TX_IDLE;
        Irq_Restore(irq_state);

        /** @req SWS_J1939Tp_00032 */
        /** @req SWS_J1939Tp_00048 */

        /* Tell PduR that transmission went wrong.
         * The value NTFRSLT_E_NOT_OK is defined in ComStack_Types.h
         * and means message not successfully sent out */
        PduR_J1939TpTxConfirmation(NSdu, NTFRSLT_E_NOT_OK);
    }

    return status;
}
/*------------------------------------------------------------------------------------------------*/

/*------------------[Send BAM message]------------------------------------------------------------*/

/**
 * @brief   Transmit the broadcast announce message (BAM) on the specified
 *          channel and using the specified message size and PGN.
 *
 * @param   ChannelInfoPtr channel used to transmit the BAM message.
 *                         The channel structure is used to access the PGN of
 *                         the transmitted PG and the SDU ID.
 * @param   TxInfoPtr      Pointer to Pdu-structure.
 *                         This is used to extract the message size.
 *
 * @retval  E_OK           message has been successfully transmitted
 * @retval  E_NOT_OK       message could not be transmitted
 */
static inline Std_ReturnType J1939Tp_Internal_SendBam
(
          J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr,
    const PduInfoType*                      TxInfoPtr
)
{
    /* Data of the BAM frame. This is what we send.
     * Its size is the standard size of the BAM message (8 bytes) plus
     * the maximal size of the metadata which we append to the BAM message
     * and which we get from the data buffer of the TxInfoPtr. */
    uint8 cmBamData[BAM_SIZE + METADATALENGTH_MAX] = { 0 };

    /* Mark this data as being a BAM message */
    cmBamData[BAM_BYTE_CONTROL]  = BAM_CONTROL_VALUE;

    /* Get the data length of the SDU to send.
     * We don't touch the actual SDU data, we just get its size and put it into
     * the BAM message. */
    PduLengthType SduLength      = TxInfoPtr->SduLength;

    /* Mask the second byte of the Sdu data length */
    PduLengthType SduLength_2nd_byte = (SduLength & 0xFF00);

    /* Extract the first byte of the SDU length */
    uint8 SduLength_byte1   = (uint8)  (SduLength & 0x00FF);

    /* Extract the second byte of the SDU length */
    uint8 SduLength_byte2   = (uint8)  (SduLength_2nd_byte >> 8);

    /* How many packets we need to send the whole data.
     * This is how many TP.DT frames we send. */
    uint8 num_DT_packets    = J1939TP_Internal_GetNumDtPacketsToSend(SduLength);

    /* Fill in message length in little endian byte order */
    cmBamData[BAM_BYTE_LENGTH_1]    = SduLength_byte1;
    cmBamData[BAM_BYTE_LENGTH_2]    = SduLength_byte2;

    /* Fill in how many packets we are going to send */
    cmBamData[BAM_BYTE_NUM_PACKETS] = num_DT_packets;
    /* This byte is reserved */
    cmBamData[BAM_BYTE_SAE_ASSIGN]  = J1939TP_RESERVED;

    /* Get the PGN we announce. */
    J1939Tp_PgnType pgn = ChannelInfoPtr->TxState->CurrentPgPtr->Pgn;

    /* Pointer to where the PGN is stored in the data segment */
    uint8* pgn_bytes    = &(cmBamData[BAM_BYTE_PGN_1]);

    /* Fill in the PGN */
    J1939Tp_Internal_SetPgn(pgn_bytes, pgn);

    /* PduId of the Connection Management frame used to send the BAM message */
    PduIdType CmNPdu = ChannelInfoPtr->ChannelConfPtr->CmNPdu;

    /* MetaDataLength parameter associated with the Connection Management PDU
     * of the channel */
    uint8 metadatalength =
            J1939Tp_Internal_Get_Pdu_MetaDataLength
            (ChannelInfoPtr->ChannelConfPtr, J1939TP_CM);

    if (TxInfoPtr->SduDataPtr == NULL_PTR)
    {
        /* cannot copy medata because provided data pointer is null */
        metadatalength = 0;
    }

    /* Copy the channel metadata into the data buffer.
     * We take the metadata from the internal channel structure and not from
     * the provided TxInfoPtr->SduDataPtr. */
    J1939Tp_Internal_Copy_Channel_Metadata
        (ChannelInfoPtr, &(cmBamData[BAM_SIZE]), metadatalength);

    /* This PDU Info is what CanIf, the underlying layer, should transmit.
     * Note that we don't pass to CanIf the PDU Info we get as argument because
     * we don't send the PDU data yet, but only announce it. */
    PduInfoType cmBamPdu;

    /* We pass to CanIf the entire data buffer with eventually metadata
     * information. */
    cmBamPdu.SduLength  = BAM_SIZE + metadatalength;
    cmBamPdu.SduDataPtr = cmBamData;

    /* Transmit! */
    Std_ReturnType r = CanIf_Transmit(CmNPdu, &cmBamPdu);
    return r;
}
/*------------------------------------------------------------------------------------------------*/

/*------------------[Extract message size from RTS and BAM message]-------------------------------*/
/**
 * @brief Extract the message size out of a RTS or BAM message.
 *
 * The RTS and BAM messages - both 8 bytes long - contain at indexes 1 and 2
 * the size of the PG to be transmitted. This size is stored in little endian
 * byte order.
 * This function extracts the numerical value of this size.
 *
 * @param   pduInfo Pointer to PDU Info structure holding a pointer to the
 *                  message's data buffer where to extract to size from.
 * @return  Message size in bytes.
 */
static inline uint16 J1939Tp_Internal_GetRtsMessageSize(PduInfoType* pduInfo)
{
    /* Get the two bytes from the message buffer */
    uint16 byte1 = (uint16) ( pduInfo->SduDataPtr[RTS_BYTE_LENGTH_1] );
    uint16 byte2 = (uint16) ( pduInfo->SduDataPtr[RTS_BYTE_LENGTH_2] );

    /* Shift first byte according to little endian order */
    byte2 = byte2 << 8;

    /* Reverse bytes */
    return (byte2 | byte1);
}
/*------------------------------------------------------------------------------------------------*/

static inline boolean J1939Tp_Internal_WaitForCts(J1939Tp_Internal_TxChannelInfoType* TxChannelState) {
    return TxChannelState->SentDtCount == TxChannelState->DtToSendBeforeCtsCount;
}

/*------------------[Compute number of packets]---------------------------------------------------*/
/**
 * @brief Compute the number of data packets to be send based on the total message size.
 *
 * @param  messageSize  The total message size in bytes.
 *                      This number must be in range 0 to 1785, which is the largest amount
 *                      of data that can be transferred with J1939. If this parameter has
 *                      a value larger than 1785, it will be corrected down to 1785.
 *
 * @return              Number of packets to send.
 */
static inline
uint8 J1939TP_Internal_GetNumDtPacketsToSend(PduLengthType messageSize)
{
    /* Correct message size */
    if (messageSize > J1939TP_MAX_DATA_LENGTH)
    {
        messageSize = J1939TP_MAX_DATA_LENGTH;
    }
    uint8 packetsToSend = messageSize/DT_DATA_SIZE;

    if (messageSize % DT_DATA_SIZE != 0) {
        packetsToSend = packetsToSend + 1;
    }

    return packetsToSend;
}
/*------------------------------------------------------------------------------------------------*/


/*------------------[Has the last DT been sent?]--------------------------------------------------*/
/**
 * @brief Returns true is the specified transmission channel state indicates no more packets
 *        to be send.
 *
 * @param  TxChannelState Transmission state, holds the number of already sent packets
 *                        and the total message size.
 *
 * @retval true           The last DT packet has been sent.
 * @retval false          There are more packets to be send.
 */
static inline boolean J1939Tp_Internal_LastDtSent(J1939Tp_Internal_TxChannelInfoType* TxChannelState)
{
    /* Total message size in bytes */
    J1939Tp_Internal_DtPayloadSizeType  totalMessageSize = 0;

    /* How many packets are needed to send the entire message */
    uint8 packetsToSend = 0;

    /* How many packets were sent already */
    uint8 SentDtCount   = 0;

    totalMessageSize = TxChannelState->TotalMessageSize;
    packetsToSend    = J1939TP_Internal_GetNumDtPacketsToSend(totalMessageSize);
    SentDtCount      = TxChannelState->SentDtCount;

    /* We may have erroneously incremented too much the counter SentDtCount,
     * which stores how many packets were confirmed to have been sent,
     * so we check if the sent data counter is greater or equal, not just equal,
     * than the total number of packets to send. */
    return (SentDtCount >= packetsToSend);
}
/*------------------------------------------------------------------------------------------------*/


static inline Std_ReturnType J1939Tp_Internal_ConfGetPg
(PduIdType NSduId, const J1939Tp_PgType* Pg)
{
    if (NSduId < J1939TP_PG_COUNT) {
        Pg = &(J1939Tp_ConfigPtr->Pgs[NSduId]);
        return E_OK;
    } else {
        return E_NOT_OK;
    }
}

static inline J1939Tp_Internal_TimerStatusType J1939Tp_Internal_IncAndCheckTimer
(J1939Tp_Internal_TimerType* TimerInfo)
{
    if (TimerInfo->Timer == 0 && TimerInfo->TimerExpire == 0)
    {
        return J1939TP_NOT_EXPIRED;
    }

    TimerInfo->Timer += J1939TP_MAIN_FUNCTION_PERIOD;

    if (TimerInfo->Timer >= TimerInfo->TimerExpire)
    {
        return J1939TP_EXPIRED;
    }

    return J1939TP_NOT_EXPIRED;
}

/* Returns the Parameter Format of a Parameter Group Number */
static inline uint8 J1939Tp_Internal_GetPf(J1939Tp_PgnType pgn)
{
    return  (uint8)(pgn >> 8);
}


/*------------------[Send Data Packet (DT)]-------------------------------------------------------*/
/**
 * @brief Send data packet.
 *
 * @param ChannelInfoPtr Channel information used to send the packet.
 *                       This contains information about message size, amount of sent packets
 *                       and SDU IDs.
 *
 * @retval E_OK          Data Packet has been sent.
 * @retval E_NOT_OK      Transmission failed.
 *                       Mainly because PduR failed to access the associated data.
 */
static inline Std_ReturnType J1939Tp_Internal_SendDt
(
    J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr
)
{
    /* Return value from this function */
    Std_ReturnType status = E_NOT_OK;

    /* How many SDU data bytes we request from PduR so send with this packet.
     * Maximum is 7 bytes. */
    uint8  requestLength   = DT_DATA_SIZE;

    /* Total message size in bytes */
    J1939Tp_Internal_DtPayloadSizeType
           TotalMessageSize = ChannelInfoPtr->TxState->TotalMessageSize;

    /* How many packets did we send up to this point */
    uint8  SentDtCount = ChannelInfoPtr->TxState->SentDtCount;

    /* How many bytes did we send up to this point */
    J1939Tp_Internal_DtPayloadSizeType sentBytes = SentDtCount * DT_DATA_SIZE;

    /* How many byte we still need to send */
    J1939Tp_Internal_DtPayloadSizeType
        bytesLeftToSend = TotalMessageSize - sentBytes;

    /* If the remaining bytes are less then the maximum number of bytes,
     * do not request the maximum number.
     * This happens when we send the last packet. */
    if (bytesLeftToSend < DT_DATA_SIZE){
        requestLength = bytesLeftToSend;
    }

    /** @req SWS_J1939Tp_00200 */
    /* Prepare TP.DT message.
     * The data message must be always 8 bytes large according to
     * SWS_J1939Tp_00200. Additionally we setup eventuall metadata. */
    uint8 dtBuffer[DT_SIZE + METADATALENGTH_MAX] = {
            J1939TP_RESERVED, J1939TP_RESERVED, J1939TP_RESERVED, J1939TP_RESERVED,
            J1939TP_RESERVED, J1939TP_RESERVED, J1939TP_RESERVED, J1939TP_RESERVED
    };

    /* This struct is what we pass CanIf to transmit.
     * First we adjust the structure for the PduR call:
     *  - we set the data pointer one byte after the actual data begin because the first byte
     *    is used for the sequence number and only after that beginns the actual SDU data
     *    (next 7 bytes) which we get from PduR
     *  - set the SDU length to the number of bytes we transmit with this packet.
     *    we reset this to 8 later
     */
    PduInfoType DtPdu = {
        .SduDataPtr = &(dtBuffer[DT_BYTE_DATA_1]),
        .SduLength = requestLength
    };

    /* SDU ID where to get the data from.
     * This ID tells PduR where to get the date from. */
    PduIdType NSdu = ChannelInfoPtr->TxState->CurrentPgPtr->NSdu;

    /* Here stores PduR the remaining number of bytes being available */
    PduLengthType availableData = 0;

    /* Tell PduR to copy the SDU data into the data buffer. */
    BufReq_ReturnType allocateBufferRes =
            PduR_J1939TpCopyTxData(NSdu, &DtPdu, NULL_PTR, &availableData);

    if (allocateBufferRes == BUFREQ_OK)
    {// successfully got data from upper layers
        /* After PduR copied the data bytes we should send (the next packet)
         * we copy the eventual metadata into the buffer of the Data PDU */
        uint8* metadata      = &(dtBuffer[DT_SIZE]);
        PduIdType DtNPdu = ChannelInfoPtr->ChannelConfPtr->DtNPdu;

        uint8 metadatalength =
                J1939Tp_Internal_Get_Pdu_MetaDataLength
                (ChannelInfoPtr->ChannelConfPtr, J1939TP_DT);
        J1939Tp_Internal_Copy_Channel_Metadata(ChannelInfoPtr, metadata, metadatalength);

        /* Correct PDU information after PduR has copied the SDU data there:
         *  - PDU length is now 8 bytes + metadata, including the sequence
         *    number in the first byte
         *  - PDU data includes the first byte */
        DtPdu.SduLength  = DT_SIZE + metadatalength;
        DtPdu.SduDataPtr = dtBuffer;

        /* Note about the SduLength:
         * SWS_CANIF_00844 specifies: "The SDU length is set to the sum of
           the payload length and MetaDataLength".
           CanIf should therefore use the global MetaDataLength in order to
           compute the actual data length code by subtracting MetaDatLength
           from the SduLength. */

        /* Fill in the sequence number */
        DtPdu.SduDataPtr[DT_BYTE_SEQ_NUM] = ChannelInfoPtr->TxState->SentDtCount+1;

        status = CanIf_Transmit(DtNPdu, &DtPdu);
    }
    else
    {/* PduR failed to provide the data we should transmit */
        status = E_NOT_OK; // we failed
    }

    return status;
}
/*------------------------------------------------------------------------------------------------*/

/*------------------[Send the RTS message]--------------------------------------------------------*/
/**
 * @brief Send the initial RTS message.
 *
 * The Request To Send message initiates a CMDT connection on the sender side.
 * The receiver is expected to send a Clear To Send or Connection Abort message
 * (or time out). After having received the CTS message the data transmission
 * can begin.
 *
 * @param ChannelInfoPtr Channel used to transmit the RTS message. The channel
 *                       is used to read the PduId of the connection management
 *                       frame used to transmit the Sdu and the PGN of this Sdu.
 *                       We assume that the channel was setup before calling
 *                       this function.
 * @param TxInfoPtr      Pdu information containing the total data size of the
 *                       Sdu to transmit and the MetaData at the end of the data
 *                       pointer if the Sdu Id of the Sdu was setup with
 *                       MetaData.
 * @param MaxNumPackets  Maximum  number of packets J1939Tp can send in response
 *                       to the CTS message. This number is part of the RTS
 *                       message.
 *
 * @returns Whatever CanIf_Transmit returns.
 */

static inline Std_ReturnType J1939Tp_Internal_SendRts
(
    J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr,
    const PduInfoType*                TxInfoPtr,
    uint8                             MaxNumPackets
)
{
    /* This array stores the RTS message we transmit */
    uint8 cmRtsData[RTS_SIZE + METADATALENGTH_MAX] = { 0 };

    cmRtsData[RTS_BYTE_CONTROL]  = RTS_CONTROL_VALUE;
    cmRtsData[RTS_BYTE_LENGTH_1] = (uint8)(TxInfoPtr->SduLength);
    cmRtsData[RTS_BYTE_LENGTH_2] = (uint8)(TxInfoPtr->SduLength >> 8);
    cmRtsData[RTS_BYTE_NUM_PACKETS] =
        J1939TP_Internal_GetNumDtPacketsToSend(TxInfoPtr->SduLength);
    cmRtsData[RTS_BYTE_MAX_NUM_PACKETS] = MaxNumPackets;

    J1939Tp_PgnType pgn = ChannelInfoPtr->TxState->CurrentPgPtr->Pgn;
    J1939Tp_Internal_SetPgn(&(cmRtsData[RTS_BYTE_PGN_1]), pgn);

    /* Connection Management Pdu Id used on this channel */
    PduIdType CmNPdu = ChannelInfoPtr->ChannelConfPtr->CmNPdu;
    /* MetaDataLength parameter associated with the connection management Pdu
     * used for RTS transmission */
    uint8 metadatalength =
            J1939Tp_Internal_Get_Pdu_MetaDataLength
            (ChannelInfoPtr->ChannelConfPtr, J1939TP_CM);

    J1939Tp_Internal_Copy_Channel_Metadata
        (ChannelInfoPtr, &(cmRtsData[RTS_SIZE]), metadatalength);

    PduInfoType cmRtsPdu = {
        .SduLength  = RTS_SIZE + metadatalength,
        .SduDataPtr = cmRtsData
    };
    Std_ReturnType r = CanIf_Transmit(CmNPdu, &cmRtsPdu);
    return r;
}

/*------------------[Send End of Message Aknowledge]----------------------------------------------*/

/**
 * @brief Send the End Of Message Aknowledge Message.
 *
 * This message is being sent by the receiving node to the transmitting node to
 * aknowledge the reception of the last data paket when using Connection Mode
 * Data Transfer (CMDT).
 *
 * @param ChannelInfoPtr channel used to send the message.
 *                       This channel is used to read the total size of the
 *                       aknowledged message, the number of the received packets
 *                       and the PGN of the aknowledged PG.
 */
static inline void J1939Tp_Internal_SendEndOfMsgAck
(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr)
{
    /* Get the total message size from the description of the receiving state
     * of the channel. */
    J1939Tp_Internal_DtPayloadSizeType message_size =
         ChannelInfoPtr->RxState->TotalMessageSize;

    /* Extract the two bytes of the total message size.
     * We put these byte into the transmitted message. */
    uint8 message_size_byte1 = (uint8) (message_size >> 0);
    uint8 message_size_byte2 = (uint8) (message_size >> 8);

    /* PDU data buffer to send.
     * This will become the 8 byte data segment of a CAN frame and is the actual
     * message after CanIf has handled the eventual metadata at the end. */
    uint8 endofmsgData[ENDOFMSGACK_SIZE + METADATALENGTH_MAX] = { 0 };

    /* Mark this data buffer as End of Message Aknowledgement */
    endofmsgData[ENDOFMSGACK_BYTE_CONTROL] = ENDOFMSGACK_CONTROL_VALUE;

    /* Fill in the aknowledged message size in little endian byte order */
    endofmsgData[ENDOFMSGACK_BYTE_TOTAL_MSG_SIZE_1] = message_size_byte1;
    endofmsgData[ENDOFMSGACK_BYTE_TOTAL_MSG_SIZE_2] = message_size_byte2;

    /* Fill in the number of packets we received */
    endofmsgData[ENDOFMSGACK_BYTE_NUM_PACKETS] = ChannelInfoPtr->RxState->ReceivedDtCount;

    /* The 5th byte of the message is reserved */
    endofmsgData[ENDOFMSGACK_BYTE_SAE_ASSIGN]  = J1939TP_RESERVED;

    /* Get the PGN of the PG to aknowledge */
    J1939Tp_Internal_PgnType PGN = ChannelInfoPtr->RxState->CurrentPgPtr->Pgn;

    /* Fill in the PGN of the aknolwdged PG */
    J1939Tp_Internal_SetPgn(&(endofmsgData[ENDOFMSGACK_BYTE_PGN_1]), PGN);

    /* Get the PDU ID of the PDU used for Flow Control for this PG. */
    PduIdType FcNPdu = ChannelInfoPtr->ChannelConfPtr->FcNPdu;

    /* Extract the MetaDataLength for the Flow Control Pdu */
    uint8 metadatalength =
                J1939Tp_Internal_Get_Pdu_MetaDataLength
                (ChannelInfoPtr->ChannelConfPtr, J1939TP_REVERSE_CM);

    /* Append the channel's addressing information to the data buffer */
    J1939Tp_Internal_Copy_Channel_Metadata
        (ChannelInfoPtr, &(endofmsgData[ENDOFMSGACK_SIZE]), metadatalength);

    /* The PDU which CanIf should send */
    PduInfoType endofmsgInfo = {
        .SduLength  = ENDOFMSGACK_SIZE + metadatalength,
        .SduDataPtr = endofmsgData
    };

    /* Transmit! */
    CanIf_Transmit(FcNPdu, &endofmsgInfo);
}
/*------------------------------------------------------------------------------------------------*/

/*------------------[Send the CTS message]--------------------------------------------------------*/
/**
 * @brief Send the CTS message.
 *
 * This function transmits the Clear To Send (CTS) flow control message in
 * response to an incoming Request To Send (RTS) message.
 * The CTS message is being sent using a reception channel.
 *
 * @param ChannelInfoPtr     Reception channel used to transmit the message.
 *                           The channel is used to access the PDU ID of the
 *                           flow control PDU.
 * @param Pgn                Parameter Group Number of the Parameter Group to
 *                           send.
 * @param NextPacketSeqNum   Next packet number to request.
 * @param NumPackets         How many packets to request. This number is also
 *                           called the block size.
 */
static inline void J1939Tp_Internal_SendCts
(
    J1939Tp_Internal_ChannelInfoType *ChannelInfoPtr,
    J1939Tp_PgnType                   Pgn,
    uint8                             NextPacketSeqNum,
    uint8                             NumPackets
)
{
    /* Data buffer used to store the CTS message and the eventual metadata. */
    uint8 ctsData[CTS_SIZE + METADATALENGTH_MAX] = { 0 };

    /* Fill in the data of the CTS message */

    /* Mark the data buffer as a CTS message */
    ctsData[CTS_BYTE_CONTROL]      = CTS_CONTROL_VALUE;
    ctsData[CTS_BYTE_NUM_PACKETS]  = NumPackets;
    ctsData[CTS_BYTE_NEXT_PACKET]  = NextPacketSeqNum;
    ctsData[CTS_BYTE_SAE_ASSIGN_1] = J1939TP_RESERVED;
    ctsData[CTS_BYTE_SAE_ASSIGN_2] = J1939TP_RESERVED;
    /* Fill in the PGN */
    J1939Tp_Internal_SetPgn(&(ctsData[CTS_BYTE_PGN_1]), Pgn);

    /* ID of the flow control PDU used to transmit the CTS message */
    const PduIdType FcNPdu = ChannelInfoPtr->ChannelConfPtr->FcNPdu;

    /* Extract the MetaDataLength for the Control Flow Pdu */
    uint8 metadatalength =
            J1939Tp_Internal_Get_Pdu_MetaDataLength
            (ChannelInfoPtr->ChannelConfPtr, J1939TP_REVERSE_CM);


    /* Append the channels addressing information to the metadata buffer */
    J1939Tp_Internal_Copy_Channel_Metadata
        (ChannelInfoPtr, &(ctsData[CTS_SIZE]), metadatalength);

    /* PDU passed to CanIf. The field SduLength contains the MetaDataLength and
     * CanIf must correct this length in order to get the actual payload size.*/
    PduInfoType ctsInfo =
    {
        .SduLength  = CTS_SIZE + metadatalength,
        .SduDataPtr = ctsData
    };

    /* Transmit! */
    CanIf_Transmit(FcNPdu, &ctsInfo);
}
/*------------------------------------------------------------------------------------------------*/

/*------------------[Send Connection Abort Message]-----------------------------------------------*/

/**
 * @brief Send a connection abort message.
 *
 * @param ChannelInfoPtr
 *                Channel using the CmNPdu Pdu Id. The channel is used to
 *                extract the addressing information into the metadata buffer.
 * @param PacketType
 *                Type of packet being sent. This argument is used to extract
 *                the right PduId used to call CanIf. It is necessary because
 *                the Send Abort message is used either as a connection
 *                management message, or as a control flow message. For these
 *                kinds of messages different PduIds are being used: for the
 *                Connection Management message (J1939TP_CM) the CmNPdu is used,
 *                for Flow Control messages (J1939TP_REVERSE_CM), the FcNPdu is
 *                used. This argument is also used to extract the associated
 *                MetaDataLength configuration parameter of the PduId.
 * @param Pgn     Parameter Group Number of the PG whose transmission is being
 *                aborted. This number is used in the connection abort message.
 * @param Reason  Connection Abort reason. Part of the connection abort message.
 *
 * @note  The abort reasons are defined in J1939Tp_Internal_Packets.h
 *
 * @returns nothing
 */
static inline void J1939Tp_Internal_SendConnectionAbort
(
    J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr,
    J1939Tp_RxPduType                 PacketType,
    J1939Tp_PgnType                   Pgn,
    uint8                             Reason
)
{
    /* Data buffer used as message.
     * Make room for the message itself and for eventual metadata bytes. */
    uint8 connAbortData[CONNABORT_SIZE + METADATALENGTH_MAX] = { 0 };

    /* Mark this message as being a connection abort message.
     * This byte is what differentiates the different CMDT messages. */
    connAbortData[CONNABORT_BYTE_CONTROL]      = CONNABORT_CONTROL_VALUE;
    /* Fill in the abort reason. */
    connAbortData[CONNABORT_BYTE_REASON]       = Reason;

    /* Next three bytes are reserved. */
    connAbortData[CONNABORT_BYTE_SAE_ASSIGN_2] = J1939TP_RESERVED;
    connAbortData[CONNABORT_BYTE_SAE_ASSIGN_3] = J1939TP_RESERVED;
    connAbortData[CONNABORT_BYTE_SAE_ASSIGN_4] = J1939TP_RESERVED;

    /* Fill in the PGN of the aborted PG. */
    J1939Tp_Internal_SetPgn(&(connAbortData[CONNABORT_BYTE_PGN_1]), Pgn);

    /* Extract the configuration parameter MetaDataLength of the
     * Connection Management PDU */
    uint8 metadatalength =
            J1939Tp_Internal_Get_Pdu_MetaDataLength
            (ChannelInfoPtr->ChannelConfPtr, PacketType);

    /* Copy the addressing information into the metadata buffer */
    J1939Tp_Internal_Copy_Channel_Metadata
            (ChannelInfoPtr, &(connAbortData[CONNABORT_SIZE]), metadatalength);

    /* Holds pointer to the data buffer to transmit and the buffer length.
     * The buffer length equals the size of the message (payload) plus
     * the metadatalength associated with the Pdu Id we pass to CanIf.
     * This is what we pass to CanIf, which should correct the actual payload
     * size by subtracting the MetaDataLength from the SduLength.*/
    PduInfoType connAbortInfo = {
        .SduLength  = CONNABORT_SIZE + metadatalength,
        .SduDataPtr = connAbortData
    };

    /* Transmit */
    PduIdType PduId = J1939Tp_Internal_Get_Pdu(ChannelInfoPtr->ChannelConfPtr, PacketType);
    CanIf_Transmit(PduId, &connAbortInfo);
}
/*------------------------------------------------------------------------------------------------*/

static inline
void J1939Tp_Internal_StartTimer
(
    J1939Tp_Internal_TimerType* TimerInfo,
    uint16                      TimerExpire
)
{
    TimerInfo->Timer       = 0;
    TimerInfo->TimerExpire = TimerExpire;
}

static inline
void J1939Tp_Internal_StopTimer(J1939Tp_Internal_TimerType* TimerInfo)
{
    TimerInfo->Timer       = 0;
    TimerInfo->TimerExpire = 0;
}


/*------------------[Extract and set the PGN into byte buffer]------------------------------------*/

/**
 * @brief Write PGN into byte buffer.
 *
 * The parameter group number (PGN) is a 18 bits identification number of a
 * J1939 parameter group (PG). This function extracts the 18 bits from the
 * specified PGN and sets the bytes in the buffer PgnBytes accordingly.
 * The byte order is little endian, as specified by the J1939 standard.
 * The 18 bits are internally extended to 24 bits (3 bytes) with the most
 * significant 6 bits set to zero.
 *
 * Example:
 * For the PGN 0x02FBFD, the content of the array PgnBytes will be set
 * to [0xFD, 0xFB, 0x02]. If PGN is for some reason 0xFFFFFFFF, then the array
 * PgnBytes will be set to [0xFF, 0xFF, 0x03], that is the most significant 6
 * bits of the extended 24 bit PGN set to zero.
 *
 * @param PgnBytes Array of at least three bytes of type uint8.
 * @param pgn      Parameter Group Number (PGN)
 */
static inline void J1939Tp_Internal_SetPgn(uint8* PgnBytes, J1939Tp_PgnType pgn)
{
    /* Get the content of the least significant 3 bytes of the PGN.
     * From the third byte from right we extract only 2 bits,
     * because a PGN has only 18 bits. */

    J1939Tp_PgnType pgn_byte1 = pgn & 0x000000FFu; /* first byte        */
    J1939Tp_PgnType pgn_byte2 = pgn & 0x0000FF00u; /* second byte       */
    J1939Tp_PgnType pgn_byte3 = pgn & 0x00030000u; /* 2 LSB of 3rd byte */

    /* Revert the byte order as J1939 specifies little endian byte order */
    PgnBytes[0] = (uint8) (pgn_byte1 >>  0);
    PgnBytes[1] = (uint8) (pgn_byte2 >>  8);
    PgnBytes[2] = (uint8) (pgn_byte3 >> 16);
}

/*----------------------------------------------------------------------------*/

/*------------------[Extract PGN from byte array]-----------------------------*/
/**
 * @brief  Extract the 18 bit PGN value from the three bytes array PgnBytes.
 *
 *
 * @param  PgnBytes Array of 3 bytes storing the PGN in little endian order.
 *                  Because the PGN is a 18 bit value, the 6 most significant
 *                  bits of the last (third) byte are discarded.
 * @return The parameter group number stored in PgnBytes
 *
 * @see J1939Tp_Internal_SetPgn
 */
static inline J1939Tp_PgnType J1939Tp_Internal_GetPgn(uint8 PgnBytes[3])
{
    /* Extract the bytes from the byte array */
    J1939Tp_PgnType byte1 = (J1939Tp_PgnType) PgnBytes[0];
    J1939Tp_PgnType byte2 = (J1939Tp_PgnType) PgnBytes[1];
    J1939Tp_PgnType byte3 = (J1939Tp_PgnType) PgnBytes[2];

    /* Extract only the last 2 bits from the last byte.
     * This byte becomes the first one of the 18 bit PGN */
    byte3 = ( byte3 &  0x03 );

    /* Shift bytes to the right positions */
    byte3 = ( byte3 <<   16 );
    byte2 = ( byte2 <<    8 );

    /* Invert bytes */
    J1939Tp_PgnType pgn = byte3 | byte2 | byte1;

    return pgn;
}
/*----------------------------------------------------------------------------*/


static inline J1939Tp_Internal_PgInfoType* J1939Tp_GetPgInfo(const J1939Tp_PgType* Pg) {
    J1939Tp_Internal_PgInfoType* pgInfoPtr = NULL_PTR;

    if (Pg != NULL_PTR)
    {
        pgInfoPtr = &(pgInfos[Pg - J1939Tp_ConfigPtr->Pgs]);
    }

    return pgInfoPtr;
}

static inline void J1939Tp_Internal_ReportError(uint8 ApiId, uint8 ErrorId) {
#if (CANSM_DEV_ERROR_DETECT == STD_ON)
    Det_ReportError(MODULE_ID_J1939TP, 0, ApiId, ApiId);
#endif
}

/*------------------[Reset channel AI]----------------------------------------*/

/**
 * @brief Reset the channels metadata (addressing information AI).
 *
 * The metadata contains parts of the CanId or the entire CanId used to transfer
 * an SDU or PDU. The metadata is provided at runtime as 0 to 4 bytes appended
 * to the actual data and contains the source address, destination address and
 * priority.
 *
 * This function sets the protocol and addressing information of a channel to
 * their default values. It does not change internal timers.
 */
static inline
void J1939Tp_Internal_Reset_Channel_Metadata
(
    J1939Tp_Internal_ChannelInfoType* channelInfo
)
{
    if (channelInfo != NULL_PTR)
    {
        /* Reset the setup protocol */
        if (channelInfo->ChannelConfPtr != NULL_PTR)
        {
            channelInfo->Protocol = channelInfo->ChannelConfPtr->Protocol;
        }
        channelInfo->SourceAddress      = J1939TP_RESERVED;
        channelInfo->DestinationAddress = J1939TP_RESERVED;
        channelInfo->Priority           = J1939TP_DEFAULT_PRIORITY;
    }
}

/*------------------[Set channel MetaData]------------------------------------*/

/**
 * @brief Set the addressing information (MetaData) of channel according to the
 * provided metadata (addressing information and priority).
 *
 * This functions first resets all channel metadata.
 * It does not modify the channel state (timer etc).
 *
 * @param[out] channelInfo    Channel information at runtime.
 * @param[in]  metadata       Buffer storing the metadata.
 * @param[in]  metadatalength Size of buffer metadata.
 */
static
void J1939Tp_Internal_Set_Channel_Metadata
(
    J1939Tp_Internal_ChannelInfoType* channelInfo,
    uint8 metadata[],
    uint8 metadatalength
)
{
    uint8  destination_address = 0;

    /* First reset the channel metadata so we have a clean channel */
    J1939Tp_Internal_Reset_Channel_Metadata(channelInfo);

    if (channelInfo != NULL_PTR && metadata != NULL_PTR && metadatalength > 0)
    {
        /* Set the source address */
        if (metadatalength > METADATA_SA)
        {
            channelInfo->SourceAddress = metadata[METADATA_SA];
        }

        /* Set the destination address */
        if (metadatalength > METADATA_DA)
        {
            destination_address = metadata[METADATA_DA];
            channelInfo->DestinationAddress = destination_address;
            /** @req SWS_J1939Tp_00039 */
            /* Set dynamic protocol according to the destination address */
            if (destination_address == BAM_DESTINATION_ADDRESS)
            {
                channelInfo->Protocol = J1939TP_PROTOCOL_BAM;
            }
            else
            {
                channelInfo->Protocol = J1939TP_PROTOCOL_CMDT;
            }
        }
        else
        {
            /* No protocol specified, use the setup one */
            channelInfo->Protocol = channelInfo->ChannelConfPtr->Protocol;
        }

        /* Set the priority */
        if (metadatalength > METADATA_PRIORITY)
        {
            channelInfo->Priority = metadata[METADATA_PRIORITY];
        }
    }
}

/*------------------[Copy MetaData into buffer]-------------------------------*/

/**
 * @brief Copy the channel's addressing information into the specified metadata
 * buffer.
 *
 * Depending on the parameter metadatalength, which specifies the size of the
 * metadata buffer, different parts of the addressing information are copied:
 *   - 1: source address
 *   - 2: source address, destination address
 *   - 3: source address, destination address, priority
 *
 * @param channelInfo     Channel where to copy the addressing information from.
 * @param metadata        Buffer where to copy the addressing information.
 * @param metadatalength  Buffer length.
 *
 * @note The parameter metadatalength should be the configuration parameter
 * MetaDataLength.
 */
static inline
void J1939Tp_Internal_Copy_Channel_Metadata
(
    J1939Tp_Internal_ChannelInfoType* channelInfo,
    uint8 metadata[],
    uint8 metadatalength
)
{
    if (channelInfo != NULL_PTR && metadata != NULL_PTR && metadatalength > 0)
    {
        if (metadatalength > METADATA_SA)
        {
            metadata[METADATA_SA] = channelInfo->SourceAddress;
        }

        if (metadatalength > METADATA_DA)
        {
            metadata[METADATA_DA] = channelInfo->DestinationAddress;
        }

        if (metadatalength > METADATA_PRIORITY)
        {
            metadata[METADATA_PRIORITY] = channelInfo->Priority;
        }
    }
}

/*------------------[Reset channel]-------------------------------------------*/

/**
 * @brief Reset the state of a channel.
 *
 * This functions stops the channel timer, sets its state to idle and resets its
 * metadata (addressing information).
 *
 * @param channel Channel to reset.
 */
static inline
void J1939Tp_Internal_Reset_Channel(J1939Tp_Internal_ChannelInfoType * channel)
{
    if (channel != NULL_PTR)
    {
        if (channel->TxState != NULL_PTR)
        {
            channel->TxState->State = J1939TP_TX_IDLE;
            J1939Tp_Internal_StopTimer(&(channel->TxState->TimerInfo));
        }

        if (channel->RxState != NULL_PTR)
        {
            channel->RxState->State = J1939TP_RX_IDLE;
            J1939Tp_Internal_StopTimer(&(channel->RxState->TimerInfo));
        }
        J1939Tp_Internal_Reset_Channel_Metadata(channel);
    }
}

/*------------------[Metadata matches channel AI]-----------------------------*/

/**
 * @brief Returns true if the specified metadata content equals the addressing
 * information of the specified channel.
 *
 * Depending on the parameter metadatalength, both source and destination
 * addresses or only source address or no address are compared with the channels
 * address fields. If metadatalength is zero, the functions returns true.
 *
 * If the specified packet type is flow control (reverse CM), then the source
 * address in the metadata buffer is compared with the destination address of
 * the channel and the destination address in the metadata buffer is compared
 * with the source address of the channel. This is because the flow control
 * messages are received on transmission channels and their addressing
 * information is reversed.
 *
 * The layout of the addressing information in the buffer metadata is specified
 * in item SWS_J1939Tp_00198 of the J1939Tp specification. See also figure in
 * J1939Tp_Internal_Packets.h.
 *
 * @param metadata        Buffer containing metadata (addressing information).
 * @param metadatalength  Size in bytes of the buffer metadata.
 * @param packetType      Packet type of the received PDU.
 * @param channel         Channel whose addressing information to compare.
 *
 * @retval true   The channels addressing information (source and destination
 *                address) matches the one specified in buffer metadata. If the
 *                parameter metadatalength is zero, the the function also
 *                returns true.
 * @retval false  Source address or destination address does not match.
 */
static
boolean J1939Tp_Internal_Metadata_Matches_Channel
(
    uint8                             metadata[],
    uint8                             metadatalength,
    J1939Tp_RxPduType                 packetType,
    J1939Tp_Internal_ChannelInfoType* channel
)
{
    boolean match = true;
    uint8 source_address      = 0;
    uint8 destination_address = 0;

    if (metadatalength > METADATA_SA)
    {
        source_address = metadata[METADATA_SA];
        if (packetType == J1939TP_REVERSE_CM)
        {
            /* Control flow messages are received on transmission channels
             * so we have to check if their source address matches the
             * destination address of the channel. */
            match = match &&
                (source_address == channel->DestinationAddress);
        }
        else
        {
            match = match &&
                (source_address == channel->SourceAddress);
        }
    }

    if (metadatalength > METADATA_DA)
    {
        destination_address = metadata[METADATA_DA];
        if (packetType == J1939TP_REVERSE_CM)
        {
            match = match &&
                (destination_address == channel->SourceAddress);
        }
        else
        {
            match = match &&
                (destination_address == channel->DestinationAddress);
        }
    }

    return match;
}

/*------------------[Filter incoming packets]---------------------------------*/

/**
 * @brief Indicate if a Pdu containing metadata should be further processed or
 * if it should be ignored.
 *
 * This function analyses the metadata bytes appended to the actual data of the
 * Pdu and checks if the pdu should be ignored or not.
 * The metadata is supposed to begin with the 9th data byte and have the size
 * <MetaDataLength> (setup at configuration time).
 *
 * For direct frames, pdus with no metadata setup and if the pdu length is
 * less than 9 (8 bytes data + 1 byte metadata), this functions returns E_OK.
 * If the pdu type is Connection Management (TP.CM) and the first data byte
 * indicates a BAM or RTS message (which initiate a new reception session), the
 * channels addressing information is setup and E_OK returned. Otherwise the
 * appended metadata is checked against the channel's addressing information and
 * if a missmatch is found, E_NOT_OK is returned.
 *
 * If the configuration parameter MetaDataLength is larger than the actual
 * number of appended bytes, then the number of appended bytes is used as
 * MetaDataLength. This allows dynamic metadatalength.
 * Note: this behaviour is not specified but seems reasonable.
 *
 * @param RxPduId    ID of the Pdu which has been received. This ID is used to
 *                   extract the configuration parameter MetaDataLength.
 * @param PduInfoPtr Pdu structure containing the Pdu length and a pointer to
 *                   the received data. The data may contain metadata
 *                   information. How large is the metadata is specified by the
 *                   configuration parameter MetaDataLength of the Pdu.
 * @param packetType Packet type of the received Pdu (CM, DT, direct or reverse
 *                   CM).
 * @param channel    Channel where the Pdu was received. The channel is used to
 *                   compare its addressing information with the one contained
 *                   in PduInfoPtr->SduDataPtr.
 *
 * @retval E_OK      The received Pdu should be further processed.
 * @retval E_NOT_OK  The received Pdu should be ignored because the previously
 *                   specified metadata does not match the current addressing
 *                   information of the channel or the extracted configuration
 *                   parameter MetaDataLength is larger than the one indicated
 *                   by the pdu length (PduInfoPtr->SduLength - 8). This
 *                   indicated a missconfigured module.
 */
static
Std_ReturnType J1939Tp_Internal_Filter_RxPdu_Metadata
(
    PduIdType                         RxPduId,
    PduInfoType*                      PduInfoPtr,
    J1939Tp_RxPduType                 packetType,
    J1939Tp_Internal_ChannelInfoType* channel
)
{
    Std_ReturnType ret            = E_NOT_OK;
    uint8          appended_bytes = 0;
    uint8*         metadata       = NULL_PTR;
    boolean        metadata_match = FALSE;
    uint8          command        = 0;
    uint8          metadatalength = J1939Tp_Internal_Get_Pdu_MetaDataLength
                                    (channel->ChannelConfPtr, packetType);


    if ((metadatalength == 0)            || /* no metadata */
        (packetType == J1939TP_DIRECT)   || /* pdu setup as direct frame */
        (PduInfoPtr->SduLength <= DIRECT_TRANSMIT_SIZE)
                                            /* which also means no metadata */
       )
    {
        /* It is ok to receive the pdu because there is no metadata to check or
         * a direct frame has been received which is not segmented and not
         * subject to any control. */
        ret = E_OK;
    }
    else
    {
        /* how many bytes are actually appended to the data */
        appended_bytes = PduInfoPtr->SduLength - DIRECT_TRANSMIT_SIZE;
        /* if the pdu has less metadata bytes then configured, use this smaller
         * value */
        metadatalength = MIN(appended_bytes, metadatalength);

        /* For TP frames, metadata is appended after the actual data */
        metadata = &(PduInfoPtr->SduDataPtr[CM_SIZE]);
        /* The command byte is used used to identify differet connection
         * management messages */
        command  = PduInfoPtr->SduDataPtr[CM_BYTE_CONTROL];

        /* Check if the metadata matches the channel addressing information */
        metadata_match = J1939Tp_Internal_Metadata_Matches_Channel
            (metadata, metadatalength, packetType, channel);

        switch (packetType)
        {
            /* Control Management frame */
            case J1939TP_CM:
                if ((command == BAM_CONTROL_VALUE) ||
                    (command == RTS_CONTROL_VALUE))
                {
                    /* RTS and BAM initiate a new connection, so copy the
                     * addressing information from the metadata into the
                     * channel. Based on this information we will filter
                     * further messages. */
                    J1939Tp_Internal_Set_Channel_Metadata
                        (channel, metadata, metadatalength);
                    ret = E_OK;
                }
                else
                {
                    /* CM packets are Connection Abort messages from sender
                     * side if they are not BAM or RTS. Check their
                     * addressing information against the one previously saved.*/
                    if (metadata_match)
                    {
                        ret = E_OK;
                    }
                }
                break;
            /* Data Packet and reverse CM*/
            case J1939TP_DT:
            case J1939TP_REVERSE_CM:
                /* Data and reverse connection management frames (CTS,
                 * EndOfMsgAck, ConnAbort on receiver side) must match
                 * previously setup addressing information. */
                if (metadata_match)
                {
                    ret = E_OK;
                }
                break;
            /* No such type */
            default:
                ret = E_NOT_OK;
                break;
        }
    }

    return ret;
}

/*------------------[Extract MetaDataLength of PduId from channel]------------*/

/**
 * @brief  Extract the configuration parameter MetaDataLength of a specific PDU
 *         based on the packet type transferred through the channel.
 *
 * Based on the PacketType argument, this functions reads from the channel
 * configuration ChannelConfPtr the value of the MetaDataLength parameter of the
 * Connection Management, Data transfer or Flow Control PDU of that channel.
 * The dependency to the packet type is the following:
 *
 *   J1939TP_CM         - CmNPdu (Connection Management PDU)
 *   J1939TP_DT         - DtNPdu (Data transfer PDU)
 *   J1939TP_REVERSE_CM - FcNPdu (Flow control PDU)
 *   default            - 0 (no metadata)
 *
 * @param  ChannelConfPtr Channel configuration.
 * @param  PacketType     Packet type.
 * @return MetaDataLength of the CmNPdu, DtNPdu, FcNPdu PDU of the channel or
 *         zero if the packet type is J1939TP_DIRECT.
 */
static inline
uint8 J1939Tp_Internal_Get_Pdu_MetaDataLength
(
    const J1939Tp_ChannelType* ChannelConfPtr,
    J1939Tp_RxPduType          PacketType
)
{
    uint8 metadatalength = 0;

    switch (PacketType)
    {
        /* Connection Management frame */
        case J1939TP_CM:
            metadatalength = ChannelConfPtr->CmNPdu_MetaDataLength;
            break;
        /* Data transfer frame */
        case J1939TP_DT:
            metadatalength = ChannelConfPtr->DtNPdu_MetaDataLength;
            break;
        /* Flow control frame */
        case J1939TP_REVERSE_CM:
            metadatalength = ChannelConfPtr->FcNPdu_MetaDataLength;
            break;
        /* Default: no metadatalength */
        default:
            metadatalength = 0;
            break;
    }

    metadatalength = MIN(metadatalength, METADATALENGTH_MAX);
    return metadatalength;
}

/*------------------[Extract PduId from channel]------------------------------*/

/**
 * @brief Extract the PduId used to transmit a packet based on the packet type.
 *
 * The different kinds of packets are being sent using different PduIds which
 * are setup for the channel during configuration. Control Management packets
 * are sent using the CmNPdu PduId, Control Flow messages are sent using the
 * FcNPdu PduId and Data transfer messages are being sent using the DtNPdu
 * PduId.
 *
 * This function inspects the packet type argument PacketType and returns the
 * appropriate PduId stored in the channel configuration at the address
 * ChannelConfPtr.
 *
 * @param  ChannelConfPtr Channel configuration storing the setup PduIds.
 * @param  PacketType     Packet type being used. The value J1939TP_DIRECT is
 *                        ignored because the PduIds for direct frames are
 *                        stored in the Parameter Group configuration structure
 *                        and not in the channel.
 *
 * @return Appropriate PduId for the packet type.
 */
static inline
PduIdType J1939Tp_Internal_Get_Pdu
(
    const J1939Tp_ChannelType* ChannelConfPtr,
    J1939Tp_RxPduType          PacketType
)
{
    PduIdType pdu = 0;

    switch (PacketType)
    {
        case J1939TP_CM:
            pdu = ChannelConfPtr->CmNPdu;
            break;
        case J1939TP_DT:
            pdu = ChannelConfPtr->DtNPdu;
            break;
        case J1939TP_REVERSE_CM:
            pdu = ChannelConfPtr->FcNPdu;
            break;
        default:
            break;
    }

    return pdu;
}
