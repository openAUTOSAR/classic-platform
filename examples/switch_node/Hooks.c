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
#include <stdio.h>
#include <assert.h>

#define USE_LDEBUG_PRINTF
#include "debug.h"

#include "Mcu.h"


#if 0
#ifdef USE_STARTUPHOOK
#ifdef CFG_MPC55XX
#if !defined(USE_SIMULATOR)
// Quick fix
//#include "Kernel_Offset.h"

extern uint8_t pcb_list[];

#endif
#endif
#endif
#endif



/* Global hooks */
ProtectionReturnType ProtectionHook( StatusType FatalError ) {
	LDEBUG_PRINTF("## ProtectionHook\n");
	return PRO_KILLAPPL;
}

void StartupHook( void ) {
	LDEBUG_PRINTF("## StartupHook\n");

	uint32_t sys_freq = McuE_GetSystemClock();

	LDEBUG_PRINTF("Sys clock %d Hz\n",sys_freq);
}

void ShutdownHook( StatusType Error ) {
	LDEBUG_PRINTF("## ShutdownHook\n");
	while(1);
}

void ErrorHook( StatusType Error ) {
	DisableAllInterrupts();

	LDEBUG_PRINTF("## ErrorHook err=%d\n",Error);
	while(1);
}

void PreTaskHook( void ) {
// 	dbg_printf("## PreTaskHook, taskid=%d\n",task);
}

void PostTaskHook( void ) {
//	dbg_printf("## PostTaskHook, taskid=%d\n",task);
}

#if 0
struct OsHooks os_conf_global_hooks = {
		.StartupHook = StartupHook,
		.ProtectionHook = ProtectionHook,
		.ShutdownHook = ShutdownHook,
		.ErrorHook = ErrorHook,
		.PreTaskHook = PreTaskHook,
		.PostTaskHook = PostTaskHook
		};
#endif
