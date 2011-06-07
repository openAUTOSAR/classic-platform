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


#ifndef SPI_CONFIGTYPES_H_
#define SPI_CONFIGTYPES_H_


typedef struct Spi_IsrInfo {
	void (*entry)(void);
	IrqType vector;
	uint8_t priority;
	Cpu_t cpu;
} Spi_IsrInfoType;


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

struct Spi_ChannelConfig;
struct Spi_ExternalDevice;
struct Spi_JobConfig;
struct Spi_SequenceConfig;
struct Spi_HwConfig;

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

  const struct Spi_HwConfig *SpiHwConfig;
} Spi_DriverType;

typedef Spi_DriverType Spi_ConfigType;


#endif /* SPI_CONFIGTYPES_H_ */
