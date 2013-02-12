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

/*
 * flash_h7f_c90.c
 *
 *  Created on: 29 aug 2011
 *      Author: mahi
 *
 * Interface for the low level flash written by freescale (flash_ll_h7f_c90.c )
 *
 * This file aims to support support all mpc55xx as well as mpc56xx.
 */

/* ----------------------------[includes]------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "flash_ll_h7f_c90.h"
#include "typedefs.h"
#include "io.h"
//#include "flash_mpc5xxx.h"
#include "Fls_Cfg.h"

#define USE_DEBUG_PRINTF
#include "debug.h"

#define ASSERT(_x)  assert(_x)

/* ----------------------------[private define]------------------------------*/

#define PFLASH_CTRL_BASE 	0xFFE8_8000
#define PFCR0				(PFLASH_CTRL_BASE + 0x1c)
#define PFCR1				(PFLASH_CTRL_BASE + 0x20)
#define PFAPR				(PFLASH_CTRL_BASE + 0x24)

#define NVLML_LME			(1u<<31)


//#define FLASH_BANK_CNT				3
#define PASSWORD_LOW_MID  			0xA1A11111UL
#define PASSWORD_HIGH				0xB2B22222UL
#define PASSWORD_SECONDARY_LOW_MID	0xC3C33333UL

/* ----------------------------[private macro]-------------------------------*/

/* Check if two ranges overlap (_a0->_a1 is first range ) */
#define OVERLAP(_a0,_a1, _b0, _b1 ) ( ( ((_a0) <= (_b0)) && ((_b0) <= (_a1)) ) || \
									  ( ((_b0) <= (_a0)) && ((_a0) <= (_b1)) ) )

/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/

/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/

/**
 * Convert address to strange block format that freescale likes.
 *
 * @param addr
 * @param size
 * @param fb
 * @return
 */
static bool getAffectedBlocks( const FlashType *bPtr, uintptr_t addr, size_t size,	uint32_t (*fb)[ADDR_SPACE_CNT] ) {
	uint16_t addrSpace;
	bool anyAffected = false;


	memset(fb, 0, sizeof(*fb) );

	/* Check if sector range overlaps */
	for (int sector = 0; sector < bPtr->sectCnt; sector++)
	{
		if (OVERLAP( addr,addr+size-1,
				bPtr->sectAddr[sector],bPtr->sectAddr[sector+1]-1))
		{
			addrSpace = bPtr->addrSpace[sector];
			(*fb)[ADDR_SPACE_GET(addrSpace)] |= (1 << ADDR_SPACE_GET_SECTOR(addrSpace));
			anyAffected = true;
		}
	}
	return anyAffected;
}


/**
 * Setup the flash
 */

void Flash_Init(void) {

	/* TODO: Does freescale setup the platform flash controller with sane
	 * values, or not?
	 */
}

uint32_t Flash_Lock(const FlashType *fPtr, uint32_t op, uintptr_t from, uint32_t size) {
	uint32_t flashBlocks[ADDR_SPACE_CNT];
	int bank;
	const FlashType *bPtr;
	uint32_t regAddr;
	uint32_t lock;

	LDEBUG_PRINTF("Flash_lock from:%p size:%ul\n");

	for (bank = 0; bank < FLASH_BANK_CNT; bank++) {
		bPtr = &fPtr[bank];

		getAffectedBlocks(bPtr, from, size, &flashBlocks);

		/* ---------- Low/Mid ---------- */
	    lock = (flashBlocks[ADDR_SPACE_MID]<<16) | flashBlocks[ADDR_SPACE_LOW];
	    if( lock != 0 ) {
			regAddr = bPtr->regBase + C90FL_LML;

			/* Unlock LML (enable LME bit) */
			if ( (READ32(regAddr) & NVLML_LME) == 0 ) {
				WRITE32(regAddr,PASSWORD_LOW_MID);
			}
			/* lock/unlock */
			if( op & FLASH_OP_UNLOCK ) {
				WRITE32(regAddr,(~lock) & READ32(regAddr) );
			} else {
				WRITE32(regAddr,(lock) | READ32(regAddr) );
			}

			regAddr = bPtr->regBase + C90FL_SLL;
			/* Unlock secondary, SLL (enable LME bit) */
			if ( (READ32(regAddr) & NVLML_LME) == 0 ) {
				WRITE32(regAddr,PASSWORD_SECONDARY_LOW_MID);
			}
			/* lock/unlock */
			if( op & FLASH_OP_UNLOCK ) {
				WRITE32(regAddr,(~lock) & READ32(regAddr) );
			} else {
				WRITE32(regAddr,(lock) | READ32(regAddr) );
			}

	    }

		/* ---------- high ----------*/
	    lock = flashBlocks[ADDR_SPACE_HIGH];
	    if( lock != 0 ) {
	    	regAddr = bPtr->regBase + C90FL_HBL;
	    	/* Unlock LML (enable LME bit) */
			if ( (READ32(regAddr) & NVLML_LME) == 0 ) {
				WRITE32(regAddr,PASSWORD_HIGH);
			}

			/* lock/unlock */
			if( op & FLASH_OP_UNLOCK ) {
				WRITE32(regAddr,(~lock) & READ32(regAddr) );
			} else {
				WRITE32(regAddr,(lock) | READ32(regAddr) );
			}
	    }
	}
	return 0;
}

/**
 *
 * @param fPtr
 * @param dest
 * @param size
 * @return
 */
uint32_t Flash_Erase( const FlashType *fPtr, uintptr_t dest, uint32_t size, flashCbType cb) {
	uint32_t rv;
	uint32_t flashBlocks[ADDR_SPACE_CNT];
	const FlashType *bPtr;
	bool affected;
	(void)cb;

	/* FSL functions are for each bank, so loop over banks */
	for (int bank = 0; bank < FLASH_BANK_CNT; bank++) {
		bPtr = &fPtr[bank];

		affected = getAffectedBlocks(bPtr, dest, size, &flashBlocks);
		if( affected == false ) {
			continue;
		}

		rv = FSL_FlashEraseStart(bPtr->regBase, dest, flashBlocks[0],
				flashBlocks[1], flashBlocks[2]);

		if (rv != EE_OK) {
			return EE_ERROR_PE_OPT;
		}
	}
	return EE_OK;
}


/**
 *
 * @param to     Flash address to write. Updated to next address to write for each call
 * @param from   RAM address to read from. Updated with next address to read from for each call
 * @param size   The size to program. Updated for each for each call.
 * @return
 */
uint32_t Flash_ProgramPageStart( const FlashType *fPtr, uint32_t *to, uint32_t * from, uint32_t * size, flashCbType cb) {
    const FlashType *bPtr;
    bool affected;
    (void)cb;


    /* Check double word alignment */
    ASSERT((*size % 8) == 0 );

    /* FSL functions are for each bank, so loop over banks */
    for (int bank = 0; bank < FLASH_BANK_CNT; bank++) {
        bPtr = &fPtr[bank];

        affected = OVERLAP( *to,
        		*to + *size - 1,
        		bPtr->sectAddr[0],
        		bPtr->sectAddr[0] + bPtr->bankSize-1);
        if( affected == false ) {
            /* This bank was not affected */
            continue;
        }

        return FSL_FlashProgramStart(bPtr->regBase, to, size, from);
    }

    return EE_OK;
}

/**
 * Check status of programming/erase
 *
 * @param fPtr
 * @param to
 * @return
 */
uint32_t Flash_CheckStatus( const FlashType *fPtr, uint32_t *to, uint32_t size ) {
    const FlashType *bPtr;
    bool affected;
    uint32_t rv = E_NOT_OK;
    uintptr_t flAddr = (uintptr_t)to;

    for (int bank = 0; bank < FLASH_BANK_CNT; bank++) {
        bPtr = &fPtr[bank];

        /* We only need to figure out what bank is used, note that multiple banks
         * must be handled at the same time here */
        affected = OVERLAP(flAddr,flAddr+size-1,bPtr->sectAddr[0],bPtr->sectAddr[0] + bPtr->bankSize-1);
        if( affected == false ) {
            /* This bank was not affected */
            continue;
        }

	    rv = FSL_FlashCheckStatus(bPtr->regBase);
	    if( rv != EE_OK) {
	        return rv;
	    }
    }

    return rv;
}


