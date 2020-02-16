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

#ifndef ETHSM_TYPES_H_
#define ETHSM_TYPES_H_

/** @req ETHSM0006 */

#include "Std_Types.h"

typedef enum{
	ETHSM_UNINITED,
	ETHSM_NO_COMMUNICATION,
	ETHSM_FULL_COMMUNICATION,
}EthSM_NetworkModeStateType;

#endif /* ETHSM_TYPES_H_ */
