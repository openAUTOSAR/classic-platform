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

#include "Std_Types.h"
#include "Port.h" /** @req PORT131 */
#include "Det.h"
#include "string.h"
#include "stm32f10x_rcc.h"


static Port_StateType _portState = PORT_UNINITIALIZED;
static Port_ConfigType * _configPtr = NULL;

/** @req PORT107 */
#if (PORT_DEV_ERROR_DETECT == STD_ON)
#define VALIDATE_PARAM_CONFIG(_ptr,_api) \
	if( (_ptr)==((void *)0) ) { \
		Det_ReportError(MODULE_ID_PORT, 0, _api, PORT_E_PARAM_CONFIG ); \
		goto cleanup; \
	}

#define VALIDATE_STATE_INIT(_api)\
	if(PORT_INITIALIZED!=_portState){\
		Det_ReportError(MODULE_ID_PORT, 0, _api, PORT_E_UNINIT ); \
		goto cleanup; \
	}

#else
#define VALIDATE_PARAM_CONFIG(_ptr,_api)
#define VALIDATE_STATE_INIT(_api)
#define VALIDATE_PARAM_PIN(_api)
#endif

static Std_VersionInfoType _Port_VersionInfo =
{ .vendorID = (uint16)1, .moduleID = (uint16) MODULE_ID_PORT,
        .instanceID = (uint8)1,
        .sw_major_version = (uint8)PORT_SW_MAJOR_VERSION,
        .sw_minor_version = (uint8)PORT_SW_MINOR_VERSION,
        .sw_patch_version = (uint8)PORT_SW_PATCH_VERSION,
        .ar_major_version = (uint8)PORT_AR_MAJOR_VERSION,
        .ar_minor_version = (uint8)PORT_AR_MINOR_VERSION,
        .ar_patch_version = (uint8)PORT_AR_PATCH_VERSION, };

/** @req PORT140 */
/** @req PORT041 Comment: To reduce flash usage the configuration tool can disable configuration of some ports  */
/** @req PORT078 See environment i.e Ecu State Manager */
/** @req PORT042 */
/** @req PORT113 Number 2 in list is applicable for all pins. */
/** @req PORT043 Comment: Output value is set before direction */
/** @req PORT071 See environment i.e Ecu State Manager */
/** @req PORT002 The _portState varialble is initialised. */
/** @req PORT003 See environment i.e Ecu State Manager */
/** @req PORT055 Comment: Output value is set before direction */
/** @req PORT121 */
void Port_Init(const Port_ConfigType *configType)
{
    VALIDATE_PARAM_CONFIG(configType, PORT_INIT_ID); /** @req PORT105 */

	uint16_t portIndex,pinIndex;

	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	  /* Enable DMA1 clock */
	  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	  /* Enable ADC1 and GPIOC clock */
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 |
				 RCC_APB2Periph_GPIOA |
				 RCC_APB2Periph_GPIOB |
				 RCC_APB2Periph_GPIOC,
				 ENABLE);

	  /* PWM: TIM4 clock enable */
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	// Set up all the ports
	for (portIndex = 0; portIndex < configType->portCount; portIndex++) {
		const Port_PortConfigType* portConfig = configType->ports[portIndex];

		// set up all pins
		for (pinIndex = 0; pinIndex < portConfig->pinCount; pinIndex++) {

			GPIO_Init(portConfig->port, &portConfig->pins[pinIndex]);

		}
	}

	// Enable remaps
	for (portIndex = 0; portIndex < configType->remapCount; portIndex++) {
		GPIO_PinRemapConfig(configType->remaps[portIndex], ENABLE);
	}

    _portState = PORT_INITIALIZED;
    _configPtr = configType;
    cleanup: return;
}

/** @req PORT141 */
/** @req PORT063 */
/** @req PORT054 */
/** @req PORT086 */
#if ( PORT_SET_PIN_DIRECTION_API == STD_ON )
void Port_SetPinDirection( Port_PinType pin, Port_PinDirectionType direction )
{
    VALIDATE_STATE_INIT(PORT_SET_PIN_DIRECTION_ID);

    {
        Det_ReportError(MODULE_ID_PORT, 0, PORT_SET_PIN_DIRECTION_ID, PORT_E_PARAM_PIN );
    }

    cleanup:return;
}
#endif

/** @req PORT066 */
/** @req PORT142 */
/** @req PORT060 */
/** @req PORT061 */
void Port_RefreshPortDirection(void)
{
    uint8_t curValue;
    VALIDATE_STATE_INIT(PORT_REFRESH_PORT_DIRECTION_ID);

    cleanup: return;
}

/** req PORT143 */
/** req PORT102 */
/** req PORT103 */
#if (PORT_VERSION_INFO_API == STD_ON)
void Port_GetVersionInfo(Std_VersionInfoType* versionInfo)
{
    VALIDATE_STATE_INIT(PORT_GET_VERSION_INFO_ID);
    memcpy(versionInfo, &_Port_VersionInfo, sizeof(Std_VersionInfoType));
    cleanup: return;
}
#endif

/** req PORT145 */
/** req PORT125 */
/** req PORT128 */
#if (PORT_SET_PIN_MODE_API == STD_ON)
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode)
{
    VALIDATE_STATE_INIT(PORT_SET_PIN_MODE_ID);

    // Mode of pins not changeable on this CPU
#if (PORT_DEV_ERROR_DETECT == STD_ON)
    Det_ReportError(MODULE_ID_PORT, 0, PORT_SET_PIN_MODE_ID, PORT_E_MODE_UNCHANGEABLE );
#endif

    cleanup: return;
}
#endif

