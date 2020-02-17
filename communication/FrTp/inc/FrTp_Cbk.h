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
#ifndef FRTP_CBK_H_
#define FRTP_CBK_H_


/*The lower layer communication module requests the buffer of the SDU for
transmission from the upper layer module. */
/* @req FRTP154 */
Std_ReturnType FrTp_TriggerTransmit(PduIdType TxPduId,PduInfoType* PduInfoPtr);

/* Indication of a received I-PDU from a lower layer communication module. */
/* @req FRTP152 */
void FrTp_RxIndication(PduIdType RxPduId,PduInfoType* PduInfoPtr);

/* The lower layer communication module confirms the transmission of an I-PDU.*/
/* @req FRTP153 */
void FrTp_TxConfirmation(PduIdType TxPduId);


#endif /* FRTP_CBK_H_ */
