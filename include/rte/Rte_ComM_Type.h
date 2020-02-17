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

#ifndef RTE_COMM_TYPE_H_
#define RTE_COMM_TYPE_H_

#define COMMM_NOT_SERVICE_COMPONENT
#ifdef USE_RTE
#warning This file should only be used when not using an RTE with ComM service component.
#include "Rte_Type.h"
#else

/** Current mode of the Communication Manager (main state of the state machine). */
/**  @req COMM879 */
typedef uint8 ComM_ModeType; /** @req COMM867 @req COMM868 */

/** Inhibition status of ComM. */
typedef uint8 ComM_InhibitionStatusType;

typedef uint8 ComM_UserHandleType;

#endif
#endif // RTE_COMM_TYPE_H_
