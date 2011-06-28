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

#ifndef ADC_CONFIGTYPES_H_
#define ADC_CONFIGTYPES_H_


typedef uint16_t Adc_ValueGroupType;


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
  ADC_NBR_OF_CHANNELS,
}Adc_ChannelType;

/* Std-type, supplier defined */
typedef enum
{
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

/* Std-type, supplier defined */
typedef enum
{
  ADC_CONVERSION_TIME_2_CLOCKS,
  ADC_CONVERSION_TIME_4_CLOCKS,
  ADC_CONVERSION_TIME_8_CLOCKS,
  ADC_CONVERSION_TIME_16_CLOCKS
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

typedef enum
{
	ADC_RESOLUTION_10_BIT,
	ADC_RESOLUTION_8_BIT,
}Adc_ResolutionType;

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
  ADC_CONV_MODE_CONTINUOUS = 9,
} Adc_GroupConvModeType;

typedef struct
{
  Adc_GroupConvModeType        conversionMode;
  Adc_TriggerSourceType        triggerSrc;
  void                         (*groupCallback)(void);
  const Adc_ChannelType        *channelList;
  Adc_ValueGroupType           *resultBuffer;
  Adc_ChannelType              numberOfChannels;
  Adc_GroupStatus              *status;
} Adc_GroupDefType;

typedef struct
{
  Adc_ConversionTimeType       convTime;
  Adc_ResolutionType           resolution;
  Adc_PrescaleType    adcPrescale;
}Adc_HWConfigurationType;

typedef struct
{
  const Adc_HWConfigurationType*      hwConfigPtr;
  const Adc_GroupDefType*             groupConfigPtr;
  const uint16_t                      nbrOfGroups;
} Adc_ConfigType;

extern const Adc_ConfigType AdcConfig[];
#endif /* ADC_CONFIGTYPES_H_ */
