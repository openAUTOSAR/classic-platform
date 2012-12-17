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
#include "Port.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#include "Cpu.h"
#include <string.h>

#define GET_PIN_PORT(_pin) (_pin >> 8)
#define GET_PIN_PIN(_pin)  (_pin & 0x1F)
#define GET_PIN_MASK(_pin) (1 << (_pin & 0x1F))

typedef enum
{
    PORT_UNINITIALIZED = 0, PORT_INITIALIZED,
} Port_StateType;


typedef volatile struct
{
    uint32 FUN;
    uint32 DIR;
    uint32 DIN;
    uint32 DOUT;
    uint32 DSET;
    uint32 DCLR;
    uint32 PDR;
    uint32 PULDIS;
    uint32 PSL;
} Port_RegisterType;


#define PORT_NOT_CONFIGURED 0x00000000

#define PORT_0_BASE ((Port_RegisterType *)0xFFF7BC30)
#define PORT_1_BASE ((Port_RegisterType *)0xFFF7BC50)
#define PORT_2_BASE ((Port_RegisterType *)PORT_NOT_CONFIGURED)
#define PORT_3_BASE ((Port_RegisterType *)PORT_NOT_CONFIGURED)
#define PORT_4_BASE ((Port_RegisterType *)PORT_NOT_CONFIGURED)
#define PORT_5_BASE ((Port_RegisterType *)PORT_NOT_CONFIGURED)
#define PORT_6_BASE ((Port_RegisterType *)PORT_NOT_CONFIGURED)
#define PORT_7_BASE ((Port_RegisterType *)PORT_NOT_CONFIGURED)
#define PORT_8_BASE ((Port_RegisterType *)0xFFF7DDE0)
#define PORT_9_BASE ((Port_RegisterType *)0xFFF7DFE0)
#define PORT_10_BASE ((Port_RegisterType *)0xFFF7E1E0)
#define PORT_NUMBER_OF_PORTS 11

static Port_RegisterType * const Port_Base[] =
{
    PORT_0_BASE,
    PORT_1_BASE,
    PORT_2_BASE,
    PORT_3_BASE,
    PORT_4_BASE,
    PORT_5_BASE,
    PORT_6_BASE,
    PORT_7_BASE,
    PORT_8_BASE,
    PORT_9_BASE,
    PORT_10_BASE,
};



static Port_StateType _portState = PORT_UNINITIALIZED;
static const Port_ConfigType * _configPtr = &PortConfigData;

#if PORT_DEV_ERROR_DETECT == STD_ON
#define VALIDATE_PARAM_CONFIG(_ptr,_api) \
	if( (_ptr)==((void *)0) ) { \
		Det_ReportError(MODULE_ID_PORT, 0, _api, PORT_E_PARAM_CONFIG ); \
		return; \
	}

#define VALIDATE_STATE_INIT(_api)\
	if(PORT_INITIALIZED!=_portState){\
		Det_ReportError(MODULE_ID_PORT, 0, _api, PORT_E_UNINIT ); \
		return; \
	}

#define VALIDATE_PARAM_PIN(_pin, _api)\
	if(GET_PIN_PORT(_pin) >= PORT_NUMBER_OF_PORTS || Port_Base[GET_PIN_PORT(_pin)] == PORT_NOT_CONFIGURED || GET_PIN_PIN(_pin) > 7 ){\
		Det_ReportError(MODULE_ID_PORT, 0, _api, PORT_E_PARAM_PIN ); \
		return; \
	}

#else
#define VALIDATE_PARAM_CONFIG(_ptr,_api)
#define VALIDATE_STATE_INIT(_api)
#define VALIDATE_PARAM_PIN(_pin, _api)
#endif

#if PORT_VERSION_INFO_API == STD_ON
static Std_VersionInfoType _Port_VersionInfo =
{
  .vendorID   = (uint16)1,
  .moduleID   = (uint16) MODULE_ID_PORT,
  .instanceID = (uint8)1,
  .sw_major_version = (uint8)PORT_SW_MAJOR_VERSION,
  .sw_minor_version = (uint8)PORT_SW_MINOR_VERSION,
  .sw_patch_version = (uint8)PORT_SW_PATCH_VERSION,
  .ar_major_version = (uint8)PORT_AR_MAJOR_VERSION,
  .ar_minor_version = (uint8)PORT_AR_MINOR_VERSION,
  .ar_patch_version = (uint8)PORT_AR_PATCH_VERSION,
};
#endif

void Port_RefreshPin(uint16 pinNumber) {
	uint8 port = GET_PIN_PORT(_configPtr->pins[pinNumber].pin);
	uint32 mask = GET_PIN_MASK(_configPtr->pins[pinNumber].pin);
	uint16 conf = _configPtr->pins[pinNumber].conf;

	if (conf & PORT_FUNC) {
		// Don't do anything, let each driver configure???
		return;
	}

	// Set pin direction
	if (conf & PORT_PIN_OUT) {
		Port_Base[port]->DIR |= mask;

		// Set open drain
		if (conf & PORT_ODE_ENABLE) {
			Port_Base[port]->PDR |= mask;
		} else {
			Port_Base[port]->PDR &= ~mask;
		}

	} else {
		Port_Base[port]->DIR &= ~mask;
	}

	// Set pull up or down or nothing.
	if (conf & PORT_PULL_NONE) {
		Port_Base[port]->PULDIS |= mask;

	} else {
		Port_Base[port]->PULDIS &= ~mask;
		if (conf & PORT_PULL_UP) {
			Port_Base[port]->PSL |= mask;

		} else {
			Port_Base[port]->PSL &= ~mask;
		}
	}
}



void Port_Init(const Port_ConfigType *configType) {
	VALIDATE_PARAM_CONFIG(configType, PORT_INIT_ID);

	_configPtr = (Port_ConfigType *)configType;

	// Bring GIO register out of reset.
	gioREG->GCR0 = 1;

	for (uint16 i = 0; i < PORT_NUMBER_OF_PINS; i++) {
		Port_RefreshPin(i);
	}

	_portState = PORT_INITIALIZED;

	return;
}

#if ( PORT_SET_PIN_DIRECTION_API == STD_ON )
void Port_SetPinDirection( Port_PinType pin, Port_PinDirectionType direction )
{
	VALIDATE_STATE_INIT(PORT_SET_PIN_DIRECTION_ID);
	VALIDATE_PARAM_PIN(pin, PORT_SET_PIN_DIRECTION_ID);

	uint8 port = GET_PIN_PORT(pin);
	uint32 mask = GET_PIN_MASK(pin);

	if (direction & PORT_PIN_IN) {
		Port_Base[port]->DIR |= mask;

	} else {
		Port_Base[port]->DIR &= ~mask;

	}

	return;
}
#endif

void Port_RefreshPortDirection( void )
{
	VALIDATE_STATE_INIT(PORT_REFRESH_PORT_DIRECTION_ID);
	for (uint16 i = 0; i < PORT_NUMBER_OF_PINS; i++) {
		if (!(_configPtr->pins[i].conf & PORT_DIRECTION_CHANGEABLE)) {
			Port_RefreshPin(i);
		}
	}
	return;
}


#if PORT_VERSION_INFO_API == STD_ON
void Port_GetVersionInfo(Std_VersionInfoType* versionInfo)
{
  VALIDATE_STATE_INIT(PORT_GET_VERSION_INFO_ID);
  memcpy(versionInfo, &_Port_VersionInfo, sizeof(Std_VersionInfoType));
  return;
}
#endif

#if (PORT_SET_PIN_MODE_API == STD_ON)
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode) {
	VALIDATE_STATE_INIT(PORT_SET_PIN_MODE_ID);
	VALIDATE_PARAM_PIN(Pin, PORT_SET_PIN_MODE_ID);

	uint8 port = GET_PIN_PORT(Pin);
	uint8 pin = GET_PIN_PIN(Pin);
	uint32 mask = GET_PIN_MASK(Pin);

    Port_Base[port]->FUN &= ~mask;
    Port_Base[port]->FUN |= ((Mode & 1) << pin);
    return;
}
#endif
