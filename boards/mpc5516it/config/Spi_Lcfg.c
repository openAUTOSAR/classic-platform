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








#warning "This default file may only be used as an example!"

#include "Spi.h"
#include "Spi_Cfg.h"
#include <stdlib.h>


// SPI_A
#define SPI_A_CS_E2 			2
#define SPI_A_CS_FLASH   1

// SPI_B
#define SPI_B_CS_E2			0


#if 1
extern void Spi_SeqEndNotification( void );
extern void Spi_JobEndNotification( void );
extern void Spi_Test_SeqEndNotification( Spi_SequenceType seq);
extern void Spi_Test_JobEndNotification( Spi_JobType job);

static void Spi_Test_SeqEndNotification_E2_CMD( void  ) { Spi_Test_SeqEndNotification(SPI_SEQ_E2_CMD); }
static void Spi_Test_SeqEndNotification_E2_CMD2( void  ) { Spi_Test_SeqEndNotification(SPI_SEQ_E2_CMD2); }
static void Spi_Test_SeqEndNotification_E2_READ( void  ){ Spi_Test_SeqEndNotification(SPI_SEQ_E2_READ); }
static void Spi_Test_SeqEndNotification_E2_WRITE( void  ){ Spi_Test_SeqEndNotification(SPI_SEQ_E2_WRITE); }

static void Spi_Test_JobEndNotification_E2_CMD( void  ){ Spi_Test_JobEndNotification(SPI_JOB_E2_CMD); }
static void Spi_Test_JobEndNotification_E2_CMD2( void  ){ Spi_Test_JobEndNotification(SPI_JOB_E2_CMD2); }
static void Spi_Test_JobEndNotification_E2_DATA( void  ){ Spi_Test_JobEndNotification(SPI_JOB_E2_DATA); }
static void Spi_Test_JobEndNotification_E2_WREN( void  ){ Spi_Test_JobEndNotification(SPI_JOB_E2_WREN); }

#define SPI_SEQ_END_NOTIFICATION	NULL
#define SPI_JOB_END_NOTIFICAITON	NULL

// Notifications
// Seq
#define SPI_SEQ_E2_CMD_END_NOTIFICATION			Spi_Test_SeqEndNotification_E2_CMD
#define SPI_SEQ_E2_CMD2_END_NOTIFICATION			Spi_Test_SeqEndNotification_E2_CMD2
#define SPI_SEQ_E2_READ_END_NOTIFICATION			Spi_Test_SeqEndNotification_E2_READ
#define SPI_SEQ_E2_WRITE_END_NOTIFICATION		Spi_Test_SeqEndNotification_E2_WRITE
// Jobs
#define SPI_JOB_E2_CMD_END_NOTIFICATION			Spi_Test_JobEndNotification_E2_CMD
#define SPI_JOB_E2_CMD2_END_NOTIFICATION			Spi_Test_JobEndNotification_E2_CMD2
#define SPI_JOB_E2_DATA_END_NOTIFICATION			Spi_Test_JobEndNotification_E2_DATA
#define SPI_JOB_E2_WREN_END_NOTIFICATION			Spi_Test_JobEndNotification_E2_WREN

#else
#define SPI_SEQ_END_NOTIFICATION	NULL
#define SPI_JOB_END_NOTIFICAITON	NULL
#endif


/* SEQUENCES */
const Spi_SequenceConfigType SpiSequenceConfigData[] =
{
  {
  .SpiSequenceId = SPI_SEQ_E2_CMD, // The EEP read sequence
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_E2_CMD_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_E2_CMD,(-1)},
  },
  {
  .SpiSequenceId = SPI_SEQ_E2_CMD2, // The EEP read sequence
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_E2_CMD2_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_E2_CMD2,(-1)},
  },
  {
  .SpiSequenceId = SPI_SEQ_E2_READ, // The EEP read sequence
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_E2_READ_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_E2_DATA,(-1)},
  },
  {
  .SpiSequenceId = SPI_SEQ_E2_WRITE, // The EEP write sequence
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_E2_WRITE_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_E2_DATA,(-1)},
  },

  //   -----------------------------------
  {
  .SpiSequenceId = SPI_SEQ_EEP_CMD, // The EEP read sequence
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_EEP_CMD,(-1)},
  },
  {
  .SpiSequenceId = SPI_SEQ_EEP_CMD2, // The EEP read sequence
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_EEP_CMD2,(-1)},
  },
  {
  .SpiSequenceId = SPI_SEQ_EEP_READ, // The EEP read sequence
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_EEP_DATA,(-1)},
  },
  {
  .SpiSequenceId = SPI_SEQ_EEP_WRITE, // The EEP write sequence
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_EEP_WREN,SPI_JOB_EEP_DATA,(-1)},
  },

  //   -----------------------------------


  {
  .SpiSequenceId = SPI_SEQ_FLASH_CMD, // The EEP write sequence
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_FLASH_CMD,(-1)},
  },
  {
  .SpiSequenceId = SPI_SEQ_FLASH_CMD2, // The EEP read sequence
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_FLASH_CMD2,(-1)},
  },
  {
  .SpiSequenceId = SPI_SEQ_FLASH_CMD_DATA, // The EEP write sequence
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_FLASH_CMD_DATA,(-1)},
  },
  {
  .SpiSequenceId = SPI_SEQ_FLASH_READ, // The EEP write sequence
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_FLASH_READ,(-1)},
  },
  {
  .SpiSequenceId = SPI_SEQ_FLASH_WRITE, // The EEP write sequence
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_FLASH_WREN,SPI_JOB_FLASH_DATA,SPI_JOB_FLASH_WRDI,(-1)},
  },
  {
  .SpiSequenceId = SPI_SEQ_FLASH_WRSR, // The EEP write sequence
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_FLASH_WREN,SPI_JOB_FLASH_WRSR,(-1)},
  },
  {
  .SpiSequenceId = SPI_SEQ_FLASH_ERASE, // The EEP write sequence
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_FLASH_WREN,SPI_JOB_FLASH_ADDR,SPI_JOB_FLASH_WRDI,(-1)},
  },
};

/* JOBS */
const Spi_JobConfigType SpiJobConfigData[] =
{
  {
  .SpiJobId = SPI_JOB_E2_CMD, // The command job for EEP
  .SpiHwUnit = DSPI_CTRL_A,
  .SpiJobPriority = 3,
  .SpiJobEndNotification = SPI_JOB_E2_CMD_END_NOTIFICATION,
  .DeviceAssignment = SPI_EXT_DEVICE_A_E2,
  .ChannelAssignment = { SPI_CH_E2_CMD ,(-1)},
  },
  {
  .SpiJobId = SPI_JOB_E2_CMD2, // The command job for EEP
  .SpiHwUnit = DSPI_CTRL_A,
  .SpiJobPriority = 3,
  .SpiJobEndNotification = SPI_JOB_E2_CMD2_END_NOTIFICATION,
  .DeviceAssignment = SPI_EXT_DEVICE_A_E2,
  .ChannelAssignment = { SPI_CH_E2_CMD ,SPI_CH_E2_DATA,(-1)},
  },
  {
  .SpiJobId = SPI_JOB_E2_DATA, // The data job for EEP
  .SpiHwUnit = DSPI_CTRL_A,
  .SpiJobPriority = 2,
  .SpiJobEndNotification = SPI_JOB_E2_DATA_END_NOTIFICATION,
  .DeviceAssignment = SPI_EXT_DEVICE_A_E2,
  .ChannelAssignment = { SPI_CH_E2_CMD, SPI_CH_E2_ADDR ,SPI_CH_E2_DATA,(-1)},
  },
  {
	.SpiJobId = SPI_JOB_E2_WREN, // The data job for EEP
	.SpiHwUnit = DSPI_CTRL_A,
	.SpiJobPriority = 2,
	.SpiJobEndNotification = SPI_JOB_E2_WREN_END_NOTIFICATION,
	.DeviceAssignment = SPI_EXT_DEVICE_A_E2,
	.ChannelAssignment = { SPI_CH_E2_WREN,(-1)},
	},

  //   -----------------------------------

  {
  .SpiJobId = SPI_JOB_EEP_CMD, // The command job for EEP
  .SpiHwUnit = DSPI_CTRL_B,
  .SpiJobPriority = 3,
  .SpiJobEndNotification = SPI_JOB_END_NOTIFICAITON,
  .DeviceAssignment = SPI_EXT_DEVICE_B_E2,
  .ChannelAssignment = { SPI_CH_EEP_CMD ,(-1)},
  },
  {
  .SpiJobId = SPI_JOB_EEP_CMD2, // The command job for EEP
  .SpiHwUnit = DSPI_CTRL_B,
  .SpiJobPriority = 3,
  .SpiJobEndNotification = SPI_JOB_END_NOTIFICAITON,
  .DeviceAssignment = SPI_EXT_DEVICE_B_E2,
  .ChannelAssignment = { SPI_CH_EEP_CMD ,SPI_CH_EEP_DATA,(-1)},
  },
  {
  .SpiJobId = SPI_JOB_EEP_DATA, // The data job for EEP
  .SpiHwUnit = DSPI_CTRL_B,
  .SpiJobPriority = 2,
  .SpiJobEndNotification = SPI_JOB_END_NOTIFICAITON,
  .DeviceAssignment = SPI_EXT_DEVICE_B_E2,
  .ChannelAssignment = { SPI_CH_EEP_CMD, SPI_CH_EEP_ADDR ,SPI_CH_EEP_DATA,(-1)},
  },
  {
	.SpiJobId = SPI_JOB_EEP_WREN, // The data job for EEP
	.SpiHwUnit = DSPI_CTRL_B,
	.SpiJobPriority = 2,
	.SpiJobEndNotification = SPI_JOB_END_NOTIFICAITON,
	.DeviceAssignment = SPI_EXT_DEVICE_B_E2,
	.ChannelAssignment = { SPI_CH_EEP_WREN,(-1)},
	},


  //   -----------------------------------
  // 8
  {
  .SpiJobId = SPI_JOB_FLASH_CMD, // The command job for EEP
  .SpiHwUnit = DSPI_CTRL_A,
  .SpiJobPriority = 3,
  .SpiJobEndNotification = SPI_JOB_END_NOTIFICAITON,
  .DeviceAssignment = SPI_EXT_DEVICE_A_FLASH,
  .ChannelAssignment = { SPI_CH_FLASH_CMD ,(-1)},
  },
  {
  .SpiJobId = SPI_JOB_FLASH_CMD2, // The command job for EEP
  .SpiHwUnit = DSPI_CTRL_A,
  .SpiJobPriority = 3,
  .SpiJobEndNotification = SPI_JOB_END_NOTIFICAITON,
  .DeviceAssignment = SPI_EXT_DEVICE_A_FLASH,
  .ChannelAssignment = { SPI_CH_FLASH_CMD ,SPI_CH_FLASH_DATA, (-1)},
  },
  {
  .SpiJobId = SPI_JOB_FLASH_CMD_DATA, // The data job for EEP
  .SpiHwUnit = DSPI_CTRL_A,
  .SpiJobPriority = 2,
  .SpiJobEndNotification = SPI_JOB_END_NOTIFICAITON,
  .DeviceAssignment = SPI_EXT_DEVICE_A_FLASH,
  .ChannelAssignment = { SPI_CH_FLASH_CMD, SPI_CH_FLASH_DATA,(-1)},
  },
  {
  .SpiJobId = SPI_JOB_FLASH_READ, // The data job for EEP
  .SpiHwUnit = DSPI_CTRL_A,
  .SpiJobPriority = 2,
  .SpiJobEndNotification = SPI_JOB_END_NOTIFICAITON,
  .DeviceAssignment = SPI_EXT_DEVICE_A_FLASH,
  .ChannelAssignment = { SPI_CH_FLASH_CMD, SPI_CH_FLASH_ADDR, SPI_CH_FLASH_DATA,(-1)},
  },
  {
	.SpiJobId = SPI_JOB_FLASH_WREN,
	.SpiHwUnit = DSPI_CTRL_A,
	.SpiJobPriority = 2,
	.SpiJobEndNotification = SPI_JOB_END_NOTIFICAITON,
	.DeviceAssignment = SPI_EXT_DEVICE_A_FLASH,
	.ChannelAssignment = { SPI_CH_FLASH_WREN,(-1)},
	},
  {
	.SpiJobId = SPI_JOB_FLASH_WRDI,
	.SpiHwUnit = DSPI_CTRL_A,
	.SpiJobPriority = 2,
	.SpiJobEndNotification = SPI_JOB_END_NOTIFICAITON,
	.DeviceAssignment = SPI_EXT_DEVICE_A_FLASH,
	.ChannelAssignment = { SPI_CH_FLASH_WRDI,(-1)},
	},
  {
  .SpiJobId = SPI_JOB_FLASH_DATA,
  .SpiHwUnit = DSPI_CTRL_A,
  .SpiJobPriority = 2,
  .SpiJobEndNotification = 0,
  .DeviceAssignment = SPI_EXT_DEVICE_A_FLASH,
  .ChannelAssignment = { SPI_CH_FLASH_CMD, SPI_CH_FLASH_ADDR ,SPI_CH_FLASH_DATA,(-1)},
  },
  {
  .SpiJobId = SPI_JOB_FLASH_WRSR,
  .SpiHwUnit = DSPI_CTRL_A,
  .SpiJobPriority = 2,
  .SpiJobEndNotification = 0,
  .DeviceAssignment = SPI_EXT_DEVICE_A_FLASH,
  .ChannelAssignment = { SPI_CH_FLASH_WRSR, SPI_CH_FLASH_DATA ,(-1)},
  },
  {
  .SpiJobId = SPI_JOB_FLASH_ADDR,
  .SpiHwUnit = DSPI_CTRL_A,
  .SpiJobPriority = 2,
  .SpiJobEndNotification = 0,
  .DeviceAssignment = SPI_EXT_DEVICE_A_FLASH,
  .ChannelAssignment = { SPI_CH_FLASH_CMD, SPI_CH_FLASH_ADDR,(-1)},
  },
};

uint32 Spi_GetJobCnt(void ) { return sizeof(SpiJobConfigData)/sizeof(SpiJobConfigData[0]); }


/* CHANNELS */
const Spi_ChannelConfigType SpiChannelConfigData[] =
{
  {
  .SpiChannelId = SPI_CH_E2_CMD,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 8,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = SPI_EB_MAX_LENGTH,
  .SpiDefaultData = 0x00,
  .SpiDmaNoIncreaseSrc = 0,
  },
  {
  .SpiChannelId = SPI_CH_E2_ADDR,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 16,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = SPI_EB_MAX_LENGTH,
  .SpiDefaultData = 0x0000,
  .SpiDmaNoIncreaseSrc = 0,
  },
  {
  .SpiChannelId = SPI_CH_E2_WREN,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 8,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = SPI_EB_MAX_LENGTH,
  .SpiDefaultData = 0x00,
  .SpiDmaNoIncreaseSrc = 0,
  },

  {
  .SpiChannelId = SPI_CH_E2_DATA,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 8,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = SPI_EB_MAX_LENGTH,
  .SpiDefaultData = 0x00,
  .SpiDmaNoIncreaseSrc = 0,
  },

  //   -----------------------------------

  {
  .SpiChannelId = SPI_CH_EEP_CMD,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 8,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = SPI_EB_MAX_LENGTH,
  .SpiDefaultData = 0x00,
  .SpiDmaNoIncreaseSrc = 0,
  },
  {
  .SpiChannelId = SPI_CH_EEP_ADDR,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 16,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = SPI_EB_MAX_LENGTH,
  .SpiDefaultData = 0x0000,
  .SpiDmaNoIncreaseSrc = 0,
  },
  {
  .SpiChannelId = SPI_CH_EEP_WREN,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 8,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = 1,
  .SpiDefaultData = E2_WREN,
  .SpiDmaNoIncreaseSrc = 0,
  },

  {
  .SpiChannelId = SPI_CH_EEP_DATA,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 8,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = SPI_EB_MAX_LENGTH,
  .SpiDefaultData = 0x00,
  .SpiDmaNoIncreaseSrc = 0,
  },

  //   -----------------------------------

  {
  .SpiChannelId = SPI_CH_FLASH_CMD,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 8,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = SPI_EB_MAX_LENGTH,
  .SpiDefaultData = 0x00,
  .SpiDmaNoIncreaseSrc = 0,
  },
  {
  .SpiChannelId = SPI_CH_FLASH_ADDR,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 8,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = SPI_EB_MAX_LENGTH,
  .SpiDefaultData = 0x0000,
  .SpiDmaNoIncreaseSrc = 0,
  },
  {
  .SpiChannelId = SPI_CH_FLASH_DATA,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 8,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = SPI_EB_MAX_LENGTH,
  .SpiDefaultData = 0x00,
  .SpiDmaNoIncreaseSrc = 0,
  },
  {
  .SpiChannelId = SPI_CH_FLASH_WREN,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 8,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = 1,
  .SpiDefaultData = FLASH_WREN,
  .SpiDmaNoIncreaseSrc = 0,
  },
  {
  .SpiChannelId = SPI_CH_FLASH_WRDI,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 8,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = 1,
  .SpiDefaultData = FLASH_WRDI,
  .SpiDmaNoIncreaseSrc = 0,
  },
  {
  .SpiChannelId = SPI_CH_FLASH_WRSR,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 8,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = 1,
  .SpiDefaultData = FLASH_WRSR,
  .SpiDmaNoIncreaseSrc = 0,
  },

  {
      .SpiChannelId = (-1),
  }
};

uint32 Spi_GetChanneCnt(void ) { return sizeof(SpiChannelConfigData)/sizeof(SpiChannelConfigData[0]); }

const Spi_ExternalDeviceType SpiExternalConfigData[] =
{
	// SPI_EXT_DEVICE_A_E2

  // E2
  {
    .SpiBaudrate = 1000000UL,
    .SpiCsIdentifier = SPI_A_CS_E2,
    .SpiCsPolarity = STD_LOW,
    .SpiDataShiftEdge = SPI_EDGE_LEADING,
    .SpiEnableCs = 1,
    .SpiShiftClockIdleLevel = STD_LOW,
    .SpiTimeClk2Cs = 606,   // ns
    .SpiTimeCs2Clk = 606,   // ns
  },
  // SPI_EXT_DEVICE_A_FLASH
  {
    .SpiBaudrate = 1000000UL,
    .SpiCsIdentifier = SPI_A_CS_FLASH,
    .SpiCsPolarity = STD_LOW,
    .SpiDataShiftEdge = SPI_EDGE_LEADING,
    .SpiEnableCs = 1,
    .SpiShiftClockIdleLevel = STD_LOW,
    .SpiTimeClk2Cs = 606,   // ns
    .SpiTimeCs2Clk = 606,   // ns
  },

	//
	// SPI_BUS_B
	//

	// SPI_EXT_DEVICE_B_E2
	{
		.SpiBaudrate = 1000000UL,
		.SpiCsIdentifier = SPI_B_CS_E2,
		.SpiCsPolarity = STD_LOW,
		.SpiDataShiftEdge = SPI_EDGE_LEADING,
		.SpiEnableCs = 1,
		.SpiShiftClockIdleLevel = STD_LOW,
		.SpiTimeClk2Cs = 0,
		.SpiTimeCs2Clk = 0,
	},
};

uint32 Spi_GetExternalDeviceCnt(void ) { return sizeof(SpiExternalConfigData)/sizeof(SpiExternalConfigData[0]); }

const Spi_HwConfigType SpiHwConfig[] =
{
  {
    .IsrPriority = 1,
    .Activated = TRUE,
    .RxDmaChannel = DMA_DSPI_A_RESULT_CHANNEL,
    .TxDmaChannel = DMA_DSPI_A_COMMAND_CHANNEL,
    .PeripheralClock = PERIPHERAL_CLOCK_DSPI_A,
  },
  {
    .IsrPriority = 1,
    .Activated = TRUE,
    .RxDmaChannel = DMA_DSPI_B_RESULT_CHANNEL,
    .TxDmaChannel = DMA_DSPI_B_COMMAND_CHANNEL,
    .PeripheralClock = PERIPHERAL_CLOCK_DSPI_B,
  },
  {
    .IsrPriority = 1,
    .Activated = FALSE,
    .RxDmaChannel = 0,
    .TxDmaChannel = 0,
    .PeripheralClock = 0,
  },
  {
    .IsrPriority = 1,
    .Activated = TRUE,
    .RxDmaChannel = 0,
    .TxDmaChannel = 0,
    .PeripheralClock = 0,
  },
};

const Spi_DriverType SpiConfigData =
{
  .SpiMaxChannel = SPI_MAX_CHANNEL,
  .SpiMaxJob = SPI_MAX_JOB,
  .SpiMaxSequence = SPI_MAX_SEQUENCE,
  .SpiChannelConfig = &SpiChannelConfigData[0],
  .SpiSequenceConfig = &SpiSequenceConfigData[0],
  .SpiJobConfig = &SpiJobConfigData[0],
  .SpiExternalDevice = &SpiExternalConfigData[0],
  .SpiHwConfig = &SpiHwConfig[0],
};

