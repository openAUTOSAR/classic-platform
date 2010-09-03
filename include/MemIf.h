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








#ifndef MEMIF_H_
#define MEMIF_H_
#include "Std_Types.h"
#include "MemIf_Cfg.h"

#define MEMIF_MODULE_ID			MODULE_ID_MEMIF
#define MEMIF_VENDOR_ID			1

#define MEMIF_SW_MAJOR_VERSION	1
#define MEMIF_SW_MINOR_VERSION	0
#define MEMIF_SW_PATCH_VERSION	0
#define MEMIF_AR_MAJOR_VERSION	3
#define MEMIF_AR_MINOR_VERSION	0
#define MEMIF_AR_PATCH_VERSION	1

#if ( MEMIF_VERSION_INFO_API == STD_ON )
#define MemIf_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi, MEMIF)
#endif /* MEMIF_VERSION_INFO_API */

#if (MEMIF_NUMBER_OF_DEVICES == 0)
#define MemIf_SetMode(_mode)
#define MemIf_Read(_deviceIndex,_blockNumber,_blockOffset,_dataBufferPtr,_length)
#define MemIf_Write(_deviceIndex,_blockNumber,_dataBufferPtr)
#define MemIf_Cancel(_deviceIndex)
#define MemIf_GetStatus(_deviceIndex)
#define MemIf_GetJobResult(_deviceIndex)
#define MemIf_InvalidateBlock(_deviceIndex, _block)
#define MemIf_GetVersionInfo(_versionInfo)
#define MemIf_EraseImmediateBlock(_deviceIndex,_blockNumber);
#endif

#if (MEMIF_NUMBER_OF_DEVICES == 1)
#endif

#if (MEMIF_NUMBER_OF_DEVICES > 1)
#endif

#endif /*MEMIF_H_*/
