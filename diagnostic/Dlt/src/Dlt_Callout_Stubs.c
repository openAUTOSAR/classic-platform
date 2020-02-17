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

#include "Std_Types.h"
#include "Dlt.h"

#define CFG_COMLOG_SIZE 500u

#if defined(USE_CDDPDUR)
#include "CddPduR.h"

static uint8 dltcombuffer[CFG_COMLOG_SIZE];

#elif defined(USE_DLTUARTCOM)
#include "DltUartCom.h"

static uint8 dltUartcom_rx_buffer[DLTUARTCOM_BUFSIZE];
static uint16 bufIdx;
static uint16 msgLen;
#else
/*lint -esym(9003,comlog) Misra violation. This log can be accessed from other modules and is used */
uint8 comlog[CFG_COMLOG_SIZE];
/*lint -esym(9003,comlog_curr) Misra violation. This log can be accessed from other modules and is used */
uint32 comlog_curr = 0;

/**
 * Print a char to the buf
 * @param c
 */
static void comlog_chr( uint8 c ) {

    comlog[comlog_curr] = c;
    comlog_curr++;
    if( comlog_curr >= CFG_COMLOG_SIZE ) {
        comlog_curr = 0;
    }
}
#endif

void DltCom_Init(void){
#if defined(USE_CDDPDUR)
    CddPduR_ProvideBuffer(CDDPDUR_PDU_ID_DLT_RX , dltcombuffer, sizeof(dltcombuffer));
#elif defined(USE_DLTUARTCOM)
    DltUartCom_Init();
#endif
}

#if defined(USE_CDDPDUR) || defined(USE_DLTUARTCOM)
static void DltCom_HandleRx(const uint8 *buffPtr, uint16 length){
    Dlt_StandardHeaderType header;
#if defined(USE_CDDPDUR)
    uint16 lenLeft = length;
    uint16 index = 0;

    while(lenLeft >= sizeof(Dlt_StandardHeaderType)){
        header.HeaderType = buffPtr[index];
        header.MessageCounter = buffPtr[index+1];
        /*lint --e{9033} inhibit lint warning to avoid false Misra violation */
        header.Length = ((uint16)(buffPtr[index+2]) << 8u) | (uint16)buffPtr[index+3];

        if(lenLeft >= header.Length){
            /* we have at least one Dlt message in length */
            Dlt_ArcProcessIncomingMessage(&header, &buffPtr[index + sizeof(Dlt_StandardHeaderType)]);
            lenLeft -= header.Length;
            index += header.Length;
        }else{
            /* Invalid message, just sink */
            lenLeft = 0;
        }
    }
#else
    if (msgLen > DLTUARTCOM_BUFSIZE) {
         /* can't receive/process message; just read and skip it */
         bufIdx++;
         if (bufIdx == msgLen) {
            /* end of message */
            bufIdx = 0;
            msgLen = 0;
         }
      }
      dltUartcom_rx_buffer[bufIdx++] = *buffPtr;

      if (bufIdx == sizeof(Dlt_StandardHeaderType)) {
         header.HeaderType = dltUartcom_rx_buffer[0U];
         header.MessageCounter = dltUartcom_rx_buffer[1U];
         header.Length = ((uint16)(dltUartcom_rx_buffer[2U]) << 8U) |
                          (uint16)dltUartcom_rx_buffer[3U];
         msgLen = header.Length;
      }

      if (bufIdx == msgLen) {
         /* complete message received -> relay it to DLT */
         /* if it is time critical to call Dlt from this ISR callback, we need to establish a cyclic dltcom function */
         Dlt_ArcProcessIncomingMessage(&header, &dltUartcom_rx_buffer[sizeof(Dlt_StandardHeaderType)]);
         bufIdx = 0U;
         msgLen = 0U;
      }
#endif
}

Std_ReturnType DltCom_ReceiveIndication(PduIdType DltRxPduId, const PduInfoType* PduInfoPtr){
#if defined(USE_CDDPDUR)
    Std_ReturnType rv;
    void *buffPtr;
    uint16 length = 0;

    rv = CddPduR_Receive( CDDPDUR_PDU_ID_DLT_RX, &buffPtr, &length);

    if(rv == E_OK){
        // Handling of received DLT frame
        DltCom_HandleRx((uint8 *)buffPtr, length);
    }

    /* Mark that you are done with the data */
    CddPduR_UnlockBuffer( CDDPDUR_PDU_ID_DLT_RX );

    /* Provide a new buffer */
    CddPduR_ProvideBuffer(CDDPDUR_PDU_ID_DLT_RX , dltcombuffer, sizeof(dltcombuffer));
#else
    // Handling of received DLT frame
    DltCom_HandleRx((uint8 *)PduInfoPtr->SduDataPtr, PduInfoPtr->SduLength);
#endif
    return E_OK;
}
#endif

Std_ReturnType DltCom_Transmit(PduIdType DltTxPduId, const PduInfoType* PduInfoPtr){

    Std_ReturnType res;
#if defined(USE_CDDPDUR)
    const uint8 *data = PduInfoPtr->SduDataPtr;
    if(TRUE == Dlt_ArcIsDltConnected()){
        res = CddPduR_Send(CDDPDUR_PDU_ID_DLT_TX,data,PduInfoPtr->SduLength);
        /* No TxConfirmation required for Control messages */
        if(DLT_ARC_MAGIC_CONTROL_NUMBER != DltTxPduId){
            Dlt_ComTxConfirmation(DltTxPduId, res);
        }
    }else{
        res = E_NOT_OK;
    }
#elif defined(USE_DLTUARTCOM)
    res = DltUartCom_Transmit(DltTxPduId, PduInfoPtr);
    Dlt_ComTxConfirmation(DltTxPduId, res);
#else
    const uint8 *data = PduInfoPtr->SduDataPtr;
    res = E_OK;

    for (uint32 i = 0; i < PduInfoPtr->SduLength; i++) {
        comlog_chr(*data);
        data++;
    }
    Dlt_ComTxConfirmation(DltTxPduId, res);
#endif
    return res;
}
