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








#ifndef CAN_TEST_H_
#define CAN_TEST_H_

void CT_CanIf_TxConfirmation_Called( PduIdType canTxPduId );
void CT_CanIf_RxIndication_Called( uint8 Hrh,Can_IdType CanId,uint8 CanDlc, const uint8 *CanSduPtr );
void CT_CanIf_CancelTxConfirmation_Called( const Can_PduType *PduInfoPtr );
void CT_CanIf_ControllerBusOff_Called( uint8 Controller );
void CT_CanIf_ControllerWakeup_Called( uint8 Controller );

#endif /*CAN_TEST_H_*/
