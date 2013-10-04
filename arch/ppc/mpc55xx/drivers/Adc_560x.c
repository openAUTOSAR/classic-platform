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


/* In order to support multiple hw units we need to match group to a certain hw controller.
 * We handle this in a very simple way i.e. group 0-99 -> hwUnitId 0, group 100-199 -> hwUnitId 1 etc.*
 */

/* ----------------------------[includes]------------------------------------*/

#include <assert.h>
#include <stdlib.h>
#include "Std_Types.h"
#include "mpc55xx.h"
#include "Modules.h"
#include "Mcu.h"
#include "Adc.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#include "Os.h"
#include "isr.h"
#include "irq.h"
#include "arc.h"
#include "Adc_Internal.h"
/* ----------------------------[private define]------------------------------*/


/* Uncomment and use DMA for 5606 only if you now what you are doing */
#define DONT_USE_DMA_IN_ADC_MPC560X

/* Are we gonna use Dma? */
#if ( defined(CFG_MPC5606S) && !defined(DONT_USE_DMA_IN_ADC_MPC560X) )
	#define ADC_USES_DMA
	#include "Dma.h"
#endif

#if ( defined(ADC_USES_DMA) && !defined(USE_DMA) )
	#error Adc is configured to use Dma but the module is not enabled.
#endif


#if defined(CFG_MPC5668)
#define ADC0_EOC_INT 	ADC_A_EOC
#define ADC0_ER_INT 		ADC_A_ERR
#define	ADC0_WD_INT		ADC_A_WD
#define ADC1_EOC_INT 	ADC_B_EOC
#define ADC1_ER_INT 		ADC_B_ERR
#define	ADC1_WD_INT		ADC_B_WD
#endif

#if defined(CFG_MPC5668)
#define ADC_BASE_ADDRESS 0xFFF80000
#else
#define ADC_BASE_ADDRESS 0xFFE00000
#endif

#define GET_HW_CONTROLLER(_controller) 	\
        					((struct ADC_tag *)(ADC_BASE_ADDRESS + 0x4000*(_controller)))

#define GET_HWUNITID_FROM_GROUP(_group) (_group / ADC_NOF_GROUP_PER_CONTROLLER)

/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/

/* static variable declarations */
static Adc_StateType adcState = ADC_UNINIT;
static const Adc_ConfigType *AdcGlobalConfigPtr;      /* Pointer to configuration structure. */

/* ----------------------------[private functions]---------------------------*/

/* Function prototypes. */
static void Adc_ConfigureADC (const Adc_ConfigType *AdcConfigPtr);
static void Adc_ConfigureADCInterrupts (const Adc_ConfigType *AdcConfigPtr);
void Adc_GroupConversionComplete (Adc_GroupType group, const Adc_ConfigType *AdcConfigPtr, volatile struct ADC_tag *hwPtr);

static const Adc_ConfigType * Adc_GetControllerConfigPtrFromHwUnitId(int unit)
{
	const Adc_ConfigType *AdcConfigPtr = NULL;

	if(adcState == ADC_INIT){
		for (int configId = 0; configId < ADC_ARC_CTRL_CONFIG_CNT; configId++) {
			if(unit == AdcGlobalConfigPtr[configId].hwConfigPtr->hwUnitId){
				AdcConfigPtr = &AdcGlobalConfigPtr[configId];
				break;
			}
		}
	}

	return AdcConfigPtr;
}

static const Adc_ConfigType * Adc_GetControllerConfigPtrFromGroupId(Adc_GroupType group)
{
	return Adc_GetControllerConfigPtrFromHwUnitId(GET_HWUNITID_FROM_GROUP(group));
}
/* ----------------------------[public functions]----------------------------*/

#if (ADC_DEINIT_API == STD_ON)
void Adc_DeInit ()
{
  volatile struct ADC_tag *hwPtr;
  boolean okToClear = TRUE;

  for (int configId = 0; configId < ADC_ARC_CTRL_CONFIG_CNT; configId++) {
	  const Adc_ConfigType *AdcConfigPtr = &AdcGlobalConfigPtr[configId];

	  if (E_OK == Adc_CheckDeInit(adcState, AdcConfigPtr))
	  {
		hwPtr = GET_HW_CONTROLLER(AdcGlobalConfigPtr[configId].hwConfigPtr->hwUnitId);
		for(Adc_GroupType group = (Adc_GroupType)0; group < AdcConfigPtr->nbrOfGroups; group++)
		{
		  /* Set group status to idle. */
		  AdcConfigPtr->groupConfigPtr[group].status->groupStatus = ADC_IDLE;
		}

		/* Disable DMA transfer*/
	#if !(defined(CFG_MPC5604B) || defined(CFG_MPC5602B))
		hwPtr->DMAE.B.DMAEN = 0;
	#endif
		/* Power down ADC */
		hwPtr->MCR.R = 0x0001;

		/* Disable all interrupt*/
		hwPtr->IMR.R = 0;
	  }
	  else
	  {
		/* Not ok to change adcState if any unit is running */
	    okToClear = FALSE;
	  }
	}

    if(okToClear)
    {
    	/* Clean internal status. */
    	AdcGlobalConfigPtr = (Adc_ConfigType *)NULL;
    	adcState = ADC_UNINIT;
    }
}
#endif

void Adc_Init (const Adc_ConfigType *ConfigPtr)
{
  if (E_OK == Adc_CheckInit(adcState, ConfigPtr))
  {
	/* First of all, store the location of the global configuration data. */
	AdcGlobalConfigPtr = ConfigPtr;

	for (int configId = 0; configId < ADC_ARC_CTRL_CONFIG_CNT; configId++)
	{
	  const Adc_ConfigType *AdcConfigPtr = &AdcGlobalConfigPtr[configId];

	  /* Enable ADC. */
      Adc_ConfigureADC(AdcConfigPtr);

	  Adc_ConfigureADCInterrupts(AdcConfigPtr);
	}

	/* Move on to INIT state. */
	adcState = ADC_INIT;
  }
}

Std_ReturnType Adc_SetupResultBuffer (Adc_GroupType group, Adc_ValueGroupType *bufferPtr)
{
  Std_ReturnType returnValue = E_NOT_OK;

  const Adc_ConfigType *AdcConfigPtr = Adc_GetControllerConfigPtrFromGroupId(group);

  /* Check for development errors. */
  if (E_OK == Adc_CheckSetupResultBuffer (adcState, AdcConfigPtr, group))
  {
    AdcConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].status->resultBufferPtr = bufferPtr;
    
    returnValue = E_OK;
  }

  return (returnValue);
}

Adc_StreamNumSampleType Adc_GetStreamLastPointer(Adc_GroupType group, Adc_ValueGroupType** PtrToSamplePtr)
{
	const Adc_ConfigType *AdcConfigPtr = Adc_GetControllerConfigPtrFromGroupId(group);
	Adc_StreamNumSampleType nofSample = 0;
	
	/** @req ADC216 */
	/* Check for development errors. */
	if (E_OK == Adc_CheckGetStreamLastPointer (adcState, AdcConfigPtr, group))
	{
		Adc_GroupDefType *groupPtr = (Adc_GroupDefType *)&AdcConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER];

		if (groupPtr->status->groupStatus != ADC_BUSY)
	    {
			/* Set resultPtr to application buffer. */
			if(groupPtr->status->currSampleCount > 0){
				*PtrToSamplePtr = &groupPtr->status->resultBufferPtr[groupPtr->status->currSampleCount-1];
			}

			if ((ADC_CONV_MODE_ONESHOT == groupPtr->conversionMode) &&
				(ADC_STREAM_COMPLETED  == groupPtr->status->groupStatus))
			{
				/** @req ADC327. */
				groupPtr->status->groupStatus = ADC_IDLE;
			}
			else if ((ADC_CONV_MODE_CONTINOUS == groupPtr->conversionMode) &&
					 (ADC_ACCESS_MODE_STREAMING == groupPtr->accessMode) &&
					 (ADC_STREAM_BUFFER_LINEAR == groupPtr->streamBufferMode) &&
					 (ADC_STREAM_COMPLETED    == groupPtr->status->groupStatus))
			{
				/** @req ADC327. */
				groupPtr->status->groupStatus = ADC_IDLE;
			}
			else if ( (ADC_CONV_MODE_CONTINOUS == groupPtr->conversionMode) &&
					  ((ADC_STREAM_COMPLETED    == groupPtr->status->groupStatus) ||
					   (ADC_COMPLETED           == groupPtr->status->groupStatus)) )
			{
				/* Restart continous mode, and reset result buffer */
				if ((ADC_CONV_MODE_CONTINOUS == groupPtr->conversionMode) &&
					(ADC_STREAM_COMPLETED    == groupPtr->status->groupStatus))
				{
				  /* Start continous conversion again */
					Adc_StartGroupConversion(group);
				}
				/** @req ADC326 */
				/** @req ADC328 */
			}
			else{/* Keep status. */}
	    }
		else
		{
			/* Some condition not met */
			*PtrToSamplePtr = NULL;
		}
	}
	else
	{
		/* Some condition not met */
		*PtrToSamplePtr = NULL;
	}

	return nofSample;

}

#if (ADC_READ_GROUP_API == STD_ON)
Std_ReturnType Adc_ReadGroup (Adc_GroupType group, Adc_ValueGroupType *dataBufferPtr)
{
  Std_ReturnType returnValue = E_OK;
  uint8_t channel;
  const Adc_ConfigType *AdcConfigPtr = Adc_GetControllerConfigPtrFromGroupId(group);

  if (E_OK == Adc_CheckReadGroup (adcState, AdcConfigPtr, group))
  {
	Adc_GroupDefType *groupPtr = (Adc_GroupDefType *)&AdcConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER];

    /* Copy the result to application buffer. */
    for (channel = 0; channel < groupPtr->numberOfChannels; channel++)
	{
		if(groupPtr->status->currSampleCount > 0){
			dataBufferPtr[channel] = (&(groupPtr->status->resultBufferPtr[groupPtr->status->currSampleCount-1]))[channel];
		}else{
			dataBufferPtr[channel] = groupPtr->status->resultBufferPtr[channel];
		}
	}

    if ((ADC_CONV_MODE_ONESHOT == groupPtr->conversionMode) &&
        (ADC_STREAM_COMPLETED  == groupPtr->status->groupStatus))
    {
		/** @req ADC330. */
		groupPtr->status->groupStatus = ADC_IDLE;
    }
    else if ((ADC_CONV_MODE_CONTINOUS == groupPtr->conversionMode) &&
             (ADC_STREAM_BUFFER_LINEAR == groupPtr->streamBufferMode) &&
             (ADC_ACCESS_MODE_STREAMING == groupPtr->accessMode) &&
             (ADC_STREAM_COMPLETED    == groupPtr->status->groupStatus))
    {
		/** @req ADC330. */
		groupPtr->status->groupStatus = ADC_IDLE;
    }
    else if ((ADC_CONV_MODE_CONTINOUS == groupPtr->conversionMode) &&
             ((ADC_STREAM_COMPLETED    == groupPtr->status->groupStatus) ||
              (ADC_COMPLETED           == groupPtr->status->groupStatus)))
    {
    	/** @req ADC329 */
      /* Restart continous mode, and reset result buffer */
      if ((ADC_CONV_MODE_CONTINOUS == groupPtr->conversionMode) &&
          (ADC_STREAM_COMPLETED    == groupPtr->status->groupStatus))
      {
		  /* Start continous conversion again */
      	Adc_StartGroupConversion(group);
      }
      /** @req ADC329 */
      /** @req ADC331 */
    }
    else{/* Keep status. */}
  }
  else
  {
    /* An error have been raised from Adc_CheckReadGroup(). */
    returnValue = E_NOT_OK;
  }

  return (returnValue);
}
#endif

void Adc_GroupConversionComplete (Adc_GroupType group, const Adc_ConfigType *AdcConfigPtr, volatile struct ADC_tag *hwPtr)
{
  Adc_GroupDefType *adcGroup = (Adc_GroupDefType *)&AdcConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER];

  if(ADC_ACCESS_MODE_SINGLE == adcGroup->accessMode )
  {
	  adcGroup->status->groupStatus = ADC_STREAM_COMPLETED;

    /* Disable trigger normal conversions for ADC0 */
    hwPtr->MCR.B.NSTART=0;

	  /* Call notification if enabled. */
	#if (ADC_GRP_NOTIF_CAPABILITY == STD_ON)
	  if (adcGroup->status->notifictionEnable && adcGroup->groupCallback != NULL)
	  {
		  adcGroup->groupCallback();
	  }
	#endif
  }
  else
  {
	if(ADC_STREAM_BUFFER_LINEAR == adcGroup->streamBufferMode)
	{
		adcGroup->status->currSampleCount++;
		if(adcGroup->status->currSampleCount < adcGroup->streamNumSamples)
		{
		  adcGroup->status->currResultBufPtr += adcGroup->numberOfChannels;
		  adcGroup->status->groupStatus = ADC_COMPLETED;

#if defined (ADC_USES_DMA)
		  /* Increase current result buffer ptr */
		Dma_ConfigureDestinationAddress((uint32_t)adcGroup->status->currResultBufPtr, adcGroup->dmaResultChannel);
#endif

		hwPtr->IMR.B.MSKECH = 1;
	    hwPtr->MCR.B.NSTART=1;
		}
		else
		{
		  /* All sample completed. */
		  adcGroup->status->groupStatus = ADC_STREAM_COMPLETED;

      /* Disable trigger normal conversions for ADC0 */
      hwPtr->MCR.B.NSTART=0;

		  /* Call notification if enabled. */
		#if (ADC_GRP_NOTIF_CAPABILITY == STD_ON)
		  if (adcGroup->status->notifictionEnable && adcGroup->groupCallback != NULL){
			adcGroup->groupCallback();
		  }
		#endif
		}
	}
	else if(ADC_STREAM_BUFFER_CIRCULAR == adcGroup->streamBufferMode)
	{
		adcGroup->status->currSampleCount++;
		if(adcGroup->status->currSampleCount < adcGroup->streamNumSamples)
		{
			adcGroup->status->currResultBufPtr += adcGroup->numberOfChannels;
#if defined (ADC_USES_DMA)
			/* Increase current result buffer ptr */
			Dma_ConfigureDestinationAddress((uint32_t)adcGroup->status->currResultBufPtr, adcGroup->dmaResultChannel);
#endif
			adcGroup->status->groupStatus = ADC_COMPLETED;

			hwPtr->IMR.B.MSKECH = 1;
		    hwPtr->MCR.B.NSTART=1;
		}
		else
		{
		  /* Sample completed. */

		  /* Disable trigger normal conversions for ADC*/
		  hwPtr->MCR.B.NSTART=0;

		  adcGroup->status->groupStatus = ADC_STREAM_COMPLETED;
		  /* Call notification if enabled. */
		#if (ADC_GRP_NOTIF_CAPABILITY == STD_ON)
		  if (adcGroup->status->notifictionEnable && adcGroup->groupCallback != NULL)
		  {
			  adcGroup->groupCallback();
		  }
		#endif
		}
	}
	else
	{
		//nothing to do.
	}
  }
}

void Adc_Group0ConversionComplete (int unit)
{
	volatile struct ADC_tag *hwPtr = GET_HW_CONTROLLER(unit);
	const Adc_ConfigType *AdcConfigPtr = Adc_GetControllerConfigPtrFromHwUnitId(unit);

	/* Clear ECH Flag and disable interruput */
	hwPtr->ISR.B.ECH = 1;
	hwPtr->IMR.B.MSKECH = 0;

	// Check which group is busy, only one is allowed to be busy at a time in a hw unit
	for (int group = 0; group < AdcConfigPtr->nbrOfGroups; group++)
	{
	  if((AdcConfigPtr->groupConfigPtr[group].status->groupStatus == ADC_BUSY) ||
       (AdcConfigPtr->groupConfigPtr[group].status->groupStatus == ADC_COMPLETED))
	  {
#if !defined (ADC_USES_DMA)
		/* Copy to result buffer */
		for(uint8 index=0; index < AdcConfigPtr->groupConfigPtr[group].numberOfChannels; index++)
		{
#if defined(CFG_MPC5606S)
			AdcConfigPtr->groupConfigPtr[group].status->currResultBufPtr[index] = hwPtr->CDR[32+AdcConfigPtr->groupConfigPtr[group].channelList[index]].B.CDATA;
#else
			AdcConfigPtr->groupConfigPtr[group].status->currResultBufPtr[index] = hwPtr->CDR[AdcConfigPtr->groupConfigPtr[group].channelList[index]].B.CDATA;
#endif
		}
#endif

	    Adc_GroupConversionComplete((Adc_GroupType)group, AdcConfigPtr, hwPtr);
		break;
	  }
	}
}

static void Adc_Group0ConversionComplete_ADC0(void){
	Adc_Group0ConversionComplete(ADC_CTRL_0);
}

static void Adc_Group0ConversionComplete_ADC1(void){
	Adc_Group0ConversionComplete(ADC_CTRL_1);
}

void Adc_WatchdogError (void){
}
void Adc_ADCError (void){
}

static void  Adc_ConfigureADC (const Adc_ConfigType *AdcConfigPtr)
{
  volatile struct ADC_tag *hwPtr = GET_HW_CONTROLLER(AdcConfigPtr->hwConfigPtr->hwUnitId);

  /* Set ADC CLOCK */
  hwPtr->MCR.B.ADCLKSEL = AdcConfigPtr->hwConfigPtr->adcPrescale;

  hwPtr->DSDR.B.DSD = 254;

  /* Power on ADC */
  hwPtr->MCR.B.PWDN = 0;

#if defined(ADC_USES_DMA)
  /* Enable DMA. */
  hwPtr->DMAE.B.DMAEN = 1;
#endif
}

void Adc_ConfigureADCInterrupts (const Adc_ConfigType *AdcConfigPtr)
{
	if(AdcConfigPtr->hwConfigPtr->hwUnitId == 0)
	{
		ISR_INSTALL_ISR2(  "Adc_Err", Adc_ADCError, ADC0_ER_INT,     2, 0 );
		ISR_INSTALL_ISR2(  "Adc_Grp", Adc_Group0ConversionComplete_ADC0, ADC0_EOC_INT,     2, 0 );
		ISR_INSTALL_ISR2(  "Adc_Wdg", Adc_WatchdogError, ADC0_WD_INT,     2, 0 );
    }
	else if(AdcConfigPtr->hwConfigPtr->hwUnitId == 1)
	{
		ISR_INSTALL_ISR2(  "Adc_Err", Adc_ADCError, ADC1_ER_INT,     2, 0 );
		ISR_INSTALL_ISR2(  "Adc_Grp", Adc_Group0ConversionComplete_ADC1, ADC1_EOC_INT,     2, 0 );
		ISR_INSTALL_ISR2(  "Adc_Wdg", Adc_WatchdogError, ADC1_WD_INT,     2, 0 );
    }
	else
    {
    	assert(0);
    }
}

#if (ADC_ENABLE_START_STOP_GROUP_API == STD_ON)
void Adc_StartGroupConversion (Adc_GroupType group)
{
	const Adc_ConfigType *AdcConfigPtr = Adc_GetControllerConfigPtrFromGroupId(group);

	/* Run development error check. */
	if (E_OK == Adc_CheckStartGroupConversion (adcState, AdcConfigPtr, group))
	{
		volatile struct ADC_tag *hwPtr = GET_HW_CONTROLLER(AdcConfigPtr->hwConfigPtr->hwUnitId);

		Adc_GroupDefType *groupPtr = (Adc_GroupDefType *)&AdcConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER];

		/* Disable trigger normal conversions for ADC0 */
		hwPtr->MCR.B.NSTART = 0;

		/* Set group state to BUSY. */
		groupPtr->status->groupStatus = ADC_BUSY;

		groupPtr->status->currSampleCount = 0;
		groupPtr->status->currResultBufPtr = groupPtr->status->resultBufferPtr; /* Set current result buffer */

#if defined(ADC_USES_DMA)
		Dma_ConfigureChannel ((Dma_TcdType *)groupPtr->groupDMAResults, groupPtr->dmaResultChannel);
		Dma_ConfigureDestinationAddress ((uint32_t)groupPtr->status->currResultBufPtr, groupPtr->dmaResultChannel);
#endif
		/* Always use single shot in streaming mode */
		if( groupPtr->accessMode == ADC_ACCESS_MODE_STREAMING)
		{
			/* Set conversion mode. */
			hwPtr->MCR.B.MODE = ADC_CONV_MODE_ONESHOT;
		}
		else
		{
			/* Set conversion mode. */
			hwPtr->MCR.B.MODE = groupPtr->conversionMode;
		}

		/* Enable Overwrite*/
		hwPtr->MCR.B.OWREN = 1;

		/* Set Conversion Time. */
#if defined(CFG_MPC5606S)
		uint32 groupChannelIdMask = 0;

		hwPtr->CTR[1].B.INPLATCH = groupPtr->adcChannelConvTime.INPLATCH;
		hwPtr->CTR[1].B.INPCMP = groupPtr->adcChannelConvTime.INPCMP;
		hwPtr->CTR[1].B.INPSAMP = groupPtr->adcChannelConvTime.INPSAMP;

		for(uint8 i =0; i < groupPtr->numberOfChannels; i++)
		{
			groupChannelIdMask |= (1 << groupPtr->channelList[i]);
		}

#if defined(ADC_USES_DMA)
		hwPtr->DMAE.R = 0x01;
		/* Enable DMA Transfer */
		hwPtr->DMAR[1].R = groupChannelIdMask;
		Dma_StartChannel(DMA_ADC_GROUP0_RESULT_CHANNEL);        /* Enable EDMA channel for ADC */
#endif

		/* Enable Normal conversion */
		hwPtr->NCMR[1].R = groupChannelIdMask;

		/* Enable Channel Interrupt */
		hwPtr->CIMR[1].R = groupChannelIdMask;

#else
		uint32 groupChannelIdMask[3] = {0,0,0};

		hwPtr->CTR[0].B.INPLATCH = groupPtr->adcChannelConvTime.INPLATCH;
		hwPtr->CTR[0].B.INPCMP = groupPtr->adcChannelConvTime.INPCMP;
		hwPtr->CTR[0].B.INPSAMP = groupPtr->adcChannelConvTime.INPSAMP;
		hwPtr->CTR[1].B.INPLATCH = groupPtr->adcChannelConvTime.INPLATCH;
		hwPtr->CTR[1].B.INPCMP = groupPtr->adcChannelConvTime.INPCMP;
		hwPtr->CTR[1].B.INPSAMP = groupPtr->adcChannelConvTime.INPSAMP;
		hwPtr->CTR[2].B.INPLATCH = groupPtr->adcChannelConvTime.INPLATCH;
		hwPtr->CTR[2].B.INPCMP = groupPtr->adcChannelConvTime.INPCMP;
		hwPtr->CTR[2].B.INPSAMP = groupPtr->adcChannelConvTime.INPSAMP;

		for(uint8 i =0; i < groupPtr->numberOfChannels; i++)
		{
#if defined(CFG_MPC5668)
			if(groupPtr->channelList[i] <= 31){
#else
			if(groupPtr->channelList[i] <= 15){
#endif
				groupChannelIdMask[0] |= (1 << groupPtr->channelList[i]);
			}else if((groupPtr->channelList[i] >= 32) && (groupPtr->channelList[i] <=47)){
				groupChannelIdMask[1] |= (1 << (groupPtr->channelList[i] - 32));
			}else if((groupPtr->channelList[i] >= 64) && (groupPtr->channelList[i] <=95)){
				groupChannelIdMask[2] |= (1 << (groupPtr->channelList[i] - 64));
			}
		}

		/* Enable Normal conversion */
		hwPtr->NCMR[0].R = groupChannelIdMask[0];
		hwPtr->NCMR[1].R = groupChannelIdMask[1];
		hwPtr->NCMR[2].R = groupChannelIdMask[2];

		/* Enable Channel Interrupt */
		hwPtr->CIMR[0].R = groupChannelIdMask[0];
		hwPtr->CIMR[1].R = groupChannelIdMask[1];
		hwPtr->CIMR[2].R = groupChannelIdMask[2];
#endif
		/* Clear interrupts */
		hwPtr->ISR.B.ECH = 1;
		/* Enable ECH interrupt */
		hwPtr->IMR.B.MSKECH = 1;

		/* Trigger normal conversions for ADC0 */
		hwPtr->MCR.B.NSTART = 1;
	}
	else
	{
	/* Error have been set within Adc_CheckStartGroupConversion(). */
	}
}

void Adc_StopGroupConversion (Adc_GroupType group)
{
  const Adc_ConfigType *AdcConfigPtr = Adc_GetControllerConfigPtrFromGroupId(group);

  if (E_OK == Adc_CheckStopGroupConversion (adcState, AdcConfigPtr, group))
  {
	volatile struct ADC_tag *hwPtr = GET_HW_CONTROLLER(AdcConfigPtr->hwConfigPtr->hwUnitId);

	/* Disable trigger normal conversions for ADC0 */
	hwPtr->MCR.B.NSTART = 0;

	/* Set group state to IDLE. */
	AdcConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].status->groupStatus = ADC_IDLE;

	/* Disable group notification if enabled. */
    if(1 == AdcConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].status->notifictionEnable){
    	Adc_DisableGroupNotification (group);
    }
  }
  else
  {
	/* Error have been set within Adc_CheckStartGroupConversion(). */
  }
}
#endif  /* endof #if (ADC_ENABLE_START_STOP_GROUP_API == STD_ON) */

#if (ADC_GRP_NOTIF_CAPABILITY == STD_ON)
void Adc_EnableGroupNotification (Adc_GroupType group)
{
	Adc_EnableInternalGroupNotification(adcState, Adc_GetControllerConfigPtrFromGroupId(group), group);
}

void Adc_DisableGroupNotification (Adc_GroupType group)
{
	Adc_InternalDisableGroupNotification(adcState, Adc_GetControllerConfigPtrFromGroupId(group), group);
}
#endif

Adc_StatusType Adc_GetGroupStatus (Adc_GroupType group)
{
	return Adc_InternalGetGroupStatus(adcState, Adc_GetControllerConfigPtrFromGroupId(group), group);
}

