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
#define ISR_DECLARE_ISR2(_name, _entry, _unique, _vector,_priority,_app )        \
	  const OsIsrConstType _entry ## _unique = { \
			.vector = _vector,   \
			.type = ISR_TYPE_2, \
			.priority = _priority,      \
			.entry = _entry,      \
			.name = _name,      \
			.resourceMask = 0,  \
			.timingProtPtr = NULL, \
			.appOwner = _app,      \
		  };                    \

#define _ISR_INSTALL_ISR2(_name, _entry, _unique, _vector,_priority,_app )        \
	do { \
	  const OsIsrConstType _entry ## _unique = { \
			.vector = _vector,   \
			.type = ISR_TYPE_2, \
			.priority = _priority,      \
			.entry = _entry,      \
			.name = _name,      \
			.resourceMask = 0,  \
			.timingProtPtr = NULL, \
			.appOwner = _app,      \
		  };                    \
	  Os_IsrAdd( & _entry ## _unique);   \
	} while(0);

#define ISR_INSTALL_ISR2(_name,_entry, _vector,_priority,_app)        \
		_ISR_INSTALL_ISR2(_name,_entry, __LINE__, _vector,_priority,_app)


/* ----------------------------[typedef]-------------------------------------*/



/* STD container : OsIsrResourceLock
 * Class: 2 and 4
 *
 * OsIsrResourceLockBudget  	1    Float in seconds (MAXRESOURCELOCKINGTIME)
 * OsIsrResourceLockResourceRef 1    Ref to OsResource
 * */

typedef struct OsIsrResourceLock {
	uint32_t lockBudget;
	uint32_t lockResourceRef; 	/* Wrong type */
} OsIsrResourceLockType;


/* STD container : OsIsrTimingProtection
 * Class: 2 and 4
 *
 * OsIsrAllInterruptLockBudget  0..1 float
 * OsIsrExecutionBudget 		0..1 float
 * OsIsrOsInterruptLockBudget 	0..1 float
 * OsIsrTimeFrame 				0..1 float
 * OsIsrResourceLock[C] 		0..*
 * */

typedef struct OsIsrTimingProtection {
	uint32_t allInterruptLockBudget;
	uint32_t executionBudget;
	uint32_t osInterruptLockBudget;
	uint32_t timeFrame;
	uint32_t resourceLock;		/* Wrong type */
} OsIsrTimingProtectionType;

typedef struct {
	void   		*curr;	/* Current stack ptr( at swap time ) */
	void   		*top;	/* Top of the stack( low address )   */
	uint32		size;	/* The size of the stack             */
} OsIsrStackType;


/* STD container : OsIsr
 * Class: ALL
 *
 * OsIsrCategory:				1    CATEGORY_1 or CATEGORY_2
 * OsIsrResourceRef:			0..* Reference to OsResources
 * OsIsrTimingProtection[C] 	0..1
 * */

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
	ISRType id;
	OsIsrStackType		stack;
	int					state;
	const OsIsrConstType *constPtr;
} OsIsrVarType;


/* ----------------------------[functions]-----------------------------------*/

void Os_IsrInit( void );
ISRType Os_IsrAdd( const OsIsrConstType * restrict isrPtr );
const OsIsrConstType * Os_IsrGet( int16_t vector);
void Os_IsrGetStackInfo( OsIsrStackType *stack );
void *Os_Isr( void *stack, int16_t vector);
#if defined(CFG_ARM_CM3)
void Os_Isr_cm3( void *isr_p );
void TailChaining(void *stack);
#endif

static inline ApplicationType Os_IsrGetApplicationOwner( ISRType id ) {
	ApplicationType rv = INVALID_OSAPPLICATION;

#if (OS_ISR_CNT!=0)
	if( id < OS_ISR_CNT ) {
		Os_IsrVarList[id]->constPtr->appOwner;
	}
#endif
	return rv;
}

#endif /*ISR_H_*/
