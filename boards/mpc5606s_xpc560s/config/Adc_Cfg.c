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

#include "Adc.h"
#include "Adc_Cfg.h"
#include "Dma.h"


/* Configuration goes here. */
void Adc_Group0Notification (void)
{
}

void Adc_Group1Notification (void)
{
}
void Adc_Group2Notification (void)
{
}

void Adc_Group3Notification (void)
{
}

const Adc_HWConfigurationType AdcHWUnitConfiguration =
{
  .hwUnitId = 0,
  .adcPrescale = ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_2,
  .clockSource = ADC_SYSTEM_CLOCK,
};

const Adc_ChannelConfigurationType AdcChannelConfiguration [ADC_NBR_OF_CHANNELS] =
{
  { ADC_REFERENCE_VOLTAGE_5V, ADC_REFERENCE_VOLTAGE_GROUND, ADC_RESOLUTION_10BITS, ADC_CALIBRATION_DISABLED},
  { ADC_REFERENCE_VOLTAGE_5V, ADC_REFERENCE_VOLTAGE_GROUND, ADC_RESOLUTION_10BITS, ADC_CALIBRATION_DISABLED},
  { ADC_REFERENCE_VOLTAGE_5V, ADC_REFERENCE_VOLTAGE_GROUND, ADC_RESOLUTION_10BITS, ADC_CALIBRATION_DISABLED},
  { ADC_REFERENCE_VOLTAGE_5V, ADC_REFERENCE_VOLTAGE_GROUND, ADC_RESOLUTION_10BITS, ADC_CALIBRATION_DISABLED},
  { ADC_REFERENCE_VOLTAGE_5V, ADC_REFERENCE_VOLTAGE_GROUND, ADC_RESOLUTION_10BITS, ADC_CALIBRATION_DISABLED},
  { ADC_REFERENCE_VOLTAGE_5V, ADC_REFERENCE_VOLTAGE_GROUND, ADC_RESOLUTION_10BITS, ADC_CALIBRATION_DISABLED},
  { ADC_REFERENCE_VOLTAGE_5V, ADC_REFERENCE_VOLTAGE_GROUND, ADC_RESOLUTION_10BITS, ADC_CALIBRATION_DISABLED},
  { ADC_REFERENCE_VOLTAGE_5V, ADC_REFERENCE_VOLTAGE_GROUND, ADC_RESOLUTION_10BITS, ADC_CALIBRATION_DISABLED},
  { ADC_REFERENCE_VOLTAGE_5V, ADC_REFERENCE_VOLTAGE_GROUND, ADC_RESOLUTION_10BITS, ADC_CALIBRATION_DISABLED},
  { ADC_REFERENCE_VOLTAGE_5V, ADC_REFERENCE_VOLTAGE_GROUND, ADC_RESOLUTION_10BITS, ADC_CALIBRATION_DISABLED},
  { ADC_REFERENCE_VOLTAGE_5V, ADC_REFERENCE_VOLTAGE_GROUND, ADC_RESOLUTION_10BITS, ADC_CALIBRATION_DISABLED},
  { ADC_REFERENCE_VOLTAGE_5V, ADC_REFERENCE_VOLTAGE_GROUND, ADC_RESOLUTION_10BITS, ADC_CALIBRATION_DISABLED},
  { ADC_REFERENCE_VOLTAGE_5V, ADC_REFERENCE_VOLTAGE_GROUND, ADC_RESOLUTION_10BITS, ADC_CALIBRATION_DISABLED},
  { ADC_REFERENCE_VOLTAGE_5V, ADC_REFERENCE_VOLTAGE_GROUND, ADC_RESOLUTION_10BITS, ADC_CALIBRATION_DISABLED},
  { ADC_REFERENCE_VOLTAGE_5V, ADC_REFERENCE_VOLTAGE_GROUND, ADC_RESOLUTION_10BITS, ADC_CALIBRATION_DISABLED},
  { ADC_REFERENCE_VOLTAGE_5V, ADC_REFERENCE_VOLTAGE_GROUND, ADC_RESOLUTION_10BITS, ADC_CALIBRATION_DISABLED},

};

//TODO: isoft restriction - curretnly we support only continuous channel setting
const Adc_ChannelType Adc_Group0ChannelList[ADC_NBR_OF_GROUP0_CHANNELS] =
{
    ADC_CH0, ADC_CH1, ADC_CH2,
};

const Adc_ChannelType Adc_Group1ChannelList[ADC_NBR_OF_GROUP1_CHANNELS] =
{
    ADC_CH2, ADC_CH3, ADC_CH4, ADC_CH5,
};

const Adc_ChannelType Adc_Group2ChannelList[ADC_NBR_OF_GROUP2_CHANNELS] =
{
    ADC_CH0, 
};

const Adc_ChannelType Adc_Group3ChannelList[ADC_NBR_OF_GROUP3_CHANNELS] =
{
    ADC_CH0, ADC_CH1, ADC_CH2, ADC_CH3, ADC_CH4, ADC_CH5, ADC_CH6, ADC_CH7, 
    ADC_CH8, ADC_CH9, ADC_CH10, ADC_CH11, ADC_CH12, ADC_CH13, ADC_CH14, ADC_CH15,
};

/* DMA configuration. */
Dma_TcdType AdcGroupDMAResultConfig[] =
{
  { .SADDR = (uint32)&ADC_0.CDR[32].R + 2,    /* Source Address */
    .SMOD = 0,      /* Source modulo feature not used */
    .SSIZE = DMA_TRANSFER_SIZE_16BITS,      /* Read 2**2 = 2 byte per transfer */
    .DMOD = 0,      /* Destination modulo feature not used */
    .DSIZE = DMA_TRANSFER_SIZE_16BITS,      /* Write 2**2 = 2 byte per transfer */
    .SOFF = 4,      /* After transfer, increment 4 = sizeof(chanel data register) */
    .NBYTESu.R = 2,    /* Transfer 2 byte per minor loop */
    .SLAST = -(4 * ADC_NBR_OF_GROUP0_CHANNELS),     /* After major loop jump to the beginning */
    .DADDR = NULL,     /* Destination Address, to be changed later */
    .CITERE_LINK = 0,   /* Linking disabled */
    .CITER = ADC_NBR_OF_GROUP0_CHANNELS,   /* Initialize current iteraction count */
    .DOFF = 2,     /* Do increment destination addr */
    .DLAST_SGA = 0,     /* After major loop continue destination addr */
    .BITERE_LINK = 0,
    .BITER = ADC_NBR_OF_GROUP0_CHANNELS,
    .BWC = 0,       /* Default bandwidth control- no stalls */
    .MAJORLINKCH = 0,   /* Linking disabled */
    .DONE = 0,
    .ACTIVE = 0,
    .MAJORE_LINK = 0,
    .E_SG = 0,
    .D_REQ = 0,     /* Disable channel when major loop is done (sending the buffer once) */
    .INT_HALF = 0,
    .INT_MAJ = 0,
    .START = 0
  },
  
  { .SADDR = (uint32)&ADC_0.CDR[34].R + 2,    /* Source Address*/
    .SMOD = 0,      /* Source modulo feature not used */
    .SSIZE = DMA_TRANSFER_SIZE_16BITS,      /* Read 2**2 = 2 byte per transfer */
    .DMOD = 0,      /* Destination modulo feature not used */
    .DSIZE = DMA_TRANSFER_SIZE_16BITS,      /* Write 2**2 = 2 byte per transfer */
    .SOFF = 4,      /* After transfer, increment 4 = sizeof(chanel data register) */
    .NBYTESu.R = 2,    /* Transfer 2 byte per minor loop */
    .SLAST = -(4 * ADC_NBR_OF_GROUP1_CHANNELS),     /* After major loop jump to the beginning */
    .DADDR = NULL,     /* Destination Address, to be changed later */
    .CITERE_LINK = 0,   /* Linking disabled */
    .CITER = ADC_NBR_OF_GROUP1_CHANNELS,   /* Initialize current iteraction count */
    .DOFF = 2,     /* Do increment destination addr */
    .DLAST_SGA = 0,     /* After major loop continue destination addr */
    .BITERE_LINK = 0,
    .BITER = ADC_NBR_OF_GROUP1_CHANNELS,
    .BWC = 0,       /* Default bandwidth control- no stalls */
    .MAJORLINKCH = 0,   /* Linking disabled */
    .DONE = 0,
    .ACTIVE = 0,
    .MAJORE_LINK = 0,
    .E_SG = 0,
    .D_REQ = 0,     /* Disable channel when major loop is done (sending the buffer once) */
    .INT_HALF = 0,
    .INT_MAJ = 0,
    .START = 0
  },
  { .SADDR = (uint32)&ADC_0.CDR[32].R + 2,    /* Source Address */
    .SMOD = 0,      /* Source modulo feature not used */
    .SSIZE = DMA_TRANSFER_SIZE_16BITS,      /* Read 2**2 = 2 byte per transfer */
    .DMOD = 0,      /* Destination modulo feature not used */
    .DSIZE = DMA_TRANSFER_SIZE_16BITS,      /* Write 2**2 = 2 byte per transfer */
    .SOFF = 4,      /* After transfer, increment 4 = sizeof(chanel data register) */
    .NBYTESu.R = 2,    /* Transfer 2 byte per minor loop */
    .SLAST = -(4 * ADC_NBR_OF_GROUP2_CHANNELS),     /* After major loop jump to the beginning */
    .DADDR = NULL,     /* Destination Address, to be changed later */
    .CITERE_LINK = 0,   /* Linking disabled */
    .CITER = ADC_NBR_OF_GROUP2_CHANNELS,   /* Initialize current iteraction count */
    .DOFF = 2,     /* Do increment destination addr */
    .DLAST_SGA = 0,     /* After major loop continue destination addr */
    .BITERE_LINK = 0,
    .BITER = ADC_NBR_OF_GROUP2_CHANNELS,
    .BWC = 0,       /* Default bandwidth control- no stalls */
    .MAJORLINKCH = 0,   /* Linking disabled */
    .DONE = 0,
    .ACTIVE = 0,
    .MAJORE_LINK = 0,
    .E_SG = 0,
    .D_REQ = 0,     /* Disable channel when major loop is done (sending the buffer once) */
    .INT_HALF = 0,
    .INT_MAJ = 0,
    .START = 0
  },
  { .SADDR = (uint32)&ADC_0.CDR[32].R + 2,    /* Source Address */
    .SMOD = 0,      /* Source modulo feature not used */
    .SSIZE = DMA_TRANSFER_SIZE_16BITS,      /* Read 2**2 = 2 byte per transfer */
    .DMOD = 0,      /* Destination modulo feature not used */
    .DSIZE = DMA_TRANSFER_SIZE_16BITS,      /* Write 2**2 = 2 byte per transfer */
    .SOFF = 4,      /* After transfer, increment 4 = sizeof(chanel data register) */
    .NBYTESu.R = 2,    /* Transfer 2 byte per minor loop */
    .SLAST = -(4 * ADC_NBR_OF_GROUP3_CHANNELS),     /* After major loop jump to the beginning */
    .DADDR = NULL,     /* Destination Address, to be changed later */
    .CITERE_LINK = 0,   /* Linking disabled */
    .CITER = ADC_NBR_OF_GROUP3_CHANNELS,   /* Initialize current iteraction count */
    .DOFF = 2,     /* Do increment destination addr */
    .DLAST_SGA = 0,     /* After major loop continue destination addr */
    .BITERE_LINK = 0,
    .BITER = ADC_NBR_OF_GROUP3_CHANNELS,
    .BWC = 0,       /* Default bandwidth control- no stalls */
    .MAJORLINKCH = 0,   /* Linking disabled */
    .DONE = 0,
    .ACTIVE = 0,
    .MAJORE_LINK = 0,
    .E_SG = 0,
    .D_REQ = 0,     /* Disable channel when major loop is done (sending the buffer once) */
    .INT_HALF = 0,
    .INT_MAJ = 0,
    .START = 0
  }
 };

Adc_GroupStatus AdcGroupStatus[ADC_NBR_OF_GROUPS]=
{
	{ .notifictionEnable = 0,
	  .resultBufferPtr = NULL,
	  .groupStatus = ADC_IDLE,
	  .isConversionStarted = FALSE,
	},
	{ .notifictionEnable = 0,
	  .resultBufferPtr = NULL,
	  .groupStatus = ADC_IDLE,
	  .isConversionStarted = FALSE,
	},
	{ .notifictionEnable = 0,
	  .resultBufferPtr = NULL,
	  .groupStatus = ADC_IDLE,
	  .isConversionStarted = FALSE,
	},
	{ .notifictionEnable = 0,
	  .resultBufferPtr = NULL,
	  .groupStatus = ADC_IDLE,
	  .isConversionStarted = FALSE,
	},
};

const Adc_GroupDefType AdcGroupConfiguration [] =
{
   { .adcChannelConvTime= {ADC_INPLATCH_1, ADC_INPCPM_3, 0xff},
     .accessMode        = ADC_ACCESS_MODE_STREAMING,
     .conversionMode    = ADC_CONV_MODE_CONTINOUS,
     .triggerSrc        = ADC_TRIGG_SRC_SW,
     .hwTriggerSignal   = ADC_NO_HW_TRIG,
     .hwTriggerTimer    = ADC_NO_TIMER,
     .groupCallback     = Adc_Group0Notification,
     .streamBufferMode  = ADC_STREAM_BUFFER_CIRCULAR,
     .streamNumSamples  = 20,
     .channelList       = Adc_Group0ChannelList,
     .numberOfChannels  = ADC_NBR_OF_GROUP0_CHANNELS,
     .status            = &AdcGroupStatus[ADC_GROUP0],
     .dmaResultChannel  = DMA_ADC_GROUP0_RESULT_CHANNEL,
     .groupDMAResults   = &AdcGroupDMAResultConfig[ADC_GROUP0],
   },

   { .adcChannelConvTime= {ADC_INPLATCH_1, ADC_INPCPM_3, 20},
     .accessMode        = ADC_ACCESS_MODE_SINGLE,
     .conversionMode = ADC_CONV_MODE_ONESHOT,
     .triggerSrc        = ADC_TRIGG_SRC_SW, 
     .hwTriggerSignal = ADC_NO_HW_TRIG,
     .hwTriggerTimer    = ADC_NO_TIMER,
     .groupCallback     = Adc_Group1Notification,
     .streamBufferMode  = ADC_NO_STREAMING,
     .streamNumSamples = 1,
     .channelList       = Adc_Group1ChannelList,
     .numberOfChannels  = ADC_NBR_OF_GROUP1_CHANNELS,
     .status            = &AdcGroupStatus[ADC_GROUP1],
     .dmaResultChannel  = DMA_ADC_GROUP0_RESULT_CHANNEL,
     .groupDMAResults   = &AdcGroupDMAResultConfig[ADC_GROUP1]
   },

   { .adcChannelConvTime= {ADC_INPLATCH_1, ADC_INPCPM_3, 50},
     .accessMode        = ADC_ACCESS_MODE_STREAMING,
     .conversionMode    = ADC_CONV_MODE_CONTINOUS,
     .triggerSrc        = ADC_TRIGG_SRC_SW,
     .hwTriggerSignal   = ADC_NO_HW_TRIG,
     .hwTriggerTimer    = ADC_NO_TIMER,
     .groupCallback     = Adc_Group2Notification,
     .streamBufferMode  = ADC_STREAM_BUFFER_CIRCULAR,
     .streamNumSamples  = 20,
     .channelList       = Adc_Group2ChannelList,
     .numberOfChannels  = ADC_NBR_OF_GROUP2_CHANNELS,
     .status            = &AdcGroupStatus[ADC_GROUP2],
     .dmaResultChannel  = DMA_ADC_GROUP0_RESULT_CHANNEL,
     .groupDMAResults   = &AdcGroupDMAResultConfig[ADC_GROUP2],
   },

   { .adcChannelConvTime= {ADC_INPLATCH_1, ADC_INPCPM_3, 0xff},
     .accessMode        = ADC_ACCESS_MODE_STREAMING,
     .conversionMode    = ADC_CONV_MODE_CONTINOUS,
     .triggerSrc        = ADC_TRIGG_SRC_SW,
     .hwTriggerSignal   = ADC_NO_HW_TRIG,
     .hwTriggerTimer    = ADC_NO_TIMER,
     .groupCallback     = Adc_Group3Notification,
     .streamBufferMode  = ADC_STREAM_BUFFER_CIRCULAR,
     .streamNumSamples  = 20,
     .channelList       = Adc_Group3ChannelList,
     .numberOfChannels  = ADC_NBR_OF_GROUP3_CHANNELS,
     .status            = &AdcGroupStatus[ADC_GROUP3],
     .dmaResultChannel  = DMA_ADC_GROUP0_RESULT_CHANNEL,
     .groupDMAResults   = &AdcGroupDMAResultConfig[ADC_GROUP3],
   },
};


/******************************************************************/
/*                                                                */
/* End of user configuration area. DO NOT modify the code below!! */
/******************************************************************/
const Adc_ConfigType AdcConfig[] =
{
  {
   .hwConfigPtr      = &AdcHWUnitConfiguration,
   .channelConfigPtr = AdcChannelConfiguration,
   .nbrOfChannels    = ADC_NBR_OF_CHANNELS,
   .groupConfigPtr   = AdcGroupConfiguration,
   .nbrOfGroups      = ADC_NBR_OF_GROUPS,
  }
};

