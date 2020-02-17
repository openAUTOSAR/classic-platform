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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.3 */

/** @req 4.1.3/SWS_Xcp_00713 *//*Support Can communication*/
/** @req 4.1.3/SWS_Xcp_00714 *//*Use API provided for CanIf*/
/** @req 4.1.3/SWS_Xcp_00716 *//*Performance information must be exchanged between Slave and Master*/
/** @req 4.1.3/SWS_Xcp_00718 *//*Support GET_SLAVE_ID command*/
/** @req 4.1.3/SWS_Xcp_00715 *//*2 can Pdu needed*/

#include "Xcp_Internal.h"

#if (XCP_PROTOCOL == XCP_PROTOCOL_CAN)

#include "XcpOnCan_Cfg.h"
#include "CanIf.h"

typedef enum {
    XCP_CAN_CMD_SET_DAQ_ID   = 0xFD,
    XCP_CAN_CMD_GET_DAQ_ID   = 0xFE,
    XCP_CAN_CMD_GET_SLAVE_ID = 0xFF,
} Xcp_CanCmdType;

/**
 * This function is responsible to find an unused buffer/Pdu
 * @param idxPtr
 * @param pduIdPtr
 * @return FALSE: There is no such Pdu
 *         TRUE: Empty buffer/Pdu has been found
 */
static boolean Xcp_FindFreeChannel(uint8* idxPtr, PduIdType* pduIdPtr)
{
    uint8 idx;

    for (idx = 0; idx < XCP_TX_CH_NUMBER; idx++) {
        if ( (XCP_TX_CH_EMPTY == xcpPtr->XcpTxCh[idx].state)                 &&
             (TRUE == xcpPtr->XcpTransmitPduAllowed(xcpPtr->XcpTxCh[idx].pduId))  ) {
            *idxPtr = idx;
            *pduIdPtr = xcpPtr->XcpTxCh[idx].pduId;
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * This function reserves buffer/Pdu which was found by Xcp_FindFreeChannel
 * previously
 * @param idxPtr
 * @param pduIdPtr
 * @return FALSE: There is no such Pdu
 *         TRUE: buffer/Pdu has been reserved
 */
static boolean Xcp_ReserveChannel(uint8* idxPtr, PduIdType* pduIdPtr)
{
    if (Xcp_FindFreeChannel(idxPtr,pduIdPtr)) {
        xcpPtr->XcpTxCh[*idxPtr].state = XCP_TX_CH_RESERVED;
        return TRUE;
    }

    return FALSE;
}

/**
 * This function frees up the previously reserves buffer/Pdu
 * @param idx
 */
static void Xcp_FreeUpChannel(uint8 idx)
{
    xcpPtr->XcpTxCh[idx].state = XCP_TX_CH_EMPTY;
    xcpPtr->XcpTxCh[idx].len = 0;
}

void Xcp_CancelTxRequests() {
	Xcp_CanLayerInit();
}

/**
 * This function initializes the communication specific Xcp layer
 */
void Xcp_CanLayerInit(void)
{
    for (uint8 idx = 0; idx < XCP_TX_CH_NUMBER; idx++) {
        Xcp_FreeUpChannel(idx);
    }
}

/**
 * Transport protocol agnostic transmit function called by Xcp core system
 *
 * @param data
 * @param len
 * @return
 * @info This function is designed to be reentrant, thus
 *       it works on static data with exclusive area protection.
 */
Std_ReturnType Xcp_Transmit(const uint8* data, PduLengthType len)
{
    Std_ReturnType retVal = E_NOT_OK;
    PduIdType pduId, pduId2;
    PduInfoType pdu;
    uint8 idx, idx2;
    boolean freeChFound = FALSE;

    /*-------------------------------------------*/
    SchM_Enter_Xcp_EA_0(); /*Enter exclusive area*/
    /*-------------------------------------------*/

    freeChFound = Xcp_ReserveChannel(&idx, &pduId);

    /*-------------------------------------------*/
    SchM_Exit_Xcp_EA_0(); /*Exit exclusive area*/
    /*-------------------------------------------*/

    if (TRUE == freeChFound) {
        //Free slot has been found, copy and try to send
        Xcp_MemCpy(xcpPtr->XcpTxCh[idx].data, data, len);
        xcpPtr->XcpTxCh[idx].len = len;
        pdu.SduDataPtr = (uint8 *)xcpPtr->XcpTxCh[idx].data;
        pdu.SduLength  = xcpPtr->XcpTxCh[idx].len;

        retVal = CanIf_Transmit(pduId, &pdu);

        if (E_OK == retVal) {
            //If there is free channel, available, indicate the next message handling
            if (Xcp_FindFreeChannel(&idx2, &pduId2)) {
                Xcp_TxConfirmation();
            }
        } else {
            //Report to Det that the message is lost
            DET_REPORTERROR(XCP_API_ID_XCP_TRANSMIT,XCP_E_MESSAGE_LOST);
            //Channel with certain pdu is not available
            Xcp_FreeUpChannel(idx);
        }
    }

    return retVal;
}

/**
 * Callback for finished transmit of PDU
 *
 * This function is called by the lower layers (i.e. FlexRay Interface, TTCAN Interface
 * and Socket Adaptor or CDD) when an AUTOSAR XCP PDU has been transmitted
 *
 * Reentrant for different XcpTxPduIds, non reentrant for the same XcpTxPduId
 *
 * ServiceId: 0x02
 *
 * @param XcpRxPduId PDU-ID that has been transmitted
 */
/** @req 4.1.3/SWS_Xcp_00814 *//*Xcp_<module>TxConfirmation API definition*/
/** @req 4.1.3/SWS_Xcp_00840 *//*Send DET if the function Xcp_<module>TxConfirmation is called before the XCP was initialized successfully.*/
/** @req 4.1.3/SWS_Xcp_00841 *//*The call context is either on interrupt level (interrupt mode) or on task level, The Xcp module is initialized correctly. - can not be tested with conventional module tests*/
void Xcp_CanIfTxConfirmation(PduIdType XcpTxPduId)
{

    DET_VALIDATE_NRV(Xcp_Inited,
                     XCP_API_ID_XCP_TX_CONFIRMATION,
                     XCP_E_NOT_INITIALIZED);

    DET_VALIDATE_NRV(XcpTxPduId < XCP_TX_CH_NUMBER,
                     XCP_API_ID_XCP_TX_CONFIRMATION,
                     XCP_E_INVALID_PDUID);

    //XcpTxPduId is expected to be configured starting from 0 in a consecutive order
    Xcp_FreeUpChannel((uint8)XcpTxPduId);
    Xcp_TxConfirmation();
}

/**
 * Receive callback from CAN network layer
 *
 * This function is called by the lower layers (i.e. FlexRay Interface, TTCAN Interface
 * and Socket Adaptor or CDD) when an AUTOSAR XCP PDU has been received
 *
 * Reentrant for different XcpRxPduIds,
 * non reentrant for the same XcpRxPduId
 *
 * The function Xcp_<module>RxIndication might be called
 * by the Xcp module�s environment in an interrupt context
 *
 * ServiceId: 0x03
 *
 * @param XcpRxPduId  PDU-ID that has been received
 * @param XcpRxPduPtr Pointer to SDU (Buffer of received payload)
 */
/** @req 4.1.3/SWS_Xcp_00813 *//*Xcp_<module>RxIndication API definition*/
/** @req 4.1.3/SWS_Xcp_00847 *//*Send DET if Xcp was not initialized, XcpRxPduPtr equals NULL_PTR, Invalid PDUID*/
void Xcp_CanIfRxIndication(PduIdType XcpRxPduId, PduInfoType* XcpRxPduPtr)
{
    DET_VALIDATE_NRV(Xcp_Inited,  XCP_API_ID_XCP_RX_INDICATION, XCP_E_NOT_INITIALIZED);
    DET_VALIDATE_NRV(XcpRxPduPtr, XCP_API_ID_XCP_RX_INDICATION, XCP_E_INV_POINTER);

#if (XCP_FEATURE_GET_SLAVE_ID == STD_ON)
    DET_VALIDATE_NRV((XcpRxPduId < XCP_RX_CH_NUMBER) ||
                     (XcpRxPduId == XCP_PDU_ID_BROADCAST),
                     XCP_API_ID_XCP_RX_INDICATION,
                     XCP_E_INVALID_PDUID);
#else
    DET_VALIDATE_NRV(XcpRxPduId < XCP_RX_CH_NUMBER,
                     XCP_API_ID_XCP_RX_INDICATION,
                     XCP_E_INVALID_PDUID);

#endif

    if (TRUE == xcpPtr->XcpRecievePduAllowed(XcpRxPduId, XcpRxPduPtr)) {
        Xcp_RxIndication(XcpRxPduPtr->SduDataPtr, XcpRxPduPtr->SduLength);
    }
}


/**
 * Command that can be used for a master to discover
 * all connected XCP slaves on a CAN bus
 * @param data
 * @param len
 * @return
 */
#if(XCP_FEATURE_GET_SLAVE_ID == STD_ON)
static Std_ReturnType Xcp_CmdGetSlaveId(uint8* data, PduLengthType len)
{
    uint8 p[4];
    uint8 mode;

    if(len < 4) {
        RETURN_ERROR(XCP_ERR_CMD_SYNTAX, "Invalid length for get_slave_id %d", len);
    }

    p[0] = GET_UINT8(data, 0);
    p[1] = GET_UINT8(data, 1);
    p[2] = GET_UINT8(data, 2);
    p[3] = 0;
    mode = GET_UINT8(data, 3);

    if(strcmp((char*)p, "XCP")) {
        RETURN_ERROR(XCP_ERR_CMD_UNKNOWN, "Unknown get_id pattern %s", p);
    }

    if(mode == 0) {
        FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
            FIFO_ADD_U8 (e, XCP_PID_RES);
            FIFO_ADD_U8 (e, p[0]);
            FIFO_ADD_U8 (e, p[1]);
            FIFO_ADD_U8 (e, p[2]);
            FIFO_ADD_U32(e, XCP_CAN_ID_RX);
        }
    } else if(mode == 1) {
        FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
            FIFO_ADD_U8 (e, XCP_PID_RES);
            FIFO_ADD_U8 (e, ~p[0]);
            FIFO_ADD_U8 (e, ~p[1]);
            FIFO_ADD_U8 (e, ~p[2]);
            FIFO_ADD_U32(e, XCP_CAN_ID_RX);
        }
    } else {
        RETURN_ERROR(XCP_ERR_CMD_UNKNOWN, "Invalid mode for get_slave_id: %u", mode);
    }

    return E_OK;
}
#endif /*XCP_FEATURE_GET_SLAVE_ID == STD_ON*/

/**
 * Called when the core of xcp have received a transport layer command
 * @param pid
 * @param data
 * @param len
 * @return
 */
Std_ReturnType Xcp_CmdTransportLayer(uint8 pid, uint8* data, PduLengthType len)
{
#if(XCP_FEATURE_GET_SLAVE_ID == STD_ON)
    Xcp_CanCmdType id = (Xcp_CanCmdType)GET_UINT8(data, 0);
    XCP_UNUSED(pid);

    if(id == XCP_CAN_CMD_GET_SLAVE_ID && len >= 2) {
        return Xcp_CmdGetSlaveId((void*)((uint8*)data+1), len-1); /*lint !e904 allow multiple exit */
    }
#else /*XCP_FEATURE_GET_SLAVE_ID == STD_ON*/
    XCP_UNUSED(pid);
    XCP_UNUSED(data); /*lint !e920  no issue this is dummy use */
	XCP_UNUSED(len);
#endif

    RETURN_ERROR(XCP_ERR_CMD_UNKNOWN, "Unknown transport cmd");
}

#endif /*XCP_PROTOCOL == XCP_PROTOCOL_CAN*/
