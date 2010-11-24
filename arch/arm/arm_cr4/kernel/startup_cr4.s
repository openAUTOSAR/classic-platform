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

.global	Default_Handler

/* Addresses used to setup RAM */
.word	_sidata
.word	_sdata
.word	_edata
.word	_sbss
.word	_ebss

/* The address of the stack to use in all modes. */
.word _estack


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

/* Initialize core registers.
   This is done to avoid mismatch between lockstep CPU and ordinary CPU
*/
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

/* System level configuration
   This mainly involves setting instruction mode for exceptions and interrupts.
*/
    mrc   p15,0,r11,c1,c0,0       /* Read current system configuration */
    mov   r12,		  #0x40000000 /* Set THUMB instruction set mode for interrupts and exceptions */
    orr   r12, r12, r11
    mcr   p15,0,r12,c1,c0,0       /* Write new configuration */


/* Initialize stack pointers.
   This is done for all processor modes. Note that we only use one stack pointer.
   In reality this means that no mode except USER and SYS is allowed to do anythin on the stack.
   IRQ mode handles its own stack in the interrupt routine.
*/
	mov   r2,		#0xD1
    msr   cpsr_c,   r2
    ldr   sp,		=_estack

    mov   r2,		#0xD2
    msr   cpsr_c,   r2
    ldr   sp,		=_estack

    mov   r2,		#0xD7
    msr   cpsr_c,   r2
    ldr   sp,		=_estack

    mov   r2,		#0xDB
    msr   cpsr_c,   r2
    ldr   sp,		=_estack

    mov   r2,		#0xDF
    msr   cpsr_c,   r2
    ldr   sp,		=_estack

    mov   r2,		#0xD3
    msr   cpsr_c,   r2
    ldr   sp,		=_estack


/* Initialize RAM.
   First the initialized RAM is copied from flash to RAM.
   Then the zeroed RAM is erased.
*/
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

.size	Reset_Handler, .-Reset_Handler




/******************************************************************************
* Interrupt and exception vectors. Vectors start at addr 0x0.
******************************************************************************/    
 	.section	.int_vecs,"ax",%progbits
	.extern Irq_Handler
	.extern Svc_Handler
	.extern Data_Exc_Handler
	.extern Prefetch_Exc_Handler
	.extern Dummy_Irq

		/* This is the reset handler. Since the CPU is in ARM mode when this instruction is executed
		   it has to be hard coded (otherwise it will compile wrong).
		   Instruction branches to address 0x22 while changing instruction mode to THUMB. */
        .word 0xfb000006

        b   Dummy_Irq          		/* Undefined instruction exception */
        b   Dummy_Irq        		/* SVC */
        b   Prefetch_Exc_Handler    /* Prefetch exception */
        b   Data_Exc_Handler   		/* Data exception */
        b   Dummy_Irq          		/* Reserved */
        b   Irq_Handler        		/* Ordinary interrupts (IRQ) */
	    b   Dummy_Irq        		/* Fast interrupts (FIR) */

		nop
		b 	Reset_Handler      /* Branch to the real reset handler. */

