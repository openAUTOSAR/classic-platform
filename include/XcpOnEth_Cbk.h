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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.3 */

/** @req 4.1.3/SWS_Xcp_00508 */ /*Provide XcpOnEth_Cfg.h - can not be tested with conventional module tests*/

#ifndef XCPONETH_CBK_H_
#define XCPONETH_CBK_H_

#include "ComStack_Types.h"

void           Xcp_SoAdIfRxIndication   (PduIdType XcpRxPduId, PduInfoType* XcpRxPduPtr);
void           Xcp_SoAdIfTxConfirmation (PduIdType XcpTxPduId);

#endif /* XCPONETH_CBK_H_ */
