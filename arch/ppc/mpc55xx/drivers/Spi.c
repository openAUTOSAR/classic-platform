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









/* CONFIGURATION NOTES
 * ------------------------------------------------------------------
 * The configuration for this module should be supplied by
 * the users of it ( see 10.4 Configuration concept )
 *
 * ALT configuration method:
 *   The method that is used today keeps the configuration in the spi_cfg files.
 *   That is not good..
 *
 *   Could do something like:
 *     SPI_ADD_CONFIG(Eep_SpiConfig)
 *   in each place a configuration needs to be added to Spi.
 *   The macro adds the pointer to a ptr list( .spi_config section )
 *
 *   Problems: Sequence/job ID's must be global
 *   --> Hhmm skip this for now, gets VERY complicated
 *
 */

/* IMPLEMENTATION NOTES
 * -----------------------------------------------
 * - This driver implements SPI_LEVEL_DELIVERED = 2 but with a number
 *   of restrictions. See REQ. below for more information.
 *
 * - A sequence may use different CS's if the controller is the same.
 *   ( All jobs that belongs to a sequence MUST share the same controller )
 *
 * - The driver uses Spi_DataType as uint8. This means that an 16-bit address have
 *   a length of 2 when setting upASetting async/sync mode is not supported
 *   ( Spi_SetAsyncMode(), SPI188, etc )
 * - Cancel API is NOT supported
 *    ( SPI_CANCEL_API must be STD_OFF )
 *
 * - It's not obvious how to use different modes. My interpretation:
 *
 *   Sync
 *     Always blocking in Spi_SyncTranmit()
 *
 *   Async and INTERRUPT
 *
 *   Async and POLLING
 *     Not supported since
 *
 * - Some sequence charts
 *
 *
 * == Sync ==
 *       WriteSeq   ISR   WriteJob   MainFunction_Driving
 *       -------------------------------------------------
 *       |          |
 * ------>
 *       ---------------->
 *       <---------------
 * ( for each job we will now get an interrupt that write's the next job)
 *                  ----->
 *                  <-----
 *                  ...
 * <-----
 *
 * == Async and INTERRUPT ==
 *
 * ------>
 *       ---------------->
 *       <---------------
 * <-----
 * ( for each job we will now get an interrupt that write's the next job)
 *                  ----->
 *                  <-----
 *       ....
 *
 *
 * == Async and POLLING ==
 * ( Not supported yet )
 *
 * ------>
 *       ---------------->
 *       <---------------
 * <-----
 * ( for each job in the sequence the sequence must be repeated )
 * ---------------------------------->
 *                 <-----------------
 *                 ------>
 *                 <-----
 *                 ------------------>
 * <----------------------------------
 * ...
 *
 *
 */

/* HW INFO
 * -----------------------------------------------
 * 4 DSPI modules, A,B,C and D
 * 7 CTAR's for each module.( data-width, baudrate )
 *
 */

/* NOTIFICATION INFORMATION
 * -----------------------------------------------
 *
 * There's a LOT of status and notification in this module....
 *
 *                           Job1              Job2
 *                     |---------------|---------------|
 *                                    JN              JN,SN
 * Status    IDLE             BUSY           BUSY           IDLE
 * HwStatus  IDLE             BUSY           BUSY           IDLE
 * JobResult JOB_OK      JOB_PENDING       JOB_PENDING      JOB_OK,SPI_JOB_FAILED
 * SeqResult SEQ_OK      SEQ_PENDING       SEQ_PENDING      SEQ_OK,SEQ_FAILED,SEQ_CANCELLED
 *
 */


#include "Spi.h"
#include "mpc55xx.h"
//#include <stdio.h>
#include "Mcu.h"
#include "math.h"
#include "Dma.h"
#include <assert.h>
#include <limits.h>
#include "Det.h"
#include <stdlib.h>

//#define USE_LDEBUG_PRINTF	1
#undef DEBUG_LVL
#define DEBUG_LVL DEBUG_HIGH
#include "debug.h"

#define MODULE_NAME 	"/driver/Spi"

#define ARRAY_SIZE(_x) (sizeof(_x) / sizeof((_x)[0]))

#define GET_SPI_HW_PTR(_unit) 	\
        ((struct DSPI_tag *)(0xFFF90000 + 0x4000*(_unit)))

#define GET_SPI_UNIT_PTR(_unit) &Spi_Unit[_unit]

#define ENABLE_EOQ_INTERRUPT(_spi_hw) _spi_hw->RSER.B.EOQFRE = 1
#define DISABLE_EOQ_INTERRUPT(_spi_hw) _spi_hw->RSER.B.EOQFRE = 0

/* Development error macros. */
#if ( SPI_DEV_ERROR_DETECT == STD_ON )
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_SPI,0,_api,_err); \
          return; \
        }

#define VALIDATE_W_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_SPI,0,_api,_err); \
          return (_rv); \
        }
#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_W_RV(_exp,_api,_err,_rv )
#endif

#define NOT_VALID 	(-1)

typedef union
{
  vuint32_t R;
  struct {
  vuint32_t CONT:1;
  vuint32_t CTAS:3;
  vuint32_t EOQ:1;
  vuint32_t CTCNT:1;
    vuint32_t:4;
  vuint32_t PCS5:1;
  vuint32_t PCS4:1;
  vuint32_t PCS3:1;
  vuint32_t PCS2:1;
  vuint32_t PCS1:1;
  vuint32_t PCS0:1;
  vuint32_t TXDATA:16;
  } B;
}SPICommandType;

typedef SPICommandType Spi_CommandType;



/* Templates for Rx/Tx DMA structures */
struct tcd_t Spi_DmaTx =
{

    .SADDR = 0,
    .SMOD = 0,
    .SSIZE = DMA_TRANSFER_SIZE_32BITS,
    .DMOD = 0,
    .DSIZE = DMA_TRANSFER_SIZE_32BITS,
    .SOFF = 4,
    .NBYTES = 4,
    .SLAST = 0,
    .DADDR = 0,
    .CITERE_LINK = 0,
    .CITER = 0,
    .DOFF = 0,
    .DLAST_SGA = 0,
    .BITERE_LINK = 0,
    .BITER = 0,
    .BWC = 0,
    .MAJORLINKCH = 0,
    .DONE = 0,
    .ACTIVE = 0,
    .MAJORE_LINK = 0,
    .E_SG = 0,
    .D_REQ = 0,
    .INT_HALF = 0,
    .INT_MAJ = 0,
    .START = 0
};

struct tcd_t Spi_DmaRx =
{
    .SADDR = 0,
    .SMOD = 0,
    .SSIZE = DMA_TRANSFER_SIZE_32BITS,
    .DMOD = 0,
    .DSIZE = DMA_TRANSFER_SIZE_32BITS,
    .SOFF = 0,
    .NBYTES = 4,
    .SLAST = 0,
    .DADDR = 0,
    .CITERE_LINK = 0,
    .CITER = 1,
    .DOFF = 4,
    .DLAST_SGA = 0,
    .BITERE_LINK = 0,
    .BITER = 1,
    .BWC = 0,
    .MAJORLINKCH = 0,
    .DONE = 0,
    .ACTIVE = 0,
    .MAJORE_LINK = 0,
    .E_SG = 0,
    .D_REQ = 0,
    .INT_HALF = 0,
#if defined(__DMA_INT)
    .INT_MAJ = 1,
#else
    .INT_MAJ = 0,
#endif
    .START = 0
};

#define GET_HW(_channel)    ( struct DSPI_tag *)((uint32)&DSPI_A + 0x4000 * _channel )



typedef struct {
	// Pointer to source buffer
	const Spi_DataType *src;
	// Pointer to destination buffer
	Spi_DataType *dest;
	// Number of elements of Spi_DataType in destination buffer
	Spi_NumberOfDataType length;
	// Set if the buffer is configured.
	// Used for sanity check
	_Bool active;
} Spi_EbType;

typedef enum {
	SPI_ASYNC,
	SPI_SYNC,
} Spi_CallTypeType;


Spi_EbType Spi_Eb[SPI_MAX_CHANNEL];

typedef struct {
	// this channel is assigned to this CTAR
	uint8 ctarId;
} Spi_ChannelInfoType;

Spi_ChannelInfoType Spi_ChannelInfo[SPI_MAX_CHANNEL];

// E2 read = cmd + addr + data = 1 + 2 + 64 ) = 67 ~ 72
#define SPI_INTERNAL_MTU    72

/**
 * This structure represents a controller unit
 */
typedef struct {

	// Tx DMA channel information
	Dma_ChannelType dmaTxChannel;
	struct tcd_t dmaTxTCD;

	// Rx DMA channel information
	Dma_ChannelType dmaRxChannel;
	struct tcd_t dmaRxTCD;

	// Pointed to by SADDR of DMA
	Spi_CommandType txQueue[SPI_INTERNAL_MTU];
	// Pointed to by DADDR of DMA
	uint32          rxQueue[SPI_INTERNAL_MTU];

	// current index for data when sending
	// mostly here for debug purposes( since it could be local )
	uint32 txCurrIndex;

	// Helper array to assign CTAR's
	uint32     channelCodes[7];

	// Status for this unit
	Spi_StatusType status;

	// The current job
	const Spi_JobConfigType *currJob;
	// Points array of jobs current
	const uint32 *currJobIndexPtr;
	// The Sequence
	const Spi_SequenceConfigType *currSeqPtr;

	// Used by sync call to check when a job is done
	//    volatile _Bool done;

	// 1 -  if the current job is sync. 0 - if not
	Spi_CallTypeType callType;
} Spi_UnitType;

typedef struct {
	Spi_SeqResultType seqResult;
} Spi_SeqUnitType;

typedef struct {
	Spi_JobResultType jobResult;
} Spi_JobUnitType;

typedef struct {
  // Initially FALSE set to TRUE if Spi_Init() have been called
  boolean initRun;

  // Pointer to the configuration
  const Spi_ConfigType *configPtr;

  // Pointer to the external buffers
  Spi_EbType *extBufPtr;

  Spi_ChannelInfoType *channelInfo;

  // Mask if the HW unit is configured or not
  uint32 spiHwConfigured;

  Spi_AsyncModeType asyncMode;

  /* This is a bunch of debug counters. */
  uint32 totalNbrOfTranfers;
  uint32 totalNbrOfStartedJobs;
  /* Counters for busy waiting for DSPI and DMA. */
  uint32 totalNbrOfWaitTXRXS;
  uint32 totalNbrOfWaitRxDMA;

} Spi_GlobalType;


//
// Instances
//
Spi_GlobalType Spi_Global = {
    .initRun = FALSE,
    .asyncMode = SPI_INTERRUPT_MODE, // TODO: according to SPI151 it should be polling
};

Spi_UnitType Spi_Unit[4];
Spi_SeqUnitType Spi_SeqUnit[SPI_MAX_SEQUENCE];
Spi_JobUnitType Spi_JobUnit[SPI_MAX_JOB];

static void Spi_Isr( uint32 );

static void Spi_Isr_A( void ) { Spi_Isr(DSPI_CTRL_A); }
static void Spi_Isr_B( void ) { Spi_Isr(DSPI_CTRL_B); }
static void Spi_Isr_C( void ) { Spi_Isr(DSPI_CTRL_C); }
static void Spi_Isr_D( void ) { Spi_Isr(DSPI_CTRL_D); }

typedef struct Spi_IsrInfo {
	void (*entry)(void);
	IrqType vector;
	uint8_t priority;
	Cpu_t cpu;
} Spi_IsrInfoType;


Spi_IsrInfoType Spi_Isr_Info[] = {
{
		.entry = Spi_Isr_A,
		.vector = DSPI_A_ISR_EOQF,
		.priority = 1,
		.cpu = CPU_Z1,
},
{
		.entry = Spi_Isr_B,
		.vector = DSPI_B_ISR_EOQF,
		.priority = 1,
		.cpu = CPU_Z1,
},
{
		.entry = Spi_Isr_C,
		.vector = DSPI_C_ISR_EOQF,
		.priority = 1,
		.cpu = CPU_Z1,
},
{
		.entry = Spi_Isr_D,
		.vector = DSPI_D_ISR_EOQF,
		.priority = 1,
		.cpu = CPU_Z1,
},
};

#if 0
static void Spi_Isr_DMA( void )
{
	// Clear interrupt
	Dma_ClearInterrupt(5);
}
#endif


static void Spi_JobWrite( Spi_JobType jobIndex );



static void Spi_SetJobResult( Spi_JobType Job, Spi_JobResultType result  )
{
  Spi_JobUnit[Job].jobResult = result;
}

static void Spi_SetHWUnitStatus(Spi_HWUnitType HWUnit,Spi_StatusType status )
{
  Spi_Unit[HWUnit].status = status;
}

/**
 * Get external Ptr to device from index
 *
 * @param deviceType The device index.
 * @return Ptr to the external device
 */

static inline const Spi_ExternalDeviceType *Spi_GetExternalDevicePtrFromIndex( Spi_ExternalDeviceTypeType deviceType ) {
  return (&(Spi_Global.configPtr->SpiExternalDevice[(deviceType)]));
}

/**
 * Get configuration job ptr from job index
 * @param jobIndex the job
 * @return Ptr to the job configuration
 */
static const Spi_JobConfigType *Spi_GetJobPtrFromIndex( Spi_JobType jobIndex ) {
  return &Spi_Global.configPtr->SpiJobConfig[jobIndex];
}

/**
 * Get sequence ptr from sequence index
 * @param seqIndex the sequence
 * @return Ptr to the sequence configuration
 */
static const Spi_SequenceConfigType *Spi_GetSeqPtrFromIndex( Spi_SequenceType SeqIndex ) {
  return &Spi_Global.configPtr->SpiSequenceConfig[SeqIndex];
}

/**
 * Get unit ptr from unit index
 * @param unit the unit
 * @return Ptr to the SPI unit
 */
static Spi_UnitType *Spi_GetUnitPtrFromIndex( uint32 unit ) {
  return &Spi_Unit[unit];
}

/**
 * Function to see if two sequences share jobs
 * @param seq - Seqence 1
 * @param seq - Seqence 2
 * @return 0 - if the don't share any jobs
 *        !=0 - if they share jobs
 */

static boolean Spi_ShareJobs(Spi_SequenceType seq1, Spi_SequenceType seq2 ) {
  uint32 seqMask1 = 0;
  uint32 seqMask2 = 0;
  const uint32 *jobPtr;
  const Spi_SequenceConfigType *seqConfig;

  // Search for jobs in sequence 1
  seqConfig = Spi_GetSeqPtrFromIndex(seq1);
  jobPtr = &seqConfig->JobAssignment[0];

  while( *jobPtr != NOT_VALID ) {
    assert(*jobPtr<31);
    seqMask1 |= (1<<*jobPtr);
    jobPtr++;
  }

  // Search for jobs in sequence 2
  seqConfig = Spi_GetSeqPtrFromIndex(seq2);
  jobPtr = &seqConfig->JobAssignment[0];

  while( *jobPtr != NOT_VALID ) {
    assert(*jobPtr<31);
    seqMask2 |= (1<<*jobPtr);
    jobPtr++;
  }

  return (seqMask1 & seqMask2 );
}

//-------------------------------------------------------------------

/**
 * Sets a result for a sequence
 *
 * @param Sequence The sequence to set the result for
 * @param result The result to set.
 */
static void Spi_SetSequenceResult(Spi_SequenceType Sequence, Spi_SeqResultType result ) {
	Spi_SeqUnit[Sequence].seqResult = result;
}

//-------------------------------------------------------------------


/**
 * Gets the next job to do
 *
 * @param spiUnit The SPI unit
 * @return The job ID. -1 if no more jobs
 */
static uint32 Spi_GetNextJob(Spi_UnitType *spiUnit ) {
	spiUnit->currJobIndexPtr++;
	return *(spiUnit->currJobIndexPtr);
}
//-------------------------------------------------------------------

/**
 * Schedules next job to do( calls Spi_jobWrite() or not )
 *
 * @param spiUnit The SPI unit
 */
static int Spi_WriteNextJob( Spi_UnitType *spiUnit ) {
	uint32 nextJob;
	// Re-cap.
	// - Jobs have the controller
	// - Sequences can we interruptible between jobs.
	// But
	// According to SPI086 you can't share a job with a sequence that
	// is in SPI_SEQ_PENDING ( that happens first thing at Spi_AsyncTranmit() )
	//
	// So, I no clue what to use the priority thing for :(

	nextJob = Spi_GetNextJob(spiUnit);
	if( nextJob == NOT_VALID) {
	  return NOT_VALID;

	} else {
		// Schedule next job
		Spi_JobWrite(nextJob);
	}
	return 0;
}

//-------------------------------------------------------------------

/**
 * Function to handle things after a transmit on the SPI is finished.
 * It copies data from it't local buffers to the buffers pointer to
 * by the external buffers
 *
 * @param spiUnit Ptr to a SPI unit
 */

static int Spi_PostTransmit( Spi_UnitType *spiUnit ) {
	_Bool printedSomeThing = 0;

	/* Stop the channels */
	Dma_StopChannel (spiUnit->dmaTxChannel);
	Dma_StopChannel (spiUnit->dmaRxChannel);

	ramlog_str("PostTransmit Job: ");
	ramlog_dec(spiUnit->currJob->SpiJobId);
	ramlog_str("\n");

	/* Copy data from RX queue to the external buffer( if a<uny ) */
	{
		int j=0;
		int currIndex =0;
		int channelIndex;
		const Spi_ChannelConfigType *chConfig;
		Spi_EbType *extChBuff;
		int gotTx;
		int sentTx;

		// Check that we got the number of bytes we sent
		sentTx = spiUnit->txCurrIndex+1;
		gotTx = (Dma_GetTcd(spiUnit->dmaRxChannel)->DADDR - (uint32)&spiUnit->rxQueue[0])/sizeof(uint32);

		if( sentTx != gotTx ) {
		  // Something failed
		  DEBUG(DEBUG_LOW,"%s: Expected %d bytes. Got %d bytes\n ",MODULE_NAME,sentTx, gotTx );
		  return (-1);
		} else {
		  ramlog_str("Rx ");
		  ramlog_dec(gotTx);
		  ramlog_str(" Bytes\n");
		  DEBUG(DEBUG_LOW,"%s: Got %d bytes\n",MODULE_NAME,gotTx);
		}


		// Find the channels for this job
		while( (channelIndex = spiUnit->currJob->ChannelAssignment[j++]) != NOT_VALID)
		{
			chConfig = &Spi_Global.configPtr->SpiChannelConfig[channelIndex];

			/* Check configuration error */
#if ( SPI_CHANNEL_BUFFERS_ALLOWED == 1  )
			assert( chConfig->SpiChannelType == SPI_EB );
#endif

			// Send the channels that are setup with external buffers
			// Get the external buffer for this channel
			extChBuff = &Spi_Global.extBufPtr[channelIndex];
			if( extChBuff->dest != NULL ) {
				// Note! No support for >8 "data"
				for(int k=0;k<extChBuff->length;k++) {
					extChBuff->dest[k] = spiUnit->rxQueue[currIndex++];
					DEBUG(DEBUG_LOW," %02x ",extChBuff->dest[k]);
					printedSomeThing = 1;
				}

			} else {
				if( chConfig->SpiDataWidth > 8 ) {
					currIndex += (extChBuff->length/2);
				} else {
					currIndex += extChBuff->length;
				}
			}
		}
		if( printedSomeThing )
			DEBUG(DEBUG_LOW,"\n");
	}

	return 0;
}

//-------------------------------------------------------------------

/**
 * ISR for End of Queue interrupt
 *
 * @param unit The HW unit it happend on
 */
static void Spi_Isr( uint32 unit ) {
	struct DSPI_tag *spiHw = GET_SPI_HW_PTR(unit);
	Spi_UnitType *spiUnit = GET_SPI_UNIT_PTR(unit);
	int rv;

	ramlog_str("Spi_Isr\n");

	// This may seem pretty stupid to wait for the controller
	// to shutdown here, but there seems to be no other way to do this.
	// Reasons:
	// - Waiting for DMA rx/tx hits earlier than EOQ.
	// - Other interrupts from SPI all hit earlier than EOQ.

	// TODO: We could implement a timeout here and fail the job
	// if this never happens.

	// This is the busy wait when called from a non-interrupt context
  while (spiHw->SR.B.TXRXS) {
    Spi_Global.totalNbrOfWaitTXRXS++;
  }

  // To be 100% sure also wait for the DMA transfer to complete.
  while(!Dma_ChannelDone (Spi_Global.configPtr->SpiHwConfig[unit].RxDmaChannel))
  {
    Spi_Global.totalNbrOfWaitRxDMA++;
  }

  /* Halt DSPI unit until we are ready for next transfer. */
  spiHw->MCR.B.HALT = 1;
  spiHw->SR.B.EOQF = 1;

  Spi_Global.totalNbrOfTranfers++;

	// Disable EOQ interrupts
  // NOTE!
  //   This does NOT clear the interrupt request.
  //   That can only be done by clearing( setting ) the EOQ
  //   bit.. but that also triggers a new transfer.
  //
  // ALT
  //   A possibility could be to use the HALT bit instead of
  //   using this trick, but hey, this works

  DISABLE_EOQ_INTERRUPT(spiHw);

  // Update external buffers
  rv = Spi_PostTransmit(spiUnit);

	// Call notification end
	if(spiUnit->currJob->SpiJobEndNotification != NULL ) {
		spiUnit->currJob->SpiJobEndNotification();
	}

	if( rv == (-1) )
	{
	  // Fail both job and sequence
	  Spi_SetHWUnitStatus(unit,SPI_IDLE);
	  Spi_SetJobResult(spiUnit->currJob->SpiJobId,SPI_JOB_FAILED);
	  Spi_SetSequenceResult(spiUnit->currSeqPtr->SpiSequenceId,SPI_SEQ_FAILED);
	}
	else
	{

		// The job is at least done..
		Spi_SetJobResult(spiUnit->currJob->SpiJobId,SPI_JOB_OK);

		// WriteNextJob should
		// 1. Update the JobResult to SPI_JOB_OK
		// 2. Update the HWUnit status to IDLE

		if( Spi_WriteNextJob(spiUnit) == (-1))
		{
			// No more jobs, so set HwUnit and sequence IDLE/OK also.
			Spi_SetHWUnitStatus(unit,SPI_IDLE);
			Spi_SetSequenceResult(spiUnit->currSeqPtr->SpiSequenceId, SPI_SEQ_OK);

			if( spiUnit->currSeqPtr->SpiSeqEndNotification != NULL ) {
				spiUnit->currSeqPtr->SpiSeqEndNotification();
			}

			Spi_SetHWUnitStatus(unit,SPI_IDLE);

			/* We are now ready for next transfer. */
		  spiHw->MCR.B.HALT = 1;

			ramlog_str("NO more jobs\n");
		} else {
      ramlog_str("More jobs\n");
		}
	}

	ramlog_str("Spi_Isr END\n");
}

//-------------------------------------------------------------------

Std_ReturnType Spi_WriteIB( Spi_ChannelType Channel, const Spi_DataType *DataBufferPtr ){
  VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_WRITEIB_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );
  VALIDATE_W_RV( ( Channel<SPI_MAX_CHANNEL ), SPI_WRITEIB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );
  VALIDATE_W_RV( ( DataBufferPtr != NULL ), SPI_WRITEIB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );
  VALIDATE_W_RV( ( SPI_IB==Spi_Global.configPtr->SpiChannelConfig[Channel].SpiChannelType ),\
      SPI_WRITEIB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );
  Std_ReturnType rv = E_NOT_OK;
  return rv;
}

/* Clock tables */
uint32 clk_table_asc[] =    {2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536 };
uint32 clk_table_cssck[] =  {2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536 };
uint16 clk_table_br[] =     {2,4,6,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768 };
uint8  clk_table_pasc[] =   { 1,3,5,7 };
uint8  clk_table_pcssck[] = { 1,3,5,7 };
uint8  clk_table_pbr[] =    { 2,3,5,7 };

/**
 * Function to setup CTAR's from configuration
 * @param spiHw - Pointer to HW SPI device
 * @param extDev - Pointer to external device configuration
 * @param ctar_unit - The ctar unit number to setup
 * @param width - The width in bits of the data to send with the CTAR
 */
static void Spi_SetupCTAR( Spi_HWUnitType unit,
                              const Spi_ExternalDeviceType *extDev,
                              Spi_ChannelType ctar_unit,
                              uint8 width )
{
	uint32 clock;
	uint32 pre_br;
	int i;
	int j;
	uint32 tmp;

	struct DSPI_tag *spiHw = GET_SPI_HW_PTR(unit);
	/* BAUDRATE CALCULATION
	 * -----------------------------
	 * Baudrate = Fsys/ PBR * ( 1+ DBR) / BR
	 * PBR range: 2 to 7
	 * DBR range: 0 to 1
	 * BR : 2 to 32768
	 *
	 * To make this easy set DBR = 0 and PBR=2
	 * --> BR=Fsys/(Baudrate.* 2 )
	 *
	 */
	clock = McuE_GetPeripheralClock(Spi_Global.configPtr->SpiHwConfig[unit].PeripheralClock);
	DEBUG(DEBUG_MEDIUM,"%s: Peripheral clock at %d Mhz\n",MODULE_NAME,clock);

	DEBUG(DEBUG_MEDIUM,"%s: Want to run at %d Mhz\n",MODULE_NAME,extDev->SpiBaudrate);

	spiHw->CTAR[ctar_unit].B.DBR = 0;
	spiHw->CTAR[ctar_unit].B.PBR = 0;	// 2
	pre_br = clock/(extDev->SpiBaudrate*clk_table_pbr[spiHw->CTAR[ctar_unit].B.PBR]);

	// find closest lesser
	for(i=0;i<sizeof(clk_table_br)/sizeof(clk_table_br[0]);i++) {
		if( clk_table_br[i] >= pre_br ) {
			break;
		}
	}

	assert(i>=0);
	// Set it
	spiHw->CTAR[ctar_unit].B.BR = i;

	DEBUG(DEBUG_LOW,"%s: CLK %d Mhz\n",MODULE_NAME,
			clock / clk_table_pbr[spiHw->CTAR[ctar_unit].B.PBR] *
			( 1 + spiHw->CTAR[ctar_unit].B.DBR)/clk_table_br[spiHw->CTAR[ctar_unit].B.BR]);


	/* For other timings autosar only specifies SpiTimeClk2Cs == "After SCK delay"
	 * in Freescale language. The dumb thing is that this should be a relative time
	 * to the clock. Not fixed.
	 * Autosar specifies 0.0--100.0 ms(float)
	 * Our intepretation is 0--1000000 ns (uint32)
	 *
	 * AFTER SCK DELAY:
	 * -----------------------------
	 * Tasc = 1/Fsys * PASC  * ASC [s]
	 *
	 * Assume the Tasc get's here in ns( typical range is ~10ns )
	 */

	// Calc the PASC * ASC value...
  tmp = extDev->SpiTimeClk2Cs * (clock / 1000000 );

	// Nothing fancy here...
	{
	  int best_i=0;
	  int best_j=0;
	  int b_value = INT_MAX;
	  int tt;


    // Find the best match of Prescaler and Scaler value
	  for(i=0;i<ARRAY_SIZE(clk_table_pasc);i++) {
	    for(j=0;j<ARRAY_SIZE(clk_table_asc);j++) {
	      tt = abs((int)clk_table_pasc[i]*clk_table_asc[j]*1000 - tmp);
	      if( tt < b_value ) {
	        best_i = i; best_j = j; b_value = tt;
	      }
	    }
	  }

	  /* After SCK delay. */
	   spiHw->CTAR[ctar_unit].B.PASC = best_i;
	   spiHw->CTAR[ctar_unit].B.ASC = best_j;
	}


	DEBUG(DEBUG_MEDIUM,"%s: Timing: Tasc %d ns\n",MODULE_NAME,
			clk_table_pasc[spiHw->CTAR[ctar_unit].B.PASC] *
			clk_table_asc[spiHw->CTAR[ctar_unit].B.ASC] * 1000/ (clock/1000000) );



  /* The PCS to SCK delay is the delay between the assertion of PCS and
   * the first edge the SCK.
   *
   * PCS TO SCK DELAY:
   * -----------------------------
   * Tcsc = 1/Fsys * PCSSCK * CSSCK [s]
   */

  // Calc the PCSSCK * CSSCK value...
  tmp = extDev->SpiTimeCs2Clk * (clock / 1000000 );

  // Nothing fancy here...
  {
    int best_i=0;
    int best_j=0;
    int b_value = INT_MAX;
    int tt;

    // Find the best match of Prescaler and Scaler value
    for(i=0;i<ARRAY_SIZE(clk_table_pcssck);i++) {
      for(j=0;j<ARRAY_SIZE(clk_table_cssck);j++) {
        tt = abs((int)clk_table_pcssck[i]*clk_table_cssck[j]*1000 - tmp);
        if( tt < b_value ) {
          best_i = i; best_j = j; b_value = tt;
        }
      }
    }

    /* PCS to SCK delay */
    spiHw->CTAR[ctar_unit].B.PCSSCK = best_i;
    spiHw->CTAR[ctar_unit].B.CSSCK = best_j;
  }


	DEBUG(DEBUG_MEDIUM,"%s: Timing: Tcsc %d ns\n",MODULE_NAME,
			clk_table_pcssck[spiHw->CTAR[ctar_unit].B.PCSSCK] *
			clk_table_cssck[spiHw->CTAR[ctar_unit].B.CSSCK]*1000/(clock/1000000));


	/* Time that PCS is high between transfers */
	spiHw->CTAR[ctar_unit].B.PDT = 2;
	spiHw->CTAR[ctar_unit].B.DT = 2;

	DEBUG(DEBUG_MEDIUM,"%s: Timing: Tdt  %d ns\n",MODULE_NAME,
			clk_table_pasc[spiHw->CTAR[ctar_unit].B.PDT] *
			clk_table_asc[spiHw->CTAR[ctar_unit].B.DT]*1000/(clock/1000000));

	/* Data is transferred MSB first */
	spiHw->CTAR[ctar_unit].B.LSBFE = 0;

	/* Set mode */
	spiHw->CTAR[ctar_unit].B.FMSZ = width - 1;
	spiHw->CTAR[ctar_unit].B.CPHA = ( extDev->SpiDataShiftEdge == SPI_EDGE_LEADING ) ?  0 : 1;
	spiHw->CTAR[ctar_unit].B.CPOL = ( extDev->SpiCsPolarity == STD_LOW ) ?  0 : 1;

	// This the ACTIVE polarity. Freescale have inactive polarity
	if( extDev->SpiCsPolarity == STD_HIGH ) {
		spiHw->MCR.R &= ~(1 <<(16+extDev->SpiCsIdentifier));
	} else {
		spiHw->MCR.R |= (1 <<(16+extDev->SpiCsIdentifier));
	}
}

//-------------------------------------------------------------------

static void Spi_InitController( uint32 unit ) {
	struct DSPI_tag *spiHw = GET_SPI_HW_PTR(unit);
	Spi_UnitType *spiUnit = GET_SPI_UNIT_PTR(unit);

	/* Module configuration register. */
	/* Master mode. */
	spiHw->MCR.B.MSTR = 1;
	/* No freeze. Run SPI when debugger is stopped. */
	spiHw->MCR.B.FRZ = 0;
	/* PSC5 as regular CS. */
	spiHw->MCR.B.PCSSE = 0;

	/* Enable FIFO's. */
	spiHw->MCR.B.DIS_RXF = 1;
	spiHw->MCR.B.DIS_TXF = 1;

	/* Set all active low. */
	spiHw->MCR.B.PCSIS0 = 1;
	spiHw->MCR.B.PCSIS1 = 1;
	spiHw->MCR.B.PCSIS2 = 1;
	spiHw->MCR.B.PCSIS3 = 1;
	spiHw->MCR.B.PCSIS4 = 1;
	spiHw->MCR.B.PCSIS5 = 1;

	/* DMA TX FIFO fill. */
	spiHw->RSER.B.TFFFRE = 1;
	spiHw->RSER.B.TFFFDIRS = 1;

	/* DMA RX FIFO drain. */
	spiHw->RSER.B.RFDFRE = 1;
	spiHw->RSER.B.RFDFDIRS = 1;

	// Setup CTAR's channel codes..
	for(int i=0;i<7;i++) {
	  spiUnit->channelCodes[i] = NOT_VALID;
	}

	/* Force to stopped state. */
	spiHw->MCR.B.HALT = 1;

	spiHw->SR.B.EOQF = 1;

	/* Enable clocks. */
	spiHw->MCR.B.MDIS = 0;

#if defined(__DMA_INT)
	Irq_InstallVector(Spi_Isr_DMA, 16 , 1, CPU_Z1);
#endif

	// Install EOFQ int..
	Irq_InstallVector(Spi_Isr_Info[unit].entry, Spi_Isr_Info[unit].vector,
	                       Spi_Global.configPtr->SpiHwConfig[unit].IsrPriority, Spi_Isr_Info[unit].cpu);
}

//-------------------------------------------------------------------

static void Spi_DmaSetup(uint32 unit ) {

	struct tcd_t *tcd;

	tcd = &Spi_Unit[unit].dmaTxTCD;
	*tcd = Spi_DmaTx;
	tcd->SADDR = (uint32)Spi_Unit[unit].txQueue;
	tcd->DADDR = (uint32)&(GET_SPI_HW_PTR(unit)->PUSHR.R);

	Dma_StopChannel(Spi_Unit[unit].dmaTxChannel);
	Dma_CheckConfig();

	// CITER and BITER set when we send
	tcd = &Spi_Unit[unit].dmaRxTCD;
	*tcd = Spi_DmaRx;
	tcd->SADDR = (uint32)&(GET_SPI_HW_PTR(unit)->POPR.R);
	tcd->DADDR = (uint32)Spi_Unit[unit].rxQueue;

	Dma_StopChannel(Spi_Unit[unit].dmaRxChannel);
	Dma_CheckConfig();

}

//-------------------------------------------------------------------

void Spi_Init( const Spi_ConfigType *ConfigPtr ) {

	const Spi_JobConfigType *jobConfig2;
	Spi_Global.configPtr = ConfigPtr;
	Spi_Global.extBufPtr = Spi_Eb;
//	Spi_Global.currSeq = NOT_VALID;

	// Set all sequence results to OK
	for(int i=0;i<SPI_MAX_SEQUENCE;i++) {
		Spi_SetSequenceResult(i,SPI_SEQ_OK);
	}

	// Figure out what HW controllers that are used
	for(int j=0;j<Spi_GetJobCnt();j++){
    jobConfig2 = &Spi_Global.configPtr->SpiJobConfig[j];
   	Spi_Global.spiHwConfigured |= (1<<jobConfig2->SpiHwUnit);
	}


	// Initialize controllers used
	{
		uint32 confMask;
		uint8  confNr;

		confMask = Spi_Global.spiHwConfigured;

		for (; confMask; confMask&=~(1<<confNr)) {
			confNr = ilog2(confMask);
			DEBUG(DEBUG_LOW,"%s:Configured HW controller %d\n",MODULE_NAME,confNr);
			Spi_InitController(confNr);
			Spi_SetHWUnitStatus(confNr,SPI_IDLE);

			// DMA init...
			//

			/* Make sure that this channel shall be used. */
      assert (ConfigPtr->SpiHwConfig[confNr].Activated);

      Spi_Unit[confNr].dmaTxChannel = ConfigPtr->SpiHwConfig[confNr].TxDmaChannel;
			Spi_Unit[confNr].dmaRxChannel = ConfigPtr->SpiHwConfig[confNr].RxDmaChannel;
			Spi_DmaSetup(confNr);

		}
	}


	/* Setup CTARS, configuration */
	{
	  Spi_UnitType *spiUnit;

	  int j = 0;
	  int k;
	  int l;
	  uint32 channelCode;
	  int channelIndex;

	  const Spi_JobConfigType *jobConfig;
	  const Spi_ChannelConfigType *chConfig;

	  for(j=0;j<Spi_GetJobCnt();j++){
	    jobConfig = &Spi_Global.configPtr->SpiJobConfig[j];
	    spiUnit = GET_SPI_UNIT_PTR( jobConfig->SpiHwUnit );

	    // Also find the controllers used while we are at it
	    Spi_Global.spiHwConfigured |= (1<<jobConfig->SpiHwUnit);

	    // ..and set the job status
	    Spi_SetJobResult(j,SPI_JOB_OK);

	    l=0;
	    // Go through all the jobs and it's channels to setup CTAS
	    // A job have the same physical controller ( SpiHwUnit )
	    while( (channelIndex = jobConfig->ChannelAssignment[l++]) != NOT_VALID) {
	      chConfig = &Spi_Global.configPtr->SpiChannelConfig[channelIndex];

	      // Form a channel code from
        // <external_device_id><channel width>
	      channelCode = ((jobConfig->DeviceAssignment<<8) +  chConfig->SpiDataWidth);

	      for(k=0;k<7;k++) {
	        if( spiUnit->channelCodes[k] == channelCode ) {
	        	Spi_ChannelInfo[channelIndex].ctarId = k;
	        	DEBUG(DEBUG_LOW,"%s: Channel %d re-uses CTAR %d@%d . device=%d,width=%d\n",MODULE_NAME,channelIndex,k,jobConfig->SpiHwUnit,jobConfig->DeviceAssignment,chConfig->SpiDataWidth);
	          // already in list, break
	          break;
	        }

	        if( spiUnit->channelCodes[k] == NOT_VALID) {
	          // Add to list
	          spiUnit->channelCodes[k] = channelCode;
	          // Assign the CTAR index to channel info..
	          DEBUG(DEBUG_LOW,"%s: Channel %d uses    CTAR %d@%d . device=%d,width=%d\n",MODULE_NAME,channelIndex,k,jobConfig->SpiHwUnit,jobConfig->DeviceAssignment,chConfig->SpiDataWidth);

	          Spi_SetupCTAR(  jobConfig->SpiHwUnit,
														Spi_GetExternalDevicePtrFromIndex( jobConfig->DeviceAssignment ),
	                          k,
	                          chConfig->SpiDataWidth );

	          Spi_ChannelInfo[channelIndex].ctarId = k;
	          break;
	        }
	      }
	    }
	  }
	}

	Spi_Global.initRun = TRUE;
}

//-------------------------------------------------------------------

Std_ReturnType Spi_DeInit( void ){
  struct DSPI_tag *spiHw;
  uint32 confMask;
  uint8  confNr;

  VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_DEINIT_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );
  if( Spi_GetStatus()==SPI_BUSY )
    return E_NOT_OK;

  // Disable the HW modules ( SPI021 )
  confMask = Spi_Global.spiHwConfigured;

  // Disable the SPI hw
  for (; confMask; confMask&=~(1<<confNr)) {
    confNr = ilog2(confMask);
    spiHw = GET_SPI_HW_PTR(confNr);
    // Disable the hardware..
    spiHw->MCR.B.MDIS = 1;

    Spi_InitController(confNr);
    Spi_SetHWUnitStatus(confNr,SPI_IDLE);
  }

  // SPI022
  Spi_Global.configPtr = NULL;
  Spi_Global.initRun = FALSE;


	return E_OK;
}

//-------------------------------------------------------------------

/**
 * Write a job to the SPI bus
 *
 * @param jobIndex The job to write to the SPI bus
 */
static void Spi_JobWrite( Spi_JobType jobIndex ) {

	const Spi_JobConfigType *jobConfig;
	const Spi_ChannelConfigType *chConfig;
	Spi_EbType *extChBuff;
	Spi_CommandType cmd;
	Spi_UnitType *spiUnit;

	int k = 0;
	int j = 0;
	int channelIndex;
	struct DSPI_tag *spiHw;

	cmd.R = 0;

	// Grab some things that may be of use..
	jobConfig = &Spi_Global.configPtr->SpiJobConfig[jobIndex];
	spiUnit = GET_SPI_UNIT_PTR( jobConfig->SpiHwUnit );

	// Be really sure that we are done with previous send
	spiHw = GET_SPI_HW_PTR(jobConfig->SpiHwUnit);

	spiUnit->txCurrIndex = 0;
	spiUnit->currJob = jobConfig;

	Spi_SetHWUnitStatus(jobConfig->SpiHwUnit,SPI_BUSY);
	Spi_SetJobResult(jobIndex,SPI_JOB_PENDING);

	j = 0;

	// Find the channels for this job
	while( (channelIndex = jobConfig->ChannelAssignment[j++]) != NOT_VALID)
	{
		chConfig = &Spi_Global.configPtr->SpiChannelConfig[channelIndex];

    /* Check configuration error */
#if ( SPI_CHANNEL_BUFFERS_ALLOWED == 1  )
    assert( chConfig->SpiChannelType == SPI_EB );
#endif

		// Send the channels that are setup with external buffers
		// Get the external buffer for this channel
		extChBuff = &Spi_Global.extBufPtr[channelIndex];

		if( extChBuff->active == 0 ) {
			LDEBUG_PRINTF("Err:External buffer %d@job %d not setup\n",channelIndex,jobIndex);
			assert(0);
		}

		// Start to fill the SPI queue for the DMA:
		for(k=0;k<extChBuff->length;k++) {
      uint32 csId = Spi_Global.configPtr->SpiExternalDevice[jobConfig->DeviceAssignment].SpiCsIdentifier;
      // Build cmd
      cmd.B.CONT = 1; // Channels should keep CS active
																// ( A job must assert CS continuously)
			cmd.R |= (1 <<(16+csId)); // Set PCS
			cmd.B.EOQ = 0;
			cmd.B.CTAS = Spi_ChannelInfo[channelIndex].ctarId;
			if( extChBuff->src != NULL ) {
				if( chConfig->SpiDataWidth > 8 ) {
					cmd.B.TXDATA = (extChBuff->src[k]<<8)+(extChBuff->src[k+1]&0xff);
					k++;
				} else {
					cmd.B.TXDATA = extChBuff->src[k];
				}
			} else {
				cmd.B.TXDATA = chConfig->SpiDefaultData;
			}

			// Just keep on filling the tx queue
			spiUnit->txQueue[spiUnit->txCurrIndex++].R = cmd.R;
		}
	} /*while( channelIndex == */

	// send last part
	cmd.B.CONT = 0;		// CS high
	cmd.B.EOQ = 1;		// last in queue
	spiUnit->txQueue[--spiUnit->txCurrIndex].R = cmd.R;

	// Set the length of the data to send
	spiUnit->dmaTxTCD.CITER = spiUnit->txCurrIndex + 1;
	spiUnit->dmaTxTCD.BITER = spiUnit->txCurrIndex + 1;

	Spi_Global.totalNbrOfStartedJobs++;

	ramlog_str("Job: ");
	ramlog_dec(jobIndex);
	ramlog_str(" Cnt: ");
	ramlog_dec(spiUnit->txCurrIndex+1);
	ramlog_str("\n");

	DEBUG(DEBUG_LOW,"%s:Tx Job:%d cnt:%d first data:%04x\n",MODULE_NAME,jobIndex,spiUnit->txCurrIndex+1,spiUnit->txQueue[0].B.TXDATA);

	{
		uint32 unit = jobConfig->SpiHwUnit;

		Spi_UnitType *spiUnit =	GET_SPI_UNIT_PTR(unit);
		struct DSPI_tag *spiHw = GET_SPI_HW_PTR(unit);


		Dma_ConfigureChannel ((struct tcd_t *)&spiUnit->dmaTxTCD, spiUnit->dmaTxChannel);
		Dma_ConfigureChannel ((struct tcd_t *)&spiUnit->dmaRxTCD, spiUnit->dmaRxChannel );
		/* Flush TX/Rx FIFO.  Ref. man. 23.5.1 step 8 */
		spiHw->MCR.B.CLR_RXF = 1;
		spiHw->MCR.B.CLR_TXF = 1;

		Dma_StartChannel (spiUnit->dmaRxChannel);
		Dma_StartChannel (spiUnit->dmaTxChannel);

		// Step 9. Clear TCNT
		spiHw->TCR.B.TCNT = 0;

		if( ( Spi_Global.asyncMode == SPI_INTERRUPT_MODE ) &&
		  ( spiUnit->callType == SPI_ASYNC ) )
		{
			ENABLE_EOQ_INTERRUPT(spiHw);
		} else {
			DISABLE_EOQ_INTERRUPT(spiHw);
		}

	  /* This will trig a new transfer. Ref. man. 23.5.1 step 11 */
	  spiHw->SR.B.EOQF = 1;
	  spiHw->MCR.B.HALT = 0;

		// Since it's not obvious on how to tell when a SPI sequence
		// is sent, keep things below to what things have been tested.
	#if 0
		/* Wait for transfer to complete. */
		while (!spiHw->SR.B.EOQF) {	arrggg++; }
	  while (spiHw->SR.B.TXRXS)	{	arrggg2++;}
		while( EDMA.TCD[spiUnit->dmaRxChannel].ACTIVE ) {;}

	#endif

	}

}

void Spi_PrintSeqInfo(const Spi_SequenceConfigType *seqConfigPtr ) {
  int i=0;
  uint32 job;
  DEBUG(DEBUG_HIGH,"%s: Seq: %d:",MODULE_NAME,seqConfigPtr->SpiSequenceId);

  while( (job = seqConfigPtr->JobAssignment[i]) != (-1) ) {
    DEBUG(DEBUG_HIGH,"%d ",job);
    i++;
  }
  DEBUG(DEBUG_HIGH,"\n");
}


/**
 * Write a sequence to the SPI bus
 *
 * @param seqIndex The sequence
 * @param sync 1 - make the call sync. 0 - make the call async
 */
static void Spi_SeqWrite( Spi_SequenceType seqIndex, Spi_CallTypeType sync ) {

	const Spi_SequenceConfigType *seqConfig;
	const Spi_JobConfigType *jobConfig;
	Spi_UnitType *spiUnit;
	Spi_JobType jobIndex;


	seqConfig = Spi_GetSeqPtrFromIndex(seqIndex);
	jobIndex = seqConfig->JobAssignment[0];
	jobConfig = Spi_GetJobPtrFromIndex(jobIndex);

	spiUnit = Spi_GetUnitPtrFromIndex(jobConfig->SpiHwUnit);
	// Fill in the required fields for job and sequence..
	spiUnit->currJobIndexPtr = &seqConfig->JobAssignment[0];
	spiUnit->callType = sync;
	spiUnit->currSeqPtr = seqConfig;

	Spi_SetSequenceResult(seqIndex, SPI_SEQ_PENDING);

	// Setup interrupt for end of queue
	if( ( Spi_Global.asyncMode == SPI_INTERRUPT_MODE ) &&
	    ( spiUnit->callType == SPI_ASYNC ) )
	{
	  DEBUG(DEBUG_MEDIUM,"%s: async/interrupt mode\n",MODULE_NAME);
	} else {
	  DEBUG(DEBUG_MEDIUM,"%s: sync/polled mode\n",MODULE_NAME);
	}

#if defined(USE_LDEBUG_PRINTF) && ( DEBUG_LVL <= DEBUG_HIGH )
	Spi_PrintSeqInfo( seqConfig );
#endif

	Spi_JobWrite(jobIndex);

	if( spiUnit->callType == SPI_SYNC ) {
	  while( Spi_GetSequenceResult(seqIndex) == SPI_SEQ_PENDING ) {
	    Spi_Isr(jobConfig->SpiHwUnit);
	  }
	}

}

//-------------------------------------------------------------------
static _Bool Spi_AnyPendingJobs(Spi_SequenceType Sequence ) {

  // Check that we don't share any jobs with another sequence that is SPI_SEQ_PENDING
  for(int i=0;i<SPI_MAX_SEQUENCE;i++) {
    if( i==Sequence ) {
      continue;
    }

    if( Spi_GetSequenceResult(i) == SPI_SEQ_PENDING ) {
      // We have found a pending sequence... check that we don't share any jobs
      // with that sequence, SPI086
      if( Spi_ShareJobs(Sequence,i) ) {
        return 1;
      }
    }
  }

  return 0;
}

//-------------------------------------------------------------------

Std_ReturnType Spi_SyncTransmit( Spi_SequenceType Sequence ) {

  VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_SYNCTRANSMIT_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );
  VALIDATE_W_RV( ( SPI_MAX_SEQUENCE>Sequence ), SPI_SYNCTRANSMIT_SERVICE_ID, SPI_E_PARAM_SEQ, E_NOT_OK );
	Std_ReturnType rv = E_OK;

  if( Spi_GetSequenceResult(Sequence) == SPI_SEQ_PENDING  ) {
		DEBUG(DEBUG_LOW,"%s: Spi_AsyncTransmit() is PENDING\n",MODULE_NAME);
  	return E_NOT_OK;		// SPI157
  }

  assert(Spi_GetSequenceResult(Sequence) == SPI_SEQ_OK);

  if( Spi_AnyPendingJobs(Sequence) ) {
    return E_NOT_OK;
  }

  Spi_SeqWrite(Sequence, SPI_SYNC );

	return rv;
}


//-------------------------------------------------------------------

Std_ReturnType Spi_AsyncTransmit( Spi_SequenceType        Sequence ) {
  VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_ASYNCTRANSMIT_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );
  VALIDATE_W_RV( ( SPI_MAX_SEQUENCE>Sequence ), SPI_ASYNCTRANSMIT_SERVICE_ID, SPI_E_PARAM_SEQ, E_NOT_OK );

  if( Spi_GetSequenceResult(Sequence) == SPI_SEQ_PENDING  ) {
		DEBUG(DEBUG_LOW,"%s: Spi_AsyncTransmit() is PENDING\n",MODULE_NAME);
  	return E_NOT_OK;		// SPI157
  }

  assert(Spi_GetSequenceResult(Sequence) == SPI_SEQ_OK);

  if( Spi_AnyPendingJobs(Sequence) ) {
    return E_NOT_OK;
  }

	DEBUG(DEBUG_LOW,"%s: Starting seq: %d\n",MODULE_NAME,Sequence);

	Spi_SeqWrite(Sequence, SPI_ASYNC );

	return E_OK;
}


//-------------------------------------------------------------------


Std_ReturnType Spi_ReadIB( Spi_ChannelType Channel, Spi_DataType *const DataBufferPtr ) {
  VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_READIB_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );
  VALIDATE_W_RV( ( Channel<SPI_MAX_CHANNEL ), SPI_READIB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );
  VALIDATE_W_RV( ( SPI_IB<Spi_Global.configPtr->SpiChannelConfig[Channel].SpiChannelType ),\
      SPI_READIB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );

  /* NOT SUPPORTED */

	Std_ReturnType rv = E_NOT_OK;
	return rv;
}

//-------------------------------------------------------------------

Std_ReturnType Spi_SetupEB( Spi_ChannelType Channel,
							const Spi_DataType*  SrcDataBufferPtr,
	     					Spi_DataType*	DesDataBufferPtr,
	     					Spi_NumberOfDataType Length )
{
  VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_SETUPEB_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );
  VALIDATE_W_RV( ( Channel<SPI_MAX_CHANNEL ), SPI_SETUPEB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );
#if ( SPI_CHANNEL_BUFFERS_ALLOWED == 1 )
  VALIDATE_W_RV( ( SPI_EB==Spi_Global.configPtr->SpiChannelConfig[Channel].SpiChannelType ),\
      SPI_SETUPEB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );
#endif
// SPI067
  VALIDATE_W_RV( ( Length<=Spi_Global.configPtr->SpiChannelConfig[Channel].SpiEbMaxLength ),\
      SPI_SETUPEB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );

	Spi_EbType *extChBuff= &Spi_Global.extBufPtr[Channel];
	const Spi_ChannelConfigType *chConfig = &Spi_Global.configPtr->SpiChannelConfig[Channel];

	if( chConfig->SpiChannelType == SPI_EB ) {
		extChBuff->src = SrcDataBufferPtr;
		extChBuff->dest = DesDataBufferPtr;
		extChBuff->length = Length;
		extChBuff->active = 1;
	} else {
		/* NOT SUPPORTED */
		assert(0);
		while(1);
	}

	return E_OK;
}

//-------------------------------------------------------------------

Spi_StatusType Spi_GetStatus( void ) {
  VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_GETSTATUS_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );

	if( !Spi_Global.initRun ) {
		return SPI_UNINIT;
	}

	// Check all sequences if they have any job pending
	for(int i=0;i<SPI_MAX_SEQUENCE;i++) {
		if( Spi_GetSequenceResult(i)==SPI_SEQ_PENDING ) {
			return SPI_BUSY;
		}
	}

	return SPI_IDLE;
}

//-------------------------------------------------------------------


Spi_JobResultType Spi_GetJobResult ( Spi_JobType       Job ) {
  VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_GETJOBRESULT_SERVICE_ID, SPI_E_UNINIT, SPI_JOB_FAILED );
  VALIDATE_W_RV( ( SPI_MAX_JOB<Job ), SPI_GETJOBRESULT_SERVICE_ID, SPI_E_PARAM_JOB, SPI_JOB_FAILED );

	return Spi_JobUnit[Job].jobResult;
}

//-------------------------------------------------------------------


Spi_SeqResultType Spi_GetSequenceResult(Spi_SequenceType Sequence ) {
	Spi_SeqResultType rv;

  VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_GETSEQUENCERESULT_SERVICE_ID, SPI_E_UNINIT, SPI_SEQ_FAILED );
  VALIDATE_W_RV( ( SPI_MAX_SEQUENCE>Sequence ), SPI_GETSEQUENCERESULT_SERVICE_ID, SPI_E_PARAM_SEQ, SPI_SEQ_FAILED );

  rv = Spi_SeqUnit[Sequence].seqResult;

	return rv;
}

//-------------------------------------------------------------------

Spi_StatusType Spi_GetHWUnitStatus(Spi_HWUnitType        HWUnit) {
  VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_GETHWUNITSTATUS_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );

	return Spi_Unit[HWUnit].status;
}

//-------------------------------------------------------------------

#if (SPI_CANCEL_API == STD_ON )
void Spi_Cancel( Spi_SequenceType Sequence ) {
  VALIDATE( ( TRUE == Spi_Global.initRun ), SPI_CANCEL_SERVICE_ID, SPI_E_UNINIT );
  VALIDATE( ( SPI_MAX_SEQUENCE<Sequence ), SPI_CANCEL_SERVICE_ID, SPI_E_PARAM_SEQ );

  /* NOT SUPPORTED */
}
#endif

//-------------------------------------------------------------------


#if ( SPI_LEVEL_DELIVERED == 2) // SPI154

Std_ReturnType Spi_SetAsyncMode( Spi_AsyncModeType        Mode ) {
  VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_SETASYNCMODE_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );

  // Note!
  // Not really sure by whom this function is supposed to be called by.
  // The users of SPI(e2,flash,etc) should probably not use this.

  for(int i=0;i<SPI_MAX_SEQUENCE;i++) {
    if( Spi_GetSequenceResult(i) == SPI_SEQ_PENDING ) {
      return E_NOT_OK;
    }
  }

  Spi_Global.asyncMode = Mode;

	return E_OK;
}
#endif

//-------------------------------------------------------------------

void Spi_MainFunction_Handling( void ) {
  /* NOT SUPPORTED */
}

//-------------------------------------------------------------------

void Spi_MainFunction_Driving( void ) {
  struct DSPI_tag *spiHw;
  uint32 confMask;
  uint8  confNr;
  Spi_UnitType *spiUnit;

  // TODO: check that the queue is empty.. if so do the next job.
  if( Spi_Global.asyncMode == SPI_POLLING_MODE ) {
    confMask = Spi_Global.spiHwConfigured;

    for (; confMask; confMask&=~(1<<confNr)) {
      confNr = ilog2(confMask);
      spiHw = GET_SPI_HW_PTR(confNr);

      if( Spi_GetHWUnitStatus(confNr) == SPI_BUSY ) {
        if (spiHw->SR.B.TXRXS) {
          // Still not done..
         } else {
           spiUnit = GET_SPI_UNIT_PTR(confNr);
           Spi_Isr(confNr);
         }
      }
    }
  }
}


