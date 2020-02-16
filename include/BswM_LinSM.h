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

#ifndef BSWM_LINSM_H_
#define BSWM_LINSM_H_

/* @req BswM0058 */
void BswM_LinSM_CurrentSchedule(NetworkHandleType Network, LinIf_SchHandleType CurrentSchedule);

/* @req BswM0052*/
void BswM_LinSM_CurrentState(NetworkHandleType Network, LinSM_ModeType CurrentState);

#endif /* BSWM_LINSM_H_ */
