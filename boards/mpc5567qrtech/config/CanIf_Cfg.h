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

/** @addtogroup CanIf CAN Interface
 *  @{ */

/** @file CanIf_Cfg.h
 */

#warning "This default file may only be used as an example!"

#ifndef CANIF_CFG_H_
#define CANIF_CFG_H_

#include "Can.h"

/** Identifiers for the elements in CanIfControllerConfig[].
 *  This is the ConfigurationIndex in CanIf_InitController(). */
typedef enum {
	CANIF_CHANNEL_0_CONFIG_0 = 0,

	CANIF_CHANNEL_1_CONFIG_0 = 1,

	CANIF_CHANNEL_CONFIGURATION_CNT
} CanIf_Arc_ConfigurationIndexType;

/** Channel id's */
typedef enum {
	CANIF_CHANNEL_0,
	CANIF_CHANNEL_1,
	CANIF_CHANNEL_CNT,
} CanIf_Arc_ChannelIdType;

#define CANIF_VERSION_INFO_API              STD_ON   /**< Build version info API */
#define CANIF_DEV_ERROR_DETECT              STD_ON   /**< Enable Development Error Trace */
#define CANIF_DLC_CHECK                     STD_ON   /**< Enable/disable DLC checking. */
#define CANIF_ARC_RUNTIME_PDU_CONFIGURATION	STD_OFF
#define CANIF_MULITPLE_DRIVER_SUPPORT       STD_OFF  /**< Not supported */
#define CANIF_READRXPDU_DATA_API            STD_OFF  /**< Not supported */
#define CANIF_READRXPDU_NOTIFY_STATUS_API	STD_OFF  /**< Not supported */
#define CANIF_READTXPDU_NOTIFY_STATUS_API	STD_OFF  /**< Not supported */
#define CANIF_SETDYNAMICTXID_API            STD_OFF  /**< Not supported */
#define CANIF_WAKEUP_EVENT_API              STD_OFF  /**< Not supported */
#define CANIF_TRANSCEIVER_API               STD_OFF  /**< Not supported */
#define CANIF_TRANSMIT_CANCELLATION         STD_OFF  /**< Not supported */

#include "CanIf_ConfigTypes.h"

/** Instance of the top level container. */
extern CanIf_ConfigType CanIf_Config;

#endif
/** @} */

