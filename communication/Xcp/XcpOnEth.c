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

#include "Xcp.h"

#if !(XCP_PROTOCOL != XCP_PROTOCOL_ETHERNET)

#include "XcpOnEth_Cfg.h"
#include "Xcp_Internal.h"
#include "ComStack_Types.h"
#include "SoAdIf.h"

static uint16_t Xcp_EthCtrRx = 0;
static uint16_t Xcp_EthCtrTx = 0;


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
/** @req XCP813 *//*Xcp_<module>RxIndication API definition*/
/** @req XCP847 *//*Send DET if Xcp was not initialized, XcpRxPduPtr equals NULL_PTR, Invalid PDUID*/
void Xcp_SoAdRxIndication   (PduIdType XcpRxPduId, PduInfoType* XcpRxPduPtr)
{
    DET_VALIDATE_NRV(Xcp_Inited                    , 0x03, XCP_E_NOT_INITIALIZED);
    DET_VALIDATE_NRV(XcpRxPduPtr                   , 0x03, XCP_E_INV_POINTER);
    DET_VALIDATE_NRV(XcpRxPduId == XCP_PDU_ID_RX   , 0x03, XCP_E_INVALID_PDUID);
    DET_VALIDATE_NRV(XcpRxPduPtr->SduLength > 4    , 0x03, XCP_E_INVALID_PDUID);

    uint16 ctr = (XcpRxPduPtr->SduDataPtr[3] << 8) | XcpRxPduPtr->SduDataPtr[2];
    if(Xcp_Connected && ctr && ctr != Xcp_EthCtrRx) {
        ASLOG(HIGH, ("Xcp_SoAdRxIndication - ctr:%d differs from expected: %d\n",  ctr, Xcp_EthCtrRx));
    }

    Xcp_EthCtrRx = ctr+1;
    Xcp_RxIndication(XcpRxPduPtr->SduDataPtr+4, XcpRxPduPtr->SduLength-4);
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
/** @req XCP814 *//*Xcp_<module>TxConfirmation API definition*/
/** @req XCP840 *//*Send DET if the function Xcp_<module>TxConfirmation is called before the XCP was initialized successfully.*/
/** @req XCP841 *//*The call context is either on interrupt level (interrupt mode) or on task level, The Xcp module is initialized correctly. - can not be tested with conventional module tests*/
void Xcp_SoAdTxConfirmation (PduIdType XcpRxPduId)
{
    DET_VALIDATE_NRV(Xcp_Inited                    , 0x02, XCP_E_NOT_INITIALIZED);
    DET_VALIDATE_NRV(XcpRxPduId == XCP_PDU_ID_TX   , 0x02, XCP_E_INVALID_PDUID);

	Xcp_TxConfirmation();
}

/**
 * Called by core Xcp to transmit data
 * @param data
 * @param len
 * @return
 */
Std_ReturnType Xcp_Transmit(const void* data, int len)
{
    uint8 buf[len+4];
    PduInfoType pdu;
    pdu.SduDataPtr = buf;
    pdu.SduLength  = len+4;

    SET_UINT16(buf, 0, len);
    SET_UINT16(buf, 2, ++Xcp_EthCtrTx);
    memcpy(buf+4, data, len);

    return SoAdIf_Transmit(XCP_PDU_ID_TX, &pdu);
}

/**
 * Called when the core of xcp have received a transport layer command
 * @param pid
 * @param data
 * @param len
 * @return
 */
extern Std_ReturnType Xcp_CmdTransportLayer(uint8 pid, void* data, int len)
{
    Xcp_TxError(XCP_ERR_CMD_UNKNOWN);
    return E_OK;
}

#endif
