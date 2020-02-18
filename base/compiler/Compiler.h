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

#include "Compiler_Arc.h"

#define INLINE __inline__

/* REQ:COMPILER005 */
#define FUNC(rettype,memclass) rettype

#define P2VAR(ptrtype, memclass, ptrclass) ptrtype *

#define P2CONST(ptrtype, memclass, ptrclass) const ptrtype *

#define CONSTP2VAR(ptrtype,memclass,ptrclass) ptrtype * const

#define CONSTP2CONST(ptrtype, memclass, ptrclass) const ptrtype * const

#define P2FUNC(rettype,ptrclass,fctname) rettype (*fctname)

#define CONST(consttype,memclass) const consttype

#define FUNC_P2CONST(rettype, ptrclass, memclass) const ptrclass rettype * memclass

#define VAR(vartype,memclass) memclass vartype

#include "Compiler_Cfg.h"

#endif /* COMPILER_H */	
