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
#ifdef USE_ETH
#include "Eth.h"
#include "Det.h"
#include "Dem.h"
#include "Eth_MemMap.h"
#include "SchM_Eth.h"

void Eth_Init( const Eth_ConfigType* CfgPtr );
Std_ReturnType Eth_ControllerInit( uint8 CtrlIdx, uint8 CfgIdx );
Std_ReturnType Eth_SetControllerMode( uint8 CtrlIdx, Eth_ModeType CtrlMode );
Std_ReturnType Eth_GetControllerMode( uint8 CtrlIdx, Eth_ModeType* CtrlModePtr );
void Eth_GetPhysAddr( uint8 CtrlIdx, uint8* PhysAddrPtr );
void Eth_SetPhysAddr( uint8 CtrlIdx, const uint8* PhysAddrPtr );
Std_ReturnType Eth_UpdatePhysAddrFilter( uint8 CtrlIdx, uint8* PhysAddrPtr, Eth_FilterActionType Action );
Eth_ReturnType Eth_WriteMii( uint8 CtrlIdx, uint8 TrcvIdx, uint8 RegIdx, uint16 RegVal );
Eth_ReturnType Eth_ReadMii( uint8 CtrlIdx, uint8 TrcvIdx, uint8 RegIdx, uint16* RegValPtr );
void Eth_GetCounterState( uint8 CtrlIdx, uint16 CtrOffs, uint32* CtrValPtr );
BufReq_ReturnType Eth_ProvideTxBuffer( uint8 CtrlIdx, uint8* BufIdxPtr, Eth_DataType** BufPtr, uint16* LenBytePtr );
Std_ReturnType Eth_Transmit( uint8 CtrlIdx, uint8 BufIdx, Eth_FrameType FrameType, boolean TxConfirmation, uint16 LenByte, uint8* PhysAddrPtr );
void Eth_Receive( uint8 CtrlIdx, Eth_RxStatusType* RxStatusPtr );
void Eth_TxConfirmation( uint8 CtrlIdx );
void Eth_GetVersionInfo( Std_VersionInfoType* VersionInfoPtr );
#endif

