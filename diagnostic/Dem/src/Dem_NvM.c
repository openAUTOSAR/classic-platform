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


#include "Dem_NvM.h"
#if defined(USE_NVM)
#include "NvM.h"
#endif

/* Local defines */
#define DEM_NO_NVM_BLOCK 0u

/* Local types */
#if defined(NVM_NOT_SERVICE_COMPONENT) && defined(USE_RTE) && defined(USE_NVM) && (DEM_USE_NVM == STD_ON)
/* NvM_BlockIdType is defined in the RTE when using NvM service component or in NvM when not using rte
 * If neither is fulfilled, typedef it here since used by DEM
 */
typedef uint16 NvM_BlockIdType;
#endif

typedef enum {
    DEM_NVMBLOCK_PRIMARY_EVENT = 0,
    DEM_NVMBLOCK_PRIMARY_FF,
    DEM_NVMBLOCK_PRIMARY_EXTDATA,
    DEM_NVMBLOCK_SECONDARY_EVENT,
    DEM_NVMBLOCK_SECONDARY_FF,
    DEM_NVMBLOCK_SECONDARY_EXTDATA,
    DEM_NVMBLOCK_INDICATORS,
    DEM_NVMBLOCK_STATUSBIT,
    DEM_NVMBLOCK_PERMANENT,
	DEM_NVMBLOCK_PRESTORED_FF,
	DEM_NVMBLOCK_IUMPR,
#if defined(USE_NVM) && (DEM_USE_NVM == STD_ON)
    DEM_NOF_SUPPORTED_NVM_BLOCKS
#endif
}DemNvMBlockIdType;

#if defined(USE_NVM) && (DEM_USE_NVM == STD_ON)
typedef enum {
    DEM_NVM_IDLE = 0,
    DEM_NVM_SETRAMBLOCKSTATUS,
    DEM_NVM_WRITEBLOCK,
    DEM_NVM_WAIT_DONE
}DemNvMStateType;

typedef struct {
    DemNvMStateType State;
    boolean Dirty;
    boolean PendingWrite;
}DemNvMBlockStatusType;

typedef struct {
    NvM_BlockIdType BlockId;
}DemNvMBlockCfgType;
#endif

/* Local function prototypes */
static void setNvMBlockChanged(DemNvMBlockIdType blockId, boolean ImmediateStorage);

/* Local constants */
#if defined(USE_NVM) && (DEM_USE_NVM == STD_ON)
const DemNvMBlockCfgType DemNvMBlockConfig[DEM_NOF_SUPPORTED_NVM_BLOCKS] = {
    [DEM_NVMBLOCK_PRIMARY_EVENT] = {
        .BlockId = DEM_EVENT_PRIMARY_NVM_BLOCK_HANDLE
    },
    [DEM_NVMBLOCK_PRIMARY_FF] = {
        .BlockId = DEM_FREEZE_FRAME_PRIMARY_NVM_BLOCK_HANDLE
    },
    [DEM_NVMBLOCK_PRIMARY_EXTDATA] = {
        .BlockId = DEM_EXTENDED_DATA_PRIMARY_NVM_BLOCK_HANDLE
    },
    [DEM_NVMBLOCK_SECONDARY_EVENT] = {
        .BlockId = DEM_EVENT_SECONDARY_NVM_BLOCK_HANDLE
    },
    [DEM_NVMBLOCK_SECONDARY_FF] = {
        .BlockId = DEM_FREEZE_FRAME_SECONDARY_NVM_BLOCK_HANDLE
    },
    [DEM_NVMBLOCK_SECONDARY_EXTDATA] = {
        .BlockId = DEM_EXTENDED_DATA_SECONDARY_NVM_BLOCK_HANDLE
    },
    [DEM_NVMBLOCK_INDICATORS] = {
        .BlockId = DEM_INDICATOR_NVM_BLOCK_HANDLE
    },
    [DEM_NVMBLOCK_STATUSBIT] = {
        .BlockId = DEM_STATUSBIT_NVM_BLOCK_HANDLE
    },
    [DEM_NVMBLOCK_PERMANENT] = {
        .BlockId = DEM_PERMANENT_NVM_BLOCK_HANDLE
    },
    [DEM_NVMBLOCK_PRESTORED_FF] = {
        .BlockId = DEM_PRESTORE_FF_NVM_BLOCK_HANDLE
    },
	[DEM_NVMBLOCK_IUMPR] = {
		.BlockId = DEM_PERMANENT_NVM_BLOCK_HANDLE
	}
};
/* Local variables */
static DemNvMBlockStatusType DemNvMBlockStatus[DEM_NOF_SUPPORTED_NVM_BLOCKS];
#endif

/* Local functions */
/**
 *
 * @param blockId
 * @param ImmediateStorage
 */
/*lint --e{522} CONFIGURATION */
static void setNvMBlockChanged(DemNvMBlockIdType blockId, boolean ImmediateStorage)
{
#if defined(USE_NVM) && (DEM_USE_NVM == STD_ON)
    if( DEM_NO_NVM_BLOCK != DemNvMBlockConfig[blockId].BlockId ) {
        DemNvMBlockStatus[blockId].Dirty = TRUE;
        if( TRUE == ImmediateStorage ) {
            DemNvMBlockStatus[blockId].PendingWrite = TRUE;
        }
    }
#else
    (void)blockId;
    (void)ImmediateStorage;
#endif
}

/* Exported functions */
/**
 *
 * @param Origin
 * @param ImmediateStorage
 */
void Dem_NvM_SetEventBlockChanged(Dem_DTCOriginType Origin, boolean ImmediateStorage)
{
    switch(Origin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
            setNvMBlockChanged(DEM_NVMBLOCK_PRIMARY_EVENT, ImmediateStorage);
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
            setNvMBlockChanged(DEM_NVMBLOCK_SECONDARY_EVENT, ImmediateStorage);
            break;
        default:
            break;
    }
}

/**
 *
 * @param Origin
 * @param ImmediateStorage
 */
void Dem_NvM_SetFreezeFrameBlockChanged(Dem_DTCOriginType Origin, boolean ImmediateStorage)
{
    switch(Origin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
            setNvMBlockChanged(DEM_NVMBLOCK_PRIMARY_FF, ImmediateStorage);
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
            setNvMBlockChanged(DEM_NVMBLOCK_SECONDARY_FF, ImmediateStorage);
            break;
        default:
            break;
    }
}

void Dem_NvM_SetPreStoreFreezeFrameBlockChanged(boolean ImmediateStorage)
{
	setNvMBlockChanged(DEM_NVMBLOCK_PRESTORED_FF, ImmediateStorage);
}

/**
 *
 * @param Origin
 * @param ImmediateStorage
 */
void Dem_NvM_SetExtendedDataBlockChanged(Dem_DTCOriginType Origin, boolean ImmediateStorage)
{
    switch(Origin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
            setNvMBlockChanged(DEM_NVMBLOCK_PRIMARY_EXTDATA, ImmediateStorage);
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
            setNvMBlockChanged(DEM_NVMBLOCK_SECONDARY_EXTDATA, ImmediateStorage);
            break;
        default:
            break;
    }
}

/**
 *
 * @param ImmediateStorage
 */
void Dem_NvM_SetIndicatorBlockChanged(boolean ImmediateStorage)
{
    setNvMBlockChanged(DEM_NVMBLOCK_INDICATORS, ImmediateStorage);
}

/**
 *
 * @param ImmediateStorage
 */
void Dem_NvM_SetStatusBitSubsetBlockChanged(boolean ImmediateStorage)
{
    setNvMBlockChanged(DEM_NVMBLOCK_STATUSBIT, ImmediateStorage);
}

/**
 *
 * @param ImmediateStorage
 */
void Dem_NvM_SetPermanentBlockChanged(boolean ImmediateStorage)
{
    setNvMBlockChanged(DEM_NVMBLOCK_PERMANENT, ImmediateStorage);
}

/**
 *
 * @param ImmediateStorage
 */
void Dem_Nvm_SetIumprBlockChanged(boolean ImmediateStorage)
{
	setNvMBlockChanged(DEM_NVMBLOCK_IUMPR, ImmediateStorage);
}

/**
 * Init function for Dem_NvM
 */
void Dem_NvM_Init(void)
{
#if defined(USE_NVM) && (DEM_USE_NVM == STD_ON)
    for( uint8 blockId = 0u; blockId < (uint8)DEM_NOF_SUPPORTED_NVM_BLOCKS; blockId++ ) {
        DemNvMBlockStatus[blockId].Dirty = FALSE;
        DemNvMBlockStatus[blockId].PendingWrite = FALSE;
        DemNvMBlockStatus[blockId].State = DEM_NVM_IDLE;
    }
#endif
}

/**
 * Main function for Dem_NvM. Should be called periodically.
 */
void Dem_NvM_MainFunction(void)
{
    /* !req DEM579 *//* No NOT ignore return values. Try again. Although the actual result of the write job */
    /* @req DEM164 *//* Only NvM_WriteBlock is used. */
    /* @req DEM329 */
#if defined(USE_NVM) && (DEM_USE_NVM == STD_ON)
    NvM_RequestResultType reqRes;
    for( uint8 blockId = 0u; blockId < (uint8)DEM_NOF_SUPPORTED_NVM_BLOCKS; blockId++ ) {
        /* Check if any pending write job */
        if( (DEM_NVM_IDLE == DemNvMBlockStatus[blockId].State) &&
                ( (TRUE == DemNvMBlockStatus[blockId].PendingWrite) || (TRUE == DemNvMBlockStatus[blockId].Dirty) ) ) {
#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)
            DemNvMBlockStatus[blockId].State = DEM_NVM_SETRAMBLOCKSTATUS;
#else
            DemNvMBlockStatus[blockId].State = DEM_NVM_WRITEBLOCK;
#endif
        }

        switch(DemNvMBlockStatus[blockId].State) {
            case DEM_NVM_SETRAMBLOCKSTATUS:
#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)
                if(E_OK == NvM_GetErrorStatus(DemNvMBlockConfig[blockId].BlockId, &reqRes)) {
                    if(NVM_REQ_PENDING != reqRes) {
                        if( E_OK == NvM_SetRamBlockStatus(DemNvMBlockConfig[blockId].BlockId, TRUE) ) {
                            DemNvMBlockStatus[blockId].Dirty = FALSE;
                            if(TRUE == DemNvMBlockStatus[blockId].PendingWrite) {
                                DemNvMBlockStatus[blockId].State = DEM_NVM_WRITEBLOCK;
                            }
                            else {
                                DemNvMBlockStatus[blockId].State = DEM_NVM_WAIT_DONE;
                            }
                        }
                    }
                }
#endif
                break;
            case DEM_NVM_WRITEBLOCK:
                if(E_OK == NvM_GetErrorStatus(DemNvMBlockConfig[blockId].BlockId, &reqRes)) {
                    if(NVM_REQ_PENDING != reqRes) {
                        if( E_OK == NvM_WriteBlock(DemNvMBlockConfig[blockId].BlockId, NULL) ) {
                            DemNvMBlockStatus[blockId].PendingWrite = FALSE;
                            DemNvMBlockStatus[blockId].Dirty = FALSE;
                            DemNvMBlockStatus[blockId].State = DEM_NVM_WAIT_DONE;
                        }
                    }
                }
                break;
            case DEM_NVM_WAIT_DONE:
                if(E_OK == NvM_GetErrorStatus(DemNvMBlockConfig[blockId].BlockId, &reqRes)) {
                    if(NVM_REQ_PENDING != reqRes) {
                        /* Write job done */
                        DemNvMBlockStatus[blockId].State = DEM_NVM_IDLE;
                    }
                }
                break;
            default:
                break;
        }
    }
#endif
}
