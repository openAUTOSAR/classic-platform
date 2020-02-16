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
/**
 * Holds an API for manipulation of internal OS structures.
 * To be used by the test-system.
 */

#include "internal.h"
#include "task_i.h"
#include "sys.h"
#include "arc.h"

int Os_ArcTest_GetTaskActivationLimit( TaskType TaskId ) {

	return Os_TaskGet(TaskId)->constPtr->activationLimit;
}

/**
 * Set number of simultaneous interrupts.
 * Used to the test for E_OS_CALLLEVEL.
 * @param level
 */

void Os_ArcTest_SetIrqNestLevel( int level ) {

	OS_SYS_PTR->intNestCnt = level;

	return;
}



