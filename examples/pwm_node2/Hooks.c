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

//#define USE_LDEBUG_PRINTF // Uncomment this to turn debug statements on.
#include "debug.h"



/* Global hooks */
ProtectionReturnType ProtectionHook( StatusType FatalError ) {
	LDEBUG_FPUTS("## ProtectionHook\n");
	return PRO_KILLAPPL;
}

void StartupHook( void ) {
	LDEBUG_FPUTS("## StartupHook\n");

	LDEBUG_PRINTF("Sys clock %u Hz\n",(unsigned)McuE_GetSystemClock());
}

void ShutdownHook( StatusType Error ) {
	LDEBUG_FPUTS("## ShutdownHook\n");
	while(1);
}

void ErrorHook( StatusType Error ) {
	DisableAllInterrupts();

	LDEBUG_PRINTF("## ErrorHook err=%d\n",Error);
	while(1);
}

void PreTaskHook( void ) {
// 	LDEBUG_PRINTF("## PreTaskHook, taskid=%d\n",task);
}

void PostTaskHook( void ) {
//	LDEBUG_PRINTF("## PostTaskHook, taskid=%d\n",task);
}
