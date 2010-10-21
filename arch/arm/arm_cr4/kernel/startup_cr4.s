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
	.cpu cortex-r4
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

/* Set big endian state */
	SETEND BE

/* Copy the data segment initializers from flash to SRAM */
  	movs	r1, #0


Init_Registers:

        mov   r0,         #0x0000
        mov   r1,         #0x0000
        mov   r2,         #0x0000
        mov   r3,         #0x0000
        mov   r4,         #0x0000
        mov   r5,         #0x0000
        mov   r6,         #0x0000
        mov   r7,         #0x0000
        mov   r8,         #0x0000
        mov   r9,         #0x0000
        mov   r10,        #0x0000
        mov   r11,        #0x0000
        mov   r12,        #0x0000
        mov   r1,         #0x03D0
        orr   r2,        r1,     #0x0001
        msr   cpsr_cxsf,  r2
        msr   spsr_cxsf,  r2
        mov   r8,         #0x0000
        mov   r9,         #0x0000
        mov   r10,        #0x0000
        mov   r11,        #0x0000
        mov   r12,        #0x0000
        orr   r12,        r1,     #0x0002
        msr   cpsr_c,     r12
        msr   spsr_cxsf,  r12
        orr   r12,        r1,     #0x0007
        msr   cpsr_c,     r12
        msr   spsr_cxsf,  r12
        orr   r12,        r1,     #0x000B
        msr   cpsr_c,     r12
        msr   spsr_cxsf,  r12
        orr   r12,        r1,     #0x0003
        msr   cpsr_c,     r12
        msr   spsr_cxsf,  r12

        /* System level configuration */
        mrc   p15,0,r11,c1,c0,0       /* Read current system configuration */
        mov   r12,		  #0x40000000 /* Set THUMB instruction set mode for interrupts and exceptions */
        orr   r12, r12, r11
        mcr   p15,0,r12,c1,c0,0       /* Write new configuration */


Init_Stack_Pointers:

user:   .word _estack
svc:    .word _estack
fiq:    .word _estack
irq:    .word _estack
abort:  .word _estack
undef:  .word _estack

		mov   r2,		#0xD1
        msr   cpsr_c,   r2
        ldr   sp,       fiq

        mov   r2,		#0xD2
        msr   cpsr_c,   r2
        ldr   sp,       irq

        mov   r2,		#0xD7
        msr   cpsr_c,   r2
        ldr   sp,       abort

        mov   r2,		#0xDB
        msr   cpsr_c,   r2
        ldr   sp,       undef

        mov   r2,		#0xDF
        msr   cpsr_c,   r2
        ldr   sp,       user

        mov   r2,		#0xD3
        msr   cpsr_c,   r2
        ldr   sp,       svc


CopyInitializedData:
	ldr	r0, =_sdata       /* r0 holds start of data in ram */
	ldr	r3, =_edata       /* r3 holds end of data in ram */
	ldr	r5, =_sidata      /* r5 start of data in flash */
	movs  r1,       #0    /* r1 is the counter */
	b	LoopCopyDataInit

CopyDataInit:
	ldr	r4, [r5, r1]          /* read current position in flash */
	str	r4, [r0, r1]          /* store current position in ram */
	adds	r1, r1, #4        /* increment counter */
    
LoopCopyDataInit:
	adds	r2, r0, r1        /* are we at the final position? */
	cmp	r2, r3                /* ... */
	bcc	CopyDataInit          /* nope, continue */

/* Fill zero areas */
	ldr	r2, =_sbss            /* r2 holds the start address */
	ldr r5, =_ebss            /* r5 holds the end address */
	bl	LoopFillZero

	ldr	r2, =_sstack            /* r2 holds the start address */
	ldr r5, =_estack            /* r5 holds the end address */
	bl	LoopFillZero

/* Call the application's entry point.*/
	mov   r2,		#0xDF
    msr   cpsr_c,   r2
	bl	main
	bx	lr

/* Zero fill the bss segment. */  
FillZero:
	movs	r3, #0
	str	    r3, [r2], #4
    
LoopFillZero:
	cmp	r2, r5
	bcc	FillZero
	bx  lr

Dummy_Irq:
	/* Go back to sys mode for easier debugging.
	 Save link register*/
	mov   r3, lr
	/* We don't want to use the IRQ mode
	   so swich back to sys mode. */
	mov   r2,		#0xDF
    msr   cpsr_c,   r2
    /* Restore link register again */
    mov   lr, r3
	b Dummy_Irq

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
* Interrupt and exception vectors. Vectors start at addr 0x0.
******************************************************************************/    
 	.section	.int_vecs,"ax",%progbits
	.extern Irq_Handler

        b   Reset_Handler      /* Reset? */
        b   Dummy_Irq          /* Undef? */
        b   Dummy_Irq          /* SVC */
        b   Dummy_Irq          /* Prefetch */
        b   Dummy_Irq          /* data */
        b   Dummy_Irq          /* ? */
        b   Irq_Handler        /* IRQ */
	    b   Irq_Handler        /* FIR */

