/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

#ifndef ASM_RH850_H_
#define ASM_RH850_H_

#define EXC_SYSERR		1
#define EXC_HVTRAP		2
#define EXC_FETRAP		3
#define EXC_TRAP0		4
#define EXC_TRAP1		5
#define EXC_RIE			6
#define EXC_FPP_FPI		7
#define EXC_UCPOP		8
#define EXC_MIP_MDP		9
#define EXC_PIE			0xa
#define EXC_DEBUG		0xb
#define EXC_MAE			0xc
#define EXC_RFU			0xd
#define EXC_FENMI		0xe
#define EXC_FEINT		0xf


#if defined(_ASSEMBLER_)

/* Use as:
 * ASM_SECTION_TEXT(.text) - For normal .text or .text_vle
 *
 *
 */

#if defined(__ghs__)
#  define ASM_SECTION_TEXT(_x) 	.section .vletext,"vax"
#  define ASM_SECTION(_x)  		.section #_x,"vax"
#  define ASM_CODE_DIRECTIVE()    .vle
#  define ASM_NEED(_x)  		.need _x
#elif defined(__GNUC__)
#  define ASM_SECTION_TEXT(_x) 	.section #_x,"ax"
#  define ASM_SECTION(_x)  		.section #_x,"ax"
#  define ASM_NEED(_x)
#elif defined(__DCC__)
#  define ASM_SECTION_TEXT(_x) 	.section _x,4,"x"
#  define ASM_SECTION(_x)		.section _x,4,"r"
#endif

#ifndef ASM_CODE_DIRECTIVE
#define ASM_CODE_DIRECTIVE()
#endif

#define LOAD_ADDR_32(reg, addr ) \
        addis   reg, 0, addr@ha; \
        addi    reg, reg, addr@l

#define MOV32(x, y) movhi hi(x),zero,y ; movea  lo(x),y,y

#if defined(__ghs__)
#define CMP16(_imm16,_x)		cmp _imm16,_x
#else
#define CMP16(_imm16,_x)		movea _imm16, r0, r1; cmp r1, _x
#endif

#define ASM_BALIGN(_x)			.balign _x



//extern const char _ep[];
//extern const char _tp[];
//extern const char _gp[];

/* GPRS */
//#define sp	    1
//#define r0      0
//#define r1      1
//#define r2      2
//#define r3      3
//#define r4      4
//#define r5      5
//#define r6      6
//#define r7      7
//#define r8	    8
//#define r9	    9
//#define r10	    10
//#define r11     11
//#define r12     12
//#define r13     13
//#define r14     14
//#define r15     15
//#define r16     16
//#define r17     17
//#define r18     18
//#define r19     19
//#define r20     20
//#define r21     21
//#define r22     22
//#define r23     23
//#define r24     24
//#define r25     25
//#define r26     26
//#define r27     27
//#define r28     28
//#define r29     29
//#define r30     30
//#define r31     31

#endif /* _ASSEMBLER_ */

#endif /*ASM_RH850_H_*/

