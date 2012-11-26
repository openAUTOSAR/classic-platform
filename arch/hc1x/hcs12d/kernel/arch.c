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
#include "isr.h"
#include "sys.h"

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

#if defined(__IAR_SYSTEMS_ICC__)
// in arch_krn.sx

#else
void *Os_ArchGetStackPtr( void ) {
  void* val;

  asm("sts _.tmp");
  asm volatile("movw _.tmp, %0":"=m" (val));

  return val;
}
#endif


unsigned int Os_ArchGetScSize( void ) {
	return CONTEXT_SIZE_W;
}


void Os_ArchSetupContext( OsTaskVarType *pcb ) {
  // Nothing to be done here
}

/**
 *
 * @param pcbPtr
 */

#if defined(__ICCHCS12__)
void Os_ArchSetTaskEntry(OsTaskVarType *pcbPtr ) {
	uint8_t *context_bytes = (uint8_t *)pcbPtr->stack.curr;
	uint32_t temp;

	/* Set Return to start function */
	if( pcbPtr->constPtr->proc_type == PROC_EXTENDED ) {
		temp = (uint32_t)Os_TaskStartExtended;
	  context_bytes[0] = temp >> 16;
		context_bytes[HIGH_BYTE_RETURN_ADRESS] = temp >> 8;
		context_bytes[LOW_BYTE_RETURN_ADRESS] = temp & 0xFF;
	} else if( pcbPtr->constPtr->proc_type == PROC_BASIC ) {
		temp = (uint32_t)Os_TaskStartBasic;
	  context_bytes[0] = temp >> 16;
		context_bytes[HIGH_BYTE_RETURN_ADRESS] = temp >> 8;
		context_bytes[LOW_BYTE_RETURN_ADRESS] = temp & 0xFF;
	}
}
#else
void Os_ArchSetTaskEntry(OsTaskVarType *pcbPtr ) {
	uint8_t *context_bytes = (uint8_t *)pcbPtr->stack.curr;
	uint16_t temp;

	/* Set Return to start function */
	context_bytes[8] = OS_KERNEL_CODE_PPAGE;

	if( pcbPtr->constPtr->proc_type == PROC_EXTENDED ) {
		temp = (uint16_t)Os_TaskStartExtended;
		context_bytes[HIGH_BYTE_RETURN_ADRESS] = temp >> 8;
		context_bytes[LOW_BYTE_RETURN_ADRESS] = temp & 0xFF;
	} else if( pcbPtr->constPtr->proc_type == PROC_BASIC ) {
		temp = (uint16_t)Os_TaskStartBasic;
		context_bytes[HIGH_BYTE_RETURN_ADRESS] = temp >> 8;
		context_bytes[LOW_BYTE_RETURN_ADRESS] = temp & 0xFF;
	}
}
#endif

void Os_ArchInit( void ) {
  // Nothing to be done here
}
