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


#ifndef ETHSM_CBK_H_
#define ETHSM_CBK_H_

/* Function prototypes of the call back functions used from outside modules, but for ETHSM */

void EthSM_CtrlModeIndication ( uint8 CtrlIdx, Eth_ModeType CtrlMode );
void EthSM_TrcvModeIndication ( uint8 CtrlIdx, EthTrcv_ModeType TrcvMode );

/**
 * Function routine to notice ETHSM about TCPIP state
 * @param CtrlIdx - controller id
 * @param TcpIpState - TCPIP Link state
 * @return E_OK or E_NOT_OK
 */
Std_ReturnType EthSM_TcpIpModeIndication( uint8 CtrlIdx, TcpIp_StateType TcpIpState );

#endif /* ETHSM_CBK_H_ */
