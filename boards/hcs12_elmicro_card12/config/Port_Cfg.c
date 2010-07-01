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

#warning "This default file may only be used as an example!"

#include "Port.h"

const Port_ConfigType PortConfigData =
{
    .corePullUpRegister = 0x90,
    .coreReducedDriveRegister = 0x00,
    .modeRoutingRegister = 0x00,

      #if ( PORTA_CONFIGURABLE == STD_ON )
    .portADirection = 0x00,
    .portAOutValue = 0x00,
    .portAMask = 0xFF,
      #endif

      #if ( PORTB_CONFIGURABLE == STD_ON )
    .portBDirection 0x00,
    .portBOutValue 0x00,
    .portBMask = 0xFF,
      #endif

      #if ( PORTE_CONFIGURABLE == STD_ON )
    .portEDirection = 0x00,
    .portEOutValue = 0x00,
    .portEMask = 0xFF,
      #endif

      #if ( PORTK_CONFIGURABLE == STD_ON )
    .portKDirection = 0x00,
    .portKOutValue = 0x00,
    .portKMask = 0xFF,
      #endif

      #if ( PORTH_CONFIGURABLE == STD_ON )
    .portHDirection = 0x80,
    .portHOutValue = 0x00,
    .portHMask = 0xFF,
    .portHPullEnableRegister = 0x00,
    .portHPullPolarityRegsiter = 0x00,
    .portHReducedDriveRegister = 0x00,
      #endif

      #if ( PORTJ_CONFIGURABLE == STD_ON )
    .portJDirection = 0x00,
    .portJOutValue = 0x00,
    .portJMask = 0xFF,
    .portJPullEnableRegister = 0xC3,
    .portJPullPolarityRegsiter 0x00,
    .portJReducedDriveRegister = 0x00,
      #endif

      #if ( PORTM_CONFIGURABLE == STD_ON )
    .portMDirection = 0x00,
    .portMOutValue = 0x00,
    .portMMask = 0xFF,
    .portMPullEnableRegister = 0x00,
    .portMPullPolarityRegsiter = 0x00,
    .portMWiredModeRegsiter = 0x00,
    .portMReducedDriveRegister = 0x00,
      #endif

      #if ( PORTP_CONFIGURABLE == STD_ON )
    .portPDirection = 0x00,
    .portPOutValue = 0x00,
    .portPMask = 0xFF,
    .portPPullEnableRegister = 0x00,
    .portPPullPolarityRegsiter = 0x00,
    .portPReducedDriveRegister = 0x00,
      #endif

      #if ( PORTS_CONFIGURABLE == STD_ON )
    .portSDirection = 0x00,
    .portSOutValue = 0x00,
    .portSMask = 0xFF,
    .portSPullEnableRegister = 0xFF,
    .portSPullPolarityRegsiter = 0x00,
    .portSWiredModeRegsiter = 0x00,
    .portSReducedDriveRegister = 0x00,
      #endif

      #if ( PORTT_CONFIGURABLE == STD_ON )
    .portTDirection = 0x00,
    .portTOutValue = 0x00,
    .portTMask = 0xFF,
    .portTPullEnableRegister = 0x00,
    .portTPullPolarityRegsiter = 0x00,
    .portTReducedDriveRegister = 0x00,
      #endif
};

