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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=3.1.5 */


/*
 * Author: pete
 *
 * Part of Release:
 *   3.1.5
 *
 * Description:
 *   Implements the Fee module
 *
 * Support:
 *   General                  Have Support
 *   -------------------------------------------
 *   FEE_DEV_ERROR_DETECT            Y
 *   FeeIndex    					 N (always 0)
 *   FEE_NVM_JOB_END_NOTIFICATION    Y (Under ArcCore FEE_USE_JOB_NOTIFICATIONS is used)
 *   FEE_NVM_JOB_ERROR_NOTIFICATION  Y (Under ArcCore FEE_USE_JOB_NOTIFICATIONS is used)
 *   FEE_POLLING_MODE    			 Y
 *   FEE_VERSION_INFO_API            Y
 *   FEE_VIRTUAL_PAGE_SIZE           Y
 *
 *   Block                    Have Support
 *   -------------------------------------------
 *   FeeBlockNumber           		Y
 *   FeeBlockSize               	Y
 *   FeeImmediateData               N
 *   FeeNumberOfWriteCycles         N
 *   FeeDeviceIndex         		N
 *
 *   Published Information       Have Support
 *   -------------------------------------------
 *   FEE_BLOCK_OVERHEAD           	N
 *   FEE_MAXIMUM_BLOCKING_TIME		N
 *   FEE_PAGE_OVERHEAD				N
 */


//lint -emacro(904,VALIDATE_RV,VALIDATE_NO_RV) //904 PC-Lint exception to MISRA 14.7 (validate macros).

// Exception made as a result of that NVM_DATASET_SELECTION_BITS can be zero
//lint -emacro(835, MIN_BLOCKNR) // 835 PC-lint: A zero has been given as right argument to operator '<<' or '>>'
//lint -emacro(835, GET_BLOCK_INDEX_FROM_BLOCK_NUMBER) // 835 PC-lint: A zero has been given as right argument to operator '<<' or '>>'
//lint -emacro(835, GET_DATASET_FROM_BLOCK_NUMBER) // 835 PC-lint: A zero has been given as right argument to operator '<<' or '>>'
//lint -emacro(778, GET_DATASET_FROM_BLOCK_NUMBER) // 778 PC-lint: Constant expression evaluates to 0 in operation '-'
//lint -emacro(845, GET_DATASET_FROM_BLOCK_NUMBER) // 845 PC-lint: The right argument to operator '&' is certain to be 0
//lint -emacro(835, BLOCK_INDEX_AND_SET_TO_BLOCKNR) // 835 PC-lint: A zero has been given as right argument to operator '<<' or '>>'

#include <string.h>
#include "Fee.h"
#include "Fee_Cbk.h"
#include "Fee_Memory_Cfg.h"
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

/*
 *  Validation macros
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

#define MIN_BLOCKNR		((uint16)((uint16)1 << NVM_DATASET_SELECTION_BITS))

#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_RV(_exp,_api,_err,_rv )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_module,_instance,_api,_err)
#endif


/*
 * Block numbering recalculation macros
 */
#define GET_DATASET_FROM_BLOCK_NUMBER(_blocknr)	((_blocknr) & ((uint16)((uint16)1u << NVM_DATASET_SELECTION_BITS) - 1u))

/*
 * Page alignment macros
 */
#define PAGE_ALIGN(_size)	((uint16)((((_size) + FEE_VIRTUAL_PAGE_SIZE - 1) / FEE_VIRTUAL_PAGE_SIZE) * FEE_VIRTUAL_PAGE_SIZE))

/*
 * Bank properties list
 */
#define NUM_OF_BANKS	2
typedef struct {
	Fls_AddressType		Start;
	Fls_LengthType		End;
} BankPropType;

static const BankPropType BankProp[NUM_OF_BANKS] = {
	{
		.Start = FEE_BANK1_OFFSET,
		.End = FEE_BANK1_OFFSET + FEE_BANK1_LENGTH
	},
	{
		.Start = FEE_BANK2_OFFSET,
		.End = FEE_BANK2_OFFSET + FEE_BANK2_LENGTH
	},
};



/*
 * Macros and variables for flash bank administration
 */
#define BANK_STATUS_OLD		0x00
#define BANK_STATUS_NEW		0xFF
typedef uint8 FlsBankStatusType;

#define BANK_CTRL_PAGE_SIZE		PAGE_ALIGN(sizeof(FlsBankStatusType))

typedef union {
	FlsBankStatusType	BankStatus;
	uint8				Data[BANK_CTRL_PAGE_SIZE];
} FlsBankCtrlPageType;


/*
 * Macros and variables for flash block administration in flash
 */
#define BLOCK_STATUS_INUSE			0x00
#define BLOCK_STATUS_INVALIDATED	0x02
#define BLOCK_STATUS_EMPTY			0xFF
typedef uint8 BlockStatusType;

typedef struct {
	BlockStatusType		Status;
	uint16				BlockNo;
	Fls_AddressType		BlockDataAddress;
	uint16				BlockDataLength;
} FlsBlockCtrlDataType;

#define BLOCK_CTRL_DATA_PAGE_SIZE		PAGE_ALIGN(sizeof(FlsBlockCtrlDataType))

typedef union {
	FlsBlockCtrlDataType	Data;
	uint8					Byte[BLOCK_CTRL_DATA_PAGE_SIZE];
} FlsBlockCtrlDataPageType;


#define BLOCK_MAGIC_LEN		4
static const uint8 BlockMagicMaster[BLOCK_MAGIC_LEN] = { 0xeb, 0xba, 0xba, 0xbe };
#define BLOCK_CTRL_MAGIC_PAGE_SIZE	PAGE_ALIGN(BLOCK_MAGIC_LEN)


typedef union {
	uint8		Magic[BLOCK_MAGIC_LEN];
	uint8		Byte[BLOCK_CTRL_MAGIC_PAGE_SIZE];
} FlsBlockCtrlMagicPageType;

typedef struct {
	FlsBlockCtrlDataPageType	DataPage;
	FlsBlockCtrlMagicPageType	MagicPage;
} FlsBlockControlType;

#define BLOCK_CTRL_PAGE_SIZE	PAGE_ALIGN(sizeof(FlsBlockControlType))

#define BLOCK_CTRL_DATA_POS_OFFSET		(/*lint --e(835)*/0)		// Inform PC-Lint that I want the constant to be zero
#define BLOCK_CTRL_MAGIC_POS_OFFSET		BLOCK_CTRL_DATA_PAGE_SIZE

typedef union {
	FlsBlockControlType	BlockCtrl;
	FlsBankCtrlPageType BankCtrl;
	uint8				Byte[BLOCK_CTRL_PAGE_SIZE];
} ReadWriteBufferType;

static ReadWriteBufferType RWBuffer;

#define RWBUFFER_SIZE	sizeof(ReadWriteBufferType)


/*
 * Variables for flash administration
 */
typedef struct {
	BlockStatusType		Status;
	Fls_AddressType		BlockAdminAddress;
	Fls_AddressType		BlockDataAddress;
} AdminFlsBlockType;

typedef struct {
	uint8				BankNumber;
	Fls_AddressType		NewBlockAdminAddress;
	Fls_AddressType		NewBlockDataAddress;
	FlsBankStatusType	BankStatus[NUM_OF_BANKS];
	AdminFlsBlockType	BlockDescrTbl[FEE_NUM_OF_BLOCKS][FEE_MAX_NUM_SETS];
} AdminFlsType;

static AdminFlsType AdminFls;


/*
 * Variables for quick reporting of status and job result
 */
static MemIf_StatusType ModuleStatus = MEMIF_UNINIT;
static MemIf_JobResultType JobResult = MEMIF_JOB_OK;

/*
 * Variables for the current job
 */
typedef enum {
  FEE_UNINITIALIZED = 0,
  FEE_STARTUP_REQUESTED,
  FEE_STARTUP_READ_BANK1_STATUS,
  FEE_STARTUP_READ_BANK2_STATUS_REQUESTED,
  FEE_STARTUP_READ_BANK2_STATUS,
  FEE_STARTUP_READ_BLOCK_ADMIN_REQUESTED,
  FEE_STARTUP_READ_BLOCK_ADMIN,

  FEE_IDLE,

  FEE_WRITE_REQUESTED,
  FEE_WRITE_MARK_BANK_OLD,
  FEE_WRITE_HEADER_REQUESTED,
  FEE_WRITE_HEADER,
  FEE_WRITE_DATA_REQUESTED,
  FEE_WRITE_DATA,
  FEE_WRITE_MAGIC_REQUESTED,
  FEE_WRITE_MAGIC,

  FEE_READ_REQUESTED,
  FEE_READ,

  FEE_INVALIDATE_REQUESTED,
  FEE_INVALIDATE_MARK_BANK_OLD,
  FEE_WRITE_INVALIDATE_HEADER_REQUESTED,
  FEE_WRITE_INVALIDATE_HEADER,

  FEE_GARBAGE_COLLECT_REQUESTED,
  FEE_GARBAGE_COLLECT_HEADER_WRITE,
  FEE_GARBAGE_COLLECT_DATA_READ_REQUESTED,
  FEE_GARBAGE_COLLECT_DATA_READ,
  FEE_GARBAGE_COLLECT_DATA_WRITE_REQUESTED,
  FEE_GARBAGE_COLLECT_DATA_WRITE,
  FEE_GARBAGE_COLLECT_MAGIC_WRITE_REQUESTED,
  FEE_GARBAGE_COLLECT_MAGIC_WRITE,
  FEE_GARBAGE_COLLECT_ERASE
} CurrentJobStateType;

typedef struct {
	CurrentJobStateType			State;
	uint16						InStateCounter;
	uint16						BlockNumber;
	uint16						Length;
	const Fee_BlockConfigType	*BlockConfigPtr;
	AdminFlsBlockType			*AdminFlsBlockPtr;
	union {
		struct {
			uint8				NrOfBanks;
			uint8				BankNumber;
			Fls_AddressType		BlockAdminAddress;
		}Startup;
		struct {
			uint16				Offset;
			uint8				*RamPtr;
		}Read;
		struct {
			uint8				*RamPtr;
			Fls_AddressType		WriteAdminAddress;
			Fls_AddressType		WriteDataAddress;
		}Write;
		struct {
			Fls_AddressType		WriteAdminAddress;
			Fls_AddressType		WriteDataAddress;
		}Invalidate;
		struct {
			uint8				BankNumber;
			Fls_AddressType		WriteAdminAddress;
			Fls_AddressType		WriteDataAddress;
			uint16				BytesLeft;
			uint16				DataOffset;
		}GarbageCollect;
	} Op;
} CurrentJobType;

static CurrentJobType CurrentJob = {
		.State = FEE_IDLE,
		.InStateCounter = 0
		//lint -e{785}		PC-Lint (785) - rest of structure members is initialized when used.
};

/*
 * Misc definitions
 */
#define STATE_COUNTER_MAX				0xffff
#define GARBAGE_COLLECTION_DELAY		10

/***************************************
 *           Local functions           *
 ***************************************/
uint16 GET_BLOCK_INDEX_FROM_BLOCK_NUMBER(uint16 blockNumber) {
	const Fee_BlockConfigType *FeeBlockCon;
	uint16 BlockIndex = FEE_NUM_OF_BLOCKS + 1; // An invalid block

	FeeBlockCon = Fee_Config.BlockConfig;
	for (uint16 i = 0; i < FEE_NUM_OF_BLOCKS; i++)
	{
		if (FeeBlockCon[i].BlockNumber == blockNumber)
		{
			BlockIndex = i;
			break;
		}
	}

	return BlockIndex;
}


#if (FEE_POLLING_MODE == STD_ON)
#define SetFlsJobBusy()			/* Nothing needs to be done here */

static boolean CheckFlsJobFinnished(void)
{
	MemIf_JobResultType flsJobResult;

	flsJobResult = Fls_GetJobResult();
	return (flsJobResult != MEMIF_JOB_PENDING);
}
#else
static boolean FlsJobReady = TRUE;

static void SetFlsJobBusy()
{
	FlsJobReady = FALSE;
}

static boolean CheckFlsJobFinnished(void)
{
	return (FlsJobReady);
}

#endif


static void FinnishStartup(void)
{
	CurrentJob.State = FEE_IDLE;
	ModuleStatus = MEMIF_IDLE;
	JobResult = MEMIF_JOB_OK;
}


static void AbortStartup(MemIf_JobResultType result)
{
	CurrentJob.State = FEE_IDLE;
	ModuleStatus = MEMIF_IDLE;
	JobResult = result;
}


static void FinnishJob(void)
{
	CurrentJob.State = FEE_IDLE;
	ModuleStatus = MEMIF_IDLE;
	JobResult = MEMIF_JOB_OK;

	if (Fee_Config.General.NvmJobEndCallbackNotificationCallback != NULL) {
		Fee_Config.General.NvmJobEndCallbackNotificationCallback();
	}
}


static void AbortJob(MemIf_JobResultType result)
{
	CurrentJob.State = FEE_IDLE;
	ModuleStatus = MEMIF_IDLE;
	JobResult = result;

	if (Fee_Config.General.NvmJobErrorCallbackNotificationCallback != NULL) {
		Fee_Config.General.NvmJobErrorCallbackNotificationCallback();
	}
}


/*
 * Start of bank status 1 read
 */
static void StartupStartJob(void)
{
	if (Fls_GetStatus() == MEMIF_IDLE) {
		CurrentJob.State = FEE_STARTUP_READ_BANK1_STATUS;
		/* Read bank status of bank 1 */
		// PC-Lint exception (MISRA 11.4) - Pointer to pointer conversion ok by AUTOSAR
		if (Fls_Read(BankProp[0].End - BANK_CTRL_PAGE_SIZE, /*lint -e(926)*/(uint8*)&AdminFls.BankStatus[0], sizeof(FlsBankStatusType)) == E_OK) {
			SetFlsJobBusy();
		} else {
			AbortStartup(Fls_GetJobResult());
		}
	}
}


/*
 *  Check job result of bank 1 status read, if ok request for bank 2 status read
 */
static void StartupReadBank1Status(void)
{
	if (CheckFlsJobFinnished()) {
		if (Fls_GetJobResult() == MEMIF_JOB_OK) {
			CurrentJob.State = FEE_STARTUP_READ_BANK2_STATUS_REQUESTED;
		} else {
			AbortStartup(Fls_GetJobResult());
		}
	}
}


/*
 * Start of bank status 2 read
 */
static void StartupReadBank2StatusRequested(void)
{
	if (Fls_GetStatus() == MEMIF_IDLE) {
		/* Read bank status of bank 2 */
		CurrentJob.State = FEE_STARTUP_READ_BANK2_STATUS;
		// PC-Lint exception (MISRA 11.4) - Pointer to pointer conversion ok by AUTOSAR
		if (Fls_Read(BankProp[1].End - BANK_CTRL_PAGE_SIZE, /*lint -e(926)*/(uint8*)&AdminFls.BankStatus[1], sizeof(FlsBankStatusType)) == E_OK) {
			SetFlsJobBusy();
		} else {
			AbortStartup(Fls_GetJobResult());
		}
	}
}


/*
 * Check job result of bank status 2 read - request for block status reading
 */
static void StartupReadBank2Status(void)
{
	MemIf_JobResultType jobResult;

	if (CheckFlsJobFinnished()) {
		jobResult = Fls_GetJobResult();
		if (jobResult == MEMIF_JOB_OK) {
			/* Select which bank to start with */
			if ((AdminFls.BankStatus[0] != BANK_STATUS_OLD) && (AdminFls.BankStatus[1] != BANK_STATUS_OLD)){
				/* None is marked as old, just start with one of them */
				CurrentJob.Op.Startup.BankNumber = 0;
				CurrentJob.Op.Startup.NrOfBanks = 2;
			} else if ((AdminFls.BankStatus[0] == BANK_STATUS_OLD) && (AdminFls.BankStatus[1] == BANK_STATUS_OLD) ) {
				/* Both banks are marked as old, this shall not be possible */
				DET_REPORTERROR(MODULE_ID_FEE, 0, FEE_STARTUP_ID, FEE_FLASH_CORRUPT);
				jobResult = MEMIF_JOB_FAILED;
			} else if (AdminFls.BankStatus[0] == BANK_STATUS_OLD) {
				CurrentJob.Op.Startup.BankNumber = 0;
				CurrentJob.Op.Startup.NrOfBanks = 2;
			} else {
				CurrentJob.Op.Startup.BankNumber = 1;
				CurrentJob.Op.Startup.NrOfBanks = 2;
			}
		}

		if (jobResult != MEMIF_JOB_OK) {
			AbortStartup(jobResult);
		} else {
			CurrentJob.Op.Startup.BlockAdminAddress = BankProp[CurrentJob.Op.Startup.BankNumber].End - (BLOCK_CTRL_PAGE_SIZE + BANK_CTRL_PAGE_SIZE);
			CurrentJob.State = FEE_STARTUP_READ_BLOCK_ADMIN_REQUESTED;
		}
	}
}

/*
 * Start of block admin read
 */
static void StartupReadBlockAdminRequested(void)
{
	if (Fls_GetStatus() == MEMIF_IDLE) {
		/* Start reading the banks */
		CurrentJob.State = FEE_STARTUP_READ_BLOCK_ADMIN;
		if (Fls_Read(CurrentJob.Op.Startup.BlockAdminAddress, RWBuffer.Byte, BLOCK_CTRL_PAGE_SIZE) == E_OK) {
			SetFlsJobBusy();
		} else {
			AbortStartup(Fls_GetJobResult());
		}
	}
}


/*
 * Check job result of block admin read, if all block processed finish
 * otherwise request for a new block admin read
 */
static void StartupReadBlockAdmin(void)
{
	if (CheckFlsJobFinnished()) {
		if (Fls_GetJobResult() == MEMIF_JOB_OK) {
			if (RWBuffer.BlockCtrl.DataPage.Data.Status == BLOCK_STATUS_EMPTY) {
				VALIDATE(CurrentJob.Op.Startup.NrOfBanks != 0, FEE_STARTUP_ID, FEE_FLASH_CORRUPT);
				CurrentJob.Op.Startup.NrOfBanks--;
				CurrentJob.Op.Startup.BankNumber = (CurrentJob.Op.Startup.BankNumber + 1) % 2;
				CurrentJob.Op.Startup.BlockAdminAddress = BankProp[CurrentJob.Op.Startup.BankNumber].End - (BLOCK_CTRL_PAGE_SIZE + BANK_CTRL_PAGE_SIZE);
			} else { /* Block not empty */
				if ((memcmp(RWBuffer.BlockCtrl.MagicPage.Magic, BlockMagicMaster, BLOCK_MAGIC_LEN) == 0) &&
						((RWBuffer.BlockCtrl.DataPage.Data.Status == BLOCK_STATUS_INUSE) || (RWBuffer.BlockCtrl.DataPage.Data.Status == BLOCK_STATUS_INVALIDATED))) {
					/* This is a valid admin block */
					uint16 blockIndex;
					uint16 dataSet;

					blockIndex = GET_BLOCK_INDEX_FROM_BLOCK_NUMBER(RWBuffer.BlockCtrl.DataPage.Data.BlockNo);
					dataSet = GET_DATASET_FROM_BLOCK_NUMBER(RWBuffer.BlockCtrl.DataPage.Data.BlockNo);

					if ((blockIndex < FEE_NUM_OF_BLOCKS) && (dataSet < FEE_MAX_NUM_SETS)) {
						AdminFls.BlockDescrTbl[blockIndex][dataSet].BlockAdminAddress = CurrentJob.Op.Startup.BlockAdminAddress;
						AdminFls.BlockDescrTbl[blockIndex][dataSet].BlockDataAddress = RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress;
						AdminFls.BlockDescrTbl[blockIndex][dataSet].Status = RWBuffer.BlockCtrl.DataPage.Data.Status;

						AdminFls.BankNumber = CurrentJob.Op.Startup.BankNumber;
						AdminFls.NewBlockDataAddress = RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress + RWBuffer.BlockCtrl.DataPage.Data.BlockDataLength;
						if (CurrentJob.Op.Startup.BlockAdminAddress <= AdminFls.NewBlockDataAddress) {
							/* This shall never happen */
							DET_REPORTERROR(MODULE_ID_FEE, 0, FEE_STARTUP_ID, FEE_FLASH_CORRUPT);
						}
					}
				}
				// TODO Check wrap around here
				CurrentJob.Op.Startup.BlockAdminAddress -= BLOCK_CTRL_PAGE_SIZE;
				AdminFls.NewBlockAdminAddress = CurrentJob.Op.Startup.BlockAdminAddress;
			}

			if (CurrentJob.Op.Startup.NrOfBanks == 0) {
				/* If current bank is marked as old we need to switch to a new bank */
				if (AdminFls.BankStatus[AdminFls.BankNumber] == BANK_STATUS_OLD) {
					AdminFls.BankNumber = (AdminFls.BankNumber + 1) % 2;
					AdminFls.NewBlockAdminAddress = BankProp[AdminFls.BankNumber].End - (BLOCK_CTRL_PAGE_SIZE + BANK_CTRL_PAGE_SIZE);
					AdminFls.NewBlockDataAddress = BankProp[AdminFls.BankNumber].Start;
				}
				/* We are done! */
				FinnishStartup();
			} else {
				CurrentJob.State = FEE_STARTUP_READ_BLOCK_ADMIN_REQUESTED;
			}


		} else { /* ErrorStatus not E_OK */
			AbortStartup(Fls_GetJobResult());
		}
	}
}


/*
 * Start of read block data
 */
static void ReadStartJob(void)
{
	if (Fls_GetStatus() == MEMIF_IDLE) {
		if (CurrentJob.AdminFlsBlockPtr->Status != BLOCK_STATUS_EMPTY) {
			if (CurrentJob.AdminFlsBlockPtr->Status != BLOCK_STATUS_INVALIDATED) {
				CurrentJob.State = FEE_READ;
				/* Read the actual data */
				if (Fls_Read(CurrentJob.AdminFlsBlockPtr->BlockDataAddress + CurrentJob.Op.Read.Offset, CurrentJob.Op.Read.RamPtr, CurrentJob.Length) == E_OK) {
					SetFlsJobBusy();
				} else {
					AbortJob(Fls_GetJobResult());
				}
			} else {
				/* Invalid */
				AbortJob(MEMIF_BLOCK_INVALID);
			}
		} else {
			/* Inconsistent */
			AbortJob(MEMIF_BLOCK_INCONSISTENT);
		}
	}
}

/*
 * Check job result of block data read
 */
static void Reading(void)
{
	if (CheckFlsJobFinnished()) {
		if (Fls_GetJobResult() == MEMIF_JOB_OK) {
			FinnishJob();
		} else {
			AbortJob(Fls_GetJobResult());
		}
	}
}


/*
 * Write bank header
 */
static void BankHeaderOldWrite(uint8 bank)
{
	/* Mark the bank as old */
	memset(RWBuffer.BankCtrl.Data, 0xff, BANK_CTRL_PAGE_SIZE);
	RWBuffer.BankCtrl.BankStatus = BANK_STATUS_OLD;
	if (Fls_Write(BankProp[bank].End - BANK_CTRL_PAGE_SIZE, RWBuffer.BankCtrl.Data, BANK_CTRL_PAGE_SIZE) == E_OK) {
		SetFlsJobBusy();
	} else {
		AbortJob(Fls_GetJobResult());
	}
}


/*
 * Write block header
 */
static void BlockHeaderDataWrite(void)
{
	/* Write the header excluding the magic */
	memset(RWBuffer.BlockCtrl.DataPage.Byte, 0xff, BLOCK_CTRL_DATA_PAGE_SIZE);
	RWBuffer.BlockCtrl.DataPage.Data.Status = BLOCK_STATUS_INUSE;
	RWBuffer.BlockCtrl.DataPage.Data.BlockNo = CurrentJob.BlockNumber;
	RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress = AdminFls.NewBlockDataAddress;
	RWBuffer.BlockCtrl.DataPage.Data.BlockDataLength = CurrentJob.Length;
	if (Fls_Write(CurrentJob.Op.Write.WriteAdminAddress + BLOCK_CTRL_DATA_POS_OFFSET, RWBuffer.Byte, BLOCK_CTRL_DATA_PAGE_SIZE) == E_OK) {
		SetFlsJobBusy();
		AdminFls.NewBlockDataAddress += CurrentJob.Length;
		AdminFls.NewBlockAdminAddress -= BLOCK_CTRL_PAGE_SIZE;
	} else {
		AbortJob(Fls_GetJobResult());
	}
}


/*
 * Check if bank switch needed:
 * - Yes, start mark current bank as old
 * - No, start of header write
 */
static void WriteStartJob(void)
{
	if (Fls_GetStatus() == MEMIF_IDLE) {
		if (AdminFls.NewBlockDataAddress + CurrentJob.BlockConfigPtr->BlockSize > AdminFls.NewBlockAdminAddress - BLOCK_CTRL_PAGE_SIZE) {
			/* Bank switch needed, mark current bank as "old" */
			CurrentJob.State = FEE_WRITE_MARK_BANK_OLD;
			BankHeaderOldWrite(AdminFls.BankNumber);
		} else {
			CurrentJob.Op.Write.WriteDataAddress = AdminFls.NewBlockDataAddress;
			CurrentJob.Op.Write.WriteAdminAddress = AdminFls.NewBlockAdminAddress;

			CurrentJob.State = FEE_WRITE_HEADER;
			BlockHeaderDataWrite();
		}
	}
}


/*
 * Check job result of mark bank as old, if ok request for header write
 */
static void WriteMarkBankOldState(void)
{
	if (CheckFlsJobFinnished()) {
		if (Fls_GetJobResult() == MEMIF_JOB_OK) {
			/* Mark for garbage collection */
			AdminFls.BankStatus[AdminFls.BankNumber] = BANK_STATUS_OLD;

			/* Change of bank */
			AdminFls.BankNumber ^= 0x1u;
			AdminFls.NewBlockDataAddress = BankProp[AdminFls.BankNumber].Start;
			AdminFls.NewBlockAdminAddress = BankProp[AdminFls.BankNumber].End - (BLOCK_CTRL_PAGE_SIZE + BANK_CTRL_PAGE_SIZE);

			CurrentJob.Op.Write.WriteDataAddress = AdminFls.NewBlockDataAddress;
			CurrentJob.Op.Write.WriteAdminAddress = AdminFls.NewBlockAdminAddress;

			CurrentJob.State = FEE_WRITE_HEADER_REQUESTED;
		} else {
			AbortJob(Fls_GetJobResult());
		}
	}
}


/*
 * Start of header write
 */
static void WriteHeaderRequested()
{
	if (Fls_GetStatus() == MEMIF_IDLE) {
		CurrentJob.State = FEE_WRITE_HEADER;
		BlockHeaderDataWrite();
	}
}


/*
 * Check job result of write header, if ok request for block data write
 */
static void WriteHeaderState(void)
{
	if (CheckFlsJobFinnished()) {
		if (Fls_GetJobResult() == MEMIF_JOB_OK) {
			CurrentJob.State = FEE_WRITE_DATA_REQUESTED;
		} else {
			AbortJob(Fls_GetJobResult());
		}
	}
}


/*
 * Start block data write
 */
static void WriteDataRequested(void)
{
	if (Fls_GetStatus() == MEMIF_IDLE) {
		CurrentJob.State = FEE_WRITE_DATA;
		/* Write the actual data */
		if (Fls_Write(CurrentJob.Op.Write.WriteDataAddress, CurrentJob.Op.Write.RamPtr, CurrentJob.Length) == E_OK) {
			SetFlsJobBusy();
		} else {
			AbortJob(Fls_GetJobResult());
		}
	}
}


/*
 * Check job result of data write - request for magic write
 */
static void WriteDataState(void)
{
	if (CheckFlsJobFinnished()) {
		if (Fls_GetJobResult() == MEMIF_JOB_OK) {
			CurrentJob.State = FEE_WRITE_MAGIC_REQUESTED;
		} else {
			AbortJob(Fls_GetJobResult());
		}
	}
}


/*
 * Start magic write
 */
static void WriteMagicRequested(void)
{
	if (Fls_GetStatus() == MEMIF_IDLE) {
		CurrentJob.State = FEE_WRITE_MAGIC;
		memset(RWBuffer.BlockCtrl.MagicPage.Byte, 0xff, BLOCK_CTRL_MAGIC_PAGE_SIZE);
		memcpy(RWBuffer.BlockCtrl.MagicPage.Magic, BlockMagicMaster, BLOCK_MAGIC_LEN);
		if (Fls_Write(CurrentJob.Op.Write.WriteAdminAddress + BLOCK_CTRL_MAGIC_POS_OFFSET, RWBuffer.BlockCtrl.MagicPage.Byte, BLOCK_CTRL_MAGIC_PAGE_SIZE) == E_OK) {
			SetFlsJobBusy();
		} else {
			AbortJob(Fls_GetJobResult());
		}
	}
}


/*
 * Check job result of write magic, if ok update the block admin table and finish
 */
static void WriteMagicState(void)
{
	if (CheckFlsJobFinnished()) {
		if (Fls_GetJobResult() == MEMIF_JOB_OK) {
			/* Update the block admin table */
			CurrentJob.AdminFlsBlockPtr->Status =  BLOCK_STATUS_INUSE;
			CurrentJob.AdminFlsBlockPtr->BlockAdminAddress = CurrentJob.Op.Write.WriteAdminAddress;
			CurrentJob.AdminFlsBlockPtr->BlockDataAddress = CurrentJob.Op.Write.WriteDataAddress;

			FinnishJob();
		} else {
			AbortJob(Fls_GetJobResult());
		}
	}
}


/*
 * Check if any bank is marked as old
 */
static void CheckIfGarbageCollectionNeeded(void)
{
	if ((AdminFls.BankStatus[0] == BANK_STATUS_OLD) || (AdminFls.BankStatus[1] == BANK_STATUS_OLD)) {
		ModuleStatus = MEMIF_BUSY_INTERNAL;
		JobResult = MEMIF_JOB_PENDING;

		CurrentJob.State = FEE_GARBAGE_COLLECT_REQUESTED;
	}
}


/*
 * Checks if any blocks needs to be moved if so start with writing a new header
 * or if no blocks needs to be moved request for bank erase.
 */
static void GarbageCollectStartJob(void)
{
	uint16 blockIndex;
	uint16 set;
	boolean found = FALSE;
	uint8 sourceBank;

	if (Fls_GetStatus() == MEMIF_IDLE) {
		if ((AdminFls.BankStatus[0] == BANK_STATUS_OLD) || (AdminFls.BankStatus[1] == BANK_STATUS_OLD)) {
			if (AdminFls.BankStatus[0] == BANK_STATUS_OLD) {
				sourceBank = 0;
			} else {
				sourceBank = 1;
			}

			for (blockIndex = 0; (blockIndex < FEE_NUM_OF_BLOCKS) && (!found); blockIndex++) {
				for (set = 0; (set < FEE_MAX_NUM_SETS) && (!found); set++) {
					if (AdminFls.BlockDescrTbl[blockIndex][set].Status != BLOCK_STATUS_EMPTY) {
						if ((AdminFls.BlockDescrTbl[blockIndex][set].BlockAdminAddress >= BankProp[sourceBank].Start) && (AdminFls.BlockDescrTbl[blockIndex][set].BlockAdminAddress < (BankProp[sourceBank].End))) {
							CurrentJob.AdminFlsBlockPtr = &AdminFls.BlockDescrTbl[blockIndex][set];
							CurrentJob.BlockConfigPtr = &Fee_Config.BlockConfig[blockIndex];
							CurrentJob.BlockNumber = Fee_Config.BlockConfig[blockIndex].BlockNumber;
							if (AdminFls.BlockDescrTbl[blockIndex][set].Status == BLOCK_STATUS_INVALIDATED) {
								CurrentJob.Length = 0;
							} else {
								CurrentJob.Length = PAGE_ALIGN(CurrentJob.BlockConfigPtr->BlockSize);
							}

							found = TRUE;
						}
					}
				}
			}

			if (found) {
				CurrentJob.Op.GarbageCollect.WriteDataAddress = AdminFls.NewBlockDataAddress;
				CurrentJob.Op.GarbageCollect.WriteAdminAddress = AdminFls.NewBlockAdminAddress;

				CurrentJob.State = FEE_GARBAGE_COLLECT_HEADER_WRITE;
				BlockHeaderDataWrite();
			} else {
				if (Fls_Erase(BankProp[sourceBank].Start, BankProp[sourceBank].End - BankProp[sourceBank].Start) == E_OK) {
					SetFlsJobBusy();
				} else {
					AbortJob(Fls_GetJobResult());
				}
				CurrentJob.Op.GarbageCollect.BankNumber = sourceBank;
				CurrentJob.State = FEE_GARBAGE_COLLECT_ERASE;
			}
		} else {
			CurrentJob.State = FEE_IDLE;
		}
	}
}


/*
 * Check job result of write header, if ok request for read block data
 */
static void GarbageCollectWriteHeader(void)
{
	if (CheckFlsJobFinnished()) {
		if (Fls_GetJobResult() == MEMIF_JOB_OK) {
			if (CurrentJob.AdminFlsBlockPtr->Status == BLOCK_STATUS_INUSE) {
				CurrentJob.Op.GarbageCollect.BytesLeft = PAGE_ALIGN(CurrentJob.BlockConfigPtr->BlockSize);
				CurrentJob.Op.GarbageCollect.DataOffset = 0;
				CurrentJob.State = FEE_GARBAGE_COLLECT_DATA_READ_REQUESTED;
			} else {
				/* Yes, we are finished */
				VALIDATE_NO_RV(CurrentJob.AdminFlsBlockPtr->Status == BLOCK_STATUS_INVALIDATED, FEE_GARBAGE_WRITE_HEADER_ID, FEE_UNEXPECTED_STATUS);
				CurrentJob.State = FEE_GARBAGE_COLLECT_MAGIC_WRITE_REQUESTED;
			}
		} else {
			AbortJob(Fls_GetJobResult());
		}
	}
}


/*
 * Start of read block data, if data length is more than buffer size
 * the reading is segmented.
 */
static void GarbageCollectReadDataRequested(void)
{
	if (Fls_GetStatus() == MEMIF_IDLE) {
		CurrentJob.State = FEE_GARBAGE_COLLECT_DATA_READ;
		if (CurrentJob.Op.GarbageCollect.BytesLeft <= RWBUFFER_SIZE) {
			CurrentJob.Length = CurrentJob.Op.GarbageCollect.BytesLeft;
		} else {
			CurrentJob.Length = RWBUFFER_SIZE;
		}
		if (Fls_Read(CurrentJob.AdminFlsBlockPtr->BlockDataAddress + CurrentJob.Op.GarbageCollect.DataOffset, RWBuffer.Byte, CurrentJob.Length) == E_OK) {
			SetFlsJobBusy();
		} else {
			AbortJob(Fls_GetJobResult());
		}
	}
}


/*
 * Check job result of read block data, if ok request for a data write
 */
static void GarbageCollectReadData(void)
{
	if (CheckFlsJobFinnished()) {
		if (Fls_GetJobResult() == MEMIF_JOB_OK) {
			CurrentJob.State = FEE_GARBAGE_COLLECT_DATA_WRITE_REQUESTED;
		} else {
			AbortJob(Fls_GetJobResult());
		}
	}
}


/*
 * Start of write block data
 */
static void GarbageCollectWriteDataRequested(void)
{
	if (Fls_GetStatus() == MEMIF_IDLE) {
		CurrentJob.State = FEE_GARBAGE_COLLECT_DATA_WRITE;
		/* Write the actual data */
		if (Fls_Write(CurrentJob.Op.GarbageCollect.WriteDataAddress + CurrentJob.Op.GarbageCollect.DataOffset, RWBuffer.Byte, CurrentJob.Length) == E_OK) {
			SetFlsJobBusy();
		} else {
			AbortJob(Fls_GetJobResult());
		}
	} else {
		AbortJob(Fls_GetJobResult());
	}
}


/*
 * Check job result of write data, if ok request for write magic or
 * next data read depending on if there are more block data to move.
 */
static void GarbageCollectWriteData(void)
{
	if (CheckFlsJobFinnished()) {
		if (Fls_GetJobResult() == MEMIF_JOB_OK) {
			if (CurrentJob.Op.GarbageCollect.BytesLeft <= RWBUFFER_SIZE) {
				/* Yes, we are finished */
				CurrentJob.State = FEE_GARBAGE_COLLECT_MAGIC_WRITE_REQUESTED;
			} else {
				/* More data to move */
				CurrentJob.Op.GarbageCollect.DataOffset += RWBUFFER_SIZE;
				CurrentJob.Op.GarbageCollect.BytesLeft -= RWBUFFER_SIZE;
				CurrentJob.State = FEE_GARBAGE_COLLECT_DATA_READ_REQUESTED;
			}
		} else {
			AbortJob(Fls_GetJobResult());
		}
	}
}


/*
 * Start write magic
 */
static void GarbageCollectWriteMagicRequested(void)
{
	if (Fls_GetStatus() == MEMIF_IDLE) {
		CurrentJob.State = FEE_GARBAGE_COLLECT_MAGIC_WRITE;
		memset(RWBuffer.BlockCtrl.MagicPage.Byte, 0xff, BLOCK_CTRL_MAGIC_PAGE_SIZE);
		memcpy(RWBuffer.BlockCtrl.MagicPage.Magic, BlockMagicMaster, BLOCK_MAGIC_LEN);
		if (Fls_Write(CurrentJob.Op.GarbageCollect.WriteAdminAddress + BLOCK_CTRL_MAGIC_POS_OFFSET, RWBuffer.BlockCtrl.MagicPage.Byte, BLOCK_CTRL_MAGIC_PAGE_SIZE) == E_OK) {
			SetFlsJobBusy();
		} else {
			AbortJob(Fls_GetJobResult());
		}
	}
}


/*
 * Check the job result of write magic, if ok update the admin table with the new position of data.
 */
static void GarbageCollectWriteMagic(void)
{
	if (CheckFlsJobFinnished()) {
		if (Fls_GetJobResult() == MEMIF_JOB_OK) {
			CurrentJob.AdminFlsBlockPtr->BlockAdminAddress = CurrentJob.Op.GarbageCollect.WriteAdminAddress;
			CurrentJob.AdminFlsBlockPtr->BlockDataAddress = CurrentJob.Op.GarbageCollect.WriteDataAddress;
			FinnishJob();
		} else {
			AbortJob(Fls_GetJobResult());
		}
	}
}


/*
 * Check the result of the erase job
 */
static void GarbageCollectErase(void)
{
	if (CheckFlsJobFinnished()) {
		if (Fls_GetJobResult() == MEMIF_JOB_OK) {
			AdminFls.BankStatus[CurrentJob.Op.GarbageCollect.BankNumber] = BANK_STATUS_NEW;
			FinnishJob();
		} else {
			AbortJob(Fls_GetJobResult());
		}
	}
}


/*
 * Write an "Invalidated" block header
 */
static void BlockHeaderInvalidWrite(void)
{
	// Write the header including the magic
	memset(RWBuffer.Byte, 0xff, BLOCK_CTRL_PAGE_SIZE);
	RWBuffer.BlockCtrl.DataPage.Data.Status = BLOCK_STATUS_INVALIDATED;
	RWBuffer.BlockCtrl.DataPage.Data.BlockNo = CurrentJob.BlockNumber;
	RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress = AdminFls.NewBlockDataAddress;
	RWBuffer.BlockCtrl.DataPage.Data.BlockDataLength = 0;
	memset(RWBuffer.BlockCtrl.MagicPage.Byte, 0xff, BLOCK_CTRL_MAGIC_PAGE_SIZE);
	memcpy(RWBuffer.BlockCtrl.MagicPage.Magic, BlockMagicMaster, BLOCK_MAGIC_LEN);

	if (Fls_Write(CurrentJob.Op.Invalidate.WriteAdminAddress + BLOCK_CTRL_DATA_POS_OFFSET, RWBuffer.Byte, BLOCK_CTRL_PAGE_SIZE) == E_OK) {
		SetFlsJobBusy();
		AdminFls.NewBlockDataAddress += CurrentJob.Length;
		AdminFls.NewBlockAdminAddress -= BLOCK_CTRL_PAGE_SIZE;
	} else {
		AbortJob(Fls_GetJobResult());
	}
}


/*
 * Check if bank switch is needed, if yes request for marking current bank as old,
 * if no request for writing a header with "Invalid" status set.
 */
static void InvalidateStartJob(void)
{
	if (Fls_GetStatus() == MEMIF_IDLE) {
		if (AdminFls.NewBlockDataAddress + CurrentJob.BlockConfigPtr->BlockSize > AdminFls.NewBlockAdminAddress - BLOCK_CTRL_PAGE_SIZE) {
			/* Bank switch needed, mark current bank as "old" */
			CurrentJob.State = FEE_INVALIDATE_MARK_BANK_OLD;
			BankHeaderOldWrite(AdminFls.BankNumber);
		} else {
			CurrentJob.Op.Invalidate.WriteDataAddress = AdminFls.NewBlockDataAddress;
			CurrentJob.Op.Invalidate.WriteAdminAddress = AdminFls.NewBlockAdminAddress;

			CurrentJob.State = FEE_WRITE_INVALIDATE_HEADER;
			BlockHeaderInvalidWrite();
		}
	}
}


/*
 * Check job result of mark bank old, if ok continue with request for writing
 * a header with "Invalid" status set.
 */
static void InvalidateMarkBankOld(void)
{
	if (CheckFlsJobFinnished()) {
		if (Fls_GetJobResult() == MEMIF_JOB_OK) {
			// Mark for garbage collection
			AdminFls.BankStatus[AdminFls.BankNumber] = BANK_STATUS_OLD;

			// Change of bank
			AdminFls.BankNumber ^= 0x1u;
			AdminFls.NewBlockDataAddress = BankProp[AdminFls.BankNumber].Start;
			AdminFls.NewBlockAdminAddress = BankProp[AdminFls.BankNumber].End - (BLOCK_CTRL_PAGE_SIZE + BANK_CTRL_PAGE_SIZE);

			CurrentJob.Op.Invalidate.WriteDataAddress = AdminFls.NewBlockDataAddress;
			CurrentJob.Op.Invalidate.WriteAdminAddress = AdminFls.NewBlockAdminAddress;

			CurrentJob.State = FEE_WRITE_INVALIDATE_HEADER_REQUESTED;
		} else {
			AbortJob(Fls_GetJobResult());
		}
	}
}

/*
 * Start the writing of the "Invalid" header.
 */
static void InvalidateWriteInvalidateHeaderRequested(void)
{
	if (Fls_GetStatus() == MEMIF_IDLE) {
		CurrentJob.State = FEE_WRITE_INVALIDATE_HEADER;
		BlockHeaderInvalidWrite();
	}
}


/*
 * Check the job result of "Invalid" header write, if ok update the block admin table
 */
static void InvalidateWriteInvalidateHeader(void)
{
	if (CheckFlsJobFinnished()) {
		if (Fls_GetJobResult() == MEMIF_JOB_OK) {
			// Update the block admin table
			CurrentJob.AdminFlsBlockPtr->Status =  BLOCK_STATUS_INVALIDATED;
			CurrentJob.AdminFlsBlockPtr->BlockAdminAddress = CurrentJob.Op.Invalidate.WriteAdminAddress;
			CurrentJob.AdminFlsBlockPtr->BlockDataAddress = CurrentJob.Op.Invalidate.WriteDataAddress;

			FinnishJob();
		} else {
			AbortJob(Fls_GetJobResult());
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
	uint16 i,j;

	/* Reporting information */
	ModuleStatus = MEMIF_BUSY_INTERNAL;
	JobResult = MEMIF_JOB_OK;

	/* State of device */
	CurrentJob.State = FEE_STARTUP_REQUESTED;
	CurrentJob.InStateCounter = 0;
#if (FEE_POLLING_MODE == STD_OFF)
	FlsJobReady = TRUE;
#endif

	AdminFls.BankNumber = 0;
	AdminFls.NewBlockDataAddress = BankProp[AdminFls.BankNumber].Start;
	AdminFls.NewBlockAdminAddress = BankProp[AdminFls.BankNumber].End - (BLOCK_CTRL_PAGE_SIZE + BANK_CTRL_PAGE_SIZE);

	for (i = 0; i < NUM_OF_BANKS; i++) {
		AdminFls.BankStatus[i] = BANK_STATUS_NEW;
	}

	for (i = 0; i < FEE_NUM_OF_BLOCKS; i++) {
		for (j = 0; j < FEE_MAX_NUM_SETS; j++) {
			AdminFls.BlockDescrTbl[i][j].Status = BLOCK_STATUS_EMPTY;
			AdminFls.BlockDescrTbl[i][j].BlockAdminAddress= 0;
			AdminFls.BlockDescrTbl[i][j].BlockDataAddress = 0;
		}
	}
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
	//lint --e{715}	PC-Lint (715) - variable "mode" not used in this case
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
	uint16 dataset;

	VALIDATE_RV(ModuleStatus != MEMIF_UNINIT, FEE_READ_ID, FEE_E_UNINIT, E_NOT_OK);
	VALIDATE_RV(ModuleStatus == MEMIF_IDLE, FEE_READ_ID, FEE_E_BUSY, E_NOT_OK);

	blockIndex = GET_BLOCK_INDEX_FROM_BLOCK_NUMBER(blockNumber);
	VALIDATE_RV(blockIndex < FEE_NUM_OF_BLOCKS, FEE_READ_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);
	VALIDATE_RV(dataBufferPtr != NULL, FEE_READ_ID, FEE_E_INVALID_DATA_PTR, E_NOT_OK);
	VALIDATE_RV(blockOffset < Fee_Config.BlockConfig[blockIndex].BlockSize, FEE_READ_ID, FEE_E_INVALID_BLOCK_OFS, E_NOT_OK);
	VALIDATE_RV(blockOffset + length <= Fee_Config.BlockConfig[blockIndex].BlockSize, FEE_READ_ID, FEE_E_INVALID_BLOCK_LEN, E_NOT_OK);

	dataset = GET_DATASET_FROM_BLOCK_NUMBER(blockNumber);
	VALIDATE_RV(dataset < FEE_MAX_NUM_SETS, FEE_READ_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);


	/** @req FEE022 */
	ModuleStatus = MEMIF_BUSY;
	JobResult = MEMIF_JOB_PENDING;

	CurrentJob.BlockNumber = blockNumber;
	CurrentJob.BlockConfigPtr = &Fee_Config.BlockConfig[blockIndex];
	CurrentJob.AdminFlsBlockPtr = &AdminFls.BlockDescrTbl[blockIndex][dataset];
	CurrentJob.Length = length;
	CurrentJob.Op.Read.Offset = blockOffset;
	CurrentJob.Op.Read.RamPtr = dataBufferPtr;
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
	uint16 dataset;

	VALIDATE_RV(ModuleStatus != MEMIF_UNINIT, FEE_WRITE_ID, FEE_E_UNINIT, E_NOT_OK);
	VALIDATE_RV(ModuleStatus == MEMIF_IDLE, FEE_WRITE_ID, FEE_E_BUSY, E_NOT_OK);

	blockIndex = GET_BLOCK_INDEX_FROM_BLOCK_NUMBER(blockNumber);
	VALIDATE_RV(blockIndex < FEE_NUM_OF_BLOCKS, FEE_WRITE_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);
	VALIDATE_RV(dataBufferPtr != NULL, FEE_WRITE_ID, FEE_E_INVALID_DATA_PTR, E_NOT_OK);

	dataset = GET_DATASET_FROM_BLOCK_NUMBER(blockNumber);
	VALIDATE_RV(dataset < FEE_MAX_NUM_SETS, FEE_WRITE_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);


	/** @req FEE025 */
	ModuleStatus = MEMIF_BUSY;
	JobResult = MEMIF_JOB_PENDING;

	CurrentJob.BlockConfigPtr = &Fee_Config.BlockConfig[blockIndex];
	CurrentJob.AdminFlsBlockPtr = &AdminFls.BlockDescrTbl[blockIndex][dataset];
	CurrentJob.BlockNumber = blockNumber;
	CurrentJob.Length = PAGE_ALIGN(CurrentJob.BlockConfigPtr->BlockSize);
	CurrentJob.Op.Write.RamPtr = dataBufferPtr;
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
	uint16 dataset;

	VALIDATE_RV(ModuleStatus != MEMIF_UNINIT, FEE_INVALIDATE_BLOCK_ID, FEE_E_UNINIT, E_NOT_OK);
	VALIDATE_RV(ModuleStatus == MEMIF_IDLE, FEE_INVALIDATE_BLOCK_ID, FEE_E_BUSY, E_NOT_OK);

	blockIndex = GET_BLOCK_INDEX_FROM_BLOCK_NUMBER(blockNumber);
	VALIDATE_RV(blockIndex < FEE_NUM_OF_BLOCKS, FEE_INVALIDATE_BLOCK_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);

	dataset = GET_DATASET_FROM_BLOCK_NUMBER(blockNumber);
	VALIDATE_RV(dataset < FEE_MAX_NUM_SETS, FEE_INVALIDATE_BLOCK_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);


	ModuleStatus = MEMIF_BUSY;
	JobResult = MEMIF_JOB_PENDING;

	CurrentJob.BlockConfigPtr = &Fee_Config.BlockConfig[blockIndex];
	CurrentJob.AdminFlsBlockPtr = &AdminFls.BlockDescrTbl[blockIndex][dataset];
	CurrentJob.BlockNumber = blockNumber;
	CurrentJob.State = FEE_INVALIDATE_REQUESTED;

	return E_OK;
}


/*
 * Procedure:	Fee_EraseImmediateBlock
 * Reentrant:	No
 */
Std_ReturnType Fee_EraseImmediateBlock(uint16 blockNumber)
{
	//lint --e{715}	PC-Lint (715) - function is not implemented and thus variable "blockNumber" is not used yet

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
	static CurrentJobStateType LastState = FEE_UNINITIALIZED;

	if (CurrentJob.State == LastState) {
		if (CurrentJob.InStateCounter < STATE_COUNTER_MAX) {
			CurrentJob.InStateCounter++;
		}
	} else {
		LastState = CurrentJob.State;
		CurrentJob.InStateCounter = 0;
	}

	switch (CurrentJob.State) {
	case FEE_UNINITIALIZED:
		break;

	case FEE_IDLE:
		if (CurrentJob.InStateCounter > GARBAGE_COLLECTION_DELAY) {
			CheckIfGarbageCollectionNeeded();
		}
		break;

		/*
		 * Startup states
		 */
	case FEE_STARTUP_REQUESTED:
		StartupStartJob();
		break;

	case FEE_STARTUP_READ_BANK1_STATUS:
		StartupReadBank1Status();
		break;

	case FEE_STARTUP_READ_BANK2_STATUS_REQUESTED:
		StartupReadBank2StatusRequested();
		break;

	case FEE_STARTUP_READ_BANK2_STATUS:
		StartupReadBank2Status();
		break;

	case FEE_STARTUP_READ_BLOCK_ADMIN_REQUESTED:
		StartupReadBlockAdminRequested();
		break;

	case FEE_STARTUP_READ_BLOCK_ADMIN:
		StartupReadBlockAdmin();
		break;

	/*
	 *  Read states
	 */
	case FEE_READ_REQUESTED:
		ReadStartJob();
		break;

	case FEE_READ:
		Reading();
		break;

	/*
	 * Write states
	 */
	case FEE_WRITE_REQUESTED:
		WriteStartJob();
		break;

	case FEE_WRITE_MARK_BANK_OLD:
		WriteMarkBankOldState();
		break;

	case FEE_WRITE_HEADER_REQUESTED:
		WriteHeaderRequested();
		break;

	case FEE_WRITE_HEADER:
		WriteHeaderState();
		break;

	case FEE_WRITE_DATA_REQUESTED:
		WriteDataRequested();
		break;

	case FEE_WRITE_DATA:
		WriteDataState();
		break;

	case FEE_WRITE_MAGIC_REQUESTED:
		WriteMagicRequested();
		break;

	case FEE_WRITE_MAGIC:
		WriteMagicState();
		break;

	/*
	 * Garbage collection states
	 */
	case FEE_GARBAGE_COLLECT_REQUESTED:
		GarbageCollectStartJob();
		break;

	case FEE_GARBAGE_COLLECT_HEADER_WRITE:
		GarbageCollectWriteHeader();
		break;

	case FEE_GARBAGE_COLLECT_DATA_READ_REQUESTED:
		GarbageCollectReadDataRequested();
		break;

	case FEE_GARBAGE_COLLECT_DATA_READ:
		GarbageCollectReadData();
		break;

	case FEE_GARBAGE_COLLECT_DATA_WRITE_REQUESTED:
		GarbageCollectWriteDataRequested();
		break;

	case FEE_GARBAGE_COLLECT_DATA_WRITE:
		GarbageCollectWriteData();
		break;

	case FEE_GARBAGE_COLLECT_MAGIC_WRITE_REQUESTED:
		GarbageCollectWriteMagicRequested();
		break;

	case FEE_GARBAGE_COLLECT_MAGIC_WRITE:
		GarbageCollectWriteMagic();
		break;

	case FEE_GARBAGE_COLLECT_ERASE:
		GarbageCollectErase();
		break;

	/*
	 * Invalidate states
	 */
	case FEE_INVALIDATE_REQUESTED:
		InvalidateStartJob();
		break;

	case FEE_INVALIDATE_MARK_BANK_OLD:
		InvalidateMarkBankOld();
		break;

	case FEE_WRITE_INVALIDATE_HEADER_REQUESTED:
		InvalidateWriteInvalidateHeaderRequested();
		break;

	case FEE_WRITE_INVALIDATE_HEADER:
		InvalidateWriteInvalidateHeader();
		break;


	/*
	 * Other
	 */
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
	FlsJobReady = TRUE;
}


/*
 * Procedure:	Fee_JobErrorNotification
 * Reentrant:	No
 */
void Fee_JobErrorNotification(void)
{
	FlsJobReady = TRUE;
}
#endif
