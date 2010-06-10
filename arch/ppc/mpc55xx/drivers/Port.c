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
#include "mpc55xx.h"
#include "Det.h"
#include "Cpu.h"
#include <string.h>
/* SHORT ON HW
 *  Have a bunch of ports:
 *  - PAxx, input only port
 *  - PBxx to PKxx, i/o ports
 *  where xx is 0 to 15
 *  Regs:
 *    PCRx( Pad Config Register ) - register function(in, out, pull up/down, etc )
 *     0  - 15 : Port A
 *     16 - 43 : Port B to J
 *     144-145 : Port K
 *    GPD0( Pin Data Output Registers ) -
 *
 */

typedef enum
{
    PORT_UNINITIALIZED = 0, PORT_INITIALIZED,
} Port_StateType;

static Port_StateType _portState = PORT_UNINITIALIZED;
static const Port_ConfigType * _configPtr = &PortConfigData;

#if (PORT_DEV_ERROR_DETECT)
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

#define VALIDATE_PARAM_PIN(_pin, _api)\
	if(_pin>sizeof(SIU.PCR)){\
		Det_ReportError(MODULE_ID_PORT, 0, _api, PORT_E_PARAM_PIN ); \
		goto cleanup; \
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

void Port_Init(const Port_ConfigType *configType)
{
  VALIDATE_PARAM_CONFIG(configType, PORT_INIT_ID);

  // Pointers to the register memory areas
  vuint16_t * padConfig = &(SIU.PCR[0].R);
  vuint8_t * outConfig = &(SIU.GPDO[0].R);
//  vuint8_t * inConfig = &(SIU.GPDI[0].R);

  // Copy config to register areas
  memcpy((void *)outConfig, configType->outConfig, configType->outCnt);
  memcpy((void *)padConfig, configType->padConfig, configType->padCnt);
  //memcpy((void *)inConfig, configType->inConfig, configType->inCnt);
  _portState = PORT_INITIALIZED;
  _configPtr = configType;
  cleanup: return;
}

#if ( PORT_SET_PIN_DIRECTION_API == STD_ON )
void Port_SetPinDirection( Port_PinType pin, Port_PinDirectionType direction )
{
  VALIDATE_STATE_INIT(PORT_SET_PIN_DIRECTION_ID);
  VALIDATE_PARAM_PIN(pin, PORT_SET_PIN_DIRECTION_ID);
  unsigned long state;

  if (direction==PORT_PIN_IN)
  {
    state = _Irq_Disable_save(); // Lock interrupts
    SIU.PCR[pin].B.IBE = 1;
    SIU.PCR[pin].B.OBE = 0;
    _Irq_Disable_restore(state); // Restore interrupts
  }
  else
  {
    state = _Irq_Disable_save(); // Lock interrupts
    SIU.PCR[pin].B.IBE = 0;
    SIU.PCR[pin].B.OBE = 1;
    _Irq_Disable_restore(state); // Restore interrupts
  }
cleanup:return;
}
#endif

void Port_RefreshPortDirection( void )
{
  VALIDATE_STATE_INIT(PORT_REFRESH_PORT_DIRECTION_ID);
  vuint16_t * pcrPtr = &(SIU.PCR[0].R);
  const uint16_t * padCfgPtr = _configPtr->padConfig;
  uint16_t bitMask = PORT_IBE_ENABLE|PORT_OBE_ENABLE;
  int i;
  unsigned long state;
  for (i=0; i < sizeof(SIU.PCR); i++)
  {
    state = _Irq_Disable_save(); // Lock interrupts
    *pcrPtr = (*pcrPtr & ~bitMask) | (*padCfgPtr & bitMask);
    _Irq_Disable_restore(state); // Restore interrups
    pcrPtr++;
    padCfgPtr++;
  }

  cleanup:return;
}

#if PORT_VERSION_INFO_API == STD_ON
void Port_GetVersionInfo(Std_VersionInfoType* versionInfo)
{
  VALIDATE_STATE_INIT(PORT_GET_VERSION_INFO_ID);
  memcpy(versionInfo, &_Port_VersionInfo, sizeof(Std_VersionInfoType));
  cleanup: return;
}
#endif

#if (PORT_SET_PIN_MODE_API == STD_ON)
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode)
{
  VALIDATE_STATE_INIT(PORT_SET_PIN_MODE_ID);
  VALIDATE_PARAM_PIN(Pin, PORT_SET_PIN_MODE_ID);
  //The pad configuration registers (SIU_PCR) in the SIU allow software control of the static electrical
  //characteristics of external pins. The PCRs can select the multiplexed function of a pin, selection of pullup
  //or pulldown devices, the slew rate of I/O signals, open drain mode for output pins, and hysteresis.
  SIU.PCR[Pin].R = Mode; // Put the selected mode to the PCR register
  cleanup: return;
}
#endif
