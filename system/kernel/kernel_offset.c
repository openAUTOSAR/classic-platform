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

