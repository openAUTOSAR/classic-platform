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

#ifndef MCU_CFG_H_
#define MCU_CFG_H_

#include "mpc55xx.h"

#define MCU_DEV_ERROR_DETECT 	STD_ON
#define MCU_PERFORM_RESET_API 	STD_ON
#define MCU_VERSION_INFO_API 	STD_ON

#include "Std_Types.h"

/* FMPLL modes( atleast in 5553/5554 ) */
typedef enum {
	MCU_FMPLL_BYPASS = 0,
	MCU_FMPLL_EXTERNAL_REF,
	MCU_FMPLL_EXTERNAL_REF_NO_FM,
	MCU_FMPLL_DUAL_CONTROLLER_MODE,
} Mcu_FMPLLmode_t;

typedef enum {
  MCU_CLOCKTYPE_EXT_REF_80MHZ = 0,
  MCU_CLOCKTYPE_EXT_REF_66MHZ,
  MCU_NBR_OF_CLOCKS,
} Mcu_ClockType;

#define MCU_DEFAULT_CONFIG McuConfigData[0]

#endif /*MCU_CFG_H_*/
