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

/** @addtogroup EcuM ECU State Manager
 *  @{ */

/** @file EcuM_Cfg.h
 *  Definitions of configuration parameters for ECU State Manager.
 *  Initialization sequences are defined in EcuM_Callout_template.c
 */

#ifndef ECUM_CFG_H_
#define ECUM_CFG_H_

/** Build version info API */
#define ECUM_VERSION_INFO_API STD_ON
/** Using NVRAM Manager */
#define ECUM_INCLUDE_NVRAM_MGR STD_OFF
/** Enable Development Error Trace */
#define ECUM_DEV_ERROR_DETECT STD_ON

#include "EcuM_Generated_Types.h"

#endif /*ECUM_CFG_H_*/
/** @} */
