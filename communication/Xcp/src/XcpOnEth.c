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

/** @req 4.1.3/SWS_Xcp_00733 *//* Support Eth communication*/
/** @req 4.1.3/SWS_Xcp_00734 *//* PDUs have to be transmitted and received using the transmitting and receive APIs provided
 * by the AUTOSAR Socket Adaptor*/
/** @req 4.1.3/SWS_Xcp_00739 *//* The header and tail of an XCP on Ethernet message have to be set properly */

#include "Xcp_Internal.h"

#if (XCP_PROTOCOL == XCP_PROTOCOL_ETHERNET)

#include "XcpOnEth_Cfg.h"
#include "ComStack_Types.h"
#include "SoAd.h"

#define XCP_ETH_FRAME_LEN_IDX  0u
#define XCP_ETH_FRAME_CNTR_IDX 2u
#define XCP_ETH_FRAME_PID_IDX  4u

static uint16 Xcp_EthCtrTx = 0;

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

/**
 * This function initializes the communication specific Xcp layer
 */
void Xcp_SoAdLayerInit(void)
{
    for (uint8 idx = 0; idx < XCP_TX_CH_NUMBER; idx++) {
        Xcp_FreeUpChannel(idx);
    }
}

void Xcp_CancelTxRequests( void ) {
	Xcp_SoAdLayerInit();
}

/**
 * Receive callback from Eth network layer
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
void Xcp_SoAdIfRxIndication(PduIdType XcpRxPduId, PduInfoType* XcpRxPduPtr)
{
    uint16 length;

    DET_VALIDATE_NRV(Xcp_Inited , XCP_API_ID_XCP_RX_INDICATION, XCP_E_NOT_INITIALIZED);
    DET_VALIDATE_NRV(XcpRxPduPtr, XCP_API_ID_XCP_RX_INDICATION, XCP_E_INV_POINTER);
    DET_VALIDATE_NRV(XcpRxPduId < XCP_RX_CH_NUMBER, XCP_API_ID_XCP_RX_INDICATION, XCP_E_INVALID_PDUID);
    DET_VALIDATE_NRV(XcpRxPduPtr->SduLength > 4u, XCP_API_ID_XCP_RX_INDICATION, XCP_E_INVALID_PDUID);

    length = GET_UINT16(XcpRxPduPtr->SduDataPtr, XCP_ETH_FRAME_LEN_IDX);

    if (TRUE == xcpPtr->XcpRecievePduAllowed(XcpRxPduId, XcpRxPduPtr)) {
        Xcp_RxIndication(XcpRxPduPtr->SduDataPtr + XCP_ETH_FRAME_PID_IDX, length);

        // Reset message counter on CONNECT command
        if (XcpRxPduPtr->SduDataPtr[4] == 0xFF) {  // 0xFF is CONNECT command
        	Xcp_EthCtrTx = 0;
        }
    }
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
void Xcp_SoAdIfTxConfirmation(PduIdType XcpTxPduId)
{
    DET_VALIDATE_NRV(Xcp_Inited,
                     XCP_API_ID_XCP_TX_CONFIRMATION,
                     XCP_E_NOT_INITIALIZED);

    DET_VALIDATE_NRV(XcpTxPduId < XCP_TX_CH_NUMBER ,
                     XCP_API_ID_XCP_TX_CONFIRMATION,
                     XCP_E_INVALID_PDUID);

    //XcpTxPduId is expected to be configured starting from 0 in a consecutive order
    Xcp_FreeUpChannel((uint8)XcpTxPduId);
    Xcp_TxConfirmation();
}

/**
 * Called by core Xcp to transmit data
 * @param data
 * @param len
 * @return
 */
Std_ReturnType Xcp_Transmit(const uint8* data, PduLengthType len)
{
    Std_ReturnType retVal = E_NOT_OK;
    PduIdType pduId, pduId2;
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
        PduInfoType pdu;

        SET_UINT16(xcpPtr->XcpTxCh[idx].data, 0,  len);
        SET_UINT16(xcpPtr->XcpTxCh[idx].data, 2,  ++Xcp_EthCtrTx);
        Xcp_MemCpy(xcpPtr->XcpTxCh[idx].data + 4, data, len);
        xcpPtr->XcpTxCh[idx].len = len + 4;

        pdu.SduDataPtr = (uint8 *)xcpPtr->XcpTxCh[idx].data;
        pdu.SduLength = xcpPtr->XcpTxCh[idx].len;

        retVal = SoAd_IfTransmit(pduId, &pdu);

        if (E_OK == retVal) {
            //If there is free channel, available, indicate the next message handling
            if (Xcp_FindFreeChannel(&idx2, &pduId2)) {
                Xcp_TxConfirmation();
            }
        } else {
            //Channel with certain pdu is not available
            Xcp_FreeUpChannel(idx);
        }
    }

    return retVal;
}

/**
 * Called when the core of xcp have received a transport layer command
 * @param pid
 * @param data
 * @param len
 * @return
 */
Std_ReturnType Xcp_CmdTransportLayer(uint8 pid, uint8* data, PduLengthType len)
{
    Xcp_TxError(XCP_ERR_CMD_UNKNOWN);
    return E_OK;
}


#endif /*XCP_PROTOCOL == XCP_PROTOCOL_ETHERNET*/
