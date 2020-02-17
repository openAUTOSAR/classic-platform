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

#ifndef SCHM_XCP_H_
#define SCHM_XCP_H_

#include "Os.h"

#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
#define SchM_Enter_Xcp_EA_0()           SYS_CALL_SuspendOSInterrupts()
#define SchM_Exit_Xcp_EA_0()            SYS_CALL_ResumeOSInterrupts()
#else
#define SchM_Enter_Xcp_EA_0() 			SuspendOSInterrupts()
#define SchM_Exit_Xcp_EA_0()  			ResumeOSInterrupts()
#endif

#define SCHM_MAINFUNCTION_XCP() 	SCHM_MAINFUNCTION(XCP,Xcp_MainFunction())



#endif /* SCHM_XCP_H_ */
