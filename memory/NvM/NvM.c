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
 */


/*
 *  General requirements
 */
/** @req NVM076 */
/** @req NVM552 */
/** @req NVM689 */


/*
 * NB! Even though some code exist for handling crc, the functionality is not complete
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
//#include "Crc.h" // Optional
#include "cirq_buffer.h"
#include "Modules.h"
#include <stdio.h>
#include "io.h"
#include "Crc.h"

#if NVM_NUM_OF_NVRAM_BLOCKS != 5
#error nOOOOO
#endif

#define DEBUG_BLOCK	1
#if defined(DEBUG_BLOCK)
#define DEBUG_BLOCK_STATE(_str,_block,_state)       printf("%s BLOCK NR:%d STATE:%d\n",_str,_block, _state); fflush(stdout);
#define DEBUG_STATE(_state,_substate)				printf("MAIN_STATE:%d/%d\n",_state,_substate); fflush(stdout);
#define DEBUG_PRINTF(format,...) 					printf(format,## __VA_ARGS__ ); fflush(stdout);
#endif


/* ----------------------------[private define]------------------------------*/



#define NVM_BLOCK_ALIGNMENT			4
#define NVM_CHECKSUM_LENGTH			4

#define NVM_BLOCK_OFFSET 		2





/* ----------------------------[private macro]-------------------------------*/

#define NVM_ASSERT(_exp)		assert(_exp)

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



/* ----------------------------[private typedef]-----------------------------*/

// State variable
typedef enum {
  NVM_UNINITIALIZED = 0,
  NVM_IDLE,
  NVM_READ_ALL,
  NVM_WRITE_ALL,
  NVM_READ_BLOCK,
  NVM_WRITE_BLOCK,
} NvmStateType;

typedef enum {
	BLOCK_STATE_MEMIF_REQ,
//	BLOCK_STATE_START,
	BLOCK_STATE_MEMIF_PROCESS,
	BLOCK_STATE_MEMIF_CRC_PROCESS,
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
	union Nvm_CRC			crc;
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
static int nvmSetNr;
static AdministrativeBlockType 		AdminBlock[NVM_NUM_OF_NVRAM_BLOCKS];
static AdministrativeMultiBlockType AdminMultiBlock;


static Nvm_QueueType  nvmQueueImmData[NVM_SIZE_IMMEDIATE_JOB_QUEUE];
static Nvm_QueueType  nvmQueueData[NVM_SIZE_STANDARD_JOB_QUEUE];

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
		jobResult = MemIf_GetJobResult();

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

static boolean CheckJobFailed( void ) {
	return CheckMemIfJobFinished() && (MemIfJobAdmin.JobResult == MEMIF_JOB_FAILED);
}

/*
 * Request a read of a block from MemIf
 */
static void ReadBlock(const NvM_BlockDescriptorType *blockDescriptor,
							AdministrativeBlockType *adminBlock,
							uint8 setNumber,
							uint8 *destAddress,
							uint16 length )
{
	Std_ReturnType returnCode;
	uint16 blockOffset = 0;	// TODO: How to calculate this?

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
							boolean multiBlock  )
{
	bool blockDone = 0;


	NVM_ASSERT( admPtr->ErrorStatus == NVM_REQ_PENDING);

	switch (admPtr->BlockState) {
	case BLOCK_STATE_MEMIF_REQ:
	{
		if( write ) {
			if( multiBlock && (dataPtr!=NULL)) {
				NVM_ASSERT(0);
			}

			if (dataPtr != NULL ) {
				WriteBlock(bPtr, admPtr, 0, dataPtr);
			} else {
				WriteBlock(bPtr, admPtr, 0, bPtr->RamBlockDataAddress);
			}
		} else {
			ReadBlock(bPtr, admPtr, 0, bPtr->RamBlockDataAddress, bPtr->NvBlockLength);
		}

		admPtr->BlockState = BLOCK_STATE_MEMIF_PROCESS;
		break;
	}

	case BLOCK_STATE_MEMIF_PROCESS:
	case BLOCK_STATE_MEMIF_CRC_PROCESS:
	{
		/* Check read */
		MemIf_JobResultType jobResult = MemIf_GetJobResult(0);

		if( MEMIF_JOB_PENDING == jobResult ) {
			/* Keep on waiting */
		} else if( MEMIF_JOB_OK == jobResult ) {
			/* We are done */

			if( BLOCK_STATE_MEMIF_CRC_PROCESS == admPtr->BlockState ) {
				/* @req 3.1.5/NVM362 NvM_ReadAll*/
				admPtr->BlockState = BLOCK_STATE_CALC_CRC_READ;
				break;
			}

			if( write ) {
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
			} else {
				/* read */
				if( bPtr->BlockUseCrc ) {
					if( bPtr->CalcRamBlockCrc) {
						/* The CRC does not fit into RAM block so we have to read
						 * it now..
						 */
						ReadBlock(bPtr, admPtr, 0, bPtr->RamBlockDataAddress + bPtr->NvBlockLength,
								(bPtr->BlockCRCType == NVM_CRC16)  ? 2 : 4 );

						admPtr->BlockState = BLOCK_STATE_MEMIF_CRC_PROCESS;

					} else {
						/* The data is transferred from NV to RAM (including checksums) */
						/* Done */
						admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
						admPtr->ErrorStatus = NVM_REQ_OK;
						blockDone = 1;
					}
				} else {
					/* Done */
					admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
					admPtr->ErrorStatus = NVM_REQ_OK;
					blockDone = 1;
				}
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
		uint32 crc32;
		NVM_ASSERT(bPtr->RamBlockDataAddress != NULL );

		/* Calculate RAM CRC checksum */
		if( bPtr->BlockCRCType == NVM_CRC16 ) {
#error HERE HERE
			crc16 = Crc_CalculateCRC16(bPtr->RamBlockDataAddress,bPtr->NvBlockLength,0xffff);
		} else {
			/* @req 3.1.5/NVM253 */
			crc32 = Crc_CalculateCRC32(bPtr->RamBlockDataAddress,bPtr->NvBlockLength,0xffffffffUL);
			if( crc32 !=  admPtr->crc.crc32 ) {
				/* The checksum is wrong, something have written to the RAM area without
				 * telling the NVM */
				NVM_ASSERT(0);
			} else {
				admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
			}
			admPtr->crc.crc32 = crc32;
		}
		break;
	}
	case BLOCK_STATE_CALC_CRC_READ:
		NVM_ASSERT(bPtr->RamBlockDataAddress != NULL );

		/* Calculate RAM CRC checksum */
		if( bPtr->BlockCRCType == NVM_CRC16 ) {
			NVM_ASSERT(0);		/* TOOD: Not supported, yet */
		} else {
			/* @req 3.1.5/NVM253 */

			/* Calculate CRC on the data we just read to RAM. Compare with CRC that is located in NV block */
			admPtr->crc.crc32 = Crc_CalculateCRC32(bPtr->RamBlockDataAddress,bPtr->NvBlockLength,0xffffffffUL);
			if( READ32_NA(bPtr->RamBlockDataAddress + bPtr->NvBlockLength ) !=  admPtr->crc.crc32 ) {
				/* TODO: Corrupt CRC */
				NVM_ASSERT(0);
			} else {
				admPtr->BlockChanged = BLOCK_STATE_MEMIF_REQ;
			}
		}
		break;
	default:
		NVM_ASSERT(0);
		break;
	}

	if( blockDone  ) {
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
					true );
}


#if 0
/*
 * Handles the MemIf result of one block read
 */
static void ReadAllCheckReadResult(void)
{
	if (MemIfJobAdmin.JobStatus == E_OK) {
		if (MemIfJobAdmin.BlockDescriptor->BlockUseCrc) {
			MemIfJobAdmin.BlockAdmin->BlockState = BLOCK_STATE_POSTCALC_CRC; /** @req NVM292 */
		} else {
			MemIfJobAdmin.BlockAdmin->BlockState = BLOCK_STATE_MEMIF_REQ;
			MemIfJobAdmin.BlockAdmin->ErrorStatus = NVM_REQ_OK;
			MemIfJobAdmin.BlockAdmin->BlockValid = TRUE;
			MemIfJobAdmin.BlockAdmin->BlockChanged = FALSE;

			if (MemIfJobAdmin.BlockDescriptor->WriteBlockOnce) {
				MemIfJobAdmin.BlockAdmin->BlockWriteProtected = TRUE;
			}

			if (MemIfJobAdmin.BlockDescriptor->SingleBlockCallback != NULL) {
				(void)MemIfJobAdmin.BlockDescriptor->SingleBlockCallback(NVM_READ_ALL_ID, MemIfJobAdmin.BlockAdmin->ErrorStatus); /** @req NVM281 */
			}
		}
	} else {
		// TODO: Handle reread from NVRAM if type NVM_BLOCK_REDUNDANT
		// TODO: Handle read from ROM if available

		// Read has failed
		AdminMultiReq.PendingErrorStatus = NVM_REQ_NOT_OK;

		MemIfJobAdmin.BlockAdmin->BlockState = BLOCK_STATE_MEMIF_REQ;
		MemIfJobAdmin.BlockAdmin->BlockValid = FALSE;
		MemIfJobAdmin.BlockAdmin->BlockChanged = FALSE;

		switch (MemIfJobAdmin.JobResult ) {
		case MEMIF_BLOCK_INVALID:
			MemIfJobAdmin.BlockAdmin->ErrorStatus = NVM_REQ_NV_INVALIDATED;	/** @req NVM342 */
			break;

		case MEMIF_BLOCK_INCONSISTENT:										/** @req NVM360 */
			MemIfJobAdmin.BlockAdmin->ErrorStatus = NVM_REQ_INTEGRITY_FAILED;
			DEM_REPORTERRORSTATUS(NVM_E_INTEGRITY_FAILED,DEM_EVENT_STATUS_FAILED);
			break;

		case MEMIF_JOB_FAILED:												/** @req NVM361 */
			MemIfJobAdmin.BlockAdmin->ErrorStatus = NVM_REQ_NOT_OK;
			DEM_REPORTERRORSTATUS(NVM_E_REQ_FAILED,DEM_EVENT_STATUS_FAILED);
			break;

		default:
			MemIfJobAdmin.BlockAdmin->ErrorStatus = NVM_REQ_NOT_OK;
			break;
		}

		if (MemIfJobAdmin.BlockDescriptor->SingleBlockCallback != NULL) {
			(void)MemIfJobAdmin.BlockDescriptor->SingleBlockCallback(NVM_SERVICE_ID, MemIfJobAdmin.BlockAdmin->ErrorStatus); /** @req NVM281 */
		}
	}
	nvmState = NVM_READ_ALL_PROCESSING;
}
#endif


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
					false,
					true );

#if 0
	const NvM_BlockDescriptorType	*BlockDescriptorList = NvM_Config.BlockDescriptor;
	AdministrativeBlockType *AdminBlockTable = AdminBlock;
	uint16 i;

	// Search forward to first unfinished block
	while ((AdminMultiReq.currBlockIndex < NVM_NUM_OF_NVRAM_BLOCKS) && (AdminBlockTable[AdminMultiReq.currBlockIndex].ErrorStatus != NVM_REQ_PENDING)) {
		AdminMultiReq.currBlockIndex++;
	}

	if (AdminMultiReq.currBlockIndex == NVM_NUM_OF_NVRAM_BLOCKS) {
		// All block processed
		if (AdminMultiReq.PendingErrorStatus == NVM_REQ_OK) {
			AdminMultiBlock.ErrorStatus = NVM_REQ_OK;
		} else {
			AdminMultiBlock.ErrorStatus = NVM_REQ_NOT_OK;
		}

		nvmState = NVM_IDLE;

		if (NvM_Config.Common.MultiBlockCallback != NULL) {
			NvM_Config.Common.MultiBlockCallback(NVM_SERVICE_ID, AdminMultiBlock.ErrorStatus);
		}

	} else {
		for (i = AdminMultiReq.currBlockIndex; (i < NVM_NUM_OF_NVRAM_BLOCKS) && (nvmState == NVM_WRITE_ALL_PROCESSING); i++) {
			switch (AdminBlockTable[i].BlockState) {
			case BLOCK_STATE_POSTCALC_CRC_DONE:
				// TODO: Check CRC
				break;

			case BLOCK_STATE_RECALC_CRC_DONE:
				// TODO: If CRC is ok do not reload from NVRAM
				// TODO: else reload
				break;

			case BLOCK_STATE_WRITE_TO_NV:
				nvmState = NVM_WRITE_ALL_PENDING;
				WriteBlock(&BlockDescriptorList[i], &AdminBlockTable[i], 0, BlockDescriptorList[i].RamBlockDataAddress);
				break;

			default:
				break;
			} // Switch
		} // for
	} // else
#endif

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
#if 0
	AdministrativeBlockType *admPtr = AdminBlock;
	const NvM_BlockDescriptorType	*bPtr = NvM_Config.BlockDescriptor;
	uint16 i;
#endif

	VALIDATE_NO_RV(nvmState != NVM_UNINITIALIZED, NVM_READ_ALL_ID, NVM_E_NOT_INITIALIZED);

	NVM_ASSERT(nvmState == NVM_IDLE);

	AdminMultiReq.state = NVM_WRITE_ALL;
	AdminMultiBlock.ErrorStatus = NVM_REQ_PENDING;

#if 0
	// Check state
	if (nvmState == NVM_IDLE) {
//		nvmState = NVM_WRITE_ALL_REQUESTED;		/** @req NVM254 */
		nvmState = NVM_WRITE_ALL;

		// Set status to pending in the administration blocks
		AdminMultiBlock.ErrorStatus = NVM_REQ_PENDING;

		for (i = 0; i < NVM_NUM_OF_NVRAM_BLOCKS; i++) {
			admPtr->ErrorStatus = NVM_REQ_PENDING;		/** @req NVM549 */
			if( bPtr->BlockUseCrc ) {
				NVM_ASSERT ( bPtr->RamBlockDataAddress != NULL );
				/* @req 3.1.5/NVM253 */
				admPtr->BlockState = BLOCK_STATE_CALC_CRC_WRITE;

			} else {
				admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
			}

			admPtr++;
			bPtr++;
		}
	}
#endif
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
	} else {
		*requestResultPtr = AdminBlock[blockId-1].ErrorStatus;
	}

}


#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)	/** @req NVM408 */
/*
 * Procedure:	Nvm_SetRamBlockStatus
 * Reentrant:	Yes
 */
void Nvm_SetRamBlockStatus(NvM_BlockIdType blockId, boolean blockChanged)
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



#if 0
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

	/* !req 3.1.5/NVM224 */
	JobQueueType *job = CirqBuffGetNext(&Nvm_Global.jobQUeue);

	job.blockId = blockId;
	job.destPtr = NvM_DestPtr;
}
#endif

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





/**
 *
 */
void NvM_MainFunction(void)
{
	int 			rv;
	Nvm_QueueType 	qEntry;
	const NvM_BlockDescriptorType *	bList = NvM_Config.BlockDescriptor;
	const NvM_BlockDescriptorType *	nvmBlock;
	const NvM_BlockDescriptorType *	currBlock;
	AdministrativeBlockType *admBlock;
	static uint32 crc32;
	static uint32 crc32Left;

	/* Check for new requested state changes */
	if( nvmState == NVM_IDLE ) {
		DEBUG_PRINTF("### Popping FIFO ###\n");
		rv = CirqBuffPop( &nvmQueue, &qEntry );
		if( rv == 0 ) {
			/* Buffer not empty */
			nvmState = qEntry.op;
			nvmBlock = &bList[qEntry.blockId-1];
			admBlock = &AdminBlock[qEntry.blockId-1];
			nvmSubState = 0;
			admBlock->ErrorStatus = NVM_REQ_PENDING;
		} else {
			/* Check multiblock req */
			if( AdminMultiReq.state != NVM_UNINITIALIZED ) {
				nvmState = AdminMultiReq.state ;
				nvmSubState = 0;
				nvmBlock = 0;
				admBlock = 0;
				AdminMultiReq.state = NVM_UNINITIALIZED;
			}
		}
	}

	DEBUG_STATE(nvmState,nvmSubState);

	switch (nvmState) {
	case NVM_UNINITIALIZED:
		break;

#if 0
	case NVM_READ_BLOCK:
	{
		switch(nvmSubState) {
		case RB_START:
			/* Copy from NVRAM block to RAM block */
			/** @req 3.1.5/NVM196 */
			if( qEntry.dataPtr == NULL) {
				/* Its a permanent RAM block  */
				NVM_ASSERT( nvmBlock->RamBlockDataAddress != NULL ); 	/** @req 3.1.5/NVM278 */
				admBlock->BlockValid = 0;							/** @req 3.1.5/NVM198 */
				qEntry.dataPtr = nvmBlock->RamBlockDataAddress;
			}
			/* Start to Read from MemIf */
			nvmSetnr = 0;
			ReadBlock( nvmBlock, admBlock, nvmSetnr, qEntry.dataPtr);
			nvmSubState = RB_WAIT_READ;
			break;

		case RB_WAIT_READ:

			/* Application "perspective" in 7.2.2.16.2 */

			if (MemIf_GetStatus(nvmBlock->NvramDeviceId) != MEMIF_IDLE) {
				/* ..job not done... continue to read from NVRAM to RAM */
				break;
			}

			/* The MemIf job (reading from NVRAM to RAM) is done, check the result */
			if( MemIf_GetJobResult(nvmBlock->NvramDeviceId) == MEMIF_JOB_OK) {
				/** @req 3.1.5/NVM199 */
				switch(nvmBlock->BlockManagementType) {
				case NVM_BLOCK_REDUNDANT:
					NVM_ASSERT(nvmBlock->NvBlockNum == 2);		/* at least 2 block according to NVM480 */
					break;
				case NVM_BLOCK_NATIVE:
					/* TODO */
					break;
				case NVM_BLOCK_DATASET:
					/* TODO */
					break;
				}


				/* Calculate CRC in the background */
				if( nvmBlock->BlockUseCrc) {
					crc32 = (-1UL);
					crc32Left = nvmBlock->NvBlockLength;
					nvmSubState = RB_CALC_CHECKSUM;
				}
			} else {
				if ( NVM_BLOCK_REDUNDANT == nvmBlock->BlockManagementType) {
					/* Try to read the redundant block */
					if( RB_CALC_CHECKSUM == 0) {
						nvmSetnr = 1;
						ReadBlock( nvmBlock, admBlock, 1, qEntry.dataPtr);
						nvmSubState = RB_WAIT_READ;
					} else {
						/* TODO: We just failed the redundant block. Read from ROM block? */
					}
				}
			}
			break;
		case RB_CALC_CHECKSUM:
		{
			crc32NumToCalc = MIN(NVM_CRC_NUM_OF_BYTES,crc32Left);
			crc32 = Crc_CalculatateCRC32(qEntry.dataPtr, crc32Left , crc32 );
			crc32Left -= crc32NumToCalc;
			if( crc32Left == 0) {
				/* Compare NVRAM and RAM checksum */
				if( crc32 != )
			}
		}
			break;
		default:
			break;
		}	/* switch */
	break;
#endif


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
		DriveBlock(nvmBlock,admBlock, qEntry.dataPtr, false, false );
		break;

	case NVM_WRITE_BLOCK:
		DriveBlock(nvmBlock,admBlock, qEntry.dataPtr, false, false );
		break;

	case NVM_WRITE_ALL:
		if( NS_INIT == nvmSubState ) {
			WriteAllInit();
			nvmSubState = NS_PROSSING;
		} else if( NS_PROSSING == nvmSubState ) {
			WriteAllMain();
		}
		break;


#if 0
	case NVM_WRITE_ALL:
		WriteAllMain();
		break;
#endif

#if 0
	case NVM_READ_ALL_PROCESSING:
		if (MemIf_GetStatus() == MEMIF_IDLE) {
			ReadAllMain();
		}
		CalcCrc();
		break;

	case NVM_READ_ALL_PENDING:
		if (CheckMemIfJobFinished()) {
			ReadAllCheckReadResult();
		}
		CalcCrc();
		break;

	case NVM_WRITE_ALL_REQUESTED:
		WriteAllInit();
		break;

	case NVM_WRITE_ALL_PROCESSING:
		if (MemIf_GetStatus() == MEMIF_IDLE) {
			WriteAllMain();
		}
		CalcCrc();
		break;

	case NVM_WRITE_ALL_PENDING:
		if (CheckMemIfJobFinished()) {
			WriteAllCheckWriteResult();
		}
		CalcCrc();
		break;
#endif

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
	MemIfJobAdmin.JobResult = MemIf_GetJobResult();
}

/*
 * Procedure:	NvM_JobErrorNotification
 * Reentrant:	No
 */
void NvM_JobErrorNotification(void)
{
	MemIfJobAdmin.JobFinished = TRUE;
	MemIfJobAdmin.JobStatus = E_NOT_OK;
	MemIfJobAdmin.JobResult = MemIf_GetJobResult();
}
#endif

