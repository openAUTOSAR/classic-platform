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

#ifndef NM_EXTENSTION_H_
#define NM_EXTENSTION_H_

#include "NmStack_Types.h"
#include "ComStack_Types.h"

/* Functions needed for every NM extension */
void Nm_Extension_Init(void);

void Nm_Extension_WriteState(const NetworkHandleType nmNetworkHandle,
                             const Nm_StateType nmCurrentState);

void Nm_Extension_NetworkStartIndication(const NetworkHandleType ComMNetworkHandle);

void Nm_Extension_NetworkMode(const NetworkHandleType ComMNetworkHandle);

void Nm_Extension_PrepareBusSleepMode(const NetworkHandleType ComMNetworkHandle);

void Nm_Extension_BusSleepMode(const NetworkHandleType ComMNetworkHandle);



#endif
