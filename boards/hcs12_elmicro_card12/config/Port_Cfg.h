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

#ifndef PORT_CFG_H_
#define PORT_CFG_H_

#include "Std_Types.h"

/** Build version info API */
#define PORT_VERSION_INFO_API				STD_ON  /** @req PORT100 PORT101 */
/** Enable Development Error Trace */
#define PORT_DEV_ERROR_DETECT				STD_ON
/** Build change pin direction API */
#define PORT_SET_PIN_DIRECTION_API	        STD_ON
/** Allow Pin mode changes during runtime (not avail on this CPU) */
#define PORT_SET_PIN_MODE_API               STD_ON

/** Parameter to enable/disable configuration on a port */
#define PORTA_CONFIGURABLE  STD_OFF
#define PORTB_CONFIGURABLE  STD_OFF
#define PORTE_CONFIGURABLE  STD_OFF
#define PORTK_CONFIGURABLE  STD_OFF
#define PORTH_CONFIGURABLE  STD_ON
#define PORTJ_CONFIGURABLE  STD_OFF
#define PORTM_CONFIGURABLE  STD_OFF
#define PORTP_CONFIGURABLE  STD_OFF
#define PORTS_CONFIGURABLE  STD_OFF
#define PORTT_CONFIGURABLE  STD_OFF

#define PORT_A_BASE 0x0100
#define PORT_B_BASE 0x0200
#define PORT_E_BASE 0x0300
#define PORT_K_BASE 0x0400
#define PORT_H_BASE 0x0500
#define PORT_J_BASE 0x0600
#define PORT_M_BASE 0x0700
#define PORT_P_BASE 0x0800
#define PORT_S_BASE 0x0900
#define PORT_T_BASE 0x0A00
#define BITMASK  0x00FF
#define BASEMASK 0xFF00;

/** HW specific symbolic names of pins */
/** @req PORT013 */
typedef enum
{
  PA0 = PORT_A_BASE,
  PA1,
  PA2,
  PA3,
  PA4,
  PA5,
  PA6,
  PA7,
  PB0 = PORT_B_BASE,
  PB1,
  PB2,
  PB3,
  PB4,
  PB5,
  PB6,
  PB7,
  PE0 = PORT_E_BASE,
  PE1,
  PE2,
  PE3,
  PE4,
  PE5,
  PE6,
  PE7,
  PH0 = PORT_H_BASE,
  PH1,
  PH2,
  PH3,
  PH4,
  PH5,
  PH6,
  PH7,
  PJ0 = PORT_J_BASE,
  PJ1,
  PJ2,
  PJ3,
  PJ4,
  PJ5,
  PJ6,
  PJ7,
  PK0 = PORT_K_BASE,
  PK1,
  PK2,
  PK3,
  PK4,
  PK5,
  PK6,
  PK7,
  PM0 = PORT_M_BASE,
  PM1,
  PM2,
  PM3,
  PM4,
  PM5,
  PM6,
  PM7,
  PP0 = PORT_P_BASE,
  PP1,
  PP2,
  PP3,
  PP4,
  PP5,
  PP6,
  PP7,
  PS0 = PORT_S_BASE,
  PS1,
  PS2,
  PS3,
  PS4,
  PS5,
  PS6,
  PS7,
  PT0 = PORT_T_BASE,
  PT1,
  PT2,
  PT3,
  PT4,
  PT5,
  PT6,
  PT7,
} Port_PinType;

/** @req PORT124 */
typedef uint8 Port_PinModeType;

/** Top level configuration container */
/** @req PORT073 */
typedef struct
{
  const uint8_t corePullUpRegister; // PUCR
  const uint8_t coreReducedDriveRegister; // RDRIV
  const uint8_t modeRoutingRegister; // MODRR

  /**
   * Every port has:
   *  a direction (input or output)
   *  a default out value
   *  a mask that is 0 if the direction is allowed to change during runtime
   *
   * Some ports have:
   *  a reduced drive conf (power saving)
   *  a pull enable conf (enable pull-up/pull-down
   *  a pull direction conf (pull-up or pull-down)
   *  a wired mode (enable open drain outputs)
   */
#if ( PORTA_CONFIGURABLE == STD_ON )
  const uint8_t portADirection;
  const uint8_t portAOutValue;
  const uint8_t portAMask;
#endif

#if ( PORTB_CONFIGURABLE == STD_ON )
  const uint8_t portBDirection;
  const uint8_t portBOutValue;
  const uint8_t portBMask;
#endif

#if ( PORTE_CONFIGURABLE == STD_ON )
  const uint8_t portEDirection;
  const uint8_t portEOutValue;
  const uint8_t portEMask;
#endif

#if ( PORTK_CONFIGURABLE == STD_ON )
  const uint8_t portKDirection;
  const uint8_t portKOutValue;
  const uint8_t portKMask;
#endif

#if ( PORTH_CONFIGURABLE == STD_ON )
  const uint8_t portHDirection;
  const uint8_t portHOutValue;
  const uint8_t portHMask;
  const uint8_t portHPullEnableRegister;
  const uint8_t portHPullPolarityRegsiter;
  const uint8_t portHReducedDriveRegister;
#endif

#if ( PORTJ_CONFIGURABLE == STD_ON )
  const uint8_t portJDirection;
  const uint8_t portJOutValue;
  const uint8_t portJMask;
  const uint8_t portJPullEnableRegister;
  const uint8_t portJPullPolarityRegsiter;
  const uint8_t portJReducedDriveRegister;
#endif

#if ( PORTM_CONFIGURABLE == STD_ON )
  const uint8_t portMDirection;
  const uint8_t portMOutValue;
  const uint8_t portMMask;
  const uint8_t portMPullEnableRegister;
  const uint8_t portMPullPolarityRegsiter;
  const uint8_t portMWiredModeRegsiter;
  const uint8_t portMReducedDriveRegister;
#endif

#if ( PORTP_CONFIGURABLE == STD_ON )
  const uint8_t portPDirection;
  const uint8_t portPOutValue;
  const uint8_t portPMask;
  const uint8_t portPPullEnableRegister;
  const uint8_t portPPullPolarityRegsiter;
  const uint8_t portPReducedDriveRegister;
#endif

#if ( PORTS_CONFIGURABLE == STD_ON )
  const uint8_t portSDirection;
  const uint8_t portSOutValue;
  const uint8_t portSMask;
  const uint8_t portSPullEnableRegister;
  const uint8_t portSPullPolarityRegsiter;
  const uint8_t portSWiredModeRegsiter;
  const uint8_t portSReducedDriveRegister;
#endif

#if ( PORTT_CONFIGURABLE == STD_ON )
  const uint8_t portTDirection;
  const uint8_t portTOutValue;
  const uint8_t portTMask;
  const uint8_t portTPullEnableRegister;
  const uint8_t portTPullPolarityRegsiter;
  const uint8_t portTReducedDriveRegister;
#endif
} Port_ConfigType;

/** Instance of the top level configuration container */
extern const Port_ConfigType PortConfigData;

#endif /*PORT_CFG_H_*/
