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
/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.0.3 */

/* @req MemIf033 */
/* !req MemIf062 *//* No version check of included modules is done */
/* @req MemIf006 */
#include "MemIf.h"
/*lint -emacro(904,VALIDATE_RV,VALIDATE)*/ /*904 PC-Lint exception to MISRA 14.7 (validate DET macros)*/

#if (MEMIF_NUMBER_OF_DEVICES > 1)
#if (MEMIF_NUMBER_OF_DEVICES > 2)
#error "MemIf: Only 2 devices supported"
#endif

/* @req MemIf007 */
#if (MEMIF_DEV_ERROR_DETECT == STD_ON)
/* @req MemIf058 */
/* @req MemIf059 */
/* @req MemIf048 */
#include "Det.h"
#define VALIDATE_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
            (void)Det_ReportError(MEMIF_MODULE_ID, 0, _api, _err); \
            return (_rv); \
        }
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
            (void)Det_ReportError(MEMIF_MODULE_ID, 0, _api, _err); \
            return; \
        }

#else
#define VALIDATE_RV(_exp,_api,_err,_rv )
#define VALIDATE(_exp,_api,_err )
#endif /* MEMIF_DEV_ERROR_DETECT */

#define MEMIF_EA_DEVICE_INDEX 0u
#define MEMIF_FEE_DEVICE_INDEX 1u
#define MEMIF_NOF_SUPPORTED_DEVICES 2u
#define MEMIF_BROADCAST_DEVICE_ID   0xFFu
#define IS_VALID_DEVICE_ID(_x) ((_x) < MEMIF_NOF_SUPPORTED_DEVICES)
#define IS_BROADCAST_DEVICE_ID(_x) ((_x) == MEMIF_BROADCAST_DEVICE_ID)

#if (2 == MEMIF_NUMBER_OF_DEVICES)
/* @req MemIf020 */
/* @req MemIf047 */
typedef void (*SetModeFcnPtrType)(MemIf_ModeType Mode);
typedef Std_ReturnType (*ReadFcnPtrType)(uint16 BlockNumber, uint16 BlockOffset, uint8 *DataBufferPtr, uint16 Length);
typedef Std_ReturnType (*WriteFcnPtrType)(uint16 blockNumber, uint8* dataBufferPtr);
typedef void (*CancelFcnPtrType)(void);
typedef MemIf_StatusType (*GetStatusFcnPtrType)(void);
typedef MemIf_JobResultType (*GetJobResultFcnPtrType)(void);
typedef Std_ReturnType (*InvalidateBlockFcnPtrType)(uint16 blockNumber);
typedef Std_ReturnType (*EraseImmediateBlockFcnPtrType)(uint16 blockNumber);

typedef struct {
    const SetModeFcnPtrType             SetMode;
    const ReadFcnPtrType                Read;
    const WriteFcnPtrType               Write;
    const CancelFcnPtrType              Cancel;
    const GetStatusFcnPtrType           GetStatus;
    const GetJobResultFcnPtrType        GetJobResult;
    const InvalidateBlockFcnPtrType     InvalidateBlock;
    const EraseImmediateBlockFcnPtrType EraseImmediateBlock;
}MemIfDeviceAPIType;

const MemIfDeviceAPIType MemIfDevice[MEMIF_NOF_SUPPORTED_DEVICES] = {
        [MEMIF_EA_DEVICE_INDEX] = {
                .SetMode =              Ea_SetMode,
                .Read =                 Ea_Read,
                .Write =                Ea_Write,
                .Cancel =               Ea_Cancel,
                .GetStatus =            Ea_GetStatus,
                .GetJobResult =         Ea_GetJobResult,
                .InvalidateBlock =      Ea_InvalidateBlock,
                .EraseImmediateBlock =  Ea_EraseImmediateBlock
        },
        [MEMIF_FEE_DEVICE_INDEX] = {
                .SetMode =              Fee_SetMode,
                .Read =                 Fee_Read,
                .Write =                Fee_Write,
                .Cancel =               Fee_Cancel,
                .GetStatus =            Fee_GetStatus,
                .GetJobResult =         Fee_GetJobResult,
                .InvalidateBlock =      Fee_InvalidateBlock,
                .EraseImmediateBlock =  Fee_EraseImmediateBlock
        }
};
#endif

/**
 * Invokes the "GetStatus" function of the underlying memory abstraction module
 * selected by the parameter DeviceIndex
 * @param Mode
 */
void MemIf_SetMode(MemIf_ModeType Mode) {
    for(uint8 deviceIndex = 0; deviceIndex < MEMIF_NOF_SUPPORTED_DEVICES; deviceIndex++) {
        MemIfDevice[deviceIndex].SetMode(Mode);
    }
}

/**
 * Invokes the "Read" function of the underlying memory abstraction module selected
 * by the parameter DeviceIndex
 * @param DeviceIndex
 * @param BlockNumber
 * @param BlockOffset
 * @param DataBufferPtr
 * @param Length
 * @return  In case development error detection is enabled for the Memory Abstraction
 *          Interface and a development error is detected according to MemIf022 the
 *          function shall return E_NOT_OK else it shall return the value of the
 *          called function of the underlying module
 */
Std_ReturnType MemIf_Read(uint8 DeviceIndex, uint16 BlockNumber, uint16 BlockOffset, uint8 *DataBufferPtr, uint16 Length) {
    /* @req MemIf022 */
    /* @req MemIf023 */
    /* @req MemIf024 */
    VALIDATE_RV(IS_VALID_DEVICE_ID(DeviceIndex), MEMIF_READ_ID, MEMIF_E_PARAM_DEVICE, E_NOT_OK);
    VALIDATE_RV((NULL != DataBufferPtr), MEMIF_READ_ID, MEMIF_E_PARAM_POINTER, E_NOT_OK);
    return MemIfDevice[DeviceIndex].Read(BlockNumber, BlockOffset, DataBufferPtr, Length);
}

/**
 * Invokes the "Write" function of the underlying memory abstraction module selected
 * by the parameter DeviceIndex
 * @param DeviceIndex
 * @param BlockNumber
 * @param DataBufferPtr
 * @return  In case development error detection is enabled for the Memory Abstraction
 *          Interface and a development error is detected according to MemIf022 the
 *          function shall return E_NOT_OK else it shall return the value of the
 *          called function of the underlying module
 */
Std_ReturnType MemIf_Write(uint8 DeviceIndex, uint16 BlockNumber, uint8 *DataBufferPtr) {
    /* @req MemIf022 */
    /* @req MemIf023 */
    /* @req MemIf024 */
    VALIDATE_RV(IS_VALID_DEVICE_ID(DeviceIndex), MEMIF_WRITE_ID, MEMIF_E_PARAM_DEVICE, E_NOT_OK);
    VALIDATE_RV((NULL != DataBufferPtr), MEMIF_WRITE_ID, MEMIF_E_PARAM_POINTER, E_NOT_OK);
    return MemIfDevice[DeviceIndex].Write(BlockNumber, DataBufferPtr);
}

/**
 * Invokes the "Cancel" function of the underlying memory abstraction module
 * selected by the parameter DeviceIndex
 * @param DeviceIndex
 */
void MemIf_Cancel(uint8 DeviceIndex) {
    /* @req MemIf022 */
    /* @req MemIf023 */
    /* @req MemIf024 */
    VALIDATE(IS_VALID_DEVICE_ID(DeviceIndex), MEMIF_CANCEL_ID, MEMIF_E_PARAM_DEVICE);
    MemIfDevice[DeviceIndex].Cancel();
}

/**
 * Invokes the "GetStatus" function of the underlying memory abstraction module
 * selected by the parameter DeviceIndex
 * @param DeviceIndex
 * @return See MemIf035
 */
MemIf_StatusType MemIf_GetStatus(uint8 DeviceIndex) {
    /* @req MemIf022 */
    /* @req MemIf023 */
    /* @req MemIf024 */
    MemIf_StatusType totalStatus = MEMIF_IDLE;
    MemIf_StatusType deviceStatus;
    VALIDATE_RV((IS_VALID_DEVICE_ID(DeviceIndex) || IS_BROADCAST_DEVICE_ID(DeviceIndex)), MEMIF_GETSTATUS_ID, MEMIF_E_PARAM_DEVICE, MEMIF_UNINIT);
    if(IS_BROADCAST_DEVICE_ID(DeviceIndex)) {
        for(uint8 devIdx = 0; devIdx < MEMIF_NOF_SUPPORTED_DEVICES; devIdx++) {
            /* @req MemIf035 */
            deviceStatus = MemIfDevice[devIdx].GetStatus();
            if( (MEMIF_UNINIT == deviceStatus) || (MEMIF_UNINIT == totalStatus) ) {
                totalStatus = MEMIF_UNINIT;
            } else if( (MEMIF_BUSY == deviceStatus) || (MEMIF_BUSY == totalStatus) ) {
                totalStatus = MEMIF_BUSY;
            } else if( MEMIF_BUSY_INTERNAL == deviceStatus ) {
                totalStatus = MEMIF_BUSY_INTERNAL;
            }
        }
    } else {
        totalStatus = MemIfDevice[DeviceIndex].GetStatus();
    }
    return totalStatus;
}

/**
 * Invokes the "GetJobResult" function of the underlying memory abstraction module
 * selected by the parameter DeviceIndex.
 * @param DeviceIndex
 * @param BlockNumber
 * @param BlockOffset
 * @param DataBufferPtr
 * @param Length
 * @return  In case development error detection is enabled for the
 *          Memory Abstraction Interface and a development error is detected
 *          according to MemIf022 the function shall return E_JOB_FAILED else
 *          it shall return the value of the called function of the underlying module
 */
MemIf_JobResultType MemIf_GetJobResult(uint8 DeviceIndex) {
    /* @req MemIf022 */
    /* @req MemIf023 */
    /* @req MemIf024 */
    VALIDATE_RV(IS_VALID_DEVICE_ID(DeviceIndex), MEMIF_GETJOBRESULT_ID, MEMIF_E_PARAM_DEVICE, MEMIF_JOB_FAILED);
    return MemIfDevice[DeviceIndex].GetJobResult();
}

/**
 * Invokes the "InvalidateBlock" function of the underlying memory abstraction
 * module selected by the parameter DeviceIndex
 * @param DeviceIndex
 * @param BlockNumber
 * @return  In case development error detection is enabled for the Memory
 *          Abstraction Interface and a development error is detected according
 *          to MemIf022 the function shall return E_NOT_OK else it shall return
 *          the value of the called function of the underlying module
 */
Std_ReturnType MemIf_InvalidateBlock(uint8 DeviceIndex, uint16 BlockNumber) {
    /* @req MemIf022 */
    /* @req MemIf023 */
    /* @req MemIf024 */
    VALIDATE_RV((IS_VALID_DEVICE_ID(DeviceIndex)), MEMIF_INVALIDATEBLOCK_ID, MEMIF_E_PARAM_DEVICE, E_NOT_OK);
    return MemIfDevice[DeviceIndex].InvalidateBlock(BlockNumber);
}

/**
 * Invokes the "EraseImmediateBlock" function of the underlying memory abstraction
 * module selected by the parameter DeviceIndex
 * @param DeviceIndex
 * @param BlockNumber
 * @return  In case devlopment error detection is enabled for the Memory Abstraction
 *          Interface and a development error is detected according to MemIf022 the
 *          function shall return E_NOT_OK else it shall return the value of the
 *          called function of the underlying module
 */
Std_ReturnType MemIf_EraseImmediateBlock(uint8 DeviceIndex, uint16 BlockNumber) {
    /* @req MemIf022 */
    /* @req MemIf023 */
    /* @req MemIf024 */
    VALIDATE_RV(IS_VALID_DEVICE_ID(DeviceIndex), MEMIF_ERASEIMMEDIATEBLOCK_ID, MEMIF_E_PARAM_DEVICE, E_NOT_OK);
    return MemIfDevice[DeviceIndex].EraseImmediateBlock(BlockNumber);
}

#endif

