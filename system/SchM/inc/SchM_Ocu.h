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

#ifndef SCHM_OCU_H_
#define SCHM_OCU_H_

#include "Os.h"

#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
#define SchM_Enter_Ocu_EA_0()    SYS_CALL_SuspendOSInterrupts()
#define SchM_Exit_Ocu_EA_0()     SYS_CALL_ResumeOSInterrupts()
#else
#define SchM_Enter_Ocu_EA_0()    SuspendOSInterrupts()
#define SchM_Exit_Ocu_EA_0()     ResumeOSInterrupts()
#endif

/*
#define SCHM_MAINFUNCTION_CAN_WRITE() 	SCHM_MAINFUNCTION(CAN_WRITE,Can_MainFunction_Write())
#define SCHM_MAINFUNCTION_CAN_READ() 	SCHM_MAINFUNCTION(CAN_READ,Can_MainFunction_Read())
#define SCHM_MAINFUNCTION_CAN_BUSOFF() 	SCHM_MAINFUNCTION(CAN_BUSOFF,Can_MainFunction_BusOff())
#define SCHM_MAINFUNCTION_CAN_WAKEUP() 	SCHM_MAINFUNCTION(CAN_WAKEUP,Can_MainFunction_Wakeup())
#define SCHM_MAINFUNCTION_CAN_ERROR() 	SCHM_MAINFUNCTION(CAN_ERROR,Can_Arc_MainFunction_Error())
*/

#endif /* SCHM_OCU_H_ */
