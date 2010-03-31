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
#include "context.h"

/**
 * Function make sure that we switch to supervisor mode(rfi) before
 * we call a task for the first time.
 */

void Os_ArchFirstCall( void )
{
	// TODO: make switch here... for now just call func.
	Irq_Enable();
	os_sys.curr_pcb->entry();
}

void *Os_ArchGetStackPtr( void ) {

//	return (void *)__get_MSP();
}

unsigned int Os_ArchGetScSize( void ) {
	return CONTEXT_SIZE_W;
}


void Os_ArchSetupContext( OsPcbType *pcb ) {
	// TODO: Add lots of things here, see ppc55xx
//	uint16_t *context = (uint16_t *)pcb->stack.curr;
//	context[C_CONTEXT_OFF/4] = SC_PATTERN;

	/* Set LR to start function */
//	if( pcb->proc_type == PROC_EXTENDED ) {
//		context[VGPR_LR_OFF/4] = (uint16_t)Os_TaskStartExtended;
//	} else if( pcb->proc_type == PROC_BASIC ) {
//		context[VGPR_LR_OFF/4] = (uint16_t)Os_TaskStartBasic;
//	}
//	Os_StackSetEndmark(pcb);
// Os_ArchSetupContext_asm(pcb->stack.curr,NULL);
}

/**
 *
 * @param pcbPtr
 */

void Os_ArchSetTaskEntry(OsPcbType *pcbPtr ) {
	uint16_t *context_words = (uint16_t *)pcbPtr->stack.curr;
	uint8_t *context_bytes = (uint8_t *)pcbPtr->stack.curr;

	/* Set Return to start function */

	context_bytes[8] = OS_KERNEL_CODE_PPAGE;

	if( pcbPtr->proc_type == PROC_EXTENDED ) {
		context_words[8] = (uint16_t)Os_TaskStartExtended;
	} else if( pcbPtr->proc_type == PROC_BASIC ) {
		context_words[8] = (uint16_t)Os_TaskStartBasic;
	}
}


void Os_ArchInit( void ) {
	// nothing to do here, yet :)
}
