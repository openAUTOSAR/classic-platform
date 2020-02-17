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

#ifndef TCPIP_ETHSM_H_
#define TCPIP_ETHSM_H_

#include "TcpIp_Types.h"

/* This header file shall shall export the part of the TcpIp API exactly required by EthSM.*/

/* @req SWS_TCPIP_00070 */
Std_ReturnType TcpIp_RequestComMode(uint8 CtrlIdx, TcpIp_StateType State);

#endif /*TCPIP_ETHSM_H_*/
