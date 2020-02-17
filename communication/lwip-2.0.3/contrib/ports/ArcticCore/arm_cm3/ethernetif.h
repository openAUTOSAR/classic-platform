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


#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"


err_t ethernetif_init(struct netif *netif);
void ethernetif_pre_tcpip_init(void);
struct pbuf * low_level_input();

/* ASR EthIf mockers */
#include "Eth_GeneralTypes.h"

/* EthIf controller IDs */
#define EthIfConf_EthIfController_EthIfController  0u
void EthIf_GetPhysAddr( uint8 CtrlIdx, uint8* PhysAddrPtr );
Std_ReturnType EthIf_UpdatePhysAddrFilter( uint8 CtrlIdx, const uint8* PhysAddrPtr, Eth_FilterActionType Action );

#endif 
