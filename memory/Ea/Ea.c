/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

/** @req EA113 @req EA112 @req EA049 @req EA038 */
/** @req EA116 @req EA117 @req EA083 @req EA097 */

#include "assert.h"
#include "string.h"

#include "Ea.h"
#include "Ea_Cbk.h"
#include "Ea_Types.h"

#include "Cpu.h"
//#include "Mcu.h"

/** @req EA011 */
/** @req EA045 */
#if (STD_ON == EA_DEV_ERROR_DETECT)
#if defined(USE_DET)
#include "Det.h"
#endif
#endif



// States used by EA_MainFunction to control the internal state of the module.
typedef enum
{
	EA_PENDING_NONE,
	EA_PENDING_WRITE,
	EA_PENDING_READ,
	EA_PENDING_ERASE,
	EA_PENDING_ADMIN_WRITE

} Ea_JobStatus;


/*
		define EA module notification callback macro 	
*/
#define EA_JOB_END_NOTIFICATION()	\
	if (Ea_Global.EaGeneralPtr->EaNvmJobEndNotification != NULL_PTR){	\
		Ea_Global.EaGeneralPtr->EaNvmJobEndNotification();		\
	}

#define EA_JOB_ERROR_NOTIFICATION() \
  	if (Ea_Global.EaGeneralPtr->EaNvmJobErrorNotification != NULL_PTR) { \
    	Ea_Global.EaGeneralPtr->EaNvmJobErrorNotification(); \
 	}

/*
 * Defines for blockstates
 */
/** @req EA047 */
#define BLOCK_INCONSISTENT  0x23
#define BLOCK_CONSISTENT    0x45
#define BLOCK_INVALIDATED   0x67

/*
 * Definition of the Admin block type
 */
/** @req EA046 */
typedef struct {
	uint16 blockNum;
	uint8  blockState;
	uint8  check;  // Simple checksum
} Ea_AdminBlock;

/*
 * Define EA Module Global Type
 */
typedef struct {
	const Ea_BlockConfigType *EaBlockConfig;
	const Ea_GeneralType	*EaGeneralPtr;
	MemIf_StatusType    	ModuleStatus;
	MemIf_JobResultType 	JobResult;
	Ea_JobType    			JobType;
	uint16					CurrentBlock; // The block we are currentlty working on
	Eep_AddressType			EepAddress; // Start adress to read from
	uint16                  Length;    // The length to read i.e the blockSize
	Ea_JobStatus            JobStatus; // Internal state to be used by main function.
	void*					Address;   // The adress to put the result into
	uint16                  ReadLength; // The length of the block to read/write
	uint16                  Offset; // The offset in the block to read from.
}Ea_GlobalType;

/*
 * Define EA Module Global
 */
static Ea_GlobalType Ea_Global = {
	NULL_PTR,
	NULL_PTR,
	MEMIF_UNINIT,
	MEMIF_JOB_OK,
	EA_JOB_NONE,
};

static uint8 Ea_TempBuffer[EA_MAX_BLOCK_SIZE + sizeof(Ea_AdminBlock)];

/*
 * Function prototypes
 */
static uint16 EA_GET_BLOCK(uint16 BlockNumber);
static Eep_AddressType calculateEepAddress(uint16 BlockIndex);
static uint16 calculateBlockLength(uint16 BlockIndex);
static void handleLowerLayerRead(MemIf_JobResultType jobResult);
static void handleLowerLayerWrite(MemIf_JobResultType jobResult);
static void handleLowerLayerErase(MemIf_JobResultType jobResult);
static uint8 verifyChecksum(Ea_AdminBlock* block);
static void addChecksum(Ea_AdminBlock* block);

/* Local functions */
static Std_ReturnType Ea_ValidateInitialized(Ea_APIServiceIDType apiId)
{
	Std_ReturnType result = E_OK;
    if( Ea_Global.ModuleStatus == MEMIF_UNINIT )
    {
#if EA_DEV_ERROR_DETECT==STD_ON
    	Det_ReportError(EA_MODULE_ID,0, apiId, EA_E_NOT_INITIALIZED);
    	result = E_NOT_OK;
#endif
    }
    return result;
}

static Std_ReturnType Ea_ValidateBlock(uint16 blockNumber, Ea_APIServiceIDType apiId)
{
	Std_ReturnType result = E_OK;
    if( (blockNumber == 0) || (blockNumber == 0xffff) )
    {
#if EA_DEV_ERROR_DETECT==STD_ON
    	Det_ReportError(EA_MODULE_ID,0, apiId, EA_E_INVALID_BLOCK_NO);
    	result = E_NOT_OK;
#endif
    }
    return result;
}

static Std_ReturnType Ea_ValidateBlockIndex(uint16 blockIndex, Ea_APIServiceIDType apiId)
{
	Std_ReturnType result = E_OK;
    if( blockIndex >= EA_NUMBER_OF_BLOCKS )
    {
#if EA_DEV_ERROR_DETECT==STD_ON
    	Det_ReportError(EA_MODULE_ID,0, apiId, EA_E_INVALID_BLOCK_NO);
    	result = E_NOT_OK;
#endif
    }
    return result;
}

/*@req <EA084>*/
/*@req <EA017>*/
void Ea_Init(void)
{
	/*init internal variables*/
	Ea_Global.EaBlockConfig = Ea_BlockConfigData;
	Ea_Global.EaGeneralPtr = &Ea_GeneralData;
	Ea_Global.JobResult = MEMIF_JOB_OK; /*@req <EA128> */
	Ea_Global.JobType = EA_JOB_NONE;
	Ea_Global.JobStatus = EA_PENDING_NONE;
	Ea_Global.ModuleStatus = MEMIF_IDLE; /*@req <EA128> */
}

/*@req <EA150> */
#if (STD_ON == EA_SET_MODE_SUPPORTED)
/*@req <EA085>*/
/*@req <EA020>*/
void Ea_SetMode(MemIf_ModeType Mode)
{
	if (E_OK != Ea_ValidateInitialized(EA_SETMODE_ID))
		return;

	/*@req <EA020> */
	Eep_SetMode(Mode);
}
#endif

/*@req <EA086>*/
/*@req <EA021>*/
Std_ReturnType Ea_Read(uint16 BlockNumber, uint16 BlockOffset, uint8* DataBufferPtr, uint16 Length)
{
	uint16 BlockIndex;
	const Ea_BlockConfigType *EaBlockCon;
    imask_t state;

	/*@req <EA130> */
	if (E_OK != Ea_ValidateInitialized(EA_READ_ID)){
		return E_NOT_OK;
	}
	/*@req <EA147> */
	if (E_OK != Ea_ValidateBlock(BlockNumber, EA_READ_ID)){
		return E_NOT_OK;
	}
	/*@req <EA137> */
	/* Lock down the module to ourself */
    Irq_Save(state);
	if (Ea_Global.ModuleStatus != MEMIF_IDLE)
	{
	    Irq_Restore(state);
		return E_NOT_OK;
	}
	/*set current state is internal busy*/
	Ea_Global.ModuleStatus = MEMIF_BUSY_INTERNAL;
    Irq_Restore(state);

	BlockIndex = EA_GET_BLOCK(BlockNumber);

	/*@req <EA147> */
	if (E_OK != Ea_ValidateBlockIndex(BlockIndex, EA_READ_ID))
	{
		Ea_Global.ModuleStatus = MEMIF_IDLE;
		return E_NOT_OK;
	}

	/*whether block device index match underlying driver in which Eep_Read will be invoked*/
	EaBlockCon = Ea_Global.EaBlockConfig;

	/*check whether BlockOffset add Length exceed the blocksize*/
	if (BlockOffset + Length > EaBlockCon[BlockIndex].EaBlockSize)
	{
		return E_NOT_OK;
	}
	
	Ea_Global.Address = DataBufferPtr;
	/*@req <EA021> :calculate the corresponding memory read address for underlying Eep_Read*/
	Ea_Global.EepAddress = calculateEepAddress(BlockIndex);
	Ea_Global.Length = EaBlockCon[BlockIndex].EaBlockSize +  sizeof(Ea_AdminBlock);
	Ea_Global.Offset = BlockOffset;
	Ea_Global.ReadLength = Length;
	Ea_Global.CurrentBlock = BlockNumber;

	/*@req <EA022> */
	Ea_Global.ModuleStatus = MEMIF_BUSY;
	Ea_Global.JobType = EA_JOB_READ;
	Ea_Global.JobResult = MEMIF_JOB_PENDING;
	Ea_Global.JobStatus = EA_PENDING_NONE;

	return E_OK;
}

/*@req <EA087> */
/*@req <EA024> */
/*@req <EA026> */
Std_ReturnType Ea_Write(uint16 BlockNumber, uint8* DataBufferPtr)
{
	uint16 BlockIndex;
	const Ea_BlockConfigType *EaBlockCon;
	Ea_AdminBlock* adminBlock;
    imask_t state;

	/*@req <EA131> */
	if (E_OK != Ea_ValidateInitialized(EA_WRITE_ID)){
		return E_NOT_OK;
	}
	/*@req <EA148> */
	if (E_OK != Ea_ValidateBlock(BlockNumber, EA_WRITE_ID)){
		return E_NOT_OK;
	}
	/*@req <EA137>
	*/
	/* Lock down the module to ourself */
    Irq_Save(state);
	if (Ea_Global.ModuleStatus != MEMIF_IDLE)
	{
	    Irq_Restore(state);
		return E_NOT_OK;
	}
	/*set current state is internal busy*/
	Ea_Global.ModuleStatus = MEMIF_BUSY_INTERNAL;
    Irq_Restore(state);

	BlockIndex = EA_GET_BLOCK(BlockNumber);

	/*@req <EA148> */
	if (E_OK != Ea_ValidateBlockIndex(BlockIndex, EA_READ_ID))
	{
		Ea_Global.ModuleStatus = MEMIF_IDLE;
		return E_NOT_OK;
	}

	/*whether block device index match underlying driver in which Eep_Read will be invoked*/
	EaBlockCon = Ea_Global.EaBlockConfig;

	/*caculate the EepAddress*/
	Ea_Global.EepAddress = calculateEepAddress(BlockIndex);
	Ea_Global.Length = EaBlockCon[BlockIndex].EaBlockSize + sizeof(Ea_AdminBlock);
	Ea_Global.Address = (void*)DataBufferPtr;
	Ea_Global.CurrentBlock = BlockNumber;

	/* Setup the admin block */
	/*@req <EA047> */
	adminBlock = (Ea_AdminBlock*) Ea_TempBuffer;
	adminBlock->blockNum = BlockNumber;
	adminBlock->blockState = BLOCK_INCONSISTENT;
	addChecksum(adminBlock);

	/* Copy the data to the buffer */
	uint8* destMem = (uint8*) (Ea_TempBuffer + sizeof(Ea_AdminBlock));
	memcpy(destMem, DataBufferPtr, EaBlockCon[BlockIndex].EaBlockSize);

	/*@req <EA025> [set internal variables set the EA module status to MEMIF_BUSY, set the job result to MEMIF_JOB_PENDING.]*/
	Ea_Global.ModuleStatus = MEMIF_BUSY;
	Ea_Global.JobType = EA_JOB_WRITE;
	Ea_Global.JobResult = MEMIF_JOB_PENDING;
	Ea_Global.JobStatus = EA_PENDING_NONE;

	return E_OK;
}

/*@req <EA088>*/
void Ea_Cancel(void)
{
	/*@req <EA132> */
	if (E_OK != Ea_ValidateInitialized(EA_CANCEL_ID)){
		// Do nothing.
	} else {
		/*@req <EA078> [Reset the Ea module's internal variables to make the module ready for a new job request.]*/
		Ea_Global.JobType = EA_JOB_NONE;
		Ea_Global.ModuleStatus = MEMIF_IDLE;
		Ea_Global.JobResult = MEMIF_JOB_CANCELLED;

		/*@req <EA077> [Call the cancel function of the underlying EEPROM driver.]*/
		Eep_Cancel();
	}

	return;
}

/*@req EA089 */
MemIf_StatusType Ea_GetStatus(void)
{
	MemIf_StatusType rv;

	/*@req <EA133> */
	/*@req <EA034> */
	if (E_OK != Ea_ValidateInitialized(EA_GETSTATUS_ID)){
		rv = MEMIF_UNINIT;
	}
	/*@req <EA156> */
	else if (MEMIF_IDLE == Ea_Global.ModuleStatus){
		rv = MEMIF_IDLE;
	}
	/*@req <EA073> [check if EA Module is busy with internal management operations.]*/	
	else if (MEMIF_BUSY_INTERNAL == Ea_Global.ModuleStatus){
		rv = MEMIF_BUSY_INTERNAL;
	} else {
		/*@req <EA157> */
		rv = MEMIF_BUSY;
	}
	return rv;
}

/*@req <EA090> */
MemIf_JobResultType Ea_GetJobResult(void)
{
	MemIf_JobResultType rv;
	/*@req <EA134> */
	if (E_OK != Ea_ValidateInitialized(EA_GETJOBRESULT_ID)){
		rv = MEMIF_JOB_FAILED;
	} else {
		/*@req <EA035> */
		rv = Ea_Global.JobResult;
	}
	return rv;
}

/*@req <EA091> */
Std_ReturnType Ea_InvalidateBlock(uint16 BlockNumber)
{
	uint16 BlockIndex;
	const Ea_BlockConfigType *EaBlockCon;
	Ea_AdminBlock* adminBlock;
	Std_ReturnType result;
    imask_t state;

	/*@req <EA135> */
	if (E_OK != Ea_ValidateInitialized(EA_INVALIDATEBLOCK_ID)){
		return E_NOT_OK;
	}
	/*@req <EA149> */
	if (E_OK != Ea_ValidateBlock(BlockNumber, EA_INVALIDATEBLOCK_ID)){
		return E_NOT_OK;
	}
	/*@req <EA137>
	*/
	/* Lock down the module to ourself */
    Irq_Save(state);
	if (Ea_Global.ModuleStatus != MEMIF_IDLE)
	{
	    Irq_Restore(state);
		return E_NOT_OK;
	}
	/*set current state is internal busy*/
	Ea_Global.ModuleStatus = MEMIF_BUSY_INTERNAL;
    Irq_Restore(state);

	BlockIndex = EA_GET_BLOCK(BlockNumber);

	/*@req <EA149> */
	if (E_OK != Ea_ValidateBlockIndex(BlockIndex, EA_INVALIDATEBLOCK_ID))
	{
		Ea_Global.ModuleStatus = MEMIF_IDLE;
		return E_NOT_OK;
	}

	/*whether block device index match underlying driver in which Eep_Read will be invoked*/
	EaBlockCon = Ea_Global.EaBlockConfig;

	/*@req <EA036> [calculate address]*/
	Ea_Global.EepAddress = calculateEepAddress(BlockIndex);
	Ea_Global.Length = EaBlockCon[BlockIndex].EaBlockSize;
	Ea_Global.CurrentBlock = BlockNumber;
	Ea_Global.ModuleStatus = MEMIF_BUSY;
	Ea_Global.JobType = EA_JOB_WRITE;
	Ea_Global.JobStatus = EA_PENDING_ADMIN_WRITE;
	Ea_Global.JobResult = MEMIF_JOB_PENDING;

	/*@req <EA037> [now we calling the erase function of the underlying device driver]*/
	/* We just set the Invalidate status of the admin block */
	/* Setup the admin block to be consistent again*/
	/*@req <EA047> */
	adminBlock = (Ea_AdminBlock*) Ea_TempBuffer;
	adminBlock->blockNum = BlockNumber;
	adminBlock->blockState = BLOCK_INVALIDATED;
	addChecksum(adminBlock);

	result = Eep_Write(Ea_Global.EepAddress, (const uint8*) Ea_TempBuffer, sizeof(Ea_AdminBlock));
	if (E_OK == result)
	{
	    Irq_Save(state);
		MemIf_StatusType status = Eep_GetStatus();
		if ((status == MEMIF_BUSY) || (status == MEMIF_BUSY_INTERNAL)){
			Ea_Global.JobStatus = EA_PENDING_ADMIN_WRITE;
		}
	    Irq_Restore(state);
	}
	else
	{
		Ea_Global.JobType = EA_JOB_NONE;
		Ea_Global.JobResult = MEMIF_JOB_FAILED;
		Ea_Global.JobStatus = EA_PENDING_NONE;
		Ea_Global.ModuleStatus = MEMIF_IDLE;
		return E_NOT_OK;
	}

	return E_OK;
}

/*@req <EA093>*/
Std_ReturnType Ea_EraseImmediateBlock(uint16 BlockNumber)
{
	uint16 BlockIndex;
	const Ea_BlockConfigType *EaBlockCon;
	imask_t state;

	/*@req <EA136> */
	if (E_OK != Ea_ValidateInitialized(EA_ERASEIMMEDIATEBLOCK_ID)){
		return E_NOT_OK;
	}
	/*@req <EA152> */
	if (E_OK != Ea_ValidateBlock(BlockNumber, EA_ERASEIMMEDIATEBLOCK_ID)){
		return E_NOT_OK;
	}
	/*@req <EA137>
	*/
	/* Lock down the module to ourself */
    Irq_Save(state);
	if (Ea_Global.ModuleStatus != MEMIF_IDLE)
	{
	    Irq_Restore(state);
		return E_NOT_OK;
	}
	/*set current state is internal busy*/
	Ea_Global.ModuleStatus = MEMIF_BUSY_INTERNAL;
    Irq_Restore(state);

	BlockIndex = EA_GET_BLOCK(BlockNumber);

	/*@req <EA152> */
	if (E_OK != Ea_ValidateBlockIndex(BlockIndex, EA_ERASEIMMEDIATEBLOCK_ID))
	{
		Ea_Global.ModuleStatus = MEMIF_IDLE;
		return E_NOT_OK;
	}

	EaBlockCon = Ea_Global.EaBlockConfig;
	
	/*@req <EA065> [check whether the addressed logical block is configured as containing immediate data]*/
	if (false == EaBlockCon[BlockIndex].EaImmediateData)
	{
		Det_ReportError(EA_MODULE_ID, 0, EA_ERASEIMMEDIATEBLOCK_ID, EA_E_INVALID_BLOCK_NO);

		Ea_Global.ModuleStatus = MEMIF_IDLE;
		return E_NOT_OK;
	}

	/*@req <EA063> [take the block number and calculate the corresponding memory block address.]*/
	Ea_Global.EepAddress = calculateEepAddress(BlockIndex);
	Ea_Global.Length = calculateBlockLength(BlockIndex); // Calculate the block length in full pages
	Ea_Global.CurrentBlock = BlockNumber;

	/*@req <EA025> [set internal variables set the EA module status to MEMIF_BUSY, set the job result to MEMIF_JOB_PENDING.]*/
	Ea_Global.ModuleStatus = MEMIF_BUSY;
	Ea_Global.JobType = EA_JOB_ERASE;
	Ea_Global.JobResult = MEMIF_JOB_PENDING;
	Ea_Global.JobStatus = EA_PENDING_NONE;

	return E_OK;
}


/*@req <EA096>*/
/*@req <EA056>*/
void Ea_MainFunction(void)
{
	Std_ReturnType result;
	imask_t state;

	if ((MEMIF_JOB_PENDING == Ea_Global.JobResult) && (Ea_Global.JobStatus == EA_PENDING_NONE))
	{
		switch (Ea_Global.JobType)
		{
			case EA_JOB_WRITE:
				/*@req <EA026> */
				if (MEMIF_IDLE == Eep_GetStatus())
				{
					result = Eep_Write(Ea_Global.EepAddress, (const uint8*) Ea_TempBuffer, Ea_Global.Length);
					if (E_OK == result)
					{
					    Irq_Save(state);
						MemIf_StatusType status = Eep_GetStatus();
						if ((status == MEMIF_BUSY) || (status == MEMIF_BUSY_INTERNAL)){
							Ea_Global.JobStatus = EA_PENDING_WRITE;
						}
					    Irq_Restore(state);
					}
				}
				break;
			case EA_JOB_READ:
				/*@req <EA072> */
				if (MEMIF_IDLE == Eep_GetStatus())
				{
					result = Eep_Read(Ea_Global.EepAddress, (uint8*) Ea_TempBuffer, Ea_Global.Length);
					if (E_OK == result)
					{
					    Irq_Save(state);
						MemIf_StatusType status = Eep_GetStatus();
						if ((status == MEMIF_BUSY) || (status == MEMIF_BUSY_INTERNAL)){
							Ea_Global.JobStatus = EA_PENDING_READ;
						}
					    Irq_Restore(state);
					}
				}
				break;
			case EA_JOB_ERASE:
				if (MEMIF_IDLE == Eep_GetStatus())
				{
					result = Eep_Erase(Ea_Global.EepAddress, Ea_Global.Length);
					if (E_OK == result)
					{
					    Irq_Save(state);
						MemIf_StatusType status = Eep_GetStatus();
						if ((status == MEMIF_BUSY) || (status == MEMIF_BUSY_INTERNAL)){
							Ea_Global.JobStatus = EA_PENDING_ERASE;
						}
					    Irq_Restore(state);
					}
				}
				break;

			default:
				break;
		}
	}
	else
	{
		// We have pending jobs. Wait for them to finish.
		switch(Ea_Global.JobStatus)
		{
		default:
			// Nothing to do for this state. Just wait
			break;
		}
	}
}


/*Callback notifications of the Ea module*/
/*@req <EA094> */
/*@req <EA153> */
/*@req <EA101> */
void Ea_JobEndNotification(void)
{
	MemIf_JobResultType jobResult = Eep_GetJobResult();

	if (MEMIF_JOB_CANCELLED == jobResult)
	{
		Ea_Global.JobType = EA_JOB_NONE;
		Ea_Global.JobResult = MEMIF_JOB_CANCELLED;
		Ea_Global.JobStatus = EA_PENDING_NONE;
		Ea_Global.ModuleStatus = MEMIF_IDLE;
		// Should we  call upper layer here?
	} else {
		/*@req <EA051> */
		/*@req <EA054> */
		switch(Ea_Global.JobStatus)
		{
		case EA_PENDING_READ:
			handleLowerLayerRead(jobResult);
			break;
		case EA_PENDING_WRITE:
			handleLowerLayerWrite(jobResult);
			break;
		case EA_PENDING_ERASE:
			handleLowerLayerErase(jobResult);
			break;
		case EA_PENDING_ADMIN_WRITE:
			Ea_Global.JobType = EA_JOB_NONE;
			Ea_Global.JobStatus = EA_PENDING_NONE;
			Ea_Global.ModuleStatus = MEMIF_IDLE;
			Ea_Global.JobResult = jobResult;
			/*@req <EA141> */
			/*@req <EA054> */
			/*@req <EA142> */
			/*@req <EA143> */
			EA_JOB_END_NOTIFICATION();
			break;
		default:
			assert(0); // Should never come here
			break;
		}
	}
	return;
}

static void handleLowerLayerRead(MemIf_JobResultType jobResult)
{
	Ea_AdminBlock* adminBlock;

	/* Check the admin block i.e the block is consistent */
	/*@req <EA104> */
	adminBlock = (Ea_AdminBlock*) Ea_TempBuffer;
	if ((!verifyChecksum(adminBlock)) || (adminBlock->blockState == BLOCK_INCONSISTENT)
			|| (adminBlock->blockNum != Ea_Global.CurrentBlock))
	{
		Ea_Global.JobType = EA_JOB_NONE;
		/*@req <EA055> */
		Ea_Global.JobResult = MEMIF_BLOCK_INCONSISTENT;
		Ea_Global.JobStatus = EA_PENDING_NONE;
		Ea_Global.ModuleStatus = MEMIF_IDLE;
		EA_JOB_ERROR_NOTIFICATION();
		return;
	}
	/* Check if block is invalidated */
	/*@req <EA074> */
	if (adminBlock->blockState == BLOCK_INVALIDATED)
	{
		Ea_Global.JobType = EA_JOB_NONE;
		/*@req <EA055> */
		Ea_Global.JobResult = MEMIF_BLOCK_INVALID;
		Ea_Global.JobStatus = EA_PENDING_NONE;
		Ea_Global.ModuleStatus = MEMIF_IDLE;
		EA_JOB_ERROR_NOTIFICATION();
		return;
	}

	// Copy the data to the destination
	uint8* srcadress = (uint8*) (Ea_TempBuffer + sizeof (Ea_AdminBlock) + Ea_Global.Offset);
	memcpy(Ea_Global.Address, srcadress, Ea_Global.ReadLength);

	/* Sucess */
	Ea_Global.JobType = EA_JOB_NONE;
	Ea_Global.JobStatus = EA_PENDING_NONE;
	Ea_Global.ModuleStatus = MEMIF_IDLE;
	Ea_Global.JobResult = jobResult;
	/*@req <EA054> */
	EA_JOB_END_NOTIFICATION();
}

static void handleLowerLayerWrite(MemIf_JobResultType jobResult)
{
	Ea_AdminBlock* adminBlock;
	Std_ReturnType result;
	imask_t state;

	if (jobResult == MEMIF_JOB_OK)
	{
		/* Setup the admin block to be consistent again*/
		/*@req <EA047> */
		adminBlock = (Ea_AdminBlock*) Ea_TempBuffer;
		adminBlock->blockNum = Ea_Global.CurrentBlock;
		adminBlock->blockState = BLOCK_CONSISTENT;
		addChecksum(adminBlock);

		result = Eep_Write(Ea_Global.EepAddress, (const uint8*) Ea_TempBuffer, sizeof(Ea_AdminBlock));
		if (E_OK == result)
		{
		    Irq_Save(state);
			MemIf_StatusType status = Eep_GetStatus();
			if ((status == MEMIF_BUSY) || (status == MEMIF_BUSY_INTERNAL)){
				Ea_Global.JobStatus = EA_PENDING_ADMIN_WRITE;
			}
		    Irq_Restore(state);
		} else
		{
			Ea_Global.JobResult = MEMIF_JOB_FAILED;
			Ea_Global.JobType = EA_JOB_NONE;
			Ea_Global.JobStatus = EA_PENDING_NONE;
			Ea_Global.ModuleStatus = MEMIF_IDLE;

			/*@req <EA054> */
			EA_JOB_END_NOTIFICATION();
		}
	}
	else
	{
		/* Report error upstreams */
		Ea_Global.JobType = EA_JOB_NONE;
		/*@req <EA055> */
		Ea_Global.JobResult = MEMIF_BLOCK_INVALID;
		Ea_Global.JobStatus = EA_PENDING_NONE;
		Ea_Global.ModuleStatus = MEMIF_IDLE;

		/*@req <EA054> */
		EA_JOB_END_NOTIFICATION();
	}
}

static void handleLowerLayerErase(MemIf_JobResultType jobResult)
{
	Ea_AdminBlock* adminBlock;
	Std_ReturnType result;
	imask_t state;

	if (jobResult == MEMIF_JOB_OK)
	{
		/* Setup the admin block to be consistent again*/
		/*@req <EA047> */
		adminBlock = (Ea_AdminBlock*) Ea_TempBuffer;
		adminBlock->blockNum = Ea_Global.CurrentBlock;
		adminBlock->blockState = BLOCK_INCONSISTENT;
		addChecksum(adminBlock);

		result = Eep_Write(Ea_Global.EepAddress, (const uint8*) Ea_TempBuffer, sizeof(Ea_AdminBlock));
		if (E_OK == result)
		{
		    Irq_Save(state);
			MemIf_StatusType status = Eep_GetStatus();
			if ((status == MEMIF_BUSY) || (status == MEMIF_BUSY_INTERNAL)){
				Ea_Global.JobStatus = EA_PENDING_ADMIN_WRITE;
			}
		    Irq_Restore(state);
		} else
		{
			Ea_Global.JobResult = MEMIF_JOB_FAILED;
			Ea_Global.JobType = EA_JOB_NONE;
			Ea_Global.JobStatus = EA_PENDING_NONE;
			Ea_Global.ModuleStatus = MEMIF_IDLE;

			/*@req <EA054> */
			EA_JOB_END_NOTIFICATION();
		}
	}
	else
	{
		/* Report error upstreams */
		Ea_Global.JobType = EA_JOB_NONE;
		/*@req <EA055> */
		Ea_Global.JobResult = MEMIF_BLOCK_INVALID;
		Ea_Global.JobStatus = EA_PENDING_NONE;
		Ea_Global.ModuleStatus = MEMIF_IDLE;

		/*@req <EA054> */
		EA_JOB_END_NOTIFICATION();
	}
}


/*@req <EA095> */
/*@req <EA102> */
void Ea_JobErrorNotification(void)
{
	imask_t state;
    Irq_Save(state);

    /*@req EA154*/
	if (Ea_Global.JobResult == MEMIF_JOB_PENDING){
		Ea_Global.JobResult = MEMIF_JOB_FAILED;
	}

	Ea_Global.JobType = EA_JOB_NONE;
	Ea_Global.JobStatus = EA_PENDING_NONE;
	Ea_Global.ModuleStatus = MEMIF_IDLE;
    Irq_Restore(state);

	/*@req <EA055> */
	/*@req <EA144> */
	/*@req <EA145> */
	/*@req <EA146> */
	/*@req <EA100> */
	/*@req <EA053> */
	EA_JOB_ERROR_NOTIFICATION();
}

/*
 * Local service to get block index in Ea_Lcfg.c
 */
static uint16 EA_GET_BLOCK(uint16 BlockNumber)
{
	const Ea_BlockConfigType *EaBlockCon;
	uint16 BlockIndex;

	EaBlockCon = Ea_Global.EaBlockConfig;
	for (BlockIndex = 0; BlockIndex < EA_NUMBER_OF_BLOCKS; BlockIndex++)
	{
		if (EaBlockCon[BlockIndex].EaBlockNumber == BlockNumber)
		{
			break;
		}
	}
	return BlockIndex;
}

/*
 * Local service to calculate the actual eep address.
 */
/*@req <EA007>*/
static Eep_AddressType calculateEepAddress(uint16 BlockIndex)
{
	const Ea_BlockConfigType *EaBlockCon;
	uint32 totalNumOfBlocks = 0;
	uint16 i;

	EaBlockCon = Ea_Global.EaBlockConfig;
	uint16 blockNum = EaBlockCon[BlockIndex].EaBlockNumber;
	uint8 device = EaBlockCon[BlockIndex].EaDeviceIndex;

	for (i = 0; i < EA_NUMBER_OF_BLOCKS; i++)
	{
		if (EaBlockCon[i].EaDeviceIndex == device) // Check that this is the same device
		{
			if (EaBlockCon[i].EaBlockNumber < blockNum) // Check that blocknum is less than the searched one
			{
				int blocksize = EaBlockCon[i].EaBlockSize + sizeof(Ea_AdminBlock);
				int numOfBlocks = blocksize / EA_VIRTUAL_PAGE_SIZE;
				if (blocksize % EA_VIRTUAL_PAGE_SIZE){
					numOfBlocks++;
				}

				totalNumOfBlocks = totalNumOfBlocks + numOfBlocks;
			}
		}
	}

	return totalNumOfBlocks * EA_VIRTUAL_PAGE_SIZE;
}

static uint16 calculateBlockLength(uint16 BlockIndex)
{
	const Ea_BlockConfigType *EaBlockCon;

	EaBlockCon = Ea_Global.EaBlockConfig;
	int blocksize = EaBlockCon[BlockIndex].EaBlockSize + sizeof(Ea_AdminBlock);
	int numOfBlocks = blocksize / EA_VIRTUAL_PAGE_SIZE;
	if (blocksize % EA_VIRTUAL_PAGE_SIZE){
		numOfBlocks++;
	}

	return numOfBlocks * EA_VIRTUAL_PAGE_SIZE;
}

/* Some very simple checksum calculations */
/* Better than nothing :-) */
static uint8 verifyChecksum(Ea_AdminBlock* block)
{
	uint8 result;
	uint8* array = (uint8*) block;

	result = array[0];
	result += array[1];
	result += array[2];

	result = result ^ 0xaaU;

	return (result == block->check);
}

static void addChecksum(Ea_AdminBlock* block)
{
	uint8 result;
	uint8* array = (uint8*) block;

	result = array[0];
	result += array[1];
	result += array[2];

	result = result ^ 0xaaU;

	block->check = result;
}
