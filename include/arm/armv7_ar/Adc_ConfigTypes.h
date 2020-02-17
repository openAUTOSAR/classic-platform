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

typedef uint16_t Adc_ValueGroupType;
/* Group definitions. */

typedef enum
{
  INTERNAL_XADC_CALIB = 0,
  INVALID_1,
  INVALID_2,
  INVALID_3,
  INVALID_4,
  INTERNAL_VCCPINT,
  INTERNAL_VCCPAUX,
  INTERNAL_VCC_DDR,
  INTERNAL_TEMP,
  INTERNAL_VCCINT,
  INTERNAL_VCCAUX,
  VP_VN,
  INTERNAL_VREFP,
  INTERNAL_VREFN,
  INTERNAL_VCCBRAM,
  INVALID_5,
  VAUX_0,
  VAUX_1,
  VAUX_2,
  VAUX_3,
  VAUX_4,
  VAUX_5,
  VAUX_6,
  VAUX_7,
  VAUX_8,
  VAUX_9,
  VAUX_10,
  VAUX_11,
  VAUX_12,
  VAUX_13,
  VAUX_14,
  VAUX_15,
  ADC_NBR_OF_CHANNELS,
}Adc_ChannelType;

typedef enum
{
  DEFAULT_FOUR_CYCLES = 0,
  TEN_CYCLES
}ArcAdc_ChannelSettlingTimeType;

typedef enum
{
    DEFAULT_UNIPOLAR = 0,
    BIPOLAR
}ArcAdc_ChannelAnalogInputModeType;

/* Std-type, supplier defined */
// XADC peripheral supports prescaler value of 0-255.
// A prescaler value of 0, 1 or 2 will equal to a division factor of 2.
typedef uint8_t Adc_PrescaleType;
// Base address to the LOGIC IP CORE AXI interface
typedef uint32_t Adc_IpCoreBaseAddr;

typedef uint16_t Adc_StreamNumSampleType;

/* Non-standard type */
typedef struct
{
  // NOT SUPPORTED Adc_ClockSourceType clockSource;
    uint8_t             hwUnitId;
    Adc_PrescaleType    adcPrescale;
    Adc_IpCoreBaseAddr  adcIpCorebaseAdrr;
}Adc_HWConfigurationType;

/* Std-type, supplier defined */
typedef Adc_PrescaleType Adc_ConversionTimeType;

/* Channel definitions, std container */
typedef struct
{
  // NOT SUPPORTED Adc_ConversionTimeType adcChannelConvTime;
  // NOT SUPPORTED Adc_VoltageSourceType  adcChannelRefVoltSrcLow;
  // NOT SUPPORTED Adc_VoltageSourceType  adcChannelRefVoltSrcHigh;
  // NOT SUPPORTED Adc_ResolutionType     adcChannelResolution;
  // NOT SUPPORTED Adc_CalibrationType    adcChannelCalibrationEnable;
    Adc_ChannelType adcChannel;
    ArcAdc_ChannelSettlingTimeType adcChannelSettlingTime;
    ArcAdc_ChannelAnalogInputModeType adcChannelInputMode;
} Adc_ChannelConfigurationType;

typedef struct
{
  uint8 				    notifictionEnable;
  Adc_ValueGroupType        *resultBufferPtr;
  Adc_StatusType 		    groupStatus;
  Adc_StreamNumSampleType   currSampleCount;   /* Samples per group counter. =0 until first round of conversions complete
                                                  (all channels in group). Then =1 until second round of conversions complete and so on.*/
  Adc_ValueGroupType        *currResultBufPtr;   /* Streaming sample current buffer pointer */
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
  Adc_GroupAccessModeType      accessMode;
  Adc_GroupConvModeType        conversionMode;
  Adc_TriggerSourceType        triggerSrc;
  // NOT SUPPORTED  Adc_HwTriggerSignalType      hwTriggerSignal;
  // NOT SUPPORTED  Adc_HwTriggerTimerType       hwTriggerTimer;
  void                         (*groupCallback)(void);
  Adc_StreamBufferModeType     streamBufferMode;
  Adc_StreamNumSampleType      streamNumSamples;
  const Adc_ChannelType        *channelList;
  Adc_ValueGroupType           *resultBuffer;
  // NOT SUPPORTED  Adc_CommandType              *commandBuffer;
  const uint16_t               numberOfChannels;
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

extern const Adc_ConfigType Adc_GlobalConfig [];


#endif /* ADC_CONFIGTYPES_H_ */
