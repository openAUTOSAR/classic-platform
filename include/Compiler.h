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
#ifndef COMPILER_H
#define COMPILER_H

/* REQ:COMPILER040,049,051 */
#define AUTOMATIC
#define STATIC 	static
#define NULL_PTR	((void *)0)

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


#if defined(__GNUC__) || defined(__CWCC__) || defined(__DCC__) || defined(__ghs__)
#define __weak		 		__attribute__ ((weak))
#else
#define __weak
#endif

#if defined(__GNUC__)
#define __balign(x)       __attribute__ ((aligned (x)))
#elif defined(__ghs__)
#define __balign(x)       __attribute__ ((aligned (x)))
#elif defined(__CWCC__)
#define __balign(x)       __attribute__ ((aligned (x)))
#elif defined(__DCC__)
#define __balign(x)       __attribute__ ((aligned (x)))
#elif defined(__ICCHCS12__)
#define Pragma(x) _Pragma(#x)
#define __balign(x)       Pragma(data_alignment=_align)
#else
#error Compiler not defined.
#endif

#define SECTION_BALIGN(x)  __balign(x)

#if defined(__ICCHCS12__)
#define restrict
#define DECLARE_WEAK
#define __simple __simple
#else
#define DECLARE_WEAK			__attribute__ ((weak))
#define __simple
#endif

/* Does this really work on all compilers.... */
#define INLINE __inline__

/* REQ:COMPILER005 */
#define FUNC(rettype,memclass) rettype

#define P2VAR(ptrtype, memclass, ptrclass) ptrtype *

#define P2CONST(ptrtype, memclass, ptrclass) const ptrtype *

#define CONSTP2VAR(ptrtype,memclass,ptrclass) ptrtype * const

#define CONSTP2CONST(ptrtype, memclass, ptrclass) const ptrtype * const

#define P2FUNC(rettype,ptrclass,fctname) rettype (*fctname)

#define CONST(consttype,memclass) const consttype

#define VAR(vartype,memclass) vartype


#endif /* COMPILER_H */	
