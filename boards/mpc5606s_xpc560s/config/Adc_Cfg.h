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

/** @addtogroup Adc ADC Driver
 *  @{ */

/** @file Adc_Cfg.h
 * Definitions of configuration parameters for ADC Driver.
 */

#warning "This default file may only be used as an example!"

#ifndef ADC_CFG_H_
#define ADC_CFG_H_

#include "Dma.h"

/** HW priority mechanism only. @see ADC_PRIORITY_IMPLEMENTATION */
#define ADC_PRIORITY_HW                   0
/** HW and SW priority mechanism. @see ADC_PRIORITY_IMPLEMENTATION  */
#define ADC_PRIORITY_HW_SW                1
/** No priority mechanism. @see ADC_PRIORITY_IMPLEMENTATION  */
#define ADC_PRIORITY_NONE                 2

/** Build DeInit API */
#define ADC_DEINIT_API                    STD_ON
/** Enable Development Error Trace */
#define ADC_DEV_ERROR_DETECT              STD_ON
/** Not supported. */
#define ADC_ENABLE_QUEUING                STD_OFF
/** Build Start/Stop group API. */
#define ADC_ENABLE_START_STOP_GROUP_API   STD_ON
/** Enable group conversion notification. */
#define ADC_GRP_NOTIF_CAPABILITY          STD_ON
/** Not supported. */
#define ADC_HW_TRIGGER_API                STD_OFF
/** Implemented priority mechanism. */
#define ADC_PRIORITY_IMPLEMENTATION       ADC_PRIORITY_HW
/** Build Read group API */
#define ADC_READ_GROUP_API                STD_ON
/** Build version info API (Not supported) */
#define ADC_VERSION_INFO_API              STD_ON

typedef uint16_t Adc_ValueGroupType;


/** HW specific data type. */
typedef vint32_t Adc_CommandType;		/* isoft, not used. */

/** ID of channel */
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

/** Type of clock input for the conversion unit. */
typedef enum
{
  ADC_SYSTEM_CLOCK
}Adc_ClockSourceType;


/** Clock prescaler factor. */
typedef enum
{
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_2, /* half bus clock */
  ADC_SYSTEM_CLOCK_DIVIDE_FACTOR_1,     /* same bus clock */
}Adc_PrescaleType;

/** Container for HW setup. */
typedef struct
{
  Adc_ClockSourceType clockSource;
  uint8_t             hwUnitId;
  Adc_PrescaleType    adcPrescale;
}Adc_HWConfigurationType;

/** Reference voltage source. */
typedef enum
{
  ADC_REFERENCE_VOLTAGE_GROUND,
  ADC_REFERENCE_VOLTAGE_5V,
}Adc_VoltageSourceType;

/** Duration of conversion. */
typedef uint32_t Adc_ConversionTimeType;
typedef uint8_t Adc_SampTimeType;
typedef enum
{
	ADC_INPLATCH_0,
	ADC_INPLATCH_1
}Adc_LatchTimeType;

typedef enum
{
	ADC_INPCPM_1 = 1,
	ADC_INPCPM_2,
	ADC_INPCPM_3
}Adc_CmpTimeType;

typedef struct
{
	Adc_LatchTimeType	INPLATCH;
	Adc_CmpTimeType		INPCMP;
	Adc_SampTimeType	INPSAMP;
}Adc_Channel_Phase_DurationType;

/** Enable/disable calibration. */
typedef enum
{
  ADC_CALIBRATION_DISABLED,
  ADC_CALIBRATION_ENABLED
}Adc_CalibrationType;

/** Channel resolution. */
typedef enum
{
  ADC_RESOLUTION_10BITS,
}Adc_ResolutionType;

/** Container for channel configuration. */
typedef struct
{
  Adc_VoltageSourceType  adcChannelRefVoltSrcLow;
  Adc_VoltageSourceType  adcChannelRefVoltSrcHigh;
  Adc_ResolutionType     adcChannelResolution;
  Adc_CalibrationType    adcChannelCalibrationEnable;
}Adc_ChannelConfigurationType;


/** Channel conversion mode. */
typedef enum
{
  ADC_CONV_MODE_ONESHOT,  /**< A single conversion. */
  ADC_CONV_MODE_CONTINOUS,  /**< Conversions performed continuously. */
}Adc_GroupConvModeType;



/* TODO list timer sources here. */
/** Not supported. */
typedef enum
{
  ADC_NO_TIMER,
}Adc_HwTriggerTimerType;


/** Not supported. */
typedef uint16_t Adc_StreamNumSampleType;

typedef enum
{
	ADC_GROUP_UNSTART,
	ADC_GROUP_START
}Adc_GroupStartStatusType;


/** Container for groups status info. */
typedef struct
{
  uint8 notifictionEnable;
  Adc_ValueGroupType *resultBufferPtr;
  Adc_StatusType groupStatus;
  boolean isConversionStarted;  /* TRUE if Adc_StartGroupConversion invoked */
}Adc_GroupStatus;


/* Group definitions. */

/** ID of group */
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
  ADC_GROUP0_CH0,
  ADC_GROUP0_CH1,
  ADC_GROUP0_CH2,
  ADC_NBR_OF_GROUP0_CHANNELS,
}Adc_Group0SignalType;

typedef enum
{
  ADC_GROUP1_CH2,
  ADC_GROUP1_CH3,
  ADC_GROUP1_CH4,
  ADC_GROUP1_CH5,
  ADC_NBR_OF_GROUP1_CHANNELS,
}Adc_Group1SignalType;

typedef enum
{
  ADC_GROUP2_CH0,
  ADC_NBR_OF_GROUP2_CHANNELS,
}Adc_Group2Signals;

typedef enum
{
  ADC_GROUP3_CH0,
  ADC_GROUP3_CH1,
  ADC_GROUP3_CH2,
  ADC_GROUP3_CH3,
  ADC_GROUP3_CH4,
  ADC_GROUP3_CH5,
  ADC_GROUP3_CH6,
  ADC_GROUP3_CH7,
  ADC_GROUP3_CH8,
  ADC_GROUP3_CH9,
  ADC_GROUP3_CH10,
  ADC_GROUP3_CH11,
  ADC_GROUP3_CH12,
  ADC_GROUP3_CH13,
  ADC_GROUP3_CH14,
  ADC_GROUP3_CH15,
  ADC_NBR_OF_GROUP3_CHANNELS,
}Adc_Group3Signals;

/** Container for group setup. */
typedef struct
{
  Adc_Channel_Phase_DurationType adcChannelConvTime;
  Adc_GroupAccessModeType      accessMode;
  Adc_GroupConvModeType        conversionMode;
  Adc_TriggerSourceType        triggerSrc;
  Adc_HwTriggerSignalType      hwTriggerSignal;
  Adc_HwTriggerTimerType       hwTriggerTimer;
  void                         (*groupCallback)(void);
  Adc_StreamBufferModeType     streamBufferMode;
  Adc_StreamNumSampleType      streamNumSamples;
  const Adc_ChannelType        *channelList;
  Adc_ValueGroupType           *resultBuffer;   /* deprecated, use resultBufferPtr instead */
  Adc_CommandType              *commandBuffer;
  Adc_ChannelType              numberOfChannels;
  Adc_GroupStatus              *status;
  Dma_ChannelType              dmaResultChannel;
  Dma_TcdType                 *groupDMAResults;
}Adc_GroupDefType;

/** Container for module initialization parameters. */
typedef struct
{
  const Adc_HWConfigurationType*      hwConfigPtr;
  const Adc_ChannelConfigurationType* channelConfigPtr;
  const uint16_t                      nbrOfChannels;
  const Adc_GroupDefType*             groupConfigPtr;
  const uint16_t                      nbrOfGroups;
}Adc_ConfigType;

extern const Adc_ConfigType AdcConfig [];

/** HW specific result buffer configarations. */
extern Dma_TcdType AdcGroupDMAResultConfig [ADC_NBR_OF_GROUPS];

#endif /*ADC_CFG_H_*/
/** @} */

