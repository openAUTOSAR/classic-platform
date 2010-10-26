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









#include <string.h>
#include "Fee.h"
#include "Fee_Cbk.h"
#include "NvM.h"
#include "Fls.h"
#include "Rte.h" // ???
#if defined(USE_DEM)
#include "Dem.h"
#endif
//#include "SchM_NvM.h"
#include "MemMap.h"

/*
 * Local definitions
 */

// Validation macros
#if  ( FEE_DEV_ERROR_DETECT == STD_ON )
#include "Det.h"
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_FEE, 0, _api, _err); \
        }

#define VALIDATE_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_FEE, 0, _api, _err); \
          return _rv; \
        }

#define VALIDATE_NO_RV(_exp,_api,_err ) \
  if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_FEE, 0, _api, _err); \
          return; \
        }
#define DET_REPORTERROR(_module,_instance,_api,_err) Det_ReportError(_module,_instance,_api,_err)

#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_RV(_exp,_api,_err,_rv )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_module,_instance,_api,_err)
#endif

// Block numbering recalculation macros
#define GET_BLOCK_INDEX_FROM_BLOCK_NUMBER(blocknr)	((blocknr >> NVM_DATASET_SELECTION_BITS) - 1)
#define GET_DATASET_FROM_BLOCK_NUMBER(blocknr)	(blocknr & ((1 << NVM_DATASET_SELECTION_BITS) - 1))

// Macro for checking if the flash is ready
#define FLASH_READY	(FlsAdmin.State == FEE_FLS_STATE_READY)

// Macros and variables for flash block administration
#define BLOCK_ADMIN_STATUS_VALID	0x01
#define BLOCK_ADMIN_STATUS_EMPTY	0x02

#define BLOCK_ADMIN_MAGIC_LEN		4
typedef struct {
	uint8	Status;
	uint8	Spare;
	uint8	Magic[BLOCK_ADMIN_MAGIC_LEN];
} FlsBlockAdminType;

static FlsBlockAdminType FlsBlockAdmin;

#define BLOCK_ADMIN_LEN				(sizeof(FlsBlockAdminType))
#define BLOCK_ADMIN_VALIDATE_POS	0
#define BLOCK_ADMIN_MAGIC_POS		2

static const uint8 MagicMaster[BLOCK_ADMIN_MAGIC_LEN] = { 0xeb, 0xba, 0xba, 0xbe };
static const FlsBlockAdminType FlsBlockAdminMaster = {
		.Status = BLOCK_ADMIN_STATUS_VALID | ~BLOCK_ADMIN_STATUS_EMPTY,
		.Magic = { 0xeb, 0xba, 0xba, 0xbe }
};

// Variables for keeping the state and status of the flash
typedef enum {
	FEE_FLS_STATE_IDLE,
	FEE_FLS_STATE_PENDING,
	FEE_FLS_STATE_READY
} FlsStateType;

typedef struct {
	FlsStateType State;
	Std_ReturnType ErrorStatus;
	MemIf_JobResultType JobResult;
} FlsAdminType;

static FlsAdminType FlsAdmin = {
		.State = FEE_FLS_STATE_IDLE,
		.ErrorStatus = E_OK,
		.JobResult = MEMIF_JOB_OK
};

// Variables for quick reporting of status and job result
static MemIf_StatusType ModuleStatus = MEMIF_UNINIT;
static MemIf_JobResultType JobResult = MEMIF_JOB_OK;

// Variables for the current job
typedef enum {
  FEE_UNINITIALIZED = 0,
  FEE_IDLE,
  FEE_WRITE_REQUESTED,
  FEE_WRITE_MAGIC_EREASE_PENDING,
  FEE_WRITE_EREASE_PENDING,
  FEE_WRITE_PENDING,
  FEE_WRITE_BLOCK_ADMIN_PENDING,
  FEE_READ_REQUESTED,
  FEE_READ_BLOCK_ADMIN_PENDING,
  FEE_READ_PENDING,
  FEE_CANCEL_REQUESTED,
  FEE_CANCEL_PENDING,
  FEE_INVALIDATE_REQUESTED,
  FEE_INVALIDATE_PENDING,
  FEE_ERASE_IMMEDIATE_REQUESTED,
  FEE_ERASE_IMMEDIATE_PENDING
} CurrentJobStateType;

typedef struct {
	CurrentJobStateType			State;
	const Fee_BlockConfigType	*BlockConfigPtr;
	uint16						Length;
	uint16						Offset;
	Fls_AddressType				FlsBlockAddr;		// Flash source/Dest depending of operation
	Fls_AddressType				FlsBlockAdminAddr;	// Startadress of admin block
	uint8						*RamPtr;			// RAM source/Dest depending of operation
} CurrentJobType;

static CurrentJobType CurrentJob = {
		.State = FEE_IDLE
};


/*
 * Local functions
 */
#if (FEE_POLLING_MODE == STD_ON)
void PollFlsJobResult(void)
{
	MemIf_JobResultType jobResult;

	if (FlsAdmin.State == FEE_FLS_STATE_PENDING) {
		jobResult = Fls_GetJobResult();

		if (jobResult == MEMIF_JOB_OK) {
			FlsAdmin.ErrorStatus = E_OK;
			FlsAdmin.JobResult = jobResult;
			FlsAdmin.State = FEE_FLS_STATE_READY;
		} else if (jobResult != MEMIF_JOB_PENDING) {
			FlsAdmin.ErrorStatus = E_NOT_OK;
			FlsAdmin.JobResult = jobResult;
			FlsAdmin.State = FEE_FLS_STATE_READY;
		}
	}
}
#else
#define PollFlsJobResult(...)
#endif


void FinnishJob(void)
{
	FlsAdmin.State = FEE_FLS_STATE_IDLE;
	CurrentJob.State = FEE_IDLE;

	ModuleStatus = MEMIF_IDLE;
	JobResult = FlsAdmin.JobResult;

	if (FlsAdmin.ErrorStatus == E_OK) {
		if (Fee_Config.General.NvmJobEndCallbackNotificationCallback != NULL) {
			Fee_Config.General.NvmJobEndCallbackNotificationCallback();
		}
	} else {
		if (Fee_Config.General.NvmJobErrorCallbackNotificationCallback != NULL) {
			Fee_Config.General.NvmJobErrorCallbackNotificationCallback();
		}
	}
}


void ReadStartJob(void)
{
	if (FlsAdmin.State == FEE_FLS_STATE_IDLE) {
		FlsAdmin.State = FEE_FLS_STATE_PENDING;
		CurrentJob.State = FEE_READ_BLOCK_ADMIN_PENDING;
		// Start by reading the admin block
		if (Fls_Read(CurrentJob.FlsBlockAdminAddr, (uint8*)&FlsBlockAdmin, BLOCK_ADMIN_LEN) != E_OK) {
			FlsAdmin.State = FEE_FLS_STATE_READY;
			FlsAdmin.ErrorStatus = E_NOT_OK;
			FlsAdmin.JobResult = Fls_GetJobResult();
			FinnishJob();
		}
	}
}

void ReadCheckBlockAdminJob(void)
{
	if (FlsAdmin.ErrorStatus == E_OK) {
		if (memcmp(FlsBlockAdmin.Magic, MagicMaster, BLOCK_ADMIN_MAGIC_LEN) == 0) {
			if (FlsBlockAdmin.Status & BLOCK_ADMIN_STATUS_VALID) {
				if (FlsAdmin.State != FEE_FLS_STATE_PENDING) {
					FlsAdmin.State = FEE_FLS_STATE_PENDING;
					CurrentJob.State = FEE_READ_PENDING;
					// Read the actual data
					if (Fls_Read(CurrentJob.FlsBlockAddr + CurrentJob.Offset, CurrentJob.RamPtr, CurrentJob.Length) != E_OK) {
						FlsAdmin.State = FEE_FLS_STATE_READY;
						FlsAdmin.ErrorStatus = E_NOT_OK;
						FlsAdmin.JobResult = Fls_GetJobResult();
						FinnishJob();
					}
				}
			} else {
				FlsAdmin.ErrorStatus = E_NOT_OK;
				FlsAdmin.JobResult = MEMIF_BLOCK_INVALID;
				FinnishJob();
			}
		} else {
			FlsAdmin.ErrorStatus = E_NOT_OK;
			FlsAdmin.JobResult = MEMIF_BLOCK_INCONSISTENT;
			FinnishJob();
		}
	} else {
		FinnishJob();
	}
}


void WriteStartJob(void)
{
	if (FlsAdmin.State == FEE_FLS_STATE_IDLE) {
		FlsAdmin.State = FEE_FLS_STATE_PENDING;
		CurrentJob.State = FEE_WRITE_MAGIC_EREASE_PENDING;
		// Start by erasing the magic
		if (Fls_Erase(CurrentJob.FlsBlockAdminAddr + BLOCK_ADMIN_MAGIC_POS, BLOCK_ADMIN_MAGIC_LEN) != E_OK) {
			FlsAdmin.State = FEE_FLS_STATE_READY;
			FlsAdmin.ErrorStatus = E_NOT_OK;
			FlsAdmin.JobResult = Fls_GetJobResult();
			FinnishJob();
		}
	}
}


void WriteCheckMagicEraseJob(void)
{
	if (FlsAdmin.ErrorStatus == E_OK) {
		if (FlsAdmin.State != FEE_FLS_STATE_PENDING) {
			FlsAdmin.State = FEE_FLS_STATE_PENDING;
			CurrentJob.State = FEE_WRITE_EREASE_PENDING;
			// Erase the rest of the block
			if (Fls_Erase(CurrentJob.FlsBlockAddr, CurrentJob.BlockConfigPtr->BlockSize + BLOCK_ADMIN_LEN - BLOCK_ADMIN_MAGIC_LEN) != E_OK) {
				FlsAdmin.State = FEE_FLS_STATE_READY;
				FlsAdmin.ErrorStatus = E_NOT_OK;
				FlsAdmin.JobResult = Fls_GetJobResult();
				FinnishJob();
			}
		}
	} else {
		FinnishJob();
	}
}


void WriteCheckEraseJob(void)
{
	if (FlsAdmin.ErrorStatus == E_OK) {
		if (FlsAdmin.State != FEE_FLS_STATE_PENDING) {
			FlsAdmin.State = FEE_FLS_STATE_PENDING;
			CurrentJob.State = FEE_WRITE_PENDING;
			// Write the actual data
			if (Fls_Write(CurrentJob.FlsBlockAddr, CurrentJob.RamPtr, CurrentJob.Length) != E_OK) {
				FlsAdmin.State = FEE_FLS_STATE_READY;
				FlsAdmin.ErrorStatus = E_NOT_OK;
				FlsAdmin.JobResult = Fls_GetJobResult();
				FinnishJob();
			}
		}
	} else {
		FinnishJob();
	}
}


void WriteCheckWriteJob(void)
{
	if (FlsAdmin.ErrorStatus == E_OK) {
		if (FlsAdmin.State != FEE_FLS_STATE_PENDING) {
			FlsAdmin.State = FEE_FLS_STATE_PENDING;
			CurrentJob.State = FEE_WRITE_BLOCK_ADMIN_PENDING;
			// Write the block admin (mark it as consistent, valid and not empty)
			if (Fls_Write(CurrentJob.FlsBlockAdminAddr, (uint8*)&FlsBlockAdminMaster, BLOCK_ADMIN_LEN) != E_OK) {
				FlsAdmin.State = FEE_FLS_STATE_READY;
				FlsAdmin.ErrorStatus = E_NOT_OK;
				FlsAdmin.JobResult = Fls_GetJobResult();
				FinnishJob();
			}
		}
	} else {
		FinnishJob();
	}
}


void InvalidateStartJob(void)
{
	static const uint8 zero = 0;

	if (FlsAdmin.State == FEE_FLS_STATE_IDLE) {
		FlsAdmin.State = FEE_FLS_STATE_PENDING;
		CurrentJob.State = FEE_INVALIDATE_PENDING;
		// Write a zero to the Validate flag
		if (Fls_Write(CurrentJob.FlsBlockAdminAddr + BLOCK_ADMIN_VALIDATE_POS, &zero, 1) != E_OK) {
			FlsAdmin.State = FEE_FLS_STATE_READY;
			FlsAdmin.ErrorStatus = E_NOT_OK;
			FlsAdmin.JobResult = Fls_GetJobResult();
		}
	}
}


/***************************************
 *    External accessible functions    *
 ***************************************/
/*
 * Procedure:	Fee_Init
 * Reentrant:	No
 */
void Fee_Init(void)
{
	// Reporting information
	ModuleStatus = MEMIF_IDLE;
	JobResult = MEMIF_JOB_OK;

	// State of device
	CurrentJob.State = FEE_IDLE;
	FlsAdmin.State = FEE_FLS_STATE_IDLE;
	FlsAdmin.ErrorStatus = E_OK;
	FlsAdmin.JobResult = MEMIF_JOB_OK;
}


/*
 * Procedure:	Fee_SetMode
 * Reentrant:	No
 */
void Fee_SetMode(MemIf_ModeType mode)
{
#if ( FLS_SET_MODE_API == STD_ON )
	Fls_SetMode(mode);
#else
	DET_REPORTERROR(MODULE_ID_FEE, 0, FEE_SET_MODE_ID, FEE_E_NOT_SUPPORTED);
#endif
}

/*
 * Procedure:	Fee_Read
 * Reentrant:	No
 */
Std_ReturnType Fee_Read(uint16 blockNumber, uint16 blockOffset, uint8* dataBufferPtr, uint16 length)
{
	uint16 blockIndex;

	VALIDATE_RV(ModuleStatus != MEMIF_UNINIT, FEE_READ_ID, FEE_E_UNINIT, E_NOT_OK);
	VALIDATE_RV(ModuleStatus == MEMIF_IDLE, FEE_READ_ID, FEE_E_BUSY, E_NOT_OK);

	VALIDATE_RV(blockNumber >= (1 << NVM_DATASET_SELECTION_BITS), FEE_READ_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);
	blockIndex = GET_BLOCK_INDEX_FROM_BLOCK_NUMBER(blockNumber);
	VALIDATE_RV(blockIndex < FEE_NUM_OF_BLOCKS, FEE_READ_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);
	VALIDATE_RV(dataBufferPtr != NULL, FEE_READ_ID, FEE_E_INVALID_DATA_PTR, E_NOT_OK);
	VALIDATE_RV(blockOffset < Fee_Config.BlockConfig[blockIndex].BlockSize, FEE_READ_ID, FEE_E_INVALID_BLOCK_OFS, E_NOT_OK);
	VALIDATE_RV(blockOffset + length <= Fee_Config.BlockConfig[blockIndex].BlockSize, FEE_READ_ID, FEE_E_INVALID_BLOCK_LEN, E_NOT_OK);


	/** @req FEE022 */
	ModuleStatus = MEMIF_BUSY;
	JobResult = MEMIF_JOB_PENDING;

	CurrentJob.BlockConfigPtr = &Fee_Config.BlockConfig[blockIndex];
	CurrentJob.Length = length;
	CurrentJob.Offset = blockOffset;
	CurrentJob.FlsBlockAddr = (CurrentJob.BlockConfigPtr->PhysBlockBaseNumber - 1) * FEE_VIRTUAL_PAGE_SIZE + GET_DATASET_FROM_BLOCK_NUMBER(blockNumber) * CurrentJob.BlockConfigPtr->BlockSize; 		/** @req FEE021 */
	CurrentJob.FlsBlockAdminAddr = CurrentJob.FlsBlockAddr + CurrentJob.BlockConfigPtr->BlockSize;
	CurrentJob.RamPtr = dataBufferPtr;
	CurrentJob.State = FEE_READ_REQUESTED;

	return E_OK;
}


/*
 * Procedure:	Fee_Write
 * Reentrant:	No
 */
Std_ReturnType Fee_Write(uint16 blockNumber, uint8* dataBufferPtr)
{
	uint16 blockIndex;

	VALIDATE_RV(ModuleStatus != MEMIF_UNINIT, FEE_WRITE_ID, FEE_E_UNINIT, E_NOT_OK);
	VALIDATE_RV(ModuleStatus == MEMIF_IDLE, FEE_WRITE_ID, FEE_E_BUSY, E_NOT_OK);

	VALIDATE_RV(blockNumber >= (1 << NVM_DATASET_SELECTION_BITS), FEE_WRITE_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);
	blockIndex = GET_BLOCK_INDEX_FROM_BLOCK_NUMBER(blockNumber);
	VALIDATE_RV(blockIndex < FEE_NUM_OF_BLOCKS, FEE_WRITE_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);
	VALIDATE_RV(dataBufferPtr != NULL, FEE_WRITE_ID, FEE_E_INVALID_DATA_PTR, E_NOT_OK);


	/** @req FEE025 */
	ModuleStatus = MEMIF_BUSY;
	JobResult = MEMIF_JOB_PENDING;

	CurrentJob.BlockConfigPtr = &Fee_Config.BlockConfig[blockIndex];
	CurrentJob.Length = CurrentJob.BlockConfigPtr->BlockSize;
	CurrentJob.FlsBlockAddr = (CurrentJob.BlockConfigPtr->PhysBlockBaseNumber - 1) * FEE_VIRTUAL_PAGE_SIZE + GET_DATASET_FROM_BLOCK_NUMBER(blockNumber) * CurrentJob.BlockConfigPtr->BlockSize; 		/** @req FEE024 */
	CurrentJob.FlsBlockAdminAddr = CurrentJob.FlsBlockAddr + CurrentJob.BlockConfigPtr->BlockSize;
	CurrentJob.RamPtr = dataBufferPtr;
	CurrentJob.State = FEE_WRITE_REQUESTED;

	return E_OK;
}


/*
 * Procedure:	Fee_Cancel
 * Reentrant:	No
 */
void Fee_Cancel(void)
{
	DET_REPORTERROR(MODULE_ID_FEE, 0, FEE_CANCEL_ID, FEE_E_NOT_IMPLEMENTED_YET);
}


/*
 * Procedure:	Fee_GetStatus
 * Reentrant:	No
 */
MemIf_StatusType Fee_GetStatus(void)
{
	return ModuleStatus;
}


/*
 * Procedure:	Fee_GetJobResult
 * Reentrant:	No
 */
MemIf_JobResultType Fee_GetJobResult(void)
{
	return JobResult;
}


/*
 * Procedure:	Fee_InvalidateBlock
 * Reentrant:	No
 */
Std_ReturnType Fee_InvalidateBlock(uint16 blockNumber)
{
	uint16 blockIndex;

	VALIDATE_RV(ModuleStatus != MEMIF_UNINIT, FEE_INVALIDATE_BLOCK_ID, FEE_E_UNINIT, E_NOT_OK);
	VALIDATE_RV(ModuleStatus == MEMIF_IDLE, FEE_INVALIDATE_BLOCK_ID, FEE_E_BUSY, E_NOT_OK);

	VALIDATE_RV(blockNumber >= (1 << NVM_DATASET_SELECTION_BITS), FEE_INVALIDATE_BLOCK_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);
	blockIndex = GET_BLOCK_INDEX_FROM_BLOCK_NUMBER(blockNumber);
	VALIDATE_RV(blockIndex < FEE_NUM_OF_BLOCKS, FEE_INVALIDATE_BLOCK_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);


	ModuleStatus = MEMIF_BUSY;
	JobResult = MEMIF_JOB_PENDING;

	CurrentJob.BlockConfigPtr = &Fee_Config.BlockConfig[blockIndex];
	CurrentJob.Length = 0;
	CurrentJob.FlsBlockAddr = (CurrentJob.BlockConfigPtr->PhysBlockBaseNumber - 1) * FEE_VIRTUAL_PAGE_SIZE + GET_DATASET_FROM_BLOCK_NUMBER(blockNumber) * CurrentJob.BlockConfigPtr->BlockSize; 		/** @req FEE024 */
	CurrentJob.FlsBlockAdminAddr = CurrentJob.FlsBlockAddr + CurrentJob.BlockConfigPtr->BlockSize;
	CurrentJob.RamPtr = NULL;
	CurrentJob.State = FEE_INVALIDATE_REQUESTED;

	return E_OK;
}


/*
 * Procedure:	Fee_EraseImmediateBlock
 * Reentrant:	No
 */
Std_ReturnType Fee_EraseImmediateBlock(uint16 blockNumber)
{
	DET_REPORTERROR(MODULE_ID_FEE, 0, FEE_ERASE_IMMEDIATE_ID, FEE_E_NOT_IMPLEMENTED_YET);

	return E_NOT_OK;
}


/***************************************
 *         Scheduled functions         *
 ***************************************/
/*
 * Procedure:	Fee_MainFunction
 * Reentrant:	No
 */
void Fee_MainFunction(void)
{
	switch (CurrentJob.State) {
	case FEE_UNINITIALIZED:
	case FEE_IDLE:
		break;

	// Read states
	case FEE_READ_REQUESTED:
		ReadStartJob();
		break;

	case FEE_READ_BLOCK_ADMIN_PENDING:
		PollFlsJobResult();
		if (FLASH_READY) {
			ReadCheckBlockAdminJob();
		}
		break;

	case FEE_READ_PENDING:
		PollFlsJobResult();
		if (FLASH_READY) {
			FinnishJob();
		}
		break;

	// Write states
	case FEE_WRITE_REQUESTED:
		WriteStartJob();
		break;

	case FEE_WRITE_MAGIC_EREASE_PENDING:
		PollFlsJobResult();
		if (FLASH_READY) {
			WriteCheckMagicEraseJob();
		}
		break;

	case FEE_WRITE_EREASE_PENDING:
		PollFlsJobResult();
		if (FLASH_READY) {
			WriteCheckEraseJob();
		}
		break;

	case FEE_WRITE_PENDING:
		PollFlsJobResult();
		if (FLASH_READY) {
			WriteCheckWriteJob();
		}
		break;

	case FEE_WRITE_BLOCK_ADMIN_PENDING:
		PollFlsJobResult();
		if (FLASH_READY) {
			FinnishJob();
		}
		break;

	// Invalidate states
	case FEE_INVALIDATE_REQUESTED:
		InvalidateStartJob();
		break;

	case FEE_INVALIDATE_PENDING:
		PollFlsJobResult();
		if (FLASH_READY) {
			FinnishJob();
		}
		break;

	// Other
	default:
		break;
	}
}


/***************************************
 *  Call-back notifications functions  *
 ***************************************/
#if (FEE_POLLING_MODE == STD_OFF)
/*
 * Procedure:	Fee_JobEndNotification
 * Reentrant:	No
 */
void Fee_JobEndNotification(void)
{
	FlsAdmin.State = FEE_FLS_STATE_READY;
	FlsAdmin.ErrorStatus = E_OK;
	FlsAdmin.JobResult = Fls_GetJobResult();
}


/*
 * Procedure:	Fee_JobErrorNotification
 * Reentrant:	No
 */
void Fee_JobErrorNotification(void)
{
	FlsAdmin.State = FEE_FLS_STATE_READY;
	FlsAdmin.ErrorStatus = E_NOT_OK;
	FlsAdmin.JobResult = Fls_GetJobResult();
}
#endif


