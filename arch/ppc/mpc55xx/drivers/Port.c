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
#if defined(USE_DET)
#include "Det.h"
#endif
#if defined(USE_DEM)
#include "Dem.h"
#endif
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
		return; \
	}

#define VALIDATE_STATE_INIT(_api)\
	if(PORT_INITIALIZED!=_portState){\
		Det_ReportError(MODULE_ID_PORT, 0, _api, PORT_E_UNINIT ); \
		return; \
	}

#define VALIDATE_PARAM_PIN(_pin, _api)\
	if(_pin>sizeof(SIU.PCR)){\
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

void Port_Init(const Port_ConfigType *configType)
{
  VALIDATE_PARAM_CONFIG(configType, PORT_INIT_ID);

#if defined(CFG_MPC560X)
	vuint16_t i = 0;
	vuint16_t j = 0;

	while(i < (configType->padCnt/sizeof(uint16_t)))
    {
		SIU.PCR[i].R = configType->padConfig[i];
    	++i;

#if defined(CFG_MPC560XB)
    	if(32 == i || 33 == i) i=34;
    	if(121 == i || 122 == i) i=123;
#elif defined(CFG_MPC5606S)
    	// Out of reset pins PH[0:3](PCR99~PCR102) are available as JTAG pins(TCK,TDI,TDO and TMS respectively)
    	if(99 == i || 100 == i || 101 == i || 102 == i) i=103;
#endif
	}

	while(j < configType->outCnt)
	{
    	SIU.GPDO[j].B.PDO = configType->outConfig[j];
    	++j;
	}

#else
	// Pointers to the register memory areas
	vuint16_t * padConfig = &(SIU.PCR[0].R);
	vuint8_t * outConfig = &(SIU.GPDO[0].R);

	//  vuint8_t * inConfig = &(SIU.GPDI[0].R); // Read only
	// Copy config to register areas
	memcpy((void *)outConfig, configType->outConfig, configType->outCnt);
    memcpy((void *)padConfig, configType->padConfig, configType->padCnt);
	//memcpy((void *)inConfig, configType->inConfig, configType->inCnt);
#endif

	_portState = PORT_INITIALIZED;
	_configPtr = configType;

	return;
}

#if ( PORT_SET_PIN_DIRECTION_API == STD_ON )
void Port_SetPinDirection( Port_PinType pin, Port_PinDirectionType direction )
{
  VALIDATE_STATE_INIT(PORT_SET_PIN_DIRECTION_ID);
  VALIDATE_PARAM_PIN(pin, PORT_SET_PIN_DIRECTION_ID);
  unsigned long state;

  Irq_Save(state); // Lock interrupts
  if (direction==PORT_PIN_IN)
  {
    SIU.PCR[pin].B.IBE = 1;
    SIU.PCR[pin].B.OBE = 0;
  }
  else
  {
    SIU.PCR[pin].B.IBE = 0;
    SIU.PCR[pin].B.OBE = 1;
  }
  Irq_Restore(state); // Restore interrupts
  return;
}
#endif

void Port_RefreshPortDirection( void )
{
  VALIDATE_STATE_INIT(PORT_REFRESH_PORT_DIRECTION_ID);
  vuint16_t * pcrPtr = &(SIU.PCR[0].R);
  const uint16_t * padCfgPtr = _configPtr->padConfig;
  uint16_t bitMask = PORT_IBE_ENABLE|PORT_OBE_ENABLE;
  int i,j=0;
  unsigned long state;
  for (i=0; i < sizeof(SIU.PCR)/sizeof(SIU.PCR[0]); i++)
  {
	Irq_Save(state); // Lock interrupts
    *pcrPtr = (*pcrPtr & ~bitMask) | (*padCfgPtr & bitMask);
    Irq_Restore(state); // Restore interrupts
    pcrPtr++;
    padCfgPtr++;
#if defined(CFG_MPC560XB)
    if(32 == i)
    {
    	i=34;
    	pcrPtr = pcrPtr+2;
    	padCfgPtr = padCfgPtr+2;
    }
    if(121 == i)
    {
    	i=123;
    	pcrPtr = pcrPtr+2;
    	padCfgPtr = padCfgPtr+2;
    }
#elif defined(CFG_MPC5606S)
    if(98 == i)
    {
    	i=103;
    	pcrPtr = pcrPtr+5;
    	padCfgPtr = padCfgPtr+5;
    }
#endif

    j++;
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
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode)
{
  VALIDATE_STATE_INIT(PORT_SET_PIN_MODE_ID);
  VALIDATE_PARAM_PIN(Pin, PORT_SET_PIN_MODE_ID);
  //The pad configuration registers (SIU_PCR) in the SIU allow software control of the static electrical
  //characteristics of external pins. The PCRs can select the multiplexed function of a pin, selection of pullup
  //or pulldown devices, the slew rate of I/O signals, open drain mode for output pins, and hysteresis.
  SIU.PCR[Pin].R = Mode; // Put the selected mode to the PCR register
  return;
}
#endif
