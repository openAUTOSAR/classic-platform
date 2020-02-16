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

/** @req XCP713 *//*Support Can communication*/
/** @req XCP714 *//*Use API provided for CanIf*/
/** @req XCP716 *//*Performance information must be exchanged between Slave and Master*/
/** @req XCP718 *//*Support GET_SLAVE_ID command*/
/** @req XCP517 *//*2 can Pdu needed*/
#ifdef USE_XCP
#include "Xcp.h"

#if !(XCP_PROTOCOL != XCP_PROTOCOL_CAN)

#include "Xcp_Internal.h"
#include "Xcp_ByteStream.h"
#include "XcpOnCan_Cfg.h"
#include "ComStack_Types.h"
#include "CanIf.h"

#define AS_LOG_XCP 0

typedef enum {
	XCP_CAN_CMD_SET_DAQ_ID   = 0xFD,
	XCP_CAN_CMD_GET_DAQ_ID   = 0xFE,
	XCP_CAN_CMD_GET_SLAVE_ID = 0xFF,
} Xcp_CanCmdType;

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
/** @req XCP813 *//*Xcp_<module>RxIndication API definition*/
/** @req XCP847 *//*Send DET if Xcp was not initialized, XcpRxPduPtr equals NULL_PTR, Invalid PDUID*/
void Xcp_CanIfRxIndication(
        PduIdType    XcpRxPduId,
        PduInfoType* XcpRxPduPtr)
{
    DET_VALIDATE_NRV(Xcp_Inited                    , 0x03, XCP_E_NOT_INITIALIZED);
    DET_VALIDATE_NRV(XcpRxPduPtr                   , 0x03, XCP_E_INV_POINTER);

#if (XCP_FEATURE_GET_SLAVE_ID == STD_ON)
    DET_VALIDATE_NRV((XcpRxPduId == XCP_PDU_ID_RX) ||
    		         (XcpRxPduId == XCP_PDU_ID_BROADCAST) , 0x03, XCP_E_INVALID_PDUID);
#else
    DET_VALIDATE_NRV(XcpRxPduId == XCP_PDU_ID_RX  , 0x03, XCP_E_INVALID_PDUID);
#endif

	Xcp_RxIndication(XcpRxPduPtr->SduDataPtr, XcpRxPduPtr->SduLength);
}

#if 0
/**
 * Wrapper callback function for use in AUTOSAR 3.0 specification
 *
 * @param XcpRxPduId PDU-ID that has been received
 * @param data       Data that has been received
 * @param len        Length of data pointed to by data
 * @param type       The CAN id of the received data
 */
void Xcp_CanIfRxSpecial(uint8 channel, PduIdType XcpRxPduId, const uint8 * data, uint8 len, Can_IdType type)
{
	PduInfoType info = {
			.SduDataPtr = (uint8*)data,
			.SduLength  = len,
	};

	XCP_UNUSED(channel);
	XCP_UNUSED(type);

	Xcp_CanIfRxIndication(XcpRxPduId, &info);


}
#endif

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
void Xcp_CanIfTxConfirmation(
        PduIdType XcpTxPduId)
{

    DET_VALIDATE_NRV(Xcp_Inited                    , 0x02, XCP_E_NOT_INITIALIZED);
    DET_VALIDATE_NRV(XcpTxPduId == XCP_PDU_ID_TX   , 0x02, XCP_E_INVALID_PDUID);

	Xcp_TxConfirmation();
}

#if 0
/**
 * Callback that is called before a PDU is transmitted
 *
 * This function is called by the lower layers (i.e. FlexRay Interface, TTCAN Interface
 * and Socket Adaptor or CDD) when an AUTOSAR XCP PDU shall be transmitted.
 * The function Xcp_<module>TriggerTransmit is called by the <bus> Interface for
 * requesting the I-PDU before transmission. Whether the function
 * Xcp_<module>TriggerTransmit is called or not is statically configured for each I-PDU
 * in the configuration.
 *
 * Can be called on interrupt level
 *
 * ServiceId: 0x05
 *
 * @param XcpTxPduId
 * @param PduInfoPtr
 * @return E_NOT_OK or E_OK
 */

Std_ReturnType Xcp_CanIfTriggerTransmit(
        PduIdType XcpTxPduId,
        PduInfoType* PduInfoPtr)
{
	DET_VALIDATE_RV(Xcp_Inited, 0x05, XCP_E_INV_POINTER, E_NOT_OK);
	XCP_UNUSED(XcpTxPduId);
	XCP_UNUSED(PduInfoPtr);

	return E_NOT_OK;
}
#endif

/**
 * Transport protocol agnostic transmit function called by Xcp core system
 *
 * @param data
 * @param len
 * @return
 */
Std_ReturnType Xcp_Transmit(const void* data, int len)
{
	PduInfoType pdu;
	pdu.SduDataPtr = (uint8*)data;
	pdu.SduLength  = len;
	return CanIf_Transmit(XCP_PDU_ID_TX, &pdu);
}


/**
 * Command that can be used for a master to discover
 * all connected XCP slaves on a CAN bus
 * @param data
 * @param len
 * @return
 */

#if(XCP_FEATURE_GET_SLAVE_ID == STD_ON)
static Std_ReturnType Xcp_CmdGetSlaveId(void* data, int len)
{
    uint8 p[4];
    uint8 mode;

    if(len < 4) {
        RETURN_ERROR(XCP_ERR_CMD_SYNTAX, ("Invalid length for get_slave_id %d", len));
    }

    p[0] = GET_UINT8(data, 0);
    p[1] = GET_UINT8(data, 1);
    p[2] = GET_UINT8(data, 2);
    p[3] = 0;
    mode = GET_UINT8(data, 3);

    if(strcmp((char*)p, "XCP")) {
        RETURN_ERROR(XCP_ERR_CMD_UNKNOWN, ("Unknown get_id pattern %s", p));
    }

    if(mode == 0) {
        FIFO_GET_WRITE(Xcp_FifoTx, e) {
            FIFO_ADD_U8 (e, XCP_PID_RES);
            FIFO_ADD_U8 (e, p[0]);
            FIFO_ADD_U8 (e, p[1]);
            FIFO_ADD_U8 (e, p[2]);
            FIFO_ADD_U32(e, XCP_CAN_ID_RX);
        }
    } else if(mode == 1) {
        FIFO_GET_WRITE(Xcp_FifoTx, e) {
            FIFO_ADD_U8 (e, XCP_PID_RES);
            FIFO_ADD_U8 (e, ~p[0]);
            FIFO_ADD_U8 (e, ~p[1]);
            FIFO_ADD_U8 (e, ~p[2]);
            FIFO_ADD_U32(e, XCP_CAN_ID_RX);
        }
    } else {
        RETURN_ERROR(XCP_ERR_CMD_UNKNOWN, ("Invalid mode for get_slave_id: %u", mode));
    }

    return E_OK;
}
#endif //XCP_FEATURE_GET_SLAVE_ID

/**
 * Called when the core of xcp have received a transport layer command
 * @param pid
 * @param data
 * @param len
 * @return
 */
Std_ReturnType Xcp_CmdTransportLayer(uint8 pid, void* data, int len)
{

	Xcp_CanCmdType id = (Xcp_CanCmdType)GET_UINT8(data, 0);
	XCP_UNUSED(pid);
	XCP_UNUSED(len);
#if(XCP_FEATURE_GET_SLAVE_ID == STD_ON)
    if(id == XCP_CAN_CMD_GET_SLAVE_ID && len >= 2) {
        return Xcp_CmdGetSlaveId((void*)((uint8*)data+1), len-1);
    }
#endif

    RETURN_ERROR(XCP_ERR_CMD_UNKNOWN, ("Unknown transport cmd:%u,  len:%u",  id, len));
}

#endif

#endif /* USE_XCP */
