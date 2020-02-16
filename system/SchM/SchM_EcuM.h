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

/*
 * "Prototypes"
 */
#define SchM_Enter_EcuM(_area) CONCAT_(SchM_Enter_EcuM_,_area)
#define SchM_Exit_EcuM(_area) CONCAT_(SchM_Exit_EcuM_,_area)

/*
 * Exclusive Areas
 */

/* Lock interrupts */
#define EXCLUSIVE_AREA_0	0

#define SchM_Enter_EcuM_0 DisableAllInterrupts
#define SchM_Exit_EcuM_0  ResumeAllInterrupts

#define SCHM_MAINFUNCTION_ECUM() 	SCHM_MAINFUNCTION(ECUM,EcuM_MainFunction())


/* Skip "instance", req INTEGR058 */
#if 0
#define SchM_Enter_EcuM(uint8 exclusiveArea )
#define SchM_Exit_EcuM(uint8 exclusiveArea )
#define SchM_ActMainFunction_EcuM(uint8 exclusiveArea )
#define SchM_CancelMainFunction_EcuM( uint8 exclusiveArea )
#endif

#endif /* SCHM_ECUM_H_ */
