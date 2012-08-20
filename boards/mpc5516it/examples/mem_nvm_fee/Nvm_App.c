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

/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/


uint8 TEST_RamBlock_Dataset_0[20];
uint8 TEST_RamBlock_Dataset_1[2];
uint8 TEST_tmpRam[100];



/* ----------------------------[private functions]---------------------------*/

static void nvmApplication(void);

/* ----------------------------[public functions]----------------------------*/


Std_ReturnType TEST_SingleBlockFunctionCallbackBlock3(uint8 ServiceId, NvM_RequestResultType JobResult)
{
	return E_OK;
}

void TEST_MultiBlockFunctionCallback(uint8 ServiceId, NvM_RequestResultType JobResult)
{
}


#define PATTERN_FILL(_block) patternFill(_block,sizeof(_block))
#define PATTERN_VERIFY(_block) patternVerify(_block,sizeof(_block))

void patternFill( uint8 *bPtr, uint32 numBytes )
{
	int i;
	for(i=0;i< numBytes; i++) {
		bPtr[i] = i;
	}
}


int patternVerify( uint8 *bPtr, uint32 numBytes )
{
	int i;
	for(i=0;i< numBytes; i++) {
		if( bPtr[i] != i ) {
			return 1;
		}
	}

	return 0;
}

void TEST_MultiBlockCallback( void ) {

}

TASK(Application) {
	for(;;) {
		nvmApplication();
	}
}



/* NvM_ReadAll()
 *
 * Read all blocks just to see that everything is as it should.
 * */
static void nvmApplication(void) {
	NvM_RequestResultType errorStatus;
	Std_ReturnType rv;
	int currBlock;

	/* All reads should be OK */
//	feeReadStatus.blockNumber = ALL_BLOCKS;
//	feeReadStatus.rv = E_OK;

//	NvM_ReadAll();
//	TEST_ASSERT( WaitToComplete( 0 , 30 ) == NVM_REQ_OK );

	currBlock = NVM_block_1_dataset_HANDLE;

	PATTERN_FILL(TEST_RamBlock_Dataset_0);
	NvM_WriteBlock(currBlock,TEST_RamBlock_Dataset_0);
//	TEST_ASSERT( 0 == PATTERN_VERIFY(TEST_RamBlock_Dataset_0));

	do {
		NvM_GetErrorStatus(currBlock, &errorStatus);
	} while( errorStatus != NVM_REQ_OK );

	memset(TEST_RamBlock_Dataset_0,0,sizeof(TEST_RamBlock_Dataset_0));
	NvM_ReadBlock(currBlock,TEST_RamBlock_Dataset_0);

	do {
		NvM_GetErrorStatus(currBlock, &errorStatus);
	} while( errorStatus != NVM_REQ_OK );

//	TEST_ASSERT( 0 == PATTERN_VERIFY(TEST_RamBlock_Dataset_0));
}


void Task_Application(void) {

}


