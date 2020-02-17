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


#ifndef ARC_H_
#define ARC_H_

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_x) 	sizeof(_x)/sizeof((_x)[0])
#endif

#define OS_STR__(x)		#x
#define OS_STRSTR__(x) 	OS_STR__(x)

#if !defined(STR__)
#define STR__(x)     #x
#endif
#if !defined(STRSTR__)
#define STRSTR__(x)  OS_STR__(x)
#endif

#ifndef MIN
#define MIN(_x,_y) (((_x) < (_y)) ? (_x) : (_y))
#endif
#ifndef MAX
#define MAX(_x,_y) (((_x) > (_y)) ? (_x) : (_y))
#endif

#define OS_ARC_PCB_NAME_SIZE		16

#define OS_STACK_USAGE(_x) ((((_x)->size - (uint32_t)((size_t)(_x)->usage - (size_t)(_x)->top))*100)/(_x)->size)

#include "Os.h"
#include "arc_assert.h"

#define ARC_TASKTYPE_BASIC      0
#define ARC_TASKTYPE_EXENDED    1

#define OS_ARC_F_TASK_ALL       0xffffffffUL
#define OS_ARC_F_TASK_BASIC     (1UL<<0u)
#define OS_ARC_F_TASK_STACK     (1UL<<1u)


/**
 * Holds information about stack usage
 */
typedef struct StackInfo_s {
    /* This task was swapped in with this stack */
    void * 	at_swap;
    /* Pointer to the top of the stack */
    void * 	top;
    /* The size of the stack in bytes */
    uint32  size;
    /* Pointer to the current place of the stack */
    void *	curr;
    /* Calculated usage in % of total */
    void *	usage;

    /* Calculated usage in % of total */
    uint8  usageInPercent;
} StackInfoType;


typedef struct Arc_PcbS {
    char name[OS_ARC_PCB_NAME_SIZE];
    uint32 tasktype;
    StackInfoType stack;
} Arc_PcbType;

/**
 * Converts OSEK StatusType to a string for easy printing
 * @param status
 */
const char *Arc_StatusToString(StatusType status);

/**
 * Get task's stack information
 * @param pid
 * @param s
 */
void Os_Arc_GetStackInfo( TaskType pid, StackInfoType *s );

/**
 * Get task name, type and stack information
 * @param pcbPtr
 * @param taskId
 * @param flags
 */
void Os_Arc_GetTaskInfo( Arc_PcbType *pcbPtr, TaskType taskId, uint32 flags );

/**
 * Get ISR name
 * @param pcbPtr
 * @param isrId
 */
void Os_Arc_GetIsrInfo( Arc_PcbType *pcbPtr, ISRType isrId );

/**
 * Get the number of configured ISRs (category1 and 2)
 * @return Total number of configured ISRs
 */
ISRType Os_Arc_GetIsrCount( void );

/**
 * If the configured trap is not handled by callouts then
 * this function will be called.and shutdown the system
 * @param excpetion: excpetion id, pData: pcb data (this argument is not handled in this function)
 */
void Os_Arc_Panic(uint32 exception, void *pData);

#endif /* ARC_H_ */
