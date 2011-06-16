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








#ifndef FLS_C90FL_H_
#define FLS_C90FL_H_

#define FLS_ERASE_SECTION __attribute__ ((section (".fls_erase")));
#define FLS_WRITE_SECTION __attribute__ ((section (".fls_write")));

#define C90FL_BUSY	0x1000

typedef struct {
	UINT32 dest;
	UINT32 size;
	UINT32 source;
	UINT32 pageSize;
	UINT32 sourceIndex;
	UINT32 temp;
	UINT32 state;
} Fls_ProgInfoType;

typedef struct {
	UINT32 state;
} Fls_EraseInfoType;

typedef struct {
	UINT32 lowEnabledBlocks;
	UINT32 midEnabledBlocks;
	UINT32 highEnabledBlocks;
	// 1 - primary, 2 - secondary
	UINT32 shadowBlocks;
} Fls_EraseBlockType;

// TODO : document API
UINT32 Fls_C90FL_FlashErase ( PSSD_CONFIG pSSDConfig,
                    BOOL shadowFlag,
                    UINT32 lowEnabledBlocks,
                    UINT32 midEnabledBlocks,
                    UINT32 highEnabledBlocks,
                    Fls_EraseInfoType *eraseInfo); //FLS_ERASE_SECTION;

UINT32 Fls_C90FL_EraseStatus (  PSSD_CONFIG pSSDConfig  );// FLS_ERASE_SECTION ;

UINT32 Fls_C90FL_Program ( PSSD_CONFIG pSSDConfig, Fls_ProgInfoType *pInfo ); //FLS_WRITE_SECTION ;

void Fls_C90FL_SetLock ( PSSD_CONFIG pSSDConfig, Fls_EraseBlockType *blocks, UINT8 logic );
#endif /*FLS_C90FL_H_*/
