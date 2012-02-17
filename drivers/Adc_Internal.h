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

#ifndef ADC_INTERNAL_H_
#define ADC_INTERNAL_H_

#include "Adc.h"

typedef enum
{
  ADC_UNINIT,
  ADC_INIT,
}Adc_StateType;

Std_ReturnType ValidateInit(Adc_StateType adcState, Adc_APIServiceIDType api);
Std_ReturnType ValidateGroup(const Adc_ConfigType *ConfigPtr, Adc_GroupType group,Adc_APIServiceIDType api);

Adc_StatusType Adc_InternalGetGroupStatus (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr, Adc_GroupType group);
void Adc_EnableInternalGroupNotification (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr, Adc_GroupType group);
void Adc_InternalDisableGroupNotification (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr, Adc_GroupType group);

/* Development error checking. */
Std_ReturnType Adc_CheckReadGroup (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr, Adc_GroupType group);
Std_ReturnType Adc_CheckStartGroupConversion (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr, Adc_GroupType group);
Std_ReturnType Adc_CheckStopGroupConversion (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr, Adc_GroupType group);
Std_ReturnType Adc_CheckInit (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr);
#if (ADC_DEINIT_API == STD_ON)
Std_ReturnType Adc_CheckDeInit (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr);
#endif
Std_ReturnType Adc_CheckSetupResultBuffer (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr, Adc_GroupType group);
Std_ReturnType Adc_CheckGetStreamLastPointer (Adc_StateType adcState, const Adc_ConfigType *ConfigPtr, Adc_GroupType group);

#endif /* ADC_INTERNAL_H_ */
