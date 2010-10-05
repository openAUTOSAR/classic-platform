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


#define GET_BLOCK_INDEX_FROM_BLOCK_NUMBER(blocknr)	((blocknr >> NVM_DATASET_SELECTION_BITS) - 1)
#define GET_DATASET_FROM_BLOCK_NUMBER(blocknr)	(blocknr & ((1 << NVM_DATASET_SELECTION_BITS) - 1))


// Variables for quick reporting of status and job result
static MemIf_StatusType ModuleStatus = MEMIF_UNINIT;
static MemIf_JobResultType JobResult = MEMIF_JOB_OK;

typedef enum {
  FEE_UNINITIALIZED = 0,
  FEE_IDLE,
  FEE_JOB_REQUESTED,
  FEE_JOB_PENDING,
} FeeStateType;

// Variables holding state of Fee.
static FeeStateType FeeState = FEE_UNINITIALIZED;

typedef enum {
	FEE_JOB_TYPE_NONE,
	FEE_JOB_TYPE_READ,
	FEE_JOB_TYPE_WRITE,
	FEE_JOB_TYPE_CANCEL,
	FEE_JOB_TYPE_INVALIDATE,
	FEE_JOB_TYPE_ERASE_IMMEDIATE_BLOCK
} CurrentJobTypeType;

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

typedef struct {
	boolean		Consistant;
} BlockAdminType;

static BlockAdminType BlockAdminList[FEE_NUM_OF_BLOCKS];

typedef struct {
	CurrentJobTypeType			Type;
	const Fee_BlockConfigType	*BlockConfigPtr;
	BlockAdminType				*BlockAdminPtr;
	uint16						Length;
	uint32						FlsAddr;		// Flash source/Dest depending of operation
	uint8						*RamPtr;		// RAM source/Dest depending of operation
} CurrentJobType;

static CurrentJobType CurrentJob = {
		.Type = FEE_JOB_TYPE_NONE,
};

void Fee_Init(void)
{
	// Reporting information
	ModuleStatus = MEMIF_IDLE;
	JobResult = MEMIF_JOB_OK;

	// State of device
	FeeState = FEE_IDLE;
	FlsAdmin.State = FEE_FLS_STATE_IDLE;
	FlsAdmin.ErrorStatus = E_OK;
	FlsAdmin.JobResult = MEMIF_JOB_OK;
}


void Fee_SetMode(MemIf_ModeType mode)
{
#if ( FLS_SET_MODE_API == STD_ON )
	Fls_SetMode(mode);
#else
	DET_REPORTERROR(MODULE_ID_FEE, 0, FEE_SET_MODE_ID, FEE_E_NOT_SUPPORTED);
#endif
}


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

	CurrentJob.Type = FEE_JOB_TYPE_READ;
	CurrentJob.BlockConfigPtr = &Fee_Config.BlockConfig[blockIndex];
	CurrentJob.BlockAdminPtr = &BlockAdminList[blockIndex];
	CurrentJob.Length = length;
	CurrentJob.FlsAddr = CurrentJob.BlockConfigPtr->PhysBaseAddress + GET_DATASET_FROM_BLOCK_NUMBER(blockNumber) * CurrentJob.BlockConfigPtr->BlockSize + blockOffset; 		/** @req FEE021 */
	CurrentJob.RamPtr = dataBufferPtr;

	FeeState = FEE_JOB_REQUESTED;

	return E_OK;
}


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

	CurrentJob.Type = FEE_JOB_TYPE_WRITE;
	CurrentJob.BlockConfigPtr = &Fee_Config.BlockConfig[blockIndex];
	CurrentJob.BlockAdminPtr = &BlockAdminList[blockIndex];
	CurrentJob.Length = CurrentJob.BlockConfigPtr->BlockSize;
	CurrentJob.FlsAddr = CurrentJob.BlockConfigPtr->PhysBaseAddress + GET_DATASET_FROM_BLOCK_NUMBER(blockNumber) * CurrentJob.BlockConfigPtr->BlockSize; 		/** @req FEE024 */
	CurrentJob.RamPtr = dataBufferPtr;

	FeeState = FEE_JOB_REQUESTED;

	return E_OK;
}


void Fee_Cancel(void)
{

}


MemIf_StatusType Fee_GetStatus(void)
{
	return ModuleStatus;
}


MemIf_JobResultType Fee_GetJobResult(void)
{
	return JobResult;
}


Std_ReturnType Fee_InvalidateBlock(uint16 blockNumber)
{
	VALIDATE_RV(blockNumber < FEE_NUM_OF_BLOCKS, FEE_INVALIDATE_BLOCK_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);

	ModuleStatus = MEMIF_BUSY;
	JobResult = MEMIF_JOB_PENDING;

	CurrentJob.Type = FEE_JOB_TYPE_INVALIDATE;

	FeeState = FEE_JOB_REQUESTED;

	return E_OK;
}


Std_ReturnType Fee_EraseImmediateBlock(uint16 blockNumber)
{
	VALIDATE_RV(blockNumber < FEE_NUM_OF_BLOCKS, FEE_ERASE_IMMEDIATE_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);
	VALIDATE_RV(Fee_Config.BlockConfig[blockNumber].ImmediateData, FEE_ERASE_IMMEDIATE_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);

	ModuleStatus = MEMIF_BUSY;
	JobResult = MEMIF_JOB_PENDING;
	CurrentJob.Type = FEE_JOB_TYPE_ERASE_IMMEDIATE_BLOCK;

	FeeState = FEE_JOB_REQUESTED;

	return E_OK;
}

void StartJob(void)
{
	if (FlsAdmin.State == FEE_FLS_STATE_IDLE) {
		switch (CurrentJob.Type) {
		case FEE_JOB_TYPE_READ:
			FlsAdmin.State = FEE_FLS_STATE_PENDING;
			FeeState = FEE_JOB_PENDING;
			if (Fls_Read(CurrentJob.FlsAddr, CurrentJob.RamPtr, CurrentJob.Length) != E_OK) {
				FlsAdmin.State = FEE_FLS_STATE_READY;
				FlsAdmin.ErrorStatus = E_NOT_OK;
				FlsAdmin.JobResult = Fls_GetJobResult();
			}
			break;

		case FEE_JOB_TYPE_WRITE:
			FlsAdmin.State = FEE_FLS_STATE_PENDING;
			FeeState = FEE_JOB_PENDING;
			if (Fls_Write(CurrentJob.FlsAddr, CurrentJob.RamPtr, CurrentJob.Length) != E_OK) {
				FlsAdmin.State = FEE_FLS_STATE_READY;
				FlsAdmin.ErrorStatus = E_NOT_OK;
				FlsAdmin.JobResult = Fls_GetJobResult();
			}
			break;

		case FEE_JOB_TYPE_CANCEL:
			break;

		case FEE_JOB_TYPE_INVALIDATE:
			break;

		case FEE_JOB_TYPE_ERASE_IMMEDIATE_BLOCK:
			break;

		default:
			break;
		}
	} // if FEE_FLSH_STATE_IDLE
}


#if (FEE_POLLING_MODE == STD_ON)
void PollFlsJobResult(void)
{
	MemIf_JobResultType jobResult;

	FlsAdmin.State = FEE_FLS_STATE_READY;
	jobResult = Fls_GetJobResult();

	if (jobResult == MEMIF_JOB_OK) {
		FlsAdmin.ErrorStatus = E_OK;
		FlsAdmin.JobResult = jobResult;
	} else if (jobResult != MEMIF_JOB_PENDING) {
		FlsAdmin.ErrorStatus = E_NOT_OK;
		FlsAdmin.JobResult = jobResult;
	}

}
#else
void Fee_JobEndNotification(void)
{
	FlsAdmin.State = FEE_FLS_STATE_READY;
	FlsAdmin.ErrorStatus = E_OK;
	FlsAdmin.JobResult = Fls_GetJobResult();
}

void Fee_JobErrorNotification(void)
{
	FlsAdmin.State = FEE_FLS_STATE_READY;
	FlsAdmin.ErrorStatus = E_NOT_OK;
	FlsAdmin.JobResult = Fls_GetJobResult();
}
#endif

void CheckFlsResult(void)
{
	FlsAdmin.State = FEE_FLS_STATE_IDLE;
	FeeState = FEE_IDLE;

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


void Fee_MainFunction(void)
{
	switch (FeeState) {
	case FEE_UNINITIALIZED:
	case FEE_IDLE:
		break;

	case FEE_JOB_REQUESTED:
		StartJob();
		break;

	case FEE_JOB_PENDING:
#if (FEE_POLLING_MODE == STD_ON)
		PollFlsJobResult();
#endif
		if (FlsAdmin.State == FEE_FLS_STATE_READY) {
			CheckFlsResult();
		}
		break;

	default:
		break;
	}
}

