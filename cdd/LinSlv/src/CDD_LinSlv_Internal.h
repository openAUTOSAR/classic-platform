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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */
/** @tagSettings DEFAULT_ARCHITECTURE=MPC5645S */

#ifndef CDD_LINSLV_INTERNAL_H_
#define CDD_LINSLV_INTERNAL_H_

#include "CDD_LinSlv.h"

Std_ReturnType LinSlv_Internal_Init(LinSlv_ContainerType* LinSlv_Cfg);
void LinSlv_Internal_Deinit(LinSlv_ContainerType* LinSlv_Cfg);
void LinSlv_Internal_GoToSleepInternal(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel);
void LinSlv_Internal_WakeupInternal(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel);
Std_ReturnType LinSlv_Internal_Wakeup(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel);
boolean LinSlv_Internal_IsWakeUpSet(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel);
Lin_StatusType LinSlv_Internal_GetStatus(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel);
void LinSlv_Internal_SetResponse(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel, uint8 Length, const uint8 *Buffer);
LinSlv_TxIsrSourceType LinSlv_Hw_Tx_GetIsrSource(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel);
void LinSlv_Hw_ClearTxIsrFlags(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel);
void LinSlv_Internal_StoreAndDisableIsr(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel);
void LinSlv_Internal_RestoreAndEnableIsr(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel);
void LinSlv_Internal_UpdatePid(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel, uint8 frameIndex, uint8 newPid);
boolean LinSlv_Internal_IsWakeUpDoneTransmitting(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel);
Std_ReturnType LinSlv_Internal_SetTrcvMode(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel, LinTrcv_TrcvModeType TransceiverMode);
Std_ReturnType LinSlv_Internal_GetTrcvMode(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel, LinTrcv_TrcvModeType *TransceiverModePtr);

Std_ReturnType LinSlv_Hw_Rx(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel, const LinIf_FrameType **frameCfgPtr, uint8 *buffer);
Std_ReturnType LinSlv_Hw_Tx_GetFramePtr(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel, const LinIf_FrameType **frameCfgPtr);

Std_ReturnType LinSlv_Hw_Err(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel);


#endif /* CDD_LINSLV_INTERNAL_H_ */





