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

/* General requirement tags */
/** !req EA113 *//* Include structure */
/** @req EA083 *//* Imported types */
/** @req EA117 *//* Not change imported types */
/** @req EA097 *//* Mandatory interface */
/** @req EA163 *//* VARIANT-PRE-COMPILE */
/** @req EA174 *//* No internal management operations performed */
/** @req EA068 *//* Logical blocks must not overlap each other and must not be contained within one another. */

#include "arc_assert.h"
#include "string.h"


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "Ea.h"
#include "Ea_Cbk.h"
#include "Ea_Types.h"

#include "Cpu.h"
//#include "Mcu.h"

#include "SchM_Ea.h"

/*lint -emacro(904,VALIDATE_NO_RV,VALIDATE_RV) *//*904 PC-Lint exception to MISRA 14.7 (validate DET macros).*/

/** @req EA011 */
/** @req EA045 */
#if (STD_ON == EA_DEV_ERROR_DETECT)
/* @req EA059 */
#if defined(USE_DET)
#include "Det.h"
#endif
/* @req EA098 *//* Optional interface Det_ReportError */
#define DET_REPORT_ERROR(_api, _error)  (void)Det_ReportError(EA_MODULE_ID, 0, (_api), (_error))

#define VALIDATE_NO_RV(_exp,_api,_err ) \
  if( !(_exp) ) { \
      DET_REPORT_ERROR((_api), (_err)); \
      return; \
  }

#define VALIDATE_RV(_exp,_api,_err,_ret ) \
  if( !(_exp) ) { \
      DET_REPORT_ERROR((_api), (_err)); \
      return (_ret); \
  }
#else
#define DET_REPORT_ERROR(_api, _error)
#define VALIDATE_NO_RV(_exp,_api,_err )
#define VALIDATE_RV(_exp,_api,_err,_ret )
#endif

#define IS_VALID_BLOCK_NO(_x) ((0x0 != (_x)) && (0xFFFF != (_x)))


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
    uint32					EepAddress; // Start adress to read from
    uint16                  Length;    // The length to read i.e the blockSize
    Ea_JobStatus            JobStatus; // Internal state to be used by main function.
    void*					Address;   // The adress to put the result into
    uint16                  ReadLength; // The length of the block to read/write
    uint16                  Offset; // The offset in the block to read from.
}Ea_GlobalType;

/*
 * Define EA Module Global
 */
/* !req EA155 */
static Ea_GlobalType Ea_Global = {
    .EaBlockConfig = NULL_PTR,
    .EaGeneralPtr = NULL_PTR,
    .ModuleStatus = MEMIF_UNINIT,
    .JobResult = MEMIF_JOB_OK,
    .JobType = EA_JOB_NONE,
};

/* This needs to be aligned since it is accessed as a structure in the code. Hence,
 * the two variables below
 */
static uint32 tempBuffer[((EA_MAX_BLOCK_SIZE + sizeof(Ea_AdminBlock)) / 4) +1 ];
static uint8* Ea_TempBuffer = (uint8*)tempBuffer;
char EaBlockNames[EA_NUMBER_OF_BLOCKS][20];
FILE *EA_Ptr;
MemIf_JobResultType lowerJobStatus = MEMIF_JOB_OK;
/*
 * Function prototypes
 */
static uint16 EA_GET_BLOCK(uint16 BlockNumber);
static uint32 calculateEepAddress(uint16 BlockIndex);
static uint16 calculateBlockLength(uint16 BlockIndex);
static void handleLowerLayerRead(MemIf_JobResultType jobResult);
static void handleLowerLayerWrite(MemIf_JobResultType jobResult);
static void handleLowerLayerErase(MemIf_JobResultType jobResult);
static uint8 verifyChecksum(Ea_AdminBlock* block);
static void addChecksum(Ea_AdminBlock* block);
static Std_ReturnType initFile(void);
static Std_ReturnType readFile(uint8 *DataBufferPtr, uint16 Length);
static Std_ReturnType writeFile(const uint8 *DataBufferPtr, uint16 Length);
static Std_ReturnType eraseFile(uint16 Length);
static int file_exist (char *filename);

static Std_ReturnType eraseFile(uint16 Length){
    uint16 BlockIndex;
    Std_ReturnType ret = E_OK;
	char blank[] = { 0xff };

	BlockIndex = EA_GET_BLOCK(Ea_Global.CurrentBlock);

    printf("Erasing from file %s \n", EaBlockNames[BlockIndex]);
	EA_Ptr = fopen(EaBlockNames[BlockIndex], "rb+");

	if (EA_Ptr == NULL) {
		printf("Unable to open file %s!\n", EaBlockNames[BlockIndex]);
		perror("Error");
		ret = E_NOT_OK;
	} else {
		printf("File open!\n");
		/* Goto begining of the file */
		fseek(EA_Ptr, 0, SEEK_SET);
		fwrite(blank, sizeof(char), Length, EA_Ptr);
		fflush( EA_Ptr );
		fclose(EA_Ptr);
	}

	return ret;
}

static Std_ReturnType writeFile(const uint8 *DataBufferPtr, uint16 Length){
    uint16 BlockIndex;
    Std_ReturnType ret = E_OK;

	BlockIndex = EA_GET_BLOCK(Ea_Global.CurrentBlock);

    printf("Writing to file %s \n", EaBlockNames[BlockIndex]);
	EA_Ptr = fopen(EaBlockNames[BlockIndex], "rb+");

	if (EA_Ptr == NULL) {
		printf("Unable to open file %s!\n", EaBlockNames[BlockIndex]);
		perror("Error");
		ret = E_NOT_OK;
	} else {
		printf("File open!\n");
		/* Goto begining of the file */
		fseek(EA_Ptr, 0, SEEK_SET);
		printf("DataBufferPtr:: %d \n", DataBufferPtr);

		fwrite(DataBufferPtr, sizeof(char), Length, EA_Ptr);
		fflush( EA_Ptr );
		fclose(EA_Ptr);
	}
	return ret;
}


Std_ReturnType readFile(uint8 *DataBufferPtr, uint16 Length) {
    uint16 BlockIndex;
    Std_ReturnType ret = E_OK;
	BlockIndex = EA_GET_BLOCK(Ea_Global.CurrentBlock);
    printf("Reading from file %s \n", EaBlockNames[BlockIndex]);
	EA_Ptr = fopen(EaBlockNames[BlockIndex], "rb+");

	if (EA_Ptr == NULL) {
		printf("Unable to open file %s!\n", EaBlockNames[BlockIndex]);
		perror("Error");
		ret = E_NOT_OK;
	} else {
		printf("File open!\n");
		/* Goto begining of the file */
		fseek(EA_Ptr, 0, SEEK_SET);
		/* Goto position to be written in the file of the file */
		//fseek(EA_Ptr, Ea_Global.Offset + sizeof(Ea_AdminBlock), SEEK_SET);
		fread (DataBufferPtr, 1, Length, EA_Ptr);
		fclose(EA_Ptr);
	    printf("DATA:: %s \n", DataBufferPtr);
	}
	return ret;
}

static int file_exist (char *filename){
	struct stat   buffer;
	return (stat (filename, &buffer) == 0);
}



/**
 * init
 * Initialize the file that will emulate the flash
 *
 * @return: Error in case there is a problem.
 */

static Std_ReturnType initFile(void) {

	Std_ReturnType ret = E_OK;

	char command[50];
	int i=0;

	char blank[] = { 0xff };

	strcpy( command, "ls -l" );
	printf("%s\n",command);
	system(command);
	strcpy( command, "pwd" );
	printf("%s\n",command);
	system(command);
	printf("Check if all required files are already present in current folder\n");

	for( i=0 ; i<EA_NUMBER_OF_BLOCKS ; i++ ) {
		EaBlockNames[i][0] = '\0';
		strcpy( EaBlockNames[i], "EA_" );
		char tempBlockNumber[10];
		sprintf(tempBlockNumber, "%d", Ea_Global.EaBlockConfig[i].EaBlockNumber);
		printf("%s\n",tempBlockNumber);
		(void)strcat(EaBlockNames[i], tempBlockNumber);
		(void)strcat(EaBlockNames[i], ".bin");
		printf("%s\n",EaBlockNames[i]);

		if (file_exist (EaBlockNames[i]))
		{
		  printf ("It exists\n");
		} else {

			printf ("File does not exist\n");
			printf ("Creating file with name %s\n", EaBlockNames[i]);

			strcpy(command, "touch " );
			strcat(command, EaBlockNames[i]);
			printf("%s\n",command);
			system(command);


			strcpy(command, "chmod 777 " );
			strcat(command, EaBlockNames[i]);
			printf("%s\n",command);
			system(command);
		}
		EA_Ptr = fopen(EaBlockNames[i], "rb+");
		if (EA_Ptr == NULL) {
			printf("Unable to open file %s!\n", EaBlockNames[i]);
			perror("Error");
			ret = E_NOT_OK;
		} else {
			//File size check
			fseek(EA_Ptr, 0L, SEEK_END);
			int size = ftell(EA_Ptr);
			rewind(EA_Ptr);

			printf("Current Size of file %s is %d\n", EaBlockNames[i], size);

			if(size != Ea_Global.EaBlockConfig[i].EaBlockSize +  sizeof(Ea_AdminBlock)){
				printf("the expected size is %d", Ea_Global.EaBlockConfig[i].EaBlockSize +  sizeof(Ea_AdminBlock));

				for (int counter = 0; counter < Ea_Global.EaBlockConfig[i].EaBlockSize +  sizeof(Ea_AdminBlock); counter++)
				fwrite(blank, sizeof(char), 1, EA_Ptr);

				fseek(EA_Ptr, 0L, SEEK_END);
				size = ftell(EA_Ptr);
				rewind(EA_Ptr);

				printf("Current Size of file %s is %d\n", EaBlockNames[i], size);
			}
			printf("Initialization done for %s\n", EaBlockNames[i]);
			fclose(EA_Ptr);
		}
	}

	return ret;
}

/* @req EA084 */
/* !req EA017 */
void Ea_Init(void)
{
    /*init internal variables*/
    Ea_Global.EaBlockConfig = Ea_BlockConfigData;
    Ea_Global.EaGeneralPtr = &Ea_GeneralData;
    Ea_Global.JobResult = MEMIF_JOB_OK; /* @req EA128 */
    Ea_Global.JobType = EA_JOB_NONE;
    Ea_Global.JobStatus = EA_PENDING_NONE;
    Ea_Global.ModuleStatus = MEMIF_IDLE; /* @req EA128 */

    initFile();
}

/* @req EA150 */
#if (STD_ON == EA_SET_MODE_SUPPORTED)
/* @req EA085 */
/* @req EA020 */
void Ea_SetMode(MemIf_ModeType Mode)
{
    /* @req EA129 */
    VALIDATE_NO_RV(Ea_Global.ModuleStatus != MEMIF_UNINIT, EA_SETMODE_ID, EA_E_UNINIT);

    /* @req EA165 */
    VALIDATE_NO_RV(Ea_Global.ModuleStatus != MEMIF_BUSY, EA_SETMODE_ID, EA_E_BUSY);

    /* @req EA166 */
    VALIDATE_NO_RV(Ea_Global.ModuleStatus != MEMIF_BUSY_INTERNAL, EA_SETMODE_ID, EA_E_BUSY_INTERNAL);

    /* @req EA020 */
    //Not making any special modes
    //Eep_SetMode(Mode);
}
#endif

/* @req EA086 */
/* @req EA021 */
Std_ReturnType Ea_Read(uint16 BlockNumber, uint16 BlockOffset, uint8* DataBufferPtr, uint16 Length)
{
	printf ("Ea_Read! \n");
    /* @req EA158 */
    uint16 BlockIndex;
    const Ea_BlockConfigType *EaBlockCon = Ea_Global.EaBlockConfig;

    /* @req EA130 */
    VALIDATE_RV(Ea_Global.ModuleStatus != MEMIF_UNINIT, EA_READ_ID, EA_E_UNINIT, E_NOT_OK);

    /* @req EA147 */
    VALIDATE_RV(IS_VALID_BLOCK_NO(BlockNumber), EA_READ_ID, EA_E_INVALID_BLOCK_NO, E_NOT_OK);
    BlockIndex = EA_GET_BLOCK(BlockNumber);
    VALIDATE_RV(BlockIndex < EA_NUMBER_OF_BLOCKS, EA_READ_ID, EA_E_INVALID_BLOCK_NO, E_NOT_OK);

    /* @req EA170 */
    VALIDATE_RV(NULL != DataBufferPtr, EA_READ_ID, EA_E_INVALID_DATA_PTR, E_NOT_OK);

    /* @req EA168 */
    VALIDATE_RV(BlockOffset < EaBlockCon[BlockIndex].EaBlockSize, EA_READ_ID, EA_E_INVALID_BLOCK_OFS, E_NOT_OK);

    /* @req EA169 */
    VALIDATE_RV((BlockOffset + Length) <= EaBlockCon[BlockIndex].EaBlockSize, EA_READ_ID, EA_E_INVALID_BLOCK_LEN, E_NOT_OK);

    /* @req EA137 */
    /* Lock down the module to ourself */
    SchM_Enter_Ea_EA_0();
    if (Ea_Global.ModuleStatus != MEMIF_IDLE)
    {
        /* @req EA167 *//* @req EA179 *//* @req EA180 */
        DET_REPORT_ERROR(EA_READ_ID, (MEMIF_BUSY == Ea_Global.ModuleStatus) ? EA_E_BUSY : EA_E_BUSY_INTERNAL);
        SchM_Exit_Ea_EA_0();
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return E_NOT_OK;
    }
    /*set current state is internal busy*/
    Ea_Global.ModuleStatus = MEMIF_BUSY_INTERNAL;
    SchM_Exit_Ea_EA_0();

    /*whether block device index match underlying driver in which Eep_Read will be invoked*/

    Ea_Global.Address = DataBufferPtr;
    /* @req EA021 :calculate the corresponding memory read address for underlying Eep_Read*/
    Ea_Global.EepAddress = calculateEepAddress(BlockIndex);
    Ea_Global.Length = EaBlockCon[BlockIndex].EaBlockSize +  sizeof(Ea_AdminBlock);
    Ea_Global.Offset = BlockOffset;
    Ea_Global.ReadLength = Length;
    Ea_Global.CurrentBlock = BlockNumber;

    /* @req EA022 */
    Ea_Global.ModuleStatus = MEMIF_BUSY;
    Ea_Global.JobType = EA_JOB_READ;
    Ea_Global.JobResult = MEMIF_JOB_PENDING;
    Ea_Global.JobStatus = EA_PENDING_NONE;

    return E_OK;
}

/* @req EA087 */
/* @req EA024 */
/* @req EA026 */
Std_ReturnType Ea_Write(uint16 BlockNumber, uint8* DataBufferPtr)
{
	printf ("Ea_Write!\n");
    /* !req EA182 */
    /* @req EA159 */
    uint16 BlockIndex;
    const Ea_BlockConfigType *EaBlockCon;
    Ea_AdminBlock* adminBlock;

    /* @req EA131 */
    VALIDATE_RV(Ea_Global.ModuleStatus != MEMIF_UNINIT, EA_WRITE_ID, EA_E_UNINIT, E_NOT_OK);

    /* @req EA148 */
    VALIDATE_RV(IS_VALID_BLOCK_NO(BlockNumber), EA_WRITE_ID, EA_E_INVALID_BLOCK_NO, E_NOT_OK);
    BlockIndex = EA_GET_BLOCK(BlockNumber);
    VALIDATE_RV(BlockIndex < EA_NUMBER_OF_BLOCKS, EA_WRITE_ID, EA_E_INVALID_BLOCK_NO, E_NOT_OK);

    /* @req EA172 */
    VALIDATE_RV(NULL != DataBufferPtr, EA_WRITE_ID, EA_E_INVALID_DATA_PTR, E_NOT_OK);
    /* @req EA137
    */
    /* Lock down the module to ourself */
    SchM_Enter_Ea_EA_0();
    if (Ea_Global.ModuleStatus != MEMIF_IDLE)
    {
        /* @req EA171 *//* @req EA181 *//* @req EA183 */
        DET_REPORT_ERROR(EA_WRITE_ID, (MEMIF_BUSY == Ea_Global.ModuleStatus) ? EA_E_BUSY : EA_E_BUSY_INTERNAL);
        SchM_Exit_Ea_EA_0();
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return E_NOT_OK;
    }
    /*set current state is internal busy*/
    Ea_Global.ModuleStatus = MEMIF_BUSY_INTERNAL;
    SchM_Exit_Ea_EA_0();

    /*whether block device index match underlying driver in which Eep_Read will be invoked*/
    EaBlockCon = Ea_Global.EaBlockConfig;

    /*calculate the EepAddress*/
    Ea_Global.EepAddress = calculateEepAddress(BlockIndex);
    /* IMPROVMENT: Should the length be aligned to the virtual page size? */
    Ea_Global.Length = EaBlockCon[BlockIndex].EaBlockSize + sizeof(Ea_AdminBlock);/* @req EA151 */
    Ea_Global.Address = (void*)DataBufferPtr;
    Ea_Global.CurrentBlock = BlockNumber;

    /* Setup the admin block */
    /* @req EA047 */
    adminBlock = (Ea_AdminBlock*) Ea_TempBuffer;
    adminBlock->blockNum = BlockNumber;
    adminBlock->blockState = BLOCK_INCONSISTENT;
    addChecksum(adminBlock);

    /* Copy the data to the buffer */
    uint8* destMem = (uint8*) (Ea_TempBuffer + sizeof(Ea_AdminBlock));
    memcpy(destMem, DataBufferPtr, EaBlockCon[BlockIndex].EaBlockSize);

    /* @req EA025 [set internal variables set the EA module status to MEMIF_BUSY, set the job result to MEMIF_JOB_PENDING.]*/
    Ea_Global.ModuleStatus = MEMIF_BUSY;
    Ea_Global.JobType = EA_JOB_WRITE;
    Ea_Global.JobResult = MEMIF_JOB_PENDING;
    Ea_Global.JobStatus = EA_PENDING_NONE;

    return E_OK;
}

/* @req EA088 */
void Ea_Cancel(void)
{
    /* @req EA132 */
    VALIDATE_NO_RV(Ea_Global.ModuleStatus != MEMIF_UNINIT, EA_CANCEL_ID, EA_E_UNINIT);

    /* @req EA078 [Reset the Ea module's internal variables to make the module ready for a new job request.]*/
    if( MEMIF_BUSY == Ea_Global.ModuleStatus ) {
        Ea_Global.JobType = EA_JOB_NONE;
        Ea_Global.ModuleStatus = MEMIF_IDLE;
        Ea_Global.JobResult = MEMIF_JOB_CANCELED;

        /* @req EA077 [Call the cancel function of the underlying EEPROM driver.]*/
        //Eep_Cancel();
        lowerJobStatus = MEMIF_JOB_CANCELED;
    } else {
        /* @req EA160 */
        /* @req EA173 */
        DET_REPORT_ERROR(EA_CANCEL_ID, EA_E_INVALID_CANCEL);
    }

}

/* @req EA089 */
MemIf_StatusType Ea_GetStatus(void)
{
    /* @req EA034 */
    /* @req EA156 */
    /* @req EA073 */
    /* @req EA157 */
    return Ea_Global.ModuleStatus;
}

/* @req EA090 */
MemIf_JobResultType Ea_GetJobResult(void)
{
    /* @req EA134 */
    VALIDATE_RV(Ea_Global.ModuleStatus != MEMIF_UNINIT, EA_GETJOBRESULT_ID, EA_E_UNINIT, MEMIF_JOB_FAILED);

    /* @req EA035 */
    return Ea_Global.JobResult;
}

/* @req EA091 */
Std_ReturnType Ea_InvalidateBlock(uint16 BlockNumber)
{
    /* @req EA161 */
    uint16 BlockIndex;
    const Ea_BlockConfigType *EaBlockCon;
    Ea_AdminBlock* adminBlock;
    Std_ReturnType result;

    /* @req EA135 */
    VALIDATE_RV(Ea_Global.ModuleStatus != MEMIF_UNINIT, EA_INVALIDATEBLOCK_ID, EA_E_UNINIT, E_NOT_OK);

    /* @req EA149 */
    VALIDATE_RV(IS_VALID_BLOCK_NO(BlockNumber), EA_INVALIDATEBLOCK_ID, EA_E_INVALID_BLOCK_NO, E_NOT_OK);
    BlockIndex = EA_GET_BLOCK(BlockNumber);
    VALIDATE_RV(BlockIndex < EA_NUMBER_OF_BLOCKS, EA_INVALIDATEBLOCK_ID, EA_E_INVALID_BLOCK_NO, E_NOT_OK);

    /* @req EA137
    */
    /* Lock down the module to ourself */
    SchM_Enter_Ea_EA_0();
    if (Ea_Global.ModuleStatus != MEMIF_IDLE)
    {
        /* @req EA175 *//* @req EA184 */
        DET_REPORT_ERROR(EA_INVALIDATEBLOCK_ID, (MEMIF_BUSY == Ea_Global.ModuleStatus) ? EA_E_BUSY : EA_E_BUSY_INTERNAL);
        SchM_Exit_Ea_EA_0();
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return E_NOT_OK;
    }
    /*set current state is internal busy*/
    Ea_Global.ModuleStatus = MEMIF_BUSY_INTERNAL;
    SchM_Exit_Ea_EA_0();

    /*whether block device index match underlying driver in which Eep_Read will be invoked*/
    EaBlockCon = Ea_Global.EaBlockConfig;

    /* @req EA036 [calculate address]*/
    Ea_Global.EepAddress = calculateEepAddress(BlockIndex);
    Ea_Global.Length = EaBlockCon[BlockIndex].EaBlockSize;
    Ea_Global.CurrentBlock = BlockNumber;
    Ea_Global.ModuleStatus = MEMIF_BUSY;
    Ea_Global.JobType = EA_JOB_WRITE;
    Ea_Global.JobStatus = EA_PENDING_ADMIN_WRITE;
    Ea_Global.JobResult = MEMIF_JOB_PENDING;

    /* @req EA037 [now we calling the erase function of the underlying device driver]*/
    /* We just set the Invalidate status of the admin block */
    /* Setup the admin block to be consistent again*/
    /* @req EA047 */
    adminBlock = (Ea_AdminBlock*) Ea_TempBuffer;
    adminBlock->blockNum = BlockNumber;
    adminBlock->blockState = BLOCK_INVALIDATED;
    addChecksum(adminBlock);
    /* IMPROVMENT: Should the length be aligned to the virtual page size? */
    //result = Eep_Write(Ea_Global.EepAddress, (const uint8*) Ea_TempBuffer, sizeof(Ea_AdminBlock));
    result = writeFile((const uint8*) Ea_TempBuffer, sizeof(Ea_AdminBlock));
    if (E_OK == result)
    {
        SchM_Enter_Ea_EA_0();
        //MemIf_StatusType status = Eep_GetStatus();
        MemIf_StatusType status = MEMIF_BUSY;
        if ((status == MEMIF_BUSY) || (status == MEMIF_BUSY_INTERNAL)){
            Ea_Global.JobStatus = EA_PENDING_ADMIN_WRITE;
        }
        SchM_Exit_Ea_EA_0();
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

/* @req EA093 */
Std_ReturnType Ea_EraseImmediateBlock(uint16 BlockNumber)
{
    /* !req EA064 ?? */
    /* @req EA162 */
    uint16 BlockIndex;
    const Ea_BlockConfigType *EaBlockCon;

    /* @req EA136 */
    VALIDATE_RV(Ea_Global.ModuleStatus != MEMIF_UNINIT, EA_ERASEIMMEDIATEBLOCK_ID, EA_E_UNINIT, E_NOT_OK);

    /* @req EA152 */
    VALIDATE_RV(IS_VALID_BLOCK_NO(BlockNumber), EA_ERASEIMMEDIATEBLOCK_ID, EA_E_INVALID_BLOCK_NO, E_NOT_OK);
    BlockIndex = EA_GET_BLOCK(BlockNumber);
    VALIDATE_RV(BlockIndex < EA_NUMBER_OF_BLOCKS, EA_ERASEIMMEDIATEBLOCK_ID, EA_E_INVALID_BLOCK_NO, E_NOT_OK);

    /* @req EA137
    */
    /* Lock down the module to ourself */
    SchM_Enter_Ea_EA_0();
    if (Ea_Global.ModuleStatus != MEMIF_IDLE)
    {
        /* @req EA176 *//* @req EA185 */
        DET_REPORT_ERROR(EA_ERASEIMMEDIATEBLOCK_ID, (MEMIF_BUSY == Ea_Global.ModuleStatus) ? EA_E_BUSY : EA_E_BUSY_INTERNAL);
        SchM_Exit_Ea_EA_0();
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return E_NOT_OK;
    }
    /*set current state is internal busy*/
    Ea_Global.ModuleStatus = MEMIF_BUSY_INTERNAL;
    SchM_Exit_Ea_EA_0();

    EaBlockCon = Ea_Global.EaBlockConfig;

    /* @req EA065 [check whether the addressed logical block is configured as containing immediate data]*/
    if (!EaBlockCon[BlockIndex].EaImmediateData)
    {
        DET_REPORT_ERROR(EA_ERASEIMMEDIATEBLOCK_ID, EA_E_INVALID_BLOCK_NO);
        Ea_Global.ModuleStatus = MEMIF_IDLE;
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return E_NOT_OK;
    }

    /* @req EA063 [take the block number and calculate the corresponding memory block address.]*/
    Ea_Global.EepAddress = calculateEepAddress(BlockIndex);
    Ea_Global.Length = calculateBlockLength(BlockIndex); // Calculate the block length in full pages
    Ea_Global.CurrentBlock = BlockNumber;

    /* @req EA025 [set internal variables set the EA module status to MEMIF_BUSY, set the job result to MEMIF_JOB_PENDING.]*/
    Ea_Global.ModuleStatus = MEMIF_BUSY;
    Ea_Global.JobType = EA_JOB_ERASE;
    Ea_Global.JobResult = MEMIF_JOB_PENDING;
    Ea_Global.JobStatus = EA_PENDING_NONE;

    return E_OK;
}


/* @req EA096 */
/* @req EA056 */
void Ea_MainFunction(void)
{
	//printf ("Ea_Mainfunction!");
    Std_ReturnType result;

    if ((MEMIF_JOB_PENDING == Ea_Global.JobResult) && (Ea_Global.JobStatus == EA_PENDING_NONE))
    {
        switch (Ea_Global.JobType)
        {
            case EA_JOB_WRITE:
                /* @req EA026 */
//                if (MEMIF_IDLE == Eep_GetStatus())
//                {
                    //result = Eep_Write(Ea_Global.EepAddress, (const uint8*) Ea_TempBuffer, Ea_Global.Length);
                    result = writeFile((const uint8*) Ea_TempBuffer, Ea_Global.Length);
                    if (E_OK == result)
                    {
                        SchM_Enter_Ea_EA_0();
                        //MemIf_StatusType status = Eep_GetStatus();
                        MemIf_StatusType status = MEMIF_BUSY;
                        if ((status == MEMIF_BUSY) || (status == MEMIF_BUSY_INTERNAL)){
                            Ea_Global.JobStatus = EA_PENDING_WRITE;
                        }
                        SchM_Exit_Ea_EA_0();
                    }
//                }
                break;
            case EA_JOB_READ:
                /* @req EA072 */
//                if (MEMIF_IDLE == Eep_GetStatus())
//                {
                    //result = Eep_Read(Ea_Global.EepAddress, (uint8*) Ea_TempBuffer, Ea_Global.Length);
                	result = readFile((uint8*) Ea_TempBuffer, Ea_Global.Length);
                    if (E_OK == result)
                    {
                        SchM_Enter_Ea_EA_0();
                        //MemIf_StatusType status = Eep_GetStatus();
                        MemIf_StatusType status = MEMIF_BUSY;
                        if ((status == MEMIF_BUSY) || (status == MEMIF_BUSY_INTERNAL)){
                            Ea_Global.JobStatus = EA_PENDING_READ;
                        }
                        SchM_Exit_Ea_EA_0();
                    }
//                }
                break;
            case EA_JOB_ERASE:
//                if (MEMIF_IDLE == Eep_GetStatus())
//                {
                    //result = Eep_Erase(Ea_Global.EepAddress, Ea_Global.Length);
                	result = eraseFile(Ea_Global.Length);
                    if (E_OK == result)
                    {
                        SchM_Enter_Ea_EA_0();
                        //MemIf_StatusType status = Eep_GetStatus();
                        MemIf_StatusType status = MEMIF_BUSY;
                        if ((status == MEMIF_BUSY) || (status == MEMIF_BUSY_INTERNAL)){
                            Ea_Global.JobStatus = EA_PENDING_ERASE;
                        }
                        SchM_Exit_Ea_EA_0();
                    }
//                }
                break;

            default:
                break;
        }
    }
    else
    {
        // We have pending jobs. Wait for them to finish.
    	if(Ea_Global.JobStatus != EA_PENDING_NONE) {
    		Ea_JobEndNotification();
    	}
    }
}


/*Callback notifications of the Ea module*/
/* @req EA094 */
/* @req EA153 */
void Ea_JobEndNotification(void)
{
    //MemIf_JobResultType jobResult = Eep_GetJobResult();
    MemIf_JobResultType jobResult = lowerJobStatus;

    if (MEMIF_JOB_CANCELED == jobResult)
    {
        Ea_Global.JobType = EA_JOB_NONE;
        Ea_Global.JobResult = MEMIF_JOB_CANCELED;
        Ea_Global.JobStatus = EA_PENDING_NONE;
        Ea_Global.ModuleStatus = MEMIF_IDLE;
        // Should we  call upper layer here?
    } else {
        /* @req EA051 */
        /* @req EA054 */
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
                /* @req EA141 */
                /* @req EA054 */
                /* @req EA142 */
                /* @req EA143 */
                EA_JOB_END_NOTIFICATION();
                break;
            default:
                ASSERT(0); // Should never come here
                break;
        }
    }
    return;
}

static void handleLowerLayerRead(MemIf_JobResultType jobResult)
{
    Ea_AdminBlock* adminBlock;

    /* Check the admin block i.e the block is consistent */
    /* @req EA104 */
    adminBlock = (Ea_AdminBlock*) Ea_TempBuffer;
    if ((!verifyChecksum(adminBlock)) || (adminBlock->blockState == BLOCK_INCONSISTENT)
            || (adminBlock->blockNum != Ea_Global.CurrentBlock))
    {
        Ea_Global.JobType = EA_JOB_NONE;
        /* @req EA055 */
        Ea_Global.JobResult = MEMIF_BLOCK_INCONSISTENT;
        Ea_Global.JobStatus = EA_PENDING_NONE;
        Ea_Global.ModuleStatus = MEMIF_IDLE;
        EA_JOB_ERROR_NOTIFICATION();
    }
    /* Check if block is invalidated */
    /* @req EA074 */
    else if (adminBlock->blockState == BLOCK_INVALIDATED)
    {
        Ea_Global.JobType = EA_JOB_NONE;
        /* @req EA055 */
        Ea_Global.JobResult = MEMIF_BLOCK_INVALID;
        Ea_Global.JobStatus = EA_PENDING_NONE;
        Ea_Global.ModuleStatus = MEMIF_IDLE;
        EA_JOB_ERROR_NOTIFICATION();
    } else {

        // Copy the data to the destination
        uint8* srcadress = (uint8*) (Ea_TempBuffer + sizeof (Ea_AdminBlock) + Ea_Global.Offset);
        memcpy(Ea_Global.Address, srcadress, Ea_Global.ReadLength);

        /* Sucess */
        Ea_Global.JobType = EA_JOB_NONE;
        Ea_Global.JobStatus = EA_PENDING_NONE;
        Ea_Global.ModuleStatus = MEMIF_IDLE;
        Ea_Global.JobResult = jobResult;
        /* @req EA054 */
        EA_JOB_END_NOTIFICATION();
    }
}

static void handleLowerLayerWrite(MemIf_JobResultType jobResult)
{
    Ea_AdminBlock* adminBlock;
    Std_ReturnType result;

    if (jobResult == MEMIF_JOB_OK)
    {
        /* Setup the admin block to be consistent again*/
        /* @req EA047 */
        adminBlock = (Ea_AdminBlock*) Ea_TempBuffer;
        adminBlock->blockNum = Ea_Global.CurrentBlock;
        adminBlock->blockState = BLOCK_CONSISTENT;
        addChecksum(adminBlock);

        //result = Eep_Write(Ea_Global.EepAddress, (const uint8*) Ea_TempBuffer, sizeof(Ea_AdminBlock));
        result = writeFile((const uint8*) Ea_TempBuffer, sizeof(Ea_AdminBlock));
        if (E_OK == result)
        {
            SchM_Enter_Ea_EA_0();
            //MemIf_StatusType status = Eep_GetStatus();
            MemIf_StatusType status = MEMIF_BUSY;
            if ((status == MEMIF_BUSY) || (status == MEMIF_BUSY_INTERNAL)){
                Ea_Global.JobStatus = EA_PENDING_ADMIN_WRITE;
            }
            SchM_Exit_Ea_EA_0();
        }
        else
        {
            Ea_Global.JobResult = MEMIF_JOB_FAILED;
            Ea_Global.JobType = EA_JOB_NONE;
            Ea_Global.JobStatus = EA_PENDING_NONE;
            Ea_Global.ModuleStatus = MEMIF_IDLE;

            EA_JOB_END_NOTIFICATION();/* IMPROVEMENT: Should this be EA_JOB_ERROR_NOTIFICATION? */
        }
    }
    else
    {
        /* Report error upstreams */
        Ea_Global.JobType = EA_JOB_NONE;
        /* @req EA055 */
        Ea_Global.JobResult = MEMIF_BLOCK_INVALID;
        Ea_Global.JobStatus = EA_PENDING_NONE;
        Ea_Global.ModuleStatus = MEMIF_IDLE;

        EA_JOB_END_NOTIFICATION();/* IMPROVEMENT: Should this be EA_JOB_ERROR_NOTIFICATION? */
    }
}

static void handleLowerLayerErase(MemIf_JobResultType jobResult)
{
    Ea_AdminBlock* adminBlock;
    Std_ReturnType result;

    if (jobResult == MEMIF_JOB_OK)
    {
        /* Setup the admin block to be consistent again*/
        /* @req EA047 */
        adminBlock = (Ea_AdminBlock*) Ea_TempBuffer;
        adminBlock->blockNum = Ea_Global.CurrentBlock;
        adminBlock->blockState = BLOCK_INCONSISTENT;
        addChecksum(adminBlock);

        //result = Eep_Write(Ea_Global.EepAddress, (const uint8*) Ea_TempBuffer, sizeof(Ea_AdminBlock));
        result = writeFile((const uint8*) Ea_TempBuffer, sizeof(Ea_AdminBlock));
        if (E_OK == result)
        {
            SchM_Enter_Ea_EA_0();
            //MemIf_StatusType status = Eep_GetStatus();
            MemIf_StatusType status = MEMIF_BUSY;
            if ((status == MEMIF_BUSY) || (status == MEMIF_BUSY_INTERNAL)){
                Ea_Global.JobStatus = EA_PENDING_ADMIN_WRITE;
            }
            SchM_Exit_Ea_EA_0();
        } else
        {
            Ea_Global.JobResult = MEMIF_JOB_FAILED;
            Ea_Global.JobType = EA_JOB_NONE;
            Ea_Global.JobStatus = EA_PENDING_NONE;
            Ea_Global.ModuleStatus = MEMIF_IDLE;

            EA_JOB_END_NOTIFICATION();/* IMPROVEMENT: Should this be EA_JOB_ERROR_NOTIFICATION? */
        }
    }
    else
    {
        /* Report error upstreams */
        Ea_Global.JobType = EA_JOB_NONE;
        /* @req EA055 */
        Ea_Global.JobResult = MEMIF_BLOCK_INVALID;
        Ea_Global.JobStatus = EA_PENDING_NONE;
        Ea_Global.ModuleStatus = MEMIF_IDLE;

        EA_JOB_END_NOTIFICATION();/* IMPROVEMENT: Should this be EA_JOB_ERROR_NOTIFICATION? */
    }
}


/* @req EA095 */
void Ea_JobErrorNotification(void)
{
    SchM_Enter_Ea_EA_0();

    /* @req EA154*/
    if (Ea_Global.JobResult == MEMIF_JOB_PENDING){
        Ea_Global.JobResult = MEMIF_JOB_FAILED;
    }

    Ea_Global.JobType = EA_JOB_NONE;
    Ea_Global.JobStatus = EA_PENDING_NONE;
    Ea_Global.ModuleStatus = MEMIF_IDLE;
    SchM_Exit_Ea_EA_0();

    /* @req EA055 */
    /* @req EA144 */
    /* @req EA145 */
    /* @req EA146 */
    /* @req EA100 */
    /* @req EA053 */
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
/* @req EA007 */
static uint32 calculateEepAddress(uint16 BlockIndex)
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
