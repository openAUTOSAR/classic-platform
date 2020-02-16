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

#ifndef ISR_H_
#define ISR_H_

#include "task_i.h"
#include "resource_i.h"

/*
 * INCLUDE "RULES"
 *  Since the types and methods defined here are used by the drivers, they should
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
#if 0
//#ifdef CFG_DRIVERS_USE_CONFIG_ISRS
#define ISR_INSTALL_ISR2( _name, _entry, _vector, _priority, _app )
#define ISR_INSTALL_ISR1(_name,_entry, _vector,_priority,_app)
#else
#define ISR_DECLARE_ISR2(_name, _entry, _unique, _vector,_priority,_app )        \
	  static const OsIsrConstType _entry ## _unique = { \
			.vector = _vector,   \
			.type = ISR_TYPE_2, \
			.priority = _priority,      \
			.entry = _entry,      \
			.name = _name,      \
			.resourceMask = 0,  \
			.timingProtPtr = NULL, \
			.appOwner = _app,      \
		  };                    \


#define __ISR_INSTALL_ISR2(_name, _entry, _unique, _vector,_priority,_app )        \
	do { \
	  static const OsIsrConstType _entry ## _unique = { \
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



#define _ISR_INSTALL_ISR2(_name,_entry, _unique, _vector,_priority,_app)        \
		__ISR_INSTALL_ISR2(_name,_entry, _unique, _vector,_priority,_app)

#define ISR_INSTALL_ISR2(_name,_entry, _vector,_priority,_app)        \
		_ISR_INSTALL_ISR2(_name,_entry, __LINE__, _vector,_priority,_app)


#define ISR_DECLARE_ISR1(_name, _entry, _unique, _vector,_priority,_app )        \
	  static const OsIsrConstType _entry ## _unique = { \
			.vector = _vector,   \
			.type = ISR_TYPE_1, \
			.priority = _priority,      \
			.entry = _entry,      \
			.name = _name,      \
			.resourceMask = 0,  \
			.timingProtPtr = NULL, \
			.appOwner = _app,      \
		  };                    \

#define __ISR_INSTALL_ISR1(_name, _entry, _unique, _vector,_priority,_app )        \
	do { \
	  static const OsIsrConstType _entry ## _unique = { \
			.vector = _vector,   \
			.type = ISR_TYPE_1, \
			.priority = _priority,      \
			.entry = _entry,      \
			.name = _name,      \
			.resourceMask = 0,  \
			.timingProtPtr = NULL, \
			.appOwner = _app,      \
		  };                    \
	  Os_IsrAdd( & _entry ## _unique);   \
	} while(0);

#define _ISR_INSTALL_ISR1(_name,_entry, _unique, _vector,_priority,_app)        \
		__ISR_INSTALL_ISR1(_name,_entry, _unique, _vector,_priority,_app)

#define ISR_INSTALL_ISR1(_name,_entry, _vector,_priority,_app)        \
		_ISR_INSTALL_ISR1(_name,_entry, __LINE__, _vector,_priority,_app)

#endif

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

typedef struct OsIsrConst {
	const char 		*name;
	uint8_t			core;
	int16_t			vector;
	int16_t 		type;
	int16_t			priority;
	void 			(*entry)( void );
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
typedef struct OsIsrVar{
	ISRType id;
#if defined(CFG_OS_ISR_HOOKS)
	ISRType preemtedId;
#endif
//	OsIsrStackType		stack;
	int					state;
	const OsIsrConstType *constPtr;
#if defined(CFG_ARM_CR4)
	int16_t activeVector;
#endif

	/* List of resource held by this ISR */
	TAILQ_HEAD(,OsResource) resourceHead;
} OsIsrVarType;


/* ----------------------------[functions]-----------------------------------*/

#if OS_ISR_MAX_CNT!=0
extern OsIsrVarType Os_IsrVarList[OS_ISR_MAX_CNT];
#endif

void Os_IsrInit( void );
ISRType Os_IsrAdd( const OsIsrConstType * restrict isrPtr );
void Os_IsrGetStackInfo( OsIsrStackType *stack );
void *Os_Isr( void *stack, int16_t vector);
#if defined(CFG_ARM_CR4)
void *Os_Isr_cr4( void *stack, int16_t virtualVector, int16_t vector );
#endif
#if defined(CFG_ARM_CM3)
void Os_Isr_cm3( int16_t vector );
void TailChaining(void *stack);
#endif

const OsIsrVarType *Os_IsrGet( ISRType id );
ApplicationType Os_IsrGetApplicationOwner( ISRType id );


static inline void Os_IsrResourceAdd( OsResourceType *rPtr, OsIsrVarType *isrPtr) {
	/* Save old task prio in resource and set new task prio */
	rPtr->owner = isrPtr->id;

	assert( rPtr->type != RESOURCE_TYPE_INTERNAL );
}

static inline  void Os_IsrResourceRemove( OsResourceType *rPtr , OsIsrVarType *isrPtr) {
	assert( rPtr->owner == isrPtr->id );
	rPtr->owner = NO_TASK_OWNER;

	if( rPtr->type != RESOURCE_TYPE_INTERNAL ) {
		/* The list can't be empty here */
		assert( !TAILQ_EMPTY(&isrPtr->resourceHead) );

		/* The list should be popped in LIFO order */
		assert( TAILQ_LAST(&isrPtr->resourceHead, head) == rPtr );

		/* Remove the entry */
		TAILQ_REMOVE(&isrPtr->resourceHead, rPtr, listEntry);
	}
}

static inline void Os_IsrResourceFreeAll( OsIsrVarType *isrPtr ) {
	OsResourceType *rPtr;

	/* Pop the queue */
	TAILQ_FOREACH(rPtr, &isrPtr->resourceHead, listEntry ) {
		Os_IsrResourceRemove(rPtr,isrPtr);
	}
}

static inline _Bool Os_IsrOccupiesResources(  OsIsrVarType *isrPtr ) {
	return !(TAILQ_EMPTY(&isrPtr->resourceHead));
}





#endif /*ISR_H_*/
