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








#ifndef TRACE_H_
#define TRACE_H_

#include "simple_printf.h"
/**
 *
 * NOTE!!!!
 * Do not use this in a header file. Should be used in the *.c file like this.
 *
 * #define USE_TRACE
 * #include "Trace.h"
 *
 * Macro's for debugging and tracing
 *
 * Define USE_DEBUG and DBG_LEVEL either globally( e.g. a makefile )
 * or in a specific file.  The DBG_LEVEL macro controls the amount
 * of detail you want in the debug printout.
 * There are 3 levels:
 * DEBUG_LOW    - Used mainly by drivers to get very detailed
 * DEBUG_MEDIUM - Medium detail
 * DEBUG_HIGH   - General init
 *
 * Example:
 * #define DEBUG_LVL	DEBUG_HIGH
 * DEBUG(DEBUG_HIGH,"Starting GPT");
 *
 * TRACE
 *   TODO:
 *
 */

#define DEBUG_LOW		1
#define DEBUG_MEDIUM	2
#define DEBUG_HIGH		3

#ifndef DEBUG_LVL
#define DEBUG_LVL		2
#endif

#define CH_ISR		0
#define CH_PROC		1

#if defined(USE_DEBUG)
#define DEBUG(_level,...) \
	do { \
		if(_level>=DEBUG_LVL) { \
			simple_printf (__VA_ARGS__); \
		}; \
	} while(0);

#else
#define DEBUG(_level,...)
#endif

#if defined(USE_DEBUG)
#define dbg_printf(format,...) simple_printf(format,## __VA_ARGS__ )
#else
#define dbg_printf(format,...)
#endif


#endif /*RAMLOG_H_*/
