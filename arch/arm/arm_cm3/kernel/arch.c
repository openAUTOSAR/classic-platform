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

#include "internal.h"
#include "Cpu.h"
#include "sys.h"
#include "arch_stack.h"
#include "stm32f10x.h"


/**
 * Function make sure that we switch to supervisor mode(rfi) before
 * we call a task for the first time.
 */

void Os_ArchFirstCall( void )
{
	// TODO: make switch here... for now just call func.
	Irq_Enable();
	Os_Sys.currTaskPtr->constPtr->entry();
}

void *Os_ArchGetStackPtr( void ) {

	return (void *)__get_MSP();
}

unsigned int Os_ArchGetScSize( void ) {
	return SC_SIZE;
}

void Os_ArchSetTaskEntry(OsTaskVarType *pcbPtr ) {
	// TODO: Add lots of things here, see ppc55xx
	uint32_t *context = (uint32_t *)pcbPtr->stack.curr;

	context[C_CONTEXT_OFFS/4] = SC_PATTERN;

	/* Set LR to start function */
	if( pcbPtr->constPtr->proc_type == PROC_EXTENDED ) {
		context[VGPR_LR_OFF/4] = (uint32_t)Os_TaskStartExtended;
	} else if( pcbPtr->constPtr->proc_type == PROC_BASIC ) {
		context[VGPR_LR_OFF/4] = (uint32_t)Os_TaskStartBasic;
	}

}

void Os_ArchSetupContext( OsTaskVarType *pcb ) {
	// TODO: Add lots of things here, see ppc55xx
	// uint32_t *context = (uint32_t *)pcb->stack.curr;

}

void Os_ArchInit( void ) {
	// nothing to do here, yet :)
}
