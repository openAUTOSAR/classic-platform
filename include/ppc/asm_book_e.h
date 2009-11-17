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









#ifndef _ASM_BOOK_E_H
#define _ASM_BOOK_E_H

/*
 *
 *
 *
 *
 *
 *
 */


/*
 * TODO: This file should be split in two. One handle the exceptions for book-e
 * and one for this arch stack-frame-> asm_stack_frame.h
 */

/*

  Mcu_Exceptions.S maps exceptions to the kernel through the exception_tbl.


  exception_tbl: ( Mcu_Exceptions.S )
    |
    |                                  exc_intc_pcb_tbl ( kernel )
    |-----> exception_IVORx:  ------> -------------
    |-----> exception_IVORy:  -,      |     0     |  ISR2
                               |      |    .. .   |<>----------> my_isr_c_routine( C )
                               |      |           |
                               '----> |    32x    |
                                      |    324    |
			  		     		      |    3      | -----------> my_isr_routine: (assembler)
				  					  '           '
                                      '-----------'
  In kernel for IVOR4:
  1. Get vector through INTC_IACKR
  2. Get pcb for vector
     If pcb==0, goto 10
  3. Get ISR type
     if type==1, goto 5
  4. Save some GPR regs
  5. Call pcb->entry
  10: done!


  ,--------,
  |  SP    |
  |  Type  | Context
  |  LR    |
   .....
  +--------+
  |        | NVGPR's
   .....
  +--------+


  b exception_IVOR4
  {
    save EF regs
    get interrupt vector
    if (soft int) {
       ack int
    }
    get intc table
    jump to function

  }

  The are two types of ISR, ISR1 and ISR2:
  ISR1 - assembler routine, return with blr( Only EF exist on stack )
  ISR2 - normal C-routine( can save small or big frame depending on config )


  Use cases:
  - InstallVector(vector, func, type )
    1. InstallVector( 320+10, exception_dec, 1 );		// dec exception
    2. InstallVector( 320+11, exception_fit, 2 );		// fit exception
    3. InstallVector( 5, softint , 2 );					// soft int
    4. InstallVector( 100, normal_int, 2 ) 		   		// normal int.
    5. InstallVector( 101, normal_int_2, 1 ) 	   		// normal int.

    In case 1, the user have to install a prologue/epilogues to access
    it from C( i.e store small frame )
*/



#define SC_PATTERN		0xde
#define LC_PATTERN		0xad


/* Alignment for architecture, see e500-ABI */
#define ARCH_ALIGN		16

#if defined(USE_KERNEL)
#define C_SIZE			32
#define C_SP_OFF		0
// 4- backchain
// 8 -padding
#define C_CONTEXT_OFF	12
#define C_LR_OFF		16
#define C_CR_OFF		20
#else
#define C_SIZE			0
#endif



#if defined(CFG_SPE)
//-----------------------------------------------------------------
#define GPR_SIZE		8
#elif defined(CFG_MPC5516)
#define GPR_SIZE		4
#else
#error No MCU set
#endif


#if defined(USE_KERNEL)
/* Small context */
#define SC_GPRS_SIZE	((31-14+1)*GPR_SIZE)
#define SC_GPRS_OFFS	C_SIZE
#define SC_SIZE 		(C_SIZE + SC_GPRS_SIZE)

/* Large context */
#define LC_GPRS_SIZE	((31-0+1)*GPR_SIZE)
#define LC_SIZE 		(C_SIZE + LC_GPRS_SIZE)
#endif

// GPR save and restore macros

#if defined(CFG_SPE)
  #define SAVE_GPR(reg,_offset,rel_reg)		evstdd	reg,(((reg)*GPR_SIZE)+_offset)(rel_reg)
  #define RESTORE_GPR(reg,_offset,rel_reg)		evldd	reg,(((reg)*GPR_SIZE)+_offset)(rel_reg)
#elif defined(CFG_MPC5516)
  #define SAVE_GPR(reg,_offset,rel_reg)		stw		reg,(((reg)*GPR_SIZE)+_offset)(rel_reg)
  #define RESTORE_GPR(reg,_offset,rel_reg)		lwz		reg,(((reg)*GPR_SIZE)+_offset)(rel_reg)
#endif

#define SAVE_GPR2(reg,_offset,rel_reg)			SAVE_GPR(reg,_offset,rel_reg);SAVE_GPR(reg+1,_offset,rel_reg)
#define SAVE_GPR4(reg,_offset,rel_reg)			SAVE_GPR2(reg,_offset,rel_reg);SAVE_GPR2(reg+2,_offset,rel_reg)
#define SAVE_GPR8(reg,_offset,rel_reg)			SAVE_GPR4(reg,_offset,rel_reg);SAVE_GPR4(reg+4,_offset,rel_reg)

#define RESTORE_GPR2(reg,_offset,rel_reg)		RESTORE_GPR(reg,_offset,rel_reg);RESTORE_GPR(reg+1,_offset,rel_reg)
#define RESTORE_GPR4(reg,_offset,rel_reg)		RESTORE_GPR2(reg,_offset,rel_reg);RESTORE_GPR2(reg+2,_offset,rel_reg)
#define RESTORE_GPR8(reg,_offset,rel_reg)		RESTORE_GPR4(reg,_offset,rel_reg);RESTORE_GPR4(reg+4,_offset,rel_reg)

// Non volatile regs, 14-31( saved by function called )
#define SAVE_NVGPR(rel_reg,_offset)			SAVE_GPR2(14,_offset,rel_reg); \
											SAVE_GPR8(16,_offset,rel_reg); \
											SAVE_GPR8(24,_offset,rel_reg);

#define RESTORE_NVGPR(rel_reg,_offset)		RESTORE_GPR2(14,_offset,rel_reg); \
											RESTORE_GPR8(16,_offset,rel_reg); \
											RESTORE_GPR8(24,_offset,rel_reg);

#define NVGPR_SIZE 								(18*GPR_SIZE)

// Volatile regs, r0, r5-r12, [r13] ( r3, r4 are assumed to be save else where )
#define SAVE_VGPR(rel_reg,_offset) 			SAVE_GPR(0,_offset,rel_reg);SAVE_GPR8(5,_offset, rel_reg)
#define RESTORE_VGPR(rel_reg,_offset)			RESTORE_GPR(0,_offset, rel_reg);RESTORE_GPR8(5,_offset, rel_reg)
#define VGPR_SIZE 								(14*GPR_SIZE)

#define EXC_BASE_OFF			0 		//(VGPR_SIZE+NVGPR_SIZE+C_SIZE)
#define EXC_OFF_FROM_BOTTOM		(VGPR_SIZE+NVGPR_SIZE+C_SIZE)


/* Exception frame */
#if defined(CFG_SPE)
#define EXC_SIZE		80
#else
#define EXC_SIZE		64		/* MUST be 16 byte aligned, again eabi */
#endif

#define EXC_SP_OFF		(EXC_BASE_OFF+0)
// 4 - backchain
// 8 - padding
#define EXC_SRR0_OFF	(EXC_BASE_OFF+12)
#define EXC_SRR1_OFF	(EXC_BASE_OFF+16)
#define EXC_LR_OFF		(EXC_BASE_OFF+20)
#define EXC_CTR_OFF		(EXC_BASE_OFF+24)
#define EXC_XER_OFF		(EXC_BASE_OFF+28)
#define EXC_CR_OFF		(EXC_BASE_OFF+32)
#define EXC_ESR_OFF		(EXC_BASE_OFF+36)
#define EXC_MCSR_OFF	(EXC_BASE_OFF+40)
#define EXC_DEAR_OFF	(EXC_BASE_OFF+44)
#define EXC_VECTOR_OFF 	(EXC_BASE_OFF+48)
#if defined(CFG_SPE)
#define EXC_SPEFSCR		(EXC_BASE_OFF+52)
#define EXC_R3_OFF 		(EXC_BASE_OFF+56)	// 8 bytes..
#define EXC_R4_OFF 		(EXC_BASE_OFF+64)	// 8 bytes..
#define EXC_SPE_ACC		(EXC_BASE_OFF+72)	// 8 bytes
#else
#define EXC_R3_OFF 		(EXC_BASE_OFF+52)
#define EXC_R4_OFF 		(EXC_BASE_OFF+56)
#endif


/*-------------------------------------------------------------------
 * Save exception frame macro
 *-----------------------------------------------------------------*/

#define SAVE_EXC_FRAME(work,rel_reg,_offset,xsrr0_spr,xsrr1_spr) 	\
		mfspr	work,xsrr0_spr; 					\
		stw		work,(EXC_SRR0_OFF+_offset)(rel_reg); 	\
		mfspr	work,xsrr1_spr; 					\
		stw		work,(EXC_SRR1_OFF+_offset)(rel_reg); 	\
		mfspr	work,SPR_XER; 					\
		stw		work,(EXC_XER_OFF+_offset)(rel_reg); 		\
		mfspr	work,SPR_CTR; 					\
		stw		work,(EXC_CTR_OFF+_offset)(rel_reg); 		\
		mflr	work; 							\
		stw		work,(EXC_LR_OFF+_offset)(rel_reg); 		\
		/* TODO: all exceptions dont't need all the info below... but it's simple */ \
		mfspr	work,SPR_ESR; 					\
		stw		work,(EXC_ESR_OFF+_offset)(rel_reg); 		\
		mfspr	work,SPR_DEAR; 					\
		stw		work,(EXC_DEAR_OFF+_offset)(rel_reg); 	\
		mfspr	work,SPR_MCSR; 					\
		stw		work,(EXC_MCSR_OFF+_offset)(rel_reg);		\
		mfcr	work; 							\
		stw		work,(EXC_CR_OFF+_offset)(rel_reg);


/*-------------------------------------------------------------------
 * Restore exception frame macro
 *-----------------------------------------------------------------*/
#define RESTORE_EXC_FRAME(work,rel_reg,_offset,xsrr0_spr,xsrr1_spr) 		\
		lwz		work,(EXC_SRR0_OFF+_offset)(rel_reg); 	\
		mtspr	xsrr0_spr,work; 						\
		lwz		work,(EXC_SRR1_OFF+_offset)(rel_reg); 	\
		mtspr	xsrr1_spr,work; 						\
		lwz		work,(EXC_XER_OFF+_offset)(rel_reg); 	\
		mtspr	SPR_XER,work; 							\
		lwz		work,(EXC_CTR_OFF+_offset)(rel_reg); 	\
		mtspr	SPR_CTR,work; 							\
		lwz		work,(EXC_LR_OFF+_offset)(rel_reg); 	\
		mtlr	work;									\
		lwz		work,(EXC_ESR_OFF+_offset)(rel_reg);	\
		mtspr	SPR_ESR,work;							\
		lwz		work,(EXC_DEAR_OFF+_offset)(rel_reg); 	\
		mtspr	SPR_DEAR,work; 							\
		lwz		work,(EXC_MCSR_OFF+_offset)(rel_reg); 	\
		mtspr	SPR_MCSR,work;                  		\
		lwz		work,(EXC_CR_OFF+_offset)(rel_reg); 	\
		mtcr	work;									\



#define EXCEPTION_CRITICAL_PROLOGUE(vector) \
    addi  sp,sp,-EXC_SIZE;          \
    stw   r3,EXC_R3_OFF(r1);        \
    stw   r4,EXC_R4_OFF(r1);        \
    li    r4,vector;                \
    stw   r4,EXC_VECTOR_OFF(r1);    \
    SAVE_EXC_FRAME(3,1,0,SPR_CSRR0,SPR_CSRR1)


#define EXCEPTION_CRITICAL_EPILOGUE(vector) \
    RESTORE_EXC_FRAME(3,1,0,SPR_CSRR0,SPR_CSRR1);         \
    lwz   r3,EXC_R3_OFF(r1);        \
    lwz   r4,EXC_R4_OFF(r1);        \
    addi  sp,sp,EXC_SIZE;


#define EXCEPTION_PROLOGUE(vector) \
    addi  sp,sp,-EXC_SIZE;          \
    stw   r3,EXC_R3_OFF(r1);        \
    stw   r4,EXC_R4_OFF(r1);        \
    li    r4,vector;                \
    stw   r4,EXC_VECTOR_OFF(r1);    \
    SAVE_EXC_FRAME(3,1,0,SPR_SRR0,SPR_SRR1)


#define EXCEPTION_EPILOGUE()       \
    RESTORE_EXC_FRAME(3,1,0,SPR_SRR0,SPR_SRR1);         \
    lwz   r3,EXC_R3_OFF(r1);        \
    lwz   r4,EXC_R4_OFF(r1);        \
    addi  sp,sp,EXC_SIZE;


#define CALL_HANDLER() CALL_HANDLER_(Irq_VectorTable)

//-------------------------------------------------------------------

#define CALL_HANDLER_(_table)         			\
    lis    r3, _table@h;     					\
    ori    r3, r3, _table@l;  					\
    slwi  r4,r4,2;   /* times 4 */     			\
    add   r3,r3,r4;                    			\
    lwz   r3,0(r3);  /* get the entry */ 		\
    cmpli 0,r3,0;                      			\
    beq+  bad_int;                   			\
    mtctr r3;                          			\
    mr    r3,r1; /* use stack as arg */			\
    subi  r1,r1,16; /* space for backchain */ 	\
    bctrl;                                    	\
    addi  r1,r1,16

//-------------------------------------------------------------------

#define EXCEPTION_CSRRx(_section,_vector)   \
    .global _section;                      \
    .balign 16;                           \
_section:;                       \
     EXCEPTION_CRITICAL_PROLOGUE(_vector); \
     CALL_HANDLER();                      \
     EXCEPTION_CRITICAL_EPILOGUE();       \
     rfci


#define EXCEPTION_SRRx(_section,_vector)   \
    .global _section;                      \
    .balign 16;                           \
_section:;                       \
     EXCEPTION_PROLOGUE(_vector); \
     CALL_HANDLER();                      \
     EXCEPTION_EPILOGUE();       \
     rfi


#if defined(_ASSEMBLER_)
#if defined(CFG_SPE)

.macro save_work_and_more
	// work on the exception frame for now..
	addi		sp,sp,-(EXC_SIZE)

	mtspr 		SPR_SPRG0_RW_S,r3
	// Enable SPE (exceptions turns it off)
	mfmsr		r3
    oris   		r3,r3,0x0200
	mtmsr 		r3
	isync

	mfspr 		r3, SPR_SPRG0_RW_S

	// Save 64-bit r3 and r4
	evstdd 		r3,EXC_R3_OFF(r1)
	evstdd 		r4,EXC_R4_OFF(r1)

	// SPEFSCR
	mfspr		r3,SPR_SPEFSCR
	clrlwi		r3,r3,24	 /* Mask off non-status bits */
	stw			r3,EXC_SPEFSCR(sp)

	// Save SPE acc
	evsubfw 	r3,r3,r3		// zero r3
	evaddumiaaw r3,r3			// Add r3 = r3 + acc -> r3 = acc
	evstdd 		r3,EXC_SPE_ACC(r1)

	SAVE_EXC_FRAME(3,1,0,SPR_SRR0,SPR_SRR1)

	// Save access through r4
	mr 		r4,r1
	// Make place for the other stack frames.
	addi 	sp,sp,-(VGPR_SIZE+NVGPR_SIZE+C_SIZE)
.endm

.macro restore_work_and_more
	// Stack is below C frame.... access EXC frame.
	addi	sp,sp,(C_SIZE+NVGPR_SIZE+VGPR_SIZE)

	// Store the SPE control/status reg.
	lwz		r3,EXC_SPEFSCR(sp)
	mtspr	SPR_CSRR0,r3

	/* Load SPE acc */
	evldd r3,EXC_SPE_ACC(r1)
	evmra r3,r3

	RESTORE_EXC_FRAME(3,1,0,SPR_SRR0,SPR_SRR1);

	evldd	r3,EXC_R3_OFF(r1)
	evldd	r4,EXC_R4_OFF(r1)

	addi	sp,sp,EXC_SIZE

.endm
#else
.macro	save_work_and_more
	addi	sp,sp,-(EXC_SIZE)

	stw  	r3,EXC_R3_OFF(r1)
	stw  	r4,EXC_R4_OFF(r1)

	SAVE_EXC_FRAME(3,1,0,SPR_SRR0,SPR_SRR1)

	// Save access through r4
	mr 		r4,r1
	// Make place for the other stack frames.
	addi 	sp,sp,-(VGPR_SIZE+NVGPR_SIZE+C_SIZE)
.endm

.macro	restore_work_and_more
	// Work on the exception frame.
	addi	sp,sp,(VGPR_SIZE+NVGPR_SIZE+C_SIZE)

	RESTORE_EXC_FRAME(3,1,0,SPR_SRR0,SPR_SRR1);

	lwz   	r3,EXC_R3_OFF(r1);
	lwz   	r4,EXC_R4_OFF(r1);

	addi	sp,sp,EXC_SIZE
.endm
#endif
#endif

#endif /* _ASM_BOOK_E_H */

