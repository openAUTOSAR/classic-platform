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


#ifndef ASM_ARM_H_
#define ASM_ARM_H

/*
 * Exception handling numbers */
#if defined(CFG_ARMV7_AR)
#define EXC_UNDEFINED_INSTRUCTION       0
#define EXC_PREFETCH_ABORT              1
#define EXC_DATA_ABORT                  2
#define EXC_SVC_CALL                    4

/* Exception flags  */
#define EXC_NOT_HANDLED 1UL
#define EXC_HANDLED     2UL
#define EXC_ADJUST_ADDR 4UL

#endif


#if defined(__GNUC__)
#define ASM_WORD(_x)	.word _x
#define ASM_LONG(_x)	.long _x
#define ASM_EXTERN(_x)	.extern _x
#define ASM_GLOBAL(_x)  .global _x
#define ASM_TYPE(_x,_y)	.type	_x, _y
#define ASM_LABEL(_x)	_x:
#define ASM_WEAK(_x)	.weak  _x
#define ASM_SIZE(_x,_y)	.size	_x, _y
#define ASM_REPT(_x)	.rept   _x
#define ASM_ENDREPT		.endr
#elif defined(__ARMCC_VERSION)
#define ASM_WORD(_x)	_x DCD 1
#define ASM_EXTERN(_x)	IMPORT _x
#define ASM_GLOBAL(_x)  GLOBAL _x
#define ASM_TYPE(_x,_y)
#define ASM_LABEL(_x)	_x
#elif defined(__ghs__)
#define ASM_WORD(_x)    .word _x
#define ASM_LONG(_x)	.long _x
#define ASM_EXTERN(_x)  .export _x
#define ASM_GLOBAL(_x)  .global _x
#define ASM_TYPE(_x,_y)
#define ASM_LABEL(_x)   _x:
#define ASM_WEAK(_x)    .weak  _x
#define ASM_SIZE(_x,_y)	.size	_x, _y
#define ASM_REPT(_x)	.rept   _x
#define ASM_ENDREPT		.endr
#elif defined(__ICCARM__)
#define ASM_WORD(_x)    DCW _x
#define ASM_LONG(_x)	 _x
#define ASM_EXTERN(_x)  EXTERN _x
#define ASM_GLOBAL(_x)  PUBLIC _x
#define ASM_TYPE(_x,_y)
#define ASM_LABEL(_x)   _x
#define ASM_WEAK(_x)    PUBWEAK  _x
#define ASM_SIZE(_x,_y)
#define ASM_REPT(_x)	REPT   _x
#define ASM_ENDREPT		ENDR
#else
#error Compiler not supported
#endif


#if defined(_ASSEMBLER_)

/* Use as:
 * ASM_SECTION_TEXT(.text) - For normal .text or .text_vle
 */

#if defined(__GNUC__) || defined(__ghs__)
#  if defined(__ghs__) && defined(CFG_VLE)
#    define ASM_SECTION_TEXT(_x) 	.section .vletext,"vax"
#    define ASM_SECTION(_x)  		.section #_x,"vax"
#    define ASM_CODE_DIRECTIVE()    .vle
#  else
#  define ASM_SECTION_TEXT(_x) 	.section #_x,"ax"
#  define ASM_SECTION(_x)  		.section #_x,"ax"
#  endif
#elif defined(__CWCC__)
#  if defined(CFG_VLE)
#    define ASM_SECTION_TEXT(_x) .section _x,text_vle
#  else
#    define ASM_SECTION_TEXT(_x) .section _x,4,"rw"
#  endif
#  define ASM_SECTION(_x)		.section _x,4,"r"
#elif defined(__DCC__)
#  if defined(CFG_VLE)
#    define ASM_SECTION_TEXT(_x) .section _x,4,"x"
#  else
#    define ASM_SECTION_TEXT(_x) .section _x,4,"x"
#  endif
#  define ASM_SECTION(_x)		.section _x,4,"r"
#elif defined(__ICCARM__)
#  if defined(CFG_VLE)
#    define ASM_SECTION_TEXT(_x) SECTION _x:TEXT:NOROOT(4)
#  else
#    define ASM_SECTION_TEXT(_x) SECTION _x:TEXT:NOROOT(4)
#  endif
#  define ASM_SECTION(_x)		 SECTION _x:DATA:NOROOT(4)
#endif

#ifndef ASM_CODE_DIRECTIVE
#define ASM_CODE_DIRECTIVE()
#endif



#endif /* _ASSEMBLER_ */

#endif /*ASM_ARM_H_*/

