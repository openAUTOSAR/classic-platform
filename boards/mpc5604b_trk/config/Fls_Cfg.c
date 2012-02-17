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

#include "Fls.h"
#include "flash.h"
#include <stdlib.h>


#if defined(CFG_MPC5606S) || defined(CFG_MPC5604B)

/* TODO: This can actually be read from the flash instead */
const FlashType flashInfo[] = {
	/* NO RWW */

	/* Bank 0, Array 0 (LOW) */
	[0].sectCnt = 8,
	[0].bankSize = 0x80000,
//	[0].bankRange = BANK_RANGE_CODE_LOW,
	[0].regBase = 0xC3F88000UL,
	[0].sectAddr[0] = 0, 	   /* 0, B0F0, LOW  */
	[0].addrSpace[0] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 0,
	[0].sectAddr[1] = 0x08000, /* 1, B0F1, LOW */
	[0].addrSpace[1] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 1,
	[0].sectAddr[2] = 0x0c000, /* 2, B0F2, LOW */
	[0].addrSpace[2] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 2,
	[0].sectAddr[3] = 0x10000, /* 3, B0F3, LOW */
	[0].addrSpace[3] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 3,
	[0].sectAddr[4] = 0x18000, /* 4, B0F4, LOW */
	[0].addrSpace[4] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 4,
	[0].sectAddr[5] = 0x20000, /* 5, B0F5, LOW */
	[0].addrSpace[5] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 5,
	[0].sectAddr[6] = 0x40000, /* 6, B0F6, MID */
	[0].addrSpace[6] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 0,
	[0].sectAddr[7] = 0x60000, /* 7, B0F7, MID */
	[0].addrSpace[7] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 1,
	[0].sectAddr[8] = 0x80000,	/* End, NOT a sector */

	/* Bank 1, Data */
	[1].sectCnt = 4,
	[1].bankSize = 0x810000 - 0x800000,
	[1].regBase = 0xC3F8C000UL,
	[1].sectAddr[0] = 0x800000,  /* LOW */
	[1].addrSpace[0] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 0,
	[1].sectAddr[1] = 0x804000,  /* LOW */
	[1].addrSpace[1] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 1,
	[1].sectAddr[2] = 0x808000,  /* LOW */
	[1].addrSpace[2] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 2,
	[1].sectAddr[3] = 0x80c000,  /* LOW */
	[1].addrSpace[3] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 3,
	[1].sectAddr[4] = 0x810000, /* End, NOT a sector */

	/* Bank 2, Array 1 (MID)*/
	[2].sectCnt = 4,
	[2].bankSize = 0x100000-0x80000,
	[2].regBase = 0xC3FB0000UL,
	[2].sectAddr[0] = 0x80000,  /* 0, B2F0, LOW  */
	[2].addrSpace[0] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 0,
	[2].sectAddr[1] = 0xa0000,	/* 1, B2F1, LOW  */
	[2].addrSpace[1] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 1,
	[2].sectAddr[2] = 0xc0000, 	/* 2, B2F2, MID  */
	[2].addrSpace[2] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 2,
	[2].sectAddr[3] = 0xe0000,  /* 3, B2F3, MID  */
	[2].addrSpace[3] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 3,
	[2].sectAddr[4] = 0x100000, /* End, NOT a sector */
};
#elif defined(CFG_MPC5668G)
	/* RWW between partitions*
	 *
	 * LOW   8*16KB + 2*64KB   = 256KB
	 * MID   2*128KB           = 256KB
	 * HIGH  2*256KB           = 1.5MB
	 *   	 2*256KB
	 *       2*256KB
	 */

const FlashType flashInfo[] = {
	/* LOW */
	[0].sectCnt = 18,
	[0].bankSize = 0x200000,
	[0].regBase = 0xffff8000UL,
	[0].sectAddr[0] = 0, 	     /* 0, B0F0, LOW  */
	[0].addrSpace[0] = ADDR_SPACE(0, ADDR_SPACE_LOW, 1 ),
	[0].sectAddr[1] = 0x00004000, /* 1, B0F1, LOW */
	[0].addrSpace[1] = ADDR_SPACE(1, ADDR_SPACE_LOW, 1 ),
	[0].sectAddr[2] = 0x00008000, /* 2, B0F2, LOW */
	[0].addrSpace[2] = ADDR_SPACE(2, ADDR_SPACE_LOW, 1 ),
	[0].sectAddr[3] = 0x0000c000, /* 3, B0F3, LOW */
	[0].addrSpace[3] = ADDR_SPACE(3, ADDR_SPACE_LOW, 1 ),
	[0].sectAddr[4] = 0x00010000, /* 4, B0F4, LOW */
	[0].addrSpace[4] = ADDR_SPACE(4, ADDR_SPACE_LOW, 2 ),
	[0].sectAddr[5] = 0x00014000, /* 5, B0F5, LOW */
	[0].addrSpace[5] = ADDR_SPACE(5, ADDR_SPACE_LOW, 2 ),
	[0].sectAddr[6] = 0x00018000, /* 6, B0F6, LOW */
	[0].addrSpace[6] = ADDR_SPACE(6, ADDR_SPACE_LOW, 2 ),
	[0].sectAddr[7] = 0x0001c000, /* 7, B0F7, LOW */
	[0].addrSpace[7] = ADDR_SPACE(7, ADDR_SPACE_LOW, 2 ),
	[0].sectAddr[8] = 0x00020000, /* 8, B0F8, LOW */
	[0].addrSpace[8] = ADDR_SPACE(8, ADDR_SPACE_LOW, 3 ),
	[0].sectAddr[9] = 0x00030000, /* 9, B0F9, LOW */
	[0].addrSpace[9] = ADDR_SPACE(9, ADDR_SPACE_LOW, 3 ),

	/* MID */
	[0].sectAddr[10] = 0x00040000, /* 0, B0F9, LOW */
	[0].addrSpace[10] = ADDR_SPACE(0, ADDR_SPACE_MID, 4 ),
	[0].sectAddr[11] = 0x00060000, /* 1, B0F9, LOW */
	[0].addrSpace[11] = ADDR_SPACE(1, ADDR_SPACE_MID, 4 ),

	/* HIGH */
	[0].sectAddr[12] = 0x00080000, /* 0, B0F9, LOW */
	[0].addrSpace[12] = ADDR_SPACE(0, ADDR_SPACE_HIGH, 5 ),
	[0].sectAddr[13] = 0x000c0000, /* 1, B0F9, LOW */
	[0].addrSpace[13] = ADDR_SPACE(1, ADDR_SPACE_HIGH, 5 ),
	[0].sectAddr[14] = 0x00100000, /* 2, B0F9, LOW */
	[0].addrSpace[14] = ADDR_SPACE(2, ADDR_SPACE_HIGH, 6 ),
	[0].sectAddr[15] = 0x00140000, /* 3, B0F9, LOW */
	[0].addrSpace[15] = ADDR_SPACE(3, ADDR_SPACE_HIGH, 6 ),
	[0].sectAddr[16] = 0x00180000, /* 4, B0F9, LOW */
	[0].addrSpace[16] = ADDR_SPACE(4, ADDR_SPACE_HIGH, 7 ),
	[0].sectAddr[17] = 0x001c0000, /* 5, B0F9, LOW */
	[0].addrSpace[17] = ADDR_SPACE(5, ADDR_SPACE_HIGH, 7 ),
	[0].sectAddr[18] = 0x00200000,	/* End, NOT a sector */
};
#elif defined(CFG_MPC5516)
	/* RWW between partitions*
	 *
	 * LOW   8*16KB + 2*64KB   = 256KB
	 * MID   2*128KB           = 256KB
	 * HIGH  4*128KB           = 512KB
	 */

const FlashType flashInfo[] = {
	[0].sectCnt = 16,
	[0].bankSize = 0x100000,
	[0].regBase = 0xffff8000UL,

	/* LOW */
	[0].sectAddr[0] = 0,
	[0].addrSpace[0] = ADDR_SPACE(0, ADDR_SPACE_LOW, 1 ),
	[0].sectAddr[1] = 0x00004000,
	[0].addrSpace[1] = ADDR_SPACE(1, ADDR_SPACE_LOW, 1 ),
	[0].sectAddr[2] = 0x00008000,
	[0].addrSpace[2] = ADDR_SPACE(2, ADDR_SPACE_LOW, 1 ),
	[0].sectAddr[3] = 0x0000c000,
	[0].addrSpace[3] = ADDR_SPACE(3, ADDR_SPACE_LOW, 1 ),
	[0].sectAddr[4] = 0x00010000,
	[0].addrSpace[4] = ADDR_SPACE(4, ADDR_SPACE_LOW, 2 ),
	[0].sectAddr[5] = 0x00014000,
	[0].addrSpace[5] = ADDR_SPACE(5, ADDR_SPACE_LOW, 2 ),
	[0].sectAddr[6] = 0x00018000,
	[0].addrSpace[6] = ADDR_SPACE(6, ADDR_SPACE_LOW, 2 ),
	[0].sectAddr[7] = 0x0001c000,
	[0].addrSpace[7] = ADDR_SPACE(7, ADDR_SPACE_LOW, 2 ),
	[0].sectAddr[8] = 0x00020000,
	[0].addrSpace[8] = ADDR_SPACE(8, ADDR_SPACE_LOW, 3 ),
	[0].sectAddr[9] = 0x00030000,
	[0].addrSpace[9] = ADDR_SPACE(9, ADDR_SPACE_LOW, 3 ),

	/* MID */
	[0].sectAddr[10] = 0x00040000,
	[0].addrSpace[10] = ADDR_SPACE(0, ADDR_SPACE_MID, 4 ),
	[0].sectAddr[11] = 0x00060000,
	[0].addrSpace[11] = ADDR_SPACE(1, ADDR_SPACE_MID, 4 ),

	/* HIGH */
	[0].sectAddr[12] = 0x00080000,
	[0].addrSpace[12] = ADDR_SPACE(0, ADDR_SPACE_HIGH, 5 ),
	[0].sectAddr[13] = 0x000a0000,
	[0].addrSpace[13] = ADDR_SPACE(1, ADDR_SPACE_HIGH, 5 ),
	[0].sectAddr[14] = 0x000c0000,
	[0].addrSpace[14] = ADDR_SPACE(2, ADDR_SPACE_HIGH, 6 ),
	[0].sectAddr[15] = 0x000e0000,
	[0].addrSpace[15] = ADDR_SPACE(3, ADDR_SPACE_HIGH, 6 ),
	[0].sectAddr[16] = 0x00100000,	/* End, NOT a sector */
};
#elif defined(CFG_MPC5567)
/* RWW between partitions*
 *
 * LOW   2*16KB + 2*48KB + 2*64KB = 256KB
 * MID   2*128KB                  = 256KB
 * HIGH  12*128KB                 = 1.5MB
 */

const FlashType flashInfo[] = {
[0].sectCnt = 20,
[0].bankSize = 0x200000,
[0].regBase = 0xc3f88000UL,

/* LOW  */
[0].sectAddr[0] = 0,
[0].addrSpace[0] = ADDR_SPACE(0, ADDR_SPACE_LOW, 1 ),
[0].sectAddr[1] = 0x00004000,
[0].addrSpace[1] = ADDR_SPACE(1, ADDR_SPACE_LOW, 1 ),
[0].sectAddr[2] = 0x00010000,
[0].addrSpace[2] = ADDR_SPACE(2, ADDR_SPACE_LOW, 1 ),
[0].sectAddr[3] = 0x00018000,
[0].addrSpace[3] = ADDR_SPACE(3, ADDR_SPACE_LOW, 1 ),
[0].sectAddr[4] = 0x00020000,
[0].addrSpace[4] = ADDR_SPACE(4, ADDR_SPACE_LOW, 2 ),
[0].sectAddr[5] = 0x00030000,
[0].addrSpace[5] = ADDR_SPACE(5, ADDR_SPACE_LOW, 2 ),

/* MID */
[0].sectAddr[6] = 0x00040000,
[0].addrSpace[6] = ADDR_SPACE(0, ADDR_SPACE_MID, 3 ),
[0].sectAddr[7] = 0x00060000,
[0].addrSpace[7] = ADDR_SPACE(1, ADDR_SPACE_MID, 3 ),

/* HIGH */
[0].sectAddr[8] = 0x00080000,
[0].addrSpace[8] = ADDR_SPACE(0, ADDR_SPACE_HIGH, 4 ),
[0].sectAddr[9] = 0x000a0000,
[0].addrSpace[9] = ADDR_SPACE(1, ADDR_SPACE_HIGH, 4 ),
[0].sectAddr[10] = 0x000c0000,
[0].addrSpace[10] = ADDR_SPACE(2, ADDR_SPACE_HIGH, 5 ),
[0].sectAddr[11] = 0x000e0000,
[0].addrSpace[11] = ADDR_SPACE(3, ADDR_SPACE_HIGH, 5 ),
[0].sectAddr[12] = 0x00100000,
[0].addrSpace[12] = ADDR_SPACE(0, ADDR_SPACE_HIGH, 6 ),
[0].sectAddr[13] = 0x00120000,
[0].addrSpace[13] = ADDR_SPACE(1, ADDR_SPACE_HIGH, 6 ),
[0].sectAddr[14] = 0x00140000,
[0].addrSpace[14] = ADDR_SPACE(2, ADDR_SPACE_HIGH, 7 ),
[0].sectAddr[15] = 0x00160000,
[0].addrSpace[15] = ADDR_SPACE(3, ADDR_SPACE_HIGH, 7 ),
[0].sectAddr[16] = 0x00180000,
[0].addrSpace[16] = ADDR_SPACE(0, ADDR_SPACE_HIGH, 8 ),
[0].sectAddr[17] = 0x001a0000,
[0].addrSpace[17] = ADDR_SPACE(1, ADDR_SPACE_HIGH, 8 ),
[0].sectAddr[18] = 0x001c0000,
[0].addrSpace[18] = ADDR_SPACE(2, ADDR_SPACE_HIGH, 9 ),
[0].sectAddr[19] = 0x001e0000,
[0].addrSpace[19] = ADDR_SPACE(3, ADDR_SPACE_HIGH, 9 ),
[0].sectAddr[20] = 0x00200000,	/* End, NOT a sector */
};
#else
#error CPU NOT supported
#endif


const Fls_ConfigType FlsConfigSet[]=
{
  {
#if ( FLS_AC_LOAD_ON_JOB_START == STD_ON)
    .FlsAcWrite = __FLS_ERASE_RAM__,
    .FlsAcErase = __FLS_WRITE_RAM__,
#else
    .FlsAcWrite = NULL,
    .FlsAcErase = NULL,
#endif
    .FlsJobEndNotification = NULL,
    .FlsJobErrorNotification = NULL,
    .FlsInfo = flashInfo,
//    .FlsSectorList = &fls_evbSectorList[0],
//    .FlsSectorListSize = sizeof(fls_evbSectorList)/sizeof(Fls_SectorType),
//    .FlsBlockToPartitionMap = Fls_BlockToPartitionMap,
  }
};
