
/*
 * IVOR4
 * 1. Save stack frames: EXC, VGPR, NVGPR and C
 * 2. Call IntCtrl_Entry(void *curr_stack)
 * 2.1 Check for exception
 * 2.2 If softint, clear it.
 * 2.3 If PROC_ISR1, then just call the function
 * 2.4 If PROC_ISR2 -> it's a PCB, let the OS handle it.
 */

#define _ASSEMBLER_
#include "asm_ppc.h"
#include "asm_offset.h"
#include "asm_book_e.h"
.extern os_intc_pcb_tbl
.extern os_intc_types_tbl
.extern os_sys


/*
 * Small assembler school
 * Compare imm(32-bit)
 * > cmpwi rA,100
 * Extract bits and right adjust
 * > extrwi rA,rS,n,b  ( n-number of bits, b- startbit )
 *
 * Note!
 * The offset's (d or D)for SPE instructins are rediculously low.
 * Normally you have 16-bits offset, but when using spe load and store
 * you can use only 8-bit.
*/

#define	LOCK()			wrteei	0
#define	UNLOCK()		wrteei	1


.extern os_proc_start_extended

//-------------------------------------------------------------------

    .global os_exception_IVOR8
    .balign 16
os_exception_IVOR8:
	stwu	sp,-(EXC_SIZE+VGPR_SIZE)(sp)
	stw  	r3,EXC_R3_OFF(r1)
	stw  	r4,EXC_R4_OFF(r1)
	SAVE_EXC_FRAME(3,1,0,SPR_SRR0,SPR_SRR1)
	SAVE_VGPR(1,EXC_SIZE);
	li		r3,328
	stw  	r3,EXC_VECTOR_OFF(r1)

	rfi

dummy_int:
		b dummy_int

/*--------------------------------------------------------------------
 * void os_swap_context(pcb_t *old, pcb_t *new )
 *
 * Saves a small context on current stack, pops a new one from new context
 *
 * r3 - pcb for old process
 * r4 - pcb for new process
 *
 *--------------------------------------------------------------------*/

// TODO: this assumes that both are in user mode?.. can this happen under trusted functions?
//       When I get here we're ALWAYS in kernel mode

.global os_arch_swap_context_to
.global os_arch_swap_context
.section .text
os_arch_swap_context:
		// allocate space for context+nvgpr
		// (no need for proper stack-frame here)
 		stwu    r1,-(C_SIZE+NVGPR_SIZE)(r1)
		// save lr and cr */
		mflr 	r0
		stw		r0,C_LR_OFF(sp)
		mfcr	r0
		stw		r0,C_CR_OFF(sp)
		// Save small-context pattern
		li		r0,SC_PATTERN
		stw		r0,C_CONTEXT_OFF(sp)
		// Save registers preserved by function call
		SAVE_NVGPR(sp,(C_SIZE-14*GPR_SIZE))
// Save stack ptr...
		stw		sp,PCB_STACK_CURR_P(r3)

// Stack frame here
// --------- bottom( high address )
//  SC_xxx
//  C_xxx
// --------- <- stack.curr
//
// --------- top( low address )


// TODO: If we change application we must change mmu setup
os_arch_swap_context_to:
// Get stack for new task
		lwz		sp,PCB_STACK_CURR_P(r4)

// Set new current process
		LOAD_ADDR_32(3,os_sys)
		stw		r4,SYS_CURR_PCB_P(r3)

// Restore C context
        lwz     r0,C_CR_OFF(sp)
        mtcr    r0
        lwz     r0,C_LR_OFF (sp)
        mtlr    r0

// Get the context type
		lwz		r0,C_CONTEXT_OFF(sp)
		cmpli	0,r0,SC_PATTERN
		beq+	os_sc_restore
		cmpli	0,r0,LC_PATTERN
		beq+ 	os_lc_restore
		b		os_bad_bad


// SC_xxx
// C_xxxx <- We point here

os_sc_restore:
		RESTORE_NVGPR(sp,(C_SIZE-14*GPR_SIZE))
		addi 	sp,sp,(C_SIZE+NVGPR_SIZE)
		// TODO: The blr will not do the trick if swapping to a user land task.
		blr

os_lc_restore:
	addi 	r1,r1,C_SIZE
	RESTORE_NVGPR(1,0)
	addi	r1,r1,-C_SIZE
	RESTORE_VGPR(1,C_SIZE)

	RESTORE_WORK_AND_MORE
	rfi

// When something really bad happens we end up here for the moment
os_bad_bad:
		b	os_bad_bad




// ------------------------------------------------------------------

/*
 * Trap interface !!!! See article http://www.linuxjournal.com/article/6516
 * http://www.osweekly.com/index.php?option=com_content&task=view&id=2229
 */

/* The T32 instruction sim can't handle trap's so we have to make something
 * - write SRR0, SRR1, MSR
 * - jump to there routines
 */

// ------------------------------------------------------------------


// System call, use this for trusted function ???
// TODO: The example in autosar is not neccesary.. sc here here instead??
//
// NOTE!!!!!
// Since the sc is a sync call, it should be enough to save NV regs(14->)
// If I don't use the NV regs here I shouldn't need to save them
// TODO: Inform compiler in SC_CALL() that I clobber volatile regs( r0, r3->
//      (since the compiler does not know it's a function call)
// TODO: Could probably do this shorter....only NV regs that I use need saving
//       ( only cr2->cr4 according to e500 ABI )






