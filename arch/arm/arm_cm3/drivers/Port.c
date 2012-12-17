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


#include "Port.h" /** @req PORT131 */
#include "stm32f10x.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#include "string.h"
#include "stm32f10x_gpio.h"

const uint32 *GpioBaseAddr[] =
{
	(uint32 *)GPIOA_BASE,
    (uint32 *)GPIOB_BASE,
    (uint32 *)GPIOC_BASE,
    (uint32 *)GPIOD_BASE,
    (uint32 *)GPIOE_BASE,
    (uint32 *)GPIOF_BASE,
    (uint32 *)GPIOG_BASE
};

const uint32 *GpioODRAddr[] =
{
    (uint32 *)&((GPIO_TypeDef *)GPIOA_BASE)->ODR,
    (uint32 *)&((GPIO_TypeDef *)GPIOB_BASE)->ODR,
    (uint32 *)&((GPIO_TypeDef *)GPIOC_BASE)->ODR,
    (uint32 *)&((GPIO_TypeDef *)GPIOD_BASE)->ODR,
    (uint32 *)&((GPIO_TypeDef *)GPIOE_BASE)->ODR,
    (uint32 *)&((GPIO_TypeDef *)GPIOF_BASE)->ODR,
    (uint32 *)&((GPIO_TypeDef *)GPIOG_BASE)->ODR
};

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
  volatile const uint32 *gpioAddr;

  for (uint8_t i = 0; i < configType->padCnt; i++)
  {
	  /* Configure pin. */
	  gpioAddr = GpioBaseAddr[i];
	  //*((GpioPinCnfMode_Type *)gpioAddr) = configType->padConfig[i];
	  *(uint32 *)gpioAddr = *(uint32 *)&configType->padConfig[i];
	  *(1+(uint32 *)gpioAddr) = *(1+(uint32 *)&configType->padConfig[i]);

	  /* Set default output level. */
	  gpioAddr = GpioODRAddr[i];
	  *((GpioPinOutLevel_Type *)gpioAddr) = configType->outConfig[i];
  }

	// Enable remaps
	for (int portIndex = 0; portIndex < configType->remapCount; portIndex++) {
		GPIO_PinRemapConfig(configType->remaps[portIndex], ENABLE);
	}

    _portState = PORT_INITIALIZED;
    _configPtr = (Port_ConfigType *)configType;
    return;
}

/** @req PORT141 */
/** @req PORT063 */
/** @req PORT054 */
/** @req PORT086 */
#if ( PORT_SET_PIN_DIRECTION_API == STD_ON )
void Port_SetPinDirection( Port_PinType pin, Port_PinDirectionType direction )
{
  VALIDATE_STATE_INIT(PORT_SET_PIN_DIRECTION_ID);

  uint32 * gpioAddr;
  static uint8 index, bit, reg;

  index = pin / 16;
  bit = pin % 16;
  reg = bit / 8;
  bit = (bit % 8) * 4;
  // Ex. DIO_CHANNEL_B8 = 24
  // index = 1 => GPIOB
  // bit = 8
  // reg = 1 => CRH
  // bit = 0 => MODE8, CNF8 => OK
  // 0  1  2
  // 0  4  8
  gpioAddr = (uint32 *)GpioBaseAddr[index];

  /* Adjust for CRL/CRH */
  gpioAddr += reg;

  *gpioAddr &= ~(0x000000F << bit);

  if (direction==PORT_PIN_IN)
  {
    /* Configure pin. */
	//*gpioAddr |= (GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF) << bit; // TODO shall this be added to conf?
	  *gpioAddr |= (GPIO_INPUT_MODE | GPIO_INPUT_PULLUP_CNF) << bit; // TODO shall this be added to conf?
  }
  else
  {
    *gpioAddr |= (GPIO_OUTPUT_2MHz_MODE | GPIO_OUTPUT_PUSHPULL_CNF) << bit; // TODO shall this be added to conf?
  }

  return;
}
#endif

/** @req PORT066 */
/** @req PORT142 */
/** @req PORT060 */
/** @req PORT061 */
void Port_RefreshPortDirection(void)
{
    VALIDATE_STATE_INIT(PORT_REFRESH_PORT_DIRECTION_ID);

    /* TODO Not implemented yet */
    return;
}

/** req PORT143 */
/** req PORT102 */
/** req PORT103 */
#if (PORT_VERSION_INFO_API == STD_ON)
void Port_GetVersionInfo(Std_VersionInfoType* versionInfo)
{
    VALIDATE_STATE_INIT(PORT_GET_VERSION_INFO_ID);
    memcpy(versionInfo, &_Port_VersionInfo, sizeof(Std_VersionInfoType));
    return;
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

    return;
}
#endif
