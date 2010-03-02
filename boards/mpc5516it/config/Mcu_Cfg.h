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

/** @addtogroup Mcu MCU Driver
 *  @{ */

/** @file Mcu_Cfg.h
 *  Definitions of configuration parameters for MCU Driver.
 */

#ifndef MCU_CFG_H_
#define MCU_CFG_H_

#include "mpc55xx.h"

/** Enable Development Error Trace */
#define MCU_DEV_ERROR_DETECT 	STD_ON
/** Enable/disable the use of the function Mcu_PerformReset() */
#define MCU_PERFORM_RESET_API 	STD_ON
/** Build version info API */
#define MCU_VERSION_INFO_API 	STD_ON

#include "Std_Types.h"


/** Symbolic names for clock settings */
typedef enum {
  MCU_CLOCKTYPE_EXT_REF_80MHZ = 0,
  MCU_CLOCKTYPE_EXT_REF_66MHZ,
  MCU_NBR_OF_CLOCKS,
} Mcu_ClockType;


#define MCU_DEFAULT_CONFIG McuConfigData[0]

#endif /*MCU_CFG_H_*/
/** @} */
