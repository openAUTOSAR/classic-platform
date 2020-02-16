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

/** @req XCP506 */ /*Provide XcpOnCan_Cfg.h - can not be tested with conventional module tests*/

#ifndef XCPONCAN_CBK_H_
#define XCPONCAN_CBK_H_

#include "ComStack_Types.h"
#include "CanIf.h"

void           Xcp_CanIfRxIndication   (PduIdType XcpRxPduId, PduInfoType* XcpRxPduPtr);
void           Xcp_CanIfTxConfirmation (PduIdType XcpTxPduId);

#if 0
void           Xcp_CanIfRxSpecial      (uint8 channel, PduIdType XcpRxPduId, const uint8 * data, uint8 len, Can_IdType type);
Std_ReturnType Xcp_CanIfTriggerTransmit(PduIdType XcpTxPduId, PduInfoType* PduInfoPtr);
#endif


#endif /* XCPONCAN_CBK_H_ */
