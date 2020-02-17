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

#ifndef LDCOM_TYPES_H_
#define LDCOM_TYPES_H_

/* @req SWS_LDCOM_00020 */
#include "Std_Types.h"
#include "ComStack_Types.h"

/* Type of Lower layer for a given IPdu */
typedef enum {
    LDCOM_IF,
    LDCOM_TP
}LdComLowerApiType;


/* IPdu direction */
typedef enum {
    LDCOM_RECEIVE,
    LDCOM_SEND
}LdComIPduDirectionType;

typedef BufReq_ReturnType (*LdComCopyTxData)(const PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr );
typedef void (*LdComTpTxConfirmation)( Std_ReturnType result );
typedef BufReq_ReturnType (*LdComStartOfReception)( const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr );
typedef BufReq_ReturnType (*LdComCopyRxData)( const PduInfoType* info, PduLengthType* bufferSizePtr );
typedef void (*LdComTpRxIndication)( Std_ReturnType result );
typedef void (*LdComRxIndication)( const PduInfoType* PduInfoPtr );
typedef void (*LdComTxConfirmation)(void);
typedef Std_ReturnType (*LdComTriggerTransmit)( PduInfoType* PduInfoPtr );


/* LdComIPdu config structure type */
typedef struct {
    LdComStartOfReception   LdComRxStartOfReceptionCbk; /* Callback for LdCom_StartOfReception */
    LdComCopyRxData         LdComCopyRxDataCbk; /* Callback for LdCom_CopyRxData */
    LdComTpRxIndication     LdComTpRxIndicationCbk; /* Callback for LdCom_TpRxIndication */
    LdComCopyTxData         LdComCopyTxDataCbk; /* Callback for LdCom_CopyTxData */
    LdComTpTxConfirmation   LdComTpTxConfirmationCbk; /* Callback for LdCom_TpTxConfirmation */
    LdComRxIndication       LdComRxIndicationCbk; /* Callback for LdCom_RxIndication */
    LdComTriggerTransmit    LdComTxTriggerTransmitCbk; /* Callback for LdCom_TriggerTransmit */
    LdComTxConfirmation     LdComTxConfirmationCbk; /* Callback for LdCom_TxConfirmation */
    PduIdType               LdComHandleId; /* Handle Id */
    PduIdType               LdComPdurPduId ; /* PduR Pdu Id for transmit */
    LdComLowerApiType       LdComLowerApi; /* Type of lower api */
    LdComIPduDirectionType  LdComIPduDir; /* Direction of IPdu */

}LdCom_IPduType;

typedef struct {
    const LdCom_IPduType *LdComIPduCfg;
    uint8 LdComIPduCnt;
}LdCom_ConfigType;

#endif /*LDCOM_TYPES_H_*/
