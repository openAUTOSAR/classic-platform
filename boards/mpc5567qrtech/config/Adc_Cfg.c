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








#include "Adc.h"
#include "Dma.h"
#include "mpc5516.h"

const struct tcd_t AdcGroupDMACommandConfig [ADC_NBR_OF_GROUPS];
const struct tcd_t AdcGroupDMAResultConfig [ADC_NBR_OF_GROUPS];

Adc_GroupStatus AdcGroupStatus[ADC_NBR_OF_GROUPS];

/* Configuration goes here. */
void Adc_Group0Notification (void)
{
}

void Adc_Group1Notification (void)
{
}

const Adc_HWConfigurationType AdcHWUnitConfiguration =
{
  .hwUnitId = 0,
  .adcPrescale = ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_8,
  .clockSource = ADC_SYSTEM_CLOCK,
};

const Adc_ChannelConfigurationType AdcChannelConfiguration [ADC_NBR_OF_CHANNELS] =
{
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_DISABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
  { ADC_CONVERSION_TIME_8_CLOCKS, 10, 0, ADC_RESOLUTION_12BITS, ADC_CALIBRATION_ENABLED},
};

const Adc_ChannelType Adc_Group0ChannelList[ADC_NBR_OF_GROUP0_CHANNELS] =
{
  ADC_CH1, ADC_CH2, ADC_CH3, ADC_CH4, ADC_CH5, ADC_CH6, ADC_CH7
};

const Adc_ChannelType Adc_Group1ChannelList[ADC_NBR_OF_GROUP1_CHANNELS] =
{
    ADC_CH0, ADC_CH2, ADC_CH3, ADC_CH4
};

const Adc_ChannelType Adc_Group2ChannelList[ADC_NBR_OF_GROUP2_CHANNELS] =
{
    ADC_CH0, ADC_CH1, ADC_CH2
};

const Adc_ChannelType Adc_Group3ChannelList[ADC_NBR_OF_GROUP3_CHANNELS] =
{
    ADC_CH0, ADC_CH1, ADC_CH2
};

/* Ram buffers for command and result queues. These are located here in the
   configuration to be able to reconfigure system without recompiling the
   drivers. */
Adc_ValueGroupType Adc_Group0Buffer [sizeof(Adc_Group0ChannelList)/sizeof(Adc_Group0ChannelList[0])];
Adc_CommandType Adc_Group0Commands [sizeof(Adc_Group0ChannelList)/sizeof(Adc_Group0ChannelList[0])];

Adc_ValueGroupType Adc_Group1Buffer [sizeof(Adc_Group1ChannelList)/sizeof(Adc_Group1ChannelList[0])];
Adc_CommandType Adc_Group1Commands [sizeof(Adc_Group1ChannelList)/sizeof(Adc_Group1ChannelList[0])];

Adc_ValueGroupType Adc_Group2Buffer [sizeof(Adc_Group2ChannelList)/sizeof(Adc_Group2ChannelList[0])];
Adc_CommandType Adc_Group2Commands [sizeof(Adc_Group2ChannelList)/sizeof(Adc_Group2ChannelList[0])];

Adc_ValueGroupType Adc_Group3Buffer [sizeof(Adc_Group3ChannelList)/sizeof(Adc_Group3ChannelList[0])];
Adc_CommandType Adc_Group3Commands [sizeof(Adc_Group3ChannelList)/sizeof(Adc_Group3ChannelList[0])];


const Adc_GroupDefType AdcGroupConfiguration [] =
{
   { .accessMode        = ADC_ACCESS_MODE_SINGLE, .conversionMode = ADC_CONV_MODE_ONESHOT,
     .triggerSrc        = ADC_TRIGG_SRC_SW, .hwTriggerSignal = ADC_NO_HW_TRIG,
     .hwTriggerTimer    = ADC_NO_TIMER,
     .groupCallback     = Adc_Group0Notification,
     .streamBufferMode  = ADC_NO_STREAMING, .streamNumSamples = 0,
     .channelList       = Adc_Group0ChannelList,
     .resultBuffer      = Adc_Group0Buffer,
     .commandBuffer     = Adc_Group0Commands,
     .numberOfChannels  = sizeof(Adc_Group0Commands)/sizeof(Adc_Group0Commands[0]),
     .status            = &AdcGroupStatus[ADC_GROUP0],
     .dmaCommandChannel = DMA_ADC_GROUP0_COMMAND_CHANNEL,
     .dmaResultChannel  = DMA_ADC_GROUP0_RESULT_CHANNEL,
     .groupDMACommands  = &AdcGroupDMACommandConfig[ADC_GROUP0],
     .groupDMAResults   = &AdcGroupDMAResultConfig[ADC_GROUP0]},

   { .accessMode        = ADC_ACCESS_MODE_SINGLE, .conversionMode = ADC_CONV_MODE_CONTINOUS,
     .triggerSrc        = ADC_TRIGG_SRC_SW, .hwTriggerSignal = ADC_NO_HW_TRIG,
     .hwTriggerTimer    = ADC_NO_TIMER,
     .groupCallback     = Adc_Group1Notification,
     .streamBufferMode  = ADC_NO_STREAMING, .streamNumSamples = 0,
     .channelList       = Adc_Group1ChannelList,
     .resultBuffer      = Adc_Group1Buffer,
     .commandBuffer     = Adc_Group1Commands,
     .numberOfChannels  = sizeof(Adc_Group1Commands)/sizeof(Adc_Group1Commands[0]),
     .status            = &AdcGroupStatus[ADC_GROUP1],
     .dmaCommandChannel = DMA_ADC_GROUP1_COMMAND_CHANNEL,
     .dmaResultChannel  = DMA_ADC_GROUP1_RESULT_CHANNEL,
     .groupDMACommands  = &AdcGroupDMACommandConfig[ADC_GROUP1],
     .groupDMAResults   = &AdcGroupDMAResultConfig[ADC_GROUP1]}
};


/******************************************************************/
/*                                                                */
/* End of user configuration area. DO NOT modify the code below!! */
/*                                                                */
/******************************************************************/
const Adc_ConfigType AdcConfig [] =
{
  {
   .hwConfigPtr      = &AdcHWUnitConfiguration,
   .channelConfigPtr = AdcChannelConfiguration,
   .nbrOfChannels    = sizeof(AdcChannelConfiguration)/sizeof(AdcChannelConfiguration[0]),
   .groupConfigPtr   = AdcGroupConfiguration,
   .nbrOfGroups      = sizeof(AdcGroupConfiguration)/sizeof(AdcGroupConfiguration[0])}
};

/* DMA configuration. */
const struct tcd_t AdcGroupDMACommandConfig [ADC_NBR_OF_GROUPS] =
{
  {
    .SADDR = (uint32_t)Adc_Group0Commands,
    .SMOD = 0,
    .SSIZE = DMA_TRANSFER_SIZE_32BITS,
    .DMOD = 0,
    .DSIZE = DMA_TRANSFER_SIZE_32BITS,
    .SOFF = sizeof(Adc_CommandType),
    .NBYTES = sizeof(Adc_CommandType),
    .SLAST = -sizeof(Adc_Group0Commands),
    .DADDR = (vint32_t)&EQADC.CFPR[ADC_GROUP0].R,
    .CITERE_LINK = 0,
    .CITER = sizeof(Adc_Group0Commands)/sizeof(Adc_Group0Commands[0]),
    .DOFF = 0,
    .DLAST_SGA = 0,
    .BITERE_LINK = 0,
    .BITER = sizeof(Adc_Group0Commands)/sizeof(Adc_Group0Commands[0]),
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
  },
  {
    .SADDR = (uint32_t)Adc_Group1Commands,
    .SMOD = 0,
    .SSIZE = DMA_TRANSFER_SIZE_32BITS,
    .DMOD = 0,
    .DSIZE = DMA_TRANSFER_SIZE_32BITS,
    .SOFF = sizeof(Adc_CommandType),
    .NBYTES = sizeof(Adc_CommandType),
    .SLAST = -sizeof(Adc_Group1Commands),
    .DADDR = (vint32_t)&EQADC.CFPR[ADC_GROUP1].R,
    .CITERE_LINK = 0,
    .CITER = sizeof(Adc_Group1Commands)/sizeof(Adc_Group1Commands[0]),
    .DOFF = 0,
    .DLAST_SGA = 0,
    .BITERE_LINK = 0,
    .BITER = sizeof(Adc_Group1Commands)/sizeof(Adc_Group1Commands[0]),
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
  }
};

const struct tcd_t AdcGroupDMAResultConfig [ADC_NBR_OF_GROUPS]=
{
  {
	  .SADDR = (vint32_t)&EQADC.RFPR[ADC_GROUP0].R + 2,
    .SMOD = 0,
    .SSIZE = DMA_TRANSFER_SIZE_16BITS,
    .DMOD = 0,
    .DSIZE = DMA_TRANSFER_SIZE_16BITS,
    .SOFF = 0,
    .NBYTES = sizeof(Adc_ValueGroupType),
    .SLAST = 0,
    .DADDR = (uint32_t)Adc_Group0Buffer,
    .CITERE_LINK = 0,
    .CITER = sizeof(Adc_Group0Buffer)/sizeof(Adc_Group0Buffer[0]),
    .DOFF = sizeof(Adc_ValueGroupType),
    .DLAST_SGA = -sizeof(Adc_Group0Buffer),
    .BITERE_LINK = 0,
    .BITER = sizeof(Adc_Group0Buffer)/sizeof(Adc_Group0Buffer[0]),
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
  },
  {
    .SADDR = (vint32_t)&EQADC.RFPR[ADC_GROUP1].R + 2,
    .SMOD = 0,
    .SSIZE = DMA_TRANSFER_SIZE_16BITS,
    .DMOD = 0,
    .DSIZE = DMA_TRANSFER_SIZE_16BITS,
    .SOFF = 0,
    .NBYTES = sizeof(Adc_ValueGroupType),
    .SLAST = 0,
    .DADDR = (uint32_t)Adc_Group1Buffer,
    .CITERE_LINK = 0,
    .CITER = sizeof(Adc_Group1Buffer)/sizeof(Adc_Group1Buffer[0]),
    .DOFF = sizeof(Adc_ValueGroupType),
    .DLAST_SGA = -sizeof(Adc_Group1Buffer),
    .BITERE_LINK = 0,
    .BITER = sizeof(Adc_Group1Buffer)/sizeof(Adc_Group1Buffer[0]),
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
}
};
