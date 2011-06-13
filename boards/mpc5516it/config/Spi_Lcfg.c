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

#include "Spi.h"
#include "Spi_Cfg.h"
#include <stdlib.h>



// SPI_0
//#define SPI_0_CS		1	/* Using PCSB1 */
#define SPI_0_CS		2	/* Using PCSB2 */


#define SPI_SEQ_END_NOTIFICATION	NULL
#define SPI_JOB_END_NOTIFICAITON	NULL

// Notifications
// Seq
#define SPI_SEQ_E2_CMD_END_NOTIFICATION			NULL
#define SPI_SEQ_E2_CMD2_END_NOTIFICATION		NULL
#define SPI_SEQ_E2_READ_END_NOTIFICATION		NULL
#define SPI_SEQ_E2_WRITE_END_NOTIFICATION		NULL
// Jobs
#define SPI_JOB_E2_CMD_END_NOTIFICATION			NULL
#define SPI_JOB_E2_CMD2_END_NOTIFICATION		NULL
#define SPI_JOB_E2_DATA_END_NOTIFICATION		NULL
#define SPI_JOB_E2_WREN_END_NOTIFICATION		NULL


/* SEQUENCES */
const Spi_SequenceConfigType SpiSequenceConfigData[] =
{
  {
  .SpiSequenceId = SPI_SEQ_E2_CMD,
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_E2_CMD_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_E2_CMD,(-1)},
  },
  {
  .SpiSequenceId = SPI_SEQ_E2_CMD2,
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_E2_CMD2_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_E2_CMD2,(-1)},
  },
  {
  .SpiSequenceId = SPI_SEQ_E2_READ,
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_E2_READ_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_E2_DATA,(-1)},
  },
  {
  .SpiSequenceId = SPI_SEQ_E2_WRITE,
  .SpiInterruptibleSequence = 0,
  .SpiSeqEndNotification = SPI_SEQ_E2_WRITE_END_NOTIFICATION,
  .JobAssignment = { SPI_JOB_E2_DATA,(-1)},
  },

};

/* JOBS */
const Spi_JobConfigType SpiJobConfigData[] =
{
  {
    .SpiJobId = SPI_JOB_E2_CMD,
    .SpiHwUnit = DSPI_CTRL_A,
    .SpiJobPriority = 3,
    .SpiJobEndNotification = SPI_JOB_E2_CMD_END_NOTIFICATION,
    .DeviceAssignment = SPI_EXT_DEVICE_A_E2,
    .ChannelAssignment = { SPI_CH_E2_CMD ,(-1)},
  },
  {
    .SpiJobId = SPI_JOB_E2_CMD2,
    .SpiHwUnit = DSPI_CTRL_A,
    .SpiJobPriority = 3,
    .SpiJobEndNotification = SPI_JOB_E2_CMD2_END_NOTIFICATION,
    .DeviceAssignment = SPI_EXT_DEVICE_A_E2,
    .ChannelAssignment = { SPI_CH_E2_CMD ,SPI_CH_E2_DATA,(-1)},
  },
  {
    .SpiJobId = SPI_JOB_E2_DATA,
    .SpiHwUnit = DSPI_CTRL_A,
    .SpiJobPriority = 2,
    .SpiJobEndNotification = SPI_JOB_E2_DATA_END_NOTIFICATION,
    .DeviceAssignment = SPI_EXT_DEVICE_A_E2,
    .ChannelAssignment = { SPI_CH_E2_CMD, SPI_CH_E2_ADDR ,SPI_CH_E2_DATA,(-1)},
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
  },
  {
  .SpiChannelId = SPI_CH_E2_ADDR,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 16,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = SPI_EB_MAX_LENGTH,
  .SpiDefaultData = 0x0000,
  },
  {
  .SpiChannelId = SPI_CH_E2_WREN,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 8,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = SPI_EB_MAX_LENGTH,
  .SpiDefaultData = 0x00,
  },

  {
  .SpiChannelId = SPI_CH_E2_DATA,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 8,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = SPI_EB_MAX_LENGTH,
  .SpiDefaultData = 0x00,
  },

  {
    .SpiChannelId = (-1),
  }
};

uint32 Spi_GetChanneCnt(void ) { return sizeof(SpiChannelConfigData)/sizeof(SpiChannelConfigData[0]); }

const Spi_ExternalDeviceType SpiExternalConfigData[] =
{

  // E2
  {
    .SpiBaudrate = 1000000UL,
    .SpiCsIdentifier = SPI_0_CS,
    .SpiCsPolarity = STD_LOW,
    .SpiDataShiftEdge = SPI_EDGE_LEADING,
    .SpiEnableCs = 1,
    .SpiShiftClockIdleLevel = STD_LOW,
    .SpiTimeClk2Cs = 606,   // ns
    .SpiTimeCs2Clk = 606,   // ns
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
    .Activated = FALSE,
  }
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

