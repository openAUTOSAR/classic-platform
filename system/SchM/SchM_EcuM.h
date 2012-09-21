/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/


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
