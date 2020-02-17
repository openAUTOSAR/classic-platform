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


/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0 */

/** @fileSafetyClassification ASIL **/ /* Common file for safety platform */


#ifndef ECUM_SP_COMMON_H_
#define ECUM_SP_COMMON_H_

extern uint32 EcuM_World_run_state_timeout;

#if defined (USE_NVM)
extern uint32 EcuM_World_go_off_one_state_timeout;
extern NvM_RequestResultType writeAllResult;
#endif

static inline boolean hasRunRequests(void){
    uint32 result = EcuM_World.run_requests;

#if defined(USE_COMM)
    result |= EcuM_World_ComM_NVM.run_comm_requests;
#endif

    return ((result != 0) ? TRUE : FALSE);
}

static inline boolean hasPostRunRequests(void){
    return ((EcuM_World.postrun_requests != 0) ? TRUE : FALSE);
}

#endif /* ECUM_SP_COMMON_H_ */
