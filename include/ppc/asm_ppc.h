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








#ifndef PPC_ASM_H_
#define PPC_ASM_H

/* Context
 * =================================================
 * We have two context's large and small. Large is saved on
 * interrupt and small is saved for everything else.
 *
 * Layout:
 *
 * offset
 * -------------------------------
 *   0--1    : context indicator, 0xde - small, 0xad - large
 *   4       : lr
 *   8       : cr
 *  12       : sp
 *
 * small layout
 *   16--    : General regs r14--r31
 *
 *  large layout
 *   16--    : General regs r0--r31
 *
 */

#define SPR_SRR0		26
#define SPR_SRR1		27

#define SPR_CSRR0		58
#define SPR_CSRR1		59

#define SPR_SPRG0_RW_S	272
#define SPR_SPRG1_RW_S	273

#define SPR_DEAR		61
#define SPR_ESR		62
#define SPR_TSR		336
#define SPR_SPEFSCR		512
#define SPR_MCSR		572

#define SPR_MAS0      624
#define SPR_MAS1      625
#define SPR_MAS2      626
#define SPR_MAS3      627
#define SPR_MAS4      628
#define SPR_MAS6      630


#define ESR_PTR		(1<<(38-32))

#define SPR_XER		1
#define SPR_CTR		9

#define INTC_SSCIR7 0xFFF48027

/* MAS bits */
#define MAS1_TSIZE_4K			(1<<8)
#define MAS1_TSIZE_16K			(2<<8)
#define MAS1_TSIZE_64K			(3<<8)
#define MAS1_TSIZE_256K			(4<<8)
#define MAS1_TSIZE_1M			(5<<8)
#define MAS1_TSIZE_4M			(6<<8)
#define MAS1_TSIZE_16M			(7<<8)
#define MAS1_TSIZE_64M			(8<<8)
#define MAS1_TSIZE_256M			(8<<9)

#define MAS2_VLE	(1<<5)
#define MAS2_W		(1<<4)
#define MAS2_I		(1<<3)
#define MAS2_M		(1<<2)
#define MAS2_G		(1<<1)
#define MAS2_E		(1<<0)

#define MAS3_UX		(1<<5)
#define MAS3_SX		(1<<4)
#define MAS3_UW		(1<<3)
#define MAS3_SW		(1<<2)
#define MAS3_UR		(1<<1)
#define MAS3_SR		(1<<0)

#define MAS3_FULL_ACCESS (MAS3_UX+MAS3_UW+MAS3_UR+MAS3_SX+MAS3_SW+MAS3_SR)


#if defined(_ASSEMBLER_)
/*
 * PPC vs VLE assembler:
 *   Most PPC assembler instructions can be pre-processed to VLE assembler.
 *   I can't find any way to load a 32-bit immediate with just search/replace (number
 *   of operators differ for addis and e_add2is )
 *   Thats why there are different load macros below.
 *
 */

#if defined(CFG_VLE)
#define LOAD_IND_32( reg, addr) \
	e_lis    reg, addr@ha;     \
    e_lwz    reg, addr@l(reg)

#define LOAD_ADDR_32(reg, addr ) \
    e_lis       reg, addr@ha; \
    e_add16i    reg, reg, addr@l

#else
#define LOAD_IND_32( reg, addr) \
    lis    reg, addr@ha;     \
      lwz    reg, addr@l(reg)

#define LOAD_ADDR_32(reg, addr ) \
        addis   reg, 0, addr@ha; \
        addi    reg, reg, addr@l

#endif



/* GPRS */
#define sp	    1
#define r0      0
#define r1      1
#define r2      2
#define r3      3
#define r4      4
#define r5      5
#define r6      6
#define r7      7
#define r8	    8
#define r9	    9
#define r10	    10
#define r11     11
#define r12     12
#define r13     13
#define r14     14
#define r15     15
#define r16     16
#define r17     17
#define r18     18
#define r19     19
#define r20     20
#define r21     21
#define r22     22
#define r23     23
#define r24     24
#define r25     25
#define r26     26
#define r27     27
#define r28     28
#define r29     29
#define r30     30
#define r31     31


#if defined(CFG_VLE)

#define lis		e_lis
#define li		e_li
#define lwz	    e_lwz
#define lbz		e_lbz
#define lbzu	e_lbzu
#define stwu	e_stwu
#define stw		e_stw
#define stb		e_stb
#define stbu	e_stbu
#define b		e_b
#define bne		e_bne
//#define addi	e_addi		/* true ?*/
#define addi	e_add16i		/* true ?*/
//#define addis	e_add16i
#define subi	e_subi		/* true ?*/
#define blr		se_blr
#define rfi		se_rfi
#define stb		e_stb
#define cmplwi	e_cmpl16i
#define cmpwi 	se_cmpi
#define ori		e_ori
#define beq		e_beq
//#define bne- 	e_bne-
#define bne		e_bne
#define bgt		e_bgt
#define extrwi	e_extrwi
#define blrl	se_blrl
#define stmw	e_stmw
#define bdnz	e_bdnz
#define	bl		e_bl
#define	bc		e_bc
#define mr		se_mr
#endif

#endif /* _ASSEMBLER_ */

#endif /*PPC_ASM_H_*/
