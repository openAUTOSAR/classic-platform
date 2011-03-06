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

#ifndef ISR_H_
#define ISR_H_

/*
 * INCLUDE "RULES"
 *  Since this types and methods defined here are used by the drivers, they should
 *  include it. E.g. #include "isr.h"
 *
 *  This file is also used internally by the kernel
 *
 *
 *  irq_types.h ( Vector enums )
 *  irq.h       ( Interface )
 *
 *  Problem:
 *    Os_Cfg.h needs types from isr.h
 *
 */

/* ----------------------------[includes]------------------------------------*/
/* ----------------------------[define]--------------------------------------*/

#define ISR_TYPE_1			0
#define ISR_TYPE_2			1

/* ----------------------------[macro]---------------------------------------*/
/* ----------------------------[typedef]-------------------------------------*/


typedef struct {
	void   		*curr;	/* Current stack ptr( at swap time ) */
	void   		*top;	/* Top of the stack( low address )   */
	uint32		size;	/* The size of the stack             */
} OsIsrStackType;


/*
 * This type should map against the AUTOSAR OsISR type
 */
typedef struct {
	const char 		*name;
	uint8_t			core;
	int16_t			vector;
	int16_t 		type;
	int16_t			priority;
	void 			(*entry)();
	uint32_t		appOwner;
	/* Mapped against OsIsrResourceRef */
	uint32_t		resourceMask;
#if (  OS_USE_ISR_TIMING_PROT == STD_ON )
	/* Mapped against OsIsrTimingProtection[C] */
	OsIsrTimingProtectionType *timingProtPtr;
#else
	void *timingProtPtr;
#endif
} OsIsrConstType;

/*
 *
 */
typedef struct {
	OsIsrStackType		stack;
	int					state;
	const OsIsrConstType *constPtr;
} OsIsrVarType;


/* ----------------------------[functions]-----------------------------------*/

void Os_IsrInit( void );
ISRType Os_IsrAdd( const OsIsrConstType * restrict isrPtr );
const OsIsrConstType * Os_IsrGet( int16_t vector);

#endif /*ISR_H_*/
