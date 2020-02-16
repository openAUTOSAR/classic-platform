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

#ifndef ETHTRCV_H_
#define ETHTRCV_H_

#include "EthTrcv_Types.h"
#include "EthTrcv_MemMap.h"

void EthTrcv_Init( const EthTrcv_ConfigType* CfgPtr );
Std_ReturnType EthTrcv_TransceiverInit( uint8 TrcvIdx, uint8 CfgIdx );
Std_ReturnType EthTrcv_SetTransceiverMode( uint8 TrcvIdx, EthTrcv_ModeType CtrlMode );
Std_ReturnType EthTrcv_GetTransceiverMode( uint8 TrcvIdx, EthTrcv_ModeType* TrcvModePtr );
Std_ReturnType EthTrcv_StartAutoNegotiation( uint8 TrcvIdx );
Std_ReturnType EthTrcv_GetLinkState( uint8 TrcvIdx, EthTrcv_LinkStateType* LinkStatePtr );
Std_ReturnType EthTrcv_GetBaudRate( uint8 TrcvIdx, EthTrcv_BaudRateType* BaudRatePtr );
Std_ReturnType EthTrcv_GetDuplexMode( uint8 TrcvIdx, EthTrcv_DuplexModeType* DuplexModePtr );
void EthTrcv_GetVersionInfo( Std_VersionInfoType* VersionInfoPtr );


#endif /* ETHTRCV_H_ */
