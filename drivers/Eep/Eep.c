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

/** @tagSettings DEFAULT_ARCHITECTURE=PPC|MPC5645S|MPC5607B  */
/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */

//lint -save -e731 -e9033 // PC-Lint Exception : Lint + GHS compiler give strange warnings on the asserts.

/*
 * IMPLEMENTATION NOTES
 * - The SPI implementation only supports 64 bytes in one go so this is
 *   a limitation for the EEP driver also.
 * - The specification if SPI functions should be blocking or not. For now
 *   the driver uses blocking SPI communication.
 *
 * CONFIG NOTES
 *   Look at the 10.4 example in the 4.0 specs, quite good.
 *   Normally the SPI E2 have a number of instructions: READ, WRITE, WRDI, WREN, etc
 *   These instructions have can different sequences. For example WREN can be just
 *   the instruction while WRIE consist of the WRITE instruction, an address and the
 *   data to write.
 *
 *   1. Identify the channels. Best way to do that is to have a look at the E2 instruction
 *      set and its sequences. Example:
 *        READ :  READ| ADDRESS | DATA |
 *         bits    8      16       upto 32*8
 *
 *        WRITE: WRITE | ADDRESS | DATA
 *         bits    8      16       upto 32*8
 *
 *        WREN:   WREN
 *         bits    8
 *
 *        RDSR:    RDSR | DATA
 *                  8       8
 */

/* DEVICE SUPPORT
 *   Microchip:
 *     25LC160
 */

/* REQUIREMENTS
 * - EEP060
 *   Only EEP_WRITE_CYCLE_REDUCTION = STD_OFF is supported
 *
 * - EEP075
 *   MEMIF_COMPARE_UNEQUAL does not exist in the MemIf specification 1.2.1(rel 3.0 )
 *   So, it's not supported. It returns MEMIF_JOB_FAILED instead.
 *
 * - EEP084
 *   EepJobCallCycle not used
 *   We are not using interrupts so EEP_USE_INTERRUPTS must be STD_OFF
 */

#include "Eep.h"
#include "Spi.h"
#if defined(USE_DEM)
#include "Dem.h"
#endif
#if defined(USE_DET)
#include "Det.h"
#endif
#include <stdlib.h>
#include "arc_assert.h"
#include <string.h>

//#define USE_LDEBUG_PRINTF	1
#include "debug.h"
#define MODULE_NAME 	"/driver/Eep"

// Define if you to check if the E2 seems sane at init..
//#define CFG_EEP_CHECK_SANE    1

/* The width in bytes used by this eeprom */
#define ADDR_LENGTH 	2

/* Helper macro for the process function */
#define PROC_SET_STATE(_done,_state) done=(_done);job->state=(_state)
#define DONE        TRUE
#define NOT_DONE    FALSE

#if ( EEP_DEV_ERROR_DETECT == STD_ON ) // Report DEV errors
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(EEP_MODULE_ID,0,_api,_err); \
          return; \
        }

#define VALIDATE_W_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(EEP_MODULE_ID,0,_api,_err); \
          return (_rv); \
        }

#define VALID_CHANNEL(_ch)    ( Gpt_Global.configured & (1<<(_ch)) )

#else // Validate but do not report
#define VALIDATE(_exp,_api,_err )\
        if( !(_exp) ) { \
          return; \
        }
#define VALIDATE_W_RV(_exp,_api,_err,_rv )\
        if( !(_exp) ) { \
          return (_rv); \
        }
#endif

#if ( EEP_DEV_ERROR_DETECT == STD_ON )
#define VALIDATE_CONFIG(_x) CONFIG_ASSERT(_x)
#define DET_REPORTERROR(_x,_y,_z,_q) (void)Det_ReportError(EEP_MODULE_ID, _y, _z, _q)
#else
#define VALIDATE_CONFIG(_x)
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif

#define EEP_JOB_END_NOTIFICATION() \
  if (Eep_Global.config->Eep_JobEndNotification!=NULL) { \
    Eep_Global.config->Eep_JobEndNotification(); \
  }

#define EEP_JOB_ERROR_NOTIFICATION() \
  if (Eep_Global.config->Eep_JobErrorNotification!=NULL) { \
    Eep_Global.config->Eep_JobErrorNotification(); \
  }

/* Job state */
typedef enum {
    EEP_NONE, EEP_COMPARE, EEP_ERASE, EEP_READ, EEP_WRITE,
} Eep_Arc_JobType;

/* Spi job state */
typedef enum {
    JOB_MAIN, JOB_READ_STATUS, JOB_READ_STATUS_RESULT,
} Job_StateType;

/* Information about a job */
typedef struct {
    uint8 *targetAddr;
    Eep_AddressType eepAddr;
    uint32 left;
    Job_StateType state;
    Eep_Arc_JobType mainState;
    Spi_SequenceType currSeq;
    uint32 chunkSize;
    uint32 pageSize;
    boolean initialOp;
} Eep_JobInfoType;

#define JOB_SET_STATE(_x,_y)		job->state=(_x);job->mainState=(_y)

/*
 * Holds all global information that is needed by the driver
 *
 */
typedef struct {
    // The configuration
    const Eep_ConfigType *config;

    // Status of driver
    MemIf_StatusType status;
    MemIf_JobResultType jobResultType;
    Eep_Arc_JobType jobType;

    // Data containers for EB buffers
    Spi_DataBufferType ebCmd;
    Spi_DataBufferType ebReadStatus;

    uint16 ebE2Addr;
    // What mode we are in ( normal/fast )
    MemIf_ModeType mode;

    // Hold job information
    Eep_JobInfoType job;

} Eep_GlobalType;

#if 0  // Use SPI synchronous transmit
#define SPI_TRANSMIT_FUNC(_x, _y)   Spi_SyncTransmit(_x)
#else  // Use SPI asynchronous transmit
#define SPI_TRANSMIT_FUNC(_x,_y)    Eep_AsyncTransmit(_x,_y)
Std_ReturnType Eep_AsyncTransmit(Spi_SequenceType Sequence, Eep_JobInfoType *job);

Std_ReturnType Eep_AsyncTransmit(Spi_SequenceType Sequence, Eep_JobInfoType *job) {
    Std_ReturnType rv;

    // Null-pointer guard
    if (job == NULL)
    {
        DET_REPORTERROR(EEP_MODULE_ID, 0, EEP_GLOBAL_SERVICE_ID, EEP_E_PARAM_POINTER);
        return E_NOT_OK;
    }

    job->currSeq = Sequence;
    rv = Spi_AsyncTransmit(Sequence);
    return rv;
}
#endif

#define CFG_SPI_P() Eep_Global.config->externalDriver

Eep_GlobalType Eep_Global;


#if (EEP_VERSION_INFO_API == STD_ON)
void Eep_GetVersionInfo(Std_VersionInfoType* versioninfo)
{
    VALIDATE( !(versioninfo == NULL), EEP_GETVERSIONINFO_ID, EEP_E_PARAM_POINTER );

    versioninfo->vendorID = EEP_VENDOR_ID;
    versioninfo->moduleID = EEP_MODULE_ID;
    versioninfo->sw_major_version = EEP_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = EEP_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = EEP_SW_PATCH_VERSION;

    return;
}
#endif


/**
 * Converts Eep_AddressType to one that can be read by SPI( Spi_DataBufferType )
 *
 * @param spiAddr Pointer to an address were the result is written.
 * @param eepAddr The Eep address to convert
 */
static void convertToSpiAddr(Spi_DataBufferType *spiAddr, Eep_AddressType eepAddr) {
    // Null-pointer guard
    if (spiAddr == NULL)
    {
        DET_REPORTERROR(EEP_MODULE_ID, 0, EEP_GLOBAL_SERVICE_ID, EEP_E_PARAM_POINTER);
        return;
    }

    *(uint16 *)spiAddr = (uint16)eepAddr;

//    spiAddr[1] = (Spi_DataBufferType)((eepAddr) & 0xffUL);
//    spiAddr[0] = (Spi_DataBufferType)((eepAddr >> 8) & 0xffUL);
}

#if defined(CFG_EEP_CHECK_SANE)
static void commandWREN(void) {
    Eep_Global.ebCmd = E2_WREN;
    (void)Spi_SetupEB(CFG_SPI_P()->EepDataChannel, NULL, NULL, 1);
    (void)Spi_SyncTransmit(CFG_SPI_P()->EepCmdSequence);
}

static void commandWRDI(void) {
    Eep_Global.ebCmd = E2_WRDI;
    (void)Spi_SetupEB(CFG_SPI_P()->EepDataChannel, NULL, NULL, 1);
    (void)Spi_SyncTransmit(CFG_SPI_P()->EepCmdSequence);

}

static uint8 readStatusReg(void) {
    (void)Spi_SetupEB(CFG_SPI_P()->EepDataChannel, NULL, &Eep_Global.ebReadStatus, 1);
    Eep_Global.ebCmd = E2_RDSR;
    (void)Spi_SyncTransmit(CFG_SPI_P()->EepCmd2Sequence);
    return Eep_Global.ebReadStatus;
}
#endif

void Eep_Init(const Eep_ConfigType* ConfigPtr) {
    VALIDATE( (ConfigPtr != NULL), EEP_INIT_ID, EEP_E_PARAM_CONFIG);
    VALIDATE( ( Eep_Global.status != MEMIF_BUSY ), EEP_INIT_ID, EEP_E_BUSY);

    Eep_Global.config = ConfigPtr;

    (void)Spi_SetupEB(CFG_SPI_P()->EepCmdChannel, &Eep_Global.ebCmd, NULL, 1u);
    (void)Spi_SetupEB(CFG_SPI_P()->EepAddrChannel, (Spi_DataBufferType *)&Eep_Global.ebE2Addr, NULL, 1 );
    (void)Spi_SetupEB(CFG_SPI_P()->EepWrenChannel, NULL, NULL, 1);

#if defined( CFG_EEP_CHECK_SANE )
    {
        uint8 status;
        // Simple check,
        // - write WREN,
        // - check if 1 by reading with RDSR,
        // - write WRDE
        // - check if 0 by reading with RDSR,

        commandWREN();

        status = readStatusReg();
        ASSERT(!((status & 0x2u) == 0));
        (void)status; /* Misra compliance */

        commandWRDI();
        status = readStatusReg();
        ASSERT(!((status & 0x2u) > 0));
        (void)status;  /* Misra compliance */
    }
#endif

    Eep_Global.status = MEMIF_IDLE;
    Eep_Global.jobResultType = MEMIF_JOB_OK;

    Eep_SetMode(Eep_Global.config->EepDefaultMode);
}

void Eep_SetMode(MemIf_ModeType Mode) {
    VALIDATE( ( Eep_Global.status != MEMIF_UNINIT ), EEP_SETMODE_ID, EEP_E_UNINIT);
    VALIDATE( ( Eep_Global.status != MEMIF_BUSY ), EEP_SETMODE_ID, EEP_E_BUSY);
    VALIDATE( ( (Mode == MEMIF_MODE_SLOW) || (Mode == MEMIF_MODE_FAST) ), EEP_SETMODE_ID, EEP_E_PARAM_DATA);

    Eep_Global.mode = Mode;
}

Std_ReturnType Eep_Read(Eep_AddressType EepromAddress, uint8 *DataBufferPtr, Eep_LengthType Length) {
    Eep_JobInfoType *job = &Eep_Global.job;

    VALIDATE_W_RV( ( Eep_Global.status != MEMIF_UNINIT ), EEP_READ_ID, EEP_E_UNINIT, E_NOT_OK);
    VALIDATE_W_RV( ( Eep_Global.status != MEMIF_BUSY ), EEP_READ_ID, EEP_E_BUSY, E_NOT_OK);
    /** @req SWS_Eep_00016 */
    VALIDATE_W_RV( ( DataBufferPtr != NULL ), EEP_READ_ID, EEP_E_PARAM_DATA, E_NOT_OK);
    /** @req SWS_Eep_00017 */
    VALIDATE_W_RV( ( (EepromAddress) < (Eep_Global.config->EepSize) ), EEP_READ_ID, EEP_E_PARAM_ADDRESS, E_NOT_OK);
    /** @req SWS_Eep_00018 */
    VALIDATE_W_RV( ( (Length >= 1u) && ((Eep_Global.config->EepSize - EepromAddress) >= Length) ), EEP_READ_ID, EEP_E_PARAM_LENGTH, E_NOT_OK);

    Eep_Global.status = MEMIF_BUSY;
    Eep_Global.jobResultType = MEMIF_JOB_PENDING;
    Eep_Global.jobType = EEP_READ;

    if (Eep_Global.mode == MEMIF_MODE_FAST) {
        job->chunkSize = Eep_Global.config->EepFastReadBlockSize;
    } else {
        job->chunkSize = Eep_Global.config->EepNormalReadBlockSize;
    }

    job->initialOp = TRUE;
    job->currSeq = CFG_SPI_P()->EepReadSequence;
    job->eepAddr = EepromAddress + Eep_Global.config->EepBaseAddress;
    job->targetAddr = DataBufferPtr;
    job->left = Length;

    JOB_SET_STATE(JOB_MAIN, EEP_READ);

    return E_OK;
}

Std_ReturnType Eep_Erase(Eep_AddressType EepromAddress, Eep_LengthType Length) {
    VALIDATE_W_RV( ( Eep_Global.status != MEMIF_UNINIT ), EEP_ERASE_ID, EEP_E_UNINIT, E_NOT_OK);
    VALIDATE_W_RV( ( Eep_Global.status != MEMIF_BUSY ), EEP_ERASE_ID, EEP_E_BUSY, E_NOT_OK);
    /** @req SWS_Eep_00017 */
    VALIDATE_W_RV( ( (EepromAddress) < (Eep_Global.config->EepSize) ), EEP_ERASE_ID, EEP_E_PARAM_ADDRESS, E_NOT_OK);
    /** @req SWS_Eep_00018 */
    VALIDATE_W_RV( ( (Length >= 1u) && ((Eep_Global.config->EepSize - EepromAddress) >= Length) ), EEP_ERASE_ID, EEP_E_PARAM_LENGTH, E_NOT_OK);

    /* IMPROVEMENT : NOT IMPLEMENTED
     * ( Since this E2 do not have erase )
     * */
    Std_ReturnType rv = E_NOT_OK;
    Eep_Global.status = MEMIF_BUSY;
    Eep_Global.status = MEMIF_IDLE;
    return rv;
}

Std_ReturnType Eep_Write(Eep_AddressType EepromAddress, const uint8* DataBufferPtr, Eep_LengthType Length) {
    Eep_JobInfoType *job = &Eep_Global.job;

    VALIDATE_W_RV( ( Eep_Global.status != MEMIF_UNINIT ), EEP_WRITE_ID, EEP_E_UNINIT, E_NOT_OK);
    VALIDATE_W_RV( ( Eep_Global.status != MEMIF_BUSY ), EEP_WRITE_ID, EEP_E_BUSY, E_NOT_OK);
    /** @req SWS_Eep_00016 */
    VALIDATE_W_RV( ( DataBufferPtr != NULL ), EEP_WRITE_ID, EEP_E_PARAM_DATA, E_NOT_OK);
    /** @req SWS_Eep_00017 */
    VALIDATE_W_RV( ( (EepromAddress) < (Eep_Global.config->EepSize) ), EEP_WRITE_ID, EEP_E_PARAM_ADDRESS, E_NOT_OK);
    /** @req SWS_Eep_00018 */
    VALIDATE_W_RV( ( (Length >= 1u) && (Length <= (Eep_Global.config->EepSize - EepromAddress)) ), EEP_WRITE_ID, EEP_E_PARAM_LENGTH, E_NOT_OK);

    Eep_Global.jobResultType = MEMIF_JOB_PENDING;
    Eep_Global.status = MEMIF_BUSY;
    Eep_Global.jobType = EEP_WRITE;

    if (Eep_Global.mode == MEMIF_MODE_FAST) {
        job->chunkSize = Eep_Global.config->EepFastWriteBlockSize;
    } else {
        job->chunkSize = Eep_Global.config->EepNormalWriteBlockSize;
    }

    job->initialOp = TRUE;
    job->currSeq = CFG_SPI_P()->EepWriteSequence;
    job->pageSize = Eep_Global.config->EepPageSize;
    job->eepAddr = EepromAddress  + Eep_Global.config->EepBaseAddress;
    job->targetAddr = (uint8 *)DataBufferPtr; /*lint !e926 !e9005 Used for both read and write in the implementation */
    job->left = Length;

    JOB_SET_STATE(JOB_MAIN, EEP_WRITE);

    return E_OK;
}

Std_ReturnType Eep_Compare(Eep_AddressType EepromAddress, const uint8 *DataBufferPtr, Eep_LengthType Length) {
    Eep_JobInfoType *job = &Eep_Global.job;

    VALIDATE_W_RV( ( Eep_Global.status != MEMIF_UNINIT ), EEP_COMPARE_ID, EEP_E_UNINIT, E_NOT_OK);
    VALIDATE_W_RV( ( Eep_Global.status != MEMIF_BUSY ), EEP_COMPARE_ID, EEP_E_BUSY, E_NOT_OK);
    /** @req SWS_Eep_00016 */
    VALIDATE_W_RV( ( DataBufferPtr != NULL ), EEP_COMPARE_ID, EEP_E_PARAM_DATA, E_NOT_OK);
    /** @req SWS_Eep_00017 */
    VALIDATE_W_RV( ( (EepromAddress) < (Eep_Global.config->EepSize) ), EEP_COMPARE_ID, EEP_E_PARAM_ADDRESS, E_NOT_OK);
    /** @req SWS_Eep_00018 */
    VALIDATE_W_RV( ( (Length >= 1u) && ((Eep_Global.config->EepSize - EepromAddress) >= Length) ), EEP_COMPARE_ID, EEP_E_PARAM_LENGTH, E_NOT_OK);

    Eep_Global.status = MEMIF_BUSY;
    Eep_Global.jobResultType = MEMIF_JOB_PENDING;
    Eep_Global.jobType = EEP_COMPARE;

    /* This is a compare job but the compare jobs really issues read in portions
     * big enough to fit it's static buffers
     */
    if (Eep_Global.mode == MEMIF_MODE_FAST) {
        job->chunkSize = Eep_Global.config->EepFastReadBlockSize;
    } else {
        job->chunkSize = Eep_Global.config->EepNormalReadBlockSize;
    }

    job->initialOp = TRUE;
    job->currSeq = CFG_SPI_P()->EepReadSequence;
    job->pageSize = Eep_Global.config->EepPageSize; // Not relevant to compare/read operations, but set anyways.
    job->eepAddr = EepromAddress  + Eep_Global.config->EepBaseAddress;
    job->targetAddr = (uint8 *)DataBufferPtr; /*lint !e926 !e9005 Used for both read and write in the implementation */
    job->left = Length;

    JOB_SET_STATE(JOB_MAIN, EEP_COMPARE);

    return E_OK;
}

void Eep_Cancel(void) {
    EEP_JOB_ERROR_NOTIFICATION();

    if (MEMIF_JOB_PENDING == Eep_Global.jobResultType) {
        Eep_Global.jobResultType = MEMIF_JOB_CANCELED;
    }

    Eep_Global.status = MEMIF_IDLE;
}

MemIf_StatusType Eep_GetStatus(void) {
    return Eep_Global.status;
}

MemIf_JobResultType Eep_GetJobResult(void) {
    return Eep_Global.jobResultType;
}

/**
 * Function that process read/write/erase requests to the SPI
 *
 * @param job The present job
 */

static Spi_SeqResultType processJob(Eep_JobInfoType *job) {

    // Null-pointer guard
    if (job == NULL)
    {
        DET_REPORTERROR(EEP_MODULE_ID, 0, EEP_GLOBAL_SERVICE_ID, EEP_E_PARAM_POINTER);
        return SPI_SEQ_FAILED;
    }

    Spi_SeqResultType rv;
    boolean done = 0;
    uint32 chunkSize = 0;
    uint32 sizeLeftInPage = 0;

    rv = Spi_GetSequenceResult(job->currSeq);

    if( job->initialOp ) {
        ASSERT( rv != SPI_SEQ_PENDING );
        ASSERT( job->state == JOB_MAIN );
        job->initialOp = FALSE;
    } else {
        if( rv != SPI_SEQ_OK ) {
            return rv;
        }
    }
    rv = SPI_SEQ_PENDING;

    do {
        switch (job->state) {
        case JOB_READ_STATUS:
            DEBUG(DEBUG_LOW,"%s: READ_STATUS\n",MODULE_NAME);
            /* Check status from erase cmd, read status from flash */
            (void)Spi_SetupEB(CFG_SPI_P()->EepDataChannel, NULL, &Eep_Global.ebReadStatus, 1);
            Eep_Global.ebCmd = E2_RDSR;
            if (SPI_TRANSMIT_FUNC(CFG_SPI_P()->EepCmd2Sequence,job ) == E_OK) {
                PROC_SET_STATE(DONE, JOB_READ_STATUS_RESULT);
            } else {
                PROC_SET_STATE(DONE, JOB_READ_STATUS);
            }
            break;

        case JOB_READ_STATUS_RESULT:
            DEBUG(DEBUG_LOW,"%s: READ_STATUS_RESULT\n",MODULE_NAME);
            /* Check WIP (Write in Progress) bit */
            if ((Eep_Global.ebReadStatus & 1UL) > 0) {
                /* Still not done */
                PROC_SET_STATE(NOT_DONE, JOB_READ_STATUS);
            } else {
                PROC_SET_STATE(NOT_DONE, JOB_MAIN);
            }
            break;

        case JOB_MAIN:
            if (job->left > 0) {
                if (job->left <= job->chunkSize) {
                    chunkSize = job->left;
                } else {
                    chunkSize = job->chunkSize;
                }

                convertToSpiAddr((Spi_DataBufferType *)&Eep_Global.ebE2Addr, job->eepAddr);

                boolean spiTransmitOK = FALSE;

                switch (job->mainState) {
                case EEP_ERASE:
                    /* NOT USED */
                    break;
                case EEP_READ:
                case EEP_COMPARE:
                    DEBUG(DEBUG_LOW,"%s: READ s:%04x d:%04x l:%04x\n",MODULE_NAME,job->eepAddr, job->targetAddr, job->left);
                    Eep_Global.ebCmd = E2_READ;
                    (void)Spi_SetupEB(CFG_SPI_P()->EepDataChannel, NULL, job->targetAddr, (Spi_NumberOfDataType)chunkSize);
                    if (SPI_TRANSMIT_FUNC(CFG_SPI_P()->EepReadSequence,job) == E_OK) {
                        spiTransmitOK = TRUE;
                    }
                    break;

                case EEP_WRITE:
                    DEBUG(DEBUG_LOW,"%s: WRITE d:%04x s:%04x first data:%02x\n",MODULE_NAME,job->eepAddr,job->targetAddr,*job->targetAddr);

                    // Calculate how much space there is left in the current EEPROM page.
                    sizeLeftInPage = job->pageSize - (job->eepAddr % job->pageSize);

                    // Handle EEPROM page boundaries, i.e. make sure that we limit the chunk
                    // size so we don't write over the page boundary.
                    if (chunkSize > sizeLeftInPage) {
                        chunkSize = sizeLeftInPage;
                    } else {
                        // Do nothing since the size of the chunk to write is less than the
                        // available space left in the page.
                    }

                    Eep_Global.ebCmd = E2_WRITE;
                    convertToSpiAddr((Spi_DataBufferType *)&Eep_Global.ebE2Addr, job->eepAddr);
                    (void)Spi_SetupEB(CFG_SPI_P()->EepDataChannel, job->targetAddr, NULL, (Spi_NumberOfDataType)chunkSize);
                    if (SPI_TRANSMIT_FUNC(CFG_SPI_P()->EepWriteSequence,job ) == E_OK) {
                        spiTransmitOK = TRUE;
                    }
                    break;

                default:
                    ASSERT(0);
                    break;
                }

                if (spiTransmitOK) {
                    job->eepAddr += chunkSize;
                    job->targetAddr = &job->targetAddr[chunkSize];
                    job->left -= chunkSize;

                    /* We have sent the data, now check for the WIP (Write In Progress)
                     * bit to become 0
                     */
                    PROC_SET_STATE(DONE, JOB_READ_STATUS);
                } else {
                    PROC_SET_STATE(DONE, JOB_MAIN);
                }

            } else {
                /* We are done :) */
                PROC_SET_STATE(DONE, JOB_MAIN);
                job->mainState = EEP_NONE;
                rv = SPI_SEQ_OK;
            }
            break;

        default:
            ASSERT(0);
            break;

        }
    } while (!done);
    return rv;
}

#define CMP_BUFF_SIZE SPI_EB_MAX_LENGTH

void Eep_MainFunction(void) {
    Spi_SeqResultType jobResult;

    if (Eep_Global.jobResultType == MEMIF_JOB_PENDING) {
        switch (Eep_Global.jobType) {
        case EEP_COMPARE: {
            static Eep_JobInfoType readJob;
            static uint8 Eep_CompareBuffer[SPI_EB_MAX_LENGTH];
            Eep_JobInfoType *gJob = &Eep_Global.job;
            static boolean firstTime = TRUE;
            static uint32 readSize;

            /* Compare jobs must use a local buffer to hold one portion
             * of the job. Since processJob() also manipulates the
             * job structure we need to create a new local job each time.
             * The global job updates is updated for each process job.
             */

            if (firstTime) {
                readJob = *gJob;

                if (gJob->left <= CMP_BUFF_SIZE) {
                    readSize = gJob->left;
                } else {
                    readSize = CMP_BUFF_SIZE;
                }
                readJob.left = readSize;
                readJob.targetAddr = Eep_CompareBuffer;
                firstTime = FALSE;
            }

            jobResult = processJob(&readJob);

            if (jobResult == SPI_SEQ_PENDING) {
                /* Do nothing */
            } else if (jobResult == SPI_SEQ_OK) {

                if (memcmp(Eep_CompareBuffer, gJob->targetAddr, (size_t)readSize) != 0) { /*lint !e746 String.h is included, lint bug in combination with ghs. */
                    DET_REPORTERROR(EEP_MODULE_ID, 0, 0x9, (uint8)MEMIF_JOB_FAILED);
                    EEP_JOB_ERROR_NOTIFICATION();
                    return;
                }
                // Update the global compare job
                gJob->targetAddr = &gJob->targetAddr[readSize];
                gJob->eepAddr += readSize;
                gJob->left -= readSize;

                // Check if we are done
                if (gJob->left == 0) {
                    Eep_Global.jobResultType = MEMIF_JOB_OK;
                    Eep_Global.jobType = EEP_NONE;
                    Eep_Global.status = MEMIF_IDLE;
                    EEP_JOB_END_NOTIFICATION();
                    firstTime = TRUE;
                    return;
                }
                // Calculate new readSize
                if (gJob->left <= CMP_BUFF_SIZE) {
                    readSize = gJob->left;
                } else {
                    readSize = CMP_BUFF_SIZE;
                }

                // Update the readjob for next session
                readJob = *gJob;
                readJob.left = readSize;
                readJob.targetAddr = Eep_CompareBuffer;
            } else {
                // all other cases are bad
                firstTime = TRUE;
                Eep_Global.jobResultType = MEMIF_JOB_FAILED;
                Eep_Global.jobType = EEP_NONE;
                Eep_Global.status = MEMIF_IDLE;

                DET_REPORTERROR(EEP_MODULE_ID, 0, EEP_COMPARE_ID, (uint8)MEMIF_JOB_FAILED);
                EEP_JOB_ERROR_NOTIFICATION();
            }
        }
            break;

        case EEP_ERASE:
        case EEP_READ:
        case EEP_WRITE:

            jobResult = processJob(&Eep_Global.job);

            if (jobResult == SPI_SEQ_OK) {
                Eep_Global.jobResultType = MEMIF_JOB_OK;
                Eep_Global.jobType = EEP_NONE;
                Eep_Global.status = MEMIF_IDLE;
                EEP_JOB_END_NOTIFICATION();
            } else if (jobResult == SPI_SEQ_PENDING) {
                /* Busy, Do nothing */
            } else {
                // Error

                Eep_Arc_JobType failedJobType = Eep_Global.jobType;

                Eep_Global.jobResultType = MEMIF_JOB_FAILED;
                Eep_Global.jobType = EEP_NONE;
                Eep_Global.status = MEMIF_IDLE;

                switch (failedJobType) {
                case EEP_ERASE:
                    DET_REPORTERROR(EEP_MODULE_ID, 0, EEP_ERASE_ID, (uint8)MEMIF_JOB_FAILED);
                    break;
                case EEP_READ:
                    DET_REPORTERROR(EEP_MODULE_ID, 0, EEP_READ_ID, (uint8)MEMIF_JOB_FAILED);
                    break;
                case EEP_WRITE:
                    DET_REPORTERROR(EEP_MODULE_ID, 0, EEP_WRITE_ID, (uint8)MEMIF_JOB_FAILED);
                    break;
                default:
                    ASSERT(0);
                }

                EEP_JOB_ERROR_NOTIFICATION();
            }
            break;
        case EEP_NONE:
            ASSERT(0);
            break;
        default:
            break; /* Misra compliance */
        }
    }
}

//lint -restore
