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


#include <assert.h>
#include <stdlib.h>
//#include "System.h"
//#include "Modules.h"
#include "Mcu.h"
#include "Adc.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "Det.h"
#if defined(USE_KERNEL)
#include "Os.h"
#include "isr.h"
#endif


typedef enum
{
  ADC_UNINIT,
  ADC_INIT,
}Adc_StateType;

/* Conversion result register for ADC1. */
#define ADC1_DR_Address    ((u32)0x4001244C)

/* Function prototypes. */

/* Development error checking. */
#if (ADC_READ_GROUP_API == STD_ON)
static Std_ReturnType Adc_CheckReadGroup (Adc_GroupType group);
#endif
#if (ADC_ENABLE_START_STOP_GROUP_API == STD_ON)
static Std_ReturnType Adc_CheckStartGroupConversion (Adc_GroupType group);
static Std_ReturnType Adc_CheckStopGroupConversion (Adc_GroupType group);
#endif
static Std_ReturnType Adc_CheckInit (const Adc_ConfigType *ConfigPtr);
static Std_ReturnType Adc_CheckSetupResultBuffer (Adc_GroupType group);
static Std_ReturnType Adc_CheckDeInit (void);

static void Adc_Group0ConversionComplete (void);


static Adc_StateType adcState = ADC_UNINIT;

/* Pointer to configuration structure. */
static const Adc_ConfigType *AdcConfigPtr;

/* VALIDATION MACROS */
#if (defined(USE_DET))
	#define ADC_REPORT_ERROR(apiid, errorid) \
		Det_ReportError(MODULE_ID_ADC, 0, apiid, errorid)
#else
	#define ADC_REPORT_ERROR(apiid, errorid)
#endif

#if (ADC_DEV_ERROR_DETECT == STD_ON)
	#define ADC_VALIDATE(_exp, _apiid, _errid, ...) \
		if (!(_exp)) { \
			ADC_REPORT_ERROR( _apiid, _errid); \
			return __VA_ARGS__; \
		}
#else
	#define ADC_VALIDATE(_exp, _apiid, _errid, ...)
#endif

#define ADC_VALIDATE_INITIALIZED(apiid, ...) \
			ADC_VALIDATE( (adcState == ADC_INIT), apiid, ADC_E_UNINIT, __VA_ARGS__)
#define ADC_VALIDATE_NOT_INITIALIZED(apiid, ...) \
			ADC_VALIDATE( (adcState != ADC_INIT), apiid, ADC_E_ALREADY_INITIALIZED, __VA_ARGS__)

/* VALIDATION MACROS */

/* Validate functions used for development error check */
#if ( ADC_DEV_ERROR_DETECT == STD_ON )
Std_ReturnType ValidateInit(Adc_APIServiceIDType api)
{
	Std_ReturnType res = E_OK;
	if(!(ADC_INIT == adcState)) {
		Det_ReportError(MODULE_ID_ADC,0,api,ADC_E_UNINIT );
		res = E_NOT_OK;
	}
	return res;
}
Std_ReturnType ValidateGroup(Adc_GroupType group,Adc_APIServiceIDType api)
{
	Std_ReturnType res = E_OK;
	if(!((group >= 0) && (group < AdcConfig->nbrOfGroups))) {
		Det_ReportError(MODULE_ID_ADC,0,api,ADC_E_PARAM_GROUP );
		res = E_NOT_OK;
	}
	return res;
}
#endif

#if (ADC_DEINIT_API == STD_ON)
Std_ReturnType Adc_DeInit (const Adc_ConfigType *ConfigPtr)
{
	if (E_OK == Adc_CheckDeInit())
	{
	  DMA_DeInit(DMA1_Channel1);
	  ADC_DeInit(ADC1);

	  adcState = ADC_UNINIT;
	}

    return (E_OK);
}
#endif

Std_ReturnType Adc_Init (const Adc_ConfigType *ConfigPtr)
{
  Std_ReturnType returnValue;
  Adc_GroupType group;

  ADC_InitTypeDef ADC_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;

  ADC_TempSensorVrefintCmd(ENABLE);

  if (E_OK == Adc_CheckInit(ConfigPtr))
  {
    /* First of all, store the location of the configuration data. */
    AdcConfigPtr = ConfigPtr;

    /* DMA1 channel1 configuration ---------------------------------------------*/
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)ConfigPtr->groupConfigPtr->resultBuffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = ConfigPtr->groupConfigPtr->numberOfChannels;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

      // Connect interrupt to correct isr
	ISR_INSTALL_ISR2( "DMA1", Adc_Group0ConversionComplete, DMA1_Channel1_IRQn, 6, 0 );

    /* Enable DMA1 channel1 */
    DMA_Cmd(DMA1_Channel1, ENABLE);

    /* Enable the DMA1 Channel1 Transfer complete interrupt */
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

    /* ADC1 configuration ------------------------------------------------------*/
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = ConfigPtr->groupConfigPtr->numberOfChannels;
    ADC_Init(ADC1, &ADC_InitStructure);

    for (group = ADC_GROUP0; group < ConfigPtr->nbrOfGroups; group++)
    {
      /* ADC307. */
      ConfigPtr->groupConfigPtr[group].status->groupStatus = ADC_IDLE;
    }

    /* Enable ADC1 DMA */
    ADC_DMACmd(ADC1, ENABLE);

    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);

    /* Enable ADC1 reset calibaration register */
    ADC_ResetCalibration(ADC1);
    /* Check the end of ADC1 reset calibration register */
    while(ADC_GetResetCalibrationStatus(ADC1)) ;

    /* Start ADC1 calibaration */
    ADC_StartCalibration(ADC1);

    /* Check the end of ADC1 calibration */
    while(ADC_GetCalibrationStatus(ADC1)) ;

    /* Move on to INIT state. */
    adcState = ADC_INIT;
    returnValue = E_OK;
  }
  else
  {
    returnValue = E_NOT_OK;
  }

  return (returnValue);
}

Std_ReturnType Adc_SetupResultBuffer (Adc_GroupType group, Adc_ValueGroupType *bufferPtr)
{
  Std_ReturnType returnValue;

  /* Check for development errors. */
  if (E_OK == Adc_CheckSetupResultBuffer (group))
  {
    AdcConfigPtr->groupConfigPtr[group].status->resultBufferPtr = bufferPtr;
    DMA_Cmd(DMA1_Channel1, DISABLE);
    DMA1_Channel1->CMAR = (u32)AdcConfigPtr->groupConfigPtr[group].status->resultBufferPtr;
    DMA1_Channel1->CNDTR = AdcConfigPtr->groupConfigPtr[group].numberOfChannels;
    DMA_Cmd(DMA1_Channel1, ENABLE);
    returnValue = E_OK;
  }
  else
  {
    /* An error have been raised from Adc_CheckSetupResultBuffer(). */
    returnValue = E_NOT_OK;
  }

  return (returnValue);
}

#if (ADC_READ_GROUP_API == STD_ON)
Std_ReturnType Adc_ReadGroup (Adc_GroupType group, Adc_ValueGroupType *dataBufferPtr)
{
  Std_ReturnType returnValue;
  Adc_ChannelType channel;

  if (E_OK == Adc_CheckReadGroup (group))
  {
    if ((ADC_CONV_MODE_CONTINUOUS == AdcConfigPtr->groupConfigPtr[group].conversionMode) &&
         ((ADC_STREAM_COMPLETED    == AdcConfigPtr->groupConfigPtr[group].status->groupStatus) ||
          (ADC_COMPLETED           == AdcConfigPtr->groupConfigPtr[group].status->groupStatus)))
    {
      /* ADC329, ADC331. */
      AdcConfigPtr->groupConfigPtr[group].status->groupStatus = ADC_BUSY;
      returnValue = E_OK;

      /* Restart continous again after read */
      ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    }
    else if ((ADC_CONV_MODE_ONESHOT == AdcConfigPtr->groupConfigPtr[group].conversionMode) &&
             (ADC_STREAM_COMPLETED  == AdcConfigPtr->groupConfigPtr[group].status->groupStatus))
    {
      /* ADC330. */
      AdcConfigPtr->groupConfigPtr[group].status->groupStatus = ADC_IDLE;

      returnValue = E_OK;
    }
    else
    {
      /* Keep status. */
      returnValue = E_OK;
    }

    if (E_OK == returnValue)
    {
      /* Copy the result to application buffer. */
      for (channel = 0; channel < AdcConfigPtr->groupConfigPtr[group].numberOfChannels; channel++)
      {
        dataBufferPtr[channel] = AdcConfigPtr->groupConfigPtr[group].status->resultBufferPtr[channel];
      }
    }
  }
  else
  {
    /* An error have been raised from Adc_CheckReadGroup(). */
    returnValue = E_NOT_OK;
  }

  return (returnValue);
}
#endif

Adc_StatusType Adc_GetGroupStatus (Adc_GroupType group)
{
	Adc_StatusType returnValue;

#if ( ADC_DEV_ERROR_DETECT == STD_ON )
	if( (ValidateInit(ADC_GETGROUPSTATUS_ID) == E_NOT_OK) ||
		(ValidateGroup(group, ADC_GETGROUPSTATUS_ID) == E_NOT_OK))
	{
		returnValue = ADC_IDLE;
	}
	else
	{
		returnValue = AdcConfigPtr->groupConfigPtr[group].status->groupStatus;
	}
#else
  returnValue = AdcConfigPtr->groupConfigPtr[group].status->groupStatus;
#endif
  return (returnValue);
}


static void Adc_Group0ConversionComplete (void)
{
  Adc_GroupDefType *groupPtr = NULL;

  /* ISR for DMA. Clear interrupt flag.  */
  DMA_ClearFlag(DMA1_FLAG_TC1);

  // Check which group is busy, only one is allowed to be busy at a time in a hw unit
  for (uint8 index = 0; index < ADC_NBR_OF_GROUPS; index++)
  {
	  if(AdcConfigPtr->groupConfigPtr[index].status->groupStatus == ADC_BUSY)
	  {
		  groupPtr = (Adc_GroupDefType *)&AdcConfigPtr->groupConfigPtr[index];
		  break;
	  }
  }
  if(groupPtr != NULL)
  {
	  if(ADC_CONV_MODE_ONESHOT == groupPtr->conversionMode)
	  {
		  groupPtr->status->groupStatus = ADC_STREAM_COMPLETED;

		  /* Call notification if enabled. */
		#if (ADC_GRP_NOTIF_CAPABILITY == STD_ON)
		  if (groupPtr->status->notifictionEnable && groupPtr->groupCallback != NULL)
		  {
			  groupPtr->groupCallback();
		  }
		#endif
	  }
	  else{
		  //Only single access supported so far for continous
		  groupPtr->status->groupStatus = ADC_STREAM_COMPLETED;

		  /* Call notification if enabled. */
		#if (ADC_GRP_NOTIF_CAPABILITY == STD_ON)
		  if (groupPtr->status->notifictionEnable && groupPtr->groupCallback != NULL)
		  {
			  groupPtr->groupCallback();
		  }
		#endif
	  }
  }
}

#if (ADC_ENABLE_START_STOP_GROUP_API == STD_ON)
void Adc_StartGroupConversion (Adc_GroupType group)
{
  /* Run development error check. */
  if (E_OK == Adc_CheckStartGroupConversion (group))
  {
	  Adc_ChannelType channel;
	  Adc_ChannelType channelId;

	  /* Configure the channel queue. */
        ADC1->SQR1 = (AdcConfigPtr->groupConfigPtr[group].numberOfChannels -1) << 20;
        ADC1->SQR2 = 0;
        ADC1->SQR3 = 0;
      /* Loop through all channels and make the command queue. */
      for (channel = 0; channel < AdcConfigPtr->groupConfigPtr[group].numberOfChannels; channel++)
      {
        /* Get physical channel. */
        channelId = AdcConfigPtr->groupConfigPtr[group].channelList[channel];

        /* Configure channel as regular. */
        ADC_RegularChannelConfig(ADC1, channelId, channel+1,
        		AdcConfigPtr->channelConfigPtr [channel].adcChannelConvTime);

      }

    /* Set conversion mode. No need we run always in single shot and handle continous ourselves */
/*	if (AdcConfigPtr->groupConfigPtr[group].conversionMode == ADC_CONV_MODE_ONESHOT){
		ADC1->CR2 = ADC1->CR2 & ~(1<<1);
	}else{
		ADC1->CR2 = ADC1->CR2 | (1<<1);
	}*/

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    /* Set group state to BUSY. */
    AdcConfigPtr->groupConfigPtr[group].status->groupStatus = ADC_BUSY;
  }
  else
  {
    /* Error have been set within Adc_CheckStartGroupConversion(). */
  }
}

void Adc_StopGroupConversion (Adc_GroupType group)
{
  /* Run development error check. */
  if (E_OK == Adc_CheckStopGroupConversion (group))
  {
	    ADC_SoftwareStartConvCmd(ADC1, DISABLE);
  }
  else
  {
	/* Error have been set within Adc_CheckStartGroupConversion(). */
  }
}
#endif

#if (ADC_GRP_NOTIF_CAPABILITY == STD_ON)
void Adc_EnableGroupNotification (Adc_GroupType group)
{
	Std_ReturnType res;

#if ( ADC_DEV_ERROR_DETECT == STD_ON )
	if( (ValidateInit(ADC_ENABLEGROUPNOTIFICATION_ID) == E_NOT_OK) ||
		(ValidateGroup(group, ADC_ENABLEGROUPNOTIFICATION_ID) == E_NOT_OK))
	{
		res = E_NOT_OK;
	}
	else if (AdcConfigPtr->groupConfigPtr[group].groupCallback == NULL)
	{
		res = E_NOT_OK;
		Det_ReportError(MODULE_ID_ADC,0,ADC_ENABLEGROUPNOTIFICATION_ID ,ADC_E_NOTIF_CAPABILITY );
	}
	else
	{
		/* Nothing strange. Go on... */
		res = E_OK;
	}
#else
	res = E_OK;
#endif
	if (E_OK == res){
		AdcConfigPtr->groupConfigPtr[group].status->notifictionEnable = 1;
	}
}

void Adc_DisableGroupNotification (Adc_GroupType group)
{
	Std_ReturnType res;

#if ( ADC_DEV_ERROR_DETECT == STD_ON )
	if( (ValidateInit(ADC_DISABLEGROUPNOTIFICATION_ID) == E_NOT_OK) ||
		(ValidateGroup(group, ADC_DISABLEGROUPNOTIFICATION_ID) == E_NOT_OK))
	{
		res = E_NOT_OK;
	}
	else if (AdcConfigPtr->groupConfigPtr[group].groupCallback == NULL)
	{
		res = E_NOT_OK;
		Det_ReportError(MODULE_ID_ADC,0,ADC_DISABLEGROUPNOTIFICATION_ID ,ADC_E_NOTIF_CAPABILITY );
	}
	else
	{
		/* Nothing strange. Go on... */
		res = E_OK;
	}
#else
	res = E_OK;
#endif
	if (E_OK == res){
		AdcConfigPtr->groupConfigPtr[group].status->notifictionEnable = 0;
	}
}
#endif


/* Development error checking functions. */
#if (ADC_READ_GROUP_API == STD_ON)
static Std_ReturnType Adc_CheckReadGroup (Adc_GroupType group)
{
  Std_ReturnType returnValue;

#if ( ADC_DEV_ERROR_DETECT == STD_ON )

  if (ADC_UNINIT == adcState)
  {
    /* ADC296. */
    returnValue = E_NOT_OK;
    Det_ReportError(MODULE_ID_ADC,0,ADC_READGROUP_ID ,ADC_E_UNINIT );
  }
  else if ((group < ADC_GROUP0) || (group >= AdcConfigPtr->nbrOfGroups))
  {
    /* ADC152. */
    returnValue = E_NOT_OK;
    Det_ReportError(MODULE_ID_ADC,0,ADC_READGROUP_ID ,ADC_E_PARAM_GROUP );
  }
  else if (ADC_IDLE == AdcConfigPtr->groupConfigPtr[group].status->groupStatus)
  {
    /* ADC388. */
    returnValue = E_NOT_OK;
    Det_ReportError(MODULE_ID_ADC,0,ADC_READGROUP_ID ,ADC_E_IDLE );
  }
  else
  {
    /* Nothing strange. Go on... */
    returnValue = E_OK;
  }
#else
  returnValue = E_OK;
#endif
  return (returnValue);
}
#endif

#if (ADC_ENABLE_START_STOP_GROUP_API == STD_ON)
static Std_ReturnType Adc_CheckStartGroupConversion (Adc_GroupType group)
{
	  Std_ReturnType returnValue;

	#if ( ADC_DEV_ERROR_DETECT == STD_ON )

	  if( (ValidateInit(ADC_STARTGROUPCONVERSION_ID) == E_NOT_OK) ||
	      (ValidateGroup(group, ADC_STARTGROUPCONVERSION_ID) == E_NOT_OK))
	  {
		  returnValue = E_NOT_OK;
	  }
	  else if ( NULL == AdcConfigPtr->groupConfigPtr[group].status->resultBufferPtr )
	  {
	      /* ResultBuffer not set, ADC424 */
		  Det_ReportError(MODULE_ID_ADC,0,ADC_STARTGROUPCONVERSION_ID, ADC_E_BUFFER_UNINIT );
		  returnValue = E_NOT_OK;
	  }
	  else if (!(ADC_TRIGG_SRC_SW == AdcConfigPtr->groupConfigPtr[group].triggerSrc))
	  {
	    /* Wrong trig source, ADC133. */
	    Det_ReportError(MODULE_ID_ADC,0,ADC_STARTGROUPCONVERSION_ID, ADC_E_WRONG_TRIGG_SRC);
	    returnValue = E_NOT_OK;
	  }
	  else if (!((ADC_IDLE             == AdcConfigPtr->groupConfigPtr[group].status->groupStatus) ||
	             (ADC_STREAM_COMPLETED == AdcConfigPtr->groupConfigPtr[group].status->groupStatus)))
	  {
	    /* Group status not OK, ADC351, ADC428 */
	    Det_ReportError(MODULE_ID_ADC,0,ADC_STARTGROUPCONVERSION_ID, ADC_E_BUSY );

	    //returnValue = E_NOT_OK;
	    returnValue = E_OK;
	  }
	  else
	  {
	    returnValue = E_OK;
	  }
	#else
	  returnValue = E_OK;
	#endif

	  return (returnValue);
}
static Std_ReturnType Adc_CheckStopGroupConversion (Adc_GroupType group)
{
  Std_ReturnType returnValue;

#if ( ADC_DEV_ERROR_DETECT == STD_ON )
  if( (ValidateInit(ADC_STOPGROUPCONVERSION_ID) == E_NOT_OK) ||
      (ValidateGroup(group, ADC_STOPGROUPCONVERSION_ID) == E_NOT_OK))
  {
	  returnValue = E_NOT_OK;
  }
  else if (!(ADC_TRIGG_SRC_SW == AdcConfigPtr->groupConfigPtr[group].triggerSrc))
  {
	/* Wrong trig source, ADC164. */
	Det_ReportError(MODULE_ID_ADC,0,ADC_STOPGROUPCONVERSION_ID, ADC_E_WRONG_TRIGG_SRC);
	returnValue = E_NOT_OK;
  }
  else if (ADC_IDLE == AdcConfigPtr->groupConfigPtr[group].status->groupStatus)
  {
	/* Group status not OK, ADC241 */
	Det_ReportError(MODULE_ID_ADC,0,ADC_STOPGROUPCONVERSION_ID, ADC_E_IDLE );
	returnValue = E_NOT_OK;
  }
  else
  {
	returnValue = E_OK;
  }
#else
  returnValue = E_OK;
#endif

  return (returnValue);
}
#endif

static Std_ReturnType Adc_CheckInit (const Adc_ConfigType *ConfigPtr)
{
  Std_ReturnType returnValue;

#if ( ADC_DEV_ERROR_DETECT == STD_ON )
  if (!(ADC_UNINIT == adcState))
  {
    /* Oops, already initialised. */
    Det_ReportError(MODULE_ID_ADC,0,ADC_INIT_ID, ADC_E_ALREADY_INITIALIZED );
    returnValue = E_NOT_OK;
  }
  else if (ConfigPtr == NULL)
  {
    /* Wrong config! */
    Det_ReportError(MODULE_ID_ADC,0,ADC_INIT_ID, ADC_E_PARAM_CONFIG );
    returnValue = E_NOT_OK;
  }
  else
  {
    /* Looks good!! */
    returnValue = E_OK;
  }
#else
    returnValue = E_OK;
#endif
  return (returnValue);
}

static Std_ReturnType Adc_CheckDeInit (void)
{
	Std_ReturnType returnValue = E_OK;

#if ( ADC_DEV_ERROR_DETECT == STD_ON )
	if(ValidateInit(ADC_DEINIT_ID) == E_OK)
	{
		for (Adc_GroupType group = ADC_GROUP0; group < AdcConfigPtr->nbrOfGroups; group++)
		{
			/*  Check ADC is IDLE or COMPLETE*/
			if((AdcConfigPtr->groupConfigPtr[group].status->groupStatus != ADC_IDLE) && (AdcConfigPtr->groupConfigPtr[group].status->groupStatus != ADC_STREAM_COMPLETED))
			{
				Det_ReportError(MODULE_ID_ADC,0,ADC_DEINIT_ID, ADC_E_BUSY );
				returnValue = E_NOT_OK;
			}
		}
	}
	else
	{
		returnValue = E_NOT_OK;
	}
#else
	returnValue = E_OK;
#endif
	return (returnValue);
}

static Std_ReturnType Adc_CheckSetupResultBuffer (Adc_GroupType group)
{
	  Std_ReturnType returnValue = E_OK;

	#if ( ADC_DEV_ERROR_DETECT == STD_ON )
	  if(ValidateGroup(group, ADC_SETUPRESULTBUFFER_ID) == E_NOT_OK)
	  {
		  returnValue = E_NOT_OK;
	  }
	#else
	  returnValue = E_OK;
	#endif
	  return (returnValue);
}


