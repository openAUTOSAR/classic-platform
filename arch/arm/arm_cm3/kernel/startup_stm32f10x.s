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
    
  .syntax unified
	.cpu cortex-m3
	.fpu softvfp
	.thumb

.global	g_pfnVectors
.global	Default_Handler

.word	_sidata
.word	_sdata
.word	_edata
.word	_sbss
.word	_ebss

/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called. 
 * @param  None
 * @retval : None
*/
    .section	.text.Reset_Handler
	.weak	Reset_Handler
	.type	Reset_Handler, %function
Reset_Handler:	

/* Copy the data segment initializers from flash to SRAM */
  ldr	r0, =_sdata       /* r0 holds start of data in ram */
  ldr	r3, =_edata       /* r3 holds end of data in ram */
  ldr	r5, =_sidata      /* r5 start of data in flash */
  movs	r1, #0
  b	LoopCopyDataInit

CopyDataInit:
	ldr	r4, [r5, r1]          /* read current position in flash */
	str	r4, [r0, r1]          /* store current position in ram */
	adds	r1, r1, #4        /* increment counter */
    
LoopCopyDataInit:
	adds	r2, r0, r1        /* are we at the final position? */
	cmp	r2, r3                /* ... */
	bcc	CopyDataInit          /* nope, continue */
	ldr	r2, =_sbss
	b	LoopFillZerobss

/* Zero fill the bss segment. */  
FillZerobss:
	movs	r3, #0
	str	r3, [r2], #4
    
LoopFillZerobss:
	ldr	r3, = _ebss
	cmp	r2, r3
	bcc	FillZerobss
/* Call the application's entry point.*/
	bl	main
	bx	lr    
.size	Reset_Handler, .-Reset_Handler

/**
 * @brief  This is the code that gets called when the processor receives an 
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 *
 * @param  None     
 * @retval : None       
*/
    .section	.text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
	b	Infinite_Loop
	.size	Default_Handler, .-Default_Handler
/******************************************************************************
* Vector table for a Cortex M3. Vectors start at addr 0x0.
******************************************************************************/    
 	.section	.isr_vector,"a",%progbits
	.type	g_pfnVectors, %object
	.size	g_pfnVectors, .-g_pfnVectors

	.extern Irq_Handler
	.extern SVC_Handler
	.extern PendSV_Handler

	.word	_estack
	.word	Reset_Handler
	.word	NMI_Handler
	.word	HardFault_Handler
	.word	MemManage_Handler
	.word	BusFault_Handler
	.word	UsageFault_Handler
	.word	0
	.word	0
	.word	0
	.word	0
	.word	SVC_Handler
	.word	DebugMon_Handler
	.word	0
	.word	PendSV_Handler
	.word   Irq_Handler+1		/* SysTick */
	.rept   83
	.word	Irq_Handler+1
	.endr
    
      .weak	NMI_Handler
	.thumb_set NMI_Handler,Default_Handler

  	.weak	HardFault_Handler
	.thumb_set HardFault_Handler,Default_Handler

  	.weak	MemManage_Handler
	.thumb_set MemManage_Handler,Default_Handler

  	.weak	BusFault_Handler
	.thumb_set BusFault_Handler,Default_Handler

	.weak	UsageFault_Handler
	.thumb_set UsageFault_Handler,Default_Handler

	.weak	SVC_Handler
	.thumb_set SVC_Handler,Default_Handler

	.weak	DebugMon_Handler
	.thumb_set DebugMon_Handler,Default_Handler

	.weak	PendSV_Handler
	.thumb_set PendSV_Handler,Default_Handler
