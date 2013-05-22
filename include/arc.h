
#ifndef ARC_H_
#define ARC_H_

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_x) 	sizeof(_x)/sizeof((_x)[0])
#endif

#define OS_STR__(x)		#x
#define OS_STRSTR__(x) 	OS_STR__(x)

#ifndef MIN
#define MIN(_x,_y) (((_x) < (_y)) ? (_x) : (_y))
#endif
#ifndef MAX
#define MAX(_x,_y) (((_x) > (_y)) ? (_x) : (_y))
#endif

#define OS_ARC_PCB_NAME_SIZE		16

/**
 * Holds information about stack usage
 */
typedef struct StackInfo_s {
	/* This task was swapped in with this stack */
	void * 	at_swap;
	/* Pointer to the top of the stack */
	void * 	top;
	/* The size of the stack in bytes */
	int    	size;
	/* Pointer to the current place of the stack */
	void *	curr;
	/* Calculated usage in % of total */
	void *	usage;
} StackInfoType;

typedef struct Arc_PcbS {
	char name[OS_ARC_PCB_NAME_SIZE];
} Arc_PcbType;

/**
 * Converts OSEK StatusType to a string for easy printing
 */
const char *Arc_StatusToString(StatusType);

void Os_Arc_GetStackInfo( TaskType pid, StackInfoType *s );

/**
 *
 * @param pcbPtr
 * @param pid
 */
void Os_Arc_GetTaskInfo( Arc_PcbType *pcbPtr, TaskType taskId );

/**
 *
 * @param pcbPtr
 * @param pid
 */
void Os_Arc_GetIsrInfo( Arc_PcbType *pcbPtr, ISRType isrId );


#define OS_STACK_USAGE(_x) ((((_x)->size - (uint32_t)((size_t)(_x)->usage - (size_t)(_x)->top))*100)/(_x)->size)

// int printf(const char *format, ...);

TaskType Os_Arc_CreateIsr( void  (*entry)(void), uint8_t prio, const char *name );

int Os_Arc_GetIsrCount( void );

/**
 * Get the task activation limit
 * @param task
 */
int Os_ArcTest_GetTaskActivationLimit( TaskType task );

/**
 * Fake Irq. Used for API testing.
 * @param level The level
 */
void Os_ArcTest_SetIrqNestLevel( int level );


#endif /* ARC_H_ */
