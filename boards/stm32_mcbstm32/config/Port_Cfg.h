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
#include "stm32f10x_gpio.h"

/** Build version info API */
#define PORT_VERSION_INFO_API				STD_ON  /** @req PORT100 PORT101 */
/** Enable Development Error Trace */
#define PORT_DEV_ERROR_DETECT				STD_ON
/** Build change pin direction API */
#define PORT_SET_PIN_DIRECTION_API	        STD_ON
/** Allow Pin mode changes during runtime (not avail on this CPU) */
#define PORT_SET_PIN_MODE_API               STD_ON

/** @req PORT124 */
typedef uint8 Port_PinModeType;

/** HW specific symbolic names of pins */
/** @req PORT013 */
typedef enum{
	PIN1,
} Port_PinType;

/* Configuration of one specific port */
typedef struct {
	uint16_t pinCount;
	GPIO_TypeDef *port;
  const GPIO_InitTypeDef pins[];

} Port_PortConfigType;

/** Top level configuration container */
/** @req PORT073 */
typedef struct {
  uint16_t remapCount;
  const uint32_t* remaps;

  uint16_t portCount;
  const Port_PortConfigType* ports[];
} Port_ConfigType;

/** Instance of the top level configuration container */
extern const Port_ConfigType PortConfigData;

#endif /*PORT_CFG_H_*/
