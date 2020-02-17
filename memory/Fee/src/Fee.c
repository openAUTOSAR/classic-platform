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


/*
 * Author: pete+maho
 *
 * Part of Release:
 *   4.0.3
 *
 * Description:
 *   Implements the Fee module
 *
 * Support:
 *   General                  Have Support
 *   -------------------------------------------
 *   FEE_DEV_ERROR_DETECT            Y
 *   FeeIndex                        N (always 0)
 *   FEE_NVM_JOB_END_NOTIFICATION    Y (Under ArcCore FEE_USE_JOB_NOTIFICATIONS is used)
 *   FEE_NVM_JOB_ERROR_NOTIFICATION  Y (Under ArcCore FEE_USE_JOB_NOTIFICATIONS is used)
 *   FEE_POLLING_MODE                Y
 *   FEE_VERSION_INFO_API            Y
 *   FEE_VIRTUAL_PAGE_SIZE           Y
 *
 *   Block                    Have Support
 *   -------------------------------------------
 *   FeeBlockNumber                 Y
 *   FeeBlockSize                   Y
 *   FeeImmediateData               N
 *   FeeNumberOfWriteCycles         N
 *   FeeDeviceIndex                 N
 *
 *   Published Information       Have Support
 *   -------------------------------------------
 *   FEE_BLOCK_OVERHEAD             N
 *   FEE_MAXIMUM_BLOCKING_TIME      N
 *   FEE_PAGE_OVERHEAD              N
 *
 * Implementation Notes:
 *   "Corrupt block"
 *      The FEE and NVM specifications are very fussy when it comes to "corrupt"
 *      blocks. The FEE says when a write operation starts that the block should
 *      be marked as corrupt and when it's done to become "not corrupt" again.
 *      These requirements seems to relate to flash with very small blocks that store
 *      block information in the same place in flash. This FEE driver instead writes
 *      in a new place in flash and keeps track of latest valid block. The driver will
 *      always use the latest valid block. So when will it return what?
 *
 *
 *      MemIf                       Description
 *      --------------------------------------------------------------------------------
 *      MEMIF_BLOCK_INCONSISTENT    Virgin start
 *      MEMIF_BLOCK_INVALID
 *
 *      So that means that the first read must check the
 *
 *
 */

/* General requirements */
/* @req FEE049 *//* BlockDataAddress used for marking block as corrupted */
/* @req FEE071 *//* Logical blocks must not overlap each other and must not be contained within one another. */
/* @req FEE084 *//* Imported types */
/* !req FEE105 *//* Mandatory interface, Fls_Cancel and Fls_Compare not used */
/* @req FEE167 *//* VARIANT_PRE_COMPILE */

/* ----------------------------[includes]------------------------------------*/

#include "Fee.h"
#include "Fee_Cbk.h"
#include "Fee_Memory_Cfg.h"
#include "Fls.h"
#if defined(USE_DEM)
/* @req FEE060 *//* Although not needed since Fee has no production errors */
#include "Dem.h"
#endif
#include <string.h>
#include "MemMap.h"
#include "Cpu.h"
#include "SchM_Fee.h"

/* The FLS_ERASED_VALIUE is not published by some MCALs,
 * this is a way around this.
 */
#if !defined(FLS_ERASED_VALUE)
#define FLS_ERASED_VALUE 0xFF
#endif



/* ----------------------------[private define]------------------------------*/
/*lint -emacro(904,DET_VALIDATE,DET_VALIDATE_RV) *//*904 PC-Lint exception to MISRA 14.7 (validate DET macros).*/


//#define DEBUG_FEE    1
#if defined(DEBUG_FEE)
#define DEBUG_PREFIX    "## "

#define DEBUG_PRINTF(format,...)                    printf( DEBUG_PREFIX format,## __VA_ARGS__ );
#define FLS_READ(_fls_addr,_target_addr,_length)    \
        ({\
            Std_ReturnType _rv; \
            /*printf( DEBUG_PREFIX "%s: Fls_Read(): 0x%x, %d\n",__FUNCTION__,_fls_addr,_length); */ \
            _rv = Fls_Read(_fls_addr,_target_addr,_length); \
            _rv; \
        })

#define FLS_WRITE(_fls_addr,_source_addr,_length)    \
        ({\
            Std_ReturnType _rv; \
            /* printf( DEBUG_PREFIX "%s: Fls_Write(): 0x%x 0x%x, %d\n",__FUNCTION__,_fls_addr,_source_addr,_length); */ \
            _rv = Fls_Write(_fls_addr,_source_addr,_length); \
            _rv; \
        })

#else
#define DEBUG_PRINTF(format,...)
#define FLS_READ(_fls_addr,_target_addr,_length)    Fls_Read(_fls_addr,_target_addr,_length)
#define FLS_WRITE(_fls_addr,_target_addr,_length)   Fls_Write(_fls_addr,_target_addr,_length)
#endif


#define INVALIDATED_BLOCK                0xfffffffdUL
#define BLOCK_CORRUPT                    0xffffffffUL

#define NUM_OF_BANKS                    2u
#define FORCED_GARBAGE_COLLECT_MARGIN   0u
#define BANK_COUNTER_INVALID            0x0
#define BANK_COUNTER_EMPTY              0xFF


#define BANK_CTRL_SIZE                  PAGE_ALIGN(sizeof(FlsBankControlDataType))
#define BLOCK_CTRL_DATA_PAGE_SIZE       PAGE_ALIGN(sizeof(FlsBlockCtrlDataType))
#define ADMIN_SIZE                      PAGE_ALIGN(sizeof(FlsBlockControlType))
#define BLOCK_CTRL_ID_PAGE_SIZE         PAGE_ALIGN(sizeof(FlsBlockCtrlIdType))
#define RWBUFFER_SIZE                   sizeof(ReadWriteBufferType)

#define MAX_NOF_FAILED_WRITE_ATTEMPTS       5
#define MAX_NOF_FAILED_GC_ATTEMPTS          5
#define MAX_NOF_FAILED_STARTUP_ATTEMPTS     2


/* ----------------------------[private macro]-------------------------------*/

#define NEXT_BANK_IDX(var)                      (((var) >= (NUM_OF_BANKS -1))? 0 : ((var) + 1))
#define NEXT_BANK_COUNTERVAL(var)               (((var) == 0xFE)? 1 : ((var) + 1))
#define IS_ADDRESS_WITHIN_BANK(addr, bank)      ((addr >= BankProp[bank].Start) && (addr < BankProp[bank].End))

/* @req FEE011 */
#if  ( FEE_DEV_ERROR_DETECT == STD_ON )
/* @req FEE045 */
/* @req FEE062 */
/* @req FEE104 */
#include "Det.h"
#define DET_VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
            (void)Det_ReportError(FEE_MODULE_ID, 0, _api, _err); \
            return; \
        }

#define DET_VALIDATE_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
            (void)Det_ReportError(FEE_MODULE_ID, 0, _api, _err); \
            return _rv; \
        }

#define DET_VALIDATE_NO_RV(_exp,_api,_err ) \
        if( !(_exp) ) { \
            (void)Det_ReportError(FEE_MODULE_ID, 0, _api, _err); \
            return; \
        }

#define DET_REPORTERROR(_api,_err) (void)Det_ReportError(FEE_MODULE_ID,0,(_api),(_err))
#else
#define DET_VALIDATE(_exp,_api,_err )
#define DET_VALIDATE_RV(_exp,_api,_err,_rv )
#define DET_VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_api,_err)
#endif

/*
 * Page alignment macros
 */
#define PAGE_ALIGN(var)    ((((uint32)(var) + FEE_VIRTUAL_PAGE_SIZE - 1u) / FEE_VIRTUAL_PAGE_SIZE) * FEE_VIRTUAL_PAGE_SIZE)


#define SET_FEE_STATUS( _status )    ModuleStatus = (_status); DEBUG_PRINTF("-- SetStatus=%s (%s)\n",memIfStatusToStr[_status],__FUNCTION__);
#define SET_FEE_JOBRESULT( _status ) JobResult = (_status); DEBUG_PRINTF("-- SetJobResult=%s (%s)\n",memIfJobToStr[_status],__FUNCTION__);


/* ----------------------------[private typedef]-----------------------------*/


typedef struct {
    Fls_AddressType Start;
    Fls_LengthType  End;
} BankPropType;

typedef uint32 BankMagicType;

typedef uint8 FlsBankCounterType;

typedef struct {
    BankMagicType       Magic;
    FlsBankCounterType  BankCounter;
    FlsBankCounterType  InvBankCounter;
} FlsBankControlDataType;

typedef union {
    FlsBankControlDataType Data;
    uint8        Byte[BANK_CTRL_SIZE];
} FlsBankControlType;

typedef struct {
    uint16                BlockDataLength; // == 0 if invalidated
    uint16                InvBlockDataLength;
    Fls_AddressType        BlockDataAddress;
} FlsBlockCtrlDataType;


typedef union {
    FlsBlockCtrlDataType    Data;
    uint8                    Byte[BLOCK_CTRL_DATA_PAGE_SIZE];
} FlsBlockCtrlDataPageType;

typedef uint32 BlockMagicType;

typedef struct {
    BlockMagicType        Magic;
    uint16                BlockNo;
    uint16                InvBlockNo;
} FlsBlockCtrlIdType;


typedef union {
    FlsBlockCtrlIdType    Data;
    uint8                Byte[BLOCK_CTRL_ID_PAGE_SIZE];
} FlsBlockCtrlIdPageType;

typedef struct {
    FlsBlockCtrlDataPageType    DataPage;
    FlsBlockCtrlIdPageType        IdPage;
} FlsBlockControlType;


typedef union {
    FlsBlockControlType    BlockCtrl;
    FlsBankControlType     BankCtrl;
    uint8 Byte[ADMIN_SIZE];
} ReadWriteBufferType;

/*
 * Variables for flash administration
 */
typedef struct {
    Fls_AddressType        BlockDataAddress;
} AdminFlsBlockType;



/*
 * Variables for the current job
 */
typedef enum {
    FEE_UNINITIALIZED = 0,

    FEE_STARTUP_REQUESTED, // setup startup
    FEE_STARTUP_READ_BANK_HEADER, // request bank header read
    FEE_STARTUP_READ_BANK_HEADER_WAIT, // wait band header read done
    FEE_STARTUP_READ_BLOCK_ADMIN,  // request admin block read
    FEE_STARTUP_READ_BLOCK_ADMIN_WAIT, // wait admin block read done

    FEE_IDLE,

    FEE_READ, // request data read
    FEE_READ_WAIT, // wait data read done

    FEE_WRITE,
    FEE_WRITE_ADMIN_DATA, // request admin block write
    FEE_WRITE_ADMIN_DATA_WAIT, // wait admin block write
    FEE_WRITE_DATA, // request data write
    FEE_WRITE_DATA_WAIT, // // wait data write done
    FEE_WRITE_ADMIN_ID, // request write of "data write done"
    FEE_WRITE_ADMIN_ID_WAIT, // wait write of "data write done" done
    FEE_WRITE_CHECK_ADMIN_DATA, // write failure of admin data, read it back to check status
    FEE_WRITE_CHECK_ADMIN_DATA_WAIT, // wait for admin data to check status

    FEE_ERASE_BANK, // request bank erase
    FEE_ERASE_BANK_WAIT, // wait bank erase done
    FEE_WRITE_BANK_HEADER, // write bank erase done in bank header
    FEE_WRITE_BANK_HEADER_WAIT, // wait write of bank erase done

    FEE_GARBAGE_COLLECT_WRITE_ADMIN_DATA, // request admin block write
    FEE_GARBAGE_COLLECT_WRITE_ADMIN_DATA_WAIT, // wait admin block write
    FEE_GARBAGE_COLLECT_READ, // read all or rwbuffersize bytes of data from old bank
    FEE_GARBAGE_COLLECT_READ_WAIT, // wait for read done
    FEE_GARBAGE_COLLECT_WRITE_DATA, // request data write
    FEE_GARBAGE_COLLECT_WRITE_DATA_WAIT, // // wait data write done
    FEE_GARBAGE_COLLECT_WRITE_ADMIN_ID, // request write of "data write done"
    FEE_GARBAGE_COLLECT_WRITE_ADMIN_ID_WAIT, // wait write of "data write done" done
    FEE_GARBAGE_COLLECT_WRITE_CHECK_ADMIN_DATA, // write failure of admin data, read it back to check status
    FEE_GARBAGE_COLLECT_WRITE_CHECK_ADMIN_DATA_WAIT, // wait for admin data to check status

    FEE_CORRUPTED

} CurrentJobStateType;

enum {
    NO_JOB = 0,
    PENDING_READ_JOB = 1,
    PENDING_WRITE_JOB = 2,
    PENDING_ERASE_JOB = 4,
    PENDING_FORCED_GARBAGE_COLLECT_JOB = 8,
    PENDING_GARBAGE_COLLECT_JOB = 16,
}pendingJobEnumType;


typedef uint8 PendingJobType;

typedef struct {
    CurrentJobStateType    State;
    PendingJobType         pendingJob; // pending job flags
    uint8                FailCounter;
    AdminFlsBlockType    BlockDescrTbl[FEE_NUM_OF_BLOCKS];
    struct {
        Fls_AddressType        NewBlockAdminAddress; // ptr to next free admin block
        Fls_AddressType        NewBlockDataAddress; // ptr to next free data storage
        uint8                BankIdx;
    }Write;
    struct {
        uint32    TotalFeeSize;
        uint16    Offset; // garbageCollect data offset
        uint16    BlockIdx; // current garbage collect block index
        uint8    BankIdx; // current garbage collect bank index
    } GarbageCollect;
    struct {
        uint8    BankIdx; // current erase bank index
        uint8    NextBankCounter;
    }Erase;
} AdminFlsType;


static AdminFlsType AdminFls = {
        .State = FEE_UNINITIALIZED,
};

typedef union {
    struct {
        uint8            *DataPtr; // read ram buffer ptr
        uint16            BlockIdx; // read block index
        uint16            Length; // read block length
        uint16          DataOffset; // read data offset
    } Read;
    struct {
        uint8            *DataPtr; // write ram buffer ptr
        uint16            BlockIdx; // write block index
        uint8            Invalidate; // invalidate flag
    }Write;
    struct {
        Fls_AddressType        BlockAdminAddress;
        Fls_AddressType        BlockDataAddress;
        FlsBankCounterType    BankStatus[NUM_OF_BANKS];
        uint8                BankIdx;
        uint8                MinBankIdx;
    }Startup;
} CurrentJobType;


/* ----------------------------[private function prototypes]-----------------*/
void StartupRequested(void);
/* ----------------------------[private variables]---------------------------*/


#if defined(DEBUG_FEE)
#define STR_ENTRY(_x)    [_x] = #_x

static const char *stateToStr[] = {
        STR_ENTRY(FEE_UNINITIALIZED), STR_ENTRY(FEE_STARTUP_REQUESTED),
        STR_ENTRY( FEE_STARTUP_READ_BANK_HEADER), STR_ENTRY( FEE_STARTUP_READ_BANK_HEADER_WAIT),
        STR_ENTRY( FEE_STARTUP_READ_BLOCK_ADMIN),
        STR_ENTRY( FEE_STARTUP_READ_BLOCK_ADMIN_WAIT),
        STR_ENTRY( FEE_IDLE),
        STR_ENTRY( FEE_READ), STR_ENTRY( FEE_READ_WAIT),
        STR_ENTRY( FEE_WRITE), STR_ENTRY( FEE_WRITE_ADMIN_DATA), STR_ENTRY( FEE_WRITE_ADMIN_DATA_WAIT),
        STR_ENTRY( FEE_WRITE_DATA),STR_ENTRY( FEE_WRITE_DATA_WAIT),
        STR_ENTRY( FEE_WRITE_ADMIN_ID), STR_ENTRY( FEE_WRITE_ADMIN_ID_WAIT),
        STR_ENTRY( FEE_WRITE_CHECK_ADMIN_DATA), STR_ENTRY( FEE_WRITE_CHECK_ADMIN_DATA_WAIT),
        STR_ENTRY( FEE_ERASE_BANK), STR_ENTRY( FEE_ERASE_BANK_WAIT), STR_ENTRY( FEE_WRITE_BANK_HEADER),
        STR_ENTRY( FEE_WRITE_BANK_HEADER_WAIT),
        STR_ENTRY( FEE_GARBAGE_COLLECT_WRITE_ADMIN_DATA), STR_ENTRY( FEE_GARBAGE_COLLECT_WRITE_ADMIN_DATA_WAIT),
        STR_ENTRY( FEE_GARBAGE_COLLECT_READ), STR_ENTRY( FEE_GARBAGE_COLLECT_READ_WAIT),
        STR_ENTRY( FEE_GARBAGE_COLLECT_WRITE_DATA),    STR_ENTRY( FEE_GARBAGE_COLLECT_WRITE_DATA_WAIT),
        STR_ENTRY( FEE_GARBAGE_COLLECT_WRITE_ADMIN_ID),    STR_ENTRY( FEE_GARBAGE_COLLECT_WRITE_ADMIN_ID_WAIT),
        STR_ENTRY( FEE_GARBAGE_COLLECT_WRITE_CHECK_ADMIN_DATA),
        STR_ENTRY( FEE_GARBAGE_COLLECT_WRITE_CHECK_ADMIN_DATA_WAIT),
        STR_ENTRY(FEE_CORRUPTED)
};

static const char *memIfJobToStr[] = {
    STR_ENTRY(MEMIF_JOB_OK), STR_ENTRY(MEMIF_JOB_FAILED),
    STR_ENTRY(MEMIF_JOB_PENDING), STR_ENTRY(MEMIF_JOB_CANCELED),
    STR_ENTRY(MEMIF_BLOCK_INCONSISTENT), STR_ENTRY(MEMIF_BLOCK_INVALID),
};

static const char *memIfStatusToStr[] = {
    STR_ENTRY(MEMIF_UNINIT), STR_ENTRY(MEMIF_IDLE),
    STR_ENTRY(MEMIF_BUSY), STR_ENTRY(MEMIF_BUSY_INTERNAL),
};

#endif


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


static const BankMagicType BankMagicMaster         = 0x2345babe; // compatibility id
static const BlockMagicType BlockMagicMaster     = 0xebbababeUL;
static ReadWriteBufferType RWBuffer; /*lint !e9018 Union usage is required so it is considered as lint exception   */


/*
 * Variables for quick reporting of status and job result
 */
static MemIf_StatusType     ModuleStatus = MEMIF_UNINIT;
static MemIf_JobResultType     JobResult = MEMIF_JOB_OK;
static CurrentJobType         CurrentJob; /*lint !e9018 Union usage is required so it is considered as lint exception   */


/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/






static uint16 GetBlockIdxFromBlockNumber(uint16 blockNumber) {
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
#define SetFlsJobBusy()            /* Nothing needs to be done here */

static boolean CheckFlsJobFinished(void)
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

static boolean CheckFlsJobFinished(void)
{
    return (FlsJobReady);
}

#endif

static uint32 BlankCheck(const uint8 *data, uint8_t val, size_t size) {
    uint32 retVal = 0;
    for(size_t i = 0; i < size; i++) {
        if(data[i] != val) {
            retVal = (uint32)i+1;
            break;
        }
    }
    return retVal;
}

static void AbortStartup(MemIf_JobResultType result)
{
    (void)result; /* Avoid compiler warning - remove it ?*/
    AdminFls.FailCounter++;
    if(AdminFls.FailCounter >= MAX_NOF_FAILED_STARTUP_ATTEMPTS) {
        DET_REPORTERROR(FEE_STARTUP_ID, FEE_FLASH_CORRUPT);
        AdminFls.State = FEE_CORRUPTED;
        SET_FEE_STATUS(MEMIF_IDLE);
    } else {
        AdminFls.State = FEE_STARTUP_REQUESTED;
    }
}

static void FinishStartup(void) {
    AdminFls.FailCounter = 0;
    AdminFls.State = FEE_IDLE;
    SET_FEE_STATUS(MEMIF_IDLE);/* @req FEE169 */
}

static void FinishJob(MemIf_JobResultType jobResult)
{
    /* !req FEE155 *//* Failed garbage collect will affect return from Fee_GetJobResult */
    AdminFls.FailCounter = 0;
    AdminFls.State = FEE_IDLE;
    SET_FEE_STATUS(MEMIF_IDLE);
    SET_FEE_JOBRESULT(jobResult);
    if(jobResult == MEMIF_JOB_OK) {
        /* !req FEE055 *//* Read job OK, write job OK or invalidate job OK */
        /* Parts of FEE055 but we do not support erase for
         * immediate data.. */
        if (Fee_Config.General.NvmJobEndCallbackNotificationCallback != NULL) {
            Fee_Config.General.NvmJobEndCallbackNotificationCallback();
        }
    } else {
        /* !req FEE056 *//* Read job failed, write job failed or invalidate job failed*/
        /* Part of FEE056 but we do not support erase for
         * immediate data.. */
        if (Fee_Config.General.NvmJobErrorCallbackNotificationCallback != NULL) {
            Fee_Config.General.NvmJobErrorCallbackNotificationCallback();
        }
    }
}

inline static void IncrementFlsState(CurrentJobStateType *adminFlsCurrentState){

    switch (*adminFlsCurrentState) {

        case FEE_WRITE_ADMIN_DATA:
            *adminFlsCurrentState = FEE_WRITE_ADMIN_DATA_WAIT;
            break;
        case FEE_WRITE_ADMIN_DATA_WAIT:
            *adminFlsCurrentState = FEE_WRITE_DATA;
            break;
        case FEE_WRITE_DATA:
            *adminFlsCurrentState = FEE_WRITE_DATA_WAIT;
            break;
        case FEE_WRITE_DATA_WAIT:
            *adminFlsCurrentState = FEE_WRITE_ADMIN_ID;
            break;
        case FEE_WRITE_ADMIN_ID:
            *adminFlsCurrentState = FEE_WRITE_ADMIN_ID_WAIT;
            break;
        case FEE_WRITE_ADMIN_ID_WAIT:
            *adminFlsCurrentState = FEE_WRITE_CHECK_ADMIN_DATA;
            break;
        case FEE_WRITE_CHECK_ADMIN_DATA:
            *adminFlsCurrentState = FEE_WRITE_CHECK_ADMIN_DATA_WAIT;
            break;
        case FEE_GARBAGE_COLLECT_WRITE_ADMIN_DATA:
            *adminFlsCurrentState = FEE_GARBAGE_COLLECT_WRITE_ADMIN_DATA_WAIT;
            break;
        case FEE_GARBAGE_COLLECT_WRITE_ADMIN_DATA_WAIT:
            *adminFlsCurrentState = FEE_GARBAGE_COLLECT_READ;
            break;
        case FEE_GARBAGE_COLLECT_READ:
            *adminFlsCurrentState = FEE_GARBAGE_COLLECT_READ_WAIT;
            break;
        case FEE_GARBAGE_COLLECT_READ_WAIT:
            *adminFlsCurrentState = FEE_GARBAGE_COLLECT_WRITE_DATA;
            break;
        case FEE_GARBAGE_COLLECT_WRITE_DATA:
            *adminFlsCurrentState = FEE_GARBAGE_COLLECT_WRITE_DATA_WAIT;
            break;
        case FEE_GARBAGE_COLLECT_WRITE_DATA_WAIT:
            *adminFlsCurrentState = FEE_GARBAGE_COLLECT_WRITE_ADMIN_ID;
            break;
        case FEE_GARBAGE_COLLECT_WRITE_ADMIN_ID:
            *adminFlsCurrentState = FEE_GARBAGE_COLLECT_WRITE_ADMIN_ID_WAIT;
            break;
        case FEE_GARBAGE_COLLECT_WRITE_ADMIN_ID_WAIT:
            *adminFlsCurrentState = FEE_GARBAGE_COLLECT_WRITE_CHECK_ADMIN_DATA;
            break;
        case FEE_GARBAGE_COLLECT_WRITE_CHECK_ADMIN_DATA:
            *adminFlsCurrentState = FEE_GARBAGE_COLLECT_WRITE_CHECK_ADMIN_DATA_WAIT;
            break;
        case FEE_CORRUPTED:
        default:
            /*do nothing*/
            break;
    }
}

static void AbortWriteJob(void) {
    if(CurrentJob.Write.Invalidate==0) {
        // only update DataPtr if not an invalidate job
        AdminFls.Write.NewBlockDataAddress += PAGE_ALIGN(Fee_Config.BlockConfig[CurrentJob.Write.BlockIdx].BlockSize);
    }
    AdminFls.Write.NewBlockAdminAddress -= ADMIN_SIZE;
    // increase error counter
    AdminFls.FailCounter++;
    if(AdminFls.FailCounter > MAX_NOF_FAILED_WRITE_ATTEMPTS) {
        FinishJob(MEMIF_JOB_FAILED);
    } else {
        AdminFls.State = FEE_IDLE; // set idle state to restart write job
    }
}

static void AbortGCJob(void) {
    AdminFls.Write.NewBlockDataAddress += PAGE_ALIGN(Fee_Config.BlockConfig[AdminFls.GarbageCollect.BlockIdx].BlockSize);
    AdminFls.Write.NewBlockAdminAddress -= ADMIN_SIZE;
    // increase error counter
    AdminFls.FailCounter++;
    if(AdminFls.FailCounter > MAX_NOF_FAILED_GC_ATTEMPTS) {
        if(ModuleStatus != MEMIF_IDLE) {
            FinishJob(MEMIF_JOB_FAILED);
        }
        AdminFls.State = FEE_CORRUPTED;
        SET_FEE_STATUS(MEMIF_IDLE);
    } else {
        AdminFls.State = FEE_IDLE; // set idle state to restart job
    }
}

/*
 * request bank header read
 */
static void StartupReadBankHeader(void) {
    SchM_Enter_Fee_EA_0();
    if(Fls_GetStatus() == MEMIF_IDLE) {
        SetFlsJobBusy();
        Std_ReturnType ret = FLS_READ(BankProp[CurrentJob.Startup.BankIdx].End - BANK_CTRL_SIZE, RWBuffer.Byte, BANK_CTRL_SIZE);
        if (ret == E_OK) {
            AdminFls.State = FEE_STARTUP_READ_BANK_HEADER_WAIT;
        } else {
            AbortStartup(Fls_GetJobResult());
        }
    }
    SchM_Exit_Fee_EA_0();
}

/*
 * Start of bank header 1 read
 */
void StartupRequested(void) {
    CurrentJob.Startup.BankIdx = 0;
    for (uint16 i = 0; i < FEE_NUM_OF_BLOCKS; i++) {
        AdminFls.BlockDescrTbl[i].BlockDataAddress = BLOCK_CORRUPT;
    }
    AdminFls.State = FEE_STARTUP_READ_BANK_HEADER;
    AdminFls.pendingJob = (uint8)NO_JOB;
    StartupReadBankHeader();
}

/*
 *  Check job result of bank header read
 */
static void StartupReadBankHeaderWait(void) {
    MemIf_JobResultType readRes;
    if (CheckFlsJobFinished() == TRUE) {
        Std_ReturnType retVal = E_OK;
        readRes = Fls_GetJobResult();
        if((MEMIF_JOB_OK == readRes) &&
                (RWBuffer.BankCtrl.Data.Magic == BankMagicMaster) &&
                (RWBuffer.BankCtrl.Data.BankCounter != BANK_COUNTER_INVALID )&&
                (RWBuffer.BankCtrl.Data.InvBankCounter != BANK_COUNTER_INVALID) &&
                (RWBuffer.BankCtrl.Data.BankCounter == (0xFFu & (~RWBuffer.BankCtrl.Data.InvBankCounter)))) {
            // magic ok, bank index ok and inv bankindex correct. Bank is ok
            CurrentJob.Startup.BankStatus[CurrentJob.Startup.BankIdx] = RWBuffer.BankCtrl.Data.BankCounter;
        } else if((MEMIF_JOB_OK == readRes) || (MEMIF_BLOCK_INCONSISTENT == readRes)) {
            /* Either bank header was not valid or inconsistent */
            DEBUG_PRINTF("Bank invalid\n");
            CurrentJob.Startup.BankStatus[CurrentJob.Startup.BankIdx] = BANK_COUNTER_INVALID;
        } else {
            AbortStartup(Fls_GetJobResult());
            retVal = E_NOT_OK;
        }
        if(retVal == E_OK) {
            CurrentJob.Startup.BankIdx++;
            if(CurrentJob.Startup.BankIdx < NUM_OF_BANKS) {
                AdminFls.State = FEE_STARTUP_READ_BANK_HEADER;
                StartupReadBankHeader();
            } else {
                // all read
                uint8 minIndex = 0;
                FlsBankCounterType counterVal = 0;
                uint32 numValid = 0;
                for(uint8 i = 0; i < NUM_OF_BANKS; i++) {
                    if(CurrentJob.Startup.BankStatus[i] != BANK_COUNTER_INVALID) {
                        FlsBankCounterType bankstat = CurrentJob.Startup.BankStatus[i] - counterVal;
                        if((numValid == 0) || ((sint8)( bankstat) < 0)) {
                            minIndex = i;
                            counterVal = CurrentJob.Startup.BankStatus[i];
                        }
                        numValid++;
                    }
                }
                if(numValid < NUM_OF_BANKS) {
                    // all banks aren't valid: set garbage collect job flag which will erase invalid banks.
                    AdminFls.pendingJob |=(uint8)PENDING_ERASE_JOB;
                } else {
                    // all banks ok, none to erase. Next bank to erase is minIndex
                    AdminFls.Erase.BankIdx = minIndex;
                    // next bank counter val is counter val for bank before minIndex
                    AdminFls.Erase.NextBankCounter = NEXT_BANK_COUNTERVAL(CurrentJob.Startup.BankStatus[(minIndex == 0)? (NUM_OF_BANKS -1) : (minIndex - 1)]);
                }
                if(numValid == 0) {
                    DEBUG_PRINTF("No valid banks\n");
                    // no valid bank:
                    // set bandIdx to 0 to ensure that bank 0 will be erased first
                    AdminFls.Erase.NextBankCounter = 1;
                    AdminFls.Erase.BankIdx = 0;
                    // also set AdminFls.GarbageCollect.BankIdx to 0: next bank to garbage collect
                    AdminFls.GarbageCollect.BankIdx = 0;
                    // configure admin data as bank 0 is valid. It will be valid and empty when state is idle
                    AdminFls.Write.BankIdx = 0;
                    AdminFls.Write.NewBlockAdminAddress = BankProp[0].End - (ADMIN_SIZE + BANK_CTRL_SIZE);
                    AdminFls.Write.NewBlockDataAddress = BankProp[0].Start;
                    // finish startup
                    FinishStartup();
                    // force state to erase to ensure that erase will be performed before any read/write jobs
                    AdminFls.State = FEE_ERASE_BANK;
                } else {
                    // minimum one bank is valid, read admin data
                    CurrentJob.Startup.BankIdx = minIndex;
                    CurrentJob.Startup.MinBankIdx = minIndex;
                    CurrentJob.Startup.BlockAdminAddress = BankProp[CurrentJob.Startup.BankIdx].End - (ADMIN_SIZE + BANK_CTRL_SIZE);
                    CurrentJob.Startup.BlockDataAddress = BankProp[CurrentJob.Startup.BankIdx].Start;
                    AdminFls.State = FEE_STARTUP_READ_BLOCK_ADMIN;
                    // set garbage collect bank index to minindex: next bank to garbage collect
                    AdminFls.GarbageCollect.BankIdx = minIndex;
                }
            }
        }
    }
}

/*
 * Start of block admin read
 */
static void StartupReadBlockAdmin(void)
{
    /* Start reading the banks */
    SchM_Enter_Fee_EA_0();
    if(Fls_GetStatus() == MEMIF_IDLE) {
        SetFlsJobBusy();
        memset(RWBuffer.Byte, FLS_ERASED_VALUE, ADMIN_SIZE);
        Std_ReturnType ret = FLS_READ(CurrentJob.Startup.BlockAdminAddress, RWBuffer.Byte, ADMIN_SIZE);
        if (ret == E_OK) {
            AdminFls.State = FEE_STARTUP_READ_BLOCK_ADMIN_WAIT;
        } else {
            AbortStartup(Fls_GetJobResult());
        }
    }
    SchM_Exit_Fee_EA_0();
}

static void HandleHeadAdminBlock(void) {
    if(CurrentJob.Startup.BlockAdminAddress != (BankProp[CurrentJob.Startup.BankIdx].End - (ADMIN_SIZE + BANK_CTRL_SIZE))) {
        // this bank was not empty, set head of this bank as next write position
        AdminFls.Write.BankIdx = CurrentJob.Startup.BankIdx;
        AdminFls.Write.NewBlockAdminAddress = CurrentJob.Startup.BlockAdminAddress;
        AdminFls.Write.NewBlockDataAddress = CurrentJob.Startup.BlockDataAddress;
        if(AdminFls.Write.BankIdx != CurrentJob.Startup.MinBankIdx) {
            // more than one bank with data, set garbage collect flag
            AdminFls.pendingJob |= (uint8)PENDING_GARBAGE_COLLECT_JOB;
            AdminFls.GarbageCollect.BlockIdx = 0;
        }
        // next bank
        uint8 prevBankIdx = CurrentJob.Startup.BankIdx;
        CurrentJob.Startup.BankIdx = NEXT_BANK_IDX(CurrentJob.Startup.BankIdx);
        if(CurrentJob.Startup.BankIdx == CurrentJob.Startup.MinBankIdx) {
            // this was the last bank, startup done
            if((AdminFls.Write.NewBlockAdminAddress - AdminFls.Write.NewBlockDataAddress) < AdminFls.GarbageCollect.TotalFeeSize) {
                // not room for all data in block, set forced garbage collect flag
                AdminFls.pendingJob |= (uint8)PENDING_FORCED_GARBAGE_COLLECT_JOB;
            }
            // finish startup
            FinishStartup();
        } else if(CurrentJob.Startup.BankStatus[CurrentJob.Startup.BankIdx] == BANK_COUNTER_INVALID) {
            // next bank is invalid, startup done
            AdminFls.Erase.NextBankCounter = NEXT_BANK_COUNTERVAL(CurrentJob.Startup.BankStatus[prevBankIdx]);
            AdminFls.Erase.BankIdx = CurrentJob.Startup.BankIdx;
            FinishStartup();
        } else {
            // read next bank
            CurrentJob.Startup.BlockAdminAddress = BankProp[CurrentJob.Startup.BankIdx].End - (ADMIN_SIZE + BANK_CTRL_SIZE);
            CurrentJob.Startup.BlockDataAddress = BankProp[CurrentJob.Startup.BankIdx].Start;
            AdminFls.State = FEE_STARTUP_READ_BLOCK_ADMIN;
            StartupReadBlockAdmin();
        }
    } else {
        // empty bank
        if(CurrentJob.Startup.BankIdx == CurrentJob.Startup.MinBankIdx) {
            // lowest id with empty bank: all banks are empty
            // set head of this bank as next write position
            AdminFls.Write.BankIdx = CurrentJob.Startup.BankIdx;
            AdminFls.Write.NewBlockAdminAddress = CurrentJob.Startup.BlockAdminAddress;
            AdminFls.Write.NewBlockDataAddress = CurrentJob.Startup.BlockDataAddress;
        }
        if((AdminFls.pendingJob & (uint8)PENDING_ERASE_JOB)!=0) {
            // pending garbage collect job: all banks aren't valid
            // set garbage collect bank idx to next bank to ensure that all remaining banks gets erased
            AdminFls.Erase.NextBankCounter = NEXT_BANK_COUNTERVAL(CurrentJob.Startup.BankStatus[CurrentJob.Startup.BankIdx]);
            AdminFls.Erase.BankIdx = NEXT_BANK_IDX(CurrentJob.Startup.BankIdx);
        } else {
            // all banks ok.
        }
        // startup done
        FinishStartup();
    }
}
/*
 * Check job result of block admin read, if all block processed finish
 * otherwise request for a new block admin read
 */
static void StartupReadBlockAdminWait(void)
{
    MemIf_JobResultType readResult;
    if (CheckFlsJobFinished()==TRUE) {
        readResult = Fls_GetJobResult();
        if ((MEMIF_JOB_OK == readResult) || (MEMIF_BLOCK_INCONSISTENT == readResult)) {
            // we ignore ECC errors and check data for validity
            if((RWBuffer.BlockCtrl.DataPage.Data.BlockDataLength == (uint16)~RWBuffer.BlockCtrl.DataPage.Data.InvBlockDataLength) && (
               (RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress == INVALIDATED_BLOCK )|| (
               (RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress >= CurrentJob.Startup.BlockDataAddress)&&
               (RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress < CurrentJob.Startup.BlockAdminAddress) &&
               (RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress == PAGE_ALIGN(RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress)) &&
               ((RWBuffer.BlockCtrl.DataPage.Data.BlockDataLength + RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress) <= CurrentJob.Startup.BlockAdminAddress)))) {
                // invalidated address or DataAddress within bank and has valid alignment, InvBlockDataLength valid, BlockDataLength has a reasonable value, we assume that BlockDataLength is valid
                uint16 blockIndex = GetBlockIdxFromBlockNumber(RWBuffer.BlockCtrl.IdPage.Data.BlockNo);
                if((MEMIF_JOB_OK == readResult) &&
                   (blockIndex < FEE_NUM_OF_BLOCKS) &&
                   (RWBuffer.BlockCtrl.IdPage.Data.Magic == BlockMagicMaster) &&
                   (RWBuffer.BlockCtrl.IdPage.Data.BlockNo == (uint16)~RWBuffer.BlockCtrl.IdPage.Data.InvBlockNo )&&
                   (Fee_Config.BlockConfig[blockIndex].BlockSize == RWBuffer.BlockCtrl.DataPage.Data.BlockDataLength)) {
                    // active blockindex, valid magic, valid InvBlockNo, BlockSize is same as in configuration: we assume that data is correct
                    AdminFls.BlockDescrTbl[blockIndex].BlockDataAddress = RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress;
                }
                DEBUG_PRINTF("Reading next admin block\n");
                if(RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress != INVALIDATED_BLOCK) {
                    // increase data pointer
                    CurrentJob.Startup.BlockDataAddress = PAGE_ALIGN(RWBuffer.BlockCtrl.DataPage.Data.BlockDataLength) + RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress;
                }
                // next admin block
                CurrentJob.Startup.BlockAdminAddress -= ADMIN_SIZE;
                // check if room for more admin blocks
                if(CurrentJob.Startup.BlockAdminAddress >= CurrentJob.Startup.BlockDataAddress) {
                    // still room for admin blocks, read next admin block
                    AdminFls.State = FEE_STARTUP_READ_BLOCK_ADMIN;
                    StartupReadBlockAdmin();
                } else {
                    HandleHeadAdminBlock();
                }
            } else if((MEMIF_JOB_OK == readResult) &&
                    (0 == BlankCheck(RWBuffer.Byte, FLS_ERASED_VALUE, ADMIN_SIZE))) {
                // admin block empty, assume that this was the head of data
                DEBUG_PRINTF("MemIf==MEMIF_JOB_OK and blankcheck OK\n");
                HandleHeadAdminBlock();
            } else {
                DEBUG_PRINTF("Invalid Adminblock\n");
                // invalid admin block: ignore
                CurrentJob.Startup.BlockAdminAddress -= ADMIN_SIZE;
                // check if room for more admin blocks
                if(CurrentJob.Startup.BlockAdminAddress >= CurrentJob.Startup.BlockDataAddress) {
                    // still room for admin blocks, read next admin block
                    AdminFls.State = FEE_STARTUP_READ_BLOCK_ADMIN;
                    StartupReadBlockAdmin();
                } else {
                    HandleHeadAdminBlock();
                }
            }
        } else { /* ErrorStatus not E_OK */
            AbortStartup(readResult);
        }
    }
}

/*
 * Request data read
 */
static void Reading(void)
{
    /* @req FEE023 */
    Fls_AddressType addr = AdminFls.BlockDescrTbl[CurrentJob.Read.BlockIdx].BlockDataAddress;

    if(addr == INVALIDATED_BLOCK ) {
        // data invalidated
        /* @req FEE075 */
        AdminFls.pendingJob &=(uint8)(~(uint8)PENDING_READ_JOB);
        FinishJob(MEMIF_BLOCK_INVALID);
    } else if(addr == BLOCK_CORRUPT ) {
        // no data
        AdminFls.pendingJob &= (uint8)(~(uint8)PENDING_READ_JOB);
        FinishJob(MEMIF_BLOCK_INCONSISTENT);
    } else {
        SchM_Enter_Fee_EA_0();
        if(Fls_GetStatus() == MEMIF_IDLE) {
            SetFlsJobBusy();
            Std_ReturnType ret = FLS_READ(AdminFls.BlockDescrTbl[CurrentJob.Read.BlockIdx].BlockDataAddress + CurrentJob.Read.DataOffset, CurrentJob.Read.DataPtr, CurrentJob.Read.Length);
            if (ret == E_OK) {
                AdminFls.State = FEE_READ_WAIT;
            } else {
                AdminFls.BlockDescrTbl[CurrentJob.Read.BlockIdx].BlockDataAddress = BLOCK_CORRUPT;
                AdminFls.pendingJob &= ~(uint8)PENDING_READ_JOB;
                FinishJob(MEMIF_BLOCK_INCONSISTENT);
            }
        }
        SchM_Exit_Fee_EA_0();
    }
}

/*
 * Request data read
 */
static void ReadWait(void)
{
    if (CheckFlsJobFinished()==TRUE) {
        MemIf_JobResultType jobResult = Fls_GetJobResult();
        AdminFls.pendingJob &= ~(uint8)PENDING_READ_JOB;
        if(jobResult == MEMIF_JOB_OK) {
            FinishJob(MEMIF_JOB_OK);
        } else {
            FinishJob(MEMIF_BLOCK_INCONSISTENT);
        }
    }
}

static void WriteAdminData(void) {

    SchM_Enter_Fee_EA_0();
    if(Fls_GetStatus() == MEMIF_IDLE) {
        SetFlsJobBusy();
#if defined(DEBUG_FEE)
        DEBUG_PRINTF("AdminData: 0x%x\n",RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress );
#endif

        Std_ReturnType ret = FLS_WRITE(AdminFls.Write.NewBlockAdminAddress, RWBuffer.BlockCtrl.DataPage.Byte, BLOCK_CTRL_DATA_PAGE_SIZE);
        if (ret == E_OK) {
            IncrementFlsState(&AdminFls.State);
        } else {
            AbortWriteJob();
        }
    }
    SchM_Exit_Fee_EA_0();
}

/*
 * Check if bank switch needed:
 * - Yes, start mark current bank as old
 * - No, start of header write
 */
static void WriteStartJob(void)
{
    // init the rw buffer
    if(CurrentJob.Write.Invalidate!=0) {
        RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress = INVALIDATED_BLOCK;
    } else {
        RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress = AdminFls.Write.NewBlockDataAddress;
    }
    RWBuffer.BlockCtrl.DataPage.Data.BlockDataLength = Fee_Config.BlockConfig[CurrentJob.Write.BlockIdx].BlockSize;
    RWBuffer.BlockCtrl.DataPage.Data.InvBlockDataLength = ~RWBuffer.BlockCtrl.DataPage.Data.BlockDataLength;
    // write the data part of the admin block
    AdminFls.State = FEE_WRITE_ADMIN_DATA;
    WriteAdminData();
}

static void WriteData(void) {
    SchM_Enter_Fee_EA_0();
    if(Fls_GetStatus() == MEMIF_IDLE) {
        SetFlsJobBusy();
        Std_ReturnType ret = FLS_WRITE(AdminFls.Write.NewBlockDataAddress, CurrentJob.Write.DataPtr, PAGE_ALIGN(Fee_Config.BlockConfig[CurrentJob.Write.BlockIdx].BlockSize));
        if (ret == E_OK) {
            AdminFls.State = FEE_WRITE_DATA_WAIT;
        } else {
            AbortWriteJob();
        }
    }
    SchM_Exit_Fee_EA_0();
}

static void WriteAdminId(void) {
    SchM_Enter_Fee_EA_0();
    if(Fls_GetStatus() == MEMIF_IDLE) {
        SetFlsJobBusy();
        Std_ReturnType ret = FLS_WRITE(AdminFls.Write.NewBlockAdminAddress + BLOCK_CTRL_DATA_PAGE_SIZE, RWBuffer.BlockCtrl.IdPage.Byte, BLOCK_CTRL_ID_PAGE_SIZE);
        if (ret == E_OK) {
            IncrementFlsState(&AdminFls.State);
        } else {
            AbortWriteJob();
        }
    }
    SchM_Exit_Fee_EA_0();
}



static void WriteDataWait(void) {
    MemIf_JobResultType readResult;
    if (CheckFlsJobFinished()==TRUE) {
        readResult = Fls_GetJobResult();
        if (MEMIF_JOB_OK == readResult) {
            // data written, write remaining part of admin block
            RWBuffer.BlockCtrl.IdPage.Data.BlockNo = Fee_Config.BlockConfig[CurrentJob.Write.BlockIdx].BlockNumber;
            RWBuffer.BlockCtrl.IdPage.Data.InvBlockNo = ~Fee_Config.BlockConfig[CurrentJob.Write.BlockIdx].BlockNumber;
            RWBuffer.BlockCtrl.IdPage.Data.Magic = BlockMagicMaster;
            AdminFls.State = FEE_WRITE_ADMIN_ID;
            WriteAdminId();
        } else {
            AbortWriteJob();
        }
    }
}

static void WriteCheckAdminData(void) {
    // read admin data
    SchM_Enter_Fee_EA_0();
    if(Fls_GetStatus() == MEMIF_IDLE) {
        SetFlsJobBusy();
        Std_ReturnType ret = FLS_READ(AdminFls.Write.NewBlockAdminAddress, RWBuffer.Byte, ADMIN_SIZE);
        if (ret == E_OK) {
            IncrementFlsState(&AdminFls.State);
        } else {
            AbortWriteJob();
        }
    }
    SchM_Exit_Fee_EA_0();
}

static void WriteAdminDataWait(void) {
    MemIf_JobResultType writeResult;
    if (CheckFlsJobFinished()==TRUE) {
        writeResult = Fls_GetJobResult();
        if (MEMIF_JOB_OK == writeResult) {
            // admin data written, write data
            if(CurrentJob.Write.Invalidate!=0) {
                // invalidate data, no data to be written, write admin block id instead
                // call WriteDataWait since it will setup the write of remaining part of admin block
                AdminFls.State = FEE_WRITE_DATA_WAIT;
                WriteDataWait();
            } else {
                AdminFls.State = FEE_WRITE_DATA;
                WriteData();
            }
        } else {
            // failed to write, check is anything was written
            memset(RWBuffer.Byte, ~FLS_ERASED_VALUE, ADMIN_SIZE);
            AdminFls.State = FEE_WRITE_CHECK_ADMIN_DATA;
            WriteCheckAdminData();
        }
    }
}

static void WriteAdminIdWait(void) {
    MemIf_JobResultType writeResult;
    if (CheckFlsJobFinished()==TRUE) {
        writeResult = Fls_GetJobResult();
        // all data written. Update admin data
        if (MEMIF_JOB_OK == writeResult) {
            if(CurrentJob.Write.Invalidate!=0) {
                AdminFls.BlockDescrTbl[CurrentJob.Write.BlockIdx].BlockDataAddress = INVALIDATED_BLOCK;
            } else {
                AdminFls.BlockDescrTbl[CurrentJob.Write.BlockIdx].BlockDataAddress = AdminFls.Write.NewBlockDataAddress;
            }
            AdminFls.pendingJob &= ~(uint8)PENDING_WRITE_JOB;
            FinishJob(MEMIF_JOB_OK);
        } else {
            // failed to write, set mode to idle to restart
            AdminFls.State = FEE_IDLE;
            // increase error counter
            AdminFls.FailCounter++;
        }
        if(CurrentJob.Write.Invalidate==0) {
            // update data head ptr if not an invalidate operation
            AdminFls.Write.NewBlockDataAddress += PAGE_ALIGN(Fee_Config.BlockConfig[CurrentJob.Write.BlockIdx].BlockSize);
        }
        AdminFls.Write.NewBlockAdminAddress -= ADMIN_SIZE;
    }
}

static void WriteCheckAdminDataWait(void) {
    MemIf_JobResultType readResult;
    if (CheckFlsJobFinished()==TRUE) {
        readResult = Fls_GetJobResult();
        if((MEMIF_JOB_OK == readResult) && (0 == BlankCheck(RWBuffer.Byte, FLS_ERASED_VALUE, ADMIN_SIZE))) {
            // admin block empty, do not increase data pointers
            // set state to idle to retry to write
            AdminFls.State = FEE_IDLE;
        } else {
            // either was data not empty or read result not ok: assume that something was written. Increase datapointers
            AbortWriteJob();
        }
    }
}

static void EraseBank(void) {
    SchM_Enter_Fee_EA_0();
    if(Fls_GetStatus() == MEMIF_IDLE) {
        SetFlsJobBusy();
        Std_ReturnType ret = Fls_Erase(BankProp[AdminFls.Erase.BankIdx].Start, BankProp[AdminFls.Erase.BankIdx].End - BankProp[AdminFls.Erase.BankIdx].Start);
        if (ret == E_OK) {
            AdminFls.State =FEE_ERASE_BANK_WAIT;
        } else {
            // failed to write, set mode to idle to restart
            AdminFls.State = FEE_IDLE;
            // increase error counter
            AdminFls.FailCounter++;
        }
    }
    SchM_Exit_Fee_EA_0();
}

static void WriteBankHeader() {
    SchM_Enter_Fee_EA_0();
    if(Fls_GetStatus() == MEMIF_IDLE) {
        SetFlsJobBusy();
        Std_ReturnType ret = FLS_WRITE(BankProp[AdminFls.Erase.BankIdx].End - BANK_CTRL_SIZE, RWBuffer.BankCtrl.Byte, BANK_CTRL_SIZE);
        if (ret == E_OK) {
            AdminFls.State =FEE_WRITE_BANK_HEADER_WAIT;
        } else {
            // failed to write, set mode to idle to restart
            AdminFls.State = FEE_IDLE;
            // increase error counter
            AdminFls.FailCounter++;
        }
    }
    SchM_Exit_Fee_EA_0();
}

static void EraseBankWait(void) {
    MemIf_JobResultType readResult;
    if (CheckFlsJobFinished()==TRUE) {
        readResult = Fls_GetJobResult();
        if (MEMIF_JOB_OK == readResult) {
            // erase done, request header write
            RWBuffer.BankCtrl.Data.BankCounter = AdminFls.Erase.NextBankCounter;
            RWBuffer.BankCtrl.Data.InvBankCounter = ~RWBuffer.BankCtrl.Data.BankCounter;
            RWBuffer.BankCtrl.Data.Magic = BankMagicMaster;
            AdminFls.State = FEE_WRITE_BANK_HEADER;
            WriteBankHeader();
        } else {
            // failed to erase, set mode to idle to restart
            AdminFls.State = FEE_IDLE;
            // increase error counter
            AdminFls.FailCounter++;
        }
    }
}

static void WriteBankHeaderWait() {
    MemIf_JobResultType readResult;
    if (CheckFlsJobFinished()==TRUE) {
        readResult = Fls_GetJobResult();
        if (MEMIF_JOB_OK == readResult) {
            // bank header written, update admin block status
            AdminFls.Erase.NextBankCounter = NEXT_BANK_COUNTERVAL(AdminFls.Erase.NextBankCounter);
            AdminFls.Erase.BankIdx = NEXT_BANK_IDX(AdminFls.Erase.BankIdx);
            if(AdminFls.GarbageCollect.BankIdx == AdminFls.Erase.BankIdx) {
                // next bank is next garbage collect bank: all erase jobs done
                AdminFls.pendingJob &= ~(uint8)PENDING_ERASE_JOB;
            }
            AdminFls.State = FEE_IDLE;
        } else {
            // increase error counter
            AdminFls.FailCounter++;
            if(AdminFls.FailCounter > MAX_NOF_FAILED_GC_ATTEMPTS) {
                AdminFls.State = FEE_CORRUPTED;
                if(ModuleStatus != MEMIF_IDLE) {
                    FinishJob(MEMIF_JOB_FAILED);
                }
            } else {
                // failed to write, set mode to idle to restart
                AdminFls.State = FEE_ERASE_BANK;
            }
        }
    }
}

static void GarbageCollectStartJob(void)
{
    // init the rw buffer
    if(AdminFls.BlockDescrTbl[AdminFls.GarbageCollect.BlockIdx].BlockDataAddress == INVALIDATED_BLOCK) {
        RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress = INVALIDATED_BLOCK;
    } else {
        RWBuffer.BlockCtrl.DataPage.Data.BlockDataAddress = AdminFls.Write.NewBlockDataAddress;
    }
    RWBuffer.BlockCtrl.DataPage.Data.BlockDataLength = Fee_Config.BlockConfig[AdminFls.GarbageCollect.BlockIdx].BlockSize;
    RWBuffer.BlockCtrl.DataPage.Data.InvBlockDataLength = ~RWBuffer.BlockCtrl.DataPage.Data.BlockDataLength;
    // write the data part of the admin block
    AdminFls.State = FEE_GARBAGE_COLLECT_WRITE_ADMIN_DATA;
    WriteAdminData();
}

static void GarbageCollectRead(void)
{
    uint16 size = Fee_Config.BlockConfig[AdminFls.GarbageCollect.BlockIdx].BlockSize - AdminFls.GarbageCollect.Offset;
    size = (size > RWBUFFER_SIZE)? RWBUFFER_SIZE : size;
    SchM_Enter_Fee_EA_0();
    if(Fls_GetStatus() == MEMIF_IDLE) {
        SetFlsJobBusy();
        Std_ReturnType ret = FLS_READ(AdminFls.BlockDescrTbl[AdminFls.GarbageCollect.BlockIdx].BlockDataAddress + AdminFls.GarbageCollect.Offset, RWBuffer.Byte, size);
        if (ret == E_OK) {
            IncrementFlsState(&AdminFls.State);
        } else {
            // failed to read data, set data to corrupt
            AdminFls.BlockDescrTbl[AdminFls.GarbageCollect.BlockIdx].BlockDataAddress = BLOCK_CORRUPT;
            AdminFls.GarbageCollect.BlockIdx++;
            AbortGCJob();
        }
    }
    SchM_Exit_Fee_EA_0();
}

static void GarbageCollectWriteCheckAdminData(void) {
    // read admin data
    SchM_Enter_Fee_EA_0();
    if(Fls_GetStatus() == MEMIF_IDLE) {
        SetFlsJobBusy();
        Std_ReturnType ret = FLS_READ(AdminFls.Write.NewBlockAdminAddress, RWBuffer.Byte, ADMIN_SIZE);
        if (ret == E_OK) {
            IncrementFlsState(&AdminFls.State);
        } else {
            AbortGCJob();
        }
    }
    SchM_Exit_Fee_EA_0();
}

static void GarbageCollectWriteAdminDataWait(void) {
    MemIf_JobResultType writeResult;
    if (CheckFlsJobFinished()==TRUE) {
        writeResult = Fls_GetJobResult();
        if (MEMIF_JOB_OK == writeResult) {
            if(AdminFls.BlockDescrTbl[AdminFls.GarbageCollect.BlockIdx].BlockDataAddress == INVALIDATED_BLOCK) {
                // no data to write, write remaining part of admin block
                RWBuffer.BlockCtrl.IdPage.Data.BlockNo = Fee_Config.BlockConfig[AdminFls.GarbageCollect.BlockIdx].BlockNumber;
                RWBuffer.BlockCtrl.IdPage.Data.InvBlockNo = ~Fee_Config.BlockConfig[AdminFls.GarbageCollect.BlockIdx].BlockNumber;
                RWBuffer.BlockCtrl.IdPage.Data.Magic = BlockMagicMaster;
                AdminFls.State = FEE_GARBAGE_COLLECT_WRITE_ADMIN_ID;
                WriteAdminId();
            } else {
                // admin data written, read data
                AdminFls.GarbageCollect.Offset = 0;
                AdminFls.State = FEE_GARBAGE_COLLECT_READ;
                GarbageCollectRead();
            }
        } else {
            // failed to write, check is anything was written
            memset(RWBuffer.Byte, ~FLS_ERASED_VALUE, ADMIN_SIZE);
            AdminFls.State = FEE_GARBAGE_COLLECT_WRITE_CHECK_ADMIN_DATA;
            GarbageCollectWriteCheckAdminData();
        }
    }
}

static void GarbageCollectWriteData(void) {
    SchM_Enter_Fee_EA_0();
    if(Fls_GetStatus() == MEMIF_IDLE) {
        uint16 size = Fee_Config.BlockConfig[AdminFls.GarbageCollect.BlockIdx].BlockSize - AdminFls.GarbageCollect.Offset;
        size = (size > RWBUFFER_SIZE)? RWBUFFER_SIZE : (uint16)PAGE_ALIGN(size);
        SetFlsJobBusy();
        Std_ReturnType ret = FLS_WRITE(AdminFls.Write.NewBlockDataAddress + AdminFls.GarbageCollect.Offset, RWBuffer.Byte, size);
        if (ret == E_OK) {
            AdminFls.GarbageCollect.Offset += size;
            IncrementFlsState(&AdminFls.State);
        } else {
            AbortGCJob();
        }
    }
    SchM_Exit_Fee_EA_0();
}

static void GarbageCollectReadWait(void)
{
    if (CheckFlsJobFinished()==TRUE) {
        MemIf_JobResultType jobResult = Fls_GetJobResult();
        if(jobResult == MEMIF_JOB_OK) {
            // read done, write data
            AdminFls.State = FEE_GARBAGE_COLLECT_WRITE_DATA;
            GarbageCollectWriteData();
        } else {
            // failed to read data, set data to invalid
            AdminFls.BlockDescrTbl[AdminFls.GarbageCollect.BlockIdx].BlockDataAddress = BLOCK_CORRUPT;
            AdminFls.GarbageCollect.BlockIdx++;
            AbortGCJob();
        }
    }
}

static void GarbageCollectWriteDataWait(void) {
    MemIf_JobResultType readResult;
    if (CheckFlsJobFinished()==TRUE) {
        readResult = Fls_GetJobResult();
        if (MEMIF_JOB_OK == readResult) {
            // check if more data to write
            if(AdminFls.GarbageCollect.Offset >= Fee_Config.BlockConfig[AdminFls.GarbageCollect.BlockIdx].BlockSize) {
                // data written, write remaining part of admin block
                RWBuffer.BlockCtrl.IdPage.Data.BlockNo = Fee_Config.BlockConfig[AdminFls.GarbageCollect.BlockIdx].BlockNumber;
                RWBuffer.BlockCtrl.IdPage.Data.InvBlockNo = ~Fee_Config.BlockConfig[AdminFls.GarbageCollect.BlockIdx].BlockNumber;
                RWBuffer.BlockCtrl.IdPage.Data.Magic = BlockMagicMaster;
                AdminFls.State = FEE_GARBAGE_COLLECT_WRITE_ADMIN_ID;
                WriteAdminId();
            } else {
                // continue to read
                AdminFls.State = FEE_GARBAGE_COLLECT_READ;
                GarbageCollectRead();
            }

        } else {
            AbortGCJob();
        }
    }
}

static void GarbageCollectWriteAdminIdWait(void) {
    MemIf_JobResultType readResult;
    if (CheckFlsJobFinished()==TRUE) {
        readResult = Fls_GetJobResult();
        if (MEMIF_JOB_OK == readResult) {
            // garbage collect of block is done
            if(AdminFls.BlockDescrTbl[AdminFls.GarbageCollect.BlockIdx].BlockDataAddress != INVALIDATED_BLOCK) {
                AdminFls.BlockDescrTbl[AdminFls.GarbageCollect.BlockIdx].BlockDataAddress = AdminFls.Write.NewBlockDataAddress;
                // all data written. Update admin data
                AdminFls.Write.NewBlockDataAddress += PAGE_ALIGN(Fee_Config.BlockConfig[AdminFls.GarbageCollect.BlockIdx].BlockSize);
            }
            AdminFls.Write.NewBlockAdminAddress -= ADMIN_SIZE;
            AdminFls.GarbageCollect.BlockIdx++;
            // change state to idle to continue with next block
            AdminFls.State = FEE_IDLE;
        } else {
            AbortGCJob();
        }
    }
}

static void GarbageCollectWriteCheckAdminDataWait(void) {
    MemIf_JobResultType readResult;
    if (CheckFlsJobFinished()==TRUE) {
        readResult = Fls_GetJobResult();
        if((MEMIF_JOB_OK == readResult) &&( 0 == BlankCheck(RWBuffer.Byte, FLS_ERASED_VALUE, ADMIN_SIZE))) {
            // admin block empty, do not increase data pointers
            AdminFls.FailCounter++;
            // set state to idle to retry to garbage collect
            AdminFls.State = FEE_IDLE;
        } else {
            // either was data not empty or read result not ok: assume that something was written. Increase datapointers
            AbortGCJob();
        }
    }
}

static void Idle(void) {
    boolean whilebreakflag = FALSE;
    if((AdminFls.pendingJob & (uint8)PENDING_READ_JOB)!=0) {
        // start read job
        AdminFls.State = FEE_READ;
        Reading();
    } else if(((AdminFls.pendingJob & (uint8) PENDING_WRITE_JOB)!=0) && (
                ((AdminFls.pendingJob & (uint8)PENDING_FORCED_GARBAGE_COLLECT_JOB) == 0 )||
                (CurrentJob.Write.Invalidate == 1 )||
                (IS_ADDRESS_WITHIN_BANK(AdminFls.BlockDescrTbl[CurrentJob.Write.BlockIdx].BlockDataAddress, AdminFls.GarbageCollect.BankIdx)) )
            ) {
        // pending write job while not in forced garbage collect or data to write is not garbage collected
        uint8 nextBank = NEXT_BANK_IDX(AdminFls.Write.BankIdx);
        /*lint -e{713} as per the logic sin32 value of address difference can not be more than is range */
        sint32 remainingBytesInBank = (AdminFls.Write.NewBlockAdminAddress - AdminFls.Write.NewBlockDataAddress); // This value might be negative: use signed type
        sint32 bytesToWrite = (CurrentJob.Write.Invalidate == 1)? 0 : (sint32)Fee_Config.BlockConfig[CurrentJob.Write.BlockIdx].BlockSize;
        sint32 identifybytes = ((sint32)AdminFls.GarbageCollect.TotalFeeSize + bytesToWrite + (sint32) ADMIN_SIZE + (sint32) FORCED_GARBAGE_COLLECT_MARGIN);
        if(((AdminFls.pendingJob & (uint8)PENDING_FORCED_GARBAGE_COLLECT_JOB) == 0) &&
           (remainingBytesInBank < identifybytes) &&
           (nextBank == AdminFls.GarbageCollect.BankIdx)
           ) {
            // no room for all data when this data written, and all banks written: set forced garbage collect
            AdminFls.pendingJob |= (uint8)PENDING_FORCED_GARBAGE_COLLECT_JOB;
            // stay in idle and perform a new evaluation in next main loop
        } else if(remainingBytesInBank < bytesToWrite) {
            // no room for this block, is it possible to switch bank?
            if(nextBank == AdminFls.GarbageCollect.BankIdx) {
                // next bank is garbage collect bank, not possible to switch bank
                // throw stored data and hope that garbage collect will finish before power is switched off and new data written. set fault code?
                AdminFls.BlockDescrTbl[CurrentJob.Write.BlockIdx].BlockDataAddress = BLOCK_CORRUPT; // next call to idle will continue with garbage collect of the other blocks, stay in idle
            } else if (nextBank == AdminFls.Erase.BankIdx) {
                // next bank isn't erased and not garbage collect bank: erase job pending, erase bank to get room for new data
                AdminFls.State = FEE_ERASE_BANK;
            } else {
                // next bank is erased: switch bank
                AdminFls.Write.BankIdx = nextBank;
                AdminFls.Write.NewBlockAdminAddress = BankProp[AdminFls.Write.BankIdx].End - (ADMIN_SIZE + BANK_CTRL_SIZE);
                AdminFls.Write.NewBlockDataAddress = BankProp[AdminFls.Write.BankIdx].Start;
                AdminFls.pendingJob |= (uint8)PENDING_GARBAGE_COLLECT_JOB; // set pending garbage collect flag since more than one bank with data
                AdminFls.State = FEE_WRITE;
                WriteStartJob();
            }
        } else {
            AdminFls.State = FEE_WRITE;
            WriteStartJob();
        }
    } else if((AdminFls.pendingJob & (uint8)PENDING_ERASE_JOB)!=0) {
        AdminFls.State = FEE_ERASE_BANK;
    } else if((AdminFls.pendingJob & (uint8)PENDING_FORCED_GARBAGE_COLLECT_JOB)!=0) {
        // search for next block to garbage collect
        while((AdminFls.GarbageCollect.BlockIdx < FEE_NUM_OF_BLOCKS) && (
                (AdminFls.BlockDescrTbl[AdminFls.GarbageCollect.BlockIdx].BlockDataAddress == BLOCK_CORRUPT) ||
                ((AdminFls.BlockDescrTbl[AdminFls.GarbageCollect.BlockIdx].BlockDataAddress != INVALIDATED_BLOCK )&&
                (!IS_ADDRESS_WITHIN_BANK(AdminFls.BlockDescrTbl[AdminFls.GarbageCollect.BlockIdx].BlockDataAddress ,AdminFls.GarbageCollect.BankIdx)))
             )
            ) {
            // block isn't in the bank that currently is garbage collected: try next block
            AdminFls.GarbageCollect.BlockIdx++;
        }
        if(AdminFls.GarbageCollect.BlockIdx >= FEE_NUM_OF_BLOCKS) {
            // garbage collect done
            AdminFls.GarbageCollect.BankIdx = NEXT_BANK_IDX(AdminFls.GarbageCollect.BankIdx);
            AdminFls.GarbageCollect.BlockIdx = 0;
            if(AdminFls.GarbageCollect.BankIdx == AdminFls.Write.BankIdx) {
                // no more data to garbage collect, clear flags
                AdminFls.pendingJob &= ~(uint8)((uint8)PENDING_FORCED_GARBAGE_COLLECT_JOB |(uint8) PENDING_GARBAGE_COLLECT_JOB);
            } else {
                // one bank free, not in forced garbage collect mode anymore
                AdminFls.pendingJob &= ~(uint8)PENDING_FORCED_GARBAGE_COLLECT_JOB;
            }
            // set pending erase job to erase block freed by garbage collect
            AdminFls.pendingJob |= (uint8)PENDING_ERASE_JOB;
        } else {
            /*lint -e{713} as per the logic sin32 value of address difference can not be more than is range */
            sint32 remainingBytes = AdminFls.Write.NewBlockAdminAddress - AdminFls.Write.NewBlockDataAddress;
            sint32 bytesToWrite = (AdminFls.BlockDescrTbl[AdminFls.GarbageCollect.BlockIdx].BlockDataAddress == INVALIDATED_BLOCK)? 0 : (sint32)(Fee_Config.BlockConfig[AdminFls.GarbageCollect.BlockIdx].BlockSize);
            if (bytesToWrite > remainingBytes) {
                // no room in bank. Throw data, set fault code?
                AdminFls.BlockDescrTbl[AdminFls.GarbageCollect.BlockIdx].BlockDataAddress = BLOCK_CORRUPT;
                AdminFls.GarbageCollect.BlockIdx++;
            } else {
                GarbageCollectStartJob();
            }
        }
    } else if((AdminFls.pendingJob & (uint8)PENDING_GARBAGE_COLLECT_JOB)!=0) {
        // search for next block to garbage collect
        while((AdminFls.GarbageCollect.BlockIdx < FEE_NUM_OF_BLOCKS)&&(whilebreakflag==FALSE)) {
            if(AdminFls.BlockDescrTbl[AdminFls.GarbageCollect.BlockIdx].BlockDataAddress == INVALIDATED_BLOCK) {
                // block invalidated, invalidate again to garbage collect the invalidation
                /*lint -e{713} as per the logic sin32 value of address difference can not be more than is range */
                sint32 remainingBytes = (AdminFls.Write.NewBlockAdminAddress - AdminFls.Write.NewBlockDataAddress);
                sint32 bytesToWrite = 0;
                if (bytesToWrite > remainingBytes) {
                    // no room in bank. Throw data, set fault code?
                    AdminFls.GarbageCollect.BlockIdx++;
                } else {
                    GarbageCollectStartJob();
                    whilebreakflag = TRUE;
                }
            } else if(IS_ADDRESS_WITHIN_BANK(AdminFls.BlockDescrTbl[AdminFls.GarbageCollect.BlockIdx].BlockDataAddress ,AdminFls.GarbageCollect.BankIdx)) {
                // block not garbage collected
                whilebreakflag = TRUE;
            } else {
                // block isn't in the bank that currently is garbage collected: try next block
                AdminFls.GarbageCollect.BlockIdx++;
            }
        }
        if(AdminFls.GarbageCollect.BlockIdx >= FEE_NUM_OF_BLOCKS) {
            // garbage collect done
            AdminFls.GarbageCollect.BankIdx = NEXT_BANK_IDX(AdminFls.GarbageCollect.BankIdx);
            AdminFls.GarbageCollect.BlockIdx = 0;
            if(AdminFls.GarbageCollect.BankIdx == AdminFls.Write.BankIdx) {
                // no more data to garbage collect, clear flags
                AdminFls.pendingJob &= ~(uint8)PENDING_GARBAGE_COLLECT_JOB;
            }
            // set pending erase job to erase block freed by garbage collect
            AdminFls.pendingJob |= (uint8) PENDING_ERASE_JOB;
        }
    }else{
        /* Do nothing */
    }
}

/***************************************
 *    External accessible functions    *
 ***************************************/
/*
 * Procedure:    Fee_Init
 * Reentrant:    No
 */
void Fee_Init(void)
{
    /* Reporting information */
    /* @req FEE120 */
    SET_FEE_STATUS(MEMIF_BUSY_INTERNAL);
    SET_FEE_JOBRESULT(MEMIF_JOB_OK);

    memset(&CurrentJob, 0 , sizeof(CurrentJob));
    memset(&AdminFls, 0 , sizeof(AdminFls));

    /* State of device */
    AdminFls.State = FEE_STARTUP_REQUESTED;
#if (FEE_POLLING_MODE == STD_OFF)
    FlsJobReady = TRUE;
#endif
    for(uint16 i = 0; i < FEE_NUM_OF_BLOCKS; i++) {
        AdminFls.GarbageCollect.TotalFeeSize += PAGE_ALIGN(Fee_Config.BlockConfig[i].BlockSize) + ADMIN_SIZE;
    }
}


/*
 * Procedure:    Fee_SetMode
 * Reentrant:    No
 */
void Fee_SetMode(MemIf_ModeType mode)
{
    /* @req FEE121 */
    DET_VALIDATE(ModuleStatus != MEMIF_UNINIT, FEE_SET_MODE_ID, FEE_E_UNINIT);
    /* @req FEE170 */
    DET_VALIDATE(ModuleStatus != MEMIF_BUSY, FEE_SET_MODE_ID, FEE_E_BUSY);
    /* @req FEE171 */
    DET_VALIDATE(ModuleStatus != MEMIF_BUSY_INTERNAL, FEE_SET_MODE_ID, FEE_E_BUSY_INTERNAL);

#if ( FLS_SET_MODE_API == STD_ON )
    /* @req FEE020 */
    Fls_SetMode(mode);
#else
    (void)mode; /* Avoid compiler warning */
    DET_REPORTERROR(FEE_SET_MODE_ID, FEE_E_NOT_SUPPORTED);
#endif
}

/*
 * Procedure:    Fee_Read
 * Reentrant:    No
 */
Std_ReturnType Fee_Read(uint16 blockNumber, uint16 blockOffset, uint8* dataBufferPtr, uint16 length)
{
    Std_ReturnType status;
    status = E_OK;
    uint16 blockIndex;

    DEBUG_PRINTF("Fee_Read %d %d\n",blockNumber, blockOffset);
    /* @req FEE122 */
    DET_VALIDATE_RV(ModuleStatus != MEMIF_UNINIT, FEE_READ_ID, FEE_E_UNINIT, E_NOT_OK);
    if(FEE_CORRUPTED == AdminFls.State){
    	status = E_NOT_OK;
    } else {
		/* @req FEE133 *//* @req FEE172 *//* @req FEE173 */
		if(ModuleStatus != MEMIF_IDLE) {
			DET_REPORTERROR(FEE_READ_ID, (ModuleStatus == MEMIF_BUSY_INTERNAL) ? FEE_E_BUSY_INTERNAL : FEE_E_BUSY);
			/*lint -e{904} Return statement is necessary in case of reporting a DET error */
			return E_NOT_OK;
		}

		blockIndex = GetBlockIdxFromBlockNumber(blockNumber);
		/* @req FEE134 */
		DET_VALIDATE_RV(blockIndex < FEE_NUM_OF_BLOCKS, FEE_READ_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);
		/* @req FEE136 */
		DET_VALIDATE_RV(dataBufferPtr != NULL, FEE_READ_ID, FEE_E_INVALID_DATA_PTR, E_NOT_OK);
		/* @req FEE135 */
		DET_VALIDATE_RV(blockOffset < Fee_Config.BlockConfig[blockIndex].BlockSize, FEE_READ_ID, FEE_E_INVALID_BLOCK_OFS, E_NOT_OK);
		/* @req FEE137 */
		DET_VALIDATE_RV((blockOffset + length) <= Fee_Config.BlockConfig[blockIndex].BlockSize, FEE_READ_ID, FEE_E_INVALID_BLOCK_LEN, E_NOT_OK);
		/* @req FEE162 *//* Will have returned above */

		/** @req FEE022 */
		SET_FEE_STATUS(MEMIF_BUSY);
		SET_FEE_JOBRESULT(MEMIF_JOB_PENDING);

		CurrentJob.Read.BlockIdx = blockIndex;
		CurrentJob.Read.DataOffset = blockOffset;
		CurrentJob.Read.DataPtr = dataBufferPtr;
		CurrentJob.Read.Length = length;
		AdminFls.pendingJob |= (uint8)PENDING_READ_JOB;
    }
    return status;
}


/*
 * Procedure:    Fee_Write
 * Reentrant:    No
 */
/* @req FEE088 */
Std_ReturnType Fee_Write(uint16 blockNumber, uint8* dataBufferPtr)
{
    /* !req FEE153 */
    /* !req FEE154 */
    Std_ReturnType status;
    status = E_OK;
    uint16 blockIndex;
    /* @req FEE123 */
    DET_VALIDATE_RV(ModuleStatus != MEMIF_UNINIT, FEE_WRITE_ID, FEE_E_UNINIT, E_NOT_OK);
    if(FEE_CORRUPTED == AdminFls.State){
    	status = E_NOT_OK;
    } else {
		/* @req FEE144 *//* @req FEE174 *//* @req FEE175 */
		if(ModuleStatus != MEMIF_IDLE) {
			DET_REPORTERROR(FEE_WRITE_ID, (ModuleStatus == MEMIF_BUSY_INTERNAL) ? FEE_E_BUSY_INTERNAL : FEE_E_BUSY);
			/*lint -e{904} Return statement is necessary in case of reporting a DET error */
			return E_NOT_OK;
		}

		blockIndex = GetBlockIdxFromBlockNumber(blockNumber);
		/* @req FEE138 */
		DET_VALIDATE_RV(blockIndex < FEE_NUM_OF_BLOCKS, FEE_WRITE_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);
		/* @req FEE139 */
		DET_VALIDATE_RV(dataBufferPtr != NULL, FEE_WRITE_ID, FEE_E_INVALID_DATA_PTR, E_NOT_OK);
		/* @req FEE163 *//* Will have returned above */

		/** @req FEE025 */
		SET_FEE_STATUS(MEMIF_BUSY);
		SET_FEE_JOBRESULT(MEMIF_JOB_PENDING);

		CurrentJob.Write.BlockIdx = blockIndex;
		CurrentJob.Write.DataPtr = dataBufferPtr;
		CurrentJob.Write.Invalidate = 0;
		AdminFls.pendingJob |= (uint8)PENDING_WRITE_JOB;
    }
    return status;
}


/*
 * Procedure:    Fee_Cancel
 * Reentrant:    No
 */
void Fee_Cancel(void)
{
    DET_REPORTERROR(FEE_CANCEL_ID, FEE_E_NOT_IMPLEMENTED_YET);
}


/*
 * Procedure:    Fee_GetStatus
 * Reentrant:    No
 */
MemIf_StatusType Fee_GetStatus(void)
{
    DEBUG_PRINTF("%s: %s\n",__FUNCTION__,memIfStatusToStr[ModuleStatus]);
    /* @req FEE034 *//* Module status initialized to MEMIF_UNINIT */
    /* @req FEE128 */
    /* @req FEE129 */
    return ModuleStatus;
}


/*
 * Procedure:    Fee_GetJobResult
 * Reentrant:    No
 */
MemIf_JobResultType Fee_GetJobResult(void)
{
    /* @req FEE125 */
    DET_VALIDATE_RV(ModuleStatus != MEMIF_UNINIT, FEE_GET_JOB_RESULT_ID, FEE_E_UNINIT, MEMIF_JOB_FAILED);

    DEBUG_PRINTF("%s: %s\n",__FUNCTION__,memIfJobToStr[JobResult]);
    /* @req FEE035 */
    /* @req FEE156 */
    /* !req FEE157 */
    /* @req FEE158 */
    /* @req FEE159 */
    /* @req FEE160 */
    return JobResult;
}


/*
 * Procedure:    Fee_InvalidateBlock
 * Reentrant:    No
 */
Std_ReturnType Fee_InvalidateBlock(uint16 blockNumber)
{
    /* @req FEE037 *//* Trigger write job where block is invalidated */
    uint16 blockIndex;
    Std_ReturnType status;
    status = E_OK;
    /* @req FEE126 */
    DET_VALIDATE_RV(ModuleStatus != MEMIF_UNINIT, FEE_INVALIDATE_BLOCK_ID, FEE_E_UNINIT, E_NOT_OK);
    if(FEE_CORRUPTED == AdminFls.State){
    	status = E_NOT_OK;
    } else {
		/* @req FEE145 *//* @req FEE176 *//* @req FEE177 */
		if( !(ModuleStatus == MEMIF_IDLE) ) {
			DET_REPORTERROR(FEE_INVALIDATE_BLOCK_ID, (ModuleStatus == MEMIF_BUSY_INTERNAL) ? FEE_E_BUSY_INTERNAL : FEE_E_BUSY);
			/*lint -e{904} Return statement is necessary in case of reporting a DET error */
			return E_NOT_OK;
		}

		blockIndex = GetBlockIdxFromBlockNumber(blockNumber);
		/* @req FEE140 */
		DET_VALIDATE_RV(blockIndex < FEE_NUM_OF_BLOCKS, FEE_INVALIDATE_BLOCK_ID, FEE_E_INVALID_BLOCK_NO, E_NOT_OK);
		/* @req FEE165 *//* Will return above */

		SET_FEE_STATUS(MEMIF_BUSY);
		SET_FEE_JOBRESULT(MEMIF_JOB_PENDING);

		CurrentJob.Write.BlockIdx = blockIndex;
		CurrentJob.Write.Invalidate = 1;
		AdminFls.pendingJob |= (uint8)PENDING_WRITE_JOB;
    }
    return status;
}


/*
 * Procedure:    Fee_EraseImmediateBlock
 * Reentrant:    No
 */
Std_ReturnType Fee_EraseImmediateBlock(uint16 blockNumber)
{
    (void)blockNumber; /* Avoid compiler warning */
    DET_REPORTERROR(FEE_ERASE_IMMEDIATE_ID, FEE_E_NOT_IMPLEMENTED_YET);
    return E_NOT_OK;
}


/***************************************
 *         Scheduled functions         *
 ***************************************/
/*
 * Procedure:    Fee_MainFunction
 * Reentrant:    No
 */
/* @req FEE097 */
void Fee_MainFunction(void)
{
    /* @req FEE057 */
    DEBUG_PRINTF("State: %s (Fee_MainFunction())\n",stateToStr[AdminFls.State]);

    switch (AdminFls.State) {
        case FEE_UNINITIALIZED:
            break;

        case FEE_IDLE:
            Idle();
            break;
        /*
         * Startup states
         */
        case FEE_STARTUP_REQUESTED:
            StartupRequested();
            break;
        case FEE_STARTUP_READ_BANK_HEADER:
            StartupReadBankHeader();
            break;
        case FEE_STARTUP_READ_BANK_HEADER_WAIT:
            StartupReadBankHeaderWait();
            break;
        case FEE_STARTUP_READ_BLOCK_ADMIN:
            StartupReadBlockAdmin();
            break;
        case FEE_STARTUP_READ_BLOCK_ADMIN_WAIT:
            StartupReadBlockAdminWait();
            break;
        /*
         *  Read states
         */
        /* @req FEE073 */
        case FEE_READ:
            Reading();
            break;

        case FEE_READ_WAIT:
            ReadWait();
            break;
        /*
         * Write states
         */
        /* @req FEE026 */
        case FEE_WRITE:
            WriteStartJob();
            break;
        case FEE_WRITE_ADMIN_DATA:
            WriteAdminData();
            break;
        case FEE_WRITE_ADMIN_DATA_WAIT:
            WriteAdminDataWait();
            break;
        case FEE_WRITE_DATA:
            WriteData();
            break;
        case FEE_WRITE_DATA_WAIT:
            WriteDataWait();
            break;
        case FEE_WRITE_ADMIN_ID:
            WriteAdminId();
            break;
        case FEE_WRITE_ADMIN_ID_WAIT:
            WriteAdminIdWait();
            break;
        case FEE_WRITE_CHECK_ADMIN_DATA:
            WriteCheckAdminData();
            break;
        case FEE_WRITE_CHECK_ADMIN_DATA_WAIT:
            WriteCheckAdminDataWait();
            break;
        /*
         * Erase bank states
         */
        case FEE_ERASE_BANK:
            EraseBank();
            break;
        case FEE_ERASE_BANK_WAIT:
            EraseBankWait();
            break;
        case FEE_WRITE_BANK_HEADER:
            WriteBankHeader();
            break;
        case FEE_WRITE_BANK_HEADER_WAIT:
            WriteBankHeaderWait();
            break;
        /*
         * Garbage collection states
         */
        case FEE_GARBAGE_COLLECT_WRITE_ADMIN_DATA:
            WriteAdminData();
            break;
        case FEE_GARBAGE_COLLECT_WRITE_ADMIN_DATA_WAIT:
            GarbageCollectWriteAdminDataWait();
            break;
        case FEE_GARBAGE_COLLECT_READ:
            GarbageCollectRead();
            break;

        case FEE_GARBAGE_COLLECT_READ_WAIT:
            GarbageCollectReadWait();
            break;
        case FEE_GARBAGE_COLLECT_WRITE_DATA:
            GarbageCollectWriteData();
            break;
        case FEE_GARBAGE_COLLECT_WRITE_DATA_WAIT:
            GarbageCollectWriteDataWait();
            break;
        case FEE_GARBAGE_COLLECT_WRITE_ADMIN_ID:
            WriteAdminId();
            break;
        case FEE_GARBAGE_COLLECT_WRITE_ADMIN_ID_WAIT:
            GarbageCollectWriteAdminIdWait();
            break;
        case FEE_GARBAGE_COLLECT_WRITE_CHECK_ADMIN_DATA:
            GarbageCollectWriteCheckAdminData();
            break;
        case FEE_GARBAGE_COLLECT_WRITE_CHECK_ADMIN_DATA_WAIT:
            GarbageCollectWriteCheckAdminDataWait();
            break;
        /*
         * Corrupted state
         */
        case FEE_CORRUPTED:
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
 * Procedure:    Fee_JobEndNotification
 * Reentrant:    No
 */
/* @req FEE095 */
void Fee_JobEndNotification(void)
{
    /* @req FEE052 *//* Done asynchronously */
    /* @req FEE142 *//* If no job is being processed, flag never checked */
    FlsJobReady = TRUE;
}


/*
 * Procedure:    Fee_JobErrorNotification
 * Reentrant:    No
 */
/* @req FEE096 */
void Fee_JobErrorNotification(void)
{
    /* @req FEE054 *//* Done asynchronously */
    /* @req FEE143 *//* If no job is being processed, flag never checked */
    FlsJobReady = TRUE;
}
#endif
