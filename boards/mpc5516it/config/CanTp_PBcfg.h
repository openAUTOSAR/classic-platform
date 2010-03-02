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


#ifndef CANTP_PBCFG_H_
#define CANTP_PBCFG_H_

#include "CanTp_Arc_Cfg_Types.h"

#define MAIN_FUNCTION_PERIOD_TIME_MS	1000

#define CANTP_RX_NSDU_CONFIG_LIST_CNT	1
#define CANTP_TX_NSDU_CONFIG_LIST_CNT	1

#define FRTP_CANCEL_TRANSMIT_REQUEST 	STD_ON
#define CANTP_VERSION_INFO_API          STD_ON   /**< Build version info API */
#define CANTP_DEV_ERROR_DETECT          STD_ON   /**< Enable Development Error Trace */


extern CanTp_ConfigType CanTpConfig;
extern const CanTp_RxNSduType CanTpRxNSduConfigList[];
extern const CanTp_TxNSduType CanTpTxNSduConfigList[];

#endif /* CANTP_PBCFG_H_ */
