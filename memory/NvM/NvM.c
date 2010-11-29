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
 *  General requirements
 */
/** @req NVM076 */
/** @req NVM552 */
/** @req NVM689 */


/*
 * NB! Even though some code exist for handling crc, the functionality is not complete
 * and shall not be used.
 */


#include "NvM.h"
#include "Rte.h" // ???
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include "MemIf.h"
//#include "SchM_NvM.h"
#include "MemMap.h"
//#include "Crc.h" // Optional

/*
 * Local definitions
 */
#define NVM_SERVICE_ID		0x00		// TODO: What number shall this ID have?

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

// State variable
typedef enum {
  NVM_UNINITIALIZED = 0,
  NVM_IDLE,
  NVM_READ_ALL_REQUESTED,
  NVM_READ_ALL_PROCESSING,
  NVM_READ_ALL_PENDING,
  NVM_WRITE_ALL_REQUESTED,
  NVM_WRITE_ALL_PROCESSING,
  NVM_WRITE_ALL_PENDING
} NvmStateType;

typedef enum {
	MEMIF_STATE_IDLE,
	MEMIF_STATE_PENDING
} MemIfStateType;

typedef enum {
	BLOCK_STATE_IDLE,
	BLOCK_STATE_RECALC_CRC,
	BLOCK_STATE_RECALC_CRC_DONE,
	BLOCK_STATE_POSTCALC_CRC,
	BLOCK_STATE_POSTCALC_CRC_DONE,

	BLOCK_STATE_LOAD_FROM_NV,
	BLOCK_STATE_LOAD_FROM_NV_DONE,
	BLOCK_STATE_LOAD_FROM_NV_REDUNDANT,
	BLOCK_STATE_LOAD_FROM_ROM,

	BLOCK_STATE_WRITE_TO_NV,
	BLOCK_STATE_WRITE_TO_NV_DONE,
} BlockStateType;


static NvmStateType nvmState = NVM_UNINITIALIZED;

typedef struct {
	BlockStateType			BlockState;
	uint8					DataIndex;				// Selected data index if "Data Set" type
	boolean					BlockWriteProtected;	// Block write protected?
	NvM_RequestResultType	ErrorStatus;			// Status of block
	boolean					BlockChanged;			// Block changed?
	boolean					BlockValid;				// Block valid?
	uint8					NumberOfWriteFailed;	// Current write retry cycle
} AdministrativeBlockType;

static AdministrativeBlockType AdminBlock[NVM_NUM_OF_NVRAM_BLOCKS];

typedef struct {
	NvM_RequestResultType	ErrorStatus;			// Status from multi block requests i.e. Read/Write/CancelWrite-all
} AdministrativeMultiBlockType;

static AdministrativeMultiBlockType AdminMultiBlock;

/*
 * This function needs to be implemented!
 */
static void CalcCrc(void)
{
	// TODO: Calculate CRC
}

typedef struct {
	boolean JobFinished;
	Std_ReturnType JobStatus;
	MemIf_JobResultType JobResult;
	const NvM_BlockDescriptorType *BlockDescriptor;
	AdministrativeBlockType *BlockAdmin;
} MemIfJobAdminType;

static MemIfJobAdminType MemIfJobAdmin = {
		.JobFinished = TRUE,
		.JobStatus = E_OK,
		.JobResult = MEMIF_JOB_OK,
		.BlockDescriptor = NULL,
		.BlockAdmin = NULL
};

typedef struct {
	uint16					NextBlockIndex;		// Keeps track of next unfinished block
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


/*
 * Request a read of a block from MemIf
 */
static void ReadBlock(const NvM_BlockDescriptorType *blockDescriptor, AdministrativeBlockType *adminBlock, uint8 setNumber, uint8 *destAddress)
{
	Std_ReturnType returnCode;
	uint16 blockOffset = 0;	// TODO: How to calculate this?

	if (setNumber < blockDescriptor->NvBlockNum) {
		SetMemifJobBusy();
		MemIfJobAdmin.BlockAdmin = adminBlock;
		MemIfJobAdmin.BlockDescriptor = blockDescriptor;
		returnCode = MemIf_Read(blockDescriptor->NvramDeviceId, (blockDescriptor->NvBlockBaseNumber << NVM_DATASET_SELECTION_BITS) | setNumber, blockOffset, destAddress, blockDescriptor->NvBlockLength);
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

	nvmState = NVM_READ_ALL_PROCESSING;
	AdminMultiReq.PendingErrorStatus = NVM_REQ_OK;
	AdminMultiReq.NextBlockIndex = 0;

	for (i = 0; i < NVM_NUM_OF_NVRAM_BLOCKS; i++) {
		if (BlockDescriptorList->SelectBlockForReadall
#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)						/** @req NVM345 */
				&& (!AdminBlockTable->BlockValid		// TODO: Check if this is to be done like this
				|| !AdminBlockTable->BlockChanged)		// TODO: Check if this is to be done like this
#endif
				) {
			VALIDATE_NO_RV(BlockDescriptorList->RamBlockDataAddress != NULL, NVM_READ_ALL_ID, NVM_E_WRONG_CONFIG);
			VALIDATE_NO_RV(BlockDescriptorList->BlockManagementType != NVM_BLOCK_DATASET, NVM_READ_ALL_ID, NVM_E_WRONG_CONFIG);
			if (BlockDescriptorList->CalcRamBlockCrc) {
				VALIDATE(BlockDescriptorList->BlockUseCrc == STD_ON, NVM_READ_ALL_ID, NVM_E_BLOCK_CONFIG);
				AdminBlockTable->BlockState = BLOCK_STATE_RECALC_CRC;
			} else {
				AdminBlockTable->BlockState = BLOCK_STATE_LOAD_FROM_NV;
			}
		} else {
			AdminBlockTable->ErrorStatus = NVM_REQ_BLOCK_SKIPPED;
		}

		AdminBlockTable++;
		BlockDescriptorList++;
	}
}


/*
 * Main function for the read all job
 */
static void ReadAllMain(void)
{
	const NvM_BlockDescriptorType	*BlockDescriptorList = NvM_Config.BlockDescriptor;
	AdministrativeBlockType *AdminBlockTable = AdminBlock;
	uint16 i;

	// Search forward to first unfinished block
	while ((AdminMultiReq.NextBlockIndex < NVM_NUM_OF_NVRAM_BLOCKS) && (AdminBlockTable[AdminMultiReq.NextBlockIndex].ErrorStatus != NVM_REQ_PENDING)) {
		AdminMultiReq.NextBlockIndex++;
	}

	if (AdminMultiReq.NextBlockIndex == NVM_NUM_OF_NVRAM_BLOCKS) {
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
		for (i = AdminMultiReq.NextBlockIndex; (i < NVM_NUM_OF_NVRAM_BLOCKS) && (nvmState == NVM_READ_ALL_PROCESSING); i++) {
			switch (AdminBlockTable[i].BlockState) {
			case BLOCK_STATE_POSTCALC_CRC_DONE:
				// TODO: Check CRC
				break;

			case BLOCK_STATE_RECALC_CRC_DONE:
				// TODO: If CRC is ok do not reload from NVRAM
				// TODO: else reload
				break;

			case BLOCK_STATE_LOAD_FROM_NV:
				nvmState = NVM_READ_ALL_PENDING;
				ReadBlock(&BlockDescriptorList[i], &AdminBlockTable[i], 0, BlockDescriptorList[i].RamBlockDataAddress);
				break;

			default:
				break;
			} // Switch
		} // for
	} // else
}


/*
 * Handles the MemIf result of one block read
 */
static void ReadAllCheckReadResult(void)
{
	if (MemIfJobAdmin.JobStatus == E_OK) {
		if (MemIfJobAdmin.BlockDescriptor->BlockUseCrc) {
			MemIfJobAdmin.BlockAdmin->BlockState = BLOCK_STATE_POSTCALC_CRC; /** @req NVM292 */
		} else {
			MemIfJobAdmin.BlockAdmin->BlockState = BLOCK_STATE_IDLE;
			MemIfJobAdmin.BlockAdmin->ErrorStatus = NVM_REQ_OK;
			MemIfJobAdmin.BlockAdmin->BlockValid = TRUE;
			MemIfJobAdmin.BlockAdmin->BlockChanged = FALSE;

			if (MemIfJobAdmin.BlockDescriptor->WriteBlockOnce) {
				MemIfJobAdmin.BlockAdmin->BlockWriteProtected = TRUE;
			}

			if (MemIfJobAdmin.BlockDescriptor->SingleBlockCallback != NULL) {
				MemIfJobAdmin.BlockDescriptor->SingleBlockCallback(NVM_SERVICE_ID, MemIfJobAdmin.BlockAdmin->ErrorStatus); /** @req NVM281 */
			}
		}
	} else {
		// TODO: Handle reread from NVRAM if type NVM_BLOCK_REDUNDANT
		// TODO: Handle read from ROM if available

		// Read has failed
		AdminMultiReq.PendingErrorStatus = NVM_REQ_NOT_OK;

		MemIfJobAdmin.BlockAdmin->BlockState = BLOCK_STATE_IDLE;
		MemIfJobAdmin.BlockAdmin->BlockValid = FALSE;
		MemIfJobAdmin.BlockAdmin->BlockChanged = FALSE;

		switch (MemIfJobAdmin.JobResult ) {
		case MEMIF_BLOCK_INVALID:
			MemIfJobAdmin.BlockAdmin->ErrorStatus = NVM_REQ_NV_INVALIDATED;	/** @req NVM342 */
			break;

		case MEMIF_BLOCK_INCONSISTENT:										/** @req NVM360 */
			MemIfJobAdmin.BlockAdmin->ErrorStatus = NVM_REQ_INTEGRITY_FAILED;
#if defined(USE_DEM)
		Dem_ReportErrorStatus(NVM_E_INTEGRITY_FAILED,DEM_EVENT_STATUS_FAILED);
#endif
			break;

		case MEMIF_JOB_FAILED:												/** @req NVM361 */
			MemIfJobAdmin.BlockAdmin->ErrorStatus = NVM_REQ_NOT_OK;
#if defined(USE_DEM)
			Dem_ReportErrorStatus(NVM_E_REQ_FAILED,DEM_EVENT_STATUS_FAILED);
#endif
			break;

		default:
			MemIfJobAdmin.BlockAdmin->ErrorStatus = NVM_REQ_NOT_OK;
			break;
		}

		if (MemIfJobAdmin.BlockDescriptor->SingleBlockCallback != NULL) {
			MemIfJobAdmin.BlockDescriptor->SingleBlockCallback(NVM_SERVICE_ID, MemIfJobAdmin.BlockAdmin->ErrorStatus); /** @req NVM281 */
		}
	}
	nvmState = NVM_READ_ALL_PROCESSING;
}


/*
 * Request writing of a block to MemIf
 */
static void WriteBlock(const NvM_BlockDescriptorType *blockDescriptor, AdministrativeBlockType *adminBlock, uint8 setNumber, uint8 *sourceAddress)
{
	Std_ReturnType returnCode;

	if (setNumber < blockDescriptor->NvBlockNum) {
		SetMemifJobBusy();
		MemIfJobAdmin.BlockAdmin = adminBlock;
		MemIfJobAdmin.BlockDescriptor = blockDescriptor;
		returnCode = MemIf_Write(blockDescriptor->NvramDeviceId, (blockDescriptor->NvBlockBaseNumber << NVM_DATASET_SELECTION_BITS) | setNumber, sourceAddress);
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

	nvmState = NVM_WRITE_ALL_PROCESSING;
	AdminMultiReq.PendingErrorStatus = NVM_REQ_OK;
	AdminMultiReq.NextBlockIndex = 0;

	for (i = 0; i < NVM_NUM_OF_NVRAM_BLOCKS; i++) {
		if ((BlockDescriptorList->RamBlockDataAddress != NULL)
#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)						/** @req NVM344 */
				&& AdminBlockTable->BlockValid						/** @req NVM682 */
				&& AdminBlockTable->BlockChanged					/** @req NVM682 */
#endif
				&& !AdminBlockTable->BlockWriteProtected){			/** @req NVM432 *//** @req NVM433 */
			if (BlockDescriptorList->BlockUseCrc) {
				AdminBlockTable->BlockState = BLOCK_STATE_RECALC_CRC;	/** @req NVM253 */
			} else {
				AdminBlockTable->BlockState = BLOCK_STATE_WRITE_TO_NV;
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
	const NvM_BlockDescriptorType	*BlockDescriptorList = NvM_Config.BlockDescriptor;
	AdministrativeBlockType *AdminBlockTable = AdminBlock;
	uint16 i;

	// Search forward to first unfinished block
	while ((AdminMultiReq.NextBlockIndex < NVM_NUM_OF_NVRAM_BLOCKS) && (AdminBlockTable[AdminMultiReq.NextBlockIndex].ErrorStatus != NVM_REQ_PENDING)) {
		AdminMultiReq.NextBlockIndex++;
	}

	if (AdminMultiReq.NextBlockIndex == NVM_NUM_OF_NVRAM_BLOCKS) {
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
		for (i = AdminMultiReq.NextBlockIndex; (i < NVM_NUM_OF_NVRAM_BLOCKS) && (nvmState == NVM_WRITE_ALL_PROCESSING); i++) {
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
}


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
		MemIfJobAdmin.BlockAdmin->BlockState = BLOCK_STATE_IDLE;
		MemIfJobAdmin.BlockAdmin->ErrorStatus = NVM_REQ_OK;

		if (MemIfJobAdmin.BlockDescriptor->SingleBlockCallback != NULL) {
			MemIfJobAdmin.BlockDescriptor->SingleBlockCallback(NVM_SERVICE_ID, MemIfJobAdmin.BlockAdmin->ErrorStatus);
		}
	} else {
		MemIfJobAdmin.BlockAdmin->NumberOfWriteFailed++;
		if (MemIfJobAdmin.BlockAdmin->NumberOfWriteFailed > NVM_MAX_NUMBER_OF_WRITE_RETRIES) {
			// TODO: Check if redundant block shall be written NVM337

			// Write has failed
			AdminMultiReq.PendingErrorStatus = NVM_REQ_NOT_OK;

			MemIfJobAdmin.BlockAdmin->BlockState = BLOCK_STATE_IDLE;
			MemIfJobAdmin.BlockAdmin->ErrorStatus = NVM_REQ_NOT_OK;	/** @req NVM296 */
#if defined(USE_DEM)
			Dem_ReportErrorStatus(NVM_E_REQ_FAILED,DEM_EVENT_STATUS_FAILED);
#endif

			if (MemIfJobAdmin.BlockDescriptor->SingleBlockCallback != NULL) {
				MemIfJobAdmin.BlockDescriptor->SingleBlockCallback(NVM_SERVICE_ID, MemIfJobAdmin.BlockAdmin->ErrorStatus);
			}
		}
	}
	nvmState = NVM_WRITE_ALL_PROCESSING;
}


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
	nvmState = NVM_IDLE;	/** @req NVM399 */
}


/*
 * Procedure:	NvM_ReadAll
 * Reentrant:	No
 */
void NvM_ReadAll(void)
{
	AdministrativeBlockType *AdminBlockTable = AdminBlock;
	uint16 i;

	VALIDATE_NO_RV(nvmState != NVM_UNINITIALIZED, NVM_READ_ALL_ID, NVM_E_NOT_INITIALIZED);
	// Check queue
	// TODO: Check queue

	// Check state
	if (nvmState == NVM_IDLE) {
		nvmState = NVM_READ_ALL_REQUESTED;		/** @req NVM243 */

		// Set status to pending in the administration blocks
		AdminMultiBlock.ErrorStatus = NVM_REQ_PENDING;

		for (i = 0; i < NVM_NUM_OF_NVRAM_BLOCKS; i++) {
			AdminBlockTable->ErrorStatus = NVM_REQ_PENDING;
			AdminBlockTable++;
		}
	}
}


/*
 * Procedure:	NvM_WriteAll
 * Reentrant:	No
 */
void NvM_WriteAll(void)
{
	AdministrativeBlockType *AdminBlockTable = AdminBlock;
	uint16 i;

	VALIDATE_NO_RV(nvmState != NVM_UNINITIALIZED, NVM_READ_ALL_ID, NVM_E_NOT_INITIALIZED);
	// Check queue
	// TODO: Check queue

	// Check state
	if (nvmState == NVM_IDLE) {
		nvmState = NVM_WRITE_ALL_REQUESTED;		/** @req NVM254 */

		// Set status to pending in the administration blocks
		AdminMultiBlock.ErrorStatus = NVM_REQ_PENDING;

		for (i = 0; i < NVM_NUM_OF_NVRAM_BLOCKS; i++) {
			AdminBlockTable->ErrorStatus = NVM_REQ_PENDING;		/** @req NVM549 */
			AdminBlockTable++;
		}
	}
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
				AdminBlock[blockId-1].BlockState = BLOCK_STATE_RECALC_CRC;	/** @req NVM121 */
			}
		} else {
			AdminBlock[blockId-1].BlockChanged = FALSE;	/** @req NVM405 */
			AdminBlock[blockId-1].BlockValid = FALSE;
		} // else blockChanged
	} // if permanent block
}
#endif


/***************************************
 *         Scheduled functions         *
 ***************************************/
/*
 * Procedure:	NvM_MainFunction
 * Reentrant:	No
 */
void NvM_MainFunction(void)
{
	switch (nvmState) {
	case NVM_UNINITIALIZED:
		break;

	case NVM_IDLE:
		CalcCrc();
		break;

	case NVM_READ_ALL_REQUESTED:
		ReadAllInit();
		break;

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

