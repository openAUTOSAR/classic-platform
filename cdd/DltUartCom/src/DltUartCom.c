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
#include "ComStack_Types.h"
#if defined(USE_DLT)
#include "Dlt.h"
#endif
#include "DltUartCom.h"
#include "Uart.h"

/*------------------------------------------------------------------------------------------------*/
/* static functions */
static void dltUartCom_txcbk(void);
static void dltUartCom_rxcbk(uint8 a_Data);
static void dltUartCom_ercbk(uint8 error);

/*------------------------------------------------------------------------------------------------*/
/** \fn       void dltUartCom_txcbk(void)
 *  \brief    transmit callback function
 */
static void dltUartCom_txcbk(void) {
   /* defined for debug purposes */
}

/*------------------------------------------------------------------------------------------------*/
/** \fn       void dltUartCom_rxcbk(uint8 a_Data)
 *  \brief    receive callback function
 *
 *  \param    a_Data       received data
 */
static void dltUartCom_rxcbk(uint8 a_Data) {
#if defined(USE_DLT)
   PduInfoType pdu;
   pdu.SduDataPtr = &a_Data;
   pdu.SduLength = sizeof(a_Data);
   DltCom_ReceiveIndication(DLTUARTCOM_PDU_ID_RX, &pdu);
#endif
}

/*------------------------------------------------------------------------------------------------*/
/** \fn       void dltUartCom_ercbk(uint8 error)
 *  \brief    error callback function
 *
 *  \param    error        error byte extracted from uart register
 */
static void dltUartCom_ercbk(uint8 error) {
   (void) error;
   /* defined for debug purposes */
}


/** \fn       void DltUartCom_Init(void)
 *  \brief    initialize Dltcom for uart
 */
void DltUartCom_Init(void) {
   (void) Uart_GetChannel(DLTUARTCOM_CHANNEL, dltUartCom_txcbk, dltUartCom_rxcbk, dltUartCom_ercbk);
}

/*------------------------------------------------------------------------------------------------*/
/** \fn       Std_ReturnType DltUartCom_Transmit(PduIdType DltTxPduId, const PduInfoType* PduInfoPtr)
 *  \brief    transmit DLT data via uart

 *  \param    DltTxPduId ID of Dlt I-PDU to be transmitted
 *  \param    PduInfoPtr  Pointer to a structure with I-PDU related data that shall be transmitted
 *  \return   E_OK: Transmit request has been accepted
 *            E_NOT_OK: Transmit request has not been accepted
 */
Std_ReturnType DltUartCom_Transmit(PduIdType DltTxPduId, const PduInfoType* PduInfoPtr) {
   Std_ReturnType res = E_NOT_OK;
   uint16 size = PduInfoPtr->SduLength;

   /* Relay message to UART if it fits into the available buffer space */
   if (size <= Uart_GetFreeTransmitData(DLTUARTCOM_CHANNEL, size)) {
      Uart_TransmitData(DLTUARTCOM_CHANNEL, size, PduInfoPtr->SduDataPtr);
      res = E_OK;
   }
   return res;
}
