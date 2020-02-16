/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 *
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with
 * the terms contained in the written license agreement between you and ArcCore,
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as
 * published by the Free Software Foundation and appearing in the file
 * LICENSE.GPL included in the packaging of this file or here
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

#ifndef ETHTRCV_TYPES_H_
#define ETHTRCV_TYPES_H_

#include "Eth_GeneralTypes.h"
#include "EthTrcv_Cfg.h"

typedef enum{
	ETHTRCV_MODE_DOWN,
	ETHTRCV_MODE_ACTIVE,
}EthTrcv_ModeType;

typedef enum{
	ETHTRCV_LINK_STATE_DOWN,
	ETHTRCV_LINK_STATE_ACTIVE,
}EthTrcv_LinkStateType;

typedef enum{
	ETHTRCV_STATE_UNINIT,
	ETHTRCV_STATE_INIT,
	ETHTRCV_STATE_ACTIVE,
}EthTrcv_StateType;

typedef enum{
	ETHTRCV_BAUD_RATE_10MBIT,
	ETHTRCV_BAUD_RATE_100MBIT,
}EthTrcv_BaudRateType;

typedef enum{
	ETHTRCV_DUPLEX_MODE_HALF,
	ETHTRCV_DUPLEX_MODE_FULL,
}EthTrcv_DuplexModeType;

#endif /* ETHTRCV_TYPES_H_ */

