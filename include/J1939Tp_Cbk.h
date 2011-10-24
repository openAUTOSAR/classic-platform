/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/


#ifndef J1939TP_CBK_H_
#define J1939TP_CBK_H_

/** @req J1939TP0102 **/
/** Indication of a received I-PDU from a lower layer communication module */
void J1939Tp_RxIndication(PduIdType RxPduId, PduInfoType* PduInfoPtr);

/** The lower layer communication module confirms the transmission of an I-PDU */
void J1939Tp_TxConfirmation(PduIdType TxPduId);


#endif
