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

/** @file Adc.h
 * API and type definitions for ADC Driver.
 */

#ifndef ADC_H_
#define ADC_H_

#include "Std_Types.h"
#include "Modules.h"

#define ADC_VENDOR_ID			    VENDOR_ID_ARCCORE
#define ADC_MODULE_ID			    MODULE_ID_ADC
#define ADC_AR_MAJOR_VERSION  		3
#define ADC_AR_MINOR_VERSION 		1
#define ADC_AR_PATCH_VERSION		5

#define ADC_SW_MAJOR_VERSION  		1
#define ADC_SW_MINOR_VERSION 		0
#define ADC_SW_PATCH_VERSION		0

/** Group status. */
typedef enum
{
  ADC_IDLE,
  ADC_BUSY,
  ADC_COMPLETED,
  ADC_STREAM_COMPLETED,
}Adc_StatusType;


/* DET errors that the ADC can produce. */
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

/** API service ID's */
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
/** Access mode to group conversion results. */
typedef enum
{
  ADC_ACCESS_MODE_SINGLE,
  ADC_ACCESS_MODE_STREAMING
}Adc_GroupAccessModeType;

/** Group trigger source (Not supported). */
typedef enum
{
  ADC_TRIGG_SRC_HW,
  ADC_TRIGG_SRC_SW,
}Adc_TriggerSourceType;

/** HW trigger edge (Not supported). */
typedef enum
{
  ADC_NO_HW_TRIG,
  ADC_HW_TRIG_BOTH_EDGES,
  ADC_HW_TRIG_FALLING_EDGE,
  ADC_HW_TRIG_RISING_EDGE,
}Adc_HwTriggerSignalType;

/** Stream buffer type. */
typedef enum
{
  ADC_NO_STREAMING,
  ADC_STREAM_BUFFER_CIRCULAR,
  ADC_STREAM_BUFFER_LINEAR,
}Adc_StreamBufferModeType;

#include "Adc_Cfg.h"

/* Function interface. */
/** Initializes the ADC hardware units and driver. */
void Adc_Init (const Adc_ConfigType *ConfigPtr);
#if (ADC_DEINIT_API == STD_ON)
/** Returns all ADC HW Units to a state comparable to their power on reset state. */
void Adc_DeInit ();
#endif
/** Sets up the result buffer for a group. */
Std_ReturnType Adc_SetupResultBuffer (Adc_GroupType group, Adc_ValueGroupType *bufferPtr);
#if (ADC_ENABLE_START_STOP_GROUP_API == STD_ON)
/** Starts the conversion of all channels of the requested ADC Channel group. */
void           Adc_StartGroupConversion (Adc_GroupType group);
void           Adc_StopGroupConversion (Adc_GroupType group);
#endif
#if (ADC_READ_GROUP_API == STD_ON)
/** Reads results from last conversion into buffer */
Std_ReturnType Adc_ReadGroup (Adc_GroupType group, Adc_ValueGroupType *dataBufferPtr);
#endif

#if !defined(CFG_HC1X)
/** Reads results from last streaming conversion into buffer */
Adc_StreamNumSampleType Adc_GetStreamLastPointer(Adc_GroupType group, Adc_ValueGroupType** PtrToSamplePtr);
#endif

#if (ADC_GRP_NOTIF_CAPABILITY == STD_ON)
/** Enables the notification mechanism for the requested ADC Channel group. */
void           Adc_EnableGroupNotification (Adc_GroupType group);
/** Disables the notification mechanism for the requested ADC Channel group. */
void           Adc_DisableGroupNotification (Adc_GroupType group);
#endif
/** Returns the conversion status of the requested ADC Channel group. */
Adc_StatusType Adc_GetGroupStatus (Adc_GroupType group);

#endif /*ADC_H_*/

/** @} */

