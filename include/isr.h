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

#include "Os.h"
#include "sys/queue.h"
#include "os_trap.h"

struct OsResource;


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
 */

/* ----------------------------[includes]------------------------------------*/
/* ----------------------------[define]--------------------------------------*/

#define ISR_TYPE_1			0
#define ISR_TYPE_2			1
#define VECTOR_ILL			0xff

/* ----------------------------[macro]---------------------------------------*/

#ifdef CFG_DRIVERS_USE_CONFIG_ISRS
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
            .appOwner = _app,      \
          };                    \
      (void)OS_TRAP_Os_IsrAdd( & _entry ## _unique);   \
    } while(FALSE);



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
            .appOwner = _app,      \
          };                    \
     (void) OS_TRAP_Os_IsrAdd( & _entry ## _unique);   \
    } while(0);

#define _ISR_INSTALL_ISR1(_name,_entry, _unique, _vector,_priority,_app)        \
        __ISR_INSTALL_ISR1(_name,_entry, _unique, _vector,_priority,_app)

#define ISR_INSTALL_ISR1(_name,_entry, _vector,_priority,_app)        \
        _ISR_INSTALL_ISR1(_name,_entry, __LINE__, _vector,_priority,_app)


#endif

/* ----------------------------[typedef]-------------------------------------*/

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
 * */

typedef struct OsIsrConst {
    const char 		*name;
    uint8           core;
    uint8           priority;
    sint16          vector;
    sint16          type;
    void            (*entry)( void );
    ApplicationType	appOwner;
    /* Mapped against OsIsrResourceRef */
    uint32          resourceMask;
} OsIsrConstType;

/*
 *
 */
typedef struct OsIsrVar{
    ISRType id;
#if defined(CFG_OS_ISR_HOOKS)
    ISRType preemtedId;
#endif
    int					state;
    const OsIsrConstType *constPtr;
#if defined(CFG_TMS570) || defined(CFG_ARMV7_AR)
    sint16 activeVector;
#endif

    /* List of resource held by this ISR */
    TAILQ_HEAD(,OsResource) resourceHead;
} OsIsrVarType;


/* ----------------------------[functions]-----------------------------------*/

#if OS_ISR_MAX_CNT!=0
extern OsIsrVarType Os_IsrVarList[OS_ISR_MAX_CNT];
#endif

/**
 * Init the ISR system
 */
void Os_IsrInit( void );

/**
 * Add and ISR.
 * @param isrPtr
 * @return
 */
ISRType Os_IsrAdd( const OsIsrConstType * isrPtr );

/**
 * Add ISR with an id
 * @param isrPtr    Pointer to ISR information
 * @param id        The id we want to add it with
 */
void Os_IsrAddWithId( const OsIsrConstType * isrPtr, ISRType id );

/**
 * Remove an installed ISR
 * @param vector    The vector
 * @param type      Not used
 * @param priority  Not used
 * @param app       Not used.
  */
void Os_IsrRemove( sint16 vector, sint16 type, uint8 priority, ApplicationType app );

/**
 * Get stack information for an ISR.
 * @param stack In/Out pointer to stack information
 */
void Os_IsrGetStackInfo( OsIsrStackType *stack );

/**
 * Called by all interrupts.
 *
 * @param stack             Pointer to current stack
 * @param isrTableIndex     Table index for vector
 */
void *Os_Isr( void *stack, uint16 isrTableIndex);

#if defined(CFG_TMS570)
void *Os_Isr_cr4( void *stack, sint16 virtualVector, sint16 vector );
#endif
#if defined(CFG_ARM_CM3)
void Os_Isr_cm3( sint16 vector );
void TailChaining(void *stack);
#endif

OsIsrVarType *Os_IsrGet( ISRType id );
ApplicationType Os_IsrGetApplicationOwner( ISRType id );
void Os_IsrResourceAdd( struct OsResource *rPtr, OsIsrVarType *isrPtr);
void Os_IsrResourceRemove(struct OsResource *rPtr , OsIsrVarType *isrPtr);

#endif /*ISR_H_*/
