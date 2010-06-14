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
 * Adc_Cfg.h
 *
 *  Created on: 2009-okt-02
 *      Author: Fredrik
 */
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

typedef uint16_t Adc_ValueGroupType;
/* Group definitions. */

typedef enum
{
  ADC_GROUP0,
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
  ADC_NBR_OF_CHANNELS,
}Adc_ChannelType;

typedef enum
{
	  ADC_TEST_BOARD_POT,
	  ADC_TEST_BOARD_POT2,
	  ADC_TEST_BOARD_POT3,
	  ADC_TEST_BOARD_POT4,
  ADC_NBR_OF_GROUP0_CHANNELS,
}Adc_Group0SignalType;

/* Std-type, supplier defined */
typedef enum
{
  ADC_SYSTEM_CLOCK
}Adc_ClockSourceType;


/* Std-type, supplier defined */
typedef enum
{
  ADC_SYSTEM_CLOCK_DISABLED,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_1,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_2,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_4,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_6,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_8,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_10,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_12,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_14,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_16,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_18,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_20,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_22,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_24,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_26,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_28,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_30,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_32,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_34,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_36,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_38,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_40,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_42,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_44,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_46,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_48,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_50,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_52,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_54,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_56,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_58,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_60,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_62,
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_64,
}Adc_PrescaleType;

/* Non-standard type */
typedef struct
{
  Adc_ClockSourceType clockSource;
  uint8_t             hwUnitId;
  Adc_PrescaleType    adcPrescale;
}Adc_HWConfigurationType;

/* Std-type, supplier defined */
typedef enum
{
  ADC_CONVERSION_TIME_2_CLOCKS,
  ADC_CONVERSION_TIME_8_CLOCKS,
  ADC_CONVERSION_TIME_64_CLOCKS,
  ADC_CONVERSION_TIME_128_CLOCKS
}Adc_ConversionTimeType;

/* Channel definitions, std container */
typedef struct
{
  Adc_ConversionTimeType adcChannelConvTime;
  // NOT SUPPORTED Adc_VoltageSourceType  adcChannelRefVoltSrcLow;
  // NOT SUPPORTED Adc_VoltageSourceType  adcChannelRefVoltSrcHigh;
  // NOT SUPPORTED Adc_ResolutionType     adcChannelResolution;
  // NOT SUPPORTED Adc_CalibrationType    adcChannelCalibrationEnable;
} Adc_ChannelConfigurationType;

/* Used ?? */
typedef struct
{
  uint8 				notifictionEnable;
  Adc_ValueGroupType *	resultBufferPtr;
  Adc_StatusType 		groupStatus;
} Adc_GroupStatus;


/* Std-type, supplier defined */
typedef enum
{
  ADC_CONV_MODE_DISABLED,
  ADC_CONV_MODE_ONESHOT   = 1,
  ADC_CONV_MODE_CONTINOUS = 9,
} Adc_GroupConvModeType;

/* Implementation specific */
typedef struct
{
  // NOT SUPPORTED  Adc_GroupAccessModeType      accessMode;
  Adc_GroupConvModeType        conversionMode;
  Adc_TriggerSourceType        triggerSrc;
  // NOT SUPPORTED  Adc_HwTriggerSignalType      hwTriggerSignal;
  // NOT SUPPORTED  Adc_HwTriggerTimerType       hwTriggerTimer;
  void                         (*groupCallback)(void);
  // NOT SUPPORTED  Adc_StreamBufferModeType     streamBufferMode;
  // NOT SUPPORTED  Adc_StreamNumSampleType      streamNumSamples;
  const Adc_ChannelType        *channelList;
  Adc_ValueGroupType           *resultBuffer;
  // NOT SUPPORTED  Adc_CommandType              *commandBuffer;
  Adc_ChannelType              numberOfChannels;
  Adc_GroupStatus              *status;
  // NOT SUPPORTED  Dma_ChannelType              dmaCommandChannel;
  // NOT SUPPORTED  Dma_ChannelType              dmaResultChannel;
  // NOT SUPPORTED  const struct tcd_t *		   groupDMACommands;
  // NOT SUPPORTED  const struct tcd_t *		   groupDMAResults;
} Adc_GroupDefType;

/* Non-standard type */
typedef struct
{
  const Adc_HWConfigurationType*      hwConfigPtr;
  const Adc_ChannelConfigurationType* channelConfigPtr;
  const uint16_t                      nbrOfChannels;
  const Adc_GroupDefType*             groupConfigPtr;
  const uint16_t                      nbrOfGroups;
} Adc_ConfigType;

extern const Adc_ConfigType AdcConfig [];


#endif /*ADC_CFG_H_*/
