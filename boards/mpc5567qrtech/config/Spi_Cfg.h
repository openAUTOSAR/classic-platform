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








#ifndef SPI_CFG_H_
#define SPI_CFG_H_

#include "Dma.h"
#include "Mcu.h"

#define DSPI_CTRL_A	0
#define DSPI_CTRL_B	1
#define DSPI_CTRL_C	2
#define DSPI_CTRL_D	3

/*
 * General configuration
 */

// Maximum amount of data that can be written/read in one go.
#define SPI_EB_MAX_LENGTH					64

// Switches the Spi_Cancel function ON or OFF.
#define SPI_CANCEL_API						STD_OFF

// Selects the SPI Handler/Driver Channel Buffers usage allowed and delivered.
// LEVEL 0 - Only Internal buffers
// LEVEL 1 - Only external buffers
// LEVEL 2 - Both internal/external buffers
#define SPI_CHANNEL_BUFFERS_ALLOWED	1

#define SPI_DEV_ERROR_DETECT 			STD_ON
// Switches the Spi_GetHWUnitStatus function ON or OFF.
#define SPI_HW_STATUS_API					STD_ON
// Switches the Interruptible Sequences handling functionality ON or OFF.
#define SPI_INTERRUPTIBLE_SEQ_ALLOWED		STD_OFF

// LEVEL 0 - Simple sync
// LEVEL 1 - Basic async
// LEVEL 2 - Enhanced mode
#define SPI_LEVEL_DELIVERED				2

#define SPI_VERSION_INFO_API				STD_ON

#if 0
#if SPI_LEVEL_DELIVERED>=1
#define SPI_INTERRUPTIBLE_SEQ_ALLOWED	STD_ON
#endif
#endif

// M95256
#define E2_WREN   0x6		// Write Enable 0000 0110
#define E2_WRDI   0x4		// Write Disable 0000 0100
#define E2_RDSR   0x5		// Read Status Register  0000 0101
													// 1 - Read data
#define E2_WRSR   0x1		// Write Status Register  0000 0001
													// 1 - Write data
#define E2_READ   0x3		// Read from Memory Array 0000 0011
													// 1  - Write 16-bit address
													// n  - 8 -bit read data
#define E2_WRITE  0x2		// WRITE  Write to Memory Array  0000 0010
													// 1  Write 16-bit address
													// n  - 8-bit reads


#define FLASH_READ_25			0x03
#define FLASH_READ_50			0x0B
#define FLASH_RDSR					0x05
#define FLASH_JEDEC_ID			0x9f
#define FLASH_RDID					0x90
#define FLASH_BYTE_WRITE 	0x02
#define FLASH_AI_WORD_WRITE 0xad
#define FLASH_WREN					0x06
#define FLASH_WRDI					0x04
#define FLASH_WRSR					0x01
#define FLASH_ERASE_4K			0x20



typedef enum
{
  SPI_EB = 0, // External Buffer
  SPI_IB // Internal Buffer
} Spi_BufferType;

typedef enum {
	SPI_EXT_DEVICE_A_E2,
	SPI_EXT_DEVICE_A_FLASH,
	SPI_EXT_DEVICE_B_E2,
} Spi_ExternalDeviceTypeType;

typedef enum
{
	SPI_CH_E2_CMD = 0,
	SPI_CH_E2_ADDR,
	SPI_CH_E2_WREN,
	SPI_CH_E2_DATA,

	SPI_CH_EEP_CMD,
	SPI_CH_EEP_ADDR,
	SPI_CH_EEP_WREN,
	SPI_CH_EEP_DATA,

	SPI_CH_FLASH_CMD,
	SPI_CH_FLASH_ADDR,
	SPI_CH_FLASH_DATA,
	SPI_CH_FLASH_WREN,
	SPI_CH_FLASH_WRDI,
	SPI_CH_FLASH_WRSR,

  SPI_MAX_CHANNEL,
} Spi_ChannelType;

typedef enum
{
	SPI_JOB_E2_CMD = 0,
	SPI_JOB_E2_CMD2,
	SPI_JOB_E2_DATA,
	SPI_JOB_E2_WREN,

	SPI_JOB_EEP_CMD,
	SPI_JOB_EEP_CMD2,
	SPI_JOB_EEP_DATA,
	SPI_JOB_EEP_WREN,

	SPI_JOB_FLASH_CMD,
	SPI_JOB_FLASH_CMD2,
	SPI_JOB_FLASH_CMD_DATA,
	SPI_JOB_FLASH_READ,
	SPI_JOB_FLASH_WREN,
	SPI_JOB_FLASH_WRDI,
	SPI_JOB_FLASH_DATA,
	SPI_JOB_FLASH_WRSR,
	SPI_JOB_FLASH_ADDR,

  SPI_MAX_JOB,
} Spi_JobType;

#define SPI_MAX_CHANNELS	8

typedef enum
{
	SPI_SEQ_E2_CMD = 0,
	SPI_SEQ_E2_CMD2,
	SPI_SEQ_E2_READ,
	SPI_SEQ_E2_WRITE,

	SPI_SEQ_EEP_CMD,
	SPI_SEQ_EEP_CMD2,
	SPI_SEQ_EEP_READ,
	SPI_SEQ_EEP_WRITE,

	SPI_SEQ_FLASH_CMD,
	SPI_SEQ_FLASH_CMD2,
	SPI_SEQ_FLASH_CMD_DATA,
	SPI_SEQ_FLASH_READ,
	SPI_SEQ_FLASH_WRITE,
	SPI_SEQ_FLASH_WRSR,
	SPI_SEQ_FLASH_ERASE,

  SPI_MAX_SEQUENCE,
} Spi_SequenceType;

typedef enum
{
  SPI_ARC_TRANSFER_START_LSB,
  SPI_ARC_TRANSFER_START_MSB,
} Spi_Arc_TransferStartType;


typedef enum {
	SPI_EDGE_LEADING,
	SPI_EDGE_TRAILING
} Spi_EdgeType;



// All data needed to configure one SPI-channel
typedef struct
{
  // Symbolic name
  Spi_ChannelType SpiChannelId;
  // Buffer usage with EB/IB channel
  // TODO: The type is wrong...
  unsigned SpiChannelType;

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
  Spi_Arc_TransferStartType SpiTransferStart;

  //
  _Bool SpiDmaNoIncreaseSrc;

} Spi_ChannelConfigType;

// All data needed to configure one SPI-Job, amongst others the connection
// between the internal SPI unit and the special settings for an external de-
// vice is done.
typedef struct
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

  // A job references several channels.
  uint32 ChannelAssignment[SPI_MAX_CHANNELS];

  // Reference to the external device used by this job
  Spi_ExternalDeviceTypeType DeviceAssignment;

//	unsigned 	SPI_NUMBER_OF_CHANNELS;
//	unsigned	SPI_LIST_OF_CHANNELS[SPI_MAX_CHANNEL];
} Spi_JobConfigType;

// The communication settings of an external device. Closely linked to Spi-
// Job.
typedef struct
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

} Spi_ExternalDeviceType;

// All data needed to configure one SPI-sequence
typedef struct
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
  uint32 JobAssignment[SPI_MAX_JOB];
} Spi_SequenceConfigType;

typedef struct
{
  /* Interrupt priority level for this SPI channel. */
  uint8 IsrPriority;

  /* This channel is to be activated for use. */
  uint8 Activated;

  /* Receive DMA channel. */
  Dma_ChannelType RxDmaChannel;

  /* Transmit DMA channel. */
  Dma_ChannelType TxDmaChannel;

  /* Peripheral clock source. */
  McuE_PeriperalClock_t PeripheralClock;
}Spi_HwConfigType;

typedef struct
{
  //	This parameter contains the number of Channels configured. It will be
  //	gathered by tools during the configuration stage.
  uint8 SpiMaxChannel;

  uint8 SpiMaxJob;

  uint8 SpiMaxSequence;

  // All data needed to configure one SPI-channel
  const Spi_ChannelConfigType * SpiChannelConfig;

  // The communication settings of an external device. Closely
  // linked to SpiJob.
  const Spi_ExternalDeviceType * SpiExternalDevice;

  //	All data needed to configure one SPI-Job, amongst others the
  //	connection between the internal SPI unit and the special set-
  //	tings for an external device is done.
  const Spi_JobConfigType * SpiJobConfig;

  // All data needed to configure one SPI-sequence
  const Spi_SequenceConfigType * SpiSequenceConfig;

  const Spi_HwConfigType *SpiHwConfig;
} Spi_DriverType;

typedef Spi_DriverType Spi_ConfigType;


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
typedef uint32 Spi_HWUnitType;

#if 0
typedef struct
{
  Spi_SequenceConfigType SpiSequenceConfig;
  Spi_JobConfigType SpiJobConfig;
  Spi_ChannelConfigType SpiChannelConfig;
  Spi_ExternalDeviceType SpiExternalDevice;
}Spi_ConfigType;
#endif

extern const Spi_ConfigType SpiConfigData;


uint32 Spi_GetJobCnt(void );
uint32 Spi_GetChannelCnt(void );
uint32 Spi_GetExternalDeviceCnt(void );



#endif /*SPI_CFG_H_*/
