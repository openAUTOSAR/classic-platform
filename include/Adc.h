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








#ifndef ADC_H_
#define ADC_H_

#include "Std_Types.h"
#include "Dma.h"
#include "Adc_Cfg.h"

typedef uint16_t Adc_ValueGroupType;

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

typedef enum
{
  ADC_SYSTEM_CLOCK
}Adc_ClockSourceType;

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

typedef struct
{
  Adc_ClockSourceType clockSource;
  uint8_t             hwUnitId;
  Adc_PrescaleType    adcPrescale;
}Adc_HWConfigurationType;

typedef enum
{
  ADC_REFERENCE_VOLTAGE_GROUND,
  ADC_REFERENCE_VOLTAGE_5V,
}Adc_VoltageSourceType;

typedef enum
{
  ADC_CONVERSION_TIME_2_CLOCKS,
  ADC_CONVERSION_TIME_8_CLOCKS,
  ADC_CONVERSION_TIME_64_CLOCKS,
  ADC_CONVERSION_TIME_128_CLOCKS
}Adc_ConversionTimeType;

typedef enum
{
  ADC_CALIBRATION_DISABLED,
  ADC_CALIBRATION_ENABLED
}Adc_CalibrationType;

typedef enum
{
  ADC_RESOLUTION_12BITS
}Adc_ResolutionType;

/* Channel definitions. */
typedef struct
{
  Adc_ConversionTimeType adcChannelConvTime;
  Adc_VoltageSourceType  adcChannelRefVoltSrcLow;
  Adc_VoltageSourceType  adcChannelRefVoltSrcHigh;
  Adc_ResolutionType     adcChannelResolution;
  Adc_CalibrationType    adcChannelCalibrationEnable;
}Adc_ChannelConfigurationType;

typedef enum
{
  ADC_ACCESS_MODE_SINGLE,
  ADC_ACCESS_MODE_STREAMING
}Adc_GroupAccessModeType;

typedef enum
{
  ADC_TRIGG_SRC_HW,
  ADC_TRIGG_SRC_SW,
}Adc_TriggerSourceType;

typedef enum
{
  ADC_NO_HW_TRIG,
  ADC_HW_TRIG_BOTH_EDGES,
  ADC_HW_TRIG_FALLING_EDGE,
  ADC_HW_TRIG_RISING_EDGE,
}Adc_HwTriggerSignalType;

/* TODO list timer sources here. */
typedef enum
{
  ADC_NO_TIMER,
}Adc_HwTriggerTimerType;

typedef enum
{
  ADC_NO_STREAMING,
  ADC_STREAM_BUFFER_CIRCULAR,
  ADC_STREAM_BUFFER_LINEAR,
}Adc_StreamBufferModeType;

typedef uint16_t Adc_StreamNumSampleType;

typedef enum
{
  ADC_CONV_MODE_DISABLED,
  ADC_CONV_MODE_ONESHOT   = 1,
  ADC_CONV_MODE_CONTINOUS = 9,
}Adc_GroupConvModeType;

typedef enum
{
  ADC_IDLE,
  ADC_BUSY,
  ADC_COMPLETED,
  ADC_STREAM_COMPLETED,
}Adc_StatusType;

typedef struct
{
  uint8 notifictionEnable;
  Adc_ValueGroupType *resultBufferPtr;
  Adc_StatusType groupStatus;
}Adc_GroupStatus;


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


/* Det error that the adc can produce. */
typedef enum
{
  ADC_E_UNINIT              = 0x0A,
  ADC_E_BUSY                = 0x0B,
  ADC_E_IDLE                = 0x0C,
  ADC_E_ALREADY_INITIALIZED = 0x0D,
  ADC_E_PARAM_CONFIG        = 0x0E,
  ADC_E_PARAM_GROUP         = 0x15,
  ADC_E_WRONG_CONV_MODE     = 0x16,
  ADC_E_WRONG_TRIGG_SRC     = 0x17,
  ADC_E_NOTIF_CAPABILITY    = 0x18,
  ADC_E_BUFFER_UNINIT       = 0x19
}Adc_DetErrorType;

/* API service ID's */
typedef enum
{
  ADC_INIT_ID = 0x00,
  ADC_DEINIT_ID = 0x01,
  ADC_STARTGROUPCONVERSION_ID = 0x02,
  ADC_STOPGROUPCONVERSION_ID = 0x03,
  ADC_READGROUP_ID = 0x04,
  ADC_ENABLEHARDWARETRIGGER_ID = 0x05,
  ADC_DISBALEHARDWARETRIGGER_ID = 0x06,
  ADC_ENABLEGROUPNOTIFICATION_ID = 0x07,
  ADC_DISABLEGROUPNOTIFICATION_ID = 0x08,
  ADC_GETGROUPSTATUS_ID = 0x09,
  ADC_GETVERSIONINFO_ID = 0x0A,
  ADC_GETSTREAMLASTPOINTER_ID = 0x0B,
  ADC_SETUPRESULTBUFFER_ID = 0x0C,
}Adc_APIServiceIDType;


typedef struct
{
  const Adc_HWConfigurationType*      hwConfigPtr;
  const Adc_ChannelConfigurationType* channelConfigPtr;
  const uint16_t                      nbrOfChannels;
  const Adc_GroupDefType*             groupConfigPtr;
  const uint16_t                      nbrOfGroups;
}Adc_ConfigType;

extern const Adc_ConfigType AdcConfig [];

/* Function interface. */
Std_ReturnType Adc_Init (const Adc_ConfigType *ConfigPtr);
#if (ADC_DEINIT_API == STD_ON)
Std_ReturnType Adc_DeInit (const Adc_ConfigType *ConfigPtr);
#endif
Std_ReturnType Adc_SetupResultBuffer (Adc_GroupType group, Adc_ValueGroupType *bufferPtr);
#if (ADC_ENABLE_START_STOP_GROUP_API == STD_ON)
void           Adc_StartGroupConversion (Adc_GroupType group);
//void           Adc_StopGroupConversion (Adc_GroupType group);
#endif
#if (ADC_READ_GROUP_API == STD_ON)
Std_ReturnType Adc_ReadGroup (Adc_GroupType group, Adc_ValueGroupType *dataBufferPtr);
#endif
#if (ADC_GRP_NOTIF_CAPABILITY == STD_ON)
void           Adc_EnableGroupNotification (Adc_GroupType group);
void           Adc_DisableGroupNotification (Adc_GroupType group);
#endif
Adc_StatusType Adc_GetGroupStatus (Adc_GroupType group);

#endif /*ADC_H_*/
