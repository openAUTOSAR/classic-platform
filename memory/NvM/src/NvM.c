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
 * Author: Peter+mahi
 *
 * Part of Release:
 *   4.0.3
 *
 * Description:
 *   Implements the NVRAM Manager module.
 *
 * Support:
 *   General                  Have Support
 *   -------------------------------------------
 *   NVM_API_CONFIG_CLASS           Y NVM_API_CONFIG_CLASS_1 and NVM_API_CONFIG_CLASS_2
 *                                    (Only NvM_InvalidateNvBlock of NVM_API_CONFIG_CLASS_3 )
 *   NVM_COMPILED_CONFIG_ID         N
 *   NVM_CRC_NUM_OF_BYTES           N
 *   NVM_DATASET_SELECTION_BITS     Y
 *   NVM_DEV_ERROR_DETECT           Y
 *   NVM_DRV_MODE_SWITCH            N
 *   NVM_DYNAMIC_CONFIGURATION      N
 *   NVM_JOB_PRIORITIZATION         N
 *   NVM_MULTI_BLOCK_CALLBACK       Y
 *   NVM_POLLING_MODE               N
 *   NVM_SET_RAM_BLOCK_STATUS_API   Y
 *   NVM_SIZE_IMMEDIATE_JOB_QUEUE   N
 *   NVM_SIZE_STANDARD_JOB_QUEUE    Y
 *   NVM_VERSION_INFO_API           Y
 *
 *   NvmBlockDescriptor        Have Support
 *   -------------------------------------------
 *   NvmBlockCRCType                Y
 *   NvmBlockJobPriority            N
 *   NvmBlockManagementType         Y, All blocks supported
 *   NvmBlockUseCrc                 Y
 *   NvmBlockWriteProt              N
 *   NvmCalcRamBlockCrc             Y
 *   NvmInitBlockCallback           Y
 *   NvmNvBlockBaseNumber           Y
 *   NvmNvBlockLength               Y
 *   NvmNvBlockNum                  Y
 *   NvmNvramBlockIdentifier        Y
 *   NvmNvramDeviceId               N (always device Id 0)
 *   NvmResistantToChangedSw        N
 *   NvmRomBlockDataAddress         Y
 *   NvmRomBlockNum                 Y
 *   NvmSelectBlockForReadall       Y
 *   NvmSingleBlockCallback         Y
 *   NvmWriteBlockOnce              N
 *
 *  Implementation notes:
 *   - The Configuration ID NV Block is generated to the configuration but can't be configured.
 *     The Editor should really force you to pick a block in Ea/Fee that should hold the configuration
 *     ID. The NVM_COMPILED_CONFIG_ID is always generated as 0 now.
 *   - Differences from 3.1.5 Release (Follow release 4.0.2 here)
 *     NvM_SetDataIndex(), NvM_GetDataIndex, NvM_SetBlockProtection, NvM_GetErrorStatus, NvM_SetRamBlockStatus,
 *     etc....all return Std_ReturnType instead of void since the RTE expects it.
 *   - NvM_GetErrorStatus() uses NvM_GetErrorStatus instead of uint8 *
 */

/*
 * RamBlockDataAddress
 *   NULL is no permanent RAM block. Otherwise allocate the number of bytes in space (like the stack)
 *
 *
 * Understanding block numbering:
 *
 *  NVM_DATASET_SELECTION_BIT=2
 *
 *    NvBlockBaseNumber
 *          0     Reserved (NVM478)
 *          1     NVM_BLOCK_NATIVE,    NvBlockNum=1
 *          2     NVM_BLOCK_REDUNDANT, NvBlockNum=2
 *          3     NVM_BLOCK_DATASET,   NvBlockNum=4
 *
 *   NvM_ReadBlock( 0, ... )    - Reserved for "multi block requests"
 *   NvM_ReadBlock( 1, ... )    - Reserved for redundant NVRAM block which holds the configuration ID.
 *   NvM_ReadBlock( 2--x, ... ) - "Normal" blocks
 *
 *
 *  NvM_BlockIdType*)  NvBlockBaseNumber   EA_BLOCK_NUMBER
 *      0**)
 *      1**)                ***)
 *      2                   1                4, (5,6,7)  ****)
 *      3                   2                8, R9,(10,11)  ****)
 *      4                   3                12, D13,D14,D15 ****)
 *
 *  Should
 *
 *
 *  *) Type used in the API.
 *  **)   Reserved ID's ( 0 - multiblock, 1 - redundant NVRAM block which hold configuration ID)
 *  ***)  Reserved ID
 *  ****) FEE/EA_BLOCK_NUMBER = NvBlockBaseNumber << NvmDatasetSelectionBits = NvBlockBaseNumber * 4
 *        () - Cannot be accesses due to NvBlockNum
 *        R9 - Redundant block
 *        Dx - Data blocks
 *
 *    SIZES
 *      Both NvM and EA/FEE have block sizes. NvM have NvNvmBlockLength (NVM479) and FEE/EA have EaBlockSize.
 *      FEE/EA also have virtual page that is the alignment of a block, with the smallest block=size of the virtual page.
 *
 *      So, who allocates space for this. FEE/EA only have EaBlockSize.
 *      NvM have NvmRamBlockDataAddress, NvmRomBlockDataAddress and mapping to a MemIf Blocks (FEE/EA blocks)
 *
 *      ASSUME: I can't really see a point for the FEE/EA EaBlockSize.. it will just a multiple of NvNvmBlockLength*NvBlockNum + overhead?
 *              Click-box in EA/FEE that leaves the size calculation to NvM?
 *              This also means that enabling NvmBlockUseCrc or set a block from NVM_BLOCK_NATIVE to NVM_BLOCK_DATASET would be "automatic"
 *              in calculation of the EaBlockSize.
 *
 *      So how much data should be read from MemIf if a CRC checksum is used. Assuming that we use a physical layout where the CRC is located
 *      after the NV Block it would be BALIGN(NvNvmBlockLength,4) + 4 bytes. The same applies to the RAM block (ROM block to not have CRC, NVM127)
 *
 * CRC
 *   NVM121: NvM_SetRamBlockStatus(), calculate CRC in background if NvmCalcRamBlockCrc==TRUE
 *   NVM362: NvM_ReadAll() , if (NvmCalcRamBlockCrc == TRUE && permanent RAM block) re-calc CRC.
 *
 *     NvmBlockUseCrc     - Global CRC switch. Space is allocated in both RAM and NV block
 *     NvmCalcRamBlockCrc - CRC re-calculation. For example during startup, you may don't want
 *                          to verify all the checksums again, but instead just copy from NV
 *                          to RAM.
 *
 *     There is also explicit RAM checksum calculations for example NVM253 (Nvm_WriteAll()) where a CRC calculation
 *     is requested (although not stated should be dependent on NvmBlockUseCrc only, not NvmCalcRamBlockCrc).
 *     You have to calculate the checksum at some point....
 *
 * QUEUES
 *   NVM185: Says "successful enqueueing a request".."set to NVM_REQ_PENDING"
 *   NVM380: The queue length for multi block request shall be one (NvM_ReadAll, NvM_WriteAll)
 *   NVM381+NVM567 : Multi block requests shall not be interrupted.
 *   NVM243+NVM245: Do not unqueue the multi block request until all single block queues are empty.
 *     So, when NvmState==IDLE and we have a multi-request
 *
 *
 *
 *
 *
 * SPEED
 *   To get some speed into this multiple thing must be done in the same MainFunction loop.
 *
 * MEMIF
 *   The interface is actually quite strange, so you may read from any address (through MemIf_Read())
 *   but MemIf_Write(..) only takes a logical block and a pointer to the data to write.
 *
 *   See two alternatives here:
 *    1. If the NVBlock also contains the checksum after the data then writing the data needs a
 *       RAM that is as big as the biggest RAM block + room for checksum.
 *    2. If checksums would be kept in a separate EA/FEE block the ALL the checksum need to be written.
 *       For example after a NvM_WriteBlock() the checksum block would need to be written. This
 *       will probably lead consistency problems also... what happens if we fail to write the checksum
 *       block?
 *
 *
 *
 * MANUAL
 *
 *
 *
 *   Provide Data for the first/initial read
 *     When a block have no
 *
 *     NVM085
 *     NVM061
 *     NVM083
 *
 *   Configuring CRCs
 *     BlockUseCrc (A):      If the block (both RAM and NV) should use CRC
 *     CalcRamBlockCrc (B):  If the permanent RAM block should re-calculate it's CRC.
 *
 *     A B
 *     ------------
 *     0 0  No error detection or recovery
 *     0 1  N/A
 *     1 0  ?
 *     1 1  ?
 *
 * RAM BLOCK VALID/UNCHANGED
 *   Figure 8 and 9 in 3.1.5/NVM is more confusing than good.
 *   What we have to know is:
 *    1. Initially the RAM block is in INVALID/UNCHANGED
 *    ALT 2. After a NvM_ReadAll() and all is well the state goes to VALID/UNCHANGED
 *    ALT 2. If ROM values are used we go to VALID/CHANGED (We failed to read from NVRAM)
 *
 *   For NvM_WriteAll()
 *   1. A block that is INVALID can't be written
 *   2. A block that is UNCHANGED should not be written.
 *   -> Write only blocks that are VALID/CHANGED.
 *
 *   VALID/UNCHANGED - RAM == NV
 *   VALID/CHANGED   - RAM != NV   (analog to cache memories, "dirty")
 *
 *   Analog to cache
 *     VALID/CHANGED state - Dirty (since RAM != NV)
 *     WriteBlock          - Flush (Flush the RAM block to NV)
 *     ReadBlock           - Invalidate (NV block is read to RAM)
 */


/*
 *  General requirements
 */
/** @req NVM076 */
/** @req NVM552 */
/** @req NVM689 */
/** @req NVM446 *//* Included types */
/** @req NVM475 *//* Block IDs in sequential order */
/** @req NVM076 *//* One or more c files containing NvM code */
/** @req NVM051 *//* The Memory Abstraction Interface provide the NvM module with a virtual linear 32bit address space */
/** @req NVM000 *//* The Native NVRAM block consists of a single NV block, RAM block and Administrative block. */
/** @req NVM001 *//* The Redundant NVRAM block consists of two NV blocks, a RAM block and an Administrative block. */
/** @req NVM006 *//* The Dataset NVRAM block consists of multiple NV user data, (optionally) CRC areas, (optional) NV block headers, a RAM block and an Admin block. */
/** @req NVM374 *//* The NvM module shall be able to read all assigned NV blocks. */
/** @req NVM375 *//* The NvM module shall only be able to write to all assigned NV blocks if (and only if) write protection is disabled. */
/** @req NVM380 *//* Job queue length for multi block requests should be one */
/** @req NVM568 *//* The NvM module shall rather queue read jobs that are requested during an ongoing NvM_ReadAll request and executed them subsequently */
/** @req NVM569 *//* The NvM module shall rather queue write jobs that are requested during an ongoing NvM_WriteAll request and executed them subsequently */
/** @req NVM725 *//* The NvM module shall rather queue write jobs that are requested during an ongoing NvM_ReadAll request and executed them subsequently */
/** @req NVM726 *//* The NvM module shall rather queue read jobs that are requested during an ongoing NvM_WriteAll request and executed them subsequently. */
/** @req NVM160 *//* The NvM module shall not store the currently used Dataset index automatically in a persistent way. */
/** @req NVM162 *//* Requests queued (although no support for priority) */
/** @req NVM164 *//* The NvM module shall provide implicit techniques to check the data consistency of NVRAM blocks */
/** @req NVM571 *//* The data consistency check of a NVRAM block shall be done by CRC recalculations of its corresponding NV block(s). */
/** @req NVM165 *//* The implicit consistency check shall be configurable for each NVRAM block (parameters NvMBlockUseCrc and NvMCalcRamBlockCrc) */
/** @req NVM020 *//* The ROM block is a basic storage object, resides in the ROM (FLASH) and is used to provide default data in case of an empty or damaged NV block. */
/** @req NVM040 *//* The NvM module shall implement implicit mechanisms for consistency / integrity checks of data saved in NV memory */
/** @req NVM047 *//* The NvM module shall provide techniques for error recovery. The error recovery depends on the NVRAM block management type */
/** @req NVM069 *//* A single NVRAM block to deal with will be selected via the NvM module API by providing a subsequently assigned Block ID. */
/** @req NVM138 *//* The NVRAM block shall consist of the mandatory basic storage objects NV block, RAM block and Administrative block. */
/** @req NVM171 *//* The NvM module shall provide implicit(fallback on read failure) and explicit(NvM_RestoreBlockDefaults) recovery techniques */
/** @req NVM174 *//* The detection of an incomplete write operation to a NV block is out of scope of the NvM module. Handled by lower layer */
/** @req NVM175 *//* Request result indicated in the error/status field of the Admin block */
/** @req NVM181 *//* Multiple concurrent single block requests shall be queueable. */
/** @req NVM373 *//* The RAM block data shall contain the permanently or temporarily assigned user data. */
/** @req NVM396 *//* The multi block requests shall indicate individual block status */
/** @req NVM443 *//* The NvM module shall not modify the configured block identifiers. */
/** @req NVM038 *//* The NvM module only provides an implicit way of accessing blocks in the NVRAM and in the shared memory (RAM). */
/** @req NVM125 *//* The NV block represents a memory area consisting of NV user data and (optionally) a CRC and (optionally) a NV block header.*/
/** @req NVM126 *//* The RAM block  represents an area in RAM consisting of user data and (optionally) a CRC value and (optionally) a NV block header. */
/** @req NVM127 *//* CRC on RAM only used if */
/** @req NVM129 *//* The user data area of a RAM block can reside in a different RAM address location (global data section) than the state of the RAM block. */
/** @req NVM130 *//* The data area of a RAM block shall be accessible from NVRAM Manager and from the application side */
/** @req NVM332 *//* NvM relies on the scheduler. Do not use any OS objects or services */
/** @req NVM557 *//* Configuration validated */
/** @req NVM558 *//* Configuration validated */
/** @req NVM559 *//* Configuration validated */
/** @req NVM696 *//* Application responsible for providing valid data before next write in case of failed read */

/* Not supported(?) unclear requirements */
/** !req NVM573 *//* What does this mean? Should crc be checked in RAM before attempting to read? */
/** !req NVM574 *//* What does this mean? Should crc be checked in RAM before attempting to read? */
/** !req NVM575 *//* What does this mean? Should crc be checked in RAM before attempting to read? */
/** !req NVM123 *//* What does this mean? Store where? */


/*
 * NB! Even though some code exist for handling RamCrc, the functionality is not complete
 * and shall not be used.
 */

//lint -esym(522,CalcCrc) // 522 PC-Lint exception for empty functions

// Exception made as a result of that NVM_DATASET_SELECTION_BITS can be zero
//lint -emacro(835, BLOCK_BASE_AND_SET_TO_BLOCKNR) // 835 PC-lint: A zero has been given as right argument to operator '<<' or '>>'

//lint -emacro(506, NVM_ASSERT) // 506 PC-lint: Ok, to have constant value.

/*lint -emacro(904,DET_VALIDATE_NO_RV)*/ /*904 PC-Lint exception to MISRA 14.7 (validate DET macros)*/
/* ----------------------------[includes]------------------------------------*/
/* @req NVM554 *//* NvM module shall include NvM.h, Dem.h, MemIf.h, SchM_NvM.h, MemMap.h. */
#include "arc_assert.h"
#include "NvM.h"
#include "NvM_Cbk.h"
#if defined(CFG_NVM_USE_SERVICE_PORTS)
#include "Rte.h" // ???
#endif
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include "MemIf.h"
#include "SchM_NvM.h"
#include "MemMap.h"
#include "cirq_buffer.h"

#include "io.h"
/* @req NVM555 */
#include "Crc.h"
#include <string.h>
#include "Cpu.h"

//#define DEBUG_BLOCK    1
#if defined(DEBUG_BLOCK)
#define DEBUG_BLOCK_STATE(_str,_block,_state)       printf("%s BLOCK NR:%d STATE:%d\n",_str,_block, _state); fflush(stdout);
#define DEBUG_STATE(_state,_substate)               printf("MAIN_STATE:%s/%d\n",StateToStr[_state],_substate); fflush(stdout);
#define DEBUG_PRINTF(format,...)                    printf(format,## __VA_ARGS__ ); fflush(stdout);
#define DEBUG_CHECKSUM(_str,_crc)                   printf("%s crc=%x\n",_str,_crc);
#define DEBUG_FPUTS(_str)                           fputs((_str),stdout); fflush(stdout);
#else
#define DEBUG_BLOCK_STATE(_str,_block,_state)
#define DEBUG_STATE(_state,_substate)
#define DEBUG_PRINTF(format,...)
#define DEBUG_CHECKSUM(_str,_crc)
#define DEBUG_FPUTS(_str)
#endif


/* ----------------------------[private define]------------------------------*/


/* ----------------------------[private macro]-------------------------------*/

#if defined(DEBUG_BLOCK)
#define NVM_ASSERT(_exp)        if( !(_exp) ) { while(1) {}; } //ASSERT(_exp)
#else
#define NVM_ASSERT(_exp)        if( !(_exp) ) { ASSERT(_exp); } //
#endif


/*lint -emacro(904,DET_VALIDATE_RV)*/ /*904 PC-Lint exception to MISRA 14.7 (validate DET macros)*/

/* @req NVM025 *//* @req NVM596 *//* @req NVM597 *//* @req NVM188 *//* @req NVM191 */
#if  ( NVM_DEV_ERROR_DETECT == STD_ON )
#if defined(USE_DET)
/* @req NVM556 */
#include "Det.h"
#endif
#define DET_VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(NVM_MODULE_ID, 0, _api, _err); \
        }


#define DET_VALIDATE_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(NVM_MODULE_ID, 0, _api, _err); \
          return _rv; \
        }

#define DET_VALIDATE_NO_RV(_exp,_api,_err ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(NVM_MODULE_ID, 0, _api, _err); \
          return; \
        }
#define DET_REPORTERROR(_api,_err) (void)Det_ReportError(NVM_MODULE_ID, 0,_api,_err)

#else
#define DET_VALIDATE(_exp,_api,_err )
#define DET_VALIDATE_RV(_exp,_api,_err,_rv )
#define DET_VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_api,_err)
#endif

/* @req NVM122 */
#define BLOCK_BASE_AND_SET_TO_BLOCKNR(_blockbase, _set) ((uint16)((_blockbase)<<NVM_DATASET_SELECTION_BITS) +(_set))

/* @req NVM189 *//* Cannot be disabled unless Dem is not used at all */
/* @req NVM026 */
#if defined(USE_DEM)
/* @req NVM739 */
#define DEM_REPORTERRORSTATUS(_err,_ev )  if (_err != DEM_EVENT_ID_NULL) { Dem_ReportErrorStatus(_err, DEM_EVENT_STATUS_FAILED); }

#else
#define DEM_REPORTERRORSTATUS(_err,_ev )
#endif


#define BLOCK_NR_FROM_PTR(_bptr)        (((_bptr) - NvM_Config.BlockDescriptor + 1))   // sizeof(NvM_BlockDescriptorType))

/*lint --e{773}  Expression-like macro 'CREATE_ENTRY' not parenthesized  */
#define CREATE_ENTRY(_val)  [_val] = #_val



/* ----------------------------[private typedef]-----------------------------*/

// State variable
typedef enum {
  NVM_UNINITIALIZED = 0,
  NVM_IDLE,
  NVM_READ_ALL,
  NVM_WRITE_ALL,
  NVM_READ_BLOCK,
  NVM_WRITE_BLOCK,
  NVM_RESTORE_BLOCK_DEFAULTS,
  NVM_SETDATAINDEX,
  NVM_GETDATAINDEX,
  NVM_SETRAMBLOCKSTATUS,
  NVM_INVALIDATE_NV_BLOCK,
} NvmStateType;

const char_t *StateToStr[NVM_INVALIDATE_NV_BLOCK] = {
    CREATE_ENTRY(NVM_UNINITIALIZED),
    CREATE_ENTRY(NVM_IDLE),
    CREATE_ENTRY(NVM_READ_ALL),
    CREATE_ENTRY(NVM_WRITE_ALL),
    CREATE_ENTRY(NVM_READ_BLOCK),
    CREATE_ENTRY(NVM_WRITE_BLOCK),
    CREATE_ENTRY(NVM_RESTORE_BLOCK_DEFAULTS),
    CREATE_ENTRY(NVM_SETDATAINDEX),
    CREATE_ENTRY(NVM_GETDATAINDEX),
    CREATE_ENTRY(NVM_SETRAMBLOCKSTATUS),
 };


typedef enum {
    BLOCK_STATE_MEMIF_REQ,
//  BLOCK_STATE_START,
    BLOCK_STATE_MEMIF_PROCESS,
//  BLOCK_STATE_MEMIF_CRC_PROCESS,
    BLOCK_STATE_CALC_CRC,
//  BLOCK_STATE_MEMIF_PROCESS_CRC,
    BLOCK_STATE_CALC_CRC_WRITE,
    BLOCK_STATE_CALC_CRC_READ,
//  BLOCK_STATE_LOAD_FROM_NV,
} BlockStateType;


typedef enum {
    BLOCK_SUBSTATE_0,
    BLOCK_SUBSTATE_1,
} BlockSubStateType;


typedef enum {
    NS_INIT = 0,
    NS_PENDING,
//  RB_PROCESSING,
} Nvm_SubStates;


union Nvm_CRC {
    uint16 crc16;
    uint32 crc32;
};


typedef struct {
    NvM_RequestResultType    ErrorStatus;            // Status from multi block requests i.e. Read/Write/CancelWrite-all @req NVM393
} AdministrativeMultiBlockType;

typedef struct {
    BlockStateType           BlockState;
    BlockSubStateType        BlockSubState;
    uint8                    DataIndex;              // Selected data index if "Data Set" type @req NVM144
    boolean                  BlockWriteProtected;    // Block write protected? @req NVM054
    NvM_RequestResultType    ErrorStatus;            // Status of block @req NVM136
    boolean                  BlockChanged;           // Block changed?
    boolean                  BlockValid;             // Block valid? (RAM block only?) @req NVM128 @req NVM132 @req NVM133
    uint8                    NumberOfWriteFailed;    // Current write retry cycle
    union Nvm_CRC            RamCrc;
    union Nvm_CRC            NvCrc;                  // The CRC of this block, read from NV
    void *                   savedDataPtr;           //
    uint8                    flags;                  // Used for all sorts of things.
} AdministrativeBlockType;


typedef struct {
    NvmStateType        op;
    NvM_BlockIdType blockId;
    uint8 *         dataPtr;    /* Src or Dest ptr */
    uint8        serviceId;
} Nvm_QueueType;



/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/

static NvmStateType                 nvmState = NVM_UNINITIALIZED;

static Nvm_SubStates                nvmSubState = NS_INIT;
static uint8                        serviceId = NVM_INIT_ID;
//static int nvmSetNr;
/* @req NVM134 *//* @req NVM135 */
static AdministrativeBlockType      AdminBlock[NVM_NUM_OF_NVRAM_BLOCKS];
static AdministrativeMultiBlockType AdminMultiBlock;



#define _ALIGN(_x,_a)    (((_x) + (_a)  - 1u) & (~ ((_a) - 1u)))

#ifndef EA_VIRTUAL_PAGE_SIZE
#define EA_VIRTUAL_PAGE_SIZE 0
#endif
#ifndef FEE_VIRTUAL_PAGE_SIZE
#define FEE_VIRTUAL_PAGE_SIZE 0
#endif

/* Assume undefined is 0 (all compilers do?) */
#if FEE_VIRTUAL_PAGE_SIZE > EA_VIRTUAL_PAGE_SIZE
#define VIRTUAL_PAGE_SIZE FEE_VIRTUAL_PAGE_SIZE
#else
#define VIRTUAL_PAGE_SIZE EA_VIRTUAL_PAGE_SIZE
#endif


#if (NVM_SIZE_STANDARD_JOB_QUEUE == 0)
#error NVM_SIZE_STANDARD_JOB_QUEUE have size 0
#endif


CirqBufferType nvmQueue;


/* ----------------------------[private functions]---------------------------*/

#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)
static void setRamBlockStatus( const NvM_BlockDescriptorType    *bPtr,
                                AdministrativeBlockType *admPtr,
                                boolean blockChanged );
#endif

/* ----------------------------[public functions]----------------------------*/



/*
 * This function needs to be implemented!
 */
static void CalcCrc(void)
{
    // IMPROVEMENT: Calculate CRC
}

typedef struct {
    boolean                         JobFinished;
    Std_ReturnType                  JobStatus;
    MemIf_JobResultType             JobResult;
    const NvM_BlockDescriptorType * BlockDescriptor;
    AdministrativeBlockType *       BlockAdmin;
} MemIfJobAdminType;

static MemIfJobAdminType MemIfJobAdmin = {
        .JobFinished = TRUE,
        .JobStatus = E_OK,
        .JobResult = MEMIF_JOB_OK,
        .BlockDescriptor = NULL,
        .BlockAdmin = NULL
};


typedef struct {
    NvmStateType            state;
    uint16                  currBlockIndex;        // Keeps track of next unfinished block
    NvM_RequestResultType   PendingErrorStatus;    // Status from multi block requests i.e. Read/Write/CancelWrite-all
    uint8                   serviceId;
} AdminMultiReqType;

static AdminMultiReqType AdminMultiReq;


/*
 * Set the MemIf job as busy
 */
static void SetMemifJobBusy()
{
    MemIfJobAdmin.JobFinished = FALSE;
}

/*
 * Abort the MemIf job with E_NOT_OK
 */
static void AbortMemIfJob(MemIf_JobResultType jobResult)
{
    MemIfJobAdmin.JobFinished = TRUE;
    MemIfJobAdmin.JobStatus = E_NOT_OK;
    MemIfJobAdmin.JobResult = jobResult;
}

/*
 * Initiate the read all job
 */
static boolean ReadAllInit(void)
{
    /*
     * Initiate the read all job
     */
    const NvM_BlockDescriptorType   *BlockDescriptorList = NvM_Config.BlockDescriptor;
    AdministrativeBlockType *AdminBlockTable = AdminBlock;
    uint16 i;
    boolean needsProcessing = FALSE;

    // Set status to pending in the administration blocks
    AdminMultiBlock.ErrorStatus = NVM_REQ_PENDING;
    AdminMultiReq.PendingErrorStatus = NVM_REQ_OK;
    AdminMultiReq.currBlockIndex = 0;

    for (i = 0; i < ( NVM_NUM_OF_NVRAM_BLOCKS ); i++) {
        /* @req NVM304 *//* @req NVM118 */
        if ((BlockDescriptorList->SelectBlockForReadall == TRUE)
#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)                        /** @req NVM345 */
                && ((AdminBlockTable->BlockValid==FALSE)            // IMPROVEMENT: Check if this is to be done like this
                || (AdminBlockTable->BlockChanged==FALSE))        // IMPROVEMENT: Check if this is to be done like this
#endif
                ) {
            NVM_ASSERT((BlockDescriptorList->RamBlockDataAddress != NULL));
            DET_VALIDATE_RV((BlockDescriptorList->RamBlockDataAddress != NULL), NVM_READ_ALL_ID, NVM_E_WRONG_CONFIG, FALSE );
            /* @req NVM245 */
            NVM_ASSERT((BlockDescriptorList->BlockManagementType != NVM_BLOCK_DATASET));
            DET_VALIDATE_RV((BlockDescriptorList->BlockManagementType != NVM_BLOCK_DATASET), NVM_READ_ALL_ID, NVM_E_WRONG_CONFIG, FALSE );

            AdminBlockTable->ErrorStatus = NVM_REQ_PENDING;
            AdminBlockTable->BlockState = BLOCK_STATE_MEMIF_REQ;
            needsProcessing = TRUE;
        } else {
            AdminBlockTable->ErrorStatus = NVM_REQ_BLOCK_SKIPPED;    /* @req NVM287 */
        }

        AdminBlockTable++;
        BlockDescriptorList++;
    }

    return needsProcessing;
}


static void writeCrcToBuffer(   uint8 *bufPtr,
                                const NvM_BlockDescriptorType *bPtr,
                                const AdministrativeBlockType *admPtr )
{
    if( bPtr->BlockUseCrc == TRUE ) {
        if(bPtr->BlockCRCType == NVM_CRC16) {
            WRITE16_NA((&bufPtr[bPtr->NvBlockLength]),admPtr->RamCrc.crc16);
        } else {
            WRITE32_NA((&bufPtr[bPtr->NvBlockLength]),admPtr->RamCrc.crc32);
        }
    }
}

#define REDUNDANT_BLOCK_OK                0
#define REDUNDANT_BLOCK_BOTH_FAIL         1


static boolean handleRedundantBlock(const NvM_BlockDescriptorType *bPtr,
                                    AdministrativeBlockType *admPtr ) {
    boolean cont = 0;
    /* According to 3.1.5/NVM137 we have 2 NV Blocks and 0..1 ROM Blocks */
    /* Req 3.1.5/NVM199 , 3.1.5/NVM279 , 3.1.5/NVM317
     * 3.1.5/NVM288, 3.1.5/NVM315
     * */
    /* @req NVM199 */
    /* @req NVM288 */
    /* @req NVM343 *//* Use of data index */
    if ( bPtr->BlockManagementType == NVM_BLOCK_REDUNDANT ) {
        if( admPtr->flags == 1 ) {
            NVM_ASSERT((bPtr->NvBlockNum == 2));        /* Configuration error */
            admPtr->DataIndex = ((admPtr->DataIndex!=0u) ? 0u : 1u);
            admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
            DEBUG_FPUTS(" # First redundant NV block failed\n");
            cont = 1;
        } else {
            DEBUG_FPUTS(" # Both redundant NV blocks failed\n");
        }
        admPtr->flags++;
    }
    return cont;
}

/*
 * Drive the read state-machine
 *
 *
 *
 */

static boolean DriveBlock( const NvM_BlockDescriptorType    *bPtr,
                            AdministrativeBlockType *admPtr,
                            void *dataPtr,
                            boolean isWrite,
                            boolean multiBlock,
                            boolean restoreFromRom )
{
    /* Define a work buffer that needs have size aligned to atleast xxx_VIRTUAL_PAGE_SIZE
     * + 4 is since the buffer may have CRC */
    /* !req NVM544 Depending on the config, NvM module shall allocate memory for the largest CRC used.*/
    static uint8 Nvm_WorkBuffer[ _ALIGN(NVM_MAX_BLOCK_LENGTH,VIRTUAL_PAGE_SIZE) + VIRTUAL_PAGE_SIZE ];


    /* @req NVM270 *//* If the NvM module has successfully processed a job, it shall return NVM_REQ_OK as job result. */
    boolean blockDone;
    boolean status;
    status = TRUE;
    blockDone = 0;
    DEBUG_BLOCK_STATE("DriveBlock", BLOCK_NR_FROM_PTR(bPtr), admPtr->BlockState );

    NVM_ASSERT((admPtr->ErrorStatus == NVM_REQ_PENDING));

    switch (admPtr->BlockState) {
    case BLOCK_STATE_MEMIF_REQ:
    {
        boolean fail = FALSE;
        Std_ReturnType rv;
        void *ramData;
        ramData = (dataPtr != NULL) ?  dataPtr : bPtr->RamBlockDataAddress;

        admPtr->savedDataPtr = ramData;

        if( isWrite == TRUE) {
            /* @req NVM209 *//* NvM_WriteBlock checks write protection. But we don't do it here */
            if( (multiBlock!=0u) && (dataPtr!=NULL)) {
                NVM_ASSERT(FALSE); /*lint !e9027 !e774  Boolean within 'if' always evaluates to True is exception */
            }
            /* Copy to work buffer */
            memcpy( Nvm_WorkBuffer, ramData, bPtr->NvBlockLength );
            /* Add the CRC to write */
            writeCrcToBuffer(Nvm_WorkBuffer, bPtr, admPtr );

            /*
             * Write the block
             */
            /* @req NVM445 */
            if (admPtr->DataIndex < bPtr->NvBlockNum) {
                SetMemifJobBusy();
                MemIfJobAdmin.BlockAdmin = admPtr;
                MemIfJobAdmin.BlockDescriptor = bPtr;

                SchM_Enter_NvM_EA_0();
                /* @req NVM338 *//* @req NVM339 */
                rv = MemIf_Write(bPtr->NvramDeviceId, BLOCK_BASE_AND_SET_TO_BLOCKNR(bPtr->NvBlockBaseNumber, admPtr->DataIndex), Nvm_WorkBuffer);

                if (rv != E_OK) {
                    if ( MemIf_GetStatus(bPtr->NvramDeviceId) == MEMIF_IDLE ) {
                        AbortMemIfJob(MEMIF_JOB_FAILED);
                        fail = TRUE;
                    } else {
                        /* Do nothing. For MEMIF_UNINIT, MEMIF_BUSY and MEMIF_BUSY_INTERNAL we just stay in the
                         * same state. Better in the next run */
                        SchM_Exit_NvM_EA_0();
                        break; /* Do NOT advance to next state */
                    }
                }
                SchM_Exit_NvM_EA_0();
            } else {
                // Error: setNumber out of range
                DET_REPORTERROR(NVM_LOC_WRITE_BLOCK_ID, NVM_PARAM_OUT_OF_RANGE);
                fail = TRUE;
            }

            if (fail==TRUE) {
                /* Fail the job */
                admPtr->ErrorStatus = NVM_REQ_NOT_OK;
                blockDone = 1;
                /* @req NVM680 *//* We do not change the validity of the ram block and it is set to invalid at init */
                break; /* Do NOT advance to next state */
            }

        } else {
            uint8 crcLen = 0;
            /* Read to workbuffer */
            if( bPtr->BlockUseCrc == TRUE ) {
                crcLen = (bPtr->BlockCRCType == NVM_CRC16) ? 2: 4;
            }

            if( restoreFromRom==TRUE ) {
                /* @req NVM267 */
                /* !req NVM266 Restoring defaults via callback not supported */
                /* @req NVM392 */
                NVM_ASSERT(( bPtr->RomBlockDataAdress != NULL ));
                /* No CRC on the ROM block */
                memcpy((void*)ramData,bPtr->RomBlockDataAdress,bPtr->NvBlockLength);

                admPtr->ErrorStatus = NVM_REQ_OK;
                blockDone = 1;
                break;        /* Do NOT advance to next state */
            } else {
                uint8 setNumber = admPtr->DataIndex;
                uint16 length = bPtr->NvBlockLength+crcLen;

                /*
                 * Read the Block
                 */
                /* @req NVM445 */
                if (setNumber < bPtr->NvBlockNum) {
                    SetMemifJobBusy();
                    MemIfJobAdmin.BlockAdmin = admPtr;
                    MemIfJobAdmin.BlockDescriptor = bPtr;

                    /* First reading the MemIf block and then checking MemIf_GetStatus() to determine
                     * if the device BUSY in anyway...is not threadsafe. The way Autosar have defined
                     * it you would have to lock over the MemIf_Read() to be sure.
                     */
                    SchM_Enter_NvM_EA_0();

                    /* We want to read from MemIf, but the device may be busy.
                     */
                    rv = MemIf_Read(bPtr->NvramDeviceId,
                                            BLOCK_BASE_AND_SET_TO_BLOCKNR(bPtr->NvBlockBaseNumber, setNumber),
                                            0,
                                            Nvm_WorkBuffer,
                                            length );
                    if (rv != E_OK) {
                        /* !req NVM657 Default values not loaded */
                        /* !req NVM679 Default values not loaded */
                        if ( MemIf_GetStatus(bPtr->NvramDeviceId) == MEMIF_IDLE ) {
                            AbortMemIfJob(MEMIF_JOB_FAILED);
                            fail = TRUE;
                        } else {
                            /* Do nothing. For MEMIF_UNINIT, MEMIF_BUSY and MEMIF_BUSY_INTERNAL we just stay in the
                             * same state. Better in the next run */
                            SchM_Exit_NvM_EA_0();
                            break; /* Do NOT advance to next state */
                        }
                    }
                    SchM_Exit_NvM_EA_0();

                } else if (setNumber < (bPtr->NvBlockNum + bPtr->RomBlockNum)) {
                    // IMPROVEMENT: Read from ROM
                    /* !req NVM354 *//* !req NVM340 *//* !req NVM376 */
                    fail = TRUE;
                } else {
                    // Error: setNumber out of range
                    DET_REPORTERROR(NVM_LOC_READ_BLOCK_ID, NVM_PARAM_OUT_OF_RANGE);
                    fail = TRUE;
                }

                if( TRUE == fail  ) {
                    /* Fail the job */
                    admPtr->ErrorStatus = NVM_REQ_NOT_OK;
                    blockDone = 1;
                    break; /* Do NOT advance to next state */
                }
            }
        }

        admPtr->BlockState = BLOCK_STATE_MEMIF_PROCESS;
        break;
    }

    case BLOCK_STATE_MEMIF_PROCESS:
    {
        void *ramData;
        ramData = ( dataPtr != NULL ) ? dataPtr : bPtr->RamBlockDataAddress;
        if( ramData == NULL  ){
            blockDone = TRUE;
            status = FALSE;
        } else {
			/* Check read */
			MemIf_JobResultType jobResult = MemIf_GetJobResult(bPtr->NvramDeviceId);

			if( MEMIF_JOB_PENDING == jobResult ) {
				/* Keep on waiting */
			} else if( MEMIF_JOB_OK == jobResult ) {
				/* We are done */
				if( isWrite==TRUE ) {
					/* @req NVM284 *//* @req NVM337 */
					admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
					admPtr->ErrorStatus = NVM_REQ_OK;
					blockDone = 1;
				} else {
					/* read */
					if( bPtr->BlockUseCrc ==TRUE ) {
						/* The read data is in the work buffer, read the CRC */
						if( bPtr->BlockCRCType == NVM_CRC16) {
							admPtr->NvCrc.crc16 = READ16_NA( (&Nvm_WorkBuffer[bPtr->NvBlockLength]) );
							DEBUG_PRINTF(">> Nv CRC %04x\n",admPtr->NvCrc.crc16);
							admPtr->RamCrc.crc16 = admPtr->NvCrc.crc16;    /* Set RAM CRC = NvRAM CRC */
						} else {
							admPtr->NvCrc.crc32 = READ32_NA( (&Nvm_WorkBuffer[bPtr->NvBlockLength]) );
							DEBUG_PRINTF(">> Nv CRC %08x\n",admPtr->NvCrc.crc32);
							admPtr->RamCrc.crc32 = admPtr->NvCrc.crc32;    /* Set RAM CRC = NvRAM CRC */
						}

						/* 3.1.5/NVM201 + 3.1.5/NVM292 NvM_ReadBlock() + NvM_ReadAll() should request
						 * recalculation of the RAM block data if configured with CRC.
						 */

						/* savedDataPtr points to the real data buffers and they do no contain the
						 * crcLen */
						/* IMPROVEMENT: Should we really copy here in case crc should be checked? */
						memcpy(admPtr->savedDataPtr, Nvm_WorkBuffer, bPtr->NvBlockLength  );

						/* Check if we should re-calculate the RAM checksum now when it's in RAM
						 * 3.1.5/NVM165 */
						if( bPtr->CalcRamBlockCrc==TRUE) {
							/* This block want its RAM block CRC checked *//* @req NVM201 *//* @req NVM292 */
							DEBUG_PRINTF(">> Recalculation of RAM checksum \n",admPtr->NvCrc.crc16);
							ASSERT( bPtr->BlockUseCrc == 1);
							admPtr->BlockState = BLOCK_STATE_CALC_CRC_READ;
							admPtr->BlockSubState = BLOCK_SUBSTATE_0;
							admPtr->flags = 0;

						} else {
							/* Done */
							admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
							admPtr->ErrorStatus = NVM_REQ_OK;/* @req NVM206 *//* @req NVM279 *//* @req NVM290 *//* @req NVM291 */
							blockDone = 1;
						}

					} else {
						DEBUG_FPUTS(">> Block have NO CRC\n");

						memcpy(admPtr->savedDataPtr, Nvm_WorkBuffer, bPtr->NvBlockLength );
						/* Done */
						admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
						admPtr->ErrorStatus = NVM_REQ_OK;/* @req NVM206 *//* @req NVM279 *//* @req NVM290 *//* @req NVM291 */
						blockDone = 1;
					}

					/* Copy from Workbuffer to the real buffer */

				}
				break;
			} else {
				/* Something failed */
				DEBUG_FPUTS(">> Read/Write FAILED\n");
				if( isWrite==TRUE ) {
					/* @req NVM213 *//* @req NVM296 */
					admPtr->NumberOfWriteFailed++;
					/* !req NVM216 Num write retries should be configurable per block */
					/* !req NVM683 Num write retries should be configurable per block */
					/* @req NVM168 */
					if( admPtr->NumberOfWriteFailed > NVM_MAX_NUMBER_OF_WRITE_RETRIES ) {
						/* !req NVM659 Error not reported to Dem */
						/* !req NVM684 Error not reported to Dem */
						DEBUG_FPUTS(">> Write FAILED COMPLETELY (all retries)\n");
						blockDone = 1;
						admPtr->NumberOfWriteFailed = 0;
					}
				} else {
					blockDone = 1;
				}

				/*
				 * MEMIF have failed. Now what?
				 * 1. Check if redundant, if so read the other block
				 * 2. If point 1 fails and not redundant nor rom-block/initcallback,
				 *   fail with MEMIF_BLOCK_INVALID, MEMIF_BLOCK_INCONSISTENT or  MEMIF_JOB_FAILED
				 */
				if( blockDone == 1 ) {
					/* All write have failed or we are reading? */
					/* @req NVM655 *//* We don't report error if first block fails (REDUNDANT) */
					if( 0 == handleRedundantBlock(bPtr,admPtr) ) {
						/* block is NOT redundant or both blocks have failed */
						/*
						 *
						 * Returned status are:
						 *
						 * MEMIF_BLOCK_INVALID
						 *   The block is currenlty under some strange service (Fee_InvalidateBlock)
						 *
						 * MEMIF_BLOCK_INCONSISTENT
						 *   Ea/Fee have detected that something is strange with the block. This may
						 *   happen for a virgin unit.
						 *
						 * MEMIF_JOB_FAILED
						 *   We failed for some reason.
						 *
						 * At this point a lot of requirements NVM360, NVM342,etc will not be active
						 * if there is a configured ROM block/InitCallback.
						 */
						switch (jobResult) {
						case MEMIF_BLOCK_INVALID:
							/* @req NVM355 *//* @req NVM341 *//* @req NVM652 *//* @req NVM317 *//* @req NVM315 *//* @req NVM342 *//* @req NVM676 */
							admPtr->ErrorStatus = NVM_REQ_NV_INVALIDATED;
							break;
						case MEMIF_BLOCK_INCONSISTENT:
							/* /NVM360 but is overridden by NVM172 (implicit revovery) */
							/* @req NVM651 *//* @req NVM358 *//* @req NVM360 */
							admPtr->ErrorStatus = NVM_REQ_INTEGRITY_FAILED;
							/* @req NVM653 *//* @req NVM677 *//* @req NVM591 */
							DEM_REPORTERRORSTATUS(NvM_Config.DemEvents.NvMIntegrityFailedDemEventId, DEM_EVENT_STATUS_FAILED);
							break;
						case MEMIF_JOB_FAILED:
							/* @req NVM361 */
							/* @req NVM359 *//* @req NVM213 (Assuming that we get job failed on write failure) */
							admPtr->ErrorStatus = NVM_REQ_NOT_OK;
							/* @req NVM654 *//* @req NVM678 *//* @req NVM302 *//* @req NVM592 */
							DEM_REPORTERRORSTATUS(NvM_Config.DemEvents.NvMReqFailedDemEventId, DEM_EVENT_STATUS_FAILED);
							break;
						default:
							DEBUG_PRINTF("## Unexpected jobResult:%d\n",jobResult);
							NVM_ASSERT(FALSE);/*lint !e9027 !e774  Boolean within 'if' always evaluates to True exception is required */
							break;
						}
						/* IMPROVEMENT: No copy of ROM block if DATASET */
						/* @req NVM172 NV block not changed during implicit recovery */
						/* @req NVM388 *//* @req NVM389 *//* @req NVM390 */
						/* @req NVM572 (ROM block configured == by the parameter NvMRomBlockDataAddress or the parameter NvMInitBlockCallback) */
						if( ( FALSE == isWrite ) && (( bPtr->RomBlockDataAdress != NULL) ||( bPtr->InitBlockCallback != NULL) ) ){
							if( bPtr->RomBlockDataAdress != NULL ) {/* @req NVM139 */
								DEBUG_FPUTS("Copying ROM data to block\n");
								memcpy(ramData, bPtr->RomBlockDataAdress,bPtr->NvBlockLength);
								admPtr->ErrorStatus = NVM_REQ_OK;
							} else if( bPtr->InitBlockCallback != NULL ) {
								/* @req NVM469 */
								DEBUG_FPUTS("Filling block with default data\n");
								/* @req NVM369 */
								(void)bPtr->InitBlockCallback();
								admPtr->ErrorStatus = NVM_REQ_OK;
							}else{
							    /* do nothing */
							}
						} else {
							/* @req NVM085 */
							admPtr->BlockState = BLOCK_STATE_MEMIF_REQ; /* NOTE, this really true for all result below */
							AdminMultiReq.PendingErrorStatus = NVM_REQ_NOT_OK;

						}
					}

				}
			}
    	}
        break;
    }

    case BLOCK_STATE_CALC_CRC:
        NVM_ASSERT(FALSE); /*lint !e9027 !e774 Boolean within 'if' always evaluates to True */
        break;

    case BLOCK_STATE_CALC_CRC_WRITE:
    {
        uint16 crc16;
        uint32 crc32;

        /* bPtr->RamBlockDataAddress dataPtr
         *        NULL                NULL       #BAD
         *        NULL                data       Use dataPtr
         *        data                NULL       Use RamBlockDataAddress
         *        data                data       Use dataPtr
         * */
        const void *ramData = ( dataPtr != NULL ) ? dataPtr : bPtr->RamBlockDataAddress;
        NVM_ASSERT( (ramData != NULL) );

        /* Calculate RAM CRC checksum */
        /* !req NVM351 Initial values published by CRC module not used */
        if( bPtr->BlockCRCType == NVM_CRC16 ) {

            crc16 = Crc_CalculateCRC16(ramData,bPtr->NvBlockLength, 0xffff, TRUE);
            DEBUG_CHECKSUM("RAM",crc16);

            /* Just save the checksum */
            admPtr->RamCrc.crc16 = crc16;

            /* Write the block */
            admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
        } else {
            crc32 = Crc_CalculateCRC32(ramData,bPtr->NvBlockLength,0xffffffffUL, TRUE);
            DEBUG_CHECKSUM("RAM",crc32);

            admPtr->RamCrc.crc32 = crc32;
        }
        /* Write the block */
        admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;

        break;
    }
    case BLOCK_STATE_CALC_CRC_READ:
    {
        //NVM_ASSERT(bPtr->RamBlockDataAddress != NULL );
        NVM_ASSERT((bPtr->CalcRamBlockCrc == true));
        uint16 crc16 = 0;
        uint32 crc32 = 0;
        boolean checksumOk;

        /* Calculate CRC on the data we just read to RAM. Compare with CRC that is located in NV block */
        /* !req NVM351 Initial values published by CRC module not used */
        if( bPtr->BlockCRCType == NVM_CRC16 ) {
            crc16 = Crc_CalculateCRC16(admPtr->savedDataPtr,bPtr->NvBlockLength,0xffff, TRUE);
        } else {
            crc32 = Crc_CalculateCRC32(admPtr->savedDataPtr,bPtr->NvBlockLength,0xffffffffUL, TRUE);
        }

        switch( admPtr->BlockSubState ) {
        case BLOCK_SUBSTATE_0:

            checksumOk = ( bPtr->BlockCRCType == NVM_CRC16 ) ? ( crc16 ==  admPtr->RamCrc.crc16 ) : ( crc32 ==  admPtr->RamCrc.crc32 );

            /* @req NVM387 */
            if( checksumOk == true) {
                admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;

                DEBUG_CHECKSUM("RAM checksum ok with ", ( bPtr->BlockCRCType == NVM_CRC16 ) ? crc16 : crc32 );
                admPtr->ErrorStatus = NVM_REQ_OK;/* @req NVM206 *//* @req NVM279 *//* @req NVM290 *//* @req NVM291 */
                admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
                blockDone = 1;

            } else {
                /* NVM387, NVM388
                 * Corrupt CRC, what choices are there:
                 * 1. Default data (=ROM) configured, just copy it.
                 * 2. Data redundancy, get it.
                 * 3. None of the above. Catastrophic failure. (NVM203)
                 */

                if( 0 == handleRedundantBlock(bPtr,admPtr) ) {
                    /* block is NOT redundant or both blocks have failed */
                    /* IMPROVEMENT: No copy from ROM if DATASET? */
                    /* @req NVM202 *//* @req NVM363 *//* @req NVM293 *//* @req NVM169 */
                    /* @req NVM172 NV block not changed during implicit recovery */
                    /* @req NVM388 *//* @req NVM389 *//* @req NVM390 */
                    /* @req NVM572 (ROM block configured == by the parameter NvMRomBlockDataAddress or the parameter NvMInitBlockCallback) */
                    if( bPtr->RomBlockDataAdress != NULL ) {/* @req NVM139 */
                        DEBUG_FPUTS("Copying ROM data to block\n");
                        /* IMPROVEMENT: Shouldn't this copy be to admPtr->savedDataPtr? */
                        memcpy(admPtr->savedDataPtr, bPtr->RomBlockDataAdress,bPtr->NvBlockLength);
                        admPtr->BlockSubState = BLOCK_SUBSTATE_1;
                    } else {

                        /* @req NVM469 */
                        if( bPtr->InitBlockCallback != NULL ) {

                            DEBUG_FPUTS("Filling block with default data\n");
                            /* @req NVM369 */
                            (void)bPtr->InitBlockCallback();
                            admPtr->BlockSubState = BLOCK_SUBSTATE_1;

                            /* The RAM CRC is at this point not calculated...so we must do this
                             * .. so just stay in this state one more MainFunction.
                             * */

                        } else {
                            /* @req NVM085 */
                            /* We have CRC mismatch -> FAIL */
                            DEBUG_FPUTS("### Block FAILED with NVM_REQ_INTEGRITY_FAILED\n");

                            /* @req NVM203 *//* @req NVM294 *//* @req NVM591 */
                            DEM_REPORTERRORSTATUS(NvM_Config.DemEvents.NvMIntegrityFailedDemEventId, DEM_EVENT_STATUS_FAILED);
                            /* @req NVM204 *//* @req NVM295 */
                            admPtr->ErrorStatus = NVM_REQ_INTEGRITY_FAILED;
                            admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
                            blockDone = 1;
                        }
                    }
                }
            }
            break;
        case BLOCK_SUBSTATE_1:
            /* The checksum is on the ROM data so just save it */
            DEBUG_CHECKSUM("RAM checksum after ROM copy ", ( bPtr->BlockCRCType == NVM_CRC16 ) ? crc16 : crc32 );

            if( bPtr->BlockCRCType == NVM_CRC16 ) {
                admPtr->RamCrc.crc16 = crc16;
            } else  {
                admPtr->RamCrc.crc32 = crc32;
            }
            admPtr->BlockSubState = BLOCK_SUBSTATE_0;
            admPtr->ErrorStatus = NVM_REQ_INTEGRITY_FAILED;
            admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
            blockDone = 1;
            break;
        default:
            break;
        }

        break;
    }
    default:
        NVM_ASSERT(FALSE);/*lint !e9027 !e774  Boolean within 'if' always evaluates to True exception is required */
        break;
    }

    if (status == TRUE) {
    	if( blockDone==TRUE ) {
			DEBUG_FPUTS("# Block Done\n");

			if( admPtr->ErrorStatus == NVM_REQ_OK ) {
				/* @req NVM200 *//* @req NVM251 *//* @req NVM472 */
				/* !req NVM366 Block not assumed changed when default values copied */
				/* !req NVM228 Block not assumed changed when default values copied */
				/* @req NVM367 *//* ErrorStatus set to OK if default values copied */
				admPtr->BlockChanged = FALSE;
				admPtr->BlockValid = TRUE;
			}


			/* @req NVM281 *//* @req NVM742 */
			if( bPtr->SingleBlockCallback != NULL ) {
				/* @req NVM368 *//* @req NVM740 *//* @req NVM741 *//* @req NVM176 */
				(void)bPtr->SingleBlockCallback(serviceId, admPtr->ErrorStatus);
			}

			if( multiBlock==FALSE ) {
				nvmState = NVM_IDLE;
				nvmSubState =NS_INIT;
			}
		}
    }
    return blockDone;
}

static void MultiblockMain( boolean isWrite ) {

    /* Cases:
     * 1. We process each block until it's finished
     * 2. We start to process a lot of blocks. The blocks may use different devices
     *    and should be able to read a lot of them. This includes CRC.
     *
     *    1) is much simpler and 2) probably much faster.
     *    This implementation will use 1) since it's simpler and maximum time that is
     *    spent in MainFunction() can be controlled much better.
     */
    /* @req NVM244 */
    while (    (AdminMultiReq.currBlockIndex < NVM_NUM_OF_NVRAM_BLOCKS) ) {
        /* @req NVM434 */
        if( AdminBlock[AdminMultiReq.currBlockIndex].ErrorStatus != NVM_REQ_BLOCK_SKIPPED) {
            if( FALSE == DriveBlock(    &NvM_Config.BlockDescriptor[AdminMultiReq.currBlockIndex],
                        &AdminBlock[AdminMultiReq.currBlockIndex],
                        NULL, isWrite, true, false ) )
            {
                /* Drive block have more job to do.. wait until next time */
                break;
            }
        }
        AdminMultiReq.currBlockIndex++;
    }

    /* Check if we are the last block */
    if( AdminMultiReq.currBlockIndex >= NVM_NUM_OF_NVRAM_BLOCKS ) {
        AdminMultiReq.currBlockIndex = 0;

        /* @req NVM301 *//* @req NVM318 *//* @req NVM395 */
        if( NVM_REQ_NOT_OK == AdminMultiReq.PendingErrorStatus ) {
            AdminMultiBlock.ErrorStatus = NVM_REQ_NOT_OK;
        } else {
            AdminMultiBlock.ErrorStatus = NVM_REQ_OK;
        }
        /* Reset state machine */
        nvmState = NVM_IDLE;
        nvmSubState = NS_INIT;

        /* @req NVM468 */
        if( NvM_Config.Common.MultiBlockCallback != NULL ) {
            /* @req NVM686 */
            NvM_Config.Common.MultiBlockCallback(serviceId, AdminMultiBlock.ErrorStatus);
        }
    }
}


/*
 * Initiate the write all job
 */
static boolean WriteAllInit(void)
{
    /* !req NVM252 Parameter NvMSelectBlockForWriteAll not supported */
    const NvM_BlockDescriptorType    *BlockDescriptorList = NvM_Config.BlockDescriptor;
    AdministrativeBlockType *AdminBlockTable = AdminBlock;
    uint16 i;
    boolean needsProcessing = FALSE;

//    nvmState = NVM_WRITE_ALL_PROCESSING;
    AdminMultiReq.PendingErrorStatus = NVM_REQ_OK;
    AdminMultiReq.currBlockIndex = 0;

    for (i = 0; i < NVM_NUM_OF_NVRAM_BLOCKS; i++) {
        if ((BlockDescriptorList->RamBlockDataAddress != NULL)
#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)                        /** @req NVM344 */
                && (AdminBlockTable->BlockValid == TRUE)                    /** @req NVM682 */
                && (AdminBlockTable->BlockChanged == TRUE)                  /** @req NVM682 */
#endif
                && (AdminBlockTable->BlockWriteProtected == FALSE)          /** @req NVM432 *//** @req NVM433 */
                && (AdminBlockTable->DataIndex < BlockDescriptorList->NvBlockNum))/* @req NVM377 *//* Treat write to ROM block like write to protected */
        {
            AdminBlockTable->ErrorStatus = NVM_REQ_PENDING;/* @req NVM549 */

            if (BlockDescriptorList->BlockUseCrc == TRUE) {
                AdminBlockTable->BlockState = BLOCK_STATE_CALC_CRC_WRITE;    /** @req NVM253 */
            } else {
                AdminBlockTable->BlockState = BLOCK_STATE_MEMIF_REQ;
                AdminBlockTable->NumberOfWriteFailed = 0;
            }
            needsProcessing = TRUE;
        } else {
            AdminBlockTable->ErrorStatus = NVM_REQ_BLOCK_SKIPPED;    /** @req NVM298 */
        }

        AdminBlockTable++;
        BlockDescriptorList++;
    }
    return needsProcessing;
}


/***************************************
 *    External accessible functions    *
 ***************************************/
/*
 * Procedure:    NvM_Init
 * Reentrant:    No
 */
/* @req NVM447 */
void NvM_Init(void)
{
    /* IMPROVEMENT: Validate configuration? E.g. NATIVE block has 1 nvBlock. */
    /** @req NVM399 *//** @req NVM193 *//** @req NVM400 */
    /* @req NVM091 */
    /* @req NVM157 */
    static Nvm_QueueType  nvmQueueData[NVM_SIZE_STANDARD_JOB_QUEUE];
    const NvM_BlockDescriptorType    *BlockDescriptorList = NvM_Config.BlockDescriptor;
    AdministrativeBlockType *AdminBlockTable = AdminBlock;
    uint16 i;


    CirqBuff_Init(&nvmQueue,nvmQueueData,sizeof(nvmQueueData)/sizeof(Nvm_QueueType),sizeof(Nvm_QueueType));

    // Initiate the administration blocks
    for (i = 0; i< NVM_NUM_OF_NVRAM_BLOCKS; i++) {
        if (BlockDescriptorList->BlockManagementType == NVM_BLOCK_DATASET) {
            AdminBlockTable->DataIndex = 0;    /** @req NVM192 */
        }
        AdminBlockTable->BlockWriteProtected = BlockDescriptorList->BlockWriteProt;/* @req NVM326 */
        AdminBlockTable->ErrorStatus = NVM_REQ_NOT_OK;
        AdminBlockTable->BlockChanged = FALSE;
        AdminBlockTable->BlockValid = FALSE;
        AdminBlockTable->NumberOfWriteFailed = 0;

        AdminBlockTable++;
        BlockDescriptorList++;
    }

    AdminMultiBlock.ErrorStatus = NVM_REQ_NOT_OK;
    (void)MemIfJobAdmin.JobFinished;/* Just to make lint happy */
    // Set status to initialized
    nvmState = NVM_IDLE;
}


/**
 *
 * Note!
 *   NvM_ReadAll() does not set status here or need to check status of the
 *   any blocks since it's done after all single reads are done.
 */
/* @req NVM460 */
void NvM_ReadAll(void)
{
    /* !req NVM364 *//* What does this mean? Should we recalculate the RAM crc before requesting block read? */
    /* !req NVM362 *//* What does this mean? Should we recalculate the RAM crc and not read the block if crc mtches the stored crc? */
    /* !req NVM226 *//* What does this mean? */
    /* @req NVM356 */
    /* @req NVM646 */
    DET_VALIDATE_NO_RV((nvmState != NVM_UNINITIALIZED), NVM_READ_ALL_ID, NVM_E_NOT_INITIALIZED);

    SchM_Enter_NvM_EA_0();
    /* @req NVM243 *//* Signal request and return */
    AdminMultiReq.state = NVM_READ_ALL;
    AdminMultiBlock.ErrorStatus = NVM_REQ_PENDING;/* @req NVM667 */
    AdminMultiReq.serviceId = NVM_READ_ALL_ID;
    SchM_Exit_NvM_EA_0();
}



/*
 * Procedure:    NvM_WriteAll
 * Reentrant:    No
 */
/* @req NVM461 */
void NvM_WriteAll(void)
{
    /* @req NVM018 */
    /* @req NVM647 */
    DET_VALIDATE_NO_RV((nvmState != NVM_UNINITIALIZED), NVM_WRITE_ALL_ID, NVM_E_NOT_INITIALIZED);

    SchM_Enter_NvM_EA_0();
    /* @req NVM254 *//* Signal request and return */
    AdminMultiReq.state = NVM_WRITE_ALL;
    AdminMultiBlock.ErrorStatus = NVM_REQ_PENDING;
    AdminMultiReq.serviceId = NVM_WRITE_ALL_ID;
    SchM_Exit_NvM_EA_0();
}


/*
 * Procedure:    NvM_CancelWriteAll
 * Reentrant:    No
 */
void NvM_CancelWriteAll(void)
{
    
}


#if (NVM_API_CONFIG_CLASS > NVM_API_CONFIG_CLASS_2)
/* @req NVM459 */
Std_ReturnType NvM_InvalidateNvBlock( NvM_BlockIdType blockId  )
{
    Std_ReturnType status;
    status = E_OK;
    /* !req NVM385 Request queued if block already queued (when not using Det) or currently in progress */
    Nvm_QueueType qEntry;
    const NvM_BlockDescriptorType * bPtr;
    AdministrativeBlockType *       admPtr;
    uint32 rv;

    NVM_ASSERT( blockId >= 2 ); /* No support for lower numbers, yet */

    /* @req NVM642 */
    DET_VALIDATE_RV(blockId <= NVM_NUM_OF_NVRAM_BLOCKS,
                    NVM_INVALIDATENV_BLOCK_ID, NVM_E_PARAM_BLOCK_ID, E_NOT_OK);
    /* @req NVM638 */
    DET_VALIDATE_RV(nvmState != NVM_UNINITIALIZED,
                    NVM_INVALIDATENV_BLOCK_ID, NVM_E_NOT_INITIALIZED, E_NOT_OK);


    bPtr = &NvM_Config.BlockDescriptor[blockId-1];
    admPtr = &AdminBlock[blockId-1];

    /* @req NVM639 ErrorStatus PENDING until finished */
    DET_VALIDATE_RV((admPtr->ErrorStatus != NVM_REQ_PENDING),
                     NVM_INVALIDATENV_BLOCK_ID, NVM_E_BLOCK_PENDING , E_NOT_OK );

    /* @req NVM423 */
    /* @req NVM272 */
    /* !req NVM273 Wrong Dem error reported*/
    /* @req NVM664 */
    if( (admPtr->BlockWriteProtected == TRUE) ||(admPtr->DataIndex >= bPtr->NvBlockNum)) {
        /* If write protected or ROM block in dataset, return E_NOT_OK */
        /* IMPROVEMENT: Wrong Dem error */
        DEM_REPORTERRORSTATUS(NvM_Config.DemEvents.NvMIntegrityFailedDemEventId, DEM_EVENT_STATUS_FAILED);
        DET_REPORTERROR(NVM_INVALIDATENV_BLOCK_ID, NVM_E_NV_WRITE_PROTECTED);

        status = E_NOT_OK;
    } else {

		/* @req NVM424 */
		/* @req NVM239 */
		qEntry.blockId = blockId;
		qEntry.op = NVM_INVALIDATE_NV_BLOCK;
		qEntry.serviceId = NVM_INVALIDATENV_BLOCK_ID;
		rv = (uint32)CirqBuffPush(&nvmQueue,&qEntry);
		/* @req NVM386 */
		if(0 != rv) {
			DET_REPORTERROR(NVM_INVALIDATENV_BLOCK_ID, NVM_E_LIST_OVERFLOW);
			/* @req NVM184 */
			/*lint -e{904} Return statement is necessary in case of reporting a DET error */
			return E_NOT_OK;
		}

		/* @req NVM185 */
		admPtr->ErrorStatus = NVM_REQ_PENDING;
    }
    return status;

    /* @req NVM422 Leave the RAM block unmodified */

}

Std_ReturnType NvM_SetBlockProtection( NvM_BlockIdType blockId, boolean protectionEnabled ) {
    (void) blockId;
    (void) protectionEnabled;

    // Empty for now

    return E_NOT_OK;
}

Std_ReturnType NvM_EraseNvBlock( NvM_BlockIdType blockId ) {
    (void) blockId;

    // Empty for now

    return E_NOT_OK;
}

#endif


/*
 * Procedure:    NvM_GetErrorStatus
 * Reentrant:    Yes
 */
/* @req NVM451 */
Std_ReturnType NvM_GetErrorStatus(NvM_BlockIdType blockId, NvM_RequestResultType *requestResultPtr)
{
    /* @req NVM083 */
    /* @req NVM610 */
    DET_VALIDATE_RV((nvmState != NVM_UNINITIALIZED), NVM_GET_ERROR_STATUS_ID, NVM_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req NVM611 */
    DET_VALIDATE_RV((blockId < (NVM_NUM_OF_NVRAM_BLOCKS+1)), NVM_GET_ERROR_STATUS_ID, NVM_E_PARAM_BLOCK_ID, E_NOT_OK );
    /* @req NVM612 */
    DET_VALIDATE_RV((NULL != requestResultPtr), NVM_GET_ERROR_STATUS_ID, NVM_E_PARAM_DATA, E_NOT_OK );

    /* @req NVM015 */
    if (blockId == 0) {
        // Multiblock ID
        /* @req NVM394 */
        *requestResultPtr = AdminMultiBlock.ErrorStatus;
    } else if (blockId == 1) {
        /* IMPROVEMENT Configuration ID */
      *requestResultPtr = NVM_REQ_OK;
    } else {
        *requestResultPtr = AdminBlock[blockId-1].ErrorStatus;
    }
    return E_OK;
}


#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)    /** @req NVM408 */
/*
 * Procedure:    Nvm_SetRamBlockStatus
 * Reentrant:    Yes
 */
/* @req NVM453 */
Std_ReturnType NvM_SetRamBlockStatus(NvM_BlockIdType blockId, boolean blockChanged)
{
    AdministrativeBlockType *         admPtr;
    /* @req NVM645 */
    DET_VALIDATE_RV((blockId < (NVM_NUM_OF_NVRAM_BLOCKS+1)), NVM_SET_RAM_BLOCK_STATUS_ID, NVM_E_PARAM_BLOCK_ID, E_NOT_OK);

    admPtr = &AdminBlock[blockId-1];

    /* @req NVM643 */
    DET_VALIDATE_RV((nvmState != NVM_UNINITIALIZED), NVM_SET_RAM_BLOCK_STATUS_ID, NVM_E_NOT_INITIALIZED, E_NOT_OK);
    DET_VALIDATE_RV((blockId > 1u), NVM_SET_RAM_BLOCK_STATUS_ID, NVM_E_PARAM_BLOCK_ID, E_NOT_OK );
    /* @req NVM644 ErrorStatus PENDING until finished */
    DET_VALIDATE_RV( (admPtr->ErrorStatus != NVM_REQ_PENDING), NVM_SET_RAM_BLOCK_STATUS_ID, NVM_E_BLOCK_PENDING , E_NOT_OK );

    setRamBlockStatus(&NvM_Config.BlockDescriptor[blockId-1],admPtr,blockChanged );

    return E_OK;
}

static void setRamBlockStatus( const NvM_BlockDescriptorType    *bPtr, AdministrativeBlockType *admPtr, boolean blockChanged ) {

    if (bPtr->RamBlockDataAddress != NULL) {    /** @req NVM240 */
        if (blockChanged == TRUE) {
            admPtr->BlockChanged = TRUE;    /** @req NVM406 */
            admPtr->BlockValid = TRUE;    /** @req NVM241 */
//    IMPROVEMENT  if (bPtr->BlockUseCrc) {
//               admPtr->BlockState = BLOCK_STATE_CALC_CRC;    /** !req NVM121 */
//           }
        } else {
            admPtr->BlockChanged = FALSE;    /** @req NVM405 */
            admPtr->BlockValid = FALSE;
        } // else blockChanged
    } // if permanent block
}
#endif

void NvM_SetBlockLockStatus( NvM_BlockIdType blockId, boolean blockLocked ) {
    (void)blockId;
    (void)blockLocked;
}


/**
 * Restore default data to its corresponding RAM block.
 *
 * @param BlockId        NVRAM block identifier.
 * @param NvM_DestPtr      Pointer to the RAM block
 * @return
 */
/* @req NVM456 */
Std_ReturnType NvM_RestoreBlockDefaults( NvM_BlockIdType blockId, void* NvM_DestPtr )
{
    /* @req NVM012 */
    /* !req NVM628 */
    /* !req NVM385 Request queued if block already queued (when not using Det) or currently in progress */
    /* @req NVM391 */
    Std_ReturnType status;
    status = E_OK;
    const NvM_BlockDescriptorType *    bPtr;
    AdministrativeBlockType *         admPtr;
    Nvm_QueueType qEntry;
    uint32 rv;

    NVM_ASSERT( blockId >= 2 );    /* No support for lower numbers, yet */
    /* @req NVM625 */
    DET_VALIDATE_RV(nvmState != NVM_UNINITIALIZED, NVM_RESTORE_BLOCK_DEFAULTS_ID, NVM_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req NVM630 */
    DET_VALIDATE_RV( (blockId <= NVM_NUM_OF_NVRAM_BLOCKS), NVM_RESTORE_BLOCK_DEFAULTS_ID, NVM_E_PARAM_BLOCK_ID,E_NOT_OK );

    bPtr = &NvM_Config.BlockDescriptor[blockId-1];
    admPtr = &AdminBlock[blockId-1];

    /* It must be a permanent RAM block but no RamBlockDataAddress -> error */
    /* @req NVM435 *//* @req NVM436 *//* @req NVM629 */
    DET_VALIDATE_RV( !((NvM_DestPtr == NULL) &&  ( bPtr->RamBlockDataAddress == NULL )),
            NVM_RESTORE_BLOCK_DEFAULTS_ID,NVM_E_PARAM_ADDRESS,E_NOT_OK );
    /* @req NVM626 ErrorStatus PENDING until finished */
    DET_VALIDATE_RV( (admPtr->ErrorStatus != NVM_REQ_PENDING), NVM_RESTORE_BLOCK_DEFAULTS_ID, NVM_E_BLOCK_PENDING , E_NOT_OK );

    /* @req NVM353 */
    if( (NVM_BLOCK_DATASET == bPtr->BlockManagementType) && (0 != bPtr->RomBlockNum) && (admPtr->DataIndex < bPtr->NvBlockNum) ) {
        status = E_NOT_OK;
    } else {
		/* @req NVM224 */
		qEntry.blockId = blockId;
		qEntry.op = NVM_RESTORE_BLOCK_DEFAULTS;
		qEntry.blockId = blockId;
		qEntry.dataPtr = (uint8_t *)NvM_DestPtr;
		qEntry.serviceId = NVM_RESTORE_BLOCK_DEFAULTS_ID;
		rv = (uint32)CirqBuffPush(&nvmQueue,&qEntry);
		/* @req NVM386 */
		if(0 != rv) {
			DET_REPORTERROR(NVM_RESTORE_BLOCK_DEFAULTS_ID, NVM_E_LIST_OVERFLOW);
			/* @req NVM184 */
			/*lint -e{904} Return statement is necessary in case of reporting a DET error */
			return E_NOT_OK;
		}

		/* @req NVM185 */
		admPtr->ErrorStatus = NVM_REQ_PENDING;

		if( bPtr->BlockUseCrc==TRUE) {
			/* @req NVM229 */
			admPtr->BlockState = BLOCK_STATE_CALC_CRC_WRITE;
		} else {
			admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
		}
    }
    return status;
}

/**
 * Service to copy the data NV block to the RAM block
 *
 * @param blockId        0 and 1 reserved. The block ID are sequential.
 * @param NvM_DstPtr
 * @return
 */
/* @req NVM454 */
Std_ReturnType NvM_ReadBlock( NvM_BlockIdType blockId, void* NvM_DstPtr )
{
    /* @req NVM010 */
    /* !req NVM385 Request queued if block already queued (when not using Det) or currently in progress */
   /* logical block:
    *
    *    1 1 1 1 1 1
    *   |5 4 3 2 1 0 9 8|7 6 5 4 3 2 1 0|
    *    b b b b b b b b b b d d d d d d
    *
    * Here we have 10 bits for block id, 16-5 for DataSetSelection bits.
    * - 2^10, 1024 blocks
    * - 64 datasets for each NVRAM block
    *
    * How are the block numbers done in EA? Assume virtual page=8
    * logical
    *  Block   size
    *  1       32
    *   2
    *   3
    *   4
    *  5       12
    *   6
    *  7
    *
    *  How can NVM/NvmNvBlockLength and EA/EaBlockSize be different?
    *  It seems that EA/FEE does not care about that logical block 2 above is
    *  "blocked"
    *
    */
    Nvm_QueueType qEntry;
    uint32 rv;

    /* !req NVM311 Request for reading block ID 1 allowed */

    /* @req NVM614 */
    DET_VALIDATE_RV(nvmState != NVM_UNINITIALIZED, NVM_READ_BLOCK_ID, NVM_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req NVM618 */
    DET_VALIDATE_RV( blockId <= NVM_NUM_OF_NVRAM_BLOCKS, NVM_READ_BLOCK_ID,NVM_E_PARAM_BLOCK_ID,E_NOT_OK );

    /** @req NVM196 */ /** @req NVM278 *//** @req NVM616 */
    DET_VALIDATE_RV( !(( NvM_DstPtr == NULL) &&
                ( NvM_Config.BlockDescriptor[blockId-1].RamBlockDataAddress == NULL )),
                0, NVM_E_PARAM_ADDRESS  , E_NOT_OK );
    /* @req NVM615 ErrorStatus PENDING until finished */
    DET_VALIDATE_RV( (AdminBlock[blockId-1].ErrorStatus != NVM_REQ_PENDING), NVM_READ_BLOCK_ID, NVM_E_BLOCK_PENDING , E_NOT_OK );

    /* @req NVM195 */
    qEntry.op = NVM_READ_BLOCK;
    qEntry.blockId = blockId;
    qEntry.dataPtr = NvM_DstPtr;
    qEntry.serviceId = NVM_READ_BLOCK_ID;
    rv =(uint32)CirqBuffPush(&nvmQueue,&qEntry);
    /* @req NVM386 */
    if(0 != rv) {
        DET_REPORTERROR(NVM_READ_BLOCK_ID, NVM_E_LIST_OVERFLOW);
        /* @req NVM184 */
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return E_NOT_OK;
    }

    /* @req NVM185 */
    AdminBlock[blockId-1].ErrorStatus = NVM_REQ_PENDING;

    return E_OK;
}


/**
 * Service to copy a RAM block to its correspnding NVRAM block
 *
 * @param blockId
 * @param NvM_SrcPtr
 * @return
 */
/* @req NVM455 */
Std_ReturnType NvM_WriteBlock( NvM_BlockIdType blockId,  const void* NvM_SrcPtr ) {
    /* @req NVM410 */
    /* !req NVM385 Request queued if block already queued (when not using Det) or currently in progress */
    const NvM_BlockDescriptorType * bPtr;
    AdministrativeBlockType *       admPtr;
    Nvm_QueueType qEntry;
    uint32 rv;

    NVM_ASSERT( blockId >= 2 );    /* No support for lower numbers, yet */
    /* @req NVM619 */
    DET_VALIDATE_RV(nvmState != NVM_UNINITIALIZED, NVM_WRITE_BLOCK_ID, NVM_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req NVM624 */
    DET_VALIDATE_RV( blockId <= NVM_NUM_OF_NVRAM_BLOCKS, NVM_WRITE_BLOCK_ID, NVM_E_PARAM_BLOCK_ID, E_NOT_OK );

    bPtr = &NvM_Config.BlockDescriptor[blockId-1];
    admPtr = &AdminBlock[blockId-1];
    /* !req NVM411 Error not reported to Dem */
    /* @req NVM377 *//* Treat write to ROM block as write to protected nv block */
    if((admPtr->BlockWriteProtected == TRUE) || !(admPtr->DataIndex < bPtr->NvBlockNum)) {
        DET_REPORTERROR(NVM_WRITE_BLOCK_ID, NVM_E_NV_WRITE_PROTECTED);
        /* @req NVM217 */
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return E_NOT_OK;
    }

    /* @req NVM210 *//* @req NVM280 *//* @req NVM622 */
    DET_VALIDATE_RV( !((NvM_SrcPtr == NULL) && ( bPtr->RamBlockDataAddress == NULL )),
            NVM_WRITE_BLOCK_ID, NVM_E_PARAM_ADDRESS, E_NOT_OK );
    /* @req NVM620 ErrorStatus PENDING until finished */
    DET_VALIDATE_RV( (admPtr->ErrorStatus != NVM_REQ_PENDING), NVM_WRITE_BLOCK_ID, NVM_E_BLOCK_PENDING , E_NOT_OK );

    /* @req NVM424 *//* @req NVM239 *//* @req NVM208 */
    qEntry.blockId = blockId;
    qEntry.op = NVM_WRITE_BLOCK;
    qEntry.blockId = blockId;
    qEntry.dataPtr = (uint8 *)NvM_SrcPtr; /*lint !e9005    attempt to cast away const/volatile from a pointer is rquired         */
    qEntry.serviceId = NVM_WRITE_BLOCK_ID;
    rv =(uint32) CirqBuffPush(&nvmQueue,&qEntry);
    /* @req NVM386 */
    if(0 != rv) {
        DET_REPORTERROR(NVM_WRITE_BLOCK_ID, NVM_E_LIST_OVERFLOW);
        /* @req NVM184 */
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return E_NOT_OK;
    }

    /* @req NVM185 */
    admPtr->ErrorStatus = NVM_REQ_PENDING;

    if( bPtr->BlockUseCrc==TRUE) {
        /* @req NVM212 */
        admPtr->BlockState = BLOCK_STATE_CALC_CRC_WRITE;
    } else {
        admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
    }


    return E_OK;
}

/* Missing from Class 2
 * - NvM_CancelWriteAll
 * */

#if (NVM_API_CONFIG_CLASS > NVM_API_CONFIG_CLASS_1)

/*
 * Note!
 * This function returns void in 3.1.5 and in 4.0 it returns Std_ReturnType.
 *
 *
 * return
 *   x NVM_E_NOT_INITIALIZED
 *   x NVM_E_BLOCK_PENDING
 *   NVM_E_PARAM_BLOCK_DATA_IDX
 *   NVM_E_PARAM_BLOCK_TYPE
 *   x NVM_E_PARAM_BLOCK_ID
 *
 */
/* @req NVM448 */
Std_ReturnType NvM_SetDataIndex( NvM_BlockIdType blockId, uint8 dataIndex ) {
    /* @req NVM263 */
    const NvM_BlockDescriptorType * bPtr = &NvM_Config.BlockDescriptor[blockId-1];
    AdministrativeBlockType *       admPtr = &AdminBlock[blockId-1];

    NVM_ASSERT( blockId >= 2 );    /* No support for lower numbers, yet */

    /* @req NVM027 */
    DET_VALIDATE_RV(nvmState != NVM_UNINITIALIZED, NVM_SET_DATA_INDEX_ID, NVM_E_NOT_INITIALIZED, E_NOT_OK);
    /* @req NVM601 */
    DET_VALIDATE_RV(blockId < (NVM_NUM_OF_NVRAM_BLOCKS+1), NVM_SET_DATA_INDEX_ID, NVM_E_PARAM_BLOCK_ID, E_NOT_OK);
    if( bPtr->BlockManagementType != NVM_BLOCK_DATASET ) {
        /* @req NVM264 *//* @req NVM600 */
        DET_REPORTERROR(NVM_SET_DATA_INDEX_ID, NVM_E_PARAM_BLOCK_TYPE);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return E_NOT_OK;
    }
    /* @req NVM599 *//* @req NVM146 */
    DET_VALIDATE_RV(dataIndex < (bPtr->NvBlockNum + bPtr->RomBlockNum) , NVM_SET_DATA_INDEX_ID, NVM_E_PARAM_BLOCK_DATA_IDX ,E_NOT_OK);
    /* @req NVM598 ErrorStatus PENDING until finished */
    DET_VALIDATE_RV( (admPtr->ErrorStatus != NVM_REQ_PENDING), NVM_SET_DATA_INDEX_ID, NVM_E_BLOCK_PENDING , E_NOT_OK);

    /* @req NVM014 */
    admPtr->DataIndex = dataIndex;
    admPtr->ErrorStatus = NVM_REQ_OK;

    return E_OK;
}

/* @req NVM449 */
Std_ReturnType NvM_GetDataIndex( NvM_BlockIdType blockId, uint8 *dataIndexPtr ) {

    const NvM_BlockDescriptorType * bPtr = &NvM_Config.BlockDescriptor[blockId-1];

    AdministrativeBlockType *       admPtr = &AdminBlock[blockId-1];

    NVM_ASSERT( blockId >= 2 );    /* No support for lower numbers, yet */
    /* @req NVM602 */
    DET_VALIDATE_RV(nvmState != NVM_UNINITIALIZED, NVM_GET_DATA_INDEX_ID, NVM_E_NOT_INITIALIZED, E_NOT_OK);
    /* @req NVM604 */
    DET_VALIDATE_RV(blockId < (NVM_NUM_OF_NVRAM_BLOCKS+1), NVM_GET_DATA_INDEX_ID, NVM_E_PARAM_BLOCK_ID, E_NOT_OK);
    /* @req NVM603 */
    DET_VALIDATE_RV(bPtr->BlockManagementType == NVM_BLOCK_DATASET ,  NVM_GET_DATA_INDEX_ID, NVM_E_PARAM_BLOCK_TYPE , E_NOT_OK);
    /* @req NVM605 */
    DET_VALIDATE_RV( (dataIndexPtr != NULL) , NVM_GET_DATA_INDEX_ID, NVM_E_PARAM_DATA  ,E_NOT_OK);

    /* @req NVM021 */
    if(NVM_BLOCK_DATASET == bPtr->BlockManagementType) {
        *dataIndexPtr = admPtr->DataIndex;
    } else {
        /* @req NVM265 */
        *dataIndexPtr = 0;
    }
    admPtr->ErrorStatus = NVM_REQ_OK;/* IMPROVEMENT: Is this really correct??? */

    return E_OK;
}
#endif




/**
 * Service for performing the processing of the NvM jobs.
 */
/* @req NVM464 */
void NvM_MainFunction(void)
{
    /* @req NVM256 */
    /* @req NVM333 *//* Done before writing */
    /* @req NVM334 */
    /* @req NVM257 */
    /* @req NVM346 *//* Always polling */
    /* @req NVM347 */
    /* !req NVM350 *//* Broadcast id not used */
    /* @req NVM349 */
    /* @req NVM324 */
    /* @req NVM381 *//* The NvM module shall not interrupt jobs originating from the NvM_ReadAll request by other requests */
    /* @req NVM567 *//* The NvM module shall not interrupt jobs originating from the NvM_WriteAll request by other requests */
    static const NvM_BlockDescriptorType *    nvmBlock;
    static AdministrativeBlockType *admBlock;
    uint32 rv;
    uint16 blkNum;
    uint16 set;
    static Nvm_QueueType     qEntry;
    const NvM_BlockDescriptorType * bList = NvM_Config.BlockDescriptor;

    /* Check for new requested state changes */
    if( nvmState == NVM_IDLE ) {
        rv = (uint32)CirqBuffPop( &nvmQueue, &qEntry );
        if( rv == 0 ) {
            /* Found something in buffer */
            nvmState = qEntry.op;
            nvmBlock = &bList[qEntry.blockId-1];
            admBlock = &AdminBlock[qEntry.blockId-1];
            nvmSubState = NS_INIT;
            admBlock->ErrorStatus = NVM_REQ_PENDING;
            serviceId = qEntry.serviceId;
            DEBUG_PRINTF("### Popped Single FIFO : %s\n",StateToStr[qEntry.op]);
            DEBUG_PRINTF("### CRC On:%d Ram:%d Type:%d\n",nvmBlock->BlockUseCrc, nvmBlock->CalcRamBlockCrc, nvmBlock->BlockCRCType );
            DEBUG_PRINTF("### RAM:%x ROM:%x\n", nvmBlock->RamBlockDataAddress, nvmBlock->RomBlockDataAdress );
        } else {
            /* Check multiblock req and do after all single block reads */
            /* @req NVM243 *//* Processed when single block job queue is empty */
            /* @req NVM254 *//* Processed when single block job queue is empty */
            if( AdminMultiReq.state != NVM_UNINITIALIZED ) {
                nvmState = AdminMultiReq.state;
                nvmSubState = NS_INIT;
                nvmBlock = NULL_PTR;
                admBlock = NULL_PTR;
                serviceId = AdminMultiReq.serviceId;
                AdminMultiReq.state = NVM_UNINITIALIZED;

                DEBUG_FPUTS("### Popped MULTI\n");
            }
        }
    }

    DEBUG_STATE(nvmState,nvmSubState);

    switch (nvmState) {
    case NVM_UNINITIALIZED:
        break;

    case NVM_IDLE:
    {
        CalcCrc();
        break;
    }
    case NVM_READ_ALL:
        if( NS_INIT == nvmSubState ) {
            if( ReadAllInit()==TRUE) {
                nvmSubState = NS_PENDING;
            } else {
                /* Nothing to do, everything is OK */
                AdminMultiBlock.ErrorStatus = NVM_REQ_OK;
                nvmState = NVM_IDLE;
                nvmSubState = NS_INIT;
                /* @req NVM468 */
                if( NvM_Config.Common.MultiBlockCallback != NULL ) {
                    /* @req NVM686 */
                    NvM_Config.Common.MultiBlockCallback(serviceId, AdminMultiBlock.ErrorStatus);
                }
            }
        } else if( NS_PENDING == nvmSubState ) {
            MultiblockMain( false /* read */ );
        }else{
            /* do nothing */
        }
        break;

    case NVM_READ_BLOCK:
        (void)DriveBlock(nvmBlock,admBlock, qEntry.dataPtr, false, false, false );
        break;

    case NVM_RESTORE_BLOCK_DEFAULTS:
        (void)DriveBlock(nvmBlock,admBlock, qEntry.dataPtr, false, false, true );
        break;

    case NVM_WRITE_BLOCK:
        (void)DriveBlock(nvmBlock,admBlock, qEntry.dataPtr, true /*write*/, false , false );
        break;

    case NVM_WRITE_ALL:
        if( NS_INIT == nvmSubState ) {
            if( WriteAllInit()==TRUE ) {
                nvmSubState = NS_PENDING;
            } else {
                /* Nothing to do, everything is OK */
                AdminMultiBlock.ErrorStatus = NVM_REQ_OK;
                nvmState = NVM_IDLE;
                nvmSubState = NS_INIT;
                /*  @req NVM468 */
                if( NvM_Config.Common.MultiBlockCallback != NULL ) {
                    /* @req NVM686 */
                    NvM_Config.Common.MultiBlockCallback(serviceId, AdminMultiBlock.ErrorStatus);
                }
            }
        } else if( NS_PENDING == nvmSubState ) {
            MultiblockMain(true /* write */ );
        }else{
            /* do nothing */
        }
        break;

    case NVM_INVALIDATE_NV_BLOCK:
    {
        /* @req NVM665 */
        Std_ReturnType ret;
        MemIf_JobResultType jobResult;
        static uint8 blockIndex = 0;

        switch(nvmSubState ) {
        case NS_INIT:
            set = (uint16)admBlock->DataIndex + (uint16)blockIndex;
            blkNum = BLOCK_BASE_AND_SET_TO_BLOCKNR(nvmBlock->NvBlockBaseNumber, set);
            /* @req NVM421 */
            ret = MemIf_InvalidateBlock(nvmBlock->NvramDeviceId,blkNum);

            if (ret != E_OK) {
                /* Do nothing. For MEMIF_UNINIT, MEMIF_BUSY and MEMIF_BUSY_INTERNAL we just stay in the
                 * same state. Better in the next run */
            } else {
                nvmSubState = NS_PENDING;
            }

            break;
        case NS_PENDING:

            jobResult = MemIf_GetJobResult(nvmBlock->NvramDeviceId);
            if( MEMIF_JOB_PENDING == jobResult ) {
                /* Keep on waiting */
            } else if( MEMIF_JOB_OK == jobResult ) {

                /* @req NVM274 */
                if( (nvmBlock->BlockManagementType ==  NVM_BLOCK_REDUNDANT) &&
                    (blockIndex == 0)) {
                    /* First block in the redundant case - need to invalidate more blocks */
                    blockIndex = 1;
                    nvmSubState = NS_INIT;
                } else {
                    /* Both block invalidated when redundant and the single block
                     * invalidated when not redundant.
                     */
                    blockIndex = 0;
                    nvmState = NVM_IDLE;
                    nvmSubState = NS_INIT;
                    admBlock->ErrorStatus = NVM_REQ_OK;
                }
            } else {
                /* MemIf job failed.... */
                /* @req NVM275 *//* @req NVM666  *//* @req NVM592 */
                DEM_REPORTERRORSTATUS(NvM_Config.DemEvents.NvMReqFailedDemEventId, DEM_EVENT_STATUS_FAILED);
                AbortMemIfJob(MEMIF_JOB_FAILED);
                nvmState = NVM_IDLE;
                nvmSubState = NS_INIT;
                admBlock->ErrorStatus = NVM_REQ_NOT_OK;
                blockIndex = 0;
            }
            break;
        default:
            /* do nothing */
            break;
        }
        break;
    }



    default:
        DET_REPORTERROR(NVM_MAIN_FUNCTION_ID, NVM_UNEXPECTED_STATE);
        break;
    }
}


/***************************************
 *  Call-back notifications functions  *
 ***************************************/
#if (NVM_POLLING_MODE == STD_OFF)
/* @req NVM156 *//* Callback runtime is reasonably short */

/*
 * Procedure:    NvM_JobEndNotification
 * Reentrant:    No
 */
/* @req NVM440 */
/* @req NVM462 */
void NvM_JobEndNotification(void)
{
    MemIfJobAdmin.JobFinished = TRUE;
    MemIfJobAdmin.JobStatus = E_OK;
    if(NULL != MemIfJobAdmin.BlockDescriptor) {
        MemIfJobAdmin.JobResult = MemIf_GetJobResult(MemIfJobAdmin.BlockDescriptor->NvramDeviceId);
    } else {
        /* IMPROVEMENT: Det error */
        MemIfJobAdmin.JobResult = MEMIF_JOB_FAILED;
    }
}

/*
 * Procedure:    NvM_JobErrorNotification
 * Reentrant:    No
 */
/* @req NVM463 */
/* @req NVM441 */
void NvM_JobErrorNotification(void)
{
    MemIfJobAdmin.JobFinished = TRUE;
    MemIfJobAdmin.JobStatus = E_NOT_OK;
    if(NULL != MemIfJobAdmin.BlockDescriptor) {
        MemIfJobAdmin.JobResult = MemIf_GetJobResult(MemIfJobAdmin.BlockDescriptor->NvramDeviceId);
    } else {
        /* IMPROVEMENT: Det error */
        MemIfJobAdmin.JobResult = MEMIF_JOB_FAILED;
    }
}
#endif

