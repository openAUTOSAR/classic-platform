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
#include "regs.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#include <string.h>

/* SHORT ON HW
 *  Have a bunch of ports:
 *  - PA, PB, PE, PK  is CPU registers (used as address bus if external RAM is used)
 *  - PH, PJ, PM, PP, PS, PT is peripheral registers
 */

typedef enum
{
    PORT_UNINITIALIZED = 0, PORT_INITIALIZED,
} Port_StateType;

static Port_StateType _portState = PORT_UNINITIALIZED;
static const Port_ConfigType * _configPtr = NULL;

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

#if PORT_VERSION_INFO_API == STD_ON
static Std_VersionInfoType _Port_VersionInfo =
{
		.vendorID = (uint16)1,
		.moduleID = (uint16) MODULE_ID_PORT,
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

    // Setup pin drives and pullups
    RDRIV = configType->coreReducedDriveRegister;
    PUCR = configType->corePullUpRegister;

    // CAN and SPI routing of pins
    MODDR = configType->modeRoutingRegister;

    // Setup ports
#if ( PORTA_CONFIGURABLE == STD_ON )
    PORTA = configType->portAOutValue;
    DDRA = configType->portADirection;
#endif

#if ( PORTB_CONFIGURABLE == STD_ON )
    PORTB = configType->portBOutValue;
    DDRB = configType->portBDirection;
#endif

#if ( PORTE_CONFIGURABLE == STD_ON )
    PORTE = configType->portEOutValue;
    DDRE = configType->portEDirection;
#endif

#if ( PORTK_CONFIGURABLE == STD_ON )
    PORTK = configType->portKOutValue;
    DDRK = configType->portKDirection;
#endif

#if ( PORTH_CONFIGURABLE == STD_ON )
    RDRH = configType->portHReducedDriveRegister;
    PERH = configType->portHPullEnableRegister;
    PPSH = configType->portHPullPolarityRegsiter;
    PTH = configType->portHOutValue;
    DDRH = configType->portHDirection;
#endif

#if ( PORTJ_CONFIGURABLE == STD_ON )
    RDRJ = configType->portJReducedDriveRegister;
    PERJ = configType->portJPullEnableRegister;
    PPSJ = configType->portJPullPolarityRegsiter;
    PTJ = configType->portJOutValue;
    DDRJ = configType->portJDirection;
#endif

#if ( PORTM_CONFIGURABLE == STD_ON )
    RDRM = configType->portMReducedDriveRegister;
    PERM = configType->portMPullEnableRegister;
    PPSM = configType->portMPullPolarityRegsiter;
    WOMM = configType->portMWiredModeRegsiter;
    PTM = configType->portMOutValue;
    DDRM = configType->portMDirection;
#endif

#if ( PORTP_CONFIGURABLE == STD_ON )
    RDRP = configType->portPReducedDriveRegister;
    PERP = configType->portPPullEnableRegister;
    PPSP = configType->portPPullPolarityRegsiter;
    PTP = configType->portPOutValue;
    DDRP = configType->portPDirection;
#endif

#if ( PORTS_CONFIGURABLE == STD_ON )
    RDRS = configType->portSReducedDriveRegister;
    PERS = configType->portSPullEnableRegister;
    PPSS = configType->portSPullPolarityRegsiter;
    WOMS = configType->portSWiredModeRegsiter;
    PTS = configType->portSOutValue;
    DDRS = configType->portSDirection;
#endif

#if ( PORTT_CONFIGURABLE == STD_ON )
    RDRT = configType->portTReducedDriveRegister;
    PERT = configType->portTPullEnableRegister;
    PPST = configType->portTPullPolarityRegsiter;
    PTT = configType->portTOutValue;
    DDRT = configType->portTDirection;
#endif

    _portState = PORT_INITIALIZED;
    _configPtr = configType;

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
    uint16_t base;
    uint8_t bitmask;
    uint8_t curValue;

    base = pin & PORT_BASEMASK;
    bitmask = 1 << (pin & PORT_BITMASK);
    if (base == PORT_A_BASE)
    {
        curValue = DDRA & ~(bitmask);
        if (direction == PORT_PIN_OUT)
        curValue = curValue | bitmask;
        DDRA = curValue;
    }
    else if (base == PORT_B_BASE)
    {
        curValue = DDRB & ~(bitmask);
        if (direction == PORT_PIN_OUT)
        curValue = curValue | bitmask;
        DDRB = curValue;
    }
    else if (base == PORT_E_BASE)
    {
        curValue = DDRE & ~(bitmask);
        if (direction == PORT_PIN_OUT)
        curValue = curValue | bitmask;
        DDRE = curValue;
    }
    else if (base == PORT_K_BASE)
    {
        curValue = DDRK & ~(bitmask);
        if (direction == PORT_PIN_OUT)
        curValue = curValue | bitmask;
        DDRK = curValue;
    }
    else if (base == PORT_H_BASE)
    {
        curValue = DDRH & ~(bitmask);
        if (direction == PORT_PIN_OUT)
        curValue = curValue | bitmask;
        DDRH = curValue;
    }
    else if (base == PORT_J_BASE)
    {
        curValue = DDRJ & ~(bitmask);
        if (direction == PORT_PIN_OUT)
        curValue = curValue | bitmask;
        DDRJ = curValue;
    }
    else if (base == PORT_M_BASE)
    {
        curValue = DDRM & ~(bitmask);
        if (direction == PORT_PIN_OUT)
        curValue = curValue | bitmask;
        DDRM = curValue;
    }
    else if (base == PORT_P_BASE)
    {
        curValue = DDRP & ~(bitmask);
        if (direction == PORT_PIN_OUT)
        curValue = curValue | bitmask;
        DDRP = curValue;
    }
    else if (base == PORT_S_BASE)
    {
        curValue = DDRS & ~(bitmask);
        if (direction == PORT_PIN_OUT)
        curValue = curValue | bitmask;
        DDRS = curValue;
    }
    else if (base == PORT_T_BASE)
    {
        curValue = DDRT & ~(bitmask);
        if (direction == PORT_PIN_OUT)
        curValue = curValue | bitmask;
        DDRT = curValue;
    }
    else
    {
        Det_ReportError(MODULE_ID_PORT, 0, PORT_SET_PIN_DIRECTION_ID, PORT_E_PARAM_PIN );
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
    uint8_t curValue;
    VALIDATE_STATE_INIT(PORT_REFRESH_PORT_DIRECTION_ID);

#if ( PORTA_CONFIGURABLE == STD_ON )
    curValue = DDRA & ~(_configPtr->portAMask);
    curValue |= (_configPtr->portADirection & _configPtr->portAMask);
    DDRA = curValue;
#endif

#if ( PORTB_CONFIGURABLE == STD_ON )
    curValue = DDRB & ~(_configPtr->portBMask);
    curValue |= (_configPtr->portBDirection & _configPtr->portBMask);
    DDRB = curValue;
#endif

#if ( PORTE_CONFIGURABLE == STD_ON )
    curValue = DDRE & ~(_configPtr->portEMask);
    curValue |= (_configPtr->portEDirection & _configPtr->portEMask);
    DDRE = curValue;
#endif

#if ( PORTK_CONFIGURABLE == STD_ON )
    curValue = DDRK & ~(_configPtr->portKMask);
    curValue |= (_configPtr->portKDirection & _configPtr->portKMask);
    DDRK = curValue;
#endif

#if ( PORTH_CONFIGURABLE == STD_ON )
    curValue = DDRH & ~(_configPtr->portHMask);
    curValue |= (_configPtr->portHDirection & _configPtr->portHMask);
    DDRH = curValue;
#endif

#if ( PORTJ_CONFIGURABLE == STD_ON )
    curValue = DDRJ & ~(_configPtr->portJMask);
    curValue |= (_configPtr->portJDirection & _configPtr->portJMask);
    DDRJ = curValue;
#endif

#if ( PORTM_CONFIGURABLE == STD_ON )
    curValue = DDRM & ~(_configPtr->portMMask);
    curValue |= (_configPtr->portMDirection & _configPtr->portMMask);
    DDRM = curValue;
#endif

#if ( PORTP_CONFIGURABLE == STD_ON )
    curValue = DDRP & ~(_configPtr->portPMask);
    curValue |= (_configPtr->portPDirection & _configPtr->portPMask);
    DDRP = curValue;
#endif

#if ( PORTS_CONFIGURABLE == STD_ON )
    curValue = DDRS & ~(_configPtr->portSMask);
    curValue |= (_configPtr->portSDirection & _configPtr->portSMask);
    DDRS = curValue;
#endif

#if ( PORTT_CONFIGURABLE == STD_ON )
    curValue = DDRT & ~(_configPtr->portTMask);
    curValue |= (_configPtr->portTDirection & _configPtr->portTMask);
    DDRT = curValue;
#endif

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
