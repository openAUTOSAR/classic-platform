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
#define ADC_ENABLE_QUEUING                STD_ON
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
#define ADC_VERSION_API                   STD_ON

typedef uint16_t Adc_ValueGroupType;


/** HW specific data type. */
typedef union
{
  vuint32_t R;
  struct
  {
    vuint32_t EOQ:1;
    vuint32_t PAUSE:1;
    vuint32_t :4;
    vuint32_t BN:1;
    vuint32_t CAL:1;
    vuint32_t MESSAGE_TAG:4;
    vuint32_t LST:2;
    vuint32_t TSR:1;
    vuint32_t FMT:1;
    vuint32_t CHANNEL_NUMBER:8;
    vuint32_t :8;
   } B;
}Adc_CommandType;

/** Type of clock input for the conversion unit. */
typedef enum
{
  ADC_SYSTEM_CLOCK
}Adc_ClockSourceType;


/** Clock prescaler factor. */
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
typedef enum
{
  ADC_CONVERSION_TIME_2_CLOCKS,
  ADC_CONVERSION_TIME_8_CLOCKS,
  ADC_CONVERSION_TIME_64_CLOCKS,
  ADC_CONVERSION_TIME_128_CLOCKS
}Adc_ConversionTimeType;

/** Enable/disable calibration. */
typedef enum
{
  ADC_CALIBRATION_DISABLED,
  ADC_CALIBRATION_ENABLED
}Adc_CalibrationType;

/** Channel resolution. */
typedef enum
{
  ADC_RESOLUTION_12BITS
}Adc_ResolutionType;

/** Container for channel configuration. */
typedef struct
{
  Adc_ConversionTimeType adcChannelConvTime;
  Adc_VoltageSourceType  adcChannelRefVoltSrcLow;
  Adc_VoltageSourceType  adcChannelRefVoltSrcHigh;
  Adc_ResolutionType     adcChannelResolution;
  Adc_CalibrationType    adcChannelCalibrationEnable;
}Adc_ChannelConfigurationType;


/* TODO list timer sources here. */
/** Not supported. */
typedef enum
{
  ADC_NO_TIMER,
}Adc_HwTriggerTimerType;


/** Not supported. */
typedef uint16_t Adc_StreamNumSampleType;

/** Channel conversion mode. */
typedef enum
{
  ADC_CONV_MODE_DISABLED,
  ADC_CONV_MODE_ONESHOT   = 1,  /**< A single conversion. */
  ADC_CONV_MODE_CONTINOUS = 9,  /**< Conversions performed continuously. */
}Adc_GroupConvModeType;


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


/** Container for groups status info. */
typedef struct
{
  uint8 notifictionEnable;
  Adc_ValueGroupType *resultBufferPtr;
  Adc_StatusType groupStatus;
}Adc_GroupStatus;

/** Container for group setup. */
typedef struct
{
  Adc_GroupAccessModeType      accessMode;
  Adc_GroupConvModeType        conversionMode;
  Adc_TriggerSourceType        triggerSrc;
  Adc_HwTriggerSignalType      hwTriggerSignal;
  Adc_HwTriggerTimerType       hwTriggerTimer;
  void                         (*groupCallback)(void);
  Adc_StreamBufferModeType     streamBufferMode;
  Adc_StreamNumSampleType      streamNumSamples;
  const Adc_ChannelType        *channelList;
  Adc_ValueGroupType           *resultBuffer;
  Adc_CommandType              *commandBuffer;
  Adc_ChannelType              numberOfChannels;
  Adc_GroupStatus              *status;
  Dma_ChannelType              dmaCommandChannel;
  Dma_ChannelType              dmaResultChannel;
  const struct tcd_t           *groupDMACommands;
  const struct tcd_t           *groupDMAResults;
}Adc_GroupDefType;

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

/** HW specific command configarations. */
extern const struct tcd_t AdcGroupDMACommandConfig [ADC_NBR_OF_GROUPS];
/** HW specific result buffer configarations. */
extern const struct tcd_t AdcGroupDMAResultConfig [ADC_NBR_OF_GROUPS];

#endif /*ADC_CFG_H_*/
/** @} */
