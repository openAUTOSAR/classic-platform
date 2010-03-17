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


#ifndef MCU_CFG_H_
#define MCU_CFG_H_

#define MCU_DEV_ERROR_DETECT 	STD_ON
#define MCU_PERFORM_RESET_API 	STD_OFF
#define MCU_VERSION_INFO_API 	STD_ON


typedef enum {
  MCU_CLOCKTYPE_EXT_REF_16MHZ = 0,
  MCU_NBR_OF_CLOCKS,
} Mcu_ClockType;

#define MCU_DEFAULT_CONFIG McuConfigData[0]

#endif /*MCU_CFG_H_*/
