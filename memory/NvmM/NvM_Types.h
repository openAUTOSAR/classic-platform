/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/








#ifndef NVM_TYPES_H_
#define NVM_TYPES_H_

#include "Std_Types.h"

typedef uint8 NvM_RequestResultType;

typedef uint16 NvM_BlockIdType;

void NvM_Init( void );
void NvM_SetDataIndex( NvM_BlockIdType BlockId, uint8 DataIndex );
void NvM_GetDataIndex( NvM_BlockIdType BlockId,uint8 *DataIndexPtr );
void NvM_SetBlockProtection( NvM_BlockIdType BlockId, boolean ProtectionEnabled );
void NvM_GetErrorStatus( NvM_BlockIdType BlockId, uint8 *RequestResultPtr );
void NvM_GetVersionInfo( Std_VersionInfoType *VersionInfo );
void Nvm_SetRamBlockStatus( NvM_BlockIdType BlockId, boolean BlockChanged );
void Nvm_ReadBlock( NvM_BlockIdType BlockId, uint8 *NvM_DstPtr );
void NvM_WriteBlock( NvM_BlockIdType BlockId, const uint8 *NvM_SrcPtr );
void Nvm_RestoreBlockDefaults( NvM_BlockIdType BlockId, uint8 *NvM_DstPtr );
void NvM_EraseNvBlock( NvM_BlockIdType BlockId );
void NvM_CancelWriteAll( void );
void NvM_InvalidateNvBlock( NvM_BlockIdType BlockId );
void NvM_ReadAll( void );
void NvM_WriteAll( void );

#endif /*NVM_TYPES_H_*/
