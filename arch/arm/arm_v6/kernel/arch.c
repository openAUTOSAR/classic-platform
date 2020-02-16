/*
 * arch.c
 *
 *  Created on:  Mar 4, 2013
 *      Author:  Zhang Shuzhou
 *  Reviewed on:
 *     Reviewer:
 *
 */
#include "internal.h"
#include "Cpu.h"
#include "sys.h"
#include "arch_stack.h"
#include "bcm2835.h"
#include "stdio.h"


void Os_ArchFirstCall( void )
{
	Irq_Enable();
	Os_Sys[GetCoreID()].currTaskPtr->constPtr->entry();
}

void *Os_ArchGetStackPtr( void ) {

	 void *x;
	 asm volatile ("mov  %0, sp": "=r" (x));
	 return x;

}

unsigned int Os_ArchGetScSize( void ) {

	return SC_SIZE;
}

void Os_ArchSetTaskEntry(OsTaskVarType *pcbPtr ) {
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


}

void Os_ArchInit( void ) {
	// nothing to do here, yet :)
}

void Os_ArchPanic(void) {

     while(1) {
    	 printf("Shuzhou is in trouble\r\n");
     } ;

}

void Os_ArchTest(void *stack_p){
//	uint32 *stack;
//	stack = (uint32 *)stack_p;
}
