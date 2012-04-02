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
 * RamBlockDataAddress
 *   NULL is no permanent RAM block. Otherwise allocate the number of bytes in space (like the stack)
 *
 *
 * Understanding block numbering:
 *
 *  NVM_DATASET_SELECTION_BIT=2
 *
 *    NvBlockBaseNumber
 *          0	  Reserved (NVM478)
 *          1     NVM_BLOCK_NATIVE,    NvBlockNum=1
 *          2	  NVM_BLOCK_REDUNDANT, NvBlockNum=2
 *          3     NVM_BLOCK_DATASET,   NvBlockNum=4
 *
 *   NvM_ReadBlock( 0, ... )    - Reserved for "multi block requests"
 *   NvM_ReadBlock( 1, ... )    - Reserved for redundant NVRAM block which holds the configuration ID.
 *   NvM_ReadBlock( 2--x, ... ) - "Normal" blocks
 *
 *
 *  NvM_BlockIdType*)  NvBlockBaseNumber   EA_BLOCK_NUMBER
 *      0**)
 *      1**)                ***)
 *      2                   1				4, (5,6,7)  ****)
 *      3                   2				8, R9,(10,11)  ****)
 *      4                   3				12, D13,D14,D15 ****)
 *
 *  *) Type used in the API.
 *  **)   Reserved ID's
 *  ***)  Reserved ID
 *  ****) FEE/EA_BLOCK_NUMBER = NvBlockBaseNumber << NvmDatasetSelectionBits = NvBlockBaseNumber * 4
 *        () - Cannot be accesses due to NvBlockNum
 *        R9 - Redundant block
 *        Dx - Data blocks
 *
 *    SIZES
 *      Both NvM and EA/FEE have block sizes. NvM have NvNvmBlockLength (NVM479) and FEE/EA have EaBlockSize.
 *      FEE/EA also have virtual page that is the alignment of a block, with the smallest block=size of the virtual page.
 *
 *      So, who allocates space for this. FEE/EA only have EaBlockSize.
 *      NvM have NvmRamBlockDataAddress, NvmRomBlockDataAddress and mapping to a MemIf Blocks (FEE/EA blocks)
 *
 *      ASSUME: I can't really see a point for the FEE/EA EaBlockSize.. it will just a multiple of NvNvmBlockLength*NvBlockNum + overhead?
 *              Click-box in EA/FEE that leaves the size calculation to NvM?
 *              This also means that enabling NvmBlockUseCrc or set a block from NVM_BLOCK_NATIVE to NVM_BLOCK_DATASET would be "automatic"
 *              in calculation of the EaBlockSize.
 *
 *      So how much data should be read from MemIf if a CRC checksum is used. Assuming that we use a physical layout where the CRC is located
 *      after the NV Block it would be BALIGN(NvNvmBlockLength,4) + 4 bytes. The same applies to the RAM block (ROM block to not have CRC, NVM127)
 *
 * CRC
 *   NVM121: NvM_SetRamBlockStatus(), calculate CRC in background if NvmCalcRamBlockCrc==TRUE
 *   NVM362: NvM_ReadAll() , if (NvmCalcRamBlockCrc == TRUE && permanent RAM block) re-calc CRC.
 *
 *     NvmBlockUseCrc     - Global CRC switch. Space is allocated in both RAM and NV block
 *     NvmCalcRamBlockCrc - CRC re-calculation. For example during startup, you may don't want
 *                          to verify all the checksums again, but instead just copy from NV
 *                          to RAM.
 *
 *     There is also explicit RAM checksum calculations for example NVM253 (Nvm_WriteAll()) where a CRC calculation
 *     is requested (although not stated should be dependent on NvmBlockUseCrc only, not NvmCalcRamBlockCrc).
 *     You have to calculate the checksum at some point....
 *
 * QUEUES
 *   NVM185: Says "successful enqueueing a request".."set to NVM_REQ_PENDING"
 *   NVM380: The queue length for multi block request shall be one (NvM_ReadAll, NvM_WriteAll)
 *   NVM381+NVM567 : Multi block requests shall not be interrupted.
 *   NVM243+NVM245: Do not unqueue the multi block request until all single block queues are empty.
 *     So, when NvmState==IDLE and we have a multi-request
 *
 *
 *
 *
 *
 * SPEED
 *   To get some speed into this multiple thing must be done in the same MainFunction loop.
 *
 * MEMIF
 *   The interface is actually quite strange, so you may read from any address (through MemIf_Read())
 *   but MemIf_Write(..) only takes a logical block and a pointer to the data to write.
 *
 *   See two alternatives here:
 *    1. If the NVBlock also contains the checksum after the data then writing the data needs a
 *       RAM that is as big as the biggest RAM block + room for checksum.
 *    2. If checksums would be kept in a separate EA/FEE block the ALL the checksum need to be written.
 *       For example after a NvM_WriteBlock() the checksum block would need to be written. This
 *       will probably lead consistency problems also... what happens if we fail to write the checksum
 *       block?
 *
 *
 *
 * MANUAL
 *
 *
 *
 *   Provide Data for the first/initial read
 *     When a block have no
 *
 *     NVM085
 *     NVM061
 *     NVM083
 *
 *   Configuring CRCs
 *     BlockUseCrc (A):      If the block (both RAM and NV) should use CRC
 *     CalcRamBlockCrc (B):  If the permanent RAM block should re-calculate it's CRC.
 *
 *     A B
 *     ------------
 *     0 0  No error detection or recovery
 *     0 1  N/A
 *     1 0  ?
 *     1 1  ?
 *
 * RAM BLOCK VALID/UNCHANGED
 *   Figure 8 and 9 in 3.1.5/NVM is more confusing than good.
 *   What we have to know is:
 *    1. Initially the RAM block is in INVALID/UNCHANGED
 *    ALT 2. After a NvM_ReadAll() and all is well the state goes to VALID/UNCHANGED
 *    ALT 2. If ROM values are used we go to VALID/CHANGED (We failed to read from NVRAM)
 *
 *   For NvM_WriteAll()
 *   1. A block that is INVALID can't be written
 *   2. A block that is UNCHANGED should not be written.
 *   -> Write only blocks that are VALID/CHANGED.
 *
 *   VALID/UNCHANGED - RAM == NV
 *   VALID/CHANGED   - RAM != NV   (analog to cache memories, "dirty")
 *
 *   Analog to cache
 *     VALID/CHANGED state - Dirty (since RAM != NV)
 *     WriteBlock          - Flush (Flush the RAM block to NV)
 *     ReadBlock           - Invalidate (NV block is read to RAM)
 */


/*
 *  General requirements
 */
/** @req NVM076 */
/** @req NVM552 */
/** @req NVM689 */


/*
 * NB! Even though some code exist for handling RamCrc, the functionality is not complete
 * and shall not be used.
 */

//lint -esym(522,CalcCrc) // 522 PC-Lint exception for empty functions
//lint -emacro(904,VALIDATE_RV,VALIDATE_NO_RV) //904 PC-Lint exception to MISRA 14.7 (validate macros).

// Exception made as a result of that NVM_DATASET_SELECTION_BITS can be zero
//lint -emacro(835, BLOCK_BASE_AND_SET_TO_BLOCKNR) // 835 PC-lint: A zero has been given as right argument to operator '<<' or '>>'


/* ----------------------------[includes]------------------------------------*/

#include <assert.h>
#include "NvM.h"
#include "NvM_Cbk.h"
#include "Rte.h" // ???
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include "MemIf.h"
//#include "SchM_NvM.h"
#include "MemMap.h"
#include "cirq_buffer.h"
#include "Modules.h"
#include <stdio.h>
#include "io.h"
#include "Crc.h"
#include <string.h>

#define FIXME		0

//#define DEBUG_BLOCK	1
#if defined(DEBUG_BLOCK)
#define DEBUG_BLOCK_STATE(_str,_block,_state)       printf("%s BLOCK NR:%d STATE:%d\n",_str,_block, _state); fflush(stdout);
#define DEBUG_STATE(_state,_substate)				printf("MAIN_STATE:%s/%d\n",StateToStr[_state],_substate); fflush(stdout);
#define DEBUG_PRINTF(format,...) 					printf(format,## __VA_ARGS__ ); fflush(stdout);
#define DEBUG_CHECKSUM(_str,_crc)					printf("%s crc=%x\n",_str,_crc);
#else
#define DEBUG_BLOCK_STATE(_str,_block,_state)
#define DEBUG_STATE(_state,_substate)
#define DEBUG_PRINTF(format,...)
#define DEBUG_CHECKSUM(_str,_crc)
#endif


/* ----------------------------[private define]------------------------------*/



#define NVM_BLOCK_ALIGNMENT			4
#define NVM_CHECKSUM_LENGTH			4

#define NVM_BLOCK_OFFSET 		2





/* ----------------------------[private macro]-------------------------------*/

#define NVM_ASSERT(_exp)		if( !(_exp) ) { while(1) {}; } //assert(_exp)

#if  ( NVM_DEV_ERROR_DETECT == STD_ON )
#include "Det.h"
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_NVM, 0, _api, _err); \
        }


#define VALIDATE_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_NVM, 0, _api, _err); \
          return _rv; \
        }

#define VALIDATE_NO_RV(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_NVM, 0, _api, _err); \
          return; \
        }
#define DET_REPORTERROR(_module,_instance,_api,_err) Det_ReportError(_module,_instance,_api,_err)

#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_RV(_exp,_api,_err,_rv )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_module,_instance,_api,_err)
#endif

#define BLOCK_BASE_AND_SET_TO_BLOCKNR(_blockbase, _set)	((uint16)(_blockbase << NVM_DATASET_SELECTION_BITS) | _set)

#if defined(USE_DEM)
#define DEM_REPORTERRORSTATUS(_err,_ev ) Dem_ReportErrorStatus(_err, DEM_EVENT_STATUS_FAILED);
#else
#define DEM_REPORTERRORSTATUS(_err,_ev )
#endif


#define BLOCK_NR_FROM_PTR(_bptr)  		(((_bptr) - NvM_Config.BlockDescriptor + 1))   // sizeof(NvM_BlockDescriptorType))

#define CREATE_ENTRY(_val)	[_val] = #_val




/* ----------------------------[private typedef]-----------------------------*/

// State variable
typedef enum {
  NVM_UNINITIALIZED = 0,
  NVM_IDLE,
  NVM_READ_ALL,
  NVM_WRITE_ALL,
  NVM_READ_BLOCK,
  NVM_WRITE_BLOCK,
  NVM_RESTORE_BLOCK_DEFAULTS,
} NvmStateType;

char *StateToStr[20] = {
	CREATE_ENTRY(NVM_UNINITIALIZED),
	CREATE_ENTRY(NVM_IDLE),
	CREATE_ENTRY(NVM_READ_ALL),
	CREATE_ENTRY(NVM_WRITE_ALL),
	CREATE_ENTRY(NVM_READ_BLOCK),
	CREATE_ENTRY(NVM_WRITE_BLOCK),
 };


typedef enum {
	BLOCK_STATE_MEMIF_REQ,
//	BLOCK_STATE_START,
	BLOCK_STATE_MEMIF_PROCESS,
//	BLOCK_STATE_MEMIF_CRC_PROCESS,
	BLOCK_STATE_CALC_CRC,
//	BLOCK_STATE_MEMIF_PROCESS_CRC,
	BLOCK_STATE_CALC_CRC_WRITE,
	BLOCK_STATE_CALC_CRC_READ,
//	BLOCK_STATE_LOAD_FROM_NV,
} BlockStateType;

typedef enum {
	NS_INIT = 0,
	NS_PROSSING,
//	RB_PROCESSING,
} Nvm_SubStates;


union Nvm_CRC {
	uint16 crc16;
	uint32 crc32;
};


typedef struct {
	NvM_RequestResultType	ErrorStatus;			// Status from multi block requests i.e. Read/Write/CancelWrite-all
} AdministrativeMultiBlockType;

typedef struct {
	BlockStateType			BlockState;
	uint8					DataIndex;				// Selected data index if "Data Set" type
	boolean					BlockWriteProtected;	// Block write protected?
	NvM_RequestResultType	ErrorStatus;			// Status of block
	boolean					BlockChanged;			// Block changed?
	boolean					BlockValid;				// Block valid? (RAM block only?)
	uint8					NumberOfWriteFailed;	// Current write retry cycle
	union Nvm_CRC			RamCrc;
	union Nvm_CRC			NvCrc;					// The CRC of this block, read from NV
	void *					savedDataPtr;			//
	uint8 					crcLen;

} AdministrativeBlockType;

/*
typedef enum {
	NVM_OP_READ_BLOCK,
	NVM_OP_WRITE_BLOCK,
	NVM_OP_RESTORE_BLOCK_DEFAULTS,
} Nvm_OpType;
*/
#define OP_READ		0
#define OP_WRITE	1
#define NO_MULTIBLOCK	0
#define MULTIBLOCK		1


typedef struct {
	NvmStateType		op;
	NvM_BlockIdType blockId;
	uint8 *			dataPtr;	/* Src or Dest ptr */
} Nvm_QueueType;



/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/

static NvmStateType 				nvmState = NVM_UNINITIALIZED;

#define RB_START			0
#define RB_WAIT_READ		1
#define RB_CALC_CHECKSUM	2

static int 							nvmSubState = 0;
//static int nvmSetNr;
static AdministrativeBlockType 		AdminBlock[NVM_NUM_OF_NVRAM_BLOCKS];
static AdministrativeMultiBlockType AdminMultiBlock;


//static Nvm_QueueType  nvmQueueImmData[NVM_SIZE_IMMEDIATE_JOB_QUEUE];
static Nvm_QueueType  nvmQueueData[NVM_SIZE_STANDARD_JOB_QUEUE];

uint8 Nvm_WorkBuffer[200];		/* TODO */

#if (NVM_SIZE_STANDARD_JOB_QUEUE == 0)
#error NVM_SIZE_STANDARD_JOB_QUEUE have size 0
#endif


CirqBufferType nvmQueue;


/* ----------------------------[private functions]---------------------------*/

static void WriteBlock(	const NvM_BlockDescriptorType *blockDescriptor,
						AdministrativeBlockType *adminBlock,
						uint8 setNumber,
						uint8 *sourceAddress);

/* ----------------------------[public functions]----------------------------*/



/*
 * This function needs to be implemented!
 */
static void CalcCrc(void)
{
	// TODO: Calculate CRC
}

typedef struct {
	boolean 						JobFinished;
	Std_ReturnType 					JobStatus;
	MemIf_JobResultType 			JobResult;
	const NvM_BlockDescriptorType *	BlockDescriptor;
	AdministrativeBlockType *		BlockAdmin;
} MemIfJobAdminType;

static MemIfJobAdminType MemIfJobAdmin = {
		.JobFinished = TRUE,
		.JobStatus = E_OK,
		.JobResult = MEMIF_JOB_OK,
		.BlockDescriptor = NULL,
		.BlockAdmin = NULL
};

enum Nvm_MultiBlockReq {
	MB_REQ_NONE,
	MB_REQ_READ_ALL,
	MB_REQ_WRITE_ALL,
};

typedef struct {
	NvmStateType			state;
	uint16					currBlockIndex;		// Keeps track of next unfinished block
	NvM_RequestResultType	PendingErrorStatus;	// Status from multi block requests i.e. Read/Write/CancelWrite-all
} AdminMultiReqType;

static AdminMultiReqType AdminMultiReq;


/*
 * Set the MemIf job as busy
 */
static void SetMemifJobBusy()
{
	MemIfJobAdmin.JobFinished = FALSE;
}


#if (NVM_POLLING_MODE == STD_ON)
/*
 * Check if the MemIf job is finished
 */
static boolean CheckMemIfJobFinished(void)
{
	MemIf_JobResultType jobResult;

	if (!MemIfJobAdmin.JobFinished) {
		jobResult = MemIf_GetJobResult(FIXME);

		if (jobResult == MEMIF_JOB_OK) {
			MemIfJobAdmin.JobFinished = TRUE;
			MemIfJobAdmin.JobStatus = E_OK;
			MemIfJobAdmin.JobResult = jobResult;
		} else if (jobResult != MEMIF_JOB_PENDING) {
			MemIfJobAdmin.JobFinished = TRUE;
			MemIfJobAdmin.JobStatus = E_NOT_OK;
			MemIfJobAdmin.JobResult = jobResult;
		}
	}

	return MemIfJobAdmin.JobFinished;
}
#else
/*
 * Check if the MemIf job is finished
 */

static boolean CheckMemIfJobFinished(void)
{
	return MemIfJobAdmin.JobFinished;
}

#endif


/*
 * Abort the MemIf job with E_NOT_OK
 */
static void AbortMemIfJob(MemIf_JobResultType jobResult)
{
	MemIfJobAdmin.JobFinished = TRUE;
	MemIfJobAdmin.JobStatus = E_NOT_OK;
	MemIfJobAdmin.JobResult = jobResult;
}

#if 0
static boolean CheckJobFailed( void ) {
	return CheckMemIfJobFinished() && (MemIfJobAdmin.JobResult == MEMIF_JOB_FAILED);
}
#endif

/*
 * Request a read of a block from MemIf
 */
static void ReadBlock(const NvM_BlockDescriptorType *blockDescriptor,
							AdministrativeBlockType *adminBlock,
							uint8 setNumber,
							uint16 blockOffset,
							uint8 *destAddress,
							uint16 length )
{
	Std_ReturnType returnCode;

	if (setNumber < blockDescriptor->NvBlockNum) {
		SetMemifJobBusy();
		MemIfJobAdmin.BlockAdmin = adminBlock;
		MemIfJobAdmin.BlockDescriptor = blockDescriptor;
		returnCode = MemIf_Read(blockDescriptor->NvramDeviceId, BLOCK_BASE_AND_SET_TO_BLOCKNR(blockDescriptor->NvBlockBaseNumber, setNumber), blockOffset, destAddress, length );
		if (returnCode != E_OK) {
			AbortMemIfJob(MEMIF_JOB_FAILED);
		}
	} else if (setNumber < blockDescriptor->NvBlockNum + blockDescriptor->RomBlockNum) {
		// TODO: Read from ROM
	} else {
		// Error: setNumber out of range
		DET_REPORTERROR(MODULE_ID_NVM, 0, NVM_LOC_READ_BLOCK_ID, NVM_PARAM_OUT_OF_RANGE);
	}
}


/*
 * Initiate the read all job
 */
static void ReadAllInit(void)
{
	/*
	 * Initiate the read all job
	 */
	const NvM_BlockDescriptorType	*BlockDescriptorList = NvM_Config.BlockDescriptor;
	AdministrativeBlockType *AdminBlockTable = AdminBlock;
	uint16 i;

	// Set status to pending in the administration blocks
	AdminMultiBlock.ErrorStatus = NVM_REQ_PENDING; /** @req 3.1.5/NVM304 */
	AdminMultiReq.PendingErrorStatus = NVM_REQ_OK;
	AdminMultiReq.currBlockIndex = 0;

	for (i = 0; i < ( NVM_NUM_OF_NVRAM_BLOCKS ); i++) {
		if ((BlockDescriptorList->SelectBlockForReadall)
#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)						/** @req NVM345 */
				&& ((!AdminBlockTable->BlockValid)			// TODO: Check if this is to be done like this
				|| (!AdminBlockTable->BlockChanged))		// TODO: Check if this is to be done like this
#endif
				) {
			VALIDATE_NO_RV(BlockDescriptorList->RamBlockDataAddress != NULL, NVM_READ_ALL_ID, NVM_E_WRONG_CONFIG);
			VALIDATE_NO_RV(BlockDescriptorList->BlockManagementType != NVM_BLOCK_DATASET, NVM_READ_ALL_ID, NVM_E_WRONG_CONFIG);

			AdminBlockTable->ErrorStatus = NVM_REQ_PENDING;
			AdminBlockTable->BlockState = BLOCK_STATE_MEMIF_REQ;
		} else {
			AdminBlockTable->ErrorStatus = NVM_REQ_BLOCK_SKIPPED;	/* @req	3.1.5/NVM287 */
		}

		AdminBlockTable++;
		BlockDescriptorList++;
	}
}


static void writeCrcToBuffer( 	void *bufPtr,
								const NvM_BlockDescriptorType *bPtr,
								AdministrativeBlockType *admPtr )
{
	if( bPtr->BlockUseCrc ) {
		if(bPtr->BlockCRCType == NVM_CRC16) {
			WRITE16_NA(bufPtr + bPtr->NvBlockLength,admPtr->RamCrc.crc16);
		} else {
			WRITE32_NA(bufPtr + bPtr->NvBlockLength,admPtr->RamCrc.crc32);
		}
	}
}


/*
 * Drive the read state-machine
 *
 *
 *
 */

static void DriveBlock( const NvM_BlockDescriptorType	*bPtr,
							AdministrativeBlockType *admPtr,
							void *dataPtr,
							boolean write,
							boolean multiBlock,
							boolean restoreFromRom )
{
	bool blockDone = 0;
	static uint8 driveBlockCnt = 0;


	NVM_ASSERT( admPtr->ErrorStatus == NVM_REQ_PENDING);

	DEBUG_BLOCK_STATE("DriveBlock", BLOCK_NR_FROM_PTR(bPtr), admPtr->BlockState );

	switch (admPtr->BlockState) {
	case BLOCK_STATE_MEMIF_REQ:
	{
		void *ramData = (dataPtr != NULL) ?  dataPtr : bPtr->RamBlockDataAddress;

		admPtr->savedDataPtr = ramData;

		if( write ) {
			if( multiBlock && (dataPtr!=NULL)) {
				NVM_ASSERT(0);
			}
			/* Copy to work buffer */
			memcpy( Nvm_WorkBuffer, ramData, bPtr->NvBlockLength );
			/* Add the CRC to write */
			writeCrcToBuffer(Nvm_WorkBuffer, bPtr, admPtr );
			WriteBlock(bPtr, admPtr, 0, Nvm_WorkBuffer);
		} else {
			uint8 crcLen = 0;
			/* Read to workbuffer */
			if( bPtr->BlockUseCrc ) {
				crcLen = (bPtr->BlockCRCType == NVM_CRC16) ? 2: 4;
			}

			if( restoreFromRom ) {
				NVM_ASSERT( bPtr->RomBlockDataAdress != NULL );
				/* No CRC on the ROM block */
				memcpy(ramData,bPtr->RomBlockDataAdress,bPtr->NvBlockLength);

				admPtr->ErrorStatus = NVM_REQ_OK;
				blockDone = 1;
				break;
			} else {
				ReadBlock(bPtr, admPtr, 0, 0, bPtr->RamBlockDataAddress, bPtr->NvBlockLength+crcLen);
			}
		}

		admPtr->BlockState = BLOCK_STATE_MEMIF_PROCESS;
		break;
	}

	case BLOCK_STATE_MEMIF_PROCESS:
	{
		/* Check read */
		MemIf_JobResultType jobResult = MemIf_GetJobResult(FIXME);

		if( MEMIF_JOB_PENDING == jobResult ) {
			/* Keep on waiting */
		} else if( MEMIF_JOB_OK == jobResult ) {
			/* We are done */

#if 0
			if( BLOCK_STATE_MEMIF_CRC_PROCESS == admPtr->BlockState ) {
				/* @req 3.1.5/NVM362 NvM_ReadAll*/
				DEBUG_CHECKSUM("RAM CRC", (bPtr->BlockCRCType == NVM_CRC16) ?  admPtr->RamCrc.crc16 : admPtr->RamCrc.crc32);
				admPtr->BlockState = BLOCK_STATE_CALC_CRC_READ;
				break;
			}
#endif

			if( write ) {
				admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
				admPtr->ErrorStatus = NVM_REQ_OK;
				blockDone = 1;

#if 0
				if( bPtr->BlockUseCrc ) {
					/* Explicit CRC calc (not dependent on NvmCalcRamBlockCrc) */
					/* @req 3.1.5/NVM212 NvM_WriteBlock */
					/* @req 3.1.5/NVM253 NvM_WriteAll   */
					admPtr->BlockState = BLOCK_STATE_CALC_CRC_WRITE;
				} else {
					/* Done */
					admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
					admPtr->ErrorStatus = NVM_REQ_OK;
					blockDone = 1;
				}
#endif
			} else {
				/* read */
				uint8 crcLen = 0;
				if( bPtr->BlockUseCrc ) {

					/* The read data is in the work buffer, read the CRC */
					if( bPtr->BlockCRCType == NVM_CRC16) {
						admPtr->NvCrc.crc16 = READ16_NA( Nvm_WorkBuffer + bPtr->NvBlockLength );
						crcLen = 2;
						DEBUG_PRINTF(">> Nv CRC %04x\n",admPtr->NvCrc.crc16);
						admPtr->RamCrc.crc16 = admPtr->NvCrc.crc16;	/* Set RAM CRC = NvRAM CRC */
					} else {
						admPtr->NvCrc.crc32 = READ32_NA( Nvm_WorkBuffer + bPtr->NvBlockLength );
						crcLen = 4;
						DEBUG_PRINTF(">> Nv CRC %08x\n",admPtr->NvCrc.crc16);
						admPtr->RamCrc.crc32 = admPtr->NvCrc.crc32;	/* Set RAM CRC = NvRAM CRC */
					}

					memcpy(admPtr->savedDataPtr, Nvm_WorkBuffer, bPtr->NvBlockLength  + crcLen );

					/* Check if we should re-calculate the RAM checksum now when it's in RAM */
					if( bPtr->CalcRamBlockCrc ) {
						/* This block want its RAM block CRC checked */
						DEBUG_PRINTF(">> Recalculation of RAM checksum \n",admPtr->NvCrc.crc16);
						assert( bPtr->BlockUseCrc == 1);
						admPtr->BlockState = BLOCK_STATE_CALC_CRC_READ;
					} else {
						/* Done */
						admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
						admPtr->ErrorStatus = NVM_REQ_OK;
						blockDone = 1;
					}

				} else {
					DEBUG_PRINTF(">> Block have NO CRC\n");

					memcpy(admPtr->savedDataPtr, Nvm_WorkBuffer, bPtr->NvBlockLength  + crcLen );
					/* Done */
					admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
					admPtr->ErrorStatus = NVM_REQ_OK;
					blockDone = 1;
				}

				/* Copy from Workbuffer to the real buffer */

			}
			break;
		} else {
			/* Something failed */
			AdminMultiReq.PendingErrorStatus = NVM_REQ_NOT_OK;
			blockDone = 1;
			admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;	/* TODO, this really true for all result below */

			switch( jobResult ) {
			case MEMIF_BLOCK_INVALID:
				/* @req 3.1.5/NVM342 */
				admPtr->ErrorStatus = NVM_REQ_NV_INVALIDATED;
				break;
			case MEMIF_BLOCK_INCONSISTENT:
				/* @req 3.1.5/NVM360 */
				admPtr->ErrorStatus = NVM_REQ_INTEGRITY_FAILED;
				DEM_REPORTERRORSTATUS(NVM_E_REQ_INTEGRITY_FAILED,DEM_EVENT_STATUS_FAILED);
				break;
			case MEMIF_JOB_FAILED:
				/* @req 3.1.5/NVM361 */
				admPtr->ErrorStatus = NVM_REQ_NOT_OK;
				DEM_REPORTERRORSTATUS(NVM_E_REQ_FAILED,DEM_EVENT_STATUS_FAILED);
			default:
				NVM_ASSERT(0);
				break;
			}
		}
		break;
	}

	case BLOCK_STATE_CALC_CRC:
		NVM_ASSERT(0);
		break;

	case BLOCK_STATE_CALC_CRC_WRITE:
	{
		uint16 crc16;
		uint32 crc32;
		NVM_ASSERT(bPtr->RamBlockDataAddress != NULL );

		/* Calculate RAM CRC checksum */
		if( bPtr->BlockCRCType == NVM_CRC16 ) {

			crc16 = Crc_CalculateCRC16(bPtr->RamBlockDataAddress,bPtr->NvBlockLength,0xffff);
			DEBUG_CHECKSUM("RAM",crc16);

			/* Just save the checksum */
			admPtr->RamCrc.crc16 = crc16;

#if 0
			/* NV CRC in admin block */
			if( admPtr->RamCrc.crc16 != crc16 ) {
				NVM_ASSERT(0);		/* TODO: Corrupt CRC */
			} else {
				admPtr->BlockChanged = BLOCK_STATE_MEMIF_REQ;
			}
#endif

			/* Write the block */
			admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
		} else {
			/* @req 3.1.5/NVM253 */
			crc32 = Crc_CalculateCRC32(bPtr->RamBlockDataAddress,bPtr->NvBlockLength,0xffffffffUL);
			if( crc32 !=  admPtr->RamCrc.crc32 ) {
				/* The checksum is wrong, something have written to the RAM area without
				 * telling the NVM */
				NVM_ASSERT(0);
			} else {
				admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
			}
			admPtr->RamCrc.crc32 = crc32;
		}
		break;
	}
	case BLOCK_STATE_CALC_CRC_READ:
	{
		NVM_ASSERT(bPtr->RamBlockDataAddress != NULL );
		NVM_ASSERT(bPtr->CalcRamBlockCrc == true );
		uint16 crc16;
		uint32 crc32;

		/* Calculate RAM CRC checksum */
		if( bPtr->BlockCRCType == NVM_CRC16 ) {


			crc16 = Crc_CalculateCRC16(bPtr->RamBlockDataAddress,bPtr->NvBlockLength,0xffff);
			DEBUG_CHECKSUM("RAM",crc16);

			/* NV CRC in admin block */

			if( driveBlockCnt == 1) {
				/* The previous "loop" we filled with default data */
				admPtr->RamCrc.crc16 = crc16;
			}

			/* @req 3.1.5/NVM387 */
			if( admPtr->RamCrc.crc16 != crc16 ) {

				/* @req 3.1.5/NVM388 Nvm_ReadAll */

				/* NVM387, NVM388
				 *
				 * Corrupt CRC, what choices are there:
				 * 1. Default data (=ROM) configured, just copy it.
				 * 2. Data redundancy, get it.
				 * 3. None of the above. Catastrophic failure. (NVM203)
				 */
				if( bPtr->RomBlockDataAdress != NULL ) {
					/* TODO: Restore block from ROM */
					NVM_ASSERT(0);
				} else {
					/* @req 3.1.5/NVM469 */
					if( bPtr->InitBlockCallback != NULL ) {

						DEBUG_PRINTF("Filling block with default data\n");
						bPtr->InitBlockCallback();
						driveBlockCnt++;
						/* NVM085 is very vague here, but the says the application should be
						 * able distinguish between when the init-callback have been called
						 * or CRC is corrupt.
						 */

						/* The RAM CRC is at this point not calculated...so we must do this
						 * .. so just stay in this state one more MainFunction.
						 * */

					} else {

						/* @req 3.1.5/NVM203 */
						DEM_REPORTERRORSTATUS(NVM_E_INTEGRITY_FAILED,DEM_EVENT_STATUS_FAILED);
						/* @req 3.1.5/NVM204 */
						admPtr->ErrorStatus = NVM_REQ_INTEGRITY_FAILED;
						admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
						blockDone = 1;
					}
				}
			} else {
				DEBUG_CHECKSUM("RAM checksum ok with ",crc16);
				admPtr->ErrorStatus = NVM_REQ_OK;
				admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
				blockDone = 1;
			}

		} else {
			/* @req 3.1.5/NVM253 */
			/* Calculate CRC on the data we just read to RAM. Compare with CRC that is located in NV block */
			crc32 = Crc_CalculateCRC32(bPtr->RamBlockDataAddress,bPtr->NvBlockLength,0xffffffffUL);
			if( crc32 !=  admPtr->RamCrc.crc32 ) {
				NVM_ASSERT(0);	/* TODO: Corrupt CRC */
			} else {
				admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
			}
		}
		break;
	}
	default:
		NVM_ASSERT(0);
		break;
	}

	if( blockDone  ) {

		DEBUG_PRINTF("# Block Done\n");

		if( admPtr->ErrorStatus == NVM_REQ_OK ) {
			admPtr->BlockChanged = FALSE;
			admPtr->BlockValid = TRUE;
		}


		/*  @req 3.1.5/NVM281 */
		if( bPtr->SingleBlockCallback != NULL ) {
			bPtr->SingleBlockCallback(NVM_READ_ALL_ID, admPtr->ErrorStatus);
		}

		if( multiBlock ) {
			AdminMultiReq.currBlockIndex++;
			if( AdminMultiReq.currBlockIndex >= NVM_NUM_OF_NVRAM_BLOCKS ) {
				AdminMultiReq.currBlockIndex = 0;

				/* @req 3.1.5/NVM301 */
				if( NVM_REQ_NOT_OK == AdminMultiReq.PendingErrorStatus ) {
					AdminMultiBlock.ErrorStatus = NVM_REQ_NOT_OK;
				} else {
					AdminMultiBlock.ErrorStatus = NVM_REQ_OK;
				}
				nvmState = NVM_IDLE;
				nvmSubState = 0;
			}
		} else {
			nvmState = NVM_IDLE;
			nvmSubState = 0;

		}
	}
}

/*
 * Main function for the read all job
 */
static void ReadAllMain(void)
{

	/* Cases:
	 * 1. We process each block until it's finished
	 * 2. We start to process a lot of blocks. The blocks may use different devices
	 *    and should be able to read a lot of them. This includes CRC.
	 *
	 *    1) is much simpler and 2) probably much faster.
	 *    This implementation will use 1) since it's simpler and maximum time that is
	 *    spent in MainFunction() can be controlled much better.
	 */

	/* Skip blocks that are skipped */

	while (	(AdminBlock[AdminMultiReq.currBlockIndex].ErrorStatus == NVM_REQ_BLOCK_SKIPPED) ) {
		if( (AdminMultiReq.currBlockIndex < NVM_NUM_OF_NVRAM_BLOCKS) ) {
			AdminMultiReq.currBlockIndex++;
		} else {
			return;
		}
	}

	DriveBlock(	&NvM_Config.BlockDescriptor[AdminMultiReq.currBlockIndex],
					&AdminBlock[AdminMultiReq.currBlockIndex],
					NULL,
					false,
					true, false);
}


/*
 * Request writing of a block to MemIf
 */
static void WriteBlock(	const NvM_BlockDescriptorType *blockDescriptor,
						AdministrativeBlockType *adminBlock,
						uint8 setNumber,
						uint8 *sourceAddress)
{
	Std_ReturnType returnCode;

	if (setNumber < blockDescriptor->NvBlockNum) {
		SetMemifJobBusy();
		MemIfJobAdmin.BlockAdmin = adminBlock;
		MemIfJobAdmin.BlockDescriptor = blockDescriptor;
		returnCode = MemIf_Write(blockDescriptor->NvramDeviceId, BLOCK_BASE_AND_SET_TO_BLOCKNR(blockDescriptor->NvBlockBaseNumber, setNumber), sourceAddress);
		if (returnCode != E_OK) {
			AbortMemIfJob(MEMIF_JOB_FAILED);
		}
	} else {
		// Error: setNumber out of range
		DET_REPORTERROR(MODULE_ID_NVM, 0, NVM_LOC_WRITE_BLOCK_ID, NVM_PARAM_OUT_OF_RANGE);
	}
}



/*
 * Initiate the write all job
 */
static void WriteAllInit(void)
{
	const NvM_BlockDescriptorType	*BlockDescriptorList = NvM_Config.BlockDescriptor;
	AdministrativeBlockType *AdminBlockTable = AdminBlock;
	uint16 i;

//	nvmState = NVM_WRITE_ALL_PROCESSING;
	AdminMultiReq.PendingErrorStatus = NVM_REQ_OK;
	AdminMultiReq.currBlockIndex = 0;

	for (i = 0; i < NVM_NUM_OF_NVRAM_BLOCKS; i++) {
		if ((BlockDescriptorList->RamBlockDataAddress != NULL)
#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)						/** @req NVM344 */
				&& (AdminBlockTable->BlockValid)					/** @req NVM682 */
				&& (AdminBlockTable->BlockChanged)					/** @req NVM682 */
#endif
				&& (!AdminBlockTable->BlockWriteProtected))			/** @req NVM432 *//** @req NVM433 */
		{
			AdminBlockTable->ErrorStatus = NVM_REQ_PENDING;

			if (BlockDescriptorList->BlockUseCrc) {
				AdminBlockTable->BlockState = BLOCK_STATE_CALC_CRC_WRITE;	/** @req NVM253 */
			} else {
				AdminBlockTable->BlockState = BLOCK_STATE_MEMIF_REQ;
				AdminBlockTable->NumberOfWriteFailed = 0;
			}
		} else {
			AdminBlockTable->ErrorStatus = NVM_REQ_BLOCK_SKIPPED;	/** @req NVM298 */
		}

		AdminBlockTable++;
		BlockDescriptorList++;
	}
}



/*
 * Main function for the write all job
 */
static void WriteAllMain(void)
{

	while (	(AdminBlock[AdminMultiReq.currBlockIndex].ErrorStatus == NVM_REQ_BLOCK_SKIPPED) ) {
		if( (AdminMultiReq.currBlockIndex < NVM_NUM_OF_NVRAM_BLOCKS) ) {
			AdminMultiReq.currBlockIndex++;
		} else {
			return;
		}
	}

	DriveBlock(	&NvM_Config.BlockDescriptor[AdminMultiReq.currBlockIndex],
					&AdminBlock[AdminMultiReq.currBlockIndex],
					NULL,
					true,
					true, false );

}


#if 0
/*
 * Handles the result of one MemIf block write
 */
static void WriteAllCheckWriteResult(void)
{
	if (MemIfJobAdmin.JobStatus == E_OK) {
		// TODO: Check if redundant block shall be written NVM337

		if (MemIfJobAdmin.BlockDescriptor->WriteBlockOnce) {
			MemIfJobAdmin.BlockAdmin->BlockWriteProtected = TRUE;	/** @req NVM329 */
		}
		MemIfJobAdmin.BlockAdmin->BlockState = BLOCK_STATE_MEMIF_REQ;
		MemIfJobAdmin.BlockAdmin->ErrorStatus = NVM_REQ_OK;

		if (MemIfJobAdmin.BlockDescriptor->SingleBlockCallback != NULL) {
			(void)MemIfJobAdmin.BlockDescriptor->SingleBlockCallback(NVM_SERVICE_ID, MemIfJobAdmin.BlockAdmin->ErrorStatus);
		}
	} else {
		MemIfJobAdmin.BlockAdmin->NumberOfWriteFailed++;
		if (MemIfJobAdmin.BlockAdmin->NumberOfWriteFailed > NVM_MAX_NUMBER_OF_WRITE_RETRIES) {
			// TODO: Check if redundant block shall be written NVM337

			// Write has failed
			AdminMultiReq.PendingErrorStatus = NVM_REQ_NOT_OK;

			MemIfJobAdmin.BlockAdmin->BlockState = BLOCK_STATE_MEMIF_REQ;
			MemIfJobAdmin.BlockAdmin->ErrorStatus = NVM_REQ_NOT_OK;	/** @req NVM296 */
#if defined(USE_DEM)
			Dem_ReportErrorStatus(NVM_E_REQ_FAILED,DEM_EVENT_STATUS_FAILED);
#endif

			if (MemIfJobAdmin.BlockDescriptor->SingleBlockCallback != NULL) {
				(void)MemIfJobAdmin.BlockDescriptor->SingleBlockCallback(NVM_SERVICE_ID, MemIfJobAdmin.BlockAdmin->ErrorStatus);
			}
		}
	}
	nvmState = NVM_WRITE_ALL_PROCESSING;
}
#endif



/***************************************
 *    External accessible functions    *
 ***************************************/
/*
 * Procedure:	NvM_Init
 * Reentrant:	No
 */
void NvM_Init(void)
{
	/** @req NVM399 *//** @req NVM193 */
	const NvM_BlockDescriptorType	*BlockDescriptorList = NvM_Config.BlockDescriptor;
	AdministrativeBlockType *AdminBlockTable = AdminBlock;
	uint16 i;


	CirqBuff_Init(&nvmQueue,nvmQueueData,sizeof(nvmQueueData),sizeof(Nvm_QueueType));

	// Initiate the administration blocks
	for (i = 0; i< NVM_NUM_OF_NVRAM_BLOCKS; i++) {
		if (BlockDescriptorList->BlockManagementType == NVM_BLOCK_DATASET) {
			AdminBlockTable->DataIndex = 0;	/** @req NVM192 */
		}
		AdminBlockTable->BlockWriteProtected = BlockDescriptorList->BlockWriteProt;
		AdminBlockTable->ErrorStatus = NVM_REQ_NOT_OK;
		AdminBlockTable->BlockChanged = FALSE;
		AdminBlockTable->BlockValid = FALSE;
		AdminBlockTable->NumberOfWriteFailed = 0;

		AdminBlockTable++;
		BlockDescriptorList++;
	}

	AdminMultiBlock.ErrorStatus = NVM_REQ_NOT_OK;

	// Set status to initialized
	nvmState = NVM_IDLE;	/** @req 3.1.5/NVM399 */
}


/*
 * Procedure:	NvM_ReadAll
 * Reentrant:	No
 */
void NvM_ReadAll(void)
{
	VALIDATE_NO_RV(nvmState != NVM_UNINITIALIZED, NVM_READ_ALL_ID, NVM_E_NOT_INITIALIZED);

	NVM_ASSERT(nvmState == NVM_IDLE);

	AdminMultiReq.state = NVM_READ_ALL;
	AdminMultiBlock.ErrorStatus = NVM_REQ_PENDING;
}



/*
 * Procedure:	NvM_WriteAll
 * Reentrant:	No
 */
void NvM_WriteAll(void)
{
	VALIDATE_NO_RV(nvmState != NVM_UNINITIALIZED, NVM_READ_ALL_ID, NVM_E_NOT_INITIALIZED);

	NVM_ASSERT(nvmState == NVM_IDLE);

	AdminMultiReq.state = NVM_WRITE_ALL;
	AdminMultiBlock.ErrorStatus = NVM_REQ_PENDING;
}


/*
 * Procedure:	NvM_CancelWriteAll
 * Reentrant:	No
 */
void NvM_CancelWriteAll(void)
{
	
}


/*
 * Procedure:	NvM_GetErrorStatus
 * Reentrant:	Yes
 */
void NvM_GetErrorStatus(NvM_BlockIdType blockId, uint8 *requestResultPtr)
{
	VALIDATE_NO_RV(nvmState != NVM_UNINITIALIZED, NVM_GET_ERROR_STATUS_ID, NVM_E_NOT_INITIALIZED);
	VALIDATE_NO_RV(blockId < NVM_NUM_OF_NVRAM_BLOCKS+1, NVM_GET_ERROR_STATUS_ID, NVM_E_PARAM_BLOCK_ID);

	if (blockId == 0) {
		// Multiblock ID
		*requestResultPtr = AdminMultiBlock.ErrorStatus;
	} else if (blockId == 1) {
		/* TODO */
	  *requestResultPtr = NVM_REQ_OK;
	} else {
		*requestResultPtr = AdminBlock[blockId-1].ErrorStatus;
	}

}


#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)	/** @req NVM408 */
/*
 * Procedure:	Nvm_SetRamBlockStatus
 * Reentrant:	Yes
 */
void NvM_SetRamBlockStatus(NvM_BlockIdType blockId, boolean blockChanged)
{
	const NvM_BlockDescriptorType	*BlockDescriptorList = NvM_Config.BlockDescriptor;

	VALIDATE_NO_RV(nvmState != NVM_UNINITIALIZED, NVM_SET_RAM_BLOCK_STATUS_ID, NVM_E_NOT_INITIALIZED);	/** @req NVM497 */
	VALIDATE_NO_RV(blockId < NVM_NUM_OF_NVRAM_BLOCKS+1, NVM_SET_RAM_BLOCK_STATUS_ID, NVM_E_PARAM_BLOCK_ID);
	VALIDATE_NO_RV(blockId > 1, NVM_SET_RAM_BLOCK_STATUS_ID, NVM_E_PARAM_BLOCK_ID);

	if (BlockDescriptorList[blockId-1].RamBlockDataAddress != NULL) {	/** @req NVM240 */
		if (blockChanged) {
			AdminBlock[blockId-1].BlockChanged = TRUE;	/** @req NVM406 */
			AdminBlock[blockId-1].BlockValid = TRUE;	/** @req NVM241 */
			if (BlockDescriptorList[blockId-1].BlockUseCrc) {
				AdminBlock[blockId-1].BlockState = BLOCK_STATE_CALC_CRC;	/** @req NVM121 */
			}
		} else {
			AdminBlock[blockId-1].BlockChanged = FALSE;	/** @req NVM405 */
			AdminBlock[blockId-1].BlockValid = FALSE;
		} // else blockChanged
	} // if permanent block
}
#endif


void NvM_SetBlockLockStatus( NvM_BlockIdType blockId, boolean blockLocked ) {
	(void)blockId;
	(void)blockLocked;
}


/**
 * Restore default data to its corresponding RAM block.
 *
 * @param BlockId		NVRAM block identifier.
 * @param NvM_DestPtr  	Pointer to the RAM block
 * @return
 */
Std_ReturnType NvM_RestoreBlockDefaults( NvM_BlockIdType blockId, uint8* NvM_DestPtr )
{
	/* !req 3.1.5/NVM012 */	/* !req 3.1.5/NVM267 */	/* !req 3.1.5/NVM266 */
	/* !req 3.1.5/NVM353 */	/* !req 3.1.5/NVM435 */	/* !req 3.1.5/NVM436 */	/* !req 3.1.5/NVM227 */
	/* !req 3.1.5/NVM228 */	/* !req 3.1.5/NVM229 */	/* !req 3.1.5/NVM413 */

	const NvM_BlockDescriptorType *	bPtr;
	AdministrativeBlockType * 		admPtr;
	Nvm_QueueType qEntry;
	int rv;

	NVM_ASSERT( blockId >= 2 );	/* No support for lower numbers, yet */

	/* @req 3.1.5/NVM618 */
	VALIDATE_RV( 	blockId <= NVM_NUM_OF_NVRAM_BLOCKS,
					NVM_WRITE_BLOCK_ID,NVM_E_PARAM_BLOCK_ID,E_NOT_OK );

	bPtr = &NvM_Config.BlockDescriptor[blockId-1];
	admPtr = &AdminBlock[blockId-1];

	/** @req 3.1.5/NVM196 */ /** @req 3.1.5/NVM278 */
	if( (NvM_DestPtr == NULL) &&  ( bPtr->RamBlockDataAddress == NULL ) ) {
		/* It must be a permanent RAM block but no RamBlockDataAddress -> error */
		NVM_ASSERT(0);		// TODO: See NVM210, DET error
		return E_NOT_OK;
	}

	/* @req 3.1.5/NVM195 */
	qEntry.blockId = blockId;
	qEntry.op = NVM_RESTORE_BLOCK_DEFAULTS;
	qEntry.blockId = blockId;
	qEntry.dataPtr = (uint8_t *)NvM_DestPtr;
	rv = CirqBuffPush(&nvmQueue,&qEntry);
	NVM_ASSERT(rv == 0 );

	/* @req 3.1.5/NVM620 */
	VALIDATE_RV( (admPtr->ErrorStatus != NVM_REQ_PENDING), 0, NVM_E_BLOCK_PENDING , E_NOT_OK );

	/* req 3.1.5/NVM185 */
	admPtr->ErrorStatus = NVM_REQ_PENDING;

	if( bPtr->BlockUseCrc) {
		admPtr->BlockState = BLOCK_STATE_CALC_CRC_WRITE;
	} else {
		admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
	}


	return E_OK;
}

/**
 * Service to copy the data NV block to the RAM block
 *
 * @param blockId		0 and 1 reserved. The block ID are sequential.
 * @param NvM_DstPtr
 * @return
 */


Std_ReturnType NvM_ReadBlock( NvM_BlockIdType blockId, uint8* NvM_DstPtr )
{
	/* !req 3.1.5/NVM010 */


	/* !req 3.1.5/NVM278 */
	/* !req 3.1.5/NVM340 */
	/* !req 3.1.5/NVM354 */
	/* !req 3.1.5/NVM200 */
	/* !req 3.1.5/NVM366 */
	/* !req 3.1.5/NVM206 */
	/* !req 3.1.5/NVM341 */
	/* !req 3.1.5/NVM358 */
	/* !req 3.1.5/NVM359 */
	/* !req 3.1.5/NVM279 */
	/* !req 3.1.5/NVM316 */
	/* !req 3.1.5/NVM317 */
	/* !req 3.1.5/NVM201 */
	/* !req 3.1.5/NVM202 */
	/* !req 3.1.5/NVM203 */
	/* !req 3.1.5/NVM204 */
	/* !req 3.1.5/NVM409 */


   /* logical block:
    *
    *    1 1 1 1 1 1
    *   |5 4 3 2 1 0 9 8|7 6 5 4 3 2 1 0|
    *    b b b b b b b b b b d d d d d d
    *
    * Here we have 10 bits for block id, 16-5 for DataSetSelection bits.
    * - 2^10, 1024 blocks
    * - 64 datasets for each NVRAM block
    *
    * How are the block numbers done in EA? Assume virtual page=8
    * logical
    *  Block   size
    *  1       32
    *   2
    *   3
    *   4
    *  5       12
    *   6
    *  7
    *
    *  How can NVM/NvmNvBlockLength and EA/EaBlockSize be different?
    *  It seems that EA/FEE does not care about that logical block 2 above is
    *  "blocked"
    *
    */
	Nvm_QueueType qEntry;
	int rv;

	/** @req 3.1.5/NVM196 */ /** @req 3.1.5/NVM278 */
	if( (NvM_DstPtr == NULL) &&  ( NvM_Config.BlockDescriptor[blockId-1].RamBlockDataAddress == NULL ) ) {
		/* It must be a permanent RAM block but no RamBlockDataAddress -> error */
		return E_NOT_OK;
	}

	/* @req 3.1.5/NVM195 */
	qEntry.blockId = blockId;
	qEntry.op = NVM_READ_BLOCK;
	qEntry.blockId = blockId;
	qEntry.dataPtr = NvM_DstPtr;
	rv = CirqBuffPush(&nvmQueue,&qEntry);
	NVM_ASSERT(rv == 0 );

	/* @req 3.1.5/NVM615 */
	VALIDATE_RV( (AdminBlock[blockId-1].ErrorStatus != NVM_REQ_PENDING), 0, NVM_E_BLOCK_PENDING , E_NOT_OK );

	/* req 3.1.5/NVM185 */
	AdminBlock[blockId-1].ErrorStatus = NVM_REQ_PENDING;

	return E_OK;
}


/**
 * Service to copy a RAM block to its correspnding NVRAM block
 *
 * @param blockId
 * @param NvM_SrcPtr
 * @return
 */
Std_ReturnType NvM_WriteBlock( NvM_BlockIdType blockId, const uint8* NvM_SrcPtr ) {

	const NvM_BlockDescriptorType *	bPtr;
	AdministrativeBlockType * 		admPtr;
	Nvm_QueueType qEntry;
	int rv;

	NVM_ASSERT( blockId >= 2 );	/* No support for lower numbers, yet */

	/* @req 3.1.5/NVM618 */
	VALIDATE_RV( 	blockId <= NVM_NUM_OF_NVRAM_BLOCKS,
					NVM_WRITE_BLOCK_ID,NVM_E_PARAM_BLOCK_ID,E_NOT_OK );

	bPtr = &NvM_Config.BlockDescriptor[blockId-1];
	admPtr = &AdminBlock[blockId-1];

	/** @req 3.1.5/NVM196 */ /** @req 3.1.5/NVM278 */
	if( (NvM_SrcPtr == NULL) &&  ( bPtr->RamBlockDataAddress == NULL ) ) {
		/* It must be a permanent RAM block but no RamBlockDataAddress -> error */
		NVM_ASSERT(0);		// TODO: See NVM210, DET error
		return E_NOT_OK;
	}

	/* @req 3.1.5/NVM195 */
	qEntry.blockId = blockId;
	qEntry.op = NVM_WRITE_BLOCK;
	qEntry.blockId = blockId;
	qEntry.dataPtr = (uint8_t *)NvM_SrcPtr;
	rv = CirqBuffPush(&nvmQueue,&qEntry);
	NVM_ASSERT(rv == 0 );

	/* @req 3.1.5/NVM620 */
	VALIDATE_RV( (admPtr->ErrorStatus != NVM_REQ_PENDING), 0, NVM_E_BLOCK_PENDING , E_NOT_OK );

	/* req 3.1.5/NVM185 */
	admPtr->ErrorStatus = NVM_REQ_PENDING;

	if( bPtr->BlockUseCrc) {
		admPtr->BlockState = BLOCK_STATE_CALC_CRC_WRITE;
	} else {
		admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
	}


	return E_OK;
}

/* Missing from Class 2
 * - NvM_CancelWriteAll
 * - NvM_SetDataIndex
 * - NvM_GetDataIndex
 * */





const NvM_BlockDescriptorType *	nvmBlock;
AdministrativeBlockType *admBlock;

/**
 *
 */
void NvM_MainFunction(void)
{
	int 			rv;
	Nvm_QueueType 	qEntry;
	const NvM_BlockDescriptorType *	bList = NvM_Config.BlockDescriptor;
//	const NvM_BlockDescriptorType *	currBlock;
//	static uint32 crc32;
//	static uint32 crc32Left;

	/* Check for new requested state changes */
	if( nvmState == NVM_IDLE ) {
		rv = CirqBuffPop( &nvmQueue, &qEntry );
		if( rv == 0 ) {
			/* Found something in buffer */
			nvmState = qEntry.op;
			nvmBlock = &bList[qEntry.blockId-1];
			admBlock = &AdminBlock[qEntry.blockId-1];
			nvmSubState = 0;
			admBlock->ErrorStatus = NVM_REQ_PENDING;
			DEBUG_PRINTF("### Popped Single FIFO \n");
			DEBUG_PRINTF("### CRC On:%d Ram:%d Type:%d\n",nvmBlock->BlockUseCrc, nvmBlock->CalcRamBlockCrc, nvmBlock->BlockCRCType );
			DEBUG_PRINTF("### RAM:%x ROM:%x\n", nvmBlock->RamBlockDataAddress, nvmBlock->RomBlockDataAdress );
		} else {
			/* Check multiblock req */
			if( AdminMultiReq.state != NVM_UNINITIALIZED ) {
				nvmState = AdminMultiReq.state ;
				nvmSubState = 0;
				nvmBlock = 0;
				admBlock = 0;
				AdminMultiReq.state = NVM_UNINITIALIZED;

				DEBUG_PRINTF("### Popped MULTI\n");
			}
		}
	}

	DEBUG_STATE(nvmState,nvmSubState);

	switch (nvmState) {
	case NVM_UNINITIALIZED:
		break;

	case NVM_IDLE:
	{
		CalcCrc();
		break;
	}
	case NVM_READ_ALL:
		if( NS_INIT == nvmSubState ) {
			ReadAllInit();
			nvmSubState = NS_PROSSING;
		} else if( NS_PROSSING == nvmSubState ) {
			ReadAllMain();
		}
		break;

	case NVM_READ_BLOCK:
		DriveBlock(nvmBlock,admBlock, qEntry.dataPtr, false, false, false );
		break;

	case NVM_RESTORE_BLOCK_DEFAULTS:
		DriveBlock(nvmBlock,admBlock, qEntry.dataPtr, false, false, true );
		break;

	case NVM_WRITE_BLOCK:
		DriveBlock(nvmBlock,admBlock, qEntry.dataPtr, true /*write*/, false , false );
		break;

	case NVM_WRITE_ALL:
		if( NS_INIT == nvmSubState ) {
			WriteAllInit();
			nvmSubState = NS_PROSSING;
		} else if( NS_PROSSING == nvmSubState ) {
			WriteAllMain();
		}
		break;

	default:
		DET_REPORTERROR(MODULE_ID_NVM, 0, NVM_MAIN_FUNCTION_ID, NVM_UNEXPECTED_STATE);
		break;
	}
}


/***************************************
 *  Call-back notifications functions  *
 ***************************************/
#if (NVM_POLLING_MODE == STD_OFF)
/*
 * Procedure:	NvM_JobEndNotification
 * Reentrant:	No
 */
void NvM_JobEndNotification(void)
{
	MemIfJobAdmin.JobFinished = TRUE;
	MemIfJobAdmin.JobStatus = E_OK;
	MemIfJobAdmin.JobResult = MemIf_GetJobResult(FIXME);
}

/*
 * Procedure:	NvM_JobErrorNotification
 * Reentrant:	No
 */
void NvM_JobErrorNotification(void)
{
	MemIfJobAdmin.JobFinished = TRUE;
	MemIfJobAdmin.JobStatus = E_NOT_OK;
	MemIfJobAdmin.JobResult = MemIf_GetJobResult(FIXME);
}
#endif

