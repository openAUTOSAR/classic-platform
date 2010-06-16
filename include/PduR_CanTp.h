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



#ifndef PDUR_CANTP_H_
#define PDUR_CANTP_H_

#include "PduR.h"

#if (PDUR_ZERO_COST_OPERATION == STD_OFF)

		BufReq_ReturnType PduR_CanTpProvideRxBuffer(PduIdType CanTpRxPduId, PduLengthType TpSduLength, PduInfoType** PduInfoPtr);
		void PduR_CanTpRxIndication(PduIdType CanTpRxPduId, NotifResultType Result);
		BufReq_ReturnType PduR_CanTpProvideTxBuffer(PduIdType CanTpTxPduId, PduInfoType** PduInfoPtr, uint16 Length);
		void PduR_CanTpTxConfirmation(PduIdType CanTpTxPduId, NotifResultType Result);

#else // Zero cost operation active

	#if (PDUR_DCM_SUPPORT == STD_ON)

		#include "Dcm_Cbk.h"

		#define PduR_CanTpProvideRxBuffer Dcm_ProvideRxBuffer
		#define PduR_CanTpRxIndication Dcm_RxIndication
		#define PduR_CanTpProvideTxBuffer Dcm_ProvideTxBuffer
		#define PduR_CanTpTxConfirmation Dcm_TxConfirmation

	#else

		#define PduR_CanTpProvideRxBuffer(...)
		#define PduR_CanTpRxIndication(...)
		#define PduR_CanTpProvideTxBuffer(...)
		#define PduR_CanTpTxConfirmation(...)

	#endif

#endif // Zero cost operation active

#endif /* PDUR_CANTP_H_ */
