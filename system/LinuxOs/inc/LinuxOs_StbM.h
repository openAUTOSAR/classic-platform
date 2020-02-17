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

#ifndef LINUXOS_STBM_H_
#define LINUXOS_STBM_H_

#include "Rte_Type.h"
#include <Rte_StbM_Type.h>

/* @req 4.2.2/SWS_StbM_00052 */
void Linos_StbM_Init();

/* Deviation from @req 4.2.2/SWS_StbM_00233: added two parameters. */
Std_ReturnType Linos_StbM_BusSetGlobalTime(StbM_SynchronizedTimeBaseType timeBaseId,
        const StbM_TimeStampType* timeStampPtr,
        const StbM_UserDataType* userDataPtr, boolean syncToTimeBase,
        int ptp_in_sync, float ptp_offset);

/* @req 4.2.2/SWS_StbM_00195 */
Std_ReturnType Linos_StbM_GetCurrentTime(
        StbM_SynchronizedTimeBaseType timeBaseId,
        StbM_TimeStampType* timeStampPtr, StbM_UserDataType* userDataPtr);

#endif /* LINUXOS_STBM_H_ */
