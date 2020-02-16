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


#include "Port.h"
#include "Det.h"
#include "string.h"
#include "Std_Types.h"
#include "bcm2835.h"

gpio_port_t IOPORT0;
gpio_port_t IOPORT1;

typedef enum
{
    PORT_UNINITIALIZED = 0, PORT_INITIALIZED,
} Port_StateType;

static Port_StateType _portState = PORT_UNINITIALIZED;
static Port_ConfigType * _configPtr = NULL;

/** @req PORT107 */
#if (PORT_DEV_ERROR_DETECT == STD_ON)
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

#else
#define VALIDATE_PARAM_CONFIG(_ptr,_api)
#define VALIDATE_STATE_INIT(_api)
#define VALIDATE_PARAM_PIN(_api)
#endif

#if (PORT_VERSION_INFO_API == STD_ON)
static Std_VersionInfoType _Port_VersionInfo =
{ .vendorID = (uint16)1, .moduleID = (uint16) MODULE_ID_PORT,
        .instanceID = (uint8)1,
        .sw_major_version = (uint8)PORT_SW_MAJOR_VERSION,
        .sw_minor_version = (uint8)PORT_SW_MINOR_VERSION,
        .sw_patch_version = (uint8)PORT_SW_PATCH_VERSION,
        .ar_major_version = (uint8)PORT_AR_MAJOR_VERSION,
        .ar_minor_version = (uint8)PORT_AR_MINOR_VERSION,
        .ar_patch_version = (uint8)PORT_AR_PATCH_VERSION, };
#endif

/*********************************************************
 *-------------------private funtion----------------------
 **********************************************************/
/*void bcm2835_gpio_fnsel(uint32_t gpio_pin, uint32_t gpio_fn)
{
  uint32_t gpfnbank = gpio_pin/10;
  uint32_t offset = (gpio_pin - (gpfnbank * 10)) * 3;
  volatile uint32_t *gpfnsel = &(*(volatile uint32_t *)(0x20200000)) + gpfnbank;
  *gpfnsel &= ~(0x07 << offset);
  *gpfnsel |= (gpio_fn << offset);
}*/

static void set_gpio_in(ioportid_t port, ioportmask_t mask) {
  int i;
  for (i = 0; i < 32; i++) {
    unsigned int bit = mask & 1;
    if (bit)
      bcm2835_GpioFnSel(i + port->pin_base, GPFN_IN);
    mask >>= 1;
  }
}

/**------------------------------------------------------------------------*/

void Port_Init(const Port_ConfigType *configType)
{
	VALIDATE_PARAM_CONFIG(configType, PORT_INIT_ID); /** @req PORT105 */

    //some cnfigure instructions
	IOPORT0.gpset = configType->gipo_select0;
	IOPORT0.gpclr = configType->gipo_clear0;
	IOPORT0.gplev = configType->gipo_level0;
	IOPORT0.gppudclk = configType->gipo_pudclk0;
	IOPORT0.pin_base = configType->gipo_pin_base;
	set_gpio_in(&IOPORT0, 0xFFFFFFFF);

	IOPORT1.gpset = configType->gipo_select1;
	IOPORT1.gpclr = configType->gipo_clear1;
	IOPORT1.gplev = configType->gipo_level1;
	IOPORT1.gppudclk = configType->gipo_pudclk1;
	IOPORT1.pin_base = configType->gipo_pin_base;
	set_gpio_in(&IOPORT1, 0xFFFFFFFF);

    _portState = PORT_INITIALIZED;
    _configPtr = (Port_ConfigType *)configType;
    return;
}


#if ( PORT_SET_PIN_DIRECTION_API == STD_ON )
void Port_SetPinDirection( Port_PinType pin, Port_PinDirectionType direction )
{
	VALIDATE_STATE_INIT(PORT_SET_PIN_DIRECTION_ID);
    //VALIDATE_PARAM_PIN(pin, PORT_SET_PIN_DIRECTION_ID);


	return;
}
#endif


void Port_RefreshPortDirection(void)
{
    VALIDATE_STATE_INIT(PORT_REFRESH_PORT_DIRECTION_ID);

    //TODO Not implemented yet  should add something here
    return;
}

//this is public
#if (PORT_VERSION_INFO_API == STD_ON)
void Port_GetVersionInfo(Std_VersionInfoType* versionInfo)
{
    VALIDATE_STATE_INIT(PORT_GET_VERSION_INFO_ID);
    memcpy(versionInfo, &_Port_VersionInfo, sizeof(Std_VersionInfoType));
    return;
}
#endif


#if (PORT_SET_PIN_MODE_API == STD_ON)
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode)
{
    VALIDATE_STATE_INIT(PORT_SET_PIN_MODE_ID);
    //VALIDATE_PARAM_PIN(Pin, PORT_SET_PIN_MODE_ID);

    //what?????

    return;
}
#endif
