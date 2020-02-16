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

/** @req ETHSM0004 */


#ifndef ETH_H_
#define ETH_H_

#include "Std_Types.h"

Std_ReturnType EthSM_Init( const EthSM_ConfigType* ConfigPtr );

void EthSM_GetVersionInfo( Std_VersionInfoType* versioninfo );
Std_ReturnType EthSM_RequestComMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode );
Std_ReturnType EthSM_GetCurrentComMode( NetworkHandleType NetworkHandle, ComM_ModeType* ComM_ModePtr );
Std_ReturnType EthSM_TrcvLinkStateChg( NetworkHandleType NetworkHandle, EthTrcv_LinkStateType TransceiverLinkState );
Std_ReturnType EthSm_TcpIpModeIndication( NetworkHandleType NetworkHandle, TcpIp_StateType TcpIpState );
Std_ReturnType EthSM_GetCurrentInternalMode( NetworkHandleType NetworkHandle, EthSM_NetworkModeStateType* EthSM_InternalMode );

void EthSM_CtrlModeIndication ( uint8 CtrlIdx, Eth_ModeType CtrlMode );
void EthSM_TrcvModeIndication ( uint8 TrcvIdx, EthTrcv_ModeType TrcvMode );

void EthSM_MainFunction( void );

#endif /* ETH_H_ */
