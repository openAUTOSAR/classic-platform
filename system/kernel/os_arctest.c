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

	Os_Sys.intNestCnt = level;

	return;
}



