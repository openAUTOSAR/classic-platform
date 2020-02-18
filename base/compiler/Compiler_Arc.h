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


/* See CopmpilerAbstraction.pdf */
#ifndef COMPILER_ARC_H
#define COMPILER_ARC_H

/* REQ:COMPILER040,049,051 */

#if defined(__GNUC__)
#define CC_EXTENSION 	__extension__
#elif defined(__CWCC__)
#define CC_EXTENSION
#pragma read_only_switch_tables on
#elif defined(__DCC__) || defined(__ghs__)
#define CC_EXTENSION
#endif

#if defined(__ghs__)
#define typeof __typeof__
#endif

#if defined(__ARMCC_VERSION)
#define asm __asm
#endif

#if defined(__GNUC__) || defined(__CWCC__) || defined(__DCC__) || defined(__ghs__)
#define __weak		 		__attribute__ ((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define __weak				__weak
#else
#define __weak
#endif

#if defined(__GNUC__) || defined(__ghs__) || defined(__CWCC__) || defined(__DCC__) || defined(__ARMCC_VERSION)
#define __balign(x)       __attribute__ ((aligned (x)))
#elif defined(__IAR_SYSTEMS_ICC__)
#define Pragma(x) _Pragma(#x)
#define __balign(x)       Pragma(data_alignment=(x))
#elif defined(_WIN32)
#define __balign(x)       __declspec    ((align(x)))
#else
#error Compiler not defined.
#endif

#define SECTION_BALIGN(x)  __balign(x)

#define DECLARE_WEAK			__attribute__ ((weak))

#if defined(__IAR_SYSTEMS_ICC__)
#define LOCAL_INLINE    Pragma(inline)
#else
#define LOCAL_INLINE    static __inline__
#endif

#if defined(CFG_BULLSEYE)
#define __CODE_COVERAGE_ON__        _Pragma("BullseyeCoverage restore")
#define __CODE_COVERAGE_OFF__       _Pragma("BullseyeCoverage save off")
#define __CODE_COVERAGE_IGNORE__    _Pragma("BullseyeCoverage ignore")
#define __CODE_COVERAGE_IGNORE2__    _Pragma("BullseyeCoverage ignore:2")
#define __CODE_COVERAGE_IGNORE3__    _Pragma("BullseyeCoverage ignore:3")

#pragma BullseyeCoverage ignore:3

#else
#define __CODE_COVERAGE_ON__
#define __CODE_COVERAGE_OFF__
#define __CODE_COVERAGE_IGNORE__
#define __CODE_COVERAGE_IGNORE1__
#define __CODE_COVERAGE_IGNORE2__
#define __CODE_COVERAGE_IGNORE3__
#endif

#endif /* COMPILER_H */	
