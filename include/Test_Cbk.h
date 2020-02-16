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
