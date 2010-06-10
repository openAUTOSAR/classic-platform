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
/*
 * Adc_Cfg.c
 *
 *  Created on: 2010-apr-24
 *      Author: Jonte
 */

#include "Adc.h"

Adc_GroupStatus AdcGroupStatus[ADC_NBR_OF_GROUPS];

void Adc_Group0Notification (void);
void Adc_Group1Notification (void);

const Adc_HWConfigurationType AdcHWUnitConfiguration[] =
{
	{
		.convTime = ADC_CONVERSION_TIME_8_CLOCKS,
		.resolution = ADC_RESOLUTION_10_BIT,
		.adcPrescale = ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_12,
	},
};

const Adc_ChannelType Adc_Group0ChannelList[ADC_NBR_OF_GROUP0_CHANNELS] =
{
	ADC_CH1,ADC_CH2,ADC_CH3,ADC_CH4
};
const Adc_ChannelType Adc_Group1ChannelList[ADC_NBR_OF_GROUP1_CHANNELS] =
{
	ADC_CH5,ADC_CH6,
};


/* Ram buffers for command and result queues. These are located here in the
   configuration to be able to reconfigure system without recompiling the
   drivers. */
Adc_ValueGroupType Adc_Group0Buffer [sizeof(Adc_Group0ChannelList)/sizeof(Adc_Group0ChannelList[0])];
Adc_ValueGroupType Adc_Group1Buffer [sizeof(Adc_Group1ChannelList)/sizeof(Adc_Group1ChannelList[0])];

const Adc_GroupDefType AdcGroupConfiguration [] =
{
		   {
		     .conversionMode    = ADC_CONV_MODE_ONESHOT,
		     .triggerSrc        = ADC_TRIGG_SRC_SW,
		     .groupCallback     = Adc_Group0Notification,
		     .channelList       = Adc_Group0ChannelList,
		     .resultBuffer      = Adc_Group0Buffer,
		     .numberOfChannels  = sizeof(Adc_Group0ChannelList)/sizeof(Adc_Group0ChannelList[0]),
		     .status            = &AdcGroupStatus[ADC_GROUP0],
		   },
		   {
		     .conversionMode    = ADC_CONV_MODE_ONESHOT,
		     .triggerSrc        = ADC_TRIGG_SRC_SW,
		     .groupCallback     = Adc_Group1Notification,
		     .channelList       = Adc_Group1ChannelList,
		     .resultBuffer      = Adc_Group1Buffer,
		     .numberOfChannels  = sizeof(Adc_Group1ChannelList)/sizeof(Adc_Group0ChannelList[0]),
		     .status            = &AdcGroupStatus[ADC_GROUP1],
		   },
};


/******************************************************************/
/*                                                                */
/* End of user configuration area. DO NOT modify the code below!! */
/*                                                                */
/******************************************************************/
const Adc_ConfigType AdcConfig[] =
{
  {
   .hwConfigPtr      = AdcHWUnitConfiguration,
   .groupConfigPtr   = AdcGroupConfiguration,
  }
};

