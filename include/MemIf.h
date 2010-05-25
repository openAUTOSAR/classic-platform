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

/* TODO: Not really know how this is connected with the rest, yet :) */
#define MemIf_SetMode( _mode )
#define MemIf_Read(_deviceIndex,_blockNumber,_blockOffset,_dataBufferPtr,_length)
#define MemIf_Write(_deviceIndex,_blockNumber,_dataBufferPtr)
#define MemIf_Cancel( _deviceIndex )
#define MemIf_GetStatus( _deviceIndex )
#define MemIf_GetJobResult( _deviceIndex )
#define MemIf_InvalidateBlock( _deviceIndex, _block )
#define MemIf_GetVersionInfo( _versionInfo )
#define MemIf_EraseImmediateBlock( _deviceIndex,_blockNumber );

#endif /*MEMIF_H_*/
