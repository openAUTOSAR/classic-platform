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

AdministrativeBlockType AdminBlock[NVM_NUM_OF_NVRAM_BLOCKS];

typedef struct {
	NvM_RequestResultType	ErrorStatus;			// Status from multi block requests i.e. Read/Write/CancelWrite-all
} AdministrativeMultiBlockType;

AdministrativeMultiBlockType AdminMultiBlock;

static void CalcCrc(void)
{
	// TODO: Calculate CRC
}

typedef struct {
	MemIfStateType State;
	Std_ReturnType Status;
	MemIf_JobResultType JobResult;
	const NvM_BlockDescriptorType *BlockDescriptor;
	AdministrativeBlockType *BlockAdmin;
} MemIfAdminType;

static MemIfAdminType MemIfAdmin = {
		.State = MEMIF_IDLE,
		.Status = E_OK,
		.JobResult = MEMIF_JOB_OK,
		.BlockDescriptor = NULL,
		.BlockAdmin = NULL
};

typedef struct {
	uint16					NextBlockIndex;		// Keeps track of next unfinished block
	NvM_RequestResultType	PendingErrorStatus;	// Status from multi block requests i.e. Read/Write/CancelWrite-all
} AdminMultiReqType;

static AdminMultiReqType AdminMultiReq;


static void ReadBlock(const NvM_BlockDescriptorType *blockDescriptor, AdministrativeBlockType *adminBlock, uint8 setNumber, uint8 *destAddress)
{
	Std_ReturnType returnCode;
	uint16 blockOffset = 0;	// TODO: How to calculate this?
	MemIfAdmin.State = MEMIF_STATE_PENDING;

	if (setNumber < blockDescriptor->NvBlockNum) {
		returnCode = MemIf_Read(blockDescriptor->NvramDeviceId, blockDescriptor->NvBlockBaseNumber << NVM_DATASET_SELECTION_BITS || setNumber, blockOffset, destAddress, blockDescriptor->NvBlockLength);
		MemIfAdmin.BlockAdmin = adminBlock;
		MemIfAdmin.BlockDescriptor = blockDescriptor;
		if (returnCode != E_OK) {
			MemIfAdmin.State = MEMIF_STATE_IDLE;
			MemIfAdmin.Status = E_NOT_OK;
			MemIfAdmin.JobResult = MEMIF_JOB_FAILED;
		}
	} else if (setNumber < blockDescriptor->NvBlockNum + blockDescriptor->RomBlockNum) {
		// TODO: Read from ROM
	} else {
		// Error: setNumber out of range
		DET_REPORTERROR(MODULE_ID_NVM, 0, NVM_LOC_READ_BLOCK_ID, NVM_PARAM_OUT_OF_RANGE);
	}
}


static void WriteBlock(const NvM_BlockDescriptorType *blockDescriptor, AdministrativeBlockType *adminBlock, uint8 setNumber, uint8 *sourceAddress)
{
	Std_ReturnType returnCode;
	MemIfAdmin.State = MEMIF_STATE_PENDING;

	if (setNumber < blockDescriptor->NvBlockNum) {
		returnCode = MemIf_Write(blockDescriptor->NvramDeviceId, blockDescriptor->NvBlockBaseNumber << NVM_DATASET_SELECTION_BITS || setNumber, sourceAddress);
		MemIfAdmin.BlockAdmin = adminBlock;
		MemIfAdmin.BlockDescriptor = blockDescriptor;
		if (returnCode != E_OK) {
			MemIfAdmin.State = MEMIF_STATE_IDLE;
			MemIfAdmin.Status = E_NOT_OK;
			MemIfAdmin.JobResult = MEMIF_JOB_FAILED;
		}
	} else {
		// Error: setNumber out of range
		DET_REPORTERROR(MODULE_ID_NVM, 0, NVM_LOC_WRITE_BLOCK_ID, NVM_PARAM_OUT_OF_RANGE);
	}
}


#if (NVM_POLLING_MODE == STD_ON)
static void PollMemIfJobResult(void)
{
	MemIf_JobResultType jobResult;

	jobResult = MemIf_GetJobResult();

	if (jobResult == MEMIF_JOB_OK) {
		MemIfAdmin.State = MEMIF_STATE_IDLE;
		MemIfAdmin.Status = E_OK;
		MemIfAdmin.JobResult = jobResult;
	} else if (jobResult != MEMIF_JOB_PENDING) {
		MemIfAdmin.State = MEMIF_STATE_IDLE;
		MemIfAdmin.Status = E_NOT_OK;
		MemIfAdmin.JobResult = jobResult;
	}
}
#else
void NvM_JobEndNotification(void)
{
	MemIfAdmin.State = MEMIF_STATE_IDLE;
	MemIfAdmin.Status = E_OK;
	MemIfAdmin.JobResult = MemIf_GetJobResult();
}

void NvM_JobErrorNotification(void)
{
	MemIfAdmin.State = MEMIF_STATE_IDLE;
	MemIfAdmin.Status = E_NOT_OK;
	MemIfAdmin.JobResult = MemIf_GetJobResult();
}
#endif


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


void ReadAllCheckReadResult(void)
{
	if (MemIfAdmin.Status == E_OK) {
		if (MemIfAdmin.BlockDescriptor->BlockUseCrc) {
			MemIfAdmin.BlockAdmin->BlockState = BLOCK_STATE_POSTCALC_CRC; /** @req NVM292 */
		} else {
			MemIfAdmin.BlockAdmin->BlockState = BLOCK_STATE_IDLE;
			MemIfAdmin.BlockAdmin->ErrorStatus = NVM_REQ_OK;
			MemIfAdmin.BlockAdmin->BlockValid = TRUE;
			MemIfAdmin.BlockAdmin->BlockChanged = FALSE;

			if (MemIfAdmin.BlockDescriptor->SingleBlockCallback != NULL) {
				MemIfAdmin.BlockDescriptor->SingleBlockCallback(NVM_SERVICE_ID, MemIfAdmin.BlockAdmin->ErrorStatus); /** @req NVM281 */
			}
		}
	} else {
		// TODO: Handle reread from NVRAM if type NVM_BLOCK_REDUNDANT
		// TODO: Handle read from ROM if available

		// Read has failed
		AdminMultiReq.PendingErrorStatus = NVM_REQ_NOT_OK;

		MemIfAdmin.BlockAdmin->BlockState = BLOCK_STATE_IDLE;
		MemIfAdmin.BlockAdmin->BlockValid = FALSE;
		MemIfAdmin.BlockAdmin->BlockChanged = FALSE;
		switch (MemIfAdmin.JobResult ) {
		case MEMIF_BLOCK_INVALID:
			MemIfAdmin.BlockAdmin->ErrorStatus = NVM_REQ_NV_INVALIDATED;	/** @req NVM342 */
			break;

		case MEMIF_BLOCK_INCONSISTENT:
			MemIfAdmin.BlockAdmin->ErrorStatus = NVM_REQ_INTEGRITY_FAILED;	/** @req NVM360 */
			break;

		default:
			MemIfAdmin.BlockAdmin->ErrorStatus = NVM_REQ_NOT_OK;
			break;
		}

		if (MemIfAdmin.BlockDescriptor->SingleBlockCallback != NULL) {
			MemIfAdmin.BlockDescriptor->SingleBlockCallback(NVM_SERVICE_ID, MemIfAdmin.BlockAdmin->ErrorStatus); /** @req NVM281 */
		}
	}
	nvmState = NVM_READ_ALL_PROCESSING;
}


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


static void WriteAllInit(void)
{
	/*
	 * Initiate the write all job
	 */
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


void WriteAllCheckWriteResult(void)
{
	if (MemIfAdmin.Status == E_OK) {
		// TODO: Check if redundant block shall be written NVM337

		if (MemIfAdmin.BlockDescriptor->WriteBlockOnce) {
			MemIfAdmin.BlockAdmin->BlockWriteProtected = TRUE;	/** @req NVM329 */
		}
		MemIfAdmin.BlockAdmin->BlockState = BLOCK_STATE_IDLE;
		MemIfAdmin.BlockAdmin->ErrorStatus = NVM_REQ_OK;

		if (MemIfAdmin.BlockDescriptor->SingleBlockCallback != NULL) {
			MemIfAdmin.BlockDescriptor->SingleBlockCallback(NVM_SERVICE_ID, MemIfAdmin.BlockAdmin->ErrorStatus);
		}
	} else {
		MemIfAdmin.BlockAdmin->NumberOfWriteFailed++;
		if (MemIfAdmin.BlockAdmin->NumberOfWriteFailed > NVM_MAX_NUMBER_OF_WRITE_RETRIES) {
			// TODO: Check if redundant block shall be written NVM337

			// Write has failed
			AdminMultiReq.PendingErrorStatus = NVM_REQ_NOT_OK;

			MemIfAdmin.BlockAdmin->BlockState = BLOCK_STATE_IDLE;
			MemIfAdmin.BlockAdmin->ErrorStatus = NVM_REQ_NOT_OK;
//			Dem_ReportErrorStatus(NVM_E_REQ_FAILED,DEM_EVENT_STATUS_FAILED); TODO: Add this!

			if (MemIfAdmin.BlockDescriptor->SingleBlockCallback != NULL) {
				MemIfAdmin.BlockDescriptor->SingleBlockCallback(NVM_SERVICE_ID, MemIfAdmin.BlockAdmin->ErrorStatus);
			}
		}
	}
	nvmState = NVM_WRITE_ALL_PROCESSING;
}


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


void NvM_CancelWriteAll(void)
{
	
}


void NvM_GetErrorStatus(NvM_BlockIdType blockId, uint8 *requestResultPtr)
{
	VALIDATE_NO_RV(nvmState != NVM_UNINITIALIZED, NVM_GET_ERROR_STATUS_ID, NVM_E_NOT_INITIALIZED);
	VALIDATE_NO_RV(blockId < NVM_NUM_OF_NVRAM_BLOCKS+2, NVM_GET_ERROR_STATUS_ID, NVM_E_PARAM_BLOCK_ID);

	if (blockId == 0) {
		// Multiblock ID
		*requestResultPtr = AdminMultiBlock.ErrorStatus;
	} else if (blockId == 1) {
		// Configuration block id
		// TODO: Shall anything be read here?
	} else {
		*requestResultPtr = AdminBlock[blockId-2].ErrorStatus;
	}

}


#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)	/** @req NVM408 */
void Nvm_SetRamBlockStatus(NvM_BlockIdType blockId, boolean blockChanged)
{
	const NvM_BlockDescriptorType	*BlockDescriptorList = NvM_Config.BlockDescriptor;

	VALIDATE_NO_RV(nvmState != NVM_UNINITIALIZED, NVM_SET_RAM_BLOCK_STATUS_ID, NVM_E_NOT_INITIALIZED);	/** @req NVM497 */
	VALIDATE_NO_RV(blockId < NVM_NUM_OF_NVRAM_BLOCKS+2, NVM_SET_RAM_BLOCK_STATUS_ID, NVM_E_PARAM_BLOCK_ID);
	VALIDATE_NO_RV(blockId > 1, NVM_SET_RAM_BLOCK_STATUS_ID, NVM_E_PARAM_BLOCK_ID);

	if (BlockDescriptorList[blockId-2].RamBlockDataAddress != NULL) {	/** @req NVM240 */
		if (blockChanged) {
			AdminBlock[blockId-2].BlockChanged = TRUE;	/** @req NVM406 */
			AdminBlock[blockId-2].BlockValid = TRUE;	/** @req NVM241 */
			if (BlockDescriptorList[blockId-2].BlockUseCrc) {
				AdminBlock[blockId-2].BlockState = BLOCK_STATE_RECALC_CRC;	/** @req NVM121 */
			}
		} else {
			AdminBlock[blockId-2].BlockChanged = FALSE;	/** @req NVM405 */
			AdminBlock[blockId-2].BlockValid = FALSE;
		} // else blockChanged
	} // if permanent block
}
#endif


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
		ReadAllMain();
		CalcCrc();
		break;

	case NVM_READ_ALL_PENDING:
#if (NVM_POLLING_MODE == STD_ON)
		PollMemIfJobResult();
#endif
		if (MemIfAdmin.State == MEMIF_STATE_IDLE) {
			ReadAllCheckReadResult();
		}
		CalcCrc();
		break;

	case NVM_WRITE_ALL_REQUESTED:
		WriteAllInit();
		break;

	case NVM_WRITE_ALL_PROCESSING:
		WriteAllMain();
		CalcCrc();
		break;

	case NVM_WRITE_ALL_PENDING:
#if (NVM_POLLING_MODE == STD_ON)
		PollMemIfJobResult();
#endif
		if (MemIfAdmin.State == MEMIF_STATE_IDLE) {
			WriteAllCheckWriteResult();
		}
		CalcCrc();
		break;

	default:
		DET_REPORTERROR(MODULE_ID_NVM, 0, NVM_MAIN_FUNCTION_ID, NVM_UNEXPECTED_STATE);
		break;
	}
}

