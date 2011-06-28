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
#include "Mcu.h"
#include "Adc.h"
#include "Det.h"
#include "Os.h"
#include "isr.h"
#include "regs.h"
#include "arc.h"


// ATDCTL2
#define BM_ADPU 0x80
#define BM_AFFC 0x40
#define BM_AWAI 0x20
#define BM_ETRIGLE 0x10
#define BM_ETRIGP 0x08
#define BM_ETRIG_E 0x04
#define BM_ASCIE 0x02
#define BM_ASCIF 0x01

// ATDCTL3
#define BM_S8C 0x40
#define BM_S4C 0x20
#define BM_S2C 0x10
#define BM_S1C 0x08
#define BM_FIFO 0x04
#define BM_FRZ1 0x02
#define BM_FRZ0 0x01

// ATDCTL4
#define BM_SRES8 0x80
#define BM_SMP1 0x40
#define BM_SMP0 0x20
#define BM_PRS4 0x10
#define BM_PRS3 0x08
#define BM_PRS2 0x04
#define BM_PRS1 0x02
#define BM_PRS0 0x01

// ATDCTL5
#define BM_DJM 0x80
#define BM_DSGN 0x40
#define BM_SCAN 0x20
#define BM_MULT 0x10
#define BM_CC 0x04
#define BM_CB 0x02
#define BM_CA 0x01

typedef enum
{
  ADC_UNINIT,
  ADC_INIT,
}Adc_StateType;

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

static void Adc_GroupConversionComplete (void);

static Adc_StateType adcState = ADC_UNINIT;

/* Pointer to configuration structure. */
static const Adc_ConfigType *AdcConfigPtr;

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
	    /* Clean internal status. */
	    AdcConfigPtr = (Adc_ConfigType *)NULL;
	    adcState = ADC_UNINIT;
	}

	 return (E_OK);
}
#endif

Std_ReturnType Adc_Init (const Adc_ConfigType *ConfigPtr)
{
  Std_ReturnType returnValue;
  Adc_GroupType group;

  if (E_OK == Adc_CheckInit(ConfigPtr))
  {
    /* First of all, store the location of the configuration data. */
    AdcConfigPtr = ConfigPtr;

    // Connect interrupt to correct isr
	ISR_INSTALL_ISR2("ADC",Adc_GroupConversionComplete,IRQ_TYPE_ATD0,6,0);


	ATD0CTL2   = BM_ADPU | BM_AFFC | BM_ASCIE;	/* power enable, Fast Flag Clear, irq enable*/
    ATD0CTL3   = 0x03;	/* 8 conversions per sequence default, freeze enable */

    ATD0CTL4   = (ConfigPtr->hwConfigPtr->resolution << 7) |
    		     (ConfigPtr->hwConfigPtr->convTime << 5) |
    		      ConfigPtr->hwConfigPtr->adcPrescale;

    for (group = 0; group < ADC_NBR_OF_GROUPS; group++)
    {
      /* ADC307. */
      ConfigPtr->groupConfigPtr[group].status->groupStatus = ADC_IDLE;
    }

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
        dataBufferPtr[channel] = AdcConfigPtr->groupConfigPtr[group].resultBuffer[channel];
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

static void Adc_GroupConversionComplete (void)
{
  uint8 index;
  Adc_GroupDefType *groupPtr = NULL;
  /* Clear SCF flag. Not needed if AFFC is set but better to always do it  */
  ATD0STAT0 = SCF;

  // Check which group is busy, only one is allowed to be busy at a time in a hw unit
  for (index = 0; index < ADC_NBR_OF_GROUPS; index++)
  {
	  if(AdcConfigPtr->groupConfigPtr[index].status->groupStatus == ADC_BUSY)
	  {
		  groupPtr = (Adc_GroupDefType *)&AdcConfigPtr->groupConfigPtr[index];
		  break;
	  }
  }
  if(groupPtr != NULL)
  {
	  // Read hw buffer,
	  volatile uint16_t *ptr = &ATD0DR0;
	  for(index=0; index<groupPtr->numberOfChannels; index++)
	  {
		  groupPtr->resultBuffer[index] = *(ptr + groupPtr->channelList[index]);
	  }

	  /* Sample completed. */
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

#if (ADC_ENABLE_START_STOP_GROUP_API == STD_ON)
void Adc_StartGroupConversion (Adc_GroupType group)
{
  /* Run development error check. */
  if (E_OK == Adc_CheckStartGroupConversion (group))
  {
    /* Set single scan enable bit if this group is one shot. */
    if (AdcConfigPtr->groupConfigPtr[group].conversionMode == ADC_CONV_MODE_ONESHOT)
    {
	   /* Start next AD conversion. */
	   ATD0CTL5 = BM_DJM | BM_MULT;	/* 10010000 Right Justified,unsigned */
							        /* No SCAN/MULT/AD0 start select */
      /* Set group state to BUSY. */
      AdcConfigPtr->groupConfigPtr[group].status->groupStatus = ADC_BUSY;
    }
    else
    {
    	// Continous mode
 	   /* Start AD conversion. */
 	   ATD0CTL5 = BM_DJM | BM_MULT | BM_SCAN; /* 10010000 Right Justified,unsigned */
 							                  /* SCAN/MULT/AD0 start select */
       /* Set group state to BUSY. */
       AdcConfigPtr->groupConfigPtr[group].status->groupStatus = ADC_BUSY;
    }
  }
}

void Adc_StopGroupConversion (Adc_GroupType group)
{
  /* Run development error check. */
  if (E_OK == Adc_CheckStopGroupConversion (group))
  {
	   ATD0CTL3 = 0x03; /* Hard write to stop current conversion */
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
  else if ((group < ADC_GROUP0) || (group >= ADC_NBR_OF_GROUPS))
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

	    /*
	     * This is a BUG!
	     * Sometimes the ADC-interrupt gets lost which means that the status is never reset to ADC_IDLE (done in Adc_ReadGroup).
	     * Therefor another group conversion is never started...
	     *
	     * The temporary fix is to always return E_OK here. But the reason for the bug needs to be investigated further.
	     */
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


