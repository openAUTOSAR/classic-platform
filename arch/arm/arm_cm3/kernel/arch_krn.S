

#define _ASSEMBLER_
#include "kernel_offset.h"
#include "arch_offset.h"
#include "stack.h"

.extern os_sys

	.syntax unified
	.cpu cortex-m3
	.fpu softvfp
	.thumb

	.global os_arch_swap_context_to
	.global os_arch_swap_context
	.section .text
/* r0 - pcb for old task
 * r1 pcb for new task
 *
 * -------------- higher addr
 * r4-r8,etc
 * -------
 * 4 - Large or Small context indicator
 * 0 - ?
 * ----------    <- JABBA
 *
 */
os_arch_swap_context:
    mov     r0, sp
    push    {r0,r4-r8,r10,r11,lr}


    // Store c-frame
    sub.w   sp,sp,#C_SIZE
    mov.w   r4,#SC_PATTERN

    str     r4,[sp,#4]
    // store old stack for old task
    str     sp,[r0,#PCB_STACK_CURR_P]
os_arch_swap_context_to:
    // Get stack for new task
    ldr     sp,[r1,#PCB_STACK_CURR_P]

// TODO: Fix this for all arch's..call pre,post hooks. Done here or after?
// Set new current pcb
    ldr     r5,= os_sys
    str     r1,[r5,#SYS_CURR_PCB_P]

// Restore C context
    ldr     r6,[sp,#4]
    cmp     r6,#SC_PATTERN
    beq     os_sc_restore
    cmp     r6,#LC_PATTERN
    beq     os_lc_restore
os_stack_problem:
// TODO: Jump to error handler
    b os_stack_problem

os_sc_restore:
    add.w   sp,sp,#C_SIZE
    pop     {r4-r8,r10,r11,lr}
        // ehh, we are in handler mode so a bx instruction works here
    bx      lr

os_lc_restore:
    add.w   sp,sp,#C_SIZE
    pop     {r4-r8,r10,r11,lr}
    // ehh, we are in handler mode so a bx instruction works here
    bx      lr



