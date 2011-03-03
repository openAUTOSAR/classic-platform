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
#endif

/* REQ:COMPILER005 */
/* TODO: skip the memclass for now */
#define FUNC(rettype,memclass) rettype

/* REQ:COMPILER006 */
#define P2VAR(ptrtype, memclass, ptrclass) ptrtype

/* TODO: memclass and more */
#define P2CONST(ptrtype, memclass, ptrclass) ptrtype

/* TODO: memclass and more */
#define CONSTP2VAR(ptrtype,memclass,ptrclass) ptrclass ptrtype * const

/* TODO: */
#define P2FUNC(rettype,ptrclass,fctname) retype (*ptrclass,fctname)

/* TODO: */
#define CONST(consttype,memclass) const consttype

/* TODO: */
#define VAR(vartype,memclass) vartype



#endif /* COMPILER_H */	
