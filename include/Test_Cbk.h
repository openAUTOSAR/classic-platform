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








#ifndef TEST_CBK_
#define TEST_CBK_


#define USE_TEST_CBK 1

#if defined(USE_TEST_CBK)
#include "Can_Test.h"
#endif

#if defined(USE_TEST_CBK)
#define CANIF_TXCONFIRMATION_CALL(canTxPduId) \
	CT_CanIf_TxConfirmation_Called( canTxPduId );
#define CANIF_RXINDICATION_CALL(Hrh,CanId,CanDlc,CanSduPtr) \
	CT_CanIf_RxIndication_Called( Hrh, CanId, CanDlc, CanSduPtr );
#define CANIF_CANCELTXCONFIRMATION_CALL(PduInfoPtr) \
	CT_CanIf_CancelTxConfirmation_Called( PduInfoPtr );
#define CANIF_CONTROLLERBUSOFF_CALL(Controller) \
	CT_CanIf_ControllerBusOff_Called( Controller );
#define CANIF_CONTROLLERWAKEUP_CALL(Controller) \
	CT_CanIf_ControllerWakeup_Called( Controller );
#else
#define CANIF_TXCONFIRMATION_CALL(canTxPduId) 
#define CANIF_RXINDICATION_CALL(Hrh,CanId,CanDlc,CanSduPtr) 
#define CANIF_CANCELTXCONFIRMATION_CALL(PduInfoPtr) 
#define CANIF_CONTROLLERBUSOFF_CALL(Controller) 
#define CANIF_CONTROLLERWAKEUP_CALL(Controller) 
#endif


#endif /*TEST_CBK_*/
