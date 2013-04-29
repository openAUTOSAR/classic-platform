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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=3.1.5 */

/* ----------------------------[information]----------------------------------*/
/*
 * Author: mahi
 *
 * Part of Release:
 *   3.1.5
 *
 * Description:
 *   Implements the SPI driver module
 *
 * Support:
 *   General                     Have Support
 *   -------------------------------------------
 *   SPI_CANCEL_API                 N
 *   SPI_CHANNEL_BUFFERS_ALLOWED    Y/N, Supports only level 1 (0:IB,1:EB,2:IB+EB)
 *   SPI_DEV_ERROR_DETECT           Y
 *   SPI_HW_STATUS_API              N
 *   SPI_INTERRUPTIBLE_SEQ_ALLOWED  N
 *   SPI_LEVEL_DELIVERED            Y/N, supports only level 2 (0:sync,1:sync,2:both)
 *   SPI_VERSION_INFO_API           Y
 *
 *   Extra:
 *   - DMA and FIFO is implementations
 *   - Soft CS by callback
 *
 *   Devices
 *   - MPC5606S
 *   - MPC5604B
 *
 * Implementation Notes:
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
 *     Always blocking in Spi_SyncTransmit()
 *
 *
 * Info:
 *   - The AsyncModeType (SPI_POLLING_MODE,SPI_INTERRUPT_MODE) is used to handle only the
 *     async API, ie Spi_AsyncTransmit().  The synchrone API just calls Spi_SyncTransmit()->Spi_Isr()....
 *
 *
 *   Async and INTERRUPT
 *
 *   Async and POLLING
 *     Not supported since ??
 *
 * - Some sequence charts
 *
 *
 * == Sync ==
 *       WriteSeq   ISR   WriteJob   MainFunction_Driving
 *       -------------------------------------------------
 *       |          |        |                |
 *       ------>
 *       ---------------->
 *       <---------------
 * ( for each job we will now get an interrupt that write's the next job)
 *                  ----->
 *                  <-----
 *                  ...
 *       <-----
 *
 * == Async and INTERRUPT ==
 *
 *       ------>
 *       ---------------->
 *       <---------------
 *       <-----
 * ( for each job we will now get an interrupt that write's the next job)
 *                  ----->
 *                  <-----
 *       ....
 *
 *
 * == Async and POLLING ==
 * ( Not supported yet )
 *
 *       ------>
 *       ---------------->
 *       <---------------
 *       <-----
 * ( for each job in the sequence the sequence must be repeated )
 *       ---------------------------------->
 *                       <-----------------
 *                 ------>
 *                 <-----
 *                       ------------------>
 *       <----------------------------------
 *       ...
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
 * JN - JOb Notification
 * SN - Sequence NOtificaiton
 */

/* ----------------------------[includes]------------------------------------*/

#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include "Spi.h"
#include "mpc55xx.h"
//#include <stdio.h>
#include "Mcu.h"
#include "math.h"
#if (SPI_IMPLEMENTATION==SPI_DMA)
#include "Dma.h"
#endif
#if defined(USE_DET)
#include "Det.h"
#endif
#include "isr.h"
#include "cirq_buffer.h"

/* ----------------------------[private define]------------------------------*/
#define DSPI_CTRL_A	0
#define DSPI_CTRL_B	1
#define DSPI_CTRL_C	2
#define DSPI_CTRL_D	3
#define DSPI_CTRL_E	4
#define DSPI_CTRL_F	5

#if defined(CFG_MPC560X)
#define DSPI_A_ISR_EOQF DSPI_0_ISR_EOQF
#define DSPI_B_ISR_EOQF DSPI_1_ISR_EOQF
#if defined(CFG_MPC560XB) || defined(CFG_MPC5604P)
#define DSPI_C_ISR_EOQF DSPI_2_ISR_EOQF
#endif
#if defined(CFG_MPC5606B)
#define DSPI_D_ISR_EOQF DSPI_3_ISR_EOQF
#define DSPI_E_ISR_EOQF DSPI_4_ISR_EOQF
#define DSPI_F_ISR_EOQF DSPI_5_ISR_EOQF
#endif
#endif

#define SPIE_BAD		  (-1)
#define SPIE_OK				0
#define SPIE_JOB_NOT_DONE   1

#if defined(CFG_MPC560XB) || defined(CFG_MPC5604P)
#define CTAR_CNT    6
#else
#define CTAR_CNT    8
#endif

//#define SPI_IMPLEMENTATION	SPI_FIFO
#define USE_DIO_CS          STD_ON

// E2 read = cmd + addr + data = 1 + 2 + 64 ) = 67 ~ 72
#define SPI_INTERNAL_MTU    72

/* The depth of the HW FIFO */
#define FIFO_DEPTH			4

/* Define for debug purposes, checks that SPI/DMA is ok */
//#define STEP_VALIDATION		1

#define MODULE_NAME 	"/driver/Spi"

//#define USE_LDEBUG_PRINTF	1
#undef DEBUG_LVL
#define DEBUG_LVL DEBUG_NONE
#include "debug.h"

//#define USE_LOCAL_RAMLOG
#if defined(USE_LOCAL_RAMLOG)
#define RAMLOG_STR(_x) ramlog_str(_x)
#define RAMLOG_DEC(_x) ramlog_dec(_x)
#define RAMLOG_HEX(_x) ramlog_hex(_x)
#else
#define RAMLOG_STR(_x)
#define RAMLOG_DEC(_x)
#define RAMLOG_HEX(_x)
#endif

#if defined(USE_LOCAL_RAMLOG)
#define SPI_ASSERT(_exp)	if( !(_exp) ) while(1) {}
#else
#define SPI_ASSERT(_exp)	assert(_exp)
#endif

/* ----------------------------[private macro]-------------------------------*/

#ifndef MIN
#define MIN(_x,_y) (((_x) < (_y)) ? (_x) : (_y))
#endif
#ifndef MAX
#define MAX(_x,_y) (((_x) > (_y)) ? (_x) : (_y))
#endif

#define GET_SPI_HW_PTR(_unit) 	\
        ((volatile struct DSPI_tag *)(0xFFF90000 + 0x4000*(_unit)))

#define GET_SPI_UNIT_PTR(_unit) &Spi_Unit[Spi_CtrlToUnit[_unit]]

#define ENABLE_EOQ_INTERRUPT(_spi_hw) _spi_hw->RSER.B.EOQF_RE = 1
#define DISABLE_EOQ_INTERRUPT(_spi_hw) _spi_hw->RSER.B.EOQF_RE = 0

#define GET_HW(_channel)    ( volatile struct DSPI_tag *)((uint32)&DSPI_A + 0x4000 * _channel )

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

/* ----------------------------[private typedef]-----------------------------*/

#if (SPI_IMPLEMENTATION == SPI_DMA )
typedef struct Spi_DmaConfig {
	 Dma_ChannelType RxDmaChannel;
	 Dma_ChannelType TxDmaChannel;
} Spi_DmaConfigType;

#endif



typedef union {
	vuint32_t R;
	struct {
		vuint32_t CONT :1;
		vuint32_t CTAS :3;
		vuint32_t EOQ :1;
		vuint32_t CTCNT :1;
		vuint32_t :4;
		vuint32_t PCS5 :1;
		vuint32_t PCS4 :1;
		vuint32_t PCS3 :1;
		vuint32_t PCS2 :1;
		vuint32_t PCS1 :1;
		vuint32_t PCS0 :1;
		vuint32_t TXDATA :16;
	} B;
} SPICommandType;

typedef SPICommandType Spi_CommandType;
typedef struct {
	const Spi_DataType *    src;    /* Pointer to source buffer */
	Spi_DataType *          dest;   /* Pointer to destination buffer */
	Spi_NumberOfDataType    length; // Number of elements of Spi_DataType in destination buffer
	_Bool                   active; // Set if the buffer is configured.
} Spi_EbType;

typedef enum {
	SPI_ASYNC_CALL,
	SPI_SYNC_CALL,
} Spi_CallTypeType;

typedef struct {
	uint8 ctarId;    // this channel is assigned to this CTAR
} Spi_ChannelInfoType;

/**
 * This structure represents a controller unit
 */
typedef struct {

#if (SPI_IMPLEMENTATION==SPI_DMA)
	Dma_ChannelType dmaTxChannel;       // Tx DMA channel information
	Dma_TcdType     dmaTxTCD;
	Dma_ChannelType dmaRxChannel;      // Rx DMA channel information
	Dma_TcdType     dmaRxTCD;
    Spi_CommandType txQueue[SPI_INTERNAL_MTU];      // Pointed to by SADDR of DMA
    uint32          rxQueue[SPI_INTERNAL_MTU];      // Pointed to by DADDR of DMA
#endif
	uint32          txCurrIndex;                    // current index for data when sending
	uint32          channelCodes[(CTAR_CNT-1)];                // Helper array to assign CTAR's
	Spi_StatusType status;                          // Status for this unit
	const Spi_JobConfigType *       currJob;         // The current job
	const Spi_JobType *             currJobIndexPtr; // Points array of jobs current
    Spi_JobType                     currJobIndex;
	const Spi_SequenceConfigType *  currSeqPtr;     // The Sequence
	Spi_CallTypeType callType;                      // 1 -  if the current job is sync. 0 - if not
	volatile struct DSPI_tag *      hwPtr;
	uint8							hwUnit;		// 0...
} Spi_UnitType;

typedef struct {
	Spi_SeqResultType seqResult;
} Spi_SeqUnitType;

typedef struct {
	volatile struct DSPI_tag *      hwPtr;            /* The HW device used by this Job */
	const Spi_ExternalDeviceType *  extDeviceCfgPtr;  /* The external device used by this job */
	const Spi_JobConfigType *       jobCfgPtr;
	Spi_UnitType *                  unitPtr;
	const Spi_ChannelType *         channelsPtr;
#if (SPI_IMPLEMENTATION == SPI_FIFO )
	uint32_t    fifoSent;          /* Number of bytes in FIFO (before EOQ is set) */
	uint8_t     currTxChIndex;     /* the currently transmitting channel index for FIFO */
	uint32_t    txChCnt;           /* number of Spi_DataType sent for the current channel */
	uint32_t    rxChCnt;           /* number of Spi_DataType received for the current channel */
	uint32_t    currRxChIndex;     /* the currently receiving channel index for FIFO */
#endif
	Spi_JobResultType jobResult;
} Spi_JobUnitType;

typedef struct {
	boolean initRun;                            // Initially FALSE set to TRUE if Spi_Init() have been called
	const Spi_ConfigType *  configPtr;          // Pointer to the configuration
	Spi_EbType *            extBufPtr;          // Pointer to the external buffers
	Spi_ChannelInfoType *   channelInfo;
	uint32                  spiHwConfigured;    // Mask if the HW unit is configured or not
	Spi_AsyncModeType       asyncMode;

	/* This is a bunch of debug counters. */
	uint32 totalNbrOfTranfers;
	uint32 totalNbrOfStartedJobs;
	/* Counters for busy waiting for DSPI and DMA. */
	uint32 totalNbrOfWaitTXRXS;
	uint32 totalNbrOfWaitRxDMA;
#if  defined(STEP_VALIDATION)
	int eoqf_cnt;
	int txrxs_cnt;
#endif
} Spi_GlobalType;

typedef struct Spi_SeqQueue {
	Spi_SequenceType seq;
	Spi_CallTypeType callType;
} Spi_QueueType;

/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/

#if (SPI_IMPLEMENTATION==SPI_DMA)
/* Templates for Rx/Tx DMA structures */
const Dma_TcdType Spi_DmaTx = {
		.SADDR = 0, .SMOD = 0, .SSIZE = DMA_TRANSFER_SIZE_32BITS, .DMOD = 0,
		.DSIZE = DMA_TRANSFER_SIZE_32BITS, .SOFF = 4, .NBYTESu.R = 4, .SLAST = 0,
		.DADDR = 0, .CITERE_LINK = 0, .CITER = 0, .DOFF = 0, .DLAST_SGA = 0,
		.BITERE_LINK = 0, .BITER = 0, .BWC = 0, .MAJORLINKCH = 0, .DONE = 0,
		.ACTIVE = 0, .MAJORE_LINK = 0, .E_SG = 0, .D_REQ = 0, .INT_HALF = 0,
		.INT_MAJ = 0, .START = 0 };

const Dma_TcdType Spi_DmaRx = { .SADDR = 0, .SMOD = 0,
		.SSIZE = DMA_TRANSFER_SIZE_32BITS, .DMOD = 0,
		.DSIZE = DMA_TRANSFER_SIZE_32BITS, .SOFF = 0, .NBYTESu.R = 4, .SLAST = 0,
		.DADDR = 0, .CITERE_LINK = 0, .CITER = 1, .DOFF = 4, .DLAST_SGA = 0,
		.BITERE_LINK = 0, .BITER = 1, .BWC = 0, .MAJORLINKCH = 0, .DONE = 0,
		.ACTIVE = 0, .MAJORE_LINK = 0, .E_SG = 0, .D_REQ = 0, .INT_HALF = 0,
#if defined(__DMA_INT)
		.INT_MAJ = 1,
#else
		.INT_MAJ = 0,
#endif
		.START = 0 };
#endif

Spi_GlobalType  Spi_Global;
Spi_EbType      Spi_Eb[SPI_MAX_CHANNEL];
Spi_UnitType    Spi_Unit[SPI_CONTROLLER_CNT];
Spi_SeqUnitType Spi_SeqUnit[SPI_MAX_SEQUENCE];
Spi_JobUnitType Spi_JobUnit[SPI_MAX_JOB];
Spi_ChannelInfoType Spi_ChannelInfo[SPI_MAX_CHANNEL];
uint8 Spi_CtrlToUnit[4];

#define SPI_QUEUE_SIZE 		2
Spi_QueueType spiQueueData[SPI_QUEUE_SIZE];
static CirqBufferType Spi_Queue;




#if (SPI_IMPLEMENTATION == SPI_DMA)
/* When using DMA it assumes predefined names */
Spi_DmaConfigType  Spi_DmaConfig[SPI_CONTROLLER_CNT] = {
#if (SPI_USE_HW_UNIT_0 == STD_ON )
	{
	    .RxDmaChannel = DMA_DSPI_A_RESULT_CHANNEL,
	    .TxDmaChannel = DMA_DSPI_A_COMMAND_CHANNEL,
	},
#endif
#if (SPI_USE_HW_UNIT_1 == STD_ON )
	{
	    .RxDmaChannel = DMA_DSPI_B_RESULT_CHANNEL,
	    .TxDmaChannel = DMA_DSPI_B_COMMAND_CHANNEL,
	},
#endif
#if (SPI_USE_HW_UNIT_2 == STD_ON )
	{
	    .RxDmaChannel = DMA_DSPI_C_RESULT_CHANNEL,
	    .TxDmaChannel = DMA_DSPI_C_COMMAND_CHANNEL,
	},
#endif
#if (SPI_USE_HW_UNIT_3 == STD_ON )
	{
	    .RxDmaChannel = DMA_DSPI_D_RESULT_CHANNEL,
	    .TxDmaChannel = DMA_DSPI_D_COMMAND_CHANNEL,
	}
#endif
#if (SPI_USE_HW_UNIT_4 == STD_ON )
	{
	    .RxDmaChannel = DMA_DSPI_E_RESULT_CHANNEL,
	    .TxDmaChannel = DMA_DSPI_E_COMMAND_CHANNEL,
	}
#endif
#if (SPI_USE_HW_UNIT_5 == STD_ON )
	{
	    .RxDmaChannel = DMA_DSPI_F_RESULT_CHANNEL,
	    .TxDmaChannel = DMA_DSPI_F_COMMAND_CHANNEL,
	}
#endif
};
#endif

/* ----------------------------[private functions]---------------------------*/

static void Spi_SeqWrite(Spi_SequenceType seqIndex, Spi_CallTypeType sync);

#if (SPI_IMPLEMENTATION == SPI_FIFO )
static void Spi_WriteJob_FIFO( Spi_JobType jobIndex );

/**
 * Get the buffer for a channel.
 *
 * @param ch
 * @param length
 * @return
 */
static Spi_DataType *spiGetRxBuf(Spi_ChannelType ch, Spi_NumberOfDataType *length ) {
	Spi_DataType *buf;
	if( Spi_Global.configPtr->SpiChannelConfig[ch].SpiChannelType == SPI_EB ) {
		*length = Spi_Global.extBufPtr[ch].length;
		buf = Spi_Global.extBufPtr[ch].dest;
	} else {
		/* No support */
		assert(0);
		buf = NULL;
	}
	return buf;
}

static const Spi_DataType *spiGetTxBuf(Spi_ChannelType ch, Spi_NumberOfDataType *length ) {
	const Spi_DataType *buf;
	if( Spi_Global.configPtr->SpiChannelConfig[ch].SpiChannelType == SPI_EB ) {
		*length = Spi_Global.extBufPtr[ch].length;
		buf = Spi_Global.extBufPtr[ch].src;
	} else {
		/* No support */
		assert(0);
		buf = NULL;
	}
	return buf;
}
#endif

static void Spi_Isr(Spi_UnitType *uPtr );

#if (SPI_USE_HW_UNIT_0 == STD_ON )
static void Spi_Isr_A(void) {
	Spi_Isr(GET_SPI_UNIT_PTR(DSPI_CTRL_A));
}
#endif
#if (SPI_USE_HW_UNIT_1 == STD_ON )
static void Spi_Isr_B(void) {
	Spi_Isr(GET_SPI_UNIT_PTR(DSPI_CTRL_B));
}
#endif
#if (SPI_USE_HW_UNIT_2 == STD_ON )
static void Spi_Isr_C(void) {
	Spi_Isr(GET_SPI_UNIT_PTR(DSPI_CTRL_C));
}
#endif
#if (SPI_USE_HW_UNIT_3 == STD_ON )
static void Spi_Isr_D(void) {
	Spi_Isr(GET_SPI_UNIT_PTR(DSPI_CTRL_D));
}
#endif
#if (SPI_USE_HW_UNIT_4 == STD_ON )
static void Spi_Isr_E(void) {
	Spi_Isr(GET_SPI_UNIT_PTR(DSPI_CTRL_E));
}
#endif
#if (SPI_USE_HW_UNIT_5 == STD_ON )
static void Spi_Isr_F(void) {
	Spi_Isr(GET_SPI_UNIT_PTR(DSPI_CTRL_F));
}
#endif
/* ----------------------------[public functions]----------------------------*/

uint32 Spi_GetJobCnt(void);
uint32 Spi_GetChannelCnt(void);
uint32 Spi_GetExternalDeviceCnt(void);

#if 0
static void Spi_Isr_DMA( void )
{
	// Clear interrupt
	Dma_ClearInterrupt(5);
}
#endif

static void Spi_JobWrite(Spi_JobType jobIndex);

static void Spi_SetJobResult(Spi_JobType Job, Spi_JobResultType result) {
	Spi_JobUnit[Job].jobResult = result;
}

static void Spi_SetHWUnitStatus( Spi_UnitType *uPtr, Spi_StatusType status) {
	uPtr->status = status;
}

/**
 * Get external Ptr to device from index
 *
 * @param deviceType The device index.
 * @return Ptr to the external device
 */

static inline const Spi_ExternalDeviceType *Spi_GetExternalDevicePtrFromIndex(
		Spi_ExternalDeviceTypeType deviceType) {
	return (&(Spi_Global.configPtr->SpiExternalDevice[(deviceType)]));
}

/**
 * Get configuration job ptr from job index
 * @param jobIndex the job
 * @return Ptr to the job configuration
 */
static const Spi_JobConfigType *Spi_GetJobPtrFromIndex(Spi_JobType jobIndex) {
	return &Spi_Global.configPtr->SpiJobConfig[jobIndex];
}

/**
 * Get sequence ptr from sequence index
 * @param seqIndex the sequence
 * @return Ptr to the sequence configuration
 */
static const Spi_SequenceConfigType *Spi_GetSeqPtrFromIndex(
		Spi_SequenceType SeqIndex) {
	return &Spi_Global.configPtr->SpiSequenceConfig[SeqIndex];
}


/**
 * Function to see if two sequences share jobs
 *
 * @param seq - Seqence 1
 * @param seq - Seqence 2
 * @return 0 - if the don't share any jobs
 *        !=0 - if they share jobs
 */

static boolean Spi_ShareJobs(Spi_SequenceType seq1, Spi_SequenceType seq2) {
	uint32 seqMask1 = 0;
	uint32 seqMask2 = 0;
	const Spi_JobType *jobPtr;
	const Spi_SequenceConfigType *seqConfig;

	// Search for jobs in sequence 1
	seqConfig = Spi_GetSeqPtrFromIndex(seq1);
	jobPtr = &seqConfig->JobAssignment[0];

	while (*jobPtr != JOB_NOT_VALID) {
		assert(*jobPtr<31);
		seqMask1 |= (1 << *jobPtr);
		jobPtr++;
	}

	// Search for jobs in sequence 2
	seqConfig = Spi_GetSeqPtrFromIndex(seq2);
	jobPtr = &seqConfig->JobAssignment[0];

	while (*jobPtr != JOB_NOT_VALID) {
		assert(*jobPtr<31);
		seqMask2 |= (1 << *jobPtr);
		jobPtr++;
	}

	return (seqMask1 & seqMask2);
}


//-------------------------------------------------------------------

/**
 * Sets a result for a sequence
 *
 * @param Sequence The sequence to set the result for
 * @param result The result to set.
 */
static void Spi_SetSequenceResult(Spi_SequenceType Sequence,
		Spi_SeqResultType result) {
	Spi_SeqUnit[Sequence].seqResult = result;
}

//-------------------------------------------------------------------


/**
 * Gets the next job to do
 *
 * @param spiUnit The SPI unit
 * @return The job ID. -1 if no more jobs
 */
static Spi_JobType Spi_GetNextJob(Spi_UnitType *spiUnit) {
	spiUnit->currJobIndexPtr++;
	return *(spiUnit->currJobIndexPtr);
}
//-------------------------------------------------------------------


#if (SPI_IMPLEMENTATION==SPI_DMA)

/**
 * Function to handle things after a transmit on the SPI is finished.
 * It copies data from it't local buffers to the buffers pointer to
 * by the external buffers
 *
 * @param spiUnit Ptr to a SPI unit
 */

static int Spi_Rx_DMA(Spi_UnitType *spiUnit) {
	_Bool printedSomeThing = 0;

	/* Stop the channels */
	Dma_StopChannel(spiUnit->dmaTxChannel);
	Dma_StopChannel(spiUnit->dmaRxChannel);

	RAMLOG_STR("PostTransmit Job: "); RAMLOG_DEC(spiUnit->currJob->SpiJobId); RAMLOG_STR("\n");

	/* Copy data from RX queue to the external buffer( if a<uny ) */
	{
		int j = 0;
		int currIndex = 0;
		int channelIndex;
		const Spi_ChannelConfigType *chConfig;
		Spi_EbType *extChBuff;
		int gotTx;
		int sentTx;

		// Check that we got the number of bytes we sent
		sentTx = spiUnit->txCurrIndex + 1;
		gotTx = (Dma_GetTcd(spiUnit->dmaRxChannel)->DADDR
				- (uint32) &spiUnit->rxQueue[0]) / sizeof(uint32);

		if (sentTx != gotTx) {
			// Something failed
			DEBUG(DEBUG_LOW,"%s: Expected %d bytes. Got %d bytes\n ",MODULE_NAME,sentTx, gotTx );
#if defined(STEP_VALIDATION)
	    SPI_ASSERT(0);
#endif
			return SPIE_BAD;
		} else {
			RAMLOG_STR("Rx "); RAMLOG_DEC(gotTx); RAMLOG_STR(" Bytes\n"); DEBUG(DEBUG_LOW,"%s: Got %d bytes\n",MODULE_NAME,gotTx);
		}

		// Find the channels for this job
		while ((channelIndex = spiUnit->currJob->ChannelAssignment[j++])
				!= CH_NOT_VALID) {
			chConfig = &Spi_Global.configPtr->SpiChannelConfig[channelIndex];

			/* Check configuration error */
#if ( SPI_CHANNEL_BUFFERS_ALLOWED == 1  )
			assert( chConfig->SpiChannelType == SPI_EB );
#endif

			// Send the channels that are setup with external buffers
			// Get the external buffer for this channel
			extChBuff = &Spi_Global.extBufPtr[channelIndex];
			if (extChBuff->dest != NULL) {
				// Note! No support for >8 "data"
				for (int k = 0; k < extChBuff->length; k++) {
					extChBuff->dest[k] = spiUnit->rxQueue[currIndex++];
					DEBUG(DEBUG_LOW," %02x ",extChBuff->dest[k]);
					printedSomeThing = 1;
				}

			} else {
				if (chConfig->SpiDataWidth > 8) {
					currIndex += (extChBuff->length / 2);
				} else {
					currIndex += extChBuff->length;
				}
			}
		}
		if (printedSomeThing)
			DEBUG(DEBUG_LOW,"\n");
	}

	return 0;
}

#elif (SPI_IMPLEMENTATION==SPI_FIFO)

static int Spi_Rx_FIFO(Spi_UnitType *spiUnit) {
	Spi_JobUnitType *    jobUnitPtr;
	Spi_DataType *       buf;
    uint32_t             copyCnt;
	uint32_t             popVal;
	Spi_ChannelType      currChannel;
	Spi_NumberOfDataType bufLen;
    int i = 0;
    int rv = SPIE_JOB_NOT_DONE;
    const Spi_ChannelConfigType *   chConfig;
    uint32_t bInChar;


//	RAMLOG_STR("PostTransmit Job: "); RAMLOG_DEC(spiUnit->currJob->SpiJobId); RAMLOG_STR("\n");
    RAMLOG_STR("Spi_Rx_FIFO\n");

	jobUnitPtr = &Spi_JobUnit[spiUnit->currJobIndex];

	if( jobUnitPtr->hwPtr->TCR.B.SPI_TCNT != jobUnitPtr->fifoSent ) {
#if defined(STEP_VALIDATION)
	    SPI_ASSERT(0);
#endif
		return SPIE_BAD;
	}

	/*
	 * Fill receive buffers, either EB or IB
	 * Example with 8-bit CMD, 16-bit ADDR and some 8-bit data.
	 *    CMD |  ADDR   |     DATA
	 *   | 12 | 23 | 34 | 00 | 01 | 02 | 03
	 *      1    2    3    4    5    6           BYTE CNT
	 *      1      2       3    4   (5)  (6)     FIFO
	 * With a FIFO of 4 we can see that the CMD, ADDR and almost the whole
	 * DATA channel is sent.
	 */
    currChannel = jobUnitPtr->channelsPtr[jobUnitPtr->currRxChIndex];
    assert(  currChannel != CH_NOT_VALID );

    if( jobUnitPtr->fifoSent != jobUnitPtr->hwPtr->SR.B.RXCTR ) {
#if defined(STEP_VALIDATION)
        SPI_ASSERT(0);
#endif
        return SPIE_BAD;
    }

	while ( jobUnitPtr->fifoSent != 0 ) {

        chConfig = &Spi_Global.configPtr->SpiChannelConfig[currChannel];

	    RAMLOG_STR("CIR#");RAMLOG_DEC(jobUnitPtr->currRxChIndex);RAMLOG_STR("\n");

	    /* Get this channels destination buffer */
		buf = spiGetRxBuf(currChannel, &bufLen);
		assert(bufLen!=0);  /* We should always get a valid bufLen */

        copyCnt = MIN( (bufLen - jobUnitPtr->rxChCnt) >> ((chConfig->SpiDataWidth-1)/8), jobUnitPtr->fifoSent );

        if( copyCnt == 0  ) {
#if defined(STEP_VALIDATION)
        SPI_ASSERT(0);
#endif
            return SPIE_BAD;
        }

        jobUnitPtr->fifoSent -= copyCnt;

        bInChar = (chConfig->SpiDataWidth > 8 ) ? 2 : 1;

        /* Pop the FIFO */
        if( buf != NULL ) {
            for(i=0;i<copyCnt;i++) {
                popVal = jobUnitPtr->hwPtr->POPR.B.RXDATA;
                RAMLOG_STR("%");RAMLOG_HEX(popVal);
                if( bInChar == 2 ) {
                	buf[jobUnitPtr->rxChCnt] = (Spi_DataType)(popVal>>8);
                	buf[jobUnitPtr->rxChCnt+1] = (Spi_DataType)popVal;
                } else {
                	buf[jobUnitPtr->rxChCnt] = (Spi_DataType)popVal;
                }

                jobUnitPtr->rxChCnt += bInChar;
            }
        } else {
            for(i=0;i<copyCnt;i++) {
                popVal = jobUnitPtr->hwPtr->POPR.B.RXDATA;
                jobUnitPtr->rxChCnt += bInChar;
            }
        }

        if( (bufLen - jobUnitPtr->rxChCnt ) == 0 ) {
            /* advance to the next channel */
            jobUnitPtr->rxChCnt = 0;
            jobUnitPtr->currRxChIndex++;
            currChannel = jobUnitPtr->channelsPtr[jobUnitPtr->currRxChIndex];
            if( currChannel == CH_NOT_VALID ) {
                //assert(  jobUnitPtr->fifoSent == 0);
                jobUnitPtr->fifoSent = 0;
                jobUnitPtr->currRxChIndex = 0;
                rv = SPIE_OK;
                break;
            }

        }
	}

#if defined(STEP_VALIDATION)
	/* Check if we are done with this job */
    if( 0 != jobUnitPtr->hwPtr->SR.B.RXCTR ) {
        SPI_ASSERT(0);
    }
#endif

	return rv;
}

#endif

//-------------------------------------------------------------------

/**
 * ISR for End of Queue interrupt
 *
 * The interrupt handling is quite simple. Since this driver assumes
 * that we are the master the EOQ interrupt is sufficient to check.
 *
 * @param unit The HW unit it happend on
 */
static void Spi_Isr( Spi_UnitType *uPtr) {
	volatile struct DSPI_tag *spiHw = uPtr->hwPtr;
	int rv;

	RAMLOG_STR("Spi_Isr\n");

	// This may seem pretty stupid to wait for the controller
	// to shutdown here, but there seems to be no other way to do this.
	// Reasons:
	// - Waiting for DMA rx/tx hits earlier than EOQ.
	// - Other interrupts from SPI all hit earlier than EOQ.
	// TODO: There is the TCF_RE bit! Use this instead?

	// TODO: We could implement a timeout here and fail the job
	// if this never happens.

	// This is the busy wait when called from a non-interrupt context
	while (spiHw->SR.B.TXRXS) {
		Spi_Global.totalNbrOfWaitTXRXS++;
	}

#if (SPI_IMPLEMENTATION == SPI_DMA)
	while (!Dma_ChannelDone(uPtr->dmaRxChannel)) {
		Spi_Global.totalNbrOfWaitRxDMA++;
	}
#endif

#if defined(STEP_VALIDATION)
	/* Since EOQF clears TXRXS */
	SPI_ASSERT( (spiHw->SR.B.EOQF==1) && (spiHw->SR.B.TXRXS == 0));
#endif

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
#if (SPI_IMPLEMENTATION==SPI_DMA)
	rv = Spi_Rx_DMA(uPtr);

#if (USE_DIO_CS == STD_ON)
    void (*cb)(int) = Spi_JobUnit[uPtr->currJobIndex].extDeviceCfgPtr->SpiCsCallback;
    if( cb != NULL ) {
        cb(0);
    }
#endif
#elif (SPI_IMPLEMENTATION==SPI_FIFO)
	rv = Spi_Rx_FIFO(uPtr);

	if( rv == SPIE_JOB_NOT_DONE ) {
	    /* RX FIFO now empty, but the job is not done -> send more */
	    Spi_WriteJob_FIFO ( uPtr->currJobIndex );
		RAMLOG_STR("Spi_Isr END\n");
		return;
	}
#if (USE_DIO_CS == STD_ON)
	else {
	    void (*cb)(int) = Spi_JobUnit[uPtr->currJobIndex].extDeviceCfgPtr->SpiCsCallback;
	    if( cb != NULL ) {
	        cb(0);
	    }
	}
#endif
#endif

	// Call notification end
	if (uPtr->currJob->SpiJobEndNotification != NULL) {
		uPtr->currJob->SpiJobEndNotification();
	}

	if (rv == SPIE_BAD) {
		// Fail both job and sequence
		Spi_SetHWUnitStatus(uPtr, SPI_IDLE);
		Spi_SetJobResult(uPtr->currJob->SpiJobId, SPI_JOB_FAILED);
		Spi_SetSequenceResult(uPtr->currSeqPtr->SpiSequenceId,
				SPI_SEQ_FAILED);
#if defined(STEP_VALIDATION)
		SPI_ASSERT(0);
#endif
	} else {
		Spi_JobType nextJob;

		// The job is at least done..
		Spi_SetJobResult(uPtr->currJob->SpiJobId, SPI_JOB_OK);

		// WriteNextJob should
		// 1. Update the JobResult to SPI_JOB_OK
		// 2. Update the HWUnit status to IDLE

		// Re-cap.
		// - Jobs have the controller
		// - Sequences can we interruptible between jobs.
		// But
		// According to SPI086 you can't share a job with a sequence that
		// is in SPI_SEQ_PENDING ( that happens first thing at Spi_AsyncTranmit() )
		//
		// So, I no clue what to use the priority thing for :(

		nextJob = Spi_GetNextJob(uPtr);
		if( nextJob != JOB_NOT_VALID ) {
			Spi_JobWrite(nextJob);
			RAMLOG_STR("more_jobs\n");
		} else {
			Spi_QueueType qEntry;

			// No more jobs, so set HwUnit and sequence IDLE/OK also.
			Spi_SetHWUnitStatus(uPtr, SPI_IDLE);
			Spi_SetSequenceResult(uPtr->currSeqPtr->SpiSequenceId,
					SPI_SEQ_OK);

			if (uPtr->currSeqPtr->SpiSeqEndNotification != NULL) {
				uPtr->currSeqPtr->SpiSeqEndNotification();
			}

			rv = CirqBuffPop( &Spi_Queue, &qEntry );
			if( rv == 0 ) {
				Spi_SeqWrite(qEntry.seq, qEntry.callType );
			} else {
				/* We are now ready for next transfer. */
				spiHw->MCR.B.HALT = 1;

				RAMLOG_STR("NO_more_jobs\n");

			}
		}
	}

	RAMLOG_STR("Spi_Isr END\n");
}

//-------------------------------------------------------------------

Std_ReturnType Spi_WriteIB(Spi_ChannelType Channel,
		const Spi_DataType *DataBufferPtr) {
	VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_WRITEIB_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );
	VALIDATE_W_RV( ( Channel<SPI_MAX_CHANNEL ), SPI_WRITEIB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );
	VALIDATE_W_RV( ( DataBufferPtr != NULL ), SPI_WRITEIB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );
	VALIDATE_W_RV( ( SPI_IB==Spi_Global.configPtr->SpiChannelConfig[Channel].SpiChannelType ),
			SPI_WRITEIB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );

	/* According to SPI051 it seems that we only have to a "depth" of 1 */


	Std_ReturnType rv = E_NOT_OK;
	return rv;
}

/* Clock tables */
const uint32 clk_table_asc[] = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048,
		4096, 8192, 16384, 32768, 65536 };
const uint32 clk_table_cssck[] = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048,
		4096, 8192, 16384, 32768, 65536 };
const uint16 clk_table_br[] = { 2, 4, 6, 8, 16, 32, 64, 128, 256, 512, 1024, 2048,
		4096, 8192, 16384, 32768 };
const uint8 clk_table_pasc[] = { 1, 3, 5, 7 };
const uint8 clk_table_pcssck[] = { 1, 3, 5, 7 };
const uint8 clk_table_pbr[] = { 2, 3, 5, 7 };

/**
 * Function to setup CTAR's from configuration
 * @param spiHw - Pointer to HW SPI device
 * @param extDev - Pointer to external device configuration
 * @param ctar_unit - The ctar unit number to setup
 * @param width - The width in bits of the data to send with the CTAR
 */
static void Spi_SetupCTAR(	Spi_HWUnitType unit,
							const Spi_ExternalDeviceType *extDev,
							Spi_ChannelType ctar_unit,
							Spi_TransferStartType transferStart,
							uint8 width)
{
	uint32 clock;
	uint32 pre_br;
	int i;
	int j;
	uint32 tmp;
	McuE_PeriperalClock_t perClock;

	volatile struct DSPI_tag *spiHw = GET_SPI_HW_PTR(unit);
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

	switch(unit) {
#if (SPI_USE_HW_UNIT_0 == STD_ON )
	case 0:
		perClock = PERIPHERAL_CLOCK_DSPI_A;
		break;
#endif
#if (SPI_USE_HW_UNIT_1 == STD_ON )
	case 1:
		perClock = PERIPHERAL_CLOCK_DSPI_B;
		break;
#endif
#if (SPI_USE_HW_UNIT_2 == STD_ON )
	case 2:
		perClock = PERIPHERAL_CLOCK_DSPI_C;
		break;
#endif
#if (SPI_USE_HW_UNIT_3 == STD_ON )
	case 3:
		perClock = PERIPHERAL_CLOCK_DSPI_D;
		break;
#endif
#if (SPI_USE_HW_UNIT_4 == STD_ON )
	case 4:
		perClock = PERIPHERAL_CLOCK_DSPI_E;
		break;
#endif
#if (SPI_USE_HW_UNIT_5 == STD_ON )
	case 5:
		perClock = PERIPHERAL_CLOCK_DSPI_F;
		break;
#endif

	default:
		assert(0);
		break;
	}
	clock = McuE_GetPeripheralClock(perClock);

	DEBUG(DEBUG_MEDIUM,"%s: Peripheral clock at %d Mhz\n",MODULE_NAME,clock);

	DEBUG(DEBUG_MEDIUM,"%s: Want to run at %d Mhz\n",MODULE_NAME,extDev->SpiBaudrate);

	spiHw->CTAR[ctar_unit].B.DBR = 0;
	spiHw->CTAR[ctar_unit].B.PBR = 0; // 2
	pre_br = clock / (extDev->SpiBaudrate
			* clk_table_pbr[spiHw->CTAR[ctar_unit].B.PBR]);

	// find closest lesser
	for (i = 0; i < sizeof(clk_table_br) / sizeof(clk_table_br[0]); i++) {
		if (clk_table_br[i] >= pre_br) {
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
	tmp = extDev->SpiTimeClk2Cs * (clock / 1000000);

	// Nothing fancy here...
	{
		int best_i = 0;
		int best_j = 0;
		int b_value = INT_MAX;
		int tt;

		// Find the best match of Prescaler and Scaler value
		for (i = 0; i < ARRAY_SIZE(clk_table_pasc); i++) {
			for (j = 0; j < ARRAY_SIZE(clk_table_asc); j++) {
				tt = abs((int) clk_table_pasc[i] * clk_table_asc[j] * 1000
						- tmp);
				if (tt < b_value) {
					best_i = i;
					best_j = j;
					b_value = tt;
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
	tmp = extDev->SpiTimeCs2Clk * (clock / 1000000);

	// Nothing fancy here...
	{
		int best_i = 0;
		int best_j = 0;
		int b_value = INT_MAX;
		int tt;

		// Find the best match of Prescaler and Scaler value
		for (i = 0; i < ARRAY_SIZE(clk_table_pcssck); i++) {
			for (j = 0; j < ARRAY_SIZE(clk_table_cssck); j++) {
				tt = abs((int) clk_table_pcssck[i] * clk_table_cssck[j] * 1000
						- tmp);
				if (tt < b_value) {
					best_i = i;
					best_j = j;
					b_value = tt;
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

	spiHw->CTAR[ctar_unit].B.LSBFE = (transferStart == SPI_TRANSFER_START_MSB ) ? 0 : 1;

	/* Set mode */
	spiHw->CTAR[ctar_unit].B.FMSZ = width - 1;
	spiHw->CTAR[ctar_unit].B.CPHA = (extDev->SpiDataShiftEdge
			== SPI_EDGE_LEADING) ? 0 : 1;
	spiHw->CTAR[ctar_unit].B.CPOL = (extDev->SpiShiftClockIdleLevel == STD_LOW) ? 0 : 1;

	// This the ACTIVE polarity. Freescale have inactive polarity
	if (extDev->SpiCsPolarity == STD_HIGH) {
		spiHw->MCR.R &= ~(1 << (16 + extDev->SpiCsIdentifier));
	} else {
		spiHw->MCR.R |= (1 << (16 + extDev->SpiCsIdentifier));
	}
}

//-------------------------------------------------------------------

static void Spi_InitController(Spi_UnitType *uPtr ) {

	volatile struct DSPI_tag *spiHw = uPtr->hwPtr;

	/* Module configuration register. */
	/* Master mode. */
	spiHw->MCR.B.MSTR = 1;
	/* No freeze. Run SPI when debugger is stopped. */
	spiHw->MCR.B.FRZ = 0;
	/* PSC5 as regular CS. */
	spiHw->MCR.B.PCSSE = 0;

	/* Enable FIFO's. */
#if (SPI_IMPLEMENTATION == SPI_DMA)
	spiHw->MCR.B.DIS_RXF = 1;
	spiHw->MCR.B.DIS_TXF = 1;
#elif (SPI_IMPLEMENTATION == SPI_FIFO)
    spiHw->MCR.B.DIS_RXF = 0;
    spiHw->MCR.B.DIS_TXF = 0;
#endif


	/* Set all active low. */
	spiHw->MCR.B.PCSIS0 = 1;
	spiHw->MCR.B.PCSIS1 = 1;
	spiHw->MCR.B.PCSIS2 = 1;
	spiHw->MCR.B.PCSIS3 = 1;
	spiHw->MCR.B.PCSIS4 = 1;
	spiHw->MCR.B.PCSIS5 = 1;

#if (SPI_IMPLEMENTATION == SPI_DMA)
	/* DMA TX FIFO fill. */
	spiHw->RSER.B.TFFF_RE = 1;
	spiHw->RSER.B.TFFF_DIRS = 1;

	/* DMA RX FIFO drain. */
	spiHw->RSER.B.RFDF_RE = 1;
	spiHw->RSER.B.RFDF_DIRS = 1;
#endif

	// Setup CTAR's channel codes..
	for (int i = 0; i < (CTAR_CNT-1); i++) {
		uPtr->channelCodes[i] = CH_NOT_VALID;
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
	switch (uPtr->hwUnit) {
#if (SPI_USE_HW_UNIT_0 == STD_ON )
	case 0:
	ISR_INSTALL_ISR2("SPI_A",Spi_Isr_A, DSPI_A_ISR_EOQF, 15, 0);
	break;
#endif
#if (SPI_USE_HW_UNIT_1 == STD_ON )
	case 1:
	ISR_INSTALL_ISR2("SPI_B",Spi_Isr_B, DSPI_B_ISR_EOQF, 15, 0);
	break;
#endif
#if (SPI_USE_HW_UNIT_2 == STD_ON )
	case 2:
	ISR_INSTALL_ISR2("SPI_C",Spi_Isr_C, DSPI_C_ISR_EOQF, 15, 0);
	break;
#endif
#if (SPI_USE_HW_UNIT_3 == STD_ON )
	case 3:
	ISR_INSTALL_ISR2("SPI_D",Spi_Isr_D, DSPI_D_ISR_EOQF, 15, 0);
	break;
#endif
#if (SPI_USE_HW_UNIT_4 == STD_ON )
	case 4:
	ISR_INSTALL_ISR2("SPI_E",Spi_Isr_E, DSPI_E_ISR_EOQF, 15, 0);
	break;
#endif
#if (SPI_USE_HW_UNIT_5 == STD_ON )
	case 5:
	ISR_INSTALL_ISR2("SPI_F",Spi_Isr_F, DSPI_F_ISR_EOQF, 15, 0);
	break;
#endif
	}
}

//-------------------------------------------------------------------

#if (SPI_IMPLEMENTATION==SPI_DMA)
static void Spi_DmaSetup( Spi_UnitType *uPtr ) {

	Dma_TcdType *tcd;

	tcd = &uPtr->dmaTxTCD;
	*tcd = Spi_DmaTx;
	tcd->SADDR = (uint32) uPtr->txQueue;
	tcd->DADDR = (uint32) &(uPtr->hwPtr->PUSHR.R);

	Dma_StopChannel(uPtr->dmaTxChannel);
	Dma_CheckConfig();

	// CITER and BITER set when we send
	tcd = &uPtr->dmaRxTCD;
	*tcd = Spi_DmaRx;
	tcd->SADDR = (uint32) &(uPtr->hwPtr->POPR.R);
	tcd->DADDR = (uint32) uPtr->rxQueue;

	Dma_StopChannel(uPtr->dmaRxChannel);
	Dma_CheckConfig();

}
#endif

//-------------------------------------------------------------------

void Spi_Init(const Spi_ConfigType *ConfigPtr) {
	const Spi_JobConfigType *jobConfig2;
	Spi_UnitType *uPtr;
	uint32 confMask;
	uint8 ctrlNr;
#if (SPI_IMPLEMENTATION == SPI_DMA )
	uint8 unitNr;
#endif


	memset(&Spi_Global,0,sizeof(Spi_Global));
	Spi_Global.configPtr = ConfigPtr;
	Spi_Global.extBufPtr = Spi_Eb;

	Spi_Global.asyncMode = SPI_INTERRUPT_MODE;

	Spi_Queue = CirqBuffStatCreate(spiQueueData, SPI_QUEUE_SIZE, sizeof(Spi_QueueType));

	// Set all sequence results to OK
	for (Spi_SequenceType i = (Spi_SequenceType) 0; i < SPI_MAX_SEQUENCE; i++) {
		Spi_SetSequenceResult(i, SPI_SEQ_OK);
	}
	// Figure out what HW controllers that are used
	for (int j = 0; j < Spi_GetJobCnt(); j++) {
		jobConfig2 = &Spi_Global.configPtr->SpiJobConfig[j];
		Spi_Global.spiHwConfigured |= (1 << jobConfig2->SpiHwUnit);
	}

	confMask = Spi_Global.spiHwConfigured;

	for (int i=0; confMask; confMask &= ~(1 << ctrlNr),i++) {
		ctrlNr = ilog2(confMask);
    Spi_CtrlToUnit[ctrlNr] = i;

		DEBUG(DEBUG_LOW,"%s:Configured HW controller %d\n",MODULE_NAME,ctrlNr);
		uPtr = GET_SPI_UNIT_PTR(ctrlNr);
		uPtr->hwPtr = GET_SPI_HW_PTR(ctrlNr);
		uPtr->hwUnit = ctrlNr;
		Spi_InitController(uPtr);
		Spi_SetHWUnitStatus(uPtr, SPI_IDLE);
#if (SPI_IMPLEMENTATION == SPI_DMA )
		// DMA init...
		//
		unitNr = Spi_CtrlToUnit[ctrlNr];

		/* Make sure that this channel shall be used. */
		//assert (ConfigPtr->SpiHwConfig[ctrlNr].Activated);
		assert(Spi_DmaConfig[unitNr].TxDmaChannel != (-1));
		assert(Spi_DmaConfig[unitNr].RxDmaChannel != (-1));

		uPtr->dmaTxChannel = Spi_DmaConfig[unitNr].TxDmaChannel;
		uPtr->dmaRxChannel = Spi_DmaConfig[unitNr].RxDmaChannel;

		Spi_DmaSetup(uPtr);
#endif

	}

	/* Setup the relations for Job, for easy access */
	for(int j=0; j<SPI_MAX_JOB; j++ ) {
		jobConfig2 = &Spi_Global.configPtr->SpiJobConfig[j];
		Spi_JobUnit[j].jobCfgPtr = jobConfig2;
		Spi_JobUnit[j].extDeviceCfgPtr = &Spi_Global.configPtr->SpiExternalDevice[jobConfig2->DeviceAssignment];
		Spi_JobUnit[j].unitPtr = GET_SPI_UNIT_PTR(jobConfig2->SpiHwUnit);
		Spi_JobUnit[j].hwPtr= GET_SPI_HW_PTR(jobConfig2->SpiHwUnit);
		Spi_JobUnit[j].channelsPtr = &jobConfig2->ChannelAssignment[0];
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

		for (j = 0; j < Spi_GetJobCnt(); j++) {
			jobConfig = &Spi_Global.configPtr->SpiJobConfig[j];
			spiUnit = GET_SPI_UNIT_PTR( jobConfig->SpiHwUnit );

			// Also find the controllers used while we are at it
			Spi_Global.spiHwConfigured |= (1 << jobConfig->SpiHwUnit);

			// ..and set the job status
			Spi_SetJobResult((Spi_JobType) j, SPI_JOB_OK);

			l = 0;
			// Go through all the jobs and it's channels to setup CTAS
			// A job have the same physical controller ( SpiHwUnit )
			while ((channelIndex = jobConfig->ChannelAssignment[l++])
					!= CH_NOT_VALID) {
				chConfig = &Spi_Global.configPtr->SpiChannelConfig[channelIndex];

				// Form a channel code from
				// <MSB/LSB><external_device_id><channel width>
				channelCode = ( ((uint32_t)chConfig->SpiTransferStart << 16) +
								(jobConfig->DeviceAssignment << 8) +
								chConfig->SpiDataWidth);

				for (k = 0; k < (CTAR_CNT-1); k++) {
					if (spiUnit->channelCodes[k] == channelCode) {
						Spi_ChannelInfo[channelIndex].ctarId = k;
						DEBUG(DEBUG_LOW,"%s: Channel %d re-uses CTAR %d@%d . device=%d,width=%d\n",MODULE_NAME,channelIndex,k,jobConfig->SpiHwUnit,jobConfig->DeviceAssignment,chConfig->SpiDataWidth);
						// already in list, break
						break;
					}

					if (spiUnit->channelCodes[k] == CH_NOT_VALID) {
						// Add to list
						spiUnit->channelCodes[k] = channelCode;
						// Assign the CTAR index to channel info..
						DEBUG(DEBUG_LOW,"%s: Channel %d uses    CTAR %d@%d . device=%d,width=%d\n",MODULE_NAME,channelIndex,k,jobConfig->SpiHwUnit,jobConfig->DeviceAssignment,chConfig->SpiDataWidth);

						Spi_SetupCTAR(
								jobConfig->SpiHwUnit,
								Spi_GetExternalDevicePtrFromIndex( jobConfig->DeviceAssignment),
								(Spi_ChannelType) k,
								chConfig->SpiTransferStart,
								chConfig->SpiDataWidth);

						Spi_ChannelInfo[channelIndex].ctarId = k;
						break;
					}
				}
				/* No more CTARS */
				assert(k<(CTAR_CNT-1));
			}
		}
	}

	Spi_Global.initRun = TRUE;
}

//-------------------------------------------------------------------

Std_ReturnType Spi_DeInit(void) {
	volatile struct DSPI_tag *spiHw;
	uint32 confMask;
	uint8 confNr;
	Spi_UnitType *uPtr;


	VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_DEINIT_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );
	if (Spi_GetStatus() == SPI_BUSY)
		return E_NOT_OK;

	// Disable the HW modules ( SPI021 )
	confMask = Spi_Global.spiHwConfigured;

	// Disable the SPI hw
	for (; confMask; confMask &= ~(1 << confNr)) {
		confNr = ilog2(confMask);
		spiHw = GET_SPI_HW_PTR(confNr);
		// Disable the hardware..
		spiHw->MCR.B.MDIS = 1;
		uPtr = GET_SPI_UNIT_PTR(confNr);

		Spi_InitController(uPtr);
		Spi_SetHWUnitStatus(uPtr, SPI_IDLE);
	}

	// SPI022
	Spi_Global.configPtr = NULL;
	Spi_Global.initRun = FALSE;

	return E_OK;
}

#if (SPI_IMPLEMENTATION==SPI_DMA)
/**
 *
 * @param spiUnit
 * @param jobConfig
 * @return
 */
static void Spi_DoWrite_DMA(	Spi_UnitType *spiUnit,Spi_JobType jobIndex,
						const Spi_JobConfigType *jobConfig )
{
	int j = 0;
	int k = 0;
	int channelIndex;
	const Spi_ChannelConfigType *chConfig;
	Spi_EbType *extChBuff;
	Spi_CommandType cmd;

	cmd.R = 0;

	// Find the channels for this job
	while ((channelIndex = jobConfig->ChannelAssignment[j++]) != CH_NOT_VALID) {
		chConfig = &Spi_Global.configPtr->SpiChannelConfig[channelIndex];

		/* Check configuration error */
#if ( SPI_CHANNEL_BUFFERS_ALLOWED == 1  )
		assert( chConfig->SpiChannelType == SPI_EB );
#endif

		// Send the channels that are setup with external buffers
		// Get the external buffer for this channel
		extChBuff = &Spi_Global.extBufPtr[channelIndex];

		if (extChBuff->active == 0) {
			LDEBUG_PRINTF("Err:External buffer %d@job %d not setup\n",channelIndex,jobIndex);
			(void)jobIndex;
			assert(0);
		}

		// Start to fill the SPI queue for the DMA:
		for (k = 0; k < extChBuff->length; k++) {
			uint32 csId =
			Spi_Global.configPtr->SpiExternalDevice[jobConfig->DeviceAssignment].SpiCsIdentifier;
			// Build cmd
			cmd.B.CONT = 1; // Channels should keep CS active
			// ( A job must assert CS continuously)
			cmd.R |= (1 << (16 + csId)); // Set PCS
			cmd.B.EOQ = 0;
			cmd.B.CTAS = Spi_ChannelInfo[channelIndex].ctarId;
			if (extChBuff->src != NULL) {
				if (chConfig->SpiDataWidth > 8) {
					cmd.B.TXDATA = (extChBuff->src[k] << 8) + (extChBuff->src[k + 1] & 0xff);
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
	cmd.B.CONT = 0; // CS high
	cmd.B.EOQ = 1; // last in queue
	spiUnit->txQueue[--spiUnit->txCurrIndex].R = cmd.R;

	// Set the length of the data to send
	spiUnit->dmaTxTCD.CITER = spiUnit->txCurrIndex + 1;
	spiUnit->dmaTxTCD.BITER = spiUnit->txCurrIndex + 1;

	Spi_Global.totalNbrOfStartedJobs++;

	RAMLOG_STR("Job: "); RAMLOG_DEC(jobIndex); RAMLOG_STR(" Cnt: "); RAMLOG_DEC(spiUnit->txCurrIndex+1); RAMLOG_STR("\n");

	DEBUG(DEBUG_LOW,"%s:Tx Job:%d cnt:%d first data:%04x\n",MODULE_NAME,jobIndex,spiUnit->txCurrIndex+1,spiUnit->txQueue[0].B.TXDATA);

	{
		Spi_UnitType *spiUnit = GET_SPI_UNIT_PTR(jobConfig->SpiHwUnit);
		volatile struct DSPI_tag *spiHw = GET_SPI_HW_PTR(jobConfig->SpiHwUnit);

		Dma_ConfigureChannel((Dma_TcdType *) &spiUnit->dmaTxTCD,
				spiUnit->dmaTxChannel);
		Dma_ConfigureChannel((Dma_TcdType *) &spiUnit->dmaRxTCD,
				spiUnit->dmaRxChannel);
		/* Flush TX/Rx FIFO.  Ref. man. 23.5.1 step 8 */
		spiHw->MCR.B.CLR_RXF = 1;
		spiHw->MCR.B.CLR_TXF = 1;

		Dma_StartChannel(spiUnit->dmaRxChannel);
		Dma_StartChannel(spiUnit->dmaTxChannel);

		// Step 9. Clear TCNT
		spiHw->TCR.B.SPI_TCNT = 0;

		if (	(Spi_Global.asyncMode == SPI_INTERRUPT_MODE) &&
				(spiUnit->callType == SPI_ASYNC_CALL)) {
			ENABLE_EOQ_INTERRUPT(spiHw);
		} else {
			DISABLE_EOQ_INTERRUPT(spiHw);
		}

		/* This will trig a new transfer. Ref. man. 23.5.1 step 11 */
		spiHw->SR.B.EOQF = 1;
		spiHw->MCR.B.HALT = 0;

		// Since it's not obvious on how to tell when a SPI sequence
		// is sent, keep things below to what things have been tested.
#if defined(STEP_VALIDATION)
		/* Wait for transfer to complete (EOQF bit is set) */
		while (spiHw->SR.B.EOQF==1) {
			Spi_Global.eoqf_cnt++;
		}
		while (spiHw->SR.B.TXRXS) {Spi_Global.txrxs_cnt++;}
		while( EDMA.TCD[spiUnit->dmaRxChannel].ACTIVE ) {;}
#endif

	}
}
#endif

#if (SPI_IMPLEMENTATION==SPI_FIFO)
/**
 *
 * @param spiUnit
 * @param jobConfig
 * @return
 */
static void Spi_WriteJob_FIFO( Spi_JobType jobIndex )
{
	Spi_ChannelType                 currChannel;
	Spi_CommandType                 cmd;
	Spi_CommandType                 dCmd;
    Spi_NumberOfDataType            bufLen;
	const Spi_ChannelConfigType *   chConfig;
    Spi_JobUnitType *               jobUnitPtr;
    const Spi_DataType *            buf;
    Spi_NumberOfDataType            copyCnt;
    Spi_NumberOfDataType            fifoLeft;
//    boolean                         done = 0;
//    boolean                         lastJob = 0;
	int     i;
	jobUnitPtr = &Spi_JobUnit[jobIndex];


	fifoLeft = FIFO_DEPTH;

	RAMLOG_STR("Spi_WriteJob_FIFO\n");

#if defined(STEP_VALIDATION)
    if(  jobUnitPtr->hwPtr->SR.B.TXCTR != 0 ) {
        SPI_ASSERT(0);
    }
#endif

    cmd.R = 0;
    dCmd.R = 0xfffffffful;

	/* Iterate over the channels for this job */
    currChannel = jobUnitPtr->channelsPtr[jobUnitPtr->currTxChIndex];
    while ( fifoLeft != 0) {

		chConfig = &Spi_Global.configPtr->SpiChannelConfig[currChannel];
		buf = spiGetTxBuf( currChannel, &bufLen);

		/* Minimum of how much data to copy and the limit of the FIFO */
		copyCnt = MIN( (bufLen - jobUnitPtr->txChCnt) >> ((chConfig->SpiDataWidth-1)/8), fifoLeft );

        /* Channels should keep CS active ( A job must assert CS continuously) */
        cmd.B.CONT = 1;
        /* Set the Chip Select (PCS) */
        cmd.R |= (1 << (16 + jobUnitPtr->extDeviceCfgPtr->SpiCsIdentifier));

#if defined(STEP_VALIDATION)
        if( cmd.B.EOQ == 1) {
            SPI_ASSERT(0);
        }
#endif

        /* Push as much as we can (FIFO or Channel limits) */
        for(i=0; i < copyCnt ; i++ ) {
            cmd.B.CTAS = Spi_ChannelInfo[currChannel].ctarId;
            if (buf != NULL) {
                if (chConfig->SpiDataWidth > 8 ) {
                    cmd.B.TXDATA =  (buf[jobUnitPtr->txChCnt] << 8) +
                                    (buf[jobUnitPtr->txChCnt  + 1] & 0xff);
                } else {
                    cmd.B.TXDATA = buf[jobUnitPtr->txChCnt];
                }
            } else {
                cmd.B.TXDATA = chConfig->SpiDefaultData;
            }

            if (chConfig->SpiDataWidth > 8 ) {
                jobUnitPtr->txChCnt+=2;
            } else {
                jobUnitPtr->txChCnt++;
            }


            if( dCmd.R != 0xfffffffful) {
                jobUnitPtr->hwPtr->PUSHR.R = dCmd.R;  // Write delayed
            }

            dCmd.R = cmd.R;     // Save it
            --fifoLeft;
        } /* for */

        RAMLOG_STR("CI#");RAMLOG_DEC(jobUnitPtr->currTxChIndex);RAMLOG_STR("\n");


        /* Done with channel? */
        if( ((bufLen -  jobUnitPtr->txChCnt) == 0) ) {
            jobUnitPtr->txChCnt = 0;
            /* Done with job? */
            jobUnitPtr->currTxChIndex++;
            currChannel = jobUnitPtr->channelsPtr[jobUnitPtr->currTxChIndex];
            if( currChannel == CH_NOT_VALID ) {
                jobUnitPtr->currTxChIndex = 0;
                cmd.B.CONT = 0;
                break;
            }
        }

    } /* while */

    cmd.B.EOQ = 1;
    jobUnitPtr->hwPtr->PUSHR.R = cmd.R;

    jobUnitPtr->fifoSent = FIFO_DEPTH - fifoLeft;

#if defined(STEP_VALIDATION)
	if( jobUnitPtr->fifoSent != jobUnitPtr->hwPtr->SR.B.TXCTR ) {
	    SPI_ASSERT(0);
	}
#endif

	jobUnitPtr->hwPtr->TCR.B.SPI_TCNT = 0;

    if ( (Spi_Global.asyncMode == SPI_INTERRUPT_MODE) &&
         (jobUnitPtr->unitPtr->callType == SPI_ASYNC_CALL)) {
        ENABLE_EOQ_INTERRUPT(jobUnitPtr->hwPtr);
    } else {
        DISABLE_EOQ_INTERRUPT(jobUnitPtr->hwPtr);
    }

#if defined(STEP_VALIDATION)
    /* Verify FIFO before sending */
    {
        SPICommandType *base = (SPICommandType *)&jobUnitPtr->hwPtr->TXFR[0];
        SPICommandType *curr;
        int i,lastIndex;

        i = jobUnitPtr->hwPtr->SR.B.TXNXTPTR;	/* Next entry to send */
        lastIndex = ( i + jobUnitPtr->hwPtr->SR.B.TXCTR - 1 ) % FIFO_DEPTH;
        while( i != lastIndex )  {
            curr = base + i;
            if( curr->B.EOQ == 1) {  /* This entry must not have EOQ set */
                SPI_ASSERT(0);
            }

            i = (i + 1) % FIFO_DEPTH;
        }
        curr = base + i;
        if( curr->B.EOQ != 1) {
            SPI_ASSERT(0);		/* Last entry must have EOQ set */
        }
    }
#endif

	jobUnitPtr->hwPtr->SR.B.EOQF = 1;
	jobUnitPtr->hwPtr->MCR.B.HALT = 0;

#if defined(STEP_VALIDATION)
        /* Wait for transfer to complete (EOQF bit is set) */
        while (jobUnitPtr->hwPtr->SR.B.EOQF==1) {
            Spi_Global.eoqf_cnt++;
        }
        while (jobUnitPtr->hwPtr->SR.B.TXRXS) {Spi_Global.txrxs_cnt++;}
#endif

	// TODO: Clear FIFO's? CLR_TXF?

}
#endif /* (SPI_IMPLEMENTATION==SPI_FIFO) */


//-------------------------------------------------------------------




/**
 * Write a job to the SPI bus
 *
 * @param jobIndex The job to write to the SPI bus
 */
static void Spi_JobWrite(Spi_JobType jobIndex) {
	Spi_UnitType *uPtr = Spi_JobUnit[jobIndex].unitPtr;

	uPtr->txCurrIndex = 0;
	uPtr->currJob = Spi_JobUnit[jobIndex].jobCfgPtr;
	uPtr->currJobIndex = jobIndex;
#if (SPI_IMPLEMENTATION == SPI_FIFO)
	Spi_JobUnit[jobIndex].txChCnt = 0;
	Spi_JobUnit[jobIndex].rxChCnt = 0;
#endif

	Spi_SetHWUnitStatus(uPtr, SPI_BUSY);
	Spi_SetJobResult(jobIndex, SPI_JOB_PENDING);

#if (USE_DIO_CS == STD_ON)
    if( Spi_JobUnit[jobIndex].extDeviceCfgPtr->SpiCsCallback != NULL ) {
        Spi_JobUnit[jobIndex].extDeviceCfgPtr->SpiCsCallback(1);
    }
#endif

#if (SPI_IMPLEMENTATION==SPI_DMA)
	Spi_DoWrite_DMA( uPtr, jobIndex, Spi_JobUnit[jobIndex].jobCfgPtr );
#elif (SPI_IMPLEMENTATION==SPI_FIFO)
	Spi_JobUnit[jobIndex].currTxChIndex = 0;
	Spi_WriteJob_FIFO ( jobIndex );
#endif

}

#if defined(USE_LDEBUG_PRINTF) && ( DEBUG_LVL <= DEBUG_HIGH )
void Spi_PrintSeqInfo(const Spi_SequenceConfigType *seqConfigPtr) {
	int i = 0;
	uint32 job;
	DEBUG(DEBUG_HIGH,"%s: Seq: %d:",MODULE_NAME,seqConfigPtr->SpiSequenceId);

	while ((job = seqConfigPtr->JobAssignment[i]) != JOB_NOT_VALID) {
		DEBUG(DEBUG_HIGH,"%d ",job);
		i++;
	} DEBUG(DEBUG_HIGH,"\n");
}
#endif


/**
 * Write a sequence to the SPI bus
 *
 * @param seqIndex The sequence
 * @param sync 1 - make the call sync. 0 - make the call async
 */
static void Spi_SeqWrite(Spi_SequenceType seqIndex, Spi_CallTypeType sync) {

	const Spi_SequenceConfigType *seqConfig;
	const Spi_JobConfigType *jobConfig;
	Spi_UnitType *uPtr;
	Spi_JobType jobIndex;

	seqConfig = Spi_GetSeqPtrFromIndex(seqIndex);
	jobIndex = seqConfig->JobAssignment[0];
	jobConfig = Spi_GetJobPtrFromIndex(jobIndex);

	Spi_SetSequenceResult(seqIndex, SPI_SEQ_PENDING);

	/* Queue the job */
	{
		const Spi_JobType *jobPtr = &seqConfig->JobAssignment[0];
		/* queue the job */
		while (*jobPtr != JOB_NOT_VALID) {
			Spi_SetJobResult(*jobPtr, SPI_JOB_QUEUED);
			jobPtr++;
		}
	}

	uPtr = GET_SPI_UNIT_PTR(jobConfig->SpiHwUnit);

	/* If busy, check later... */
	if( uPtr->status == SPI_BUSY ) {
		Spi_QueueType qEntry;
		qEntry.seq = seqIndex;
		qEntry.callType = sync;
		CirqBuffPush(&Spi_Queue,&qEntry);
		return;
	}

	/* Fill in the required fields for job and sequence.. */
	uPtr->currJobIndexPtr = &seqConfig->JobAssignment[0];
	uPtr->callType = sync;
	uPtr->currSeqPtr = seqConfig;


	// Setup interrupt for end of queue
	if (	(Spi_Global.asyncMode == SPI_INTERRUPT_MODE) &&
			(uPtr->callType== SPI_ASYNC_CALL)) {
		DEBUG(DEBUG_MEDIUM,"%s: async/interrupt mode\n",MODULE_NAME);
	} else {
		DEBUG(DEBUG_MEDIUM,"%s: sync/polled mode\n",MODULE_NAME);
	}

#if defined(USE_LDEBUG_PRINTF) && ( DEBUG_LVL <= DEBUG_HIGH )
	Spi_PrintSeqInfo( seqConfig );
#endif

	Spi_JobWrite(jobIndex);

	if (uPtr->callType == SPI_SYNC_CALL) {
		while (Spi_GetSequenceResult(seqIndex) == SPI_SEQ_PENDING) {
			Spi_Isr(uPtr);
		}
	}
}

//-------------------------------------------------------------------
static _Bool Spi_AnyPendingJobs(Spi_SequenceType Sequence) {

	// Check that we don't share any jobs with another sequence that is SPI_SEQ_PENDING
	for (int i = 0; i < SPI_MAX_SEQUENCE; i++) {
		if (i == Sequence) {
			continue;
		}

		if (Spi_GetSequenceResult(i) == SPI_SEQ_PENDING) {
			// We have found a pending sequence... check that we don't share any jobs
			// with that sequence, SPI086
			if (Spi_ShareJobs(Sequence, i)) {
				return 1;
			}
		}
	}

	return 0;
}

//-------------------------------------------------------------------


/**
 * Blocking write
 *
 * @param Sequence
 * @return
 */
Std_ReturnType Spi_SyncTransmit(Spi_SequenceType Sequence) {

	VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_SYNCTRANSMIT_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );
	VALIDATE_W_RV( ( SPI_MAX_SEQUENCE>Sequence ), SPI_SYNCTRANSMIT_SERVICE_ID, SPI_E_PARAM_SEQ, E_NOT_OK );
	Std_ReturnType rv = E_OK;

	if (Spi_GetSequenceResult(Sequence) == SPI_SEQ_PENDING) {
		DEBUG(DEBUG_LOW,"%s: Spi_AsyncTransmit() is PENDING\n",MODULE_NAME);
		return E_NOT_OK; // SPI157
	}

	assert(Spi_GetSequenceResult(Sequence) == SPI_SEQ_OK);

	if (Spi_AnyPendingJobs(Sequence)) {
		return E_NOT_OK;
	}

	Spi_SeqWrite(Sequence, SPI_SYNC_CALL);

	return rv;
}

//-------------------------------------------------------------------

Std_ReturnType Spi_AsyncTransmit(Spi_SequenceType Sequence) {
	VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_ASYNCTRANSMIT_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );
	VALIDATE_W_RV( ( SPI_MAX_SEQUENCE>Sequence ), SPI_ASYNCTRANSMIT_SERVICE_ID, SPI_E_PARAM_SEQ, E_NOT_OK );

	if (Spi_GetSequenceResult(Sequence) == SPI_SEQ_PENDING) {
		DEBUG(DEBUG_LOW,"%s: Spi_AsyncTransmit() is PENDING\n",MODULE_NAME);
		return E_NOT_OK; // SPI157
	}

	assert(Spi_GetSequenceResult(Sequence) == SPI_SEQ_OK);

	if (Spi_AnyPendingJobs(Sequence)) {
		return E_NOT_OK;
	}

	DEBUG(DEBUG_LOW,"%s: Starting seq: %d\n",MODULE_NAME,Sequence);

	Spi_SeqWrite(Sequence, SPI_ASYNC_CALL);

	return E_OK;
}

//-------------------------------------------------------------------


Std_ReturnType Spi_ReadIB(	Spi_ChannelType Channel,
							Spi_DataType * const DataBufferPtr)
{
	(void)DataBufferPtr;

	VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_READIB_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );
	VALIDATE_W_RV( ( Channel<SPI_MAX_CHANNEL ), SPI_READIB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );
	VALIDATE_W_RV( ( SPI_IB<Spi_Global.configPtr->SpiChannelConfig[Channel].SpiChannelType ),
			SPI_READIB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );

	/* NOT SUPPORTED */

	Std_ReturnType rv = E_NOT_OK;
	return rv;
}

//-------------------------------------------------------------------

Std_ReturnType Spi_SetupEB(	Spi_ChannelType Channel,
							const Spi_DataType* SrcDataBufferPtr,
							Spi_DataType* DesDataBufferPtr,
							Spi_NumberOfDataType Length)
{
	VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_SETUPEB_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );
	VALIDATE_W_RV( ( Channel<SPI_MAX_CHANNEL ), SPI_SETUPEB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );
#if ( SPI_CHANNEL_BUFFERS_ALLOWED == 1 )
	VALIDATE_W_RV( ( SPI_EB==Spi_Global.configPtr->SpiChannelConfig[Channel].SpiChannelType ),
			SPI_SETUPEB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );
#endif
	// SPI067
	VALIDATE_W_RV( ( Length<=Spi_Global.configPtr->SpiChannelConfig[Channel].SpiEbMaxLength ),
			SPI_SETUPEB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );

	Spi_EbType *extChBuff = &Spi_Global.extBufPtr[Channel];
	const Spi_ChannelConfigType *chConfig =
			&Spi_Global.configPtr->SpiChannelConfig[Channel];

	if (chConfig->SpiChannelType == SPI_EB) {
		extChBuff->src = SrcDataBufferPtr;
		extChBuff->dest = DesDataBufferPtr;
		extChBuff->length = Length;
		extChBuff->active = 1;
	} else {
		/* NOT SUPPORTED */
		assert(0);
		while (1)
			;
	}

	return E_OK;
}

//-------------------------------------------------------------------

Spi_StatusType Spi_GetStatus(void) {
	VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_GETSTATUS_SERVICE_ID, SPI_E_UNINIT, SPI_UNINIT );

	// Check all sequences if they have any job pending
	for (int i = 0; i < SPI_MAX_SEQUENCE; i++) {
		if (Spi_GetSequenceResult(i) == SPI_SEQ_PENDING) {
			return SPI_BUSY;
		}
	}

	return SPI_IDLE;
}

//-------------------------------------------------------------------


Spi_JobResultType Spi_GetJobResult(Spi_JobType Job) {
	VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_GETJOBRESULT_SERVICE_ID, SPI_E_UNINIT, SPI_JOB_FAILED );
	VALIDATE_W_RV( ( SPI_MAX_JOB<Job ), SPI_GETJOBRESULT_SERVICE_ID, SPI_E_PARAM_JOB, SPI_JOB_FAILED );

	return Spi_JobUnit[Job].jobResult;
}

//-------------------------------------------------------------------


Spi_SeqResultType Spi_GetSequenceResult(Spi_SequenceType Sequence) {
	Spi_SeqResultType rv;

	VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_GETSEQUENCERESULT_SERVICE_ID, SPI_E_UNINIT, SPI_SEQ_FAILED );
	VALIDATE_W_RV( ( SPI_MAX_SEQUENCE>Sequence ), SPI_GETSEQUENCERESULT_SERVICE_ID, SPI_E_PARAM_SEQ, SPI_SEQ_FAILED );

	rv = Spi_SeqUnit[Sequence].seqResult;

	return rv;
}

//-------------------------------------------------------------------

Spi_StatusType Spi_GetHWUnitStatus(Spi_HWUnitType HWUnit) {
	VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_GETHWUNITSTATUS_SERVICE_ID, SPI_E_UNINIT, SPI_UNINIT );

	return (GET_SPI_UNIT_PTR(HWUnit))->status;
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
Std_ReturnType Spi_SetAsyncMode(Spi_AsyncModeType Mode) {
	VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_SETASYNCMODE_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );

	// Note!
	// Not really sure by whom this function is supposed to be called by.
	// The users of SPI(e2,flash,etc) should probably not use this.

	for (int i = 0; i < SPI_MAX_SEQUENCE; i++) {
		if (Spi_GetSequenceResult(i) == SPI_SEQ_PENDING) {
			return E_NOT_OK;
		}
	}

	Spi_Global.asyncMode = Mode;

	return E_OK;
}
#endif

//-------------------------------------------------------------------

void Spi_MainFunction_Handling(void) {
	/* NOT SUPPORTED */
}

//-------------------------------------------------------------------

void Spi_MainFunction_Driving(void) {
	volatile struct DSPI_tag *spiHw;
	uint32 confMask;
	uint8 ctrlNr;
	Spi_UnitType *uPtr;

	// TODO: check that the queue is empty.. if so do the next job.
	if (Spi_Global.asyncMode == SPI_POLLING_MODE) {
		confMask = Spi_Global.spiHwConfigured;

		for (; confMask; confMask &= ~(1 << ctrlNr)) {
			ctrlNr = ilog2(confMask);
			uPtr = GET_SPI_UNIT_PTR(ctrlNr);
			spiHw = uPtr->hwPtr;

			if (Spi_GetHWUnitStatus(ctrlNr) == SPI_BUSY) {
				if (spiHw->SR.B.TXRXS) {
					// Still not done..
				} else {
					uPtr = GET_SPI_UNIT_PTR(ctrlNr);
					Spi_Isr(uPtr);
				}
			}
		}
	}
}

