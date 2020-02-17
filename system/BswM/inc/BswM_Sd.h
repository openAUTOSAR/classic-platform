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


#ifndef BSWM_SD_H_
#define BSWM_SD_H_

#include "SD_Types.h"

/** @req SWS_BswM_00204 */
void BswM_Sd_ClientServiceCurrentState(uint16 SdClientServiceHandleId,
                                       Sd_ClientServiceCurrentStateType CurrentClientState);

/** @req SWS_BswM_00207 */
void BswM_Sd_ConsumedEventGroupCurrentState(uint16 SdConsumedEventGroupHandleId,
                                            Sd_ConsumedEventGroupCurrentStateType ConsumedEventGroupState);

/** @req SWS_BswM_00210 */
void BswM_Sd_EventHandlerCurrentState(uint16 SdEventHandlerHandleId,
                                      Sd_EventHandlerCurrentStateType EventHandlerStatus);

#endif /* BSWM_SD_H_ */
