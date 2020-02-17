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
/** @reqSettings DEFAULT_SPECIFICATION_REVISION=NONE */
/** @tagSettings DEFAULT_ARCHITECTURE=RH850F1H|ZYNQ|JACINTO6 */

#ifndef CDD_ETHTRCV_H_
#define CDD_ETHTRCV_H_

#include "EthTrcv_Types.h"
#include "CDD_EthTrcvTypes.h"
#include "CddEthTrcv_Cfg.h"

#define ETHTRCV_MAX_ADDRESS              32u

Std_ReturnType EthTrcv_TransceiverInit  (uint8 ctrlIdx);
void           EthTrcv_TransceiverLinkUp(uint8 ctrlIdx);
uint8          EthTrcv_FindPhyAdrs       (uint8 CtrlIdx);
uint8          EthTrcv_FindConfiguredPhyAdrs(uint8 CtrlIdx);

#endif /* CDD_ETHTRCV_H_ */
