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








#ifndef DEBUG_H_
#define DEBUG_H_

#include <stdio.h>

#ifdef USE_DBG_PRINTF

#define DBG_T32_TERMINAL	1
// Swap hook
#define DGB_HOOK_SWAP		1

#define DBG_START_OS		1

#define DBG_EXCEPTION 		1

// C99, see gcc manual for more info if no args..
// WARNING!! This eats lots and lots of stack, about 500bytes ??

//#defineos_printf(format,...) printf(format,__VA_ARGS__)
#define os_printf(format,...) simple_printf(format,__VA_ARGS__)
// #define os_printf(format,...) iprintf(format,__VA_ARGS__)

// Macro's that don't take the whole stack...
// #define os_print_hex()
// #define os_print(char *str);

#else

#define os_printf(format,...)

#endif




#endif /*DEBUG_H_*/
