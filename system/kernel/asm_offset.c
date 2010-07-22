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

#include <stddef.h>
#include "internal.h"

#define DECLARE(sym,val) \
	__asm("#define\t" #sym "\t%0" : : "n" ((val)))

void  asm_foo(void) {
	DECLARE(PCB_STACK_CURR_P,	offsetof(OsPcbType, stack));
	DECLARE(PCB_ENTRY_P,		offsetof(OsPcbType, entry));
	DECLARE(SYS_CURR_PCB_P,		offsetof(sys_t, curr_pcb));
	DECLARE(SYS_INT_NEST_CNT, offsetof(sys_t, int_nest_cnt));
	DECLARE(SYS_INT_STACK, offsetof(sys_t, int_stack));
}



