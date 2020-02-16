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

#ifndef ETH_TYPES_H_
#define ETH_TYPES_H_

#include "Eth_GeneralTypes.h"

/* This type defines the controller modes */
typedef enum{
	ETH_MODE_DOWN, /* Controller disabled */
	ETH_MODE_ACTIVE, /* Controller enabled*/
}Eth_ModeType;

/* Status supervision used for Development Error Detection. The state shall be available for debugging */
typedef enum{
	ETH_STATE_UNINIT, /* Driver is not yet configured */
	ETH_STATE_INIT, /* Driver is configured */
	ETH_STATE_ACTIVE, /* Driver is active */
}Eth_StateType;

typedef uint16 Eth_FrameType;

typedef uint32 Eth_DataType;

#endif /* ETH_TYPES_H_ */
