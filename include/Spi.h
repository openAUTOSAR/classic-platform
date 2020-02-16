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

#ifndef SPI_H_
#define SPI_H_

#include "Std_Types.h"
#include "Modules.h"


/* STD container : SpiGeneral
 * SPI_CANCEL_API					1	  Bool
 * SPI_CHANNEL_BUFFERS_ALLOWED		1	  Int 0..2
 * SPI_DEV_ERROR_DETECT				1	  Bool
 * SPI_HW_STATUS_API				1	  Bool
 * SPI_INTERRUPTABLE_SEQ_ALLOWED	1	  Bool
 * SPI_LEVEL_DELIVERED				1     Int 0..2
 * SPI_VERSION_INFO_API				1     Bool
 */

/* SPI container: SpiSequence
 * SpiInterruptableSequence			1     Bool
 * SpiSeqEndNotification 			1     Func
 * SpiSequenceId					1     Int     (name of the sequence)
 * JobAssignment					1..*  Ref to SpiJob
 */

/* SPI container: SpiChannel
 * SpiChannelId						1     Int ( name of the channel )
 * SpiChannelType					1	  enum IB, EB
 * SpiDataWidth						1	  Int, 1..32
 * SpiDefaultData					1     Int
 * SpiEbMaxLength					1	  Int
 * SpiHwUnitSynchronous				0..1  			[New in 4.0]
 * SpiIbNBuffers 					1	  Int
 * SpiTransferStart 				1     Enum LSB, MSB
 */

/* SPI container: SpiJob
 * SpiHwUnit						1     Enum, CSIB0,CSIB1,CSIB2,CSIB3
 * SpiJobEndNotification 			1	  Func
 * SpiJobId 						1	  Int ( name of job )
 * SpiJobPriority					1	  Int 0..3
 * ChannelAssignment 				1..*  Ref to channel
 * DeviceAssignment					1	  Ref to exteral device
 */

/* SPI container: SpiExternalDevice
 * SpiBaudrate						1     float
 * SpiCsIdentifier					1	  String
 * SpiCsPolarity 					1     enum, HIGH, LOW
 * SpiDataShiftEdge					1	  enum LEADING, TRAILING
 * SpiEnableCs 						1	  Bool
 * SpiShiftClockIdleLevel			1	  Enum, HIGH, LOW
 * SpiTimeClk2Cs 					1     float
 */

/* SPI container: SpiDriver
 * SpiMaxChannel					0..1  Int
 * SpiMaxJob 						0..1  Int
 * SpiMaxSequence 					0..1  Int
 * SpiChannel[c]					1..*  Channel Data
 * SpiExternalDevice[c]				1..*  External device data
 * SpiJob[c]						1..*  Job data
 * SpiSequence[c]					1..*  Sequence data.
  */


/* Standard info */
#define SPI_VENDOR_ID             VENDOR_ID_ARCCORE
#define SPI_MODULE_ID         		 MODULE_ID_SPI
#define SPI_SW_MAJOR_VERSION      1
#define SPI_SW_MINOR_VERSION      0
#define SPI_SW_PATCH_VERSION      0
/* Part of release 3.0 rev 0001 */
#define SPI_AR_MAJOR_VERSION      3
#define SPI_AR_MINOR_VERSION      1
#define SPI_AR_PATCH_VERSION      5


/* --- Service IDs --- */
#define SPI_INIT_SERVICE_ID               0x00
#define SPI_DEINIT_SERVICE_ID             0x01
#define SPI_WRITEIB_SERVICE_ID            0x02
#define SPI_ASYNCTRANSMIT_SERVICE_ID      0x03
#define SPI_READIB_SERVICE_ID             0x04
#define SPI_SETUPEB_SERVICE_ID            0x05
#define SPI_GETSTATUS_SERVICE_ID          0x06
#define SPI_GETJOBRESULT_SERVICE_ID       0x07
#define SPI_GETSEQUENCERESULT_SERVICE_ID  0x08
#define SPI_GETVERSIONINFO_SERVICE_ID     0x09
#define SPI_SYNCTRANSMIT_SERVICE_ID       0x0A
#define SPI_GETHWUNITSTATUS_SERVICE_ID    0x0B
#define SPI_CANCEL_SERVICE_ID             0x0C
#define SPI_SETASYNCMODE_SERVICE_ID       0x0D

/* --- Error codes --- */
#define SPI_E_PARAM_CHANNEL               0x0A
#define SPI_E_PARAM_JOB                   0x0B
#define SPI_E_PARAM_SEQ                   0x0C
#define SPI_E_PARAM_LENGTH                0x0D
#define SPI_E_PARAM_UNIT                  0x0E
#define SPI_E_UNINIT                      0x1A
#define SPI_E_SEQ_PENDING                 0x2A
#define SPI_E_SEQ_IN_PROCESS              0x3A
#define SPI_E_ALREADY_INITIALIZED         0x4A



#if 1
#define CH_NOT_VALID 	(0xff)
#define JOB_NOT_VALID	(0xffff)
#define SEQ_NOT_VALID	(0xff)

/* Types according to standard */
typedef uint8_t Spi_ChannelType ;
typedef uint16_t Spi_JobType;
typedef uint8_t Spi_SequenceType;
#else

#define CH_NOT_VALID 	(-1)
#define JOB_NOT_VALID	(-1)
#define SEQ_NOT_VALID	(-1)

typedef uint8_t Spi_ChannelType ;
typedef uint16_t Spi_JobType;
typedef uint8_t Spi_SequenceType;

#endif

#define SPI_SIMPLE         0   /* Not implemented, NOT TESTED */
#define SPI_FIFO           1
#define SPI_DMA            2

#define SPI_EB_MAX_LENGTH 64

#define CSIB0	0
#define CSIB1   1
#define CSIB2   2
#define CSIB3   3

typedef enum {
	SPI_UNINIT=0, // The SPI Handler/Driver is not initialized or not usable.
				// SPI011: This shall be the default value after reset. This
    			// status shall have the value 0.

	SPI_IDLE,	// The SPI Handler/Driver is not currently transmitting any
    			// Job.

	SPI_BUSY, 	// The SPI Handler/Driver is performing a SPI Job( transmit )
} Spi_StatusType;

typedef enum {
	SPI_JOB_OK=0, 		// The last transmission of the Job has been finished
    					// successfully.
    					// SPI012: This shall be the default value after reset.
    					// This status shall have the value 0.

	SPI_JOB_PENDING,	// The SPI Handler/Driver is performing a SPI Job.
						// The meaning of this status is equal to SPI_BUSY.

	SPI_JOB_FAILED, 	// The last transmission of the Job has failed.

	SPI_JOB_QUEUED, 	// Job accepted but not started.

} Spi_JobResultType;

typedef enum {

	SPI_SEQ_OK, 		//	The last transmission of the Sequence has been
						//    finished successfully.
						//    SPI017: This shall be the default value after reset.
						//    This status shall have the value 0.

	SPI_SEQ_PENDING, 	// The SPI Handler/Driver is performing a SPI
    					// Sequence. The meaning of this status is equal to
    					// SPI_BUSY.

	SPI_SEQ_FAILED, 	// The last transmission of the Sequence has failed.

	SPI_SEQ_CANCELLED, 	// The last transmission of the Sequence has been
						//    cancelled by user.
} Spi_SeqResultType;


// Type for defining the number of data elements of the type Spi_DataType to
// send and / or receive by Channel
typedef uint16 Spi_NumberOfDataType;


// Specifies the asynchronous mechanism mode for SPI busses handled
// asynchronously in LEVEL 2.
// SPI150: This type is available or not accordint to the pre compile time parameter:
// SPI_LEVEL_DELIVERED. This is only relevant for LEVEL 2.

typedef enum {

	//	The asynchronous mechanism is ensured by
	//    polling, so interrupts related to SPI busses
	//    handled asynchronously are disabled.
	SPI_POLLING_MODE,

	//    The asynchronous mechanism is ensured by
	//    interrupt, so interrupts related to SPI busses
	//    handled asynchronously are enabled.

	SPI_INTERRUPT_MODE,
} Spi_AsyncModeType;

typedef enum
{
  SPI_EB = 0, // External Buffer
  SPI_IB // Internal Buffer
} Spi_BufferType;

typedef enum
{
  SPI_TRANSFER_START_LSB,
  SPI_TRANSFER_START_MSB,
} Spi_TransferStartType;


typedef enum {
	SPI_EDGE_LEADING,
	SPI_EDGE_TRAILING
} Spi_EdgeType;


#include "Spi_Cfg.h"
#if (SPI_IMPLEMENTATION==SPI_DMA)
#include "Dma.h"
#endif

// All data needed to configure one SPI-channel
typedef struct Spi_ChannelConfig
{
  // Symbolic name
  Spi_ChannelType SpiChannelId;
  // Buffer usage with EB/IB channel
  Spi_BufferType SpiChannelType;

  // This parameter is the width of a transmitted data unit.
  uint32 SpiDataWidth;
  // This parameter is the default value to transmit.
  uint32 SpiDefaultData;

  // This parameter contains the maximum size of data buffers in case of EB
  // Channels and only.
  Spi_NumberOfDataType SpiEbMaxLength;

  // This parameter contains the maximum number of data buffers in case of IB
  // Channels and only.
  Spi_NumberOfDataType SpiIbNBuffers;

  // This parameter defines the first starting bit for transmission.
  Spi_TransferStartType SpiTransferStart;

  //
//  _Bool SpiDmaNoIncreaseSrc;

} Spi_ChannelConfigType;

// All data needed to configure one SPI-Job, amongst others the connection
// between the internal SPI unit and the special settings for an external de-
// vice is done.
typedef struct Spi_JobConfig
{

  Spi_JobType SpiJobId;

  //	This parameter is the symbolic name to identify the HW SPI Hardware micro-
  //	controller peripheral allocated to this Job.
  uint32 SpiHwUnit;

  // This parameter is a reference to a notification function.
  void (*SpiJobEndNotification)();

  // Priority of the Job
  // range 0..3
  unsigned SpiJobPriority;

  // A job references several channels. TODO: Optimize this...
  Spi_ChannelType ChannelAssignment[SPI_MAX_CHANNEL+1];

  // Reference to the external device used by this job
  Spi_ExternalDeviceTypeType DeviceAssignment;

//	unsigned 	SPI_NUMBER_OF_CHANNELS;
//	unsigned	SPI_LIST_OF_CHANNELS[SPI_MAX_CHANNEL];
} Spi_JobConfigType;

// The communication settings of an external device. Closely linked to Spi-
// Job.
typedef struct Spi_ExternalDevice
{

  // This parameter is the communication baudrate - This parameter allows
  // using a range of values, from the point of view of configuration tools, from
  // Hz up to MHz.
  // Note! Float in config case, not here
  uint32 SpiBaudrate;

  // Symbolic name to identify the CS used for this job
  uint32 SpiCsIdentifier;

  // This parameter defines the active polarity of Chip Select.
  // STD_HIGH or STD_LOW
  uint8 SpiCsPolarity;

  // This parameter defines the SPI data shift edge.
  Spi_EdgeType SpiDataShiftEdge;

  // This parameter enables or not the Chip Select handling functions.
  uint8 SpiEnableCs;

  // This parameter defines the SPI shift clock idle level.
  uint8 SpiShiftClockIdleLevel;

  // Timing between clock and chip select - This parameter allows to use a
  // range of values from 0 up to 100 microSec. the real configuration-value
  // used in software BSW-SPI is calculated out of this by the generator-tools
  // Note! Float in config case, not here. Unit ns
  uint32 SpiTimeClk2Cs;

  // Timing between PCS and first edge of SCK. Unit ns.
  uint32 SpiTimeCs2Clk;

  // ArcCore extension...
  // The controller ID(0..3)
  //uint32 SpiControllerId;
  void (*SpiCsCallback)(int);

} Spi_ExternalDeviceType;

// All data needed to configure one SPI-sequence
typedef struct Spi_SequenceConfig
{
  // This parameter allows or not this Sequence to be suspended by another
  // one.
  unsigned SpiInterruptibleSequence;
  // This parameter is a reference to a notification function.
  void (*SpiSeqEndNotification)();
  //
  Spi_SequenceType SpiSequenceId;
  //	unsigned			SPI_NUMBER_OF_JOBS;
  // A sequence references several jobs, which are executed during a commu-
  // nication sequence
  Spi_JobType JobAssignment[SPI_MAX_JOB+1];
} Spi_SequenceConfigType;

typedef struct Spi_HwConfig
{
  /* Interrupt priority level for this SPI channel. */
//  uint8 IsrPriority;

  /* This channel is to be activated for use. */
  uint8 Activated;

  /* Receive DMA channel. */
#if (SPI_IMPLEMENTATION==SPI_DMA)
  Dma_ChannelType RxDmaChannel;

  /* Transmit DMA channel. */
  Dma_ChannelType TxDmaChannel;
#endif

  /* Peripheral clock source. */
//  McuE_PeriperalClock_t PeripheralClock;
}Spi_HwConfigType;


#if 0
struct SpiDriverConfiguration_s
{
  Spi_ChannelType SPI_MAX_CHANNEL;
  Spi_JobType SPI_MAX_JOB;
  Spi_SequenceType SPI_MAX_SEQUENCE;
};
#endif

// This is implementation specific but not all values may be valid
// within the type.This type shall be chosen in order to have the
// most efficient implementation on a specific microcontroller
// platform.
// In-short: Type of application data buffer elements
// The 5516 TXDATA is 16-bit.. fits ?

typedef uint8 Spi_DataType;
//typedef uint16 Spi_DataType;

// Specifies the identification (ID) for a SPI Hardware microcontroller peripheral (unit).
// SPI140: This type is configurable (On / Off) at pre-compile time. The switch
// SPI_HW_STATUS_API shall activate or deactivate the declaration of this
// type.
typedef uint8 Spi_HWUnitType;

#if 0
typedef struct
{
  Spi_SequenceConfigType SpiSequenceConfig;
  Spi_JobConfigType SpiJobConfig;
  Spi_ChannelConfigType SpiChannelConfig;
  Spi_ExternalDeviceType SpiExternalDevice;
}Spi_ConfigType;
#endif


typedef struct Spi_Driver
{
  //	This parameter contains the number of Channels configured. It will be
  //	gathered by tools during the configuration stage.
  uint8 SpiMaxChannel;

  uint8 SpiMaxJob;

  uint8 SpiMaxSequence;

  // All data needed to configure one SPI-channel
  const struct Spi_ChannelConfig * SpiChannelConfig;

  // The communication settings of an external device. Closely
  // linked to SpiJob.
  const struct Spi_ExternalDevice * SpiExternalDevice;

  //	All data needed to configure one SPI-Job, amongst others the
  //	connection between the internal SPI unit and the special set-
  //	tings for an external device is done.
  const struct Spi_JobConfig * SpiJobConfig;

  // All data needed to configure one SPI-sequence
  const struct Spi_SequenceConfig * SpiSequenceConfig;

//  const struct Spi_HwConfig *SpiHwConfig;
} Spi_DriverType;

typedef Spi_DriverType Spi_ConfigType;


void Spi_Init( const Spi_ConfigType *ConfigPtr );
Std_ReturnType Spi_DeInit( void );
Std_ReturnType Spi_WriteIB( Spi_ChannelType Channel, const Spi_DataType *DataBufferPtr );
Std_ReturnType Spi_AsyncTransmit( Spi_SequenceType Sequence );
Std_ReturnType Spi_ReadIB( Spi_ChannelType Channel, Spi_DataType *const DataBufferPtr ) ;
Std_ReturnType Spi_SetupEB( Spi_ChannelType Channel,
							const Spi_DataType*  SrcDataBufferPtr,
	     					Spi_DataType* DesDataBufferPtr,
	     					Spi_NumberOfDataType Length );

Spi_StatusType Spi_GetStatus( void );
Spi_JobResultType Spi_GetJobResult ( Spi_JobType       Job );
Spi_SeqResultType Spi_GetSequenceResult(Spi_SequenceType Sequence );

#if ( SPI_VERSION_INFO_API == STD_ON  )
#define Spi_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,SPI)
#endif

Std_ReturnType Spi_SyncTransmit( Spi_SequenceType        Sequence );
Spi_StatusType Spi_GetHWUnitStatus(Spi_HWUnitType        HWUnit);

#if (SPI_CANCEL_API == STD_ON )
void Spi_Cancel( Spi_SequenceType Sequence );
#endif

Std_ReturnType Spi_SetAsyncMode( Spi_AsyncModeType        Mode );
void Spi_MainFunction_Handling( void );
void Spi_MainFunction_Driving( void );

extern const Spi_ConfigType SpiConfigData;


#endif /*SPI_H_*/
