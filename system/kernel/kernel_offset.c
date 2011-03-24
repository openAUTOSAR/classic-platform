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


#include <stddef.h>

#include "Os.h"
//#include "app_i.h"
#include "task_i.h"
#include "sys.h"
#include "application.h"


#if defined(__GNUC__)

#define DECLARE(sym,val) \
	__asm("#define " #sym " %0" : : "i" ((val)))

void  oil_foo(void) {

/* PCB */

#elif defined(__CWCC__)
#define DECLARE(_var,_offset) \
    __declspec(section ".apa") char _var[100+(_offset)]
#pragma section ".apa" ".apa"
#endif


	DECLARE(PCB_T_SIZE,			sizeof(OsTaskVarType));
#if	(OS_USE_APPLICATIONS == STD_ON)
//	DECLARE(APP_T_SIZE,			sizeof(OsApplicationType));

#endif
	DECLARE(PCB_STACK_CURR_P,	offsetof(OsTaskVarType, stack));
	DECLARE(PCB_ENTRY_P,		offsetof(OsTaskVarType, entry));
	DECLARE(SYS_CURR_PCB_P,		offsetof(Os_SysType, currTaskPtr));
	DECLARE(SYS_INT_NEST_CNT, 	offsetof(Os_SysType, intNestCnt));
	DECLARE(SYS_INT_STACK, 		offsetof(Os_SysType, intStack));
#if defined(__GNUC__)
}
#endif

