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
  ADC_GROUP0,
  ADC_GROUP1,
  ADC_GROUP2,
  ADC_GROUP3,
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
  ADC_TEST_BOARD_AIN1,
  ADC_TEST_BOARD_AIN2,
  ADC_TEST_BOARD_AIN3,
  ADC_TEST_BOARD_AIN4,
  ADC_TEST_BOARD_AIN5,
  ADC_TEST_BOARD_AIN6,
  ADC_TEST_BOARD_AIN7,
  ADC_NBR_OF_GROUP0_CHANNELS,
}Adc_Group0SignalType;

typedef enum
{
  ADC_GROUP1_CH1,
  ADC_GROUP1_CH2,
  ADC_GROUP1_CH3,
  ADC_GROUP1_CH4,
  ADC_NBR_OF_GROUP1_CHANNELS,
}Adc_Group1SignalType;

typedef enum
{
  ADC_GROUP2_CH0,
  ADC_GROUP2_CH1,
  ADC_GROUP2_CH2,
  ADC_NBR_OF_GROUP2_CHANNELS,
}Adc_Group2Signals;

typedef enum
{
  ADC_GROUP3_CH0,
  ADC_GROUP3_CH1,
  ADC_GROUP3_CH2,
  ADC_NBR_OF_GROUP3_CHANNELS,
}Adc_Group3Signals;

extern const struct tcd_t AdcGroupDMACommandConfig [ADC_NBR_OF_GROUPS];
extern const struct tcd_t AdcGroupDMAResultConfig [ADC_NBR_OF_GROUPS];

#endif /*ADC_CFG_H_*/
