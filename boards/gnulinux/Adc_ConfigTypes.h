/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

#ifndef ADC_CONFIGTYPES_H_
#define ADC_CONFIGTYPES_H_

#if defined(CFG_GNULINUX)

typedef uint16_t Adc_ValueGroupType;
/* Group definitions. */

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
  ADC_CONV_MODE_CONTINUOUS = 9,
} Adc_GroupConvModeType;

/** Not supported. */
typedef uint16_t Adc_StreamNumSampleType;

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

#elif defined(CFG_JACINTO)

/* @req:JACINTO5 SWS_Adc_00508 */
typedef uint16_t Adc_ValueGroupType;

/* @req:JACINTO5 SWS_Adc_00507 */
typedef uint16 Adc_GroupType;

/* @req:JACINTO5 SWS_Adc_00506 */
typedef uint8_t  Adc_ChannelType;

/* @req:JACINTO5 SWS_Adc_00509 */
typedef uint16_t Adc_PrescaleType;

/* !req:JACINTO5 SWS_Adc_00510 Adc_ConversionTimeType not configurable */

/* !req:JACINTO5 SWS_Adc_00511 Adc_SamplingTimeType not configurable */

/* !req:JACINTO5 SWS_Adc_00512 Adc_ResolutionType not configurable*/

/* @req:JACINTO5 SWS_Adc_00518 */
typedef uint16_t Adc_StreamNumSampleType;

/* @req:JACINTO5 SWS_Adc_00515 */
typedef enum
{
    ADC_CONV_MODE_ONESHOT = 0,
    ADC_CONV_MODE_CONTINUOUS
} Adc_GroupConvModeType;

typedef uint32 Adc_ArcControllerIdType;

/* Channel definitions, std container */
typedef struct
{
    Adc_ChannelType Adc_Channel;
    uint32          Adc_ChnDiff;
    uint32          Adc_ChnSelrfm;
    uint32          Adc_ChnSelrfp;
} Adc_ChannelConfigurationType;

typedef struct
{
  uint8                     notifictionEnable;
  Adc_ValueGroupType        *resultBufferPtr;
  Adc_StatusType            groupStatus;
  Adc_StreamNumSampleType   currSampleCount;   /* Samples per group counter. =0 until first round of conversions complete
                                                  (all channels in group). Then =1 until second round of conversions complete and so on.*/
  Adc_ValueGroupType        *currResultBufPtr;   /* Streaming sample current buffer pointer */
} Adc_GroupStatus;

typedef struct
{
  uint8_t             hwUnitId;
  Adc_PrescaleType    adcPrescale;
  uint32              numberOfChannels;
  const Adc_ChannelConfigurationType        *channelList;
}Adc_HWConfigurationType;

/** Container for group setup. */
typedef struct
{
  Adc_GroupAccessModeType      accessMode;
  Adc_GroupConvModeType        conversionMode;
  Adc_TriggerSourceType        triggerSrc;
  /* @req SWS_Adc_00085 */
  void                         (*groupCallback)(void);
  Adc_StreamBufferModeType     streamBufferMode;
  Adc_StreamNumSampleType      streamNumSamples;
  const uint32                 *channelMappingList;
  Adc_ChannelType              numberOfChannels;
  Adc_GroupStatus              *status;
  uint32                        hwUnit;
}Adc_GroupDefType;

/** Container for module initialization parameters. */
typedef struct
{
  const Adc_HWConfigurationType*      hwConfigPtr;
  const Adc_GroupDefType*             groupConfigPtr;
  const Adc_GroupType                 nbrOfGroups;
}Adc_ConfigType;

#endif

#endif /* ADC_CONFIGTYPES_H_ */
