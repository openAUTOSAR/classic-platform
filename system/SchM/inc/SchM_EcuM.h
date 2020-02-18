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


#ifndef SCHM_ECUM_H_
#define SCHM_ECUM_H_

#include "Os.h"

#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
#define SchM_Enter_EcuM_EA_0() SYS_CALL_SuspendOSInterrupts()
#define SchM_Exit_EcuM_EA_0() SYS_CALL_ResumeOSInterrupts()
#else
#define SchM_Enter_EcuM_EA_0() SuspendOSInterrupts()
#define SchM_Exit_EcuM_EA_0() ResumeOSInterrupts()
#endif // (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)


#if defined(CFG_SAFETY_PLATFORM)
#define SCHM_MAINFUNCTION_ECUM_ASIL(_state,_rv) SP_SCHM_MAINFUNCTION(_rv,ECUM_A0,EcuM_MainFunction_Partition_A0(_state))
#define SCHM_MAINFUNCTION_ECUM_QM(_state,_rv)   SP_SCHM_MAINFUNCTION(_rv,ECUM_QM,EcuM_MainFunction_Partition_QM(_state))
#else
#define SCHM_MAINFUNCTION_ECUM()    SCHM_MAINFUNCTION(ECUM,EcuM_MainFunction())
#endif // CFG_SAFETY_PLATFORM

/* Skip "instance", req INTEGR058 */
#if 0
#define SchM_Enter_EcuM(uint8 exclusiveArea )
#define SchM_Exit_EcuM(uint8 exclusiveArea )
#define SchM_ActMainFunction_EcuM(uint8 exclusiveArea )
#define SchM_CancelMainFunction_EcuM( uint8 exclusiveArea )
#endif

#endif /* SCHM_ECUM_H_ */
