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
 * EXAMPLE
 *   Just a simple example that uses the NvM API's.
 *
 */



/* ----------------------------[includes]------------------------------------*/
#include "Std_Types.h"
#include "Os.h"
#include "Det.h"
#include "stdio.h"
#include "NvM.h"
#include "NvM_Cbk.h"
#include "MemIf.h"
#include <string.h>
#include <assert.h>

/* ----------------------------[private define]------------------------------*/

/* STD_ON  - Block_2_Native will be written. Block_2_Native will only
 *           be called once.
 * STD_OFF - Block_2_Native's will be called at every startup since no
 *           data is ever written to the block
 */
#define WRITE_BLOCK_2		STD_OFF

/* ----------------------------[private macro]-------------------------------*/

#define PATTERN_FILL(_block,_st_val) patternFill(_block,sizeof(_block),_st_val)
#define PATTERN_VERIFY(_block,_st_val) patternVerify(_block,sizeof(_block),_st_val)

#define BUSY_WAIT(_block,_var)	\
	do { \
		NvM_GetErrorStatus(_block, &_var); \
	} while( _var != NVM_REQ_OK );


/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/


uint8 TEST_RamBlock_Dataset_0[20];
uint8 TEST_RamBlock_Dataset_1[2];
uint8 TEST_tmpRam[100];

uint8 Block_2_RamBlock[10];

/* ----------------------------[private functions]---------------------------*/

static void nvmApplication(void);

/**
 * Fill memory with a increasing number starting with startVal
 *
 * @param bPtr
 * @param numBytes
 * @param startVal
 */
static void patternFill( uint8 *bPtr, uint32 numBytes, uint8 startVal )
{
	int i;
	for(i=0;i< numBytes; i++) {
		bPtr[i] = startVal++;
	}
}

/**
 * Verify a pattern filled by patternFill.
 *
 * @param bPtr
 * @param numBytes
 * @param startVal
 * @return
 */
static int patternVerify( uint8 *bPtr, uint32 numBytes, uint8 startVal )
{
	int i;
	for(i=0;i< numBytes; i++, startVal++) {
		if( bPtr[i] != startVal ) {
			return 1;
		}
	}

	return 0;
}


/* ----------------------------[public functions]----------------------------*/

void MultiBlock_Callback(uint8 ServiceId, NvM_RequestResultType JobResult) {

}

/*
 * Block_1_DataSet
 */

void Block_Dataset_SingleCallback( void ) {
}

/*
 * Block_2_Native
 */

void Block_2_InitCallback( void ) {
	PATTERN_FILL(Block_2_RamBlock,10);
}


/*
 * The application task
 */
TASK(Application) {
	for(;;) {
		nvmApplication();
	}
}

/**
 *
 */
static void nvmApplication(void) {
	NvM_RequestResultType errorStatus;
	Std_ReturnType rv;
	int currBlock;
	static int counter = 0;

	/* Verify that a block read during NvM_ReadAll() is there */
	rv = PATTERN_VERIFY(Block_2_RamBlock,10);
	assert(rv == 0);

	currBlock = NVM_Block_1_Dataset_HANDLE;

	/*
	 * Write some pattern to Dataset 0
	 */
	PATTERN_FILL(TEST_RamBlock_Dataset_0,0);
	NvM_WriteBlock(currBlock,TEST_RamBlock_Dataset_0);
	BUSY_WAIT(currBlock,&errorStatus);

	memset(TEST_RamBlock_Dataset_0,0,sizeof(TEST_RamBlock_Dataset_0));
	NvM_ReadBlock(currBlock,TEST_RamBlock_Dataset_0);
	BUSY_WAIT(currBlock,&errorStatus);

	rv = PATTERN_VERIFY(TEST_RamBlock_Dataset_0,0);
	assert(rv == 0);

	/*
	 * Write some pattern to Dataset 1
	 */
	rv = NvM_SetDataIndex(currBlock,1);
	assert( rv == E_OK );
	BUSY_WAIT(currBlock,&errorStatus);

	PATTERN_FILL(TEST_RamBlock_Dataset_0,5);
	NvM_WriteBlock(currBlock,TEST_RamBlock_Dataset_0);
	BUSY_WAIT(currBlock,&errorStatus);

	memset(TEST_RamBlock_Dataset_0,0,sizeof(TEST_RamBlock_Dataset_0));
	NvM_ReadBlock(currBlock,TEST_RamBlock_Dataset_0);
	BUSY_WAIT(currBlock,&errorStatus);

	rv = PATTERN_VERIFY(TEST_RamBlock_Dataset_0,5);
	assert(rv == 0);


	/*
	 * Back to Dataset 0 and check data.
	 */
	NvM_SetDataIndex(currBlock,0);
	assert( rv == E_OK );
	BUSY_WAIT(currBlock,&errorStatus);

	NvM_ReadBlock(currBlock,TEST_RamBlock_Dataset_0);
	BUSY_WAIT(currBlock,&errorStatus);

	rv = PATTERN_VERIFY(TEST_RamBlock_Dataset_0,0);
	assert(rv == 0);


#if (WRITE_BLOCK_2 == STD_ON)

	currBlock = NVM_Block_2_Native_HANDLE;

	rv = NvM_WriteBlock(currBlock,NULL);
	assert( rv == E_OK );

	do {
		NvM_GetErrorStatus(currBlock, &errorStatus);
	} while( errorStatus != NVM_REQ_OK );

#endif

}


