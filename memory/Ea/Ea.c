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

/** @req EA113 @req EA112 @req EA049 @req EA038 */
/** @req EA116 @req EA117 @req EA083 @req EA097 */

#include "Ea.h"
#include "Ea_Cbk.h"
#include "Ea_Types.h"

// States used by EA_MainFunction to control the internal state of the module.
typedef enum
{
	EA_PENDING_NONE,
	EA_PENDING_WRITE,
	EA_PENDING_READ,
	EA_PENDING_ERASE
} Ea_JobStatus;

#include "Mcu.h"

 /** @req EA098 */
#if (STD_ON == EA_DEV_ERROR_DETECT)
#include "Det.h"
#endif

/*
	ITEM NAME:		<EA_VALIDATE_RV>
	SCOPE:			<Ea Module>
	DESCRIPTION:
		validation macro	
*/
/* @req EA011 @req EA059 @req EA045 */
#if (STD_ON == EA_DEV_ERROR_DETECT)
#define EA_VALIDATE(condition, api, errid)	\
					if (!(condition))	\
					{	\
						Det_ReportError(MODULE_ID_EA, 0, api, errid);	\
						return;		\
					}
#else
#define EA_VALIDATE(condition, api, errid)	\
					if (!(condition))	\
					{	\
						return;	\
					}
#endif

#if (STD_ON == EA_DEV_ERROR_DETECT)
#define EA_VALIDATE_RV(condition, api, errid)	\
					if (!(condition))	\
					{	\
						Det_ReportError(MODULE_ID_EA, 0, api, errid);	\
						return E_NOT_OK;		\
					}
#else
#define EA_VALIDATE_RV(condition, api, errid)	\
					if (!(condition))	\
					{	\
						return E_NOT_OK;	\
					}
#endif

/*
	ITEM NAME:		<Ea Notify Callback Macro>
	SCOPE:			<EA Module>
	DESCRIPTION:
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
	MACRO NAME:	<Job Error Process Macro>
	SCOPE:			<EA Module>
	DESCRIPTION:
		define EA module Job Error Process macro 	
*/
#define EA_JOB_ERROR_PROCESS()	\
	Ea_Global.JobType = EA_JOB_NONE;	\
	if (MEMIF_JOB_PENDING == Ea_Global.JobResult)	\
		Ea_Global.JobResult = MEMIF_JOB_FAILED;	\
	Ea_Global.ModuleStatus = MEMIF_IDLE;	

/*
	MACRO NAME:	<Job End Process Macro>
	SCOPE:			<EA Module>
	DESCRIPTION:
		define EA module Job End Process macro
*/
#define EA_JOB_END_PROCESS()	\
	Ea_Global.JobType = EA_JOB_NONE;	\
	if (MEMIF_JOB_PENDING == Ea_Global.JobResult)	\
		Ea_Global.JobResult = MEMIF_JOB_OK;	\
	Eep_Global.ModuleStatus = MEMIF_IDLE;

/*
	ITEM NAME:		<Ea_GlobalType>
	SCOPE:			<EA Module>
	DESCRIPTION:
		define EA Module Global Type  	
*/
typedef struct {
	const Ea_BlockConfigType *EaBlockConfig;
	const Ea_GeneralType	*EaGeneralPtr;
	uint8 					CurBlockIndex;
	uint16					EaBlockBaseNum;
	MemIf_StatusType    	ModuleStatus;
	MemIf_JobResultType 	JobResult;
	Ea_JobType    			JobType;
	Eep_LengthType			Length;
	Eep_AddressType			EepAddress;
	Ea_JobStatus            JobStatus; // Internal state to be used by main function.
	void*					Address;
}Ea_GlobalType;

/*
	ITEM NAME:		<Ea_Global>
	SCOPE:			<EA Module>
	DESCRIPTION:
		define EA Module Global  	
*/
Ea_GlobalType Ea_Global = {
	NULL_PTR,
	NULL_PTR,
	0,
	0,
	MEMIF_UNINIT,
	MEMIF_JOB_OK,
	EA_JOB_NONE,
};

static uint16 EA_GET_BLOCK(uint16 BlockNumber);
static Eep_AddressType CalculateEepAddress(uint16 BlockIndex, uint16 BlockOffset);
/*
	FUNCTION NAME:		<Ea_Init>
	COMPONENT:			<Ea Driver>
	SCOPE:				<Ea Module>
	DESCRIPTION:
		Initializes the EEPROM abstraction module.
*/
/*@req <EA084>
*/
/*@req <EA017>
*/
void Ea_Init(void)
{
	/*init internal variables*/
	Ea_Global.EaBlockConfig = Ea_BlockConfigData;
	Ea_Global.EaGeneralPtr = &Ea_GeneralData;
	Ea_Global.EaBlockBaseNum = Ea_BlockConfigData[0].EaBlockNumber >> NVM_DATASET_SEL_BITS;
	Ea_Global.JobResult = MEMIF_JOB_OK;
	Ea_Global.JobType = EA_JOB_NONE;
	Ea_Global.JobStatus = EA_PENDING_NONE;
	Ea_Global.ModuleStatus = MEMIF_IDLE;
}

#if (STD_ON == EA_SET_MODE_SUPPORTED)
/*@req <EA085>
*/
/*@req <EA020>
*/
void Ea_SetMode(MemIf_ModeType Mode)
{
	Eep_SetMode(Mode);
}
#endif

/*@req <EA086>
*/
/*@req <EA021>
*/
Std_ReturnType Ea_Read(uint16 BlockNumber, uint16 BlockOffset, uint8* DataBufferPtr, uint16 Length)
{
	uint16 BlockIndex;
	const Ea_BlockConfigType *EaBlockCon;
	Std_ReturnType ReturnValue;

	/* Lock down the module to ourself */
	imask_t mask = McuE_EnterCriticalSection();
	if (Ea_Global.ModuleStatus != MEMIF_IDLE)
	{
		McuE_ExitCriticalSection(mask);
		return E_NOT_OK;
	}
	/*set current state is internal busy*/
	Ea_Global.ModuleStatus = MEMIF_BUSY_INTERNAL;
	McuE_ExitCriticalSection(mask);

	BlockIndex = EA_GET_BLOCK(BlockNumber);

	EA_VALIDATE_RV(EA_NUMBER_OF_BLOCKS > BlockIndex, EA_READ_ID, EA_E_INVALID_BLOCK_NO);

	/*whether block device index match underlying driver in which Eep_Read will be invoked*/
	EaBlockCon = Ea_Global.EaBlockConfig;
	if (EaBlockCon[BlockIndex].EaDeviceIndex != EEP_DRIVER_INDEX)	//EepGeneralData.EepDriverIndex) changed by lee
	{
		return E_NOT_OK;
	}
	/*check whether BlockOffset add Length exceed the blocksize*/
	if (BlockOffset + Length > EaBlockCon[BlockIndex].EaBlockSize)
	{
		return E_NOT_OK;
	}
	/*@req <EA007>
	*/
	/*@req <EA066>
	*/
	
	/*EA021:calculate the corresponding memory read address for underlying Eep_Read*/
	Ea_Global.EepAddress = CalculateEepAddress(BlockIndex, BlockOffset);

	/*EA115:check whether the requested block has been invalidated (see Ea_InvalidateBlock) 
	by the application. */


	/*@req <EA022> [Ea_Read shall call the read function of the underlying EEPROM driver with the calculated read 
	address, the length and data buffer parameters]*/
	/*set Module busy*/
	Ea_Global.ModuleStatus = MEMIF_BUSY;
	Ea_Global.JobType = EA_JOB_READ;
	Ea_Global.JobResult = MEMIF_JOB_PENDING;
	Ea_Global.JobStatus = EA_PENDING_READ;

	ReturnValue = Eep_Read(Ea_Global.EepAddress, DataBufferPtr, Length);

	/*@req <EA072> [return the ReturnValue]*/
	return ReturnValue;
}

/*@req <EA087>
*/
/*@req <EA024>
*/
/*@req <EA026>
*/
Std_ReturnType Ea_Write(uint16 BlockNumber, uint8* DataBufferPtr)
{
	uint16 BlockIndex;
	const Ea_BlockConfigType *EaBlockCon;

	/* Lock down the module to ourself */
	imask_t mask = McuE_EnterCriticalSection();
	if (Ea_Global.ModuleStatus != MEMIF_IDLE)
	{
		McuE_ExitCriticalSection(mask);
		return E_NOT_OK;
	}
	/*set current state is internal busy*/
	Ea_Global.ModuleStatus = MEMIF_BUSY_INTERNAL;
	McuE_ExitCriticalSection(mask);

	BlockIndex = EA_GET_BLOCK(BlockNumber);

	EA_VALIDATE_RV(EA_NUMBER_OF_BLOCKS > BlockIndex, EA_WRITE_ID, EA_E_INVALID_BLOCK_NO);

	/*whether block device index match underlying driver in which Eep_Read will be invoked*/
	EaBlockCon = Ea_Global.EaBlockConfig;
	if (EaBlockCon[BlockIndex].EaDeviceIndex != EEP_DRIVER_INDEX)	//EepGeneralData.EepDriverIndex) changed by lee
	{
		return E_NOT_OK;
	}

	/*caculate the EepAddress*/
	Ea_Global.EepAddress = CalculateEepAddress(BlockIndex, 0);  // No offsets used in write operations
	Ea_Global.Length = EaBlockCon[BlockIndex].EaBlockSize;
	Ea_Global.Address = (void*)DataBufferPtr;

	/*@req <EA025> [set internal variables set the EA module status to MEMIF_BUSY, set the job result to MEMIF_JOB_PENDING.]*/
	Ea_Global.CurBlockIndex = BlockIndex;
	Ea_Global.JobType = EA_JOB_WRITE;
	Ea_Global.JobResult = MEMIF_JOB_PENDING;
	Ea_Global.ModuleStatus = MEMIF_BUSY;

	return E_OK;
}

/*@req <EA088>
*/
void Ea_Cancel(void)
{
	/*@req <EA078> [Reset the Ea module's internal variables to make the module ready for a new job request.]*/
	Ea_Global.JobType = EA_JOB_NONE;
	Ea_Global.ModuleStatus = MEMIF_IDLE;
	Ea_Global.JobResult = MEMIF_JOB_CANCELLED;

	/*@req <EA077> [Call the cancel function of the underlying EEPROM driver.]*/
	Eep_Cancel();

	return;
}

/** @req EA089 */
MemIf_StatusType Ea_GetStatus(void)
{
	MemIf_StatusType Rv;
	
	/*@req <EA073> [check if EA Module is busy with internal management operations.]*/	
	if (MEMIF_BUSY_INTERNAL == Ea_Global.ModuleStatus)
	{
		return MEMIF_BUSY_INTERNAL;
	}

	/*@req <EA034> [If no internal operation is currently ongoing, call the GetStatus function.]*/
	if (MEMIF_UNINIT == Ea_Global.ModuleStatus)
	{
		return MEMIF_UNINIT;
	}
	Rv = Eep_GetStatus();
	
	return Rv;
}

/*@req <EA090>
*/
MemIf_JobResultType Ea_GetJobResult(void)
{
	MemIf_JobResultType Rv;

	/*@req <EA035> [call the 'GetJobResult' function of the underlying EEPROM driver and pass the return value back to the caller.]*/
	Rv = Eep_GetJobResult();

	return Rv;
}

/*@req <EA091>
*/
Std_ReturnType Ea_InvalidateBlock(uint16 BlockNumber)
{
	uint16 BlockIndex;
	const Ea_BlockConfigType *EaBlockCon;
	Std_ReturnType ReturnValue;

	/* Lock down the module to ourself */
	imask_t mask = McuE_EnterCriticalSection();
	if (Ea_Global.ModuleStatus != MEMIF_IDLE)
	{
		McuE_ExitCriticalSection(mask);
		return E_NOT_OK;
	}
	/*set current state is internal busy*/
	Ea_Global.ModuleStatus = MEMIF_BUSY_INTERNAL;
	McuE_ExitCriticalSection(mask);

	BlockIndex = EA_GET_BLOCK(BlockNumber);

	EA_VALIDATE_RV(EA_NUMBER_OF_BLOCKS != BlockIndex, EA_INVALIDATEBLOCK_ID, EA_E_INVALID_BLOCK_NO);

	/*whether block device index match underlying driver in which Eep_Read will be invoked*/
	EaBlockCon = Ea_Global.EaBlockConfig;
	if (EaBlockCon[BlockIndex].EaDeviceIndex != EEP_DRIVER_INDEX)	//EepGeneralData.EepDriverIndex) changed by lee
	{
		return E_NOT_OK;
	}

	/*@req <EA036> [calculate address]*/
	Ea_Global.EepAddress = CalculateEepAddress(BlockIndex, 0);
	Ea_Global.Length = EaBlockCon[BlockIndex].EaBlockSize;
	Ea_Global.ModuleStatus = MEMIF_BUSY;
	Ea_Global.JobType = EA_JOB_ERASE;
	Ea_Global.JobStatus = EA_PENDING_ERASE;
	Ea_Global.JobResult = MEMIF_JOB_PENDING;

	/*@req <EA037> [now we calling the erase function of the underlying device driver]*/
	ReturnValue = Eep_Erase(Ea_Global.EepAddress, Ea_Global.Length);
	
	return ReturnValue;
}

/*@req <EA093>
*/
Std_ReturnType Ea_EraseImmediateBlock(uint16 BlockNumber)
{
	uint16 BlockIndex;
	const Ea_BlockConfigType *EaBlockCon;
	Std_ReturnType Rv;

	/*set current state is interal busy*/
	Ea_Global.ModuleStatus = MEMIF_BUSY_INTERNAL;
	BlockIndex = EA_GET_BLOCK(BlockNumber);
	EaBlockCon = Ea_Global.EaBlockConfig;

	EA_VALIDATE_RV(EA_NUMBER_OF_BLOCKS > BlockIndex, EA_ERASEIMMEDIATEBLOCK_ID, EA_E_INVALID_BLOCK_NO);
	
	/*@req <EA065> [check whether the addressed logical block is configured as containing immediate data]*/
	EA_VALIDATE_RV((TRUE == EaBlockCon[BlockIndex].EaImmediateData), EA_ERASEIMMEDIATEBLOCK_ID, EA_E_INVALID_BLOCK_NO);
	
	/*whether block device index match underlying driver in which Eep_Read will be invoked*/
	if (EaBlockCon[BlockIndex].EaDeviceIndex != EEP_DRIVER_INDEX)	//EepGeneralData.EepDriverIndex) changed by lee
	{
		return E_NOT_OK;
	}

	/*@req <EA063> [take the block number and calculate the corresponding memory block address.]*/
	Ea_Global.EepAddress = CalculateEepAddress(BlockIndex, 0);
	Ea_Global.Length = EaBlockCon[BlockIndex].EaBlockSize;

	Ea_Global.ModuleStatus = MEMIF_BUSY;
	Ea_Global.JobType = EA_JOB_ERASE;
	Ea_Global.JobResult = MEMIF_JOB_PENDING;

	/*@req <EA064> [ensure that the EA module can write immediate data.call erase function of the underlying driver]*/
	
	Rv = Eep_Erase(Ea_Global.EepAddress, Ea_Global.Length);

	return Rv;
}

/*Scheduled functions*/
/*
	FUNCTION NAME:		<Ea_MainFunction>
	COMPONENT:			<Ea Driver>
	SCOPE:				<Ea Module>
	DESCRIPTION:
		Service to handle the requested jobs and the internal management operations.
*/

/*@req <EA096>
*/
/*@req <EA056>
*/
void Ea_MainFunction(void)
{
	Std_ReturnType result;

	if ((MEMIF_JOB_PENDING == Ea_Global.JobResult) && (Ea_Global.JobStatus == EA_PENDING_NONE))
	{
		/*check whether the block requested for reading has been invalidated by the upper layer module.*/

		/*check the consistency of the logical block being read before notifying the caller. */

		switch (Ea_Global.JobType)
		{
			case EA_JOB_WRITE:
				if (MEMIF_IDLE == Eep_GetStatus())
				{
					result = Eep_Write(Ea_Global.EepAddress, (const uint8*) Ea_Global.Address, Ea_Global.Length);
					if (E_OK == result)
					{
						imask_t mask = McuE_EnterCriticalSection();
						MemIf_StatusType status = Eep_GetStatus();
						if ((status == MEMIF_BUSY) || (status == MEMIF_BUSY_INTERNAL))
							Ea_Global.JobStatus = EA_PENDING_WRITE;
						McuE_ExitCriticalSection(mask);
					}
				}
				break;

			default:
				break;
		}
	}
}

/*Callback notifications of the Ea module*/
 /** @req EA099 @req EA054 @req EA106 */
void Ea_JobEndNotification(void)
{
	imask_t mask = McuE_EnterCriticalSection();
	Ea_Global.JobResult = Eep_GetJobResult();
	Ea_Global.JobType = EA_JOB_NONE;
	Ea_Global.JobStatus = EA_PENDING_NONE;
	Ea_Global.ModuleStatus = MEMIF_IDLE;
	McuE_ExitCriticalSection(mask);

	EA_JOB_END_NOTIFICATION();
}

/** @req EA100 @req EA055 @req EA107 */
void Ea_JobErrorNotification(void)
{
	imask_t mask = McuE_EnterCriticalSection();
	Ea_Global.JobResult = Eep_GetJobResult();
	Ea_Global.JobType = EA_JOB_NONE;
	Ea_Global.JobStatus = EA_PENDING_NONE;
	Ea_Global.ModuleStatus = MEMIF_IDLE;
	McuE_ExitCriticalSection(mask);

	EA_JOB_ERROR_NOTIFICATION();
}

/*
	DESCRIPTION:
		Local service to get block index in Ea_Lcfg.c 
*/
static uint16 EA_GET_BLOCK(uint16 BlockNumber)
{
	const Ea_BlockConfigType *EaBlockCon;
	uint8 BlockIndex;

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
	DESCRIPTION:
		Local service to calculate the actual eep address.
*/
Eep_AddressType CalculateEepAddress(uint16 BlockIndex, uint16 BlockOffset)
{
	const Ea_BlockConfigType *EaBlockCon;
	Eep_AddressType rVal = 0;
	int i = 0;

	EaBlockCon = Ea_Global.EaBlockConfig;
	uint16 blockNum = EaBlockCon[BlockIndex].EaBlockNumber;
	uint8 device = EaBlockCon[BlockIndex].EaDeviceIndex;

	for (i = 0; i < EA_NUMBER_OF_BLOCKS; i++)
	{
		if (EaBlockCon[i].EaDeviceIndex == device) // Check that this is the same device
		{
			if (EaBlockCon[i].EaBlockNumber < blockNum) // Check that blocknum is less than the searched one
				rVal = rVal + EaBlockCon[i].EaBlockSize;
		}
	}

	return rVal;
}
