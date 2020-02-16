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








#ifndef MEMIF_H_
#define MEMIF_H_

#include "Modules.h"

#define MEMIF_MODULE_ID			MODULE_ID_MEMIF
#define MEMIF_VENDOR_ID			VENDOR_ID_ARCCORE

#define MEMIF_SW_MAJOR_VERSION	1
#define MEMIF_SW_MINOR_VERSION	0
#define MEMIF_SW_PATCH_VERSION	0
#define MEMIF_AR_MAJOR_VERSION	3
#define MEMIF_AR_MINOR_VERSION	1
#define MEMIF_AR_PATCH_VERSION	5

#include "Std_Types.h"
#include "MemIf_Cfg.h"

#if defined(USE_FEE)
#include "Fee.h"
#endif

#if defined(USE_EA)
#include "Ea.h"
#endif

#if (MEMIF_VERSION_INFO_API == STD_ON)
#define _MemIf_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi, MEMIF)
#endif /* MEMIF_VERSION_INFO_API */

#if (MEMIF_NUMBER_OF_DEVICES == 0)
#define _MemIf_SetMode(_mode)
#define _MemIf_Read(_deviceIndex,_blockNumber,_blockOffset,_dataBufferPtr,_length)
#define _MemIf_Write(_deviceIndex,_blockNumber,_dataBufferPtr)
#define _MemIf_Cancel(_deviceIndex)
#define _MemIf_GetStatus(_deviceIndex)
#define _MemIf_GetJobResult(_deviceIndex)
#define _MemIf_InvalidateBlock(_deviceIndex,_blockNumber)
#define _MemIf_EraseImmediateBlock(_deviceIndex,_blockNumber)
#endif

#if (MEMIF_NUMBER_OF_DEVICES == 1)
#if (MEMIF_DEVICE_TO_USE == FLS_DRIVER_INDEX)
#define _MemIf_SetMode(_mode)	Fee_SetMode(_mode)
#define _MemIf_Read(_deviceIndex,_blockNumber,_blockOffset,_dataBufferPtr,_length)	Fee_Read(_blockNumber,_blockOffset,_dataBufferPtr,_length)
#define _MemIf_Write(_deviceIndex,_blockNumber,_dataBufferPtr)	Fee_Write(_blockNumber,_dataBufferPtr)
#define _MemIf_Cancel(_deviceIndex)	Fee_Cancel()
#define _MemIf_GetStatus(_deviceIndex)	Fee_GetStatus()
#define _MemIf_GetJobResult(_deviceIndex)	Fee_GetJobResult()
#define _MemIf_InvalidateBlock(_deviceIndex,_blockNumber)	Fee_InvalidateBlock(_blockNumber)
#define _MemIf_EraseImmediateBlock(_deviceIndex,_blockNumber)	Fee_EraseImmediateBlock(_blockNumber)

#elif (MEMIF_DEVICE_TO_USE == EEP_DRIVER_INDEX)
#define _MemIf_SetMode(_mode)	Ea_SetMode(_mode)
#define _MemIf_Read(_deviceIndex,_blockNumber,_blockOffset,_dataBufferPtr,_length)	Ea_Read(_blockNumber,_blockOffset,_dataBufferPtr,_length)
#define _MemIf_Write(_deviceIndex,_blockNumber,_dataBufferPtr)	Ea_Write(_blockNumber,_dataBufferPtr)
#define _MemIf_Cancel(_deviceIndex)	Ea_Cancel()
#define _MemIf_GetStatus(_deviceIndex)	Ea_GetStatus()
#define _MemIf_GetJobResult(_deviceIndex)	Ea_GetJobResult()
#define _MemIf_InvalidateBlock(_deviceIndex,_blockNumber)	Ea_InvalidateBlock(_blockNumber)
#define _MemIf_EraseImmediateBlock(_deviceIndex,_blockNumber)	Ea_EraseImmediateBlock(_blockNumber)
#else
#error "Memory device unknown (MEMIF_DEVICE_TO_USE)"
#endif

#endif

#if (MEMIF_NUMBER_OF_DEVICES > 1) || defined(CFG_MEMIF_NO_MACRO)
#if (MEMIF_NUMBER_OF_DEVICES > 1)
#error "Support for more than one device is not implemented yet!"
#endif
void MemIf_SetMode(MemIf_ModeType Mode);
Std_ReturnType MemIf_Read(uint8 DeviceIndex, uint16 BlockNumber, uint16 BlockOffset, uint8 *DataBufferPtr, uint16 Length);
Std_ReturnType MemIf_Write(uint8 DeviceIndex, uint16 BlockNumber, uint8 *DataBufferPtr);
void MemIf_Cancel(uint8 DeviceIndex);
MemIf_StatusType MemIf_GetStatus(uint8 DeviceIndex);
MemIf_JobResultType MemIf_GetJobResult(uint8 DeviceIndex);
Std_ReturnType MemIf_InvalidateBlock(uint8 DeviceIndex, uint16 BlockNumber);
Std_ReturnType MemIf_EraseImmediateBlock(uint8 DeviceIndex, uint16 BlockNumber);
#else
#define MemIf_SetMode(_mode)						_MemIf_SetMode(_mode)
#define MemIf_Read(_deviceIndex,_blockNumber,_blockOffset,_dataBufferPtr,_length)	_MemIf_Read(_deviceIndex, _blockNumber,_blockOffset,_dataBufferPtr,_length)
#define MemIf_Write(_deviceIndex,_blockNumber,_dataBufferPtr)	_MemIf_Write(_deviceIndex,_blockNumber,_dataBufferPtr)
#define MemIf_Cancel(_deviceIndex)	_MemIf_Cancel(_deviceIndex)
#define MemIf_GetStatus(_deviceIndex)	_MemIf_GetStatus(_deviceIndex)
#define MemIf_GetJobResult(_deviceIndex)	_MemIf_GetJobResult(_deviceIndex)
#define MemIf_InvalidateBlock(_deviceIndex,_blockNumber)	_MemIf_InvalidateBlock(_deviceIndex,_blockNumber)
#define MemIf_EraseImmediateBlock(_deviceIndex,_blockNumber)	_MemIf_EraseImmediateBlock(_deviceIndex,_blockNumber)
#endif

#endif /*MEMIF_H_*/
