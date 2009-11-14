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

#define ADC_VENDOR_ID			    1
#define ADC_MODULE_ID			    MODULE_ID_ADC
#define ADC_AR_MAJOR_VERSION  		3
#define ADC_AR_MINOR_VERSION 		0
#define ADC_AR_PATCH_VERSION		3

#define ADC_SW_MAJOR_VERSION  		1
#define ADC_SW_MINOR_VERSION 		0
#define ADC_SW_PATCH_VERSION		0

/* Std-type */
typedef enum
{
  ADC_IDLE,
  ADC_BUSY,
  ADC_COMPLETED,
  ADC_STREAM_COMPLETED,
}Adc_StatusType;


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

/*
 * Std-types
 *
 */
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

typedef enum
{
  ADC_NO_STREAMING,
  ADC_STREAM_BUFFER_CIRCULAR,
  ADC_STREAM_BUFFER_LINEAR,
}Adc_StreamBufferModeType;

#include "Adc_Cfg.h"

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
