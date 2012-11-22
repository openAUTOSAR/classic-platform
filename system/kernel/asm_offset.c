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

/* A steal in an idea from
 * http://blogs.sun.com/nickstephen/entry/some_assembly_required_-_down
 * According to blog original BSD license..
 *
 * Generate assembler from this file( -S option for GNU ).
 * extract the "#defines" with some tool e.g. "sed -n '/#define/p' <asm_file>"
 */

#define restrict
#include <stddef.h>
#include "task_i.h"
#include "sys.h"

#if defined(__GNUC__)

/* For ARM this will generate to #define APA #12 */
#define DECLARE(sym,val) \
	__asm("#define\t" #sym "\t%0" : : "n" ((val)))

void  asm_foo(void) {
#elif defined(__CWCC__)
#define DECLARE(_var,_offset) \
    __declspec(section ".apa") char _var[100+ (_offset)]
#pragma section ".apa" ".apa"
#elif (defined(__DCC__))
#define DECLARE(_sym,_val) \
	const int arc_dummy_ ## _sym = _val
#else
//#define DECLARE(_sym,_val) const char arc_dummy_ ## _sym = _val
#define DECLARE(_sym,_val) const int arc_dummy_ ## _sym = _val
#endif
	DECLARE(PCB_STACK_CURR_P,	offsetof(OsTaskVarType, stack));
	DECLARE(PCB_CONST_P,		offsetof(OsTaskVarType, constPtr));
////	DECLARE(PCB_ENTRY_P,		offsetof(OsTaskVarType, entry));
	DECLARE(SYS_CURR_PCB_P,		offsetof(Os_SysType, currTaskPtr));
	DECLARE(SYS_INT_NEST_CNT, offsetof(Os_SysType, intNestCnt));
	DECLARE(SYS_INT_STACK, offsetof(Os_SysType, intStack));
#if defined(__GNUC__)
}
#endif


