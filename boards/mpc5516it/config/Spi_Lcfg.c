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
#define SPI_SEQ_CMD_END_NOTIFICATION		NULL
#define SPI_SEQ_WRITE_END_NOTIFICATION		NULL
#define SPI_SEQ_READ_END_NOTIFICATION		NULL
#define SPI_SEQ_CMD2_END_NOTIFICATION		NULL
// Jobs
#define SPI_JOB_CMD2_END_NOTIFICATION		NULL
#define SPI_JOB_DATA_END_NOTIFICATION		NULL
#define SPI_JOB_CMD_END_NOTIFICATION		NULL
#define SPI_JOB_WREN_END_NOTIFICATION		NULL


/*************** Sequences **************/
const Spi_SequenceConfigType SpiSequenceConfigData[] =
{
 {
  .SpiSequenceId = SPI_SEQ_CMD,
  .SpiInterruptibleSequence = false,
  .SpiSeqEndNotification = SPI_SEQ_CMD_END_NOTIFICATION,
  .JobAssignment = {        
   SPI_JOB_CMD,
   (-1)
   },
    },
 {
  .SpiSequenceId = SPI_SEQ_WRITE,
  .SpiInterruptibleSequence = false,
  .SpiSeqEndNotification = SPI_SEQ_WRITE_END_NOTIFICATION,
  .JobAssignment = {        
   SPI_JOB_WREN,
   SPI_JOB_DATA,
   (-1)
   },
    },
 {
  .SpiSequenceId = SPI_SEQ_READ,
  .SpiInterruptibleSequence = false,
  .SpiSeqEndNotification = SPI_SEQ_READ_END_NOTIFICATION,
  .JobAssignment = {        
   SPI_JOB_DATA,
   (-1)
   },
    },
 {
  .SpiSequenceId = SPI_SEQ_CMD2,
  .SpiInterruptibleSequence = false,
  .SpiSeqEndNotification = SPI_SEQ_CMD2_END_NOTIFICATION,
  .JobAssignment = {        
   SPI_JOB_CMD2,
   (-1)
   },
    },
};

/*************** Jobs **************/
const Spi_JobConfigType SpiJobConfigData[] =
{
 {
  .SpiJobId = SPI_JOB_CMD2,
  .SpiHwUnit = CSIB0,
  .SpiJobPriority = 0,//NOT CONFIGURABLE IN TOOLS
  .SpiJobEndNotification = SPI_JOB_CMD2_END_NOTIFICATION,
  .ChannelAssignment = {
    SPI_CH_CMD,
    SPI_CH_DATA,
   (-1)
   },
  .DeviceAssignment = SPI_device_1,
    },
 {
  .SpiJobId = SPI_JOB_DATA,
  .SpiHwUnit = CSIB0,
  .SpiJobPriority = 0,//NOT CONFIGURABLE IN TOOLS
  .SpiJobEndNotification = SPI_JOB_DATA_END_NOTIFICATION,
  .ChannelAssignment = {
    SPI_CH_CMD,
    SPI_CH_ADDR,
    SPI_CH_DATA,
   (-1)
   },
  .DeviceAssignment = SPI_device_1,
    },
 {
  .SpiJobId = SPI_JOB_CMD,
  .SpiHwUnit = CSIB0,
  .SpiJobPriority = 0,//NOT CONFIGURABLE IN TOOLS
  .SpiJobEndNotification = SPI_JOB_CMD_END_NOTIFICATION,
  .ChannelAssignment = {
    SPI_CH_CMD,
   (-1)
   },
  .DeviceAssignment = SPI_device_1,
    },
 {
  .SpiJobId = SPI_JOB_WREN,
  .SpiHwUnit = CSIB0,
  .SpiJobPriority = 0,//NOT CONFIGURABLE IN TOOLS
  .SpiJobEndNotification = SPI_JOB_WREN_END_NOTIFICATION,
  .ChannelAssignment = {
    SPI_CH_WREN,
   (-1)
   },
  .DeviceAssignment = SPI_device_1,
    },
};

uint32 Spi_GetJobCnt(void ) { return sizeof(SpiJobConfigData)/sizeof(SpiJobConfigData[0]); }


/*************** Channels **************/
const Spi_ChannelConfigType SpiChannelConfigData[] =
{
 {
  .SpiChannelId = SPI_CH_WREN,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 8,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = 1,  
  .SpiDefaultData = 6,
  .SpiTransferStart = SPI_TRANSFER_START_MSB,
 },
 {
  .SpiChannelId = SPI_CH_CMD,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 8,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = 64,  
  .SpiDefaultData = 0,
  .SpiTransferStart = SPI_TRANSFER_START_MSB,
 },
 {
  .SpiChannelId = SPI_CH_DATA,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 8,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = 64,  
  .SpiDefaultData = 0,
  .SpiTransferStart = SPI_TRANSFER_START_MSB,
 },
 {
  .SpiChannelId = SPI_CH_ADDR,
  .SpiChannelType = SPI_EB,
  .SpiDataWidth = 16,
  .SpiIbNBuffers = 0,
  .SpiEbMaxLength = 64,  
  .SpiDefaultData = 0,
  .SpiTransferStart = SPI_TRANSFER_START_MSB,
 },
 {
   .SpiChannelId = (-1),
 }
};

uint32 Spi_GetChanneCnt(void ) { return sizeof(SpiChannelConfigData)/sizeof(SpiChannelConfigData[0]); }

/*************** External Devices **************/
const Spi_ExternalDeviceType SpiExternalConfigData[] =
{
 {
  .SpiBaudrate = 100000UL,
  .SpiCsIdentifier = 2,
  .SpiCsPolarity = STD_LOW,
  .SpiDataShiftEdge = SPI_EDGE_LEADING,
  .SpiEnableCs = 0, // NOT SUPPORTED IN TOOLS
  .SpiShiftClockIdleLevel = STD_LOW,
  .SpiTimeClk2Cs = 606,   // ns
  .SpiTimeCs2Clk = 606,   // ns
 },
};

uint32 Spi_GetExternalDeviceCnt(void ) { return sizeof(SpiExternalConfigData)/sizeof(SpiExternalConfigData[0]); }



const Spi_ConfigType SpiConfigData =
{
  .SpiMaxChannel = SPI_MAX_CHANNEL,
  .SpiMaxJob = SPI_MAX_JOB,
  .SpiMaxSequence = SPI_MAX_SEQUENCE,
  .SpiChannelConfig = &SpiChannelConfigData[0],
  .SpiSequenceConfig = &SpiSequenceConfigData[0],
  .SpiJobConfig = &SpiJobConfigData[0],
  .SpiExternalDevice = &SpiExternalConfigData[0],
};

