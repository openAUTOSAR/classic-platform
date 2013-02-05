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

#include "Modules.h"
#include "Adc.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#include "Os.h"
#include "arc.h"
#include "Adc_Internal.h"

#ifndef CFG_MPC560X
#define ADC_NOF_GROUP_PER_CONTROLLER 100
#endif

/* Validate functions used for development error check */
#if ( ADC_DEV_ERROR_DETECT == STD_ON )
Std_ReturnType ValidateInit(Adc_StateType adcState, Adc_APIServiceIDType api)
{
	Std_ReturnType res = E_OK;
	if(!(ADC_INIT == adcState)) {
		Det_ReportError(MODULE_ID_ADC,0,api,ADC_E_UNINIT );
		res = E_NOT_OK;
	}
	return res;
}
Std_ReturnType ValidateGroup(const Adc_ConfigType *ConfigPtr, Adc_GroupType group,Adc_APIServiceIDType api)
{
	Std_ReturnType res = E_OK;
	if(!(((group % ADC_NOF_GROUP_PER_CONTROLLER) >= 0) && ((group % ADC_NOF_GROUP_PER_CONTROLLER) < ConfigPtr->nbrOfGroups))
     || ConfigPtr == 0) {
		Det_ReportError(MODULE_ID_ADC,0,api,ADC_E_PARAM_GROUP );
		res = E_NOT_OK;
	}
	return res;
}
#endif


Adc_StatusType Adc_InternalGetGroupStatus (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr, Adc_GroupType group)
{
	Adc_StatusType returnValue;

#if ( ADC_DEV_ERROR_DETECT == STD_ON )
	if( (ValidateInit(adcState, ADC_GETGROUPSTATUS_ID) == E_NOT_OK) ||
		(ValidateGroup(ConfigPtr, group, ADC_GETGROUPSTATUS_ID) == E_NOT_OK))
	{
		returnValue = ADC_IDLE;
	}
	else
	{
		returnValue = ConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].status->groupStatus;
	}
#else
  returnValue = ConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].status->groupStatus;
#endif
  return (returnValue);
}

#if (ADC_GRP_NOTIF_CAPABILITY == STD_ON)
void Adc_EnableInternalGroupNotification (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr, Adc_GroupType group)
{
	Std_ReturnType res;

#if ( ADC_DEV_ERROR_DETECT == STD_ON )
	if( (ValidateInit(adcState, ADC_ENABLEGROUPNOTIFICATION_ID) == E_NOT_OK) ||
		(ValidateGroup(ConfigPtr, group, ADC_ENABLEGROUPNOTIFICATION_ID) == E_NOT_OK))
	{
		res = E_NOT_OK;
	}
	else if (ConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].groupCallback == NULL)
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
		ConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].status->notifictionEnable = 1;
	}
}

void Adc_InternalDisableGroupNotification (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr, Adc_GroupType group)
{
	Std_ReturnType res;

#if ( ADC_DEV_ERROR_DETECT == STD_ON )
	if( (ValidateInit(adcState, ADC_DISABLEGROUPNOTIFICATION_ID) == E_NOT_OK) ||
		(ValidateGroup(ConfigPtr, group, ADC_DISABLEGROUPNOTIFICATION_ID) == E_NOT_OK))
	{
		res = E_NOT_OK;
	}
	else if (ConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].groupCallback == NULL)
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
		ConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].status->notifictionEnable = 0;
	}
}
#endif



/* Development error checking functions. */
#if (ADC_READ_GROUP_API == STD_ON)
Std_ReturnType Adc_CheckReadGroup (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr, Adc_GroupType group)
{
  Std_ReturnType returnValue = E_OK;

#if ( ADC_DEV_ERROR_DETECT == STD_ON )

  if( (ValidateInit(adcState, ADC_READGROUP_ID) == E_NOT_OK) ||
      (ValidateGroup(ConfigPtr, group, ADC_READGROUP_ID) == E_NOT_OK))
  {
	  returnValue = E_NOT_OK;
  }
  else if (ADC_IDLE == ConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].status->groupStatus)
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
#endif
  return (returnValue);
}
#endif

#if (ADC_ENABLE_START_STOP_GROUP_API == STD_ON)
Std_ReturnType Adc_CheckStartGroupConversion (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr, Adc_GroupType group)
{
  Std_ReturnType returnValue = E_OK;

#if ( ADC_DEV_ERROR_DETECT == STD_ON )

  if( (ValidateInit(adcState, ADC_STARTGROUPCONVERSION_ID) == E_NOT_OK) ||
      (ValidateGroup(ConfigPtr, group, ADC_STARTGROUPCONVERSION_ID) == E_NOT_OK))
  {
	  returnValue = E_NOT_OK;
  }
  else if ( NULL == ConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].status->resultBufferPtr )
  {
      /* ResultBuffer not set, ADC424 */
	  Det_ReportError(MODULE_ID_ADC,0,ADC_STARTGROUPCONVERSION_ID, ADC_E_BUFFER_UNINIT );
	  returnValue = E_NOT_OK;
  }
  else if (!(ADC_TRIGG_SRC_SW == ConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].triggerSrc))
  {
    /* Wrong trig source, ADC133. */
    Det_ReportError(MODULE_ID_ADC,0,ADC_STARTGROUPCONVERSION_ID, ADC_E_WRONG_TRIGG_SRC);
    returnValue = E_NOT_OK;
  }
  else if (!((ADC_IDLE             == ConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].status->groupStatus) ||
             (ADC_STREAM_COMPLETED == ConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].status->groupStatus)))
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
#endif

  return (returnValue);
}

Std_ReturnType Adc_CheckStopGroupConversion (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr, Adc_GroupType group)
{
  Std_ReturnType returnValue = E_OK;

#if ( ADC_DEV_ERROR_DETECT == STD_ON )
  if( (ValidateInit(adcState, ADC_STOPGROUPCONVERSION_ID) == E_NOT_OK) ||
      (ValidateGroup(ConfigPtr, group, ADC_STOPGROUPCONVERSION_ID) == E_NOT_OK))
  {
	  returnValue = E_NOT_OK;
  }
  else if (!(ADC_TRIGG_SRC_SW == ConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].triggerSrc))
  {
	/* Wrong trig source, ADC164. */
	Det_ReportError(MODULE_ID_ADC,0,ADC_STOPGROUPCONVERSION_ID, ADC_E_WRONG_TRIGG_SRC);
	returnValue = E_NOT_OK;
  }
  else if (ADC_IDLE == ConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].status->groupStatus)
  {
	/* Group status not OK, ADC241 */
	Det_ReportError(MODULE_ID_ADC,0,ADC_STOPGROUPCONVERSION_ID, ADC_E_IDLE );
	returnValue = E_NOT_OK;
  }
  else
  {
	returnValue = E_OK;
  }
#endif

  return (returnValue);
}
#endif

Std_ReturnType Adc_CheckInit (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr)
{
  Std_ReturnType returnValue = E_OK;

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
#endif
  return (returnValue);
}

#if (ADC_DEINIT_API == STD_ON)
Std_ReturnType Adc_CheckDeInit (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr)
{
	Std_ReturnType returnValue = E_OK;

#if ( ADC_DEV_ERROR_DETECT == STD_ON )
	if(ValidateInit(adcState, ADC_DEINIT_ID) == E_OK)
	{
		for (Adc_GroupType group = (Adc_GroupType)0; group < ConfigPtr->nbrOfGroups; group++)
		{
			/*  Check ADC is IDLE or COMPLETE*/
			if((ConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].status->groupStatus != ADC_IDLE) && (ConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].status->groupStatus != ADC_STREAM_COMPLETED))
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
#endif
	return (returnValue);
}
#endif

Std_ReturnType Adc_CheckSetupResultBuffer (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr, Adc_GroupType group)
{
  Std_ReturnType returnValue = E_OK;

#if ( ADC_DEV_ERROR_DETECT == STD_ON )
  if( (ValidateInit(adcState, ADC_SETUPRESULTBUFFER_ID) == E_NOT_OK) ||
	    (ValidateGroup(ConfigPtr, group, ADC_SETUPRESULTBUFFER_ID) == E_NOT_OK))
  {
	  returnValue = E_NOT_OK;
  }
#endif
  return (returnValue);
}

Std_ReturnType Adc_CheckGetStreamLastPointer (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr, Adc_GroupType group)
{
  Std_ReturnType returnValue = E_OK;

#if ( ADC_DEV_ERROR_DETECT == STD_ON )
  if( (ValidateInit(adcState, ADC_GETSTREAMLASTPOINTER_ID) == E_NOT_OK) ||
	  (ValidateGroup(ConfigPtr, group, ADC_GETSTREAMLASTPOINTER_ID) == E_NOT_OK))
  {
	  returnValue = E_NOT_OK;
  }
  else if(ConfigPtr->groupConfigPtr[group%ADC_NOF_GROUP_PER_CONTROLLER].status->groupStatus == ADC_IDLE)
  { /** @req ADC215 Check ADC is not in IDLE */
	Det_ReportError(MODULE_ID_ADC,0,ADC_GETSTREAMLASTPOINTER_ID, ADC_E_IDLE );
	returnValue = E_NOT_OK;
  }
#endif
  return (returnValue);
}



