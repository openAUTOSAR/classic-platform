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









//#include "arch_offset.h"
#include "pcb.h"
#include "sys.h"
#include <stdlib.h>
#include "task_i.h"
#include "stack.h"
#include "arch_offset.h"
#include "stm32f10x.h"
#include "core_cm3.h"
#include "arch.h"


/**
 * Function make sure that we switch to supervisor mode(rfi) before
 * we call a task for the first time.
 */

void os_arch_first_call( void )
{
	// TODO: make switch here... for now just call func.
	Irq_Enable();
	os_sys.curr_pcb->entry();
}

void *os_arch_get_stackptr( void ) {

	return (void *)__get_MSP();
}

unsigned int os_arch_get_sc_size( void ) {
	return SC_SIZE;
}


void os_arch_setup_context( pcb_t *pcb ) {
	// TODO: Add lots of things here, see ppc55xx
	uint32_t *context = (uint32_t *)pcb->stack.curr;
	context[C_CONTEXT_OFFS/4] = SC_PATTERN;

	/* Set LR to start function */
	if( pcb->proc_type == PROC_EXTENDED ) {
		context[VGPR_LR_OFF/4] = (uint32_t)os_proc_start_extended;
	} else if( pcb->proc_type == PROC_BASIC ) {
		context[VGPR_LR_OFF/4] = (uint32_t)os_proc_start_basic;
	}
	os_arch_stack_set_endmark(pcb);
// os_arch_setup_context_asm(pcb->stack.curr,NULL);
}

void os_arch_init( void ) {
	// nothing to do here, yet :)
}
