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








#include "Os.h"
#include "Mcu.h"
#include <stdio.h>
#include <assert.h>
#define USE_TRACE	1
#include "Trace.h"


#if 0
#ifdef USE_STARTUPHOOK
#ifdef CFG_MPC55XX
#if !defined(USE_SIMULATOR)
// Quick fix
//#include "kernel_offset.h"

extern uint8_t pcb_list[];

#endif
#endif
#endif
#endif



/* Global hooks */
ProtectionReturnType ProtectionHook( StatusType FatalError ) {
	dbg_printf("## ProtectionHook\n");
	return PRO_KILLAPPL;
}

void StartupHook( void ) {
	dbg_printf("## StartupHook\n");

	uint32_t sys_freq = McuE_GetSystemClock();

	dbg_printf("Sys clock %d Hz\n",sys_freq);
}

void ShutdownHook( StatusType Error ) {
	dbg_printf("## ShutdownHook\n");
	while(1);
}

void ErrorHook( StatusType Error ) {
	DisableAllInterrupts();

	dbg_printf("## ErrorHook err=%d\n",Error);
	while(1);
}

void PreTaskHook( void ) {
	TaskType task;
	GetTaskID(&task);
	if( task > 10 ) {
		while(1);
	}
// 	dbg_printf("## PreTaskHook, taskid=%d\n",task);
}

void PostTaskHook( void ) {
	TaskType task;
	GetTaskID(&task);
	if( task > 10 ) {
		while(1);
	}
//	dbg_printf("## PostTaskHook, taskid=%d\n",task);
}

#if 0
struct os_conf_global_hooks_s os_conf_global_hooks = {
		.StartupHook = StartupHook,
		.ProtectionHook = ProtectionHook,
		.ShutdownHook = ShutdownHook,
		.ErrorHook = ErrorHook,
		.PreTaskHook = PreTaskHook,
		.PostTaskHook = PostTaskHook
		};
#endif
