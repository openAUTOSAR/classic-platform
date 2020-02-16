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


/*
 * Author: Peter+mahi
 *
 * Part of Release:
 *   3.1.5
 *
 * Description:
 *   Implements the NVRAM Manager module.
 *
 * Support:
 *   General                  Have Support
 *   -------------------------------------------
 *   NVM_API_CONFIG_CLASS           Y NVM_API_CONFIG_CLASS_1 and NVM_API_CONFIG_CLASS_2
 *                                    (Only NvM_InvalidateNvBlock of NVM_API_CONFIG_CLASS_3 )
 *   NVM_COMPILED_CONFIG_ID			N
 *   NVM_CRC_NUM_OF_BYTES   		N
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
 *   NvmBlockCRCType            	Y
 *   NvmBlockJobPriority			N
 *   NvmBlockManagementType 		Y, All blocks supported
 *   NvmBlockUseCrc 				Y
 *   NvmBlockWriteProt				N
 *   NvmCalcRamBlockCrc				Y
 *   NvmInitBlockCallback 			Y
 *   NvmNvBlockBaseNumber			Y
 *   NvmNvBlockLength				Y
 *   NvmNvBlockNum 					Y
 *   NvmNvramBlockIdentifier		Y
 *   NvmNvramDeviceId 				N (always device Id 0)
 *   NvmResistantToChangedSw		N
 *   NvmRomBlockDataAddress			Y
 *   NvmRomBlockNum					Y
 *   NvmSelectBlockForReadall 		Y
 *   NvmSingleBlockCallback 		Y
 *   NvmWriteBlockOnce 				N
 *
 *  Implementation notes:
 *   - The Configuration ID NV Block is generated to the configuration but can't be configured.
 *     The Editor should really force you to pick a block in Ea/Fee that should hold the configuration
 *     ID. The NVM_COMPILED_CONFIG_ID is always generated as 0 now.
 *   - You can ONLY configure one block type for the entire NvM since NvmNvramDeviceId is not supported.
 *     ie "Target Block" must all be eihter FEE or EA blocks.
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
 *          0	  Reserved (NVM478)
 *          1     NVM_BLOCK_NATIVE,    NvBlockNum=1
 *          2	  NVM_BLOCK_REDUNDANT, NvBlockNum=2
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
 *      2                   1				4, (5,6,7)  ****)
 *      3                   2				8, R9,(10,11)  ****)
 *      4                   3				12, D13,D14,D15 ****)
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


/*
 * NB! Even though some code exist for handling RamCrc, the functionality is not complete
 * and shall not be used.
 */

//lint -esym(522,CalcCrc) // 522 PC-Lint exception for empty functions
//lint -emacro(904,DET_VALIDATE_RV,DET_VALIDATE_NO_RV) //904 PC-Lint exception to MISRA 14.7 (validate macros).

// Exception made as a result of that NVM_DATASET_SELECTION_BITS can be zero
//lint -emacro(835, BLOCK_BASE_AND_SET_TO_BLOCKNR) // 835 PC-lint: A zero has been given as right argument to operator '<<' or '>>'

//lint -emacro(506, NVM_ASSERT) // 506 PC-lint: Ok, to have constant value.


/* ----------------------------[includes]------------------------------------*/

#include <assert.h>
#include "NvM.h"
#include "NvM_Cbk.h"
#if defined(CFG_NVM_USE_SERVICE_PORTS)
#include "Rte.h" // ???
#endif
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include "MemIf.h"
//#include "SchM_NvM.h"
#include "MemMap.h"
#include "cirq_buffer.h"
#include "Modules.h"
#include <stdio.h>
#include "io.h"
#include "Crc.h"
#include <string.h>
#include "Cpu.h"

#define FIXME		0

//#define DEBUG_BLOCK	1
#if defined(DEBUG_BLOCK)
#define DEBUG_BLOCK_STATE(_str,_block,_state)       printf("%s BLOCK NR:%d STATE:%d\n",_str,_block, _state); fflush(stdout);
#define DEBUG_STATE(_state,_substate)				printf("MAIN_STATE:%s/%d\n",StateToStr[_state],_substate); fflush(stdout);
#define DEBUG_PRINTF(format,...) 					printf(format,## __VA_ARGS__ ); fflush(stdout);
#define DEBUG_CHECKSUM(_str,_crc)					printf("%s crc=%x\n",_str,_crc);
#define DEBUG_FPUTS(_str) 							fputs((_str),stdout); fflush(stdout);
#else
#define DEBUG_BLOCK_STATE(_str,_block,_state)
#define DEBUG_STATE(_state,_substate)
#define DEBUG_PRINTF(format,...)
#define DEBUG_CHECKSUM(_str,_crc)
#define DEBUG_FPUTS(_str)
#endif


/* ----------------------------[private define]------------------------------*/



#define NVM_BLOCK_ALIGNMENT			4
#define NVM_CHECKSUM_LENGTH			4

#define NVM_BLOCK_OFFSET 		2





/* ----------------------------[private macro]-------------------------------*/

#if defined(DEBUG_BLOCK)
#define NVM_ASSERT(_exp)		if( !(_exp) ) { while(1) {}; } //assert(_exp)
#else
#define NVM_ASSERT(_exp)		if( !(_exp) ) { assert(_exp); } //
#endif


#if  ( NVM_DEV_ERROR_DETECT == STD_ON )
#define DET_REPORT_ERROR( _api, _err) Det_ReportError(MODULE_ID_NVM, 0, _api, _err);
#else
#define DET_REPORT_ERROR( _api, _err)
#endif

#if  ( NVM_DEV_ERROR_DETECT == STD_ON )
#if defined(USE_DET)
#include "Det.h"
#endif
#define DET_VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_NVM, 0, _api, _err); \
        }


#define DET_VALIDATE_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_NVM, 0, _api, _err); \
          return _rv; \
        }

#define DET_VALIDATE_NO_RV(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_NVM, 0, _api, _err); \
          return; \
        }
#define DET_REPORTERROR(_module,_instance,_api,_err) Det_ReportError(_module,_instance,_api,_err)

#else
#define DET_VALIDATE(_exp,_api,_err )
#define DET_VALIDATE_RV(_exp,_api,_err,_rv )
#define DET_VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_module,_instance,_api,_err)
#endif

#define BLOCK_BASE_AND_SET_TO_BLOCKNR(_blockbase, _set)	((uint16)(_blockbase) | (_set))

#if defined(USE_DEM)
#define DEM_REPORTERRORSTATUS(_err,_ev ) Dem_ReportErrorStatus(_err, DEM_EVENT_STATUS_FAILED);
#else
#define DEM_REPORTERRORSTATUS(_err,_ev )
#endif


#define BLOCK_NR_FROM_PTR(_bptr)  		(((_bptr) - NvM_Config.BlockDescriptor + 1))   // sizeof(NvM_BlockDescriptorType))

#define CREATE_ENTRY(_val)	[_val] = #_val




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

char *StateToStr[20] = {
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
//	BLOCK_STATE_START,
	BLOCK_STATE_MEMIF_PROCESS,
//	BLOCK_STATE_MEMIF_CRC_PROCESS,
	BLOCK_STATE_CALC_CRC,
//	BLOCK_STATE_MEMIF_PROCESS_CRC,
	BLOCK_STATE_CALC_CRC_WRITE,
	BLOCK_STATE_CALC_CRC_READ,
//	BLOCK_STATE_LOAD_FROM_NV,
} BlockStateType;


typedef enum {
	BLOCK_SUBSTATE_0,
	BLOCK_SUBSTATE_1,
} BlockSubStateType;


typedef enum {
	NS_INIT = 0,
	NS_PENDING,
//	RB_PROCESSING,
} Nvm_SubStates;


union Nvm_CRC {
	uint16 crc16;
	uint32 crc32;
};


typedef struct {
	NvM_RequestResultType	ErrorStatus;			// Status from multi block requests i.e. Read/Write/CancelWrite-all
} AdministrativeMultiBlockType;

typedef struct {
	BlockStateType			BlockState;
	BlockSubStateType       BlockSubState;
	uint8					DataIndex;				// Selected data index if "Data Set" type
	boolean					BlockWriteProtected;	// Block write protected?
	NvM_RequestResultType	ErrorStatus;			// Status of block
	boolean					BlockChanged;			// Block changed?
	boolean					BlockValid;				// Block valid? (RAM block only?)
	uint8					NumberOfWriteFailed;	// Current write retry cycle
	union Nvm_CRC			RamCrc;
	union Nvm_CRC			NvCrc;					// The CRC of this block, read from NV
	void *					savedDataPtr;			//
	uint8 					crcLen;
	uint8					flags;					// Used for all sorts of things.
} AdministrativeBlockType;

/*
typedef enum {
	NVM_OP_READ_BLOCK,
	NVM_OP_WRITE_BLOCK,
	NVM_OP_RESTORE_BLOCK_DEFAULTS,
} Nvm_OpType;
*/
#define OP_READ		0
#define OP_WRITE	1
#define NO_MULTIBLOCK	0
#define MULTIBLOCK		1


typedef struct {
	NvmStateType		op;
	NvM_BlockIdType blockId;
	uint8 *			dataPtr;	/* Src or Dest ptr */
	uint8 		dataIndex;
	boolean		blockChanged;
	uint8	    serviceId;
} Nvm_QueueType;



/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/

static NvmStateType 				nvmState = NVM_UNINITIALIZED;

#define RB_START			0
#define RB_WAIT_READ		1
#define RB_CALC_CHECKSUM	2

static int 							nvmSubState = 0;
static uint8 						serviceId = NVM_INIT_ID;
//static int nvmSetNr;
static AdministrativeBlockType 		AdminBlock[NVM_NUM_OF_NVRAM_BLOCKS];
static AdministrativeMultiBlockType AdminMultiBlock;


//static Nvm_QueueType  nvmQueueImmData[NVM_SIZE_IMMEDIATE_JOB_QUEUE];
static Nvm_QueueType  nvmQueueData[NVM_SIZE_STANDARD_JOB_QUEUE];

#define _ALIGN(_x,_a)	(((_x) + (_a)  - 1) & ~ ((_a) - 1))

/* Assume undefined is 0 (all compilers do?) */
#if FEE_VIRTUAL_PAGE_SIZE > EA_VIRTUAL_PAGE_SIZE
#define VIRTUAL_PAGE_SIZE FEE_VIRTUAL_PAGE_SIZE
#else
#define VIRTUAL_PAGE_SIZE EA_VIRTUAL_PAGE_SIZE
#endif

/* Define a work buffer that needs have size aligned to atleast xxx_VIRTUAL_PAGE_SIZE
 * + 4 is since the buffer may have CRC */
uint8 Nvm_WorkBuffer[ _ALIGN(NVM_MAX_BLOCK_LENGTH,VIRTUAL_PAGE_SIZE) + 4 ];

#if (NVM_SIZE_STANDARD_JOB_QUEUE == 0)
#error NVM_SIZE_STANDARD_JOB_QUEUE have size 0
#endif


CirqBufferType nvmQueue;


/* ----------------------------[private functions]---------------------------*/


static void setRamBlockStatus( const NvM_BlockDescriptorType	*bPtr,
								AdministrativeBlockType *admPtr,
								boolean blockChanged );


/* ----------------------------[public functions]----------------------------*/



/*
 * This function needs to be implemented!
 */
static void CalcCrc(void)
{
	// TODO: Calculate CRC
}

typedef struct {
	boolean 						JobFinished;
	Std_ReturnType 					JobStatus;
	MemIf_JobResultType 			JobResult;
	const NvM_BlockDescriptorType *	BlockDescriptor;
	AdministrativeBlockType *		BlockAdmin;
} MemIfJobAdminType;

static MemIfJobAdminType MemIfJobAdmin = {
		.JobFinished = TRUE,
		.JobStatus = E_OK,
		.JobResult = MEMIF_JOB_OK,
		.BlockDescriptor = NULL,
		.BlockAdmin = NULL
};

enum Nvm_MultiBlockReq {
	MB_REQ_NONE,
	MB_REQ_READ_ALL,
	MB_REQ_WRITE_ALL,
};

typedef struct {
	NvmStateType			state;
	uint16					currBlockIndex;		// Keeps track of next unfinished block
	NvM_RequestResultType	PendingErrorStatus;	// Status from multi block requests i.e. Read/Write/CancelWrite-all
	uint8					serviceId;
} AdminMultiReqType;

static AdminMultiReqType AdminMultiReq;


/*
 * Set the MemIf job as busy
 */
static void SetMemifJobBusy()
{
	MemIfJobAdmin.JobFinished = FALSE;
}

#if 0
/* The removed this code since it is not used and causes a compile warning*/


#if (NVM_POLLING_MODE == STD_ON)
/*
 * Check if the MemIf job is finished
 */
static boolean CheckMemIfJobFinished(void)
{
	MemIf_JobResultType jobResult;

	if (!MemIfJobAdmin.JobFinished) {
		jobResult = MemIf_GetJobResult(FIXME);

		if (jobResult == MEMIF_JOB_OK) {
			MemIfJobAdmin.JobFinished = TRUE;
			MemIfJobAdmin.JobStatus = E_OK;
			MemIfJobAdmin.JobResult = jobResult;
		} else if (jobResult != MEMIF_JOB_PENDING) {
			MemIfJobAdmin.JobFinished = TRUE;
			MemIfJobAdmin.JobStatus = E_NOT_OK;
			MemIfJobAdmin.JobResult = jobResult;
		}
	}

	return MemIfJobAdmin.JobFinished;
}
#else
/*
 * Check if the MemIf job is finished
 */

static boolean CheckMemIfJobFinished(void)
{
	return MemIfJobAdmin.JobFinished;
}

#endif
#endif

/*
 * Abort the MemIf job with E_NOT_OK
 */
static void AbortMemIfJob(MemIf_JobResultType jobResult)
{
	MemIfJobAdmin.JobFinished = TRUE;
	MemIfJobAdmin.JobStatus = E_NOT_OK;
	MemIfJobAdmin.JobResult = jobResult;
}

#if 0
static boolean CheckJobFailed( void ) {
	return CheckMemIfJobFinished() && (MemIfJobAdmin.JobResult == MEMIF_JOB_FAILED);
}
#endif

/*
 * Initiate the read all job
 */
static boolean ReadAllInit(void)
{
	/*
	 * Initiate the read all job
	 */
	const NvM_BlockDescriptorType	*BlockDescriptorList = NvM_Config.BlockDescriptor;
	AdministrativeBlockType *AdminBlockTable = AdminBlock;
	uint16 i;
	boolean needsProcessing = FALSE;

	// Set status to pending in the administration blocks
	AdminMultiBlock.ErrorStatus = NVM_REQ_PENDING; /** @req 3.1.5/NVM304 */
	AdminMultiReq.PendingErrorStatus = NVM_REQ_OK;
	AdminMultiReq.currBlockIndex = 0;

	for (i = 0; i < ( NVM_NUM_OF_NVRAM_BLOCKS ); i++) {
		if ((BlockDescriptorList->SelectBlockForReadall)
#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)						/** @req NVM345 */
				&& ((!AdminBlockTable->BlockValid)			// TODO: Check if this is to be done like this
				|| (!AdminBlockTable->BlockChanged))		// TODO: Check if this is to be done like this
#endif
				) {
			NVM_ASSERT(BlockDescriptorList->RamBlockDataAddress != NULL);
			DET_VALIDATE_RV(BlockDescriptorList->RamBlockDataAddress != NULL, NVM_READ_ALL_ID, NVM_E_WRONG_CONFIG, FALSE );
			/* 3.1.5/NVM245 */
			NVM_ASSERT(BlockDescriptorList->BlockManagementType != NVM_BLOCK_DATASET);
			DET_VALIDATE_RV(BlockDescriptorList->BlockManagementType != NVM_BLOCK_DATASET, NVM_READ_ALL_ID, NVM_E_WRONG_CONFIG, FALSE );

			AdminBlockTable->ErrorStatus = NVM_REQ_PENDING;
			AdminBlockTable->BlockState = BLOCK_STATE_MEMIF_REQ;
			needsProcessing = TRUE;
		} else {
			AdminBlockTable->ErrorStatus = NVM_REQ_BLOCK_SKIPPED;	/* @req	3.1.5/NVM287 */
		}

		AdminBlockTable++;
		BlockDescriptorList++;
	}

	return needsProcessing;
}


static void writeCrcToBuffer( 	void *bufPtr,
								const NvM_BlockDescriptorType *bPtr,
								AdministrativeBlockType *admPtr )
{
	if( bPtr->BlockUseCrc ) {
		if(bPtr->BlockCRCType == NVM_CRC16) {
			WRITE16_NA(bufPtr + bPtr->NvBlockLength,admPtr->RamCrc.crc16);
		} else {
			WRITE32_NA(bufPtr + bPtr->NvBlockLength,admPtr->RamCrc.crc32);
		}
	}
}

#define REDUNDANT_BLOCK_OK				0
#define REDUNDANT_BLOCK_BOTH_FAIL		1
#define REDUNDANT_BLOCK_OK				0


static boolean handleRedundantBlock(const NvM_BlockDescriptorType *bPtr,
									AdministrativeBlockType *admPtr ) {
	boolean cont = 0;
	/* According to 3.1.5/NVM137 we have 2 NV Blocks and 0..1 ROM Blocks */
	/* Req 3.1.5/NVM199 , 3.1.5/NVM279 , 3.1.5/NVM317
	 * 3.1.5/NVM288, 3.1.5/NVM315
	 * */

	if ( bPtr->BlockManagementType == NVM_BLOCK_REDUNDANT ) {
		if( admPtr->flags == 1 ) {
			NVM_ASSERT(bPtr->NvBlockNum == 2);		/* Configuration error */
			admPtr->DataIndex = ((admPtr->DataIndex) ? 0 : 1);
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

static boolean DriveBlock( const NvM_BlockDescriptorType	*bPtr,
							AdministrativeBlockType *admPtr,
							void *dataPtr,
							boolean write,
							boolean multiBlock,
							boolean restoreFromRom )
{
	bool blockDone = 0;

	DEBUG_BLOCK_STATE("DriveBlock", BLOCK_NR_FROM_PTR(bPtr), admPtr->BlockState );

	NVM_ASSERT( admPtr->ErrorStatus == NVM_REQ_PENDING);

	switch (admPtr->BlockState) {
	case BLOCK_STATE_MEMIF_REQ:
	{
		boolean fail = FALSE;
		Std_ReturnType rv;
		void *ramData = (dataPtr != NULL) ?  dataPtr : bPtr->RamBlockDataAddress;

		admPtr->savedDataPtr = ramData;

		if( write ) {
			if( multiBlock && (dataPtr!=NULL)) {
				NVM_ASSERT(0);
			}
			/* Copy to work buffer */
			memcpy( Nvm_WorkBuffer, ramData, bPtr->NvBlockLength );
			/* Add the CRC to write */
			writeCrcToBuffer(Nvm_WorkBuffer, bPtr, admPtr );

			/*
			 * Write the block
			 */
			if (admPtr->DataIndex < bPtr->NvBlockNum) {
				imask_t state;
				SetMemifJobBusy();
				MemIfJobAdmin.BlockAdmin = admPtr;
				MemIfJobAdmin.BlockDescriptor = bPtr;

				Irq_Save(state);

				rv = MemIf_Write(bPtr->NvramDeviceId, BLOCK_BASE_AND_SET_TO_BLOCKNR(bPtr->NvBlockBaseNumber, admPtr->DataIndex), Nvm_WorkBuffer);

				if (rv != E_OK) {
					if ( MemIf_GetStatus(FIXME) == MEMIF_IDLE ) {
						AbortMemIfJob(MEMIF_JOB_FAILED);
						fail = TRUE;
					} else {
						/* Do nothing. For MEMIF_UNINIT, MEMIF_BUSY and MEMIF_BUSY_INTERNAL we just stay in the
						 * same state. Better in the next run */
						Irq_Restore(state);
						break; /* Do NOT advance to next state */
					}
				}
				Irq_Restore(state);
			} else {
				// Error: setNumber out of range
				DET_REPORTERROR(MODULE_ID_NVM, 0, NVM_LOC_WRITE_BLOCK_ID, NVM_PARAM_OUT_OF_RANGE);
				fail = TRUE;
			}

			if (fail) {
				/* Fail the job */
				admPtr->ErrorStatus = NVM_REQ_NOT_OK;
				blockDone = 1;
				break; /* Do NOT advance to next state */
			}

		} else {
			uint8 crcLen = 0;
			/* Read to workbuffer */
			if( bPtr->BlockUseCrc ) {
				crcLen = (bPtr->BlockCRCType == NVM_CRC16) ? 2: 4;
			}

			if( restoreFromRom ) {
				NVM_ASSERT( bPtr->RomBlockDataAdress != NULL );
				/* No CRC on the ROM block */
				memcpy(ramData,bPtr->RomBlockDataAdress,bPtr->NvBlockLength);

				admPtr->ErrorStatus = NVM_REQ_OK;
				blockDone = 1;
				break;		/* Do NOT advance to next state */
			} else {
				uint8 setNumber = admPtr->DataIndex;
				uint16 length = bPtr->NvBlockLength+crcLen;

				/*
				 * Read the Block
				 */

				if (setNumber < bPtr->NvBlockNum) {
					SetMemifJobBusy();
					MemIfJobAdmin.BlockAdmin = admPtr;
					MemIfJobAdmin.BlockDescriptor = bPtr;
					imask_t state;

					/* First reading the MemIf block and then checking MemIf_GetStatus() to determine
					 * if the device BUSY in anyway...is not threadsafe. The way Autosar have defined
					 * it you would have to lock over the MemIf_Read() to be sure.
					 */
					Irq_Save(state);

					/* We want to read from MemIf, but the device may be busy.
					 */
					rv = MemIf_Read(bPtr->NvramDeviceId,
											BLOCK_BASE_AND_SET_TO_BLOCKNR(bPtr->NvBlockBaseNumber, setNumber),
											0,
											Nvm_WorkBuffer,
											length );
					if (rv != E_OK) {
						if ( MemIf_GetStatus(FIXME) == MEMIF_IDLE ) {
							AbortMemIfJob(MEMIF_JOB_FAILED);
							fail = TRUE;
						} else {
							/* Do nothing. For MEMIF_UNINIT, MEMIF_BUSY and MEMIF_BUSY_INTERNAL we just stay in the
							 * same state. Better in the next run */
							Irq_Restore(state);
							break; /* Do NOT advance to next state */
						}
					}
					Irq_Restore(state);

				} else if (setNumber < bPtr->NvBlockNum + bPtr->RomBlockNum) {
					// TODO: Read from ROM
				} else {
					// Error: setNumber out of range
					DET_REPORTERROR(MODULE_ID_NVM, 0, NVM_LOC_READ_BLOCK_ID, NVM_PARAM_OUT_OF_RANGE);
					fail = TRUE;
				}

				if( fail ) {
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
		/* Check read */
		MemIf_JobResultType jobResult = MemIf_GetJobResult(FIXME);

		if( MEMIF_JOB_PENDING == jobResult ) {
			/* Keep on waiting */
		} else if( MEMIF_JOB_OK == jobResult ) {
			/* We are done */
			if( write ) {
				admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
				admPtr->ErrorStatus = NVM_REQ_OK;
				blockDone = 1;
			} else {
				/* read */
				uint8 crcLen = 0;
				if( bPtr->BlockUseCrc ) {

					/* The read data is in the work buffer, read the CRC */
					if( bPtr->BlockCRCType == NVM_CRC16) {
						admPtr->NvCrc.crc16 = READ16_NA( Nvm_WorkBuffer + bPtr->NvBlockLength );
						crcLen = 2;
						DEBUG_PRINTF(">> Nv CRC %04x\n",admPtr->NvCrc.crc16);
						admPtr->RamCrc.crc16 = admPtr->NvCrc.crc16;	/* Set RAM CRC = NvRAM CRC */
					} else {
						admPtr->NvCrc.crc32 = READ32_NA( Nvm_WorkBuffer + bPtr->NvBlockLength );
						crcLen = 4;
						DEBUG_PRINTF(">> Nv CRC %08x\n",admPtr->NvCrc.crc16);
						admPtr->RamCrc.crc32 = admPtr->NvCrc.crc32;	/* Set RAM CRC = NvRAM CRC */
					}

					/* 3.1.5/NVM201 + 3.1.5/NVM292 NvM_ReadBlock() + NvM_ReadAll() should request
					 * recalculation of the RAM block data if configured with CRC.
					 */

					/* savedDataPtr points to the real data buffers and they do no contain the
					 * crcLen */
					memcpy(admPtr->savedDataPtr, Nvm_WorkBuffer, bPtr->NvBlockLength  );

					/* Check if we should re-calculate the RAM checksum now when it's in RAM
					 * 3.1.5/NVM165 */
					if( bPtr->CalcRamBlockCrc ) {
						/* This block want its RAM block CRC checked */
						DEBUG_PRINTF(">> Recalculation of RAM checksum \n",admPtr->NvCrc.crc16);
						assert( bPtr->BlockUseCrc == 1);
						admPtr->BlockState = BLOCK_STATE_CALC_CRC_READ;
						admPtr->BlockSubState = BLOCK_SUBSTATE_0;
						admPtr->flags = 0;

					} else {
						/* Done */
						admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
						admPtr->ErrorStatus = NVM_REQ_OK;
						blockDone = 1;
					}

				} else {
					DEBUG_FPUTS(">> Block have NO CRC\n");

					memcpy(admPtr->savedDataPtr, Nvm_WorkBuffer, bPtr->NvBlockLength  + crcLen );
					/* Done */
					admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
					admPtr->ErrorStatus = NVM_REQ_OK;
					blockDone = 1;
				}

				/* Copy from Workbuffer to the real buffer */

			}
			break;
		} else {
			/* Something failed */
			DEBUG_FPUTS(">> Read/Write FAILED\n");
			if( write ) {
				admPtr->NumberOfWriteFailed++;
				if( admPtr->NumberOfWriteFailed > NVM_MAX_NUMBER_OF_WRITE_RETRIES ) {
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
                        /* @req 3.1.5/NVM342 */
                        admPtr->ErrorStatus = NVM_REQ_NV_INVALIDATED;
                        break;
                    case MEMIF_BLOCK_INCONSISTENT:
                        /* @req 3.1.5/NVM360 but is overridden by NVM172 (implicit revovery) */
                        admPtr->ErrorStatus = NVM_REQ_INTEGRITY_FAILED;
                        DEM_REPORTERRORSTATUS(NVM_E_INTEGRITY_FAILED,DEM_EVENT_STATUS_FAILED);
                        break;
                    case MEMIF_JOB_FAILED:
                        /* @req 3.1.5/NVM361 */
                        admPtr->ErrorStatus = NVM_REQ_NOT_OK;
                        DEM_REPORTERRORSTATUS(NVM_E_REQ_FAILED,DEM_EVENT_STATUS_FAILED);
                        break;
                    default:
                        DEBUG_PRINTF("## Unexpected jobResult:%d\n",jobResult);
                        NVM_ASSERT(0);
                        break;
                    }

					if( ( FALSE == write ) && ( bPtr->RomBlockDataAdress != NULL ||bPtr->InitBlockCallback != NULL ) ){
						if( bPtr->RomBlockDataAdress != NULL ) {
							DEBUG_FPUTS("Copying ROM data to block\n");
							memcpy(bPtr->RamBlockDataAddress, bPtr->RomBlockDataAdress,bPtr->NvBlockLength);
							admPtr->ErrorStatus = NVM_REQ_OK;
						} else if( bPtr->InitBlockCallback != NULL ) {
							/* @req 3.1.5/NVM469 */
							DEBUG_FPUTS("Filling block with default data\n");
							bPtr->InitBlockCallback();
							admPtr->ErrorStatus = NVM_REQ_OK;
						}
					} else {
						admPtr->BlockState = BLOCK_STATE_MEMIF_REQ; /* TODO, this really true for all result below */
						AdminMultiReq.PendingErrorStatus = NVM_REQ_NOT_OK;

					}
				}

			}
		}
		break;
	}

	case BLOCK_STATE_CALC_CRC:
		NVM_ASSERT(0);
		break;

	case BLOCK_STATE_CALC_CRC_WRITE:
	{
		uint16 crc16;
		uint32 crc32;

		/* bPtr->RamBlockDataAddress dataPtr
		 *        NULL                NULL 		 #BAD
		 *        NULL 				  data 		 Use dataPtr
		 *        data				  NULL       Use RamBlockDataAddress
		 *        data 				  data 		 Use dataPtr
		 * */
		void *ramData = ( dataPtr != NULL ) ? dataPtr : bPtr->RamBlockDataAddress;
		NVM_ASSERT( ramData != NULL  );

		/* Calculate RAM CRC checksum */
		if( bPtr->BlockCRCType == NVM_CRC16 ) {

			crc16 = Crc_CalculateCRC16(ramData,bPtr->NvBlockLength, 0xffff, TRUE);
			DEBUG_CHECKSUM("RAM",crc16);

			/* Just save the checksum */
			admPtr->RamCrc.crc16 = crc16;

			/* Write the block */
			admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
		} else {
			/* @req 3.1.5/NVM253 */
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
		NVM_ASSERT(bPtr->CalcRamBlockCrc == true );
		uint16 crc16 = 0;
		uint32 crc32 = 0;
		boolean checksumOk;


		/* @req 3.1.5/NVM253 */
		/* Calculate CRC on the data we just read to RAM. Compare with CRC that is located in NV block */

		if( bPtr->BlockCRCType == NVM_CRC16 ) {
			crc16 = Crc_CalculateCRC16(admPtr->savedDataPtr,bPtr->NvBlockLength,0xffff, TRUE);
		} else {
			crc32 = Crc_CalculateCRC32(admPtr->savedDataPtr,bPtr->NvBlockLength,0xffffffffUL, TRUE);
		}

		switch( admPtr->BlockSubState ) {
		case BLOCK_SUBSTATE_0:

			checksumOk = ( bPtr->BlockCRCType == NVM_CRC16 ) ? ( crc16 ==  admPtr->RamCrc.crc16 ) : ( crc32 ==  admPtr->RamCrc.crc32 );

			/* @req 3.1.5/NVM387 */
			if( checksumOk ) {
				admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;

				DEBUG_CHECKSUM("RAM checksum ok with ", ( bPtr->BlockCRCType == NVM_CRC16 ) ? crc16 : crc32 );
				admPtr->ErrorStatus = NVM_REQ_OK;
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
					if( bPtr->RomBlockDataAdress != NULL ) {
						DEBUG_FPUTS("Copying ROM data to block\n");
						memcpy(bPtr->RamBlockDataAddress, bPtr->RomBlockDataAdress,bPtr->NvBlockLength);
						admPtr->BlockSubState = BLOCK_SUBSTATE_1;
					} else {

						/* @req 3.1.5/NVM469 */
						if( bPtr->InitBlockCallback != NULL ) {

							DEBUG_FPUTS("Filling block with default data\n");
							bPtr->InitBlockCallback();
							admPtr->BlockSubState = BLOCK_SUBSTATE_1;

							/* NVM085 is very vague here, but the says the application should be
							 * able distinguish between when the init-callback have been called
							 * or CRC is corrupt.
							 */

							/* The RAM CRC is at this point not calculated...so we must do this
							 * .. so just stay in this state one more MainFunction.
							 * */

						} else {

							/* We have CRC mismatch -> FAIL */
							DEBUG_FPUTS("### Block FAILED with NVM_REQ_INTEGRITY_FAILED\n");


							/* @req 3.1.5/NVM203 */
							DEM_REPORTERRORSTATUS(NVM_E_INTEGRITY_FAILED,DEM_EVENT_STATUS_FAILED);
							/* @req 3.1.5/NVM204 */
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
			admPtr->ErrorStatus = NVM_REQ_OK;
			admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
			blockDone = 1;
			break;
		default:
			break;
		}

		break;
	}
	default:
		NVM_ASSERT(0);
		break;
	}

	if( blockDone ) {
		DEBUG_FPUTS("# Block Done\n");

		if( admPtr->ErrorStatus == NVM_REQ_OK ) {
			admPtr->BlockChanged = FALSE;
			admPtr->BlockValid = TRUE;
		}


		/*  @req 3.1.5/NVM281 */
		if( bPtr->SingleBlockCallback != NULL ) {
			bPtr->SingleBlockCallback(serviceId, admPtr->ErrorStatus);
		}

		if( !multiBlock ) {
			nvmState = NVM_IDLE;
			nvmSubState = 0;
		}
	}

	return blockDone;
}

static void MultiblockMain( boolean write ) {

	/* Cases:
	 * 1. We process each block until it's finished
	 * 2. We start to process a lot of blocks. The blocks may use different devices
	 *    and should be able to read a lot of them. This includes CRC.
	 *
	 *    1) is much simpler and 2) probably much faster.
	 *    This implementation will use 1) since it's simpler and maximum time that is
	 *    spent in MainFunction() can be controlled much better.
	 */

	while (	(AdminMultiReq.currBlockIndex < NVM_NUM_OF_NVRAM_BLOCKS) ) {

		if( AdminBlock[AdminMultiReq.currBlockIndex].ErrorStatus != NVM_REQ_BLOCK_SKIPPED) {
			if( !DriveBlock(	&NvM_Config.BlockDescriptor[AdminMultiReq.currBlockIndex],
						&AdminBlock[AdminMultiReq.currBlockIndex],
						NULL, write, true, false ) )
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

		/* @req 3.1.5/NVM301 */
		if( NVM_REQ_NOT_OK == AdminMultiReq.PendingErrorStatus ) {
			AdminMultiBlock.ErrorStatus = NVM_REQ_NOT_OK;
		} else {
			AdminMultiBlock.ErrorStatus = NVM_REQ_OK;
		}
		/* Reset state machine */
		nvmState = NVM_IDLE;
		nvmSubState = 0;

		/*  @req 3.1.5/NVM468 */
		if( NvM_Config.Common.MultiBlockCallback != NULL ) {
			NvM_Config.Common.MultiBlockCallback(serviceId, AdminMultiBlock.ErrorStatus);
		}
	}
}


/*
 * Initiate the write all job
 */
static boolean WriteAllInit(void)
{
	const NvM_BlockDescriptorType	*BlockDescriptorList = NvM_Config.BlockDescriptor;
	AdministrativeBlockType *AdminBlockTable = AdminBlock;
	uint16 i;
	boolean needsProcessing = FALSE;

//	nvmState = NVM_WRITE_ALL_PROCESSING;
	AdminMultiReq.PendingErrorStatus = NVM_REQ_OK;
	AdminMultiReq.currBlockIndex = 0;

	for (i = 0; i < NVM_NUM_OF_NVRAM_BLOCKS; i++) {
		if ((BlockDescriptorList->RamBlockDataAddress != NULL)
#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)						/** @req NVM344 */
				&& (AdminBlockTable->BlockValid)					/** @req NVM682 */
				&& (AdminBlockTable->BlockChanged)					/** @req NVM682 */
#endif
				&& (!AdminBlockTable->BlockWriteProtected))			/** @req NVM432 *//** @req NVM433 */
		{
			AdminBlockTable->ErrorStatus = NVM_REQ_PENDING;

			if (BlockDescriptorList->BlockUseCrc) {
				AdminBlockTable->BlockState = BLOCK_STATE_CALC_CRC_WRITE;	/** @req NVM253 */
			} else {
				AdminBlockTable->BlockState = BLOCK_STATE_MEMIF_REQ;
				AdminBlockTable->NumberOfWriteFailed = 0;
			}
			needsProcessing = TRUE;
		} else {
			AdminBlockTable->ErrorStatus = NVM_REQ_BLOCK_SKIPPED;	/** @req NVM298 */
		}

		AdminBlockTable++;
		BlockDescriptorList++;
	}
	return needsProcessing;
}




#if 0
/*
 * Handles the result of one MemIf block write
 */
static void WriteAllCheckWriteResult(void)
{
	if (MemIfJobAdmin.JobStatus == E_OK) {
		// TODO: Check if redundant block shall be written NVM337

		if (MemIfJobAdmin.BlockDescriptor->WriteBlockOnce) {
			MemIfJobAdmin.BlockAdmin->BlockWriteProtected = TRUE;	/** @req NVM329 */
		}
		MemIfJobAdmin.BlockAdmin->BlockState = BLOCK_STATE_MEMIF_REQ;
		MemIfJobAdmin.BlockAdmin->ErrorStatus = NVM_REQ_OK;

		if (MemIfJobAdmin.BlockDescriptor->SingleBlockCallback != NULL) {
			(void)MemIfJobAdmin.BlockDescriptor->SingleBlockCallback(NVM_SERVICE_ID, MemIfJobAdmin.BlockAdmin->ErrorStatus);
		}
	} else {
		MemIfJobAdmin.BlockAdmin->NumberOfWriteFailed++;
		if (MemIfJobAdmin.BlockAdmin->NumberOfWriteFailed > NVM_MAX_NUMBER_OF_WRITE_RETRIES) {
			// TODO: Check if redundant block shall be written NVM337

			// Write has failed
			AdminMultiReq.PendingErrorStatus = NVM_REQ_NOT_OK;

			MemIfJobAdmin.BlockAdmin->BlockState = BLOCK_STATE_MEMIF_REQ;
			MemIfJobAdmin.BlockAdmin->ErrorStatus = NVM_REQ_NOT_OK;	/** @req NVM296 */
#if defined(USE_DEM)
			Dem_ReportErrorStatus(NVM_E_REQ_FAILED,DEM_EVENT_STATUS_FAILED);
#endif

			if (MemIfJobAdmin.BlockDescriptor->SingleBlockCallback != NULL) {
				(void)MemIfJobAdmin.BlockDescriptor->SingleBlockCallback(NVM_SERVICE_ID, MemIfJobAdmin.BlockAdmin->ErrorStatus);
			}
		}
	}
	nvmState = NVM_WRITE_ALL_PROCESSING;
}
#endif



/***************************************
 *    External accessible functions    *
 ***************************************/
/*
 * Procedure:	NvM_Init
 * Reentrant:	No
 */
void NvM_Init(void)
{
	/** @req NVM399 *//** @req NVM193 */
	const NvM_BlockDescriptorType	*BlockDescriptorList = NvM_Config.BlockDescriptor;
	AdministrativeBlockType *AdminBlockTable = AdminBlock;
	uint16 i;


	CirqBuff_Init(&nvmQueue,nvmQueueData,sizeof(nvmQueueData)/sizeof(Nvm_QueueType),sizeof(Nvm_QueueType));

	// Initiate the administration blocks
	for (i = 0; i< NVM_NUM_OF_NVRAM_BLOCKS; i++) {
		if (BlockDescriptorList->BlockManagementType == NVM_BLOCK_DATASET) {
			AdminBlockTable->DataIndex = 0;	/** @req NVM192 */
		}
		AdminBlockTable->BlockWriteProtected = BlockDescriptorList->BlockWriteProt;
		AdminBlockTable->ErrorStatus = NVM_REQ_NOT_OK;
		AdminBlockTable->BlockChanged = FALSE;
		AdminBlockTable->BlockValid = FALSE;
		AdminBlockTable->NumberOfWriteFailed = 0;

		AdminBlockTable++;
		BlockDescriptorList++;
	}

	AdminMultiBlock.ErrorStatus = NVM_REQ_NOT_OK;

	// Set status to initialized
	nvmState = NVM_IDLE;	/** @req 3.1.5/NVM399 */
}


/**
 *
 * Note!
 *   NvM_ReadAll() does not set status here or need to check status of the
 *   any blocks since it's done after all single reads are done.
 */
void NvM_ReadAll(void)
{
    imask_t state;
	DET_VALIDATE_NO_RV(nvmState != NVM_UNINITIALIZED, NVM_READ_ALL_ID, NVM_E_NOT_INITIALIZED);

	Irq_Save(state);
	AdminMultiReq.state = NVM_READ_ALL;
	AdminMultiBlock.ErrorStatus = NVM_REQ_PENDING;
	AdminMultiReq.serviceId = NVM_READ_ALL_ID;
	Irq_Restore(state);
}



/*
 * Procedure:	NvM_WriteAll
 * Reentrant:	No
 */
void NvM_WriteAll(void)
{
    imask_t state;
	DET_VALIDATE_NO_RV(nvmState != NVM_UNINITIALIZED, NVM_READ_ALL_ID, NVM_E_NOT_INITIALIZED);

	Irq_Save(state);
	AdminMultiReq.state = NVM_WRITE_ALL;
	AdminMultiBlock.ErrorStatus = NVM_REQ_PENDING;
	AdminMultiReq.serviceId = NVM_WRITE_ALL_ID;
	Irq_Restore(state);
}


/*
 * Procedure:	NvM_CancelWriteAll
 * Reentrant:	No
 */
void NvM_CancelWriteAll(void)
{
	
}


#if (NVM_API_CONFIG_CLASS > NVM_API_CONFIG_CLASS_2)
Std_ReturnType NvM_InvalidateNvBlock( NvM_BlockIdType blockId  )
{
    Nvm_QueueType qEntry;
    const NvM_BlockDescriptorType * bPtr;
    AdministrativeBlockType *       admPtr;
    int rv;

    NVM_ASSERT( blockId >= 2 ); /* No support for lower numbers, yet */

    /* @req 4.0.3|3.1.5/NVM027 */
    DET_VALIDATE_RV(blockId <= NVM_NUM_OF_NVRAM_BLOCKS,
                    NVM_INVALIDATENV_BLOCK_ID, NVM_E_PARAM_BLOCK_ID, E_NOT_OK);
    DET_VALIDATE_RV(nvmState != NVM_UNINITIALIZED,
                    NVM_INVALIDATENV_BLOCK_ID, NVM_E_NOT_INITIALIZED, E_NOT_OK);


    bPtr = &NvM_Config.BlockDescriptor[blockId-1];
    admPtr = &AdminBlock[blockId-1];

    /* @req 4.0.3|3.1.5/NVM027 */
    DET_VALIDATE_RV((admPtr->ErrorStatus != NVM_REQ_PENDING),
                     NVM_INVALIDATENV_BLOCK_ID, NVM_E_BLOCK_PENDING , E_NOT_OK );

    /* @req 4.0.3|3.1.5/NVM423 */
    /* @req 4.0.3|3.1.5/NVM272 */
    /* @req 4.0.3|3.1.5/NVM273 */
    if( admPtr->BlockWriteProtected || admPtr->DataIndex >= bPtr->NvBlockNum) {
        /* If write protected or ROM block in dataset, return E_NOT_OK */
#if NVM_AR_VERSION >= 40300
        DEM_REPORTERRORSTATUS(NVM_E_INTEGRITY_FAILED, DEM_EVENT_STATUS_FAILED);
#endif
        /* @req 4.0.3|3.1.5/NVM027 */
        DET_REPORTERROR(MODULE_ID_NVM, NVM_INVALIDATENV_BLOCK_ID,
                        NVM_WRITE_BLOCK_ID, NVM_E_NV_WRITE_PROTECTED);

        return E_NOT_OK;
    }

    /* @req 3.1.5|4.0.3/NVM424 */
    /* @req 3.1.5|4.0.3/NVM239 */
    qEntry.blockId = blockId;
    qEntry.op = NVM_INVALIDATE_NV_BLOCK;
    qEntry.serviceId = NVM_INVALIDATENV_BLOCK_ID;
    rv = CirqBuffPush(&nvmQueue,&qEntry);

    if(0 != rv) {
        /* @req 4.0.3|3.1.5/NVM027 */
        DET_REPORTERROR(MODULE_ID_NVM, NVM_INVALIDATENV_BLOCK_ID,
                        NVM_WRITE_BLOCK_ID, NVM_E_LIST_OVERFLOW);
        return E_NOT_OK;
    }

    /* req 3.1.5/NVM185 */
    admPtr->ErrorStatus = NVM_REQ_PENDING;

    return E_OK;

    /* !req 4.0.3|3.1.5/NVM422 Leave the RAM block unmodified */

    /* These are not requirements on this module (same req but different numbering) */
    /* !req 3.1.5/NVM425 The environment must init the module before calling this funcation */
    /* !req 4.0.3/NVM717 The environment must init the module before calling this funcation */

}
#endif


/*
 * Procedure:	NvM_GetErrorStatus
 * Reentrant:	Yes
 */
Std_ReturnType NvM_GetErrorStatus(NvM_BlockIdType blockId, NvM_RequestResultType *requestResultPtr)
{
	DET_VALIDATE_RV(nvmState != NVM_UNINITIALIZED, NVM_GET_ERROR_STATUS_ID, NVM_E_NOT_INITIALIZED, E_NOT_OK );
	DET_VALIDATE_RV(blockId < NVM_NUM_OF_NVRAM_BLOCKS+1, NVM_GET_ERROR_STATUS_ID, NVM_E_PARAM_BLOCK_ID, E_NOT_OK );

	if (blockId == 0) {
		// Multiblock ID
		*requestResultPtr = AdminMultiBlock.ErrorStatus;
	} else if (blockId == 1) {
		/* TODO Configuration ID */
	  *requestResultPtr = NVM_REQ_OK;
	} else {
		*requestResultPtr = AdminBlock[blockId-1].ErrorStatus;
	}
	return E_OK;
}


#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)	/** @req NVM408 */
/*
 * Procedure:	Nvm_SetRamBlockStatus
 * Reentrant:	Yes
 */
Std_ReturnType NvM_SetRamBlockStatus(NvM_BlockIdType blockId, boolean blockChanged)
{
	AdministrativeBlockType * 		admPtr;
	Nvm_QueueType qEntry;
	int rv;

	DET_VALIDATE_RV(blockId < NVM_NUM_OF_NVRAM_BLOCKS+1, NVM_SET_RAM_BLOCK_STATUS_ID, NVM_E_PARAM_BLOCK_ID, E_NOT_OK);

	admPtr = &AdminBlock[blockId-1];

	DET_VALIDATE_RV(nvmState != NVM_UNINITIALIZED, NVM_SET_RAM_BLOCK_STATUS_ID, NVM_E_NOT_INITIALIZED, E_NOT_OK);	/** @req NVM497 */
	DET_VALIDATE_RV(blockId > 1, NVM_SET_RAM_BLOCK_STATUS_ID, NVM_E_PARAM_BLOCK_ID, E_NOT_OK );
	DET_VALIDATE_RV( (admPtr->ErrorStatus != NVM_REQ_PENDING), 0, NVM_E_BLOCK_PENDING , E_NOT_OK );

	qEntry.blockId = blockId;
	qEntry.op = NVM_SETRAMBLOCKSTATUS;
	qEntry.blockChanged = blockChanged;
	qEntry.serviceId = NVM_SET_RAM_BLOCK_STATUS_ID;
	rv = CirqBuffPush(&nvmQueue,&qEntry);

	if(0 != rv) {
		DET_REPORTERROR(MODULE_ID_NVM, 0, NVM_SET_RAM_BLOCK_STATUS_ID, NVM_E_LIST_OVERFLOW);
		return E_NOT_OK;
	}
	/* req 3.1.5/NVM185 */
	admPtr->ErrorStatus = NVM_REQ_PENDING;

	return E_OK;
}
#endif

static void setRamBlockStatus( const NvM_BlockDescriptorType	*bPtr, AdministrativeBlockType *admPtr, boolean blockChanged ) {

	if (bPtr->RamBlockDataAddress != NULL) {	/** @req NVM240 */
		if (blockChanged) {
			admPtr->BlockChanged = TRUE;	/** @req NVM406 */
			admPtr->BlockValid = TRUE;	/** @req NVM241 */
//	TODO?	if (bPtr->BlockUseCrc) {
//				admPtr->BlockState = BLOCK_STATE_CALC_CRC;	/** @req NVM121 */
//			}
		} else {
			admPtr->BlockChanged = FALSE;	/** @req NVM405 */
			admPtr->BlockValid = FALSE;
		} // else blockChanged
	} // if permanent block
}

void NvM_SetBlockLockStatus( NvM_BlockIdType blockId, boolean blockLocked ) {
	(void)blockId;
	(void)blockLocked;
}


/**
 * Restore default data to its corresponding RAM block.
 *
 * @param BlockId		NVRAM block identifier.
 * @param NvM_DestPtr  	Pointer to the RAM block
 * @return
 */
Std_ReturnType NvM_RestoreBlockDefaults( NvM_BlockIdType blockId, uint8* NvM_DestPtr )
{
	/* !req 3.1.5/NVM012 */	/* !req 3.1.5/NVM267 */	/* !req 3.1.5/NVM266 */
	/* !req 3.1.5/NVM353 */	/* !req 3.1.5/NVM435 */	/* !req 3.1.5/NVM436 */	/* !req 3.1.5/NVM227 */
	/* !req 3.1.5/NVM228 */	/* !req 3.1.5/NVM229 */	/* !req 3.1.5/NVM413 */

	const NvM_BlockDescriptorType *	bPtr;
	AdministrativeBlockType * 		admPtr;
	Nvm_QueueType qEntry;
	int rv;

	NVM_ASSERT( blockId >= 2 );	/* No support for lower numbers, yet */

	/* @req 3.1.5/NVM618 */
	DET_VALIDATE_RV( 	blockId <= NVM_NUM_OF_NVRAM_BLOCKS,
					NVM_WRITE_BLOCK_ID,NVM_E_PARAM_BLOCK_ID,E_NOT_OK );

	bPtr = &NvM_Config.BlockDescriptor[blockId-1];
	admPtr = &AdminBlock[blockId-1];

	/** @req 3.1.5/NVM196 */ /** @req 3.1.5/NVM278 */ /** @req 3.1.5/NVM210 */
	/* It must be a permanent RAM block but no RamBlockDataAddress -> error */
	DET_VALIDATE_RV( !((NvM_DestPtr == NULL) &&  ( bPtr->RamBlockDataAddress == NULL )),
						NVM_WRITE_BLOCK_ID,NVM_E_PARAM_BLOCK_ID,E_NOT_OK );

	DET_VALIDATE_RV( (admPtr->ErrorStatus != NVM_REQ_PENDING), 0, NVM_E_BLOCK_PENDING , E_NOT_OK );

	/* @req 3.1.5/NVM195 */
	qEntry.blockId = blockId;
	qEntry.op = NVM_RESTORE_BLOCK_DEFAULTS;
	qEntry.blockId = blockId;
	qEntry.dataPtr = (uint8_t *)NvM_DestPtr;
	qEntry.serviceId = NVM_RESTORE_BLOCK_DEFAULTS_ID;
	rv = CirqBuffPush(&nvmQueue,&qEntry);

	if(0 != rv) {
		DET_REPORTERROR(MODULE_ID_NVM, 0, NVM_RESTORE_BLOCK_DEFAULTS_ID, NVM_E_LIST_OVERFLOW);
		return E_NOT_OK;
	}

	/* req 3.1.5/NVM185 */
	admPtr->ErrorStatus = NVM_REQ_PENDING;

	if( bPtr->BlockUseCrc) {
		admPtr->BlockState = BLOCK_STATE_CALC_CRC_WRITE;
	} else {
		admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
	}


	return E_OK;
}

/**
 * Service to copy the data NV block to the RAM block
 *
 * @param blockId		0 and 1 reserved. The block ID are sequential.
 * @param NvM_DstPtr
 * @return
 */


Std_ReturnType NvM_ReadBlock( NvM_BlockIdType blockId, uint8* NvM_DstPtr )
{
	/* !req 3.1.5/NVM010 */


	/* !req 3.1.5/NVM278 */
	/* !req 3.1.5/NVM340 */
	/* !req 3.1.5/NVM354 */
	/* !req 3.1.5/NVM200 */
	/* !req 3.1.5/NVM366 */
	/* !req 3.1.5/NVM206 */
	/* !req 3.1.5/NVM341 */
	/* !req 3.1.5/NVM358 */
	/* !req 3.1.5/NVM359 */
	/* !req 3.1.5/NVM279 */
	/* !req 3.1.5/NVM316 */
	/* !req 3.1.5/NVM317 */
	/* !req 3.1.5/NVM201 */
	/* !req 3.1.5/NVM202 */
	/* !req 3.1.5/NVM203 */
	/* !req 3.1.5/NVM204 */
	/* !req 3.1.5/NVM409 */


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
	int rv;

	/** @req 3.1.5/NVM196 */ /** @req 3.1.5/NVM278 */
    DET_VALIDATE_RV( !(( NvM_DstPtr == NULL) &&
    			( NvM_Config.BlockDescriptor[blockId-1].RamBlockDataAddress == NULL )),
    			0, NVM_E_PARAM_ADDRESS  , E_NOT_OK );
	DET_VALIDATE_RV( (AdminBlock[blockId-1].ErrorStatus != NVM_REQ_PENDING), 0, NVM_E_BLOCK_PENDING , E_NOT_OK );

	/* @req 3.1.5/NVM195 */
	qEntry.blockId = blockId;
	qEntry.op = NVM_READ_BLOCK;
	qEntry.blockId = blockId;
	qEntry.dataPtr = NvM_DstPtr;
	qEntry.serviceId = NVM_READ_BLOCK_ID;
	rv = CirqBuffPush(&nvmQueue,&qEntry);

	if(0 != rv) {
		DET_REPORTERROR(MODULE_ID_NVM, 0, NVM_READ_BLOCK_ID, NVM_E_LIST_OVERFLOW);
		return E_NOT_OK;
	}

	/* req 3.1.5/NVM185 */
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
Std_ReturnType NvM_WriteBlock( NvM_BlockIdType blockId, const uint8* NvM_SrcPtr ) {

	const NvM_BlockDescriptorType *	bPtr;
	AdministrativeBlockType * 		admPtr;
	Nvm_QueueType qEntry;
	int rv;

	NVM_ASSERT( blockId >= 2 );	/* No support for lower numbers, yet */

	/* @req 3.1.5/NVM618 */
	DET_VALIDATE_RV( 	blockId <= NVM_NUM_OF_NVRAM_BLOCKS,
					NVM_WRITE_BLOCK_ID,NVM_E_PARAM_BLOCK_ID,E_NOT_OK );

	bPtr = &NvM_Config.BlockDescriptor[blockId-1];
	admPtr = &AdminBlock[blockId-1];

	/** @req 3.1.5/NVM196 */ /** @req 3.1.5/NVM278 */
	DET_VALIDATE_RV( !((NvM_SrcPtr == NULL) && ( bPtr->RamBlockDataAddress == NULL )),
				0, NVM_E_PARAM_ADDRESS, E_NOT_OK );
	DET_VALIDATE_RV( (admPtr->ErrorStatus != NVM_REQ_PENDING), 0, NVM_E_BLOCK_PENDING , E_NOT_OK );

	/* @req 3.1.5/NVM195 */
	qEntry.blockId = blockId;
	qEntry.op = NVM_WRITE_BLOCK;
	qEntry.blockId = blockId;
	qEntry.dataPtr = (uint8_t *)NvM_SrcPtr;
	qEntry.serviceId = NVM_WRITE_BLOCK_ID;
	rv = CirqBuffPush(&nvmQueue,&qEntry);

	if(0 != rv) {
		DET_REPORTERROR(MODULE_ID_NVM, 0, NVM_WRITE_BLOCK_ID, NVM_E_LIST_OVERFLOW);
		return E_NOT_OK;
	}

	/* req 3.1.5/NVM185 */
	admPtr->ErrorStatus = NVM_REQ_PENDING;

	if( bPtr->BlockUseCrc) {
		admPtr->BlockState = BLOCK_STATE_CALC_CRC_WRITE;
	} else {
		admPtr->BlockState = BLOCK_STATE_MEMIF_REQ;
	}


	return E_OK;
}

/* Missing from Class 2
 * - NvM_CancelWriteAll
 * */

//#if (NVM_API_CONFIG_CLASS > NVM_API_CONFIG_CLASS_1)

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
Std_ReturnType NvM_SetDataIndex( NvM_BlockIdType blockId, uint8 dataIndex ) {
#if  ( NVM_DEV_ERROR_DETECT == STD_ON )
	const NvM_BlockDescriptorType *	bPtr = &NvM_Config.BlockDescriptor[blockId-1];
#endif
	AdministrativeBlockType * 		admPtr = &AdminBlock[blockId-1];

	Nvm_QueueType qEntry;
	int rv;

	NVM_ASSERT( blockId >= 2 );	/* No support for lower numbers, yet */

	DET_VALIDATE_RV(nvmState != NVM_UNINITIALIZED, NVM_SET_DATA_INDEX_ID, NVM_E_NOT_INITIALIZED, E_NOT_OK);
	DET_VALIDATE_RV(blockId < NVM_NUM_OF_NVRAM_BLOCKS+1, NVM_SET_DATA_INDEX_ID, NVM_E_PARAM_BLOCK_ID, E_NOT_OK);
	DET_VALIDATE_RV(bPtr->BlockManagementType != NVM_BLOCK_NATIVE ,  NVM_SET_DATA_INDEX_ID, NVM_E_PARAM_BLOCK_TYPE , E_NOT_OK);
	DET_VALIDATE_RV(dataIndex < bPtr->NvBlockNum + bPtr->RomBlockNum , NVM_SET_DATA_INDEX_ID, NVM_E_PARAM_BLOCK_DATA_IDX ,E_NOT_OK);
	DET_VALIDATE_RV( (admPtr->ErrorStatus != NVM_REQ_PENDING), NVM_SET_DATA_INDEX_ID, NVM_E_BLOCK_PENDING , E_NOT_OK);

	qEntry.blockId = blockId;
	qEntry.op = NVM_SETDATAINDEX;
	qEntry.blockId = blockId;
	qEntry.dataIndex = dataIndex;
	qEntry.serviceId = NVM_SET_DATA_INDEX_ID;
	rv = CirqBuffPush(&nvmQueue,&qEntry);

	if(0 != rv) {
		DET_REPORTERROR(MODULE_ID_NVM, 0, NVM_SET_DATA_INDEX_ID, NVM_E_LIST_OVERFLOW);
		return E_NOT_OK;
	}

	/* req 3.1.5/NVM185 */
	admPtr->ErrorStatus = NVM_REQ_PENDING;
	return E_OK;
}


Std_ReturnType NvM_GetDataIndex( NvM_BlockIdType blockId, uint8 *dataIndexPtr ) {
#if  ( NVM_DEV_ERROR_DETECT == STD_ON )
	const NvM_BlockDescriptorType *	bPtr = &NvM_Config.BlockDescriptor[blockId-1];
#endif
	AdministrativeBlockType * 		admPtr = &AdminBlock[blockId-1];
	Nvm_QueueType qEntry;
	int rv;

	NVM_ASSERT( blockId >= 2 );	/* No support for lower numbers, yet */

	DET_VALIDATE_RV(nvmState != NVM_UNINITIALIZED, NVM_GET_DATA_INDEX_ID, NVM_E_NOT_INITIALIZED, E_NOT_OK);
	DET_VALIDATE_RV(blockId < NVM_NUM_OF_NVRAM_BLOCKS+1, NVM_GET_DATA_INDEX_ID, NVM_E_PARAM_BLOCK_ID, E_NOT_OK);
	DET_VALIDATE_RV(bPtr->BlockManagementType != NVM_BLOCK_NATIVE ,  NVM_GET_DATA_INDEX_ID, NVM_E_PARAM_BLOCK_TYPE , E_NOT_OK);
	DET_VALIDATE_RV( (dataIndexPtr != NULL) , NVM_GET_DATA_INDEX_ID, NVM_E_PARAM_DATA  ,E_NOT_OK);
	DET_VALIDATE_RV( (admPtr->ErrorStatus != NVM_REQ_PENDING), NVM_GET_DATA_INDEX_ID, NVM_E_BLOCK_PENDING , E_NOT_OK);

	qEntry.blockId = blockId;
	qEntry.op = NVM_GETDATAINDEX;
	qEntry.blockId = blockId;
	qEntry.dataPtr = dataIndexPtr;
	qEntry.serviceId = NVM_GET_DATA_INDEX_ID;
	rv = CirqBuffPush(&nvmQueue,&qEntry);

	if(0 != rv) {
		DET_REPORTERROR(MODULE_ID_NVM, 0, NVM_GET_DATA_INDEX_ID, NVM_E_LIST_OVERFLOW);
		return E_NOT_OK;
	}

	/* req 3.1.5/NVM185 */
	admPtr->ErrorStatus = NVM_REQ_PENDING;
	return E_OK;
}
//#endif

const NvM_BlockDescriptorType *	nvmBlock;
AdministrativeBlockType *admBlock;

/**
 *
 */
void NvM_MainFunction(void)
{
	int 			rv;
	static Nvm_QueueType 	qEntry;
	const NvM_BlockDescriptorType *	bList = NvM_Config.BlockDescriptor;
//	const NvM_BlockDescriptorType *	currBlock;
//	static uint32 crc32;
//	static uint32 crc32Left;

	/* Check for new requested state changes */
	if( nvmState == NVM_IDLE ) {
	    /* req 4.0.3|3.1.5/NVM273 */
		rv = CirqBuffPop( &nvmQueue, &qEntry );
		if( rv == 0 ) {
			/* Found something in buffer */
			nvmState = qEntry.op;
			nvmBlock = &bList[qEntry.blockId-1];
			admBlock = &AdminBlock[qEntry.blockId-1];
			nvmSubState = 0;
			admBlock->ErrorStatus = NVM_REQ_PENDING;
			serviceId = qEntry.serviceId;
			DEBUG_PRINTF("### Popped Single FIFO : %s\n",StateToStr[qEntry.op]);
			DEBUG_PRINTF("### CRC On:%d Ram:%d Type:%d\n",nvmBlock->BlockUseCrc, nvmBlock->CalcRamBlockCrc, nvmBlock->BlockCRCType );
			DEBUG_PRINTF("### RAM:%x ROM:%x\n", nvmBlock->RamBlockDataAddress, nvmBlock->RomBlockDataAdress );
		} else {
			/* Check multiblock req and do after all single block reads (3.1.5/NVM243) */
			if( AdminMultiReq.state != NVM_UNINITIALIZED ) {
				nvmState = AdminMultiReq.state ;
				nvmSubState = 0;
				nvmBlock = 0;
				admBlock = 0;
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
			if( ReadAllInit() ) {
				nvmSubState = NS_PENDING;
			} else {
				/* Nothing to do, everything is OK */
				AdminMultiBlock.ErrorStatus = NVM_REQ_OK;
				nvmState = NVM_IDLE;
				nvmSubState = 0;
			}
		} else if( NS_PENDING == nvmSubState ) {
			MultiblockMain( false /* read */ );
		}
		break;

	case NVM_READ_BLOCK:
		DriveBlock(nvmBlock,admBlock, qEntry.dataPtr, false, false, false );
		break;

	case NVM_RESTORE_BLOCK_DEFAULTS:
		DriveBlock(nvmBlock,admBlock, qEntry.dataPtr, false, false, true );
		break;

	case NVM_WRITE_BLOCK:
		DriveBlock(nvmBlock,admBlock, qEntry.dataPtr, true /*write*/, false , false );
		break;

	case NVM_WRITE_ALL:
		if( NS_INIT == nvmSubState ) {
			if( WriteAllInit() ) {
				nvmSubState = NS_PENDING;
			} else {
				/* Nothing to do, everything is OK */
				AdminMultiBlock.ErrorStatus = NVM_REQ_OK;
				nvmState = NVM_IDLE;
				nvmSubState = 0;
			}
		} else if( NS_PENDING == nvmSubState ) {
			MultiblockMain(true /* write */ );
		}
		break;
	case NVM_SETDATAINDEX:
		admBlock->DataIndex = qEntry.dataIndex;
		nvmState = NVM_IDLE;
		nvmSubState = 0;
		admBlock->ErrorStatus = NVM_REQ_OK;
		break;
	case NVM_GETDATAINDEX:
		*qEntry.dataPtr = admBlock->DataIndex;
		nvmState = NVM_IDLE;
		nvmSubState = 0;
		admBlock->ErrorStatus = NVM_REQ_OK;
		break;
	case NVM_SETRAMBLOCKSTATUS:
		setRamBlockStatus(nvmBlock,admBlock,qEntry.blockChanged );
		nvmState = NVM_IDLE;
		nvmSubState = 0;
		admBlock->ErrorStatus = NVM_REQ_OK;
		break;

    case NVM_INVALIDATE_NV_BLOCK:
    {
        Std_ReturnType rv;
        MemIf_JobResultType jobResult;
        static uint8 blockIndex = 0;

        switch(nvmSubState ) {
        case NS_INIT:

            /* @req 4.0.3|3.1.5/NVM421 */
            rv = MemIf_InvalidateBlock(nvmBlock->NvramDeviceId,
                    BLOCK_BASE_AND_SET_TO_BLOCKNR(nvmBlock->NvBlockBaseNumber, admBlock->DataIndex + blockIndex ));

            if (rv != E_OK) {
                /* Do nothing. For MEMIF_UNINIT, MEMIF_BUSY and MEMIF_BUSY_INTERNAL we just stay in the
                 * same state. Better in the next run */
            } else {
                nvmSubState = NS_PENDING;
            }

            break;
        case NS_PENDING:

            jobResult = MemIf_GetJobResult(FIXME);
            if( MEMIF_JOB_PENDING == jobResult ) {
                /* Keep on waiting */
            } else if( MEMIF_JOB_OK == jobResult ) {

                /* @req 4.0.3|3.1.5/NVM274 */
                if( (nvmBlock->BlockManagementType ==  NVM_BLOCK_REDUNDANT) &&
                    (blockIndex == 0)) {
                    /* First block in the redundant case - need to validate more blocks */
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
                /* @req 4.0.3|3.1.5/NVM275 */
                /* @req 4.0.3/NVM666  */
                DEM_REPORTERRORSTATUS(NVM_E_REQ_FAILED,DEM_EVENT_STATUS_FAILED);
                AbortMemIfJob(MEMIF_JOB_FAILED);
                nvmState = NVM_IDLE;
                nvmSubState = NS_INIT;
                admBlock->ErrorStatus = NVM_REQ_NOT_OK;
                blockIndex = 0;
            }
            break;
        }
        break;
    }



	default:
		DET_REPORTERROR(MODULE_ID_NVM, 0, NVM_MAIN_FUNCTION_ID, NVM_UNEXPECTED_STATE);
		break;
	}
}


/***************************************
 *  Call-back notifications functions  *
 ***************************************/
#if (NVM_POLLING_MODE == STD_OFF)
/*
 * Procedure:	NvM_JobEndNotification
 * Reentrant:	No
 */
void NvM_JobEndNotification(void)
{
	MemIfJobAdmin.JobFinished = TRUE;
	MemIfJobAdmin.JobStatus = E_OK;
	MemIfJobAdmin.JobResult = MemIf_GetJobResult(FIXME);
}

/*
 * Procedure:	NvM_JobErrorNotification
 * Reentrant:	No
 */
void NvM_JobErrorNotification(void)
{
	MemIfJobAdmin.JobFinished = TRUE;
	MemIfJobAdmin.JobStatus = E_NOT_OK;
	MemIfJobAdmin.JobResult = MemIf_GetJobResult(FIXME);
}
#endif

