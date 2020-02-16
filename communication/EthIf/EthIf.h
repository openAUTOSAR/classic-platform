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

#ifndef ETHIF_H_
#define ETHIF_H_
#include "EthIf_Types.h"
#ifdef USE_ETHIF
#include "EthIf_MemMap.h"

void EthIf_Init( const EthIf_ConfigType* CfgPtr );
Std_ReturnType EthIf_ControllerInit( uint8 CtrlIdx, uint8 CfgIdx );
Std_ReturnType EthIf_SetControllerMode( uint8 CtrlIdx, Eth_ModeType CtrlMode );
Std_ReturnType EthIf_GetControllerMode( uint8 CtrlIdx, Eth_ModeType* CtrlModePtr );
Std_ReturnType EthIf_TransceiverInit( uint8 TrcvIdx, uint8 CfgIdx );
Std_ReturnType EthIf_SetTransceiverMode( uint8 TrcvIdx, EthTrcv_ModeType TrcvMode );
Std_ReturnType EthIf_GetTransceiverMode( uint8 TrcvIdx, EthTrcv_ModeType* TrcvModePtr );
void EthIf_GetPhysAddr( uint8 CtrlIdx, uint8* PhysAddrPtr );
void EthIf_SetPhysAddr( uint8 CtrlIdx, const uint8 PhysAddrPtr );
Std_ReturnType EthIf_UpdatePhysAddrFilter( uint8 CtrlIdx, uint8* PhysAddrPtr, Eth_FilterActionType Action );
BufReq_ReturnType EthIf_ProvideTxBuffer( uint8 CtrlIdx, Eth_FrameType FrameType, uint8 Priority, uint8* BufIdxPtr, uint8** BufPtr, uint16* LenBytePtr );
Std_ReturnType EthIf_Transmit( uint8 CtrlIdx, uint8 BufIdx, Eth_FrameType FrameType, boolean TxConfirmation, uint16 LenByte, uint8* PhysAddrPtr );
void EthIf_GetVersionInfo( Std_VersionInfoType* VersionInfoPtr );

void EthIf_RxIndication( uint8 CtrlIdx, Eth_FrameType FrameType, boolean IsBroadcast, uint8* PhysAddrPtr, Eth_DataType* DataPtr, uint16 LenByte );
void EthIf_TxConfirmation( uint8 CtrlIdx, uint8 BufIdx );

void EthIf_MainFunctionRx( void );
void EthIf_MainFunctionTx( void );
#endif /* USE_ETHIF */
#endif /* ETHIF_H_ */
