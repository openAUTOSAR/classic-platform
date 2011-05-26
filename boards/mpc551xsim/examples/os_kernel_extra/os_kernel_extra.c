/*
 * kernel_extra.c
 *
 *  Created on: 13 mar 2011
 *      Author: mahi
 *
 * DESCRIPTION
 *  Shows the features of KERNEL_EXTRA and normal resources.
 *
 */

#include "Os.h"
#include "isr.h"
#include "irq.h"
#include <stdio.h>

OsMutexType mutex;
OsSemaphoreType semIsr;

static int state = 0;

enum {
	DEMO_SEMAPHORE = 0,
	DEMO_SEMAPHORE_SLEEP,
	DEMO_SLEEP,
	DEMO_MUTEX,
	DEMO_RESOURCE
};

/**
 * Function that prints str to stdout and can activate
 * another task if interrupt is set.
 *
 * @param interrupt  true if a call to ActivateTask() should be done
 * @param str        The string tp print
 */

void mutexPrint( bool interrupt, char *str ) {
	WaitMutex(&mutex);
	if( interrupt ) {
		ActivateTask(TASK_ID_bTask);
	}
	printf("Mutex protected: %s\n",str);
	ReleaseMutex(&mutex);
}

void resourcePrint( bool interrupt, char *str ) {
	GetResource(RES_ID_Resource1);
	if( interrupt ) {
		/* Activating another task (although it could have higher prio)
		 * will not change anything, since the priority of the activated
		 * task will be at most equal after getting the resource.
		 */
		ActivateTask(TASK_ID_bTask);
	}
	printf("Resource protected: %s\n",str);
	ReleaseResource(RES_ID_Resource1);
}


void bTask( void ) {

	switch(state) {
	case 0:
		fputs("SignalSemaphore\n",stdout);
		SignalSemaphore(&semIsr);
		break;
	case 1:
		resourcePrint(false,"from bTask");
		break;
/*
	case 1:
		mutexPrint(false, "from bTask");
		break;
*/
	default:
		assert(0);
	}

	TerminateTask();
}


void eTask1( void ) {

	InitSemaphore(&semIsr,0 );

	for(;;) {

		/*
		 * Example: Use SignalSemaphore() and WaitSemaphore()
		 */

		state = 0;
		/* Trigger bTaskSem that calls SignalSemaphore */
		SetRelAlarm(ALARM_ID_Alarm1,2,0);
		WaitSemaphore(&semIsr, TICK_MAX);

		/*
		 * Example: Traditional GetResource() and ReleaseResource() calls
		 */
		state = 1;
		resourcePrint( true,"from eTask1");

#if 0
		/*
		 * Example: Use a shared resource that is protected with
		 *          WaitMutex() and  ReleaseMutex()
		 */
		state = 2;
		mutexPrint( true,"from eTask1");
#endif

		/* OSEK resources */
		GetResource(RES_ID_Resource1);
		Irq_GenerateSoftInt(5);
		ReleaseResource(RES_ID_Resource1);

	}
}

void OsIdle( void ) {
	for(;;);
}

