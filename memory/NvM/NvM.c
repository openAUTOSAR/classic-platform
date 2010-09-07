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
#if  ( NVM_DEV_ERROR_DETECT == STD_ON )
#include "Det.h"
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_NVM, 0, _api, _err); \
          return E_NOT_OK; \
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
#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(_x, _y, _z, _q)

#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_RV(_exp,_api,_err,_rv )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif

// State variable
typedef enum
{
  NVM_UNINITIALIZED = 0,
  NVM_INITIALIZED
} Nvm_StateType;

static Nvm_StateType nvmState = DEM_UNINITIALIZED;

typedef struct {
	uint8					DataIndex;				// Selected data index if "Data Set" type
	boolean					BlockWriteProtected;	// Block write protected?
	NvM_RequestResultType	ErrorStatus;			// Status of block
	boolean					BlockChanged;			// Block changed?
	boolean					BlockValid;				// Block valid?
	uint8					NumberOfWriteRetries;	// Current write retry cycle


} AdministrativeBlockType;

AdministrativeBlockType AdminBlock[NUM_OF_NVRAM_BLOCKS];



const NvM_BlockDescriptorType	*BlockDescriptorList = NvM_Config.BlockDescriptor;


void NvM_Init(void)
{
	uint i;

	// Initiate the administration blocks
	for (i = 2; i< NUM_OF_NVRAM_BLOCKS; i++) {
		AdminBlock[i].DataIndex = 0;
		AdminBlock[i].BlockWriteProtected = BlockDescriptorList[i].BlockWriteProt;
		AdminBlock[i].ErrorStatus = NVM_REQ_OK;
		AdminBlock[i].BlockChanged = FALSE;
		AdminBlock[i].BlockValid = FALSE;
		AdminBlock[i].NumberOfWriteRetries = 0;
	}

	// Set status to initialized
	nvmState = NVM_INITIALIZED;	/** @req NVM399 */
}

void NvM_MainFunction(void)
{

}

void NvM_ReadAll(void)
{

}

void NvM_WriteAll(void)
{
	
}

void NvM_CancelWriteAll(void)
{
	
}

void NvM_GetErrorStatus(NvM_BlockIdType BlockId, uint8 *RequestResultPtr)
{

}

#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)
void Nvm_SetRamBlockStatus(NvM_BlockIdType BlockId, boolean BlockChanged)
{
	
}
#endif
