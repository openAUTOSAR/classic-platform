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

#include "PduR.h"
#include "CddPduR.h"
#include "PduR_CddPduR.h"
#include <string.h>
#if defined(USE_DLT)
#include "Dlt.h"
#endif


typedef enum
{
    BUFF_STATE_NO_BUFFER=0,
    BUFF_STATE_RX_OK = 1,
    BUFF_STATE_ERROOR = 2,
    BUFF_STATE_FREE = 3,
    BUFF_STATE_BUSY = 4
} BuffStateType;


typedef struct CddPduR_BuffInfo {
    uint8 				*sduDataPtr;
    PduLengthType 		sduLength;
    uint16 				idx;
    BuffStateType  		state;
    /* Size of buffer provided in CddPduR_ProvideBuffer() */
    uint16 				buffSize;
} CddPduR_BuffInfoType;

CddPduR_BuffInfoType CddPduR_BInfo[CDDPDUR_MAX_N_IPDUS];

void CddPduR_RxIndication(PduIdType id, PduInfoType* pduInfoPtr) {
    if ( (CddPduR_BInfo[id].state == BUFF_STATE_FREE) &&
         (CddPduR_BInfo[id].buffSize >= pduInfoPtr->SduLength) )
    {
        CddPduR_BInfo[id].sduLength = pduInfoPtr->SduLength;
        // Copy IPDU data
        memcpy(	CddPduR_BInfo[id].sduDataPtr,
                pduInfoPtr->SduDataPtr,
                pduInfoPtr->SduLength);
        CddPduR_BInfo[id].state = BUFF_STATE_RX_OK;
#if defined(USE_DLT)
        DltCom_ReceiveIndication(id,pduInfoPtr);
#endif
    }
}


void CddPduR_TpRxIndication( PduIdType id, Std_ReturnType result ) {
    if( result == E_OK ) {
        CddPduR_BInfo[id].state = BUFF_STATE_RX_OK;
    }
}


/**
 * Function is called once the I-PDU have been transmitted over the network
 *
 * @param id
 * @param result
 */
void CddPduR_TpTxConfirmation(PduIdType id, Std_ReturnType result ) {
    CddPduR_BInfo[id].state = BUFF_STATE_NO_BUFFER;
}
void CddPduR_TxConfirmation(PduIdType id, Std_ReturnType result ) {
    CddPduR_BInfo[id].state = BUFF_STATE_NO_BUFFER;
}

Std_ReturnType CddPduR_Send(PduIdType id, const void* dataPtr, uint16 length ) {

    Std_ReturnType pduRv = E_NOT_OK;
    PduInfoType	info = {.SduDataPtr = (void *)dataPtr, .SduLength = length };

    if( CddPduR_BInfo[id].state == BUFF_STATE_NO_BUFFER ) {
        CddPduR_BInfo[id].state = BUFF_STATE_BUSY;
        CddPduR_BInfo[id].sduDataPtr = (uint8_t *)dataPtr;
        CddPduR_BInfo[id].sduLength = length;
        CddPduR_BInfo[id].idx = 0;

        /* Call the PduR function */
        pduRv = PduR_CddPduRTransmit(CddPduR_PduMap[id], &info );
        if(pduRv == E_NOT_OK){
            CddPduR_BInfo[id].state = BUFF_STATE_NO_BUFFER;
        }
    }

    return pduRv;
}

Std_ReturnType CddPduR_Receive( PduIdType id, void** dataPtr, uint16* length ) {

    Std_ReturnType rv;

    /* Return the buffer if the message is complete */
    if (CddPduR_BInfo[id].state == BUFF_STATE_RX_OK) {

        *dataPtr = CddPduR_BInfo[id].sduDataPtr;
        *length = CddPduR_BInfo[id].sduLength;
        rv = E_OK;
    } else {
        *length = 0;
        /* Busy */
        rv = E_NOT_OK;
    }

    return rv;
}


Std_ReturnType CddPduR_ProvideBuffer( PduIdType id, void* dataPtr, uint16 length ) {

    /* Return the buffer if the message is complete */
    CddPduR_BInfo[id].sduDataPtr = dataPtr;
    CddPduR_BInfo[id].sduLength = 0;
    CddPduR_BInfo[id].buffSize = length;

    CddPduR_BInfo[id].state = BUFF_STATE_FREE;

    return E_OK;
}

Std_ReturnType CddPduR_UnlockBuffer( PduIdType id ) {

    CddPduR_BInfo[id].state = BUFF_STATE_NO_BUFFER;

    return E_OK;
}



/**
 * Copy received data.
 *
 * @param PduId
 * @param PduInfoPtr
 * @param RxBufferSizePtr
 * @return
 */
BufReq_ReturnType CddPduR_CopyRxData(PduIdType id, const PduInfoType* pduInfoPtr, PduLengthType* rxBufferSizePtr) {

    BufReq_ReturnType rv = BUFREQ_E_NOT_OK;

    /* Copy received data to buffer */
    if (CddPduR_BInfo[id].state == BUFF_STATE_BUSY)
    {
        if (pduInfoPtr->SduLength > 0)
        {
            memcpy(	&CddPduR_BInfo[id].sduDataPtr[CddPduR_BInfo[id].idx],
                    pduInfoPtr->SduDataPtr,
                    pduInfoPtr->SduLength);

            CddPduR_BInfo[id].idx += pduInfoPtr->SduLength;
            *rxBufferSizePtr = CddPduR_BInfo[id].sduLength - CddPduR_BInfo[id].idx;
            rv = BUFREQ_OK;
        } else {
            *rxBufferSizePtr = CddPduR_BInfo[id].sduLength - CddPduR_BInfo[id].idx;
            rv = BUFREQ_OK;
        }
    }

    return rv;
}


/**
 *
 * @param PduId
 * @param PduInfoPtr    Provides destination buffer and number for bytes to copy.
 * @param RetryInfoPtr
 * @param TxDataCntPtr
 * @return				BUFREQ_OK - : Data has been copied to the transmit buffer completely as requested
 * 						BUFREQ_E_NOT_OK: Fail
 * 						BUFREQ_E_BUSY:   Fail
 *
 */
BufReq_ReturnType CddPduR_CopyTxData(	PduIdType id,
                                        PduInfoType* pduInfoPtr,
                                        RetryInfoType* retryInfoPtr,
                                        PduLengthType* txDataCntPtr)
{
    /* Copy from interal buffer to PduR buffer */
    memcpy(	pduInfoPtr->SduDataPtr,
            &CddPduR_BInfo[id].sduDataPtr[CddPduR_BInfo[id].idx],
            pduInfoPtr->SduLength);

    CddPduR_BInfo[id].idx += pduInfoPtr->SduLength;

    /* return remaining TX data */
    *txDataCntPtr =  CddPduR_BInfo[id].sduLength - CddPduR_BInfo[id].idx;

    return BUFREQ_OK;
}

/**
 * Start the reception of PduId
 *
 * @param PduId
 * @param TpSduLength
 * @param RxBufferSizePtr
 * @return
 */

/**
 *
 * @param id				The PduId
 * @param infoPtr			Pointer to structure containing content and length
 *                          of FF or SF.
 *                          NOT USED..is null
 * @param tpSduLength		Total length of the N-SDU to be received
 * @param rxBufferSizePtr	[out] Length of the available buffer
 * @return 	BUFREQ_OK 		- Accept
 * 			BUFREQ_E_NOT_OK - Reject connection
 * 			BUFREQ_E_OVFL 	- Cannot supply buffer, reception abort.
 */
BufReq_ReturnType CddPduR_StartOfReception(	PduIdType id,
                                            const PduInfoType* infoPtr,
                                            PduLengthType tpSduLength,
                                            PduLengthType* rxBufferSizePtr)
{
    BufReq_ReturnType rv;

    if ( (CddPduR_BInfo[id].state == BUFF_STATE_FREE) &&
         (CddPduR_BInfo[id].buffSize >= tpSduLength) )
    {
        /* Set available length for PduR */
        *rxBufferSizePtr = CddPduR_BInfo[id].buffSize;

        CddPduR_BInfo[id].sduLength = tpSduLength;
        CddPduR_BInfo[id].idx = 0;
        CddPduR_BInfo[id].state = BUFF_STATE_BUSY;
        rv = BUFREQ_OK;
    } else {
        rv = BUFREQ_E_OVFL;
    }

    return rv;
}

