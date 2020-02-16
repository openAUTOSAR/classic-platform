/*
 * Spi.c
 *
 *  Created on: May 7, 2013
 *      Author: Zhang Shuzhou
 */

/* ----------------------------[includes]------------------------------------*/

#include <assert.h>
#include "Spi.h"
#include "bcm2835.h"
#include "Det.h"

//#include "isr.h"
//#include "irq_types.h"

/****************************************************************************/
/*	 				 			Definitions							   		*/
/****************************************************************************/

#define LENTH 						16
#define SPI_CONTROLLER_CNT 			1

typedef enum {
	SPIE_BAD = -1,
	SPIE_OK = 0,
	SPIE_JOB_NOT_DONE = 1
} Spi_TransferStatus;

/* The depth of the HW FIFO */
#define FIFO_DEPTH					4

typedef struct {
    Spi_DataType   					txbuf[LENTH];     	/* Pointer to transmit buffer */
	Spi_DataType   					rxbuf[LENTH];		/* Pointer to receive buffer */
	uint8    						n; 					/* Number of elements of Spi_DataType in destination buffer */
	uint8    						rn;
} Spi_DataExchangeType;

typedef struct {
	const Spi_DataType *   		 	src;   				/* Pointer to source buffer */
	Spi_DataType *          		dest;   			/* Pointer to destination buffer */
	Spi_NumberOfDataType    		length; 			/* Number of elements of Spi_DataType in destination buffer */
	boolean                   		active; 			/* Set if the buffer is configured */
} Spi_EbType;

typedef enum {
	SPI_ASYNC_CALL,
	SPI_SYNC_CALL,
} Spi_CallType;

typedef struct {
	uint8 							ctarId;    			// this channel is assigned to this CTAR
} Spi_ChannelInfoType;

/**
 * This structure represents an SPI controller unit
 */
typedef struct {
	uint32                          txCurrIndex;      	// current index for data when sending
	const Spi_JobConfigType *       currJob;         	// The current job
	const Spi_JobType *             currJobIndexPtr; 	// Points array of jobs current
	const Spi_SequenceConfigType *  currSeqPtr;      	// The Sequence
    Spi_DataExchangeType*           Spi_DataExchange;
    Spi_StatusType                  status;          	// Status for this unit
    Spi_JobType                     currJobIndex;
	Spi_CallType 					callType;           // 1 -  if the current job is sync. 0 - if not
	uint8							hwUnit;				// 0...
} Spi_UnitType;

typedef struct {
	Spi_SeqResultType 				seqResult;
} Spi_SeqUnitType;

typedef struct {
	const Spi_ExternalDeviceType *  extDeviceCfgPtr;  	/* The external device used by this job */
	const Spi_JobConfigType *       jobCfgPtr;
	Spi_UnitType *                  unitPtr;
	const Spi_ChannelType *         channelsPtr;

	uint32    						fifoSent;          /* Number of bytes in FIFO (before EOQ is set) */
	uint8     						currTxChIndex;     /* the currently transmitting channel index for FIFO */
	uint32    						txChCnt;           /* number of Spi_DataType sent for the current channel */
	uint32    						rxChCnt;           /* number of Spi_DataType received for the current channel */
	uint32    						currRxChIndex;     /* the currently receiving channel index for FIFO */

	Spi_JobResultType jobResult;
} Spi_JobUnitType;

typedef struct {
	boolean 				initRun;            // Initially FALSE, set to TRUE if Spi_Init() have been called
	const Spi_ConfigType *  configPtr;          // Pointer to the configuration
	Spi_EbType *            extBufPtr;          // Pointer to the external buffers
	Spi_ChannelInfoType *   channelInfo;		// Channel info (containing its id)
	uint32                  spiHwConfigured;    // Mask if the HW unit is configured or not
	Spi_AsyncModeType       asyncMode;			// Asynchronous communication mode
} Spi_GlobalType;

static Spi_GlobalType  Spi_Global;
static Spi_EbType      Spi_Eb[SPI_MAX_CHANNEL];
static Spi_UnitType    Spi_Unit[SPI_CONTROLLER_CNT];
static Spi_SeqUnitType Spi_SeqUnit[SPI_MAX_SEQUENCE];
static Spi_JobUnitType Spi_JobUnit[SPI_MAX_JOB];
static uint8 Spi_CtrlToUnit[sizeof(Spi_Global.spiHwConfigured)*8];

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

#define GET_SPI_UNIT_PTR(_unit) &Spi_Unit[Spi_CtrlToUnit[_unit]]

/****************************************************************************/
/*	 				 	Functions implemented in Spi_Lcfg.c 				*/
/****************************************************************************/

uint32 Spi_GetJobCnt(void);
uint32 Spi_GetChannelCnt(void);
uint32 Spi_GetExternalDeviceCnt(void);


/****************************************************************************/
/*	 				 		Local functions 							   	*/
/****************************************************************************/

// Not implemented (yet)
//static void Spi_Isr(Spi_UnitType *uPtr );

/**
 * Get the next job to be executed
 *
 * @param spiUnit 			----- The SPI unit, with the info about pending SPI communication
 * @return 					----- job ID or -1 if no more jobs are scheduled
 */
static Spi_JobType Spi_GetNextJob(Spi_UnitType *spiUnit) {
	spiUnit->currJobIndexPtr++;
	return *(spiUnit->currJobIndexPtr);
}

/**
 * Record hardware status in an SPI unit structure
 *
 * @param spiUnit 			----- SPI unit
 * @param status 			----- new HW status, e.g. idle or busy
 */
static void Spi_SetHWUnitStatus( Spi_UnitType *spiUnit, Spi_StatusType status) {
	spiUnit->status = status;
}

/**
 * Record job result in a job unit structure
 *
 * @param job 				----- job index
 * @param result 			----- new job result, e.g. ok, pending, failed or queued
 */
static void Spi_SetJobResult(Spi_JobType job, Spi_JobResultType result) {
	Spi_JobUnit[job].jobResult = result;
}

/**
 * Record sequence result in a sequence unit structure
 *
 * @param sequence 			----- sequence index
 * @param result 			----- new sequence result, e.g. ok, pending, failed or cancelled
 */
static void Spi_SetSequenceResult(Spi_SequenceType sequence, Spi_SeqResultType result) {
	Spi_SeqUnit[sequence].seqResult = result;
}

/**
 * Get pointer to sequence configuration from the sequence index
 *
 * @param seqIndex 			----- sequence index
 * @return 					----- pointer to sequence configuration
 */
static const Spi_SequenceConfigType *Spi_GetSeqPtrFromIndex(Spi_SequenceType SeqIndex) {
	return &Spi_Global.configPtr->SpiSequenceConfig[SeqIndex];
}

/**
 * Get pointer to job configuration from the job index
 *
 * @param jobIndex 			----- job index
 * @return 					----- pointer to job configuration
 */
static const Spi_JobConfigType *Spi_GetJobCfgPtrFromIndex(Spi_JobType jobIndex) {
	return &Spi_Global.configPtr->SpiJobConfig[jobIndex];
}

/**
 * Check if two sequences share common jobs
 *
 * @param seq1 				----- sequence 1
 * @param seq2 				----- sequence 2
 * @return 					----- ::0 - if the sequences don't share any jobs
 *        						  ::!=0 - if they share jobs
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

/**
 * Check if a sequence contains any jobs that are also a part of another, currently pending, sequence
 *
 * @param seq 				----- sequence index
 * @return 					----- ::1 - if there is a pending sequence that share jobs with this one
 *        						  ::0 - if they are no pending sequences that share any of this sequences jobs
 */
static boolean Spi_AnyPendingJobs(Spi_SequenceType seq) {

	// Check that we don't share any jobs with another sequence that is SPI_SEQ_PENDING
	for (int i = 0; i < SPI_MAX_SEQUENCE; i++) {
		if (i == seq) {
			continue;
		}

		if (Spi_GetSequenceResult(i) == SPI_SEQ_PENDING) {
			// We have found a pending sequence... check that we don't share any jobs
			// with that sequence, SPI086
			if (Spi_ShareJobs(seq, i)) {
				return 1;
			}
		}
	}

	return 0;
}

/**
 * Get destination (receive) buffer for a channel.
 *
 * @param ch				----- channel index
 * @param length			----- pointer to the destination data length
 * @return buf				----- pointer to the destination buffer
 */
static Spi_DataType *Spi_GetRxBuf(Spi_ChannelType ch, Spi_NumberOfDataType *length) {
	Spi_DataType *buf;

	/* Find the configuration for this channels destination and store it in the return variables */
	if( Spi_Global.configPtr->SpiChannelConfig[ch].SpiChannelType == SPI_EB ) {
		*length = Spi_Global.extBufPtr[ch].length;
		buf = Spi_Global.extBufPtr[ch].dest;

	} else {
		/* No support for internal buffer */
		assert(0);
		buf = NULL;
	}

	return buf;
}

/**
 * Get source (transmit) buffer for a channel.
 *
 * @param ch				----- channel index
 * @param length			----- pointer to the source data length
 * @return buf				----- pointer to the source buffer
 */
static const Spi_DataType *Spi_GetTxBuf(Spi_ChannelType ch, Spi_NumberOfDataType *length) {
	const Spi_DataType *buf;

	/* Find the configuration for this channels source and store it in the return variables */
	if( Spi_Global.configPtr->SpiChannelConfig[ch].SpiChannelType == SPI_EB ) {
		*length = Spi_Global.extBufPtr[ch].length;
		buf = Spi_Global.extBufPtr[ch].src;
	} else {
		/* No support for internal buffer */
		assert(0);
		buf = NULL;
	}

	return buf;
}

/**
 * Fill receive buffers, either external (EB) or internal (IB)
 *
 * Example with 8-bit CMD, 16-bit ADDR and some 8-bit data.
 *    CMD |  ADDR   |     DATA
 *   | 12 | 23 | 34 | 00 | 01 | 02 | 03
 *      1    2    3    4    5    6           BYTE CNT
 *      1      2       3    4   (5)  (6)     FIFO
 * With a FIFO of 4 we can see that the CMD, ADDR and almost the whole
 * DATA channel is sent.
 *
 * @param spiUnit			----- pointer to the SPI unit
 * @return rv				----- result status
 */
static Spi_TransferStatus Spi_Rx_FIFO(Spi_UnitType *spiUnit) {
	const Spi_ChannelConfigType * 	chConfig;			// Channel configuration
	Spi_ChannelType      			currChannel;		// Channel
	Spi_JobUnitType *    			jobUnitPtr;			// Job
	Spi_DataType *       			buf;				// Data buffer
	Spi_NumberOfDataType 			bufLen;				// Data buffer length
    uint32               			copyCnt;			// Nr of characters to copy
	uint32               			bInChar;			// Bytes per character (normally 1, sometimes 2)
	uint32               			popVal;				// Popped value from FIFO

    int i = 0;											// Counter
    Spi_TransferStatus rv = SPIE_JOB_NOT_DONE;			// Result

    /* Get currently scheduled job and rx-channel */
	jobUnitPtr = &Spi_JobUnit[spiUnit->currJobIndex];
	currChannel = jobUnitPtr->channelsPtr[jobUnitPtr->currRxChIndex];
	assert(currChannel != CH_NOT_VALID);

	/* Copy data from FIFO to receiving channels */
   	while (jobUnitPtr->fifoSent != 0) {
	    /* Get this channels destination buffer (it should always have a valid length) */
		buf = Spi_GetRxBuf(currChannel, &bufLen);
		assert(bufLen != 0);

		chConfig = &Spi_Global.configPtr->SpiChannelConfig[currChannel];
		bInChar = (chConfig->SpiDataWidth > 8 ) ? 2 : 1;

		/*
		 * Get the number of SpiData-packages that can be copied to the rx-buffer.
		 * Either the rx-buffer size will set the limit or all incoming
		 * data can be copied. The below function MIN(a>>b, c) reads as follows:
		 *		a: remaining buffer size in terms of sizeof(SpiDataType)
		 *		b: adjustment for larger data width (e.g. if we use 2 bytes per character)
		 *		c: size of data in FIFO
		 */
        copyCnt = MIN( (bufLen - jobUnitPtr->rxChCnt) >> ((chConfig->SpiDataWidth-1)/8), jobUnitPtr->fifoSent );
        jobUnitPtr->fifoSent -= copyCnt;

        if( copyCnt == 0  ) {
            return SPIE_BAD;
        }

        /* Pop the FIFO and copy its data into the current channels buffer */
		for (i=0; i<copyCnt; i++) {
			/* Wait until FIFO contains some data */
			while (!(SPI0_CS & SPI_CS_RXD));
			popVal = SPI0_FIFO;

			/* Copy (if there is a buffer) */
			if (buf != NULL) {
				if (bInChar == 2) {
					buf[jobUnitPtr->rxChCnt] = (Spi_DataType) (popVal >> 8);
					buf[jobUnitPtr->rxChCnt + 1] = (Spi_DataType) popVal;
				} else {
					buf[jobUnitPtr->rxChCnt] = (Spi_DataType) popVal;
				}
			}

			/* Step */
			jobUnitPtr->rxChCnt += bInChar;
		}

		/* If this buffers channel is full, advance to the next channel */
		if((bufLen - jobUnitPtr->rxChCnt) == 0) {

            jobUnitPtr->rxChCnt = 0;
            jobUnitPtr->currRxChIndex++;
            currChannel = jobUnitPtr->channelsPtr[jobUnitPtr->currRxChIndex];

            if (currChannel == CH_NOT_VALID) {
                jobUnitPtr->fifoSent = 0;
                jobUnitPtr->currRxChIndex = 0;

                rv = SPIE_OK;
                break;
            }
        }
	}

   	/* Return status */
	return rv;
}

/**
 * Write a job to FIFO
 *
 * @param jobIndex			----- index of the job that should be transmitted through FIFO
 */
static void Spi_WriteJob_FIFO(Spi_JobType jobIndex)
{
	Spi_ChannelType                 currChannel;		// Channel
	const Spi_ChannelConfigType *   chConfig;			// Channel configuration
	const Spi_DataType *            buf;				// Data buffer
    Spi_NumberOfDataType            bufLen;				// Data buffer length
    Spi_JobUnitType *               jobUnitPtr;			// Job
    Spi_NumberOfDataType            copyCnt;			// Nr of characters to copy
    Spi_NumberOfDataType            fifoLeft;			// Nr SpiData left in FIFO
    uint32							bInChar;			// Bytes per character (normally 1, sometimes 2)

	int i;												// Counter

	/* Set chip selection alternative_0 and then activate transfer */
	SPI0_CS &= SPI_CS_CS0;
	SPI0_CS |= SPI_CS_TA;

	jobUnitPtr = &Spi_JobUnit[jobIndex];
	fifoLeft = FIFO_DEPTH;

	/* Iterate over the channels for this job */
    currChannel = jobUnitPtr->channelsPtr[jobUnitPtr->currTxChIndex];

    while (fifoLeft != 0) {
    	/* Get tx-buffer */
    	buf = Spi_GetTxBuf(currChannel, &bufLen);

    	/* Get channel configuration */
		chConfig = &Spi_Global.configPtr->SpiChannelConfig[currChannel];
		bInChar = (chConfig->SpiDataWidth > 8 ) ? 2 : 1;

		/*
		 * Get the number of SpiData-packages that can be written to FIFO.
		 * Either the FIFO size will set the limit or all outgoing
		 * data can be written. The below function MIN(a>>b, c) reads as follows:
		 *		a: remaining number of characters in the tx-buffer, in terms of sizeof(SpiDataType)
		 *		b: adjustment for larger data width (e.g. if we use 2 bytes per character)
		 *		c: remaining space in FIFO
		 */
		copyCnt = MIN( (bufLen - jobUnitPtr->txChCnt) >> ((chConfig->SpiDataWidth-1)/8), fifoLeft );

        /* Push as much as we can to FIFO (FIFO or channel limits) */
        for (i=0; i < copyCnt; i++) {
        	while (!(SPI0_CS & SPI_CS_TXD));		// Wait until FIFO can accept data

            if (buf != NULL) {
                if (chConfig->SpiDataWidth > 8) {
                	SPI0_FIFO = (buf[jobUnitPtr->txChCnt] << 8) + (buf[jobUnitPtr->txChCnt + 1] & 0xff);
                } else {
                	SPI0_FIFO = buf[jobUnitPtr->txChCnt];
                }
            } else {
            	SPI0_FIFO = chConfig->SpiDefaultData;
            }

            /* Step */
            jobUnitPtr->txChCnt += bInChar;

            /* Update the SPI master data length register */
			SPI0_DLEN = jobUnitPtr->txChCnt;

            --fifoLeft;
        }

        // Done with the channel?
        if ((bufLen - jobUnitPtr->txChCnt) == 0) {
            jobUnitPtr->txChCnt = 0;
            jobUnitPtr->currTxChIndex++;

            //Done with the job?
            currChannel = jobUnitPtr->channelsPtr[jobUnitPtr->currTxChIndex];
            if( currChannel == CH_NOT_VALID ) {
                jobUnitPtr->currTxChIndex = 0;
                break;
            }
        }
    }

    jobUnitPtr->fifoSent = FIFO_DEPTH - fifoLeft;

    // Interrupt-driven SPI not implemented
	if ((Spi_Global.asyncMode == SPI_INTERRUPT_MODE)
			&& (jobUnitPtr->unitPtr->callType == SPI_ASYNC_CALL)) {
        //ENABLE_EOQ_INTERRUPT(jobUnitPtr->hwPtr);
		//if this is a interrupt mode, open the interrupt
	} else {}
}

/**
 * Prepare a job for being transmitted and then write it to FIFO
 *
 * @param jobIndex			----- index of the job that should be transmitted
 */
static void Spi_JobWrite(Spi_JobType jobIndex) {
	/* Configure the SPI unit structure to point to the current job */
	Spi_UnitType *uPtr = Spi_JobUnit[jobIndex].unitPtr;

	uPtr->txCurrIndex = 0;
	uPtr->currJob = Spi_JobUnit[jobIndex].jobCfgPtr;
	uPtr->currJobIndex = jobIndex;

	/* Reset rx/tx-counters */
	Spi_JobUnit[jobIndex].txChCnt = 0;
	Spi_JobUnit[jobIndex].rxChCnt = 0;
	Spi_JobUnit[jobIndex].currTxChIndex = 0;

	/* Set appropriate "pending"-statuses */
	Spi_SetHWUnitStatus(uPtr, SPI_BUSY);
	Spi_SetJobResult(jobIndex, SPI_JOB_PENDING);

	/* Write data to FIFO */
	Spi_WriteJob_FIFO(jobIndex);
}

/**
 * Read data on SPI (and if necessary write in order to receive more data)
 *
 * @param spiUnit			----- pointer to the SPI unit
 */
static void Spi_Poll(Spi_UnitType *uPtr) {
	Spi_TransferStatus rv;					// Transfer status

	/* If data is being transmitted, wait for the transfer to complete */
	while(!(SPI0_CS & SPI_CS_DONE));

	/* Read data from FIFO */
    rv = Spi_Rx_FIFO(uPtr);

	if (rv == SPIE_JOB_NOT_DONE) {
		/* Rx FIFO now empty, but the job is not done -> send more */
		Spi_WriteJob_FIFO(uPtr->currJobIndex);
	}
	else if (rv == SPIE_BAD) {
		// Fail both job and sequence
		Spi_SetHWUnitStatus(uPtr, SPI_IDLE);
		Spi_SetJobResult(uPtr->currJob->SpiJobId, SPI_JOB_FAILED);
		Spi_SetSequenceResult(uPtr->currSeqPtr->SpiSequenceId, SPI_SEQ_FAILED);
	}
	else { // rv == SPIE_OK
		Spi_JobType nextJob;

		// The job is done
		Spi_SetJobResult(uPtr->currJob->SpiJobId, SPI_JOB_OK);

		// Step to the next job in this sequence
		nextJob = Spi_GetNextJob(uPtr);
		if (nextJob != JOB_NOT_VALID) {
			Spi_JobWrite(nextJob);
		} else {
			// No more jobs, so set HwUnit and sequence IDLE/OK also.
			Spi_SetHWUnitStatus(uPtr, SPI_IDLE);
			Spi_SetSequenceResult(uPtr->currSeqPtr->SpiSequenceId, SPI_SEQ_OK);

			// Clear the transfer active bit (which sets all CS lines to high)
			SPI0_CS &= ~SPI_CS_TA;
		}
	}
}

/**
 * Write a sequence of jobs to the SPI bus
 *
 * @param seqIndex 			----- sequence index
 * @param sync 				----- 1 - make the call sync; 0 - make the call async
 */
static void Spi_SeqWrite(Spi_SequenceType seqIndex, Spi_CallType sync) {
	const Spi_SequenceConfigType 	*seqConfig;						// Sequence configuration
	const Spi_JobConfigType 		*jobConfig;						// Configuration of the first job in the sequence
	Spi_UnitType 					*uPtr;							// SPI unit pointer
	const Spi_JobType 				*jobPtr;						// Job pointer
	Spi_JobType 					jobIndex;						// Index of the first job in the sequence

	seqConfig 	= Spi_GetSeqPtrFromIndex(seqIndex);
	jobPtr 		= &seqConfig->JobAssignment[0];
	jobIndex 	= *jobPtr;
	jobConfig 	= Spi_GetJobCfgPtrFromIndex(jobIndex);
	uPtr 		= GET_SPI_UNIT_PTR(jobConfig->SpiHwUnit);

	/* Fill in the required fields for job and sequence */
	uPtr->currJobIndexPtr = jobPtr;
	uPtr->callType = sync;
	uPtr->currSeqPtr = seqConfig;

	/* Set sequence status */
	Spi_SetSequenceResult(seqIndex, SPI_SEQ_PENDING);

	/* Queue the jobs in this sequence */
	while (*jobPtr != JOB_NOT_VALID) {
		Spi_SetJobResult(*jobPtr, SPI_JOB_QUEUED);
		jobPtr++;
	}

	// SPI interrupt and asynchronous calls not implemented
	if ((Spi_Global.asyncMode == SPI_INTERRUPT_MODE) &&
				(uPtr->callType == SPI_ASYNC_CALL)) {
	} else {}

	/* Write the first job in this sequence */
	Spi_JobWrite(jobIndex);

	/* If synchronous call, then loop through all the jobs in this sequence */
	if (uPtr->callType == SPI_SYNC_CALL) {
		while (Spi_GetSequenceResult(seqIndex) == SPI_SEQ_PENDING) {
			Spi_Poll(uPtr);
		}
	}
}

/**
 * Initialize SPI-related hardware (SPI registers and GPIO pins)
 */
static void Spi_InitHW(void) {
	/* Set pre-defined SPI pins to alternative_0, i.e. SPI (see p.102 in BCM2835-ARM-Peripherals.pdf) */
	bcm2835_GpioFnSel(GPIO_SPI0_CE1, GPFN_ALT0);
	bcm2835_GpioFnSel(GPIO_SPI0_CE0, GPFN_ALT0);
	bcm2835_GpioFnSel(GPIO_SPI0_MISO, GPFN_ALT0);
	bcm2835_GpioFnSel(GPIO_SPI0_MOSI, GPFN_ALT0);
	bcm2835_GpioFnSel(GPIO_SPI0_CLK, GPFN_ALT0);

	/* Clear rx/tx-FIFOs, leaving remaining bits at 0
	 * (which implicitly means that we don't use bidirectional mode) */
    SPI0_CS = SPI_CS_CLEAR_RX | SPI_CS_CLEAR_TX;

    /* Set the clock divider (which should be a power of 2)
     * (the value is experimentally derived, this one works and gives some CAN speed margins) */
	SPI0_CLK = BCM2835_SPI_CLOCK_DIVIDER_128; 			// => 1.953125MHz

	// Not implemented
    //install the spi isr
    //ISR_INSTALL_ISR2("SPI0",Spi_Isr, BCM2835_IRQ_ID_SPI, 9, 0);
}

/**
 * Transmit a sequence, if everything is in order for its transmission
 *
 * @param Sequence			----- sequence index
 * @param callType			----- call type, sync (1) or async (0)
 */
static Std_ReturnType Spi_Transmit(Spi_SequenceType Sequence, Spi_CallType callType) {
	/* Check that SPI has been initialized and that this is a valid
	 * sequence being in a state ready for transmission.
	 * Neither this sequence, nor any of its jobs should be in a pending state */
	VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ),
			SPI_SYNCTRANSMIT_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );
	VALIDATE_W_RV( ( Sequence < SPI_MAX_SEQUENCE ),
			SPI_SYNCTRANSMIT_SERVICE_ID, SPI_E_PARAM_SEQ, E_NOT_OK );
	VALIDATE_W_RV( ( Spi_GetSequenceResult(Sequence) != SPI_SEQ_PENDING ),
			SPI_SYNCTRANSMIT_SERVICE_ID, SPI_E_SEQ_PENDING, E_NOT_OK );
	VALIDATE_W_RV( ( !Spi_AnyPendingJobs(Sequence) ),
			SPI_SYNCTRANSMIT_SERVICE_ID, SPI_E_SEQ_PENDING, E_NOT_OK );
	assert(Spi_GetSequenceResult(Sequence) == SPI_SEQ_OK);

	/* Write the sequence */
	Spi_SeqWrite(Sequence, callType);

	return E_OK;
}

/****************************************************************************/
/*	 				 		Global functions 							   	*/
/****************************************************************************/

/**
 * Initialize necessary SW structures and call initialization of SPI-related HW
 *
 * @param ConfigPtr			----- pointer to SPI configuration
 */
void Spi_Init(const Spi_ConfigType *ConfigPtr) {
	const Spi_JobConfigType *jobConfig;				// Job configuration
	Spi_UnitType 			*uPtr;					// SPI unit
	uint32 					confMask;				// Configuration mask
	uint8 					ctrlNr;					// Control number

	Spi_Global.configPtr = ConfigPtr;
	Spi_Global.extBufPtr = Spi_Eb;
	Spi_Global.asyncMode = SPI_INTERRUPT_MODE;

	// Set all sequence results to OK
	for (Spi_SequenceType i = (Spi_SequenceType) 0; i < SPI_MAX_SEQUENCE; i++) {
		Spi_SetSequenceResult(i, SPI_SEQ_OK);
	}

	// Set all sequence jobs to OK and store HW unit indices in SPI HW configuration mask
	for (int j = 0; j < Spi_GetJobCnt(); j++) {
		jobConfig = &Spi_Global.configPtr->SpiJobConfig[j];
		Spi_Global.spiHwConfigured |= (1 << jobConfig->SpiHwUnit);
		Spi_SetJobResult(j, SPI_JOB_OK);
	}

	/* Assign each HW-unit that is being used by a job to a unique control number */
	confMask = Spi_Global.spiHwConfigured;
	for (int i = 0; confMask; confMask &= ~(1 << ctrlNr), i++) {
		ctrlNr = ilog2(confMask);					// Get the most significant non-zero bit in HW-conf mask (clear it in the loop condition)
		Spi_CtrlToUnit[ctrlNr] = i;					// Assign id to it

		uPtr = GET_SPI_UNIT_PTR(ctrlNr);			// Store the HW control number in the SPI unit info
		uPtr->hwUnit = ctrlNr;
		Spi_SetHWUnitStatus(uPtr, SPI_IDLE);		// Let the HW be idle
	}

	/* Init HW registers and GPIO pins */
	Spi_InitHW();

	/* Setup relations for Job, for easy access */
	for(int j=0; j<SPI_MAX_JOB; j++ ) {
		jobConfig = &Spi_Global.configPtr->SpiJobConfig[j];
		Spi_JobUnit[j].jobCfgPtr = jobConfig;
		Spi_JobUnit[j].extDeviceCfgPtr = &Spi_Global.configPtr->SpiExternalDevice[jobConfig->DeviceAssignment];
		Spi_JobUnit[j].unitPtr = GET_SPI_UNIT_PTR(jobConfig->SpiHwUnit);
		Spi_JobUnit[j].channelsPtr = &jobConfig->ChannelAssignment[0];
	}

	/* Initialization done */
	Spi_Global.initRun = TRUE;
}

/**
 * De-initialize SPI (not implemented, but perhaps it should be)
 */
Std_ReturnType Spi_DeInit(void) {
	return E_OK;
}

/**
 * Setup an external buffer
 *
 * @param Channel			----- channel index
 * @param SrcDataBufferPtr	----- pointer to source data buffer
 * @param DesDataBufferPtr	----- pointer to destination data buffer
 * @param Length			----- data length
 * @return 					----- result status
 */
Std_ReturnType Spi_SetupEB(	Spi_ChannelType Channel,
							const Spi_DataType* SrcDataBufferPtr,
							Spi_DataType* DesDataBufferPtr,
							Spi_NumberOfDataType Length)
{
	/* Check that some pre-conditions have been fulfilled, such as that
	 * SPI has been initialized, the channel is valid and configured for an external buffer of this length */
	VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ),
			SPI_SETUPEB_SERVICE_ID, SPI_E_UNINIT, E_NOT_OK );
	VALIDATE_W_RV( ( Channel < SPI_MAX_CHANNEL ),
			SPI_SETUPEB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );
	VALIDATE_W_RV( ( SPI_EB == Spi_Global.configPtr->SpiChannelConfig[Channel].SpiChannelType ),
			SPI_SETUPEB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );
	VALIDATE_W_RV( ( Length <= Spi_Global.configPtr->SpiChannelConfig[Channel].SpiEbMaxLength ),
			SPI_SETUPEB_SERVICE_ID, SPI_E_PARAM_CHANNEL, E_NOT_OK );

	/* Get the external buffer for this channel */
	Spi_EbType *extChBuff = &Spi_Global.extBufPtr[Channel];

	/* Configure and activate the external buffer */
	extChBuff->src = SrcDataBufferPtr;
	extChBuff->dest = DesDataBufferPtr;
	extChBuff->length = Length;
	extChBuff->active = 1;

	return E_OK;
}

/**
 * Transmit a sequence synchronously
 *
 * @param Sequence			----- sequence index
 */
Std_ReturnType Spi_SyncTransmit(Spi_SequenceType Sequence) {
	Std_ReturnType rv = Spi_Transmit(Sequence, SPI_SYNC_CALL);
	return rv;
}

/**
 * Transmit a sequence asynchronously
 *
 * @param Sequence			----- sequence index
 */
Std_ReturnType Spi_AsyncTransmit(Spi_SequenceType Sequence) {
	Std_ReturnType rv = Spi_Transmit(Sequence, SPI_ASYNC_CALL);
	return rv;
}

/**
 * Get current result for the transmission of a sequence, e.g. ok, pending, cancelled or failed
 *
 * @param Sequence			----- sequence index
 */
Spi_SeqResultType Spi_GetSequenceResult(Spi_SequenceType Sequence) {
	/* Check that SPI has been initialized and that this is a valid sequence */
	VALIDATE_W_RV( ( TRUE == Spi_Global.initRun ), SPI_GETSEQUENCERESULT_SERVICE_ID, SPI_E_UNINIT, SPI_SEQ_FAILED );
	VALIDATE_W_RV( ( Sequence < SPI_MAX_SEQUENCE ), SPI_GETSEQUENCERESULT_SERVICE_ID, SPI_E_PARAM_SEQ, SPI_SEQ_FAILED );

	return Spi_SeqUnit[Sequence].seqResult;
}
