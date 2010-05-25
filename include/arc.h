
#ifndef ARC_H_
#define ARC_H_


#define ARRAY_SIZE(_x) 	sizeof(_x)/sizeof((_x)[0])

#define OS_STR__(x)		#x
#define OS_STRSTR__(x) 	OS_STR__(x)

#ifndef MIN
#define MIN(_x,_y) (((_x) < (_y)) ? (_x) : (_y))
#endif
#ifndef MAX
#define MAX(_x,_y) (((_x) > (_y)) ? (_x) : (_y))
#endif

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

/**
 * Converts OSEK StatusType to a string for easy printing
 */
const char *Arc_StatusToString(StatusType);

void Os_Arc_GetStackInfo( TaskType pid, StackInfoType *s );

#define OS_STACK_USAGE(_x) ((((_x)->size - (uint32_t)((_x)->usage - (_x)->top))*100)/(_x)->size)

// int printf(const char *format, ...);

TaskType Os_Arc_CreateIsr( void  (*entry)(void), uint8_t prio, const char *name );

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
