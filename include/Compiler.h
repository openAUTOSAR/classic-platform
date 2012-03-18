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
#elif defined(__DCC__)
#define CC_EXTENSION
#endif

#if defined(__GNUC__)
#define SECTION_BALIGN(_align )  __attribute__ ((aligned (_align)))
#elif defined(__CWCC__)
#define SECTION_BALIGN(_align )  __attribute__ ((aligned (_align)))
#elif defined(__DCC__)
#define SECTION_BALIGN(_align )  __attribute__ ((aligned (_align)))
#endif

#define DECLARE_WEAK			__attribute__ ((weak))

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
