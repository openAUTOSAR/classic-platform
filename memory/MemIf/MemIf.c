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

/*
 * Note!
 *
 * This file is controlled by config CFG_MEMIF_NO_MACRO and MEMIF_NUMBER_OF_DEVICES .
 */

#include "MemIf.h"

#if (MEMIF_NUMBER_OF_DEVICES > 1) || defined(CFG_MEMIF_NO_MACRO)

void MemIf_SetMode(MemIf_ModeType Mode) {
	return _MemIf_SetMode(Mode);
}

Std_ReturnType MemIf_Read(uint8 DeviceIndex, uint16 BlockNumber, uint16 BlockOffset, uint8 *DataBufferPtr, uint16 Length) {
	return _MemIf_Read(DeviceIndex, BlockNumber , BlockOffset, DataBufferPtr, Length );
}

Std_ReturnType MemIf_Write(uint8 DeviceIndex, uint16 BlockNumber, uint8 *DataBufferPtr) {
	return _MemIf_Write(DeviceIndex, BlockNumber, DataBufferPtr );
}

void MemIf_Cancel(uint8 DeviceIndex) {
	return _MemIf_Cancel(DeviceIndex);
}
MemIf_StatusType MemIf_GetStatus(uint8 DeviceIndex) {
	return _MemIf_GetStatus(DeviceIndex);
}
MemIf_JobResultType MemIf_GetJobResult(uint8 DeviceIndex) {
	return _MemIf_GetJobResult(DeviceIndex);
}
Std_ReturnType MemIf_InvalidateBlock(uint8 DeviceIndex, uint16 BlockNumber) {

	return _MemIf_InvalidateBlock(DeviceIndex,BlockNumber);
}
Std_ReturnType MemIf_EraseImmediateBlock(uint8 DeviceIndex, uint16 BlockNumber) {
	return _MemIf_EraseImmediateBlock( DeviceIndex, BlockNumber );
}
#endif

