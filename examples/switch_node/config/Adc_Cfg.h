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








#ifndef ADC_CFG_H_
#define ADC_CFG_H_

#define ADC_PRIORITY_HW                   0
#define ADC_PRIORITY_HW_SW                1
#define ADC_PRIORITY_NONE                 2

#define ADC_DEINIT_API                    STD_ON
#define ADC_DEV_ERROR_DETECT              STD_ON
#define ADC_ENABLE_QUEUING                STD_ON
#define ADC_ENABLE_START_STOP_GROUP_API   STD_ON
#define ADC_GRP_NOTIF_CAPABILITY          STD_ON
#define ADC_HW_TRIGGER_API                STD_OFF           /* Not implemented. */
#define ADC_PRIORITY_IMPLEMENTATION       ADC_PRIORITY_HW
#define ADC_READ_GROUP_API                STD_ON
#define ADC_VERSION_API                   STD_ON            /* Not implemented. */

/* Group definitions. */


typedef enum
{
  ADC_SWITCHES,
  ADC_POTENTIOMETERS
}Adc_GroupType_NiceNames;

typedef enum
{
	ADC_GROUP0,
	ADC_GROUP1,
	ADC_NBR_OF_GROUPS
}Adc_GroupType;

typedef enum
{
  ADC_CH0,
  ADC_CH1,
  ADC_CH2,
  ADC_CH3,
  ADC_CH4,
  ADC_CH5,
  ADC_CH6,
  ADC_CH7,
  ADC_CH8,
  ADC_CH9,
  ADC_CH10,
  ADC_CH11,
  ADC_CH12,
  ADC_CH13,
  ADC_CH14,
  ADC_CH15,
  ADC_CH16,
  ADC_CH17,
  ADC_CH18,
  ADC_CH19,
  ADC_CH20,
  ADC_NBR_OF_CHANNELS,
}Adc_ChannelType;

typedef enum
{
  ADC_SWITCH_RED,
  ADC_SWITCH_BLACK,
  ADC_NBR_OF_SWITCH_CHANNELS,
}Adc_SwitchesSignalType;

typedef enum
{
  ADC_POTENTIOMETER_NOT_USED,
  ADC_POTENTIOMETER_0,

  ADC_POTENTIOMETER_1,
  ADC_NBR_OF_POTENTIOMETER_CHANNELS,
}Adc_PotentiometersSignals;

extern const struct tcd_t AdcGroupDMACommandConfig [ADC_NBR_OF_GROUPS];
extern const struct tcd_t AdcGroupDMAResultConfig [ADC_NBR_OF_GROUPS];

#endif /*ADC_CFG_H_*/
