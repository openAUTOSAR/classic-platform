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
 *  Created on: 2009-okt-02
 *      Author: Fredrik
 */

#include "Adc.h"
#include "stm32f10x_adc.h"


Adc_GroupStatus AdcGroupStatus[ADC_NBR_OF_GROUPS];

/* Configuration goes here. */
void Adc_Group0Notification (void)
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
  { .adcChannelConvTime = ADC_SampleTime_55Cycles5 },
  { .adcChannelConvTime = ADC_SampleTime_55Cycles5 },
  { .adcChannelConvTime = ADC_SampleTime_55Cycles5 },
  { .adcChannelConvTime = ADC_SampleTime_55Cycles5 },
  { .adcChannelConvTime = ADC_SampleTime_55Cycles5 },
  { .adcChannelConvTime = ADC_SampleTime_55Cycles5 },
  { .adcChannelConvTime = ADC_SampleTime_55Cycles5 },
  { .adcChannelConvTime = ADC_SampleTime_55Cycles5 },
  { .adcChannelConvTime = ADC_SampleTime_55Cycles5 },
  { .adcChannelConvTime = ADC_SampleTime_55Cycles5 },
  { .adcChannelConvTime = ADC_SampleTime_55Cycles5 },
  { .adcChannelConvTime = ADC_SampleTime_55Cycles5 },
  { .adcChannelConvTime = ADC_SampleTime_55Cycles5 },
  { .adcChannelConvTime = ADC_SampleTime_55Cycles5 },
  { .adcChannelConvTime = ADC_SampleTime_55Cycles5 },
  { .adcChannelConvTime = ADC_SampleTime_55Cycles5 },
};

const Adc_ChannelType Adc_Group0ChannelList[ADC_NBR_OF_GROUP0_CHANNELS] =
{
	ADC_CH1,ADC_CH1,ADC_CH1,ADC_CH1
};


/* Ram buffers for command and result queues. These are located here in the
   configuration to be able to reconfigure system without recompiling the
   drivers. */
Adc_ValueGroupType Adc_Group0Buffer [sizeof(Adc_Group0ChannelList)/sizeof(Adc_Group0ChannelList[0])];

const Adc_GroupDefType AdcGroupConfiguration [] =
{
   {
	 // NOT SUPPORTED  .accessMode        = ADC_ACCESS_MODE_SINGLE,
     .conversionMode    = ADC_CONV_MODE_ONESHOT,
     .triggerSrc        = ADC_TRIGG_SRC_SW,
     // NOT SUPPORTED .hwTriggerSignal   = ADC_NO_HW_TRIG,
     // NOT SUPPORTED .hwTriggerTimer    = ADC_NO_TIMER,
     .groupCallback     = Adc_Group0Notification,
     // NOT SUPPORTED .streamBufferMode  = ADC_NO_STREAMING,
     // NOT SUPPORTED .streamNumSamples  = 0,
     .channelList       = Adc_Group0ChannelList,
     .resultBuffer      = Adc_Group0Buffer,
     .numberOfChannels  = sizeof(Adc_Group0ChannelList)/sizeof(Adc_Group0ChannelList[0]),
     .status            = &AdcGroupStatus[ADC_GROUP0]},
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

