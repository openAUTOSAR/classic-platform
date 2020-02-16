/*
* sempphore.c
 *
 *  Created on: 2013-10-31
 *      Author: Zhang Shuzhou
 */
#include "stdio.h"
#include <stdlib.h>
#include "Os.h"
#include "usb_semaphore.h"
#include "usb_util.h"
#include "bcm2835.h"
#include "Std_Types.h"
#include "Uart.h"
#include "internal.h"
#include "task_i.h"
#include "sys.h"

#define USB_SEM_MAX 9
struct semlist
{
	uint8 		val;
	boolean 	used;
	uint32 		event;
	TaskType 	task[USB_SEM_MAX];
	uint8 		taskIndex;
};
static struct semlist semlist_usb[USB_SEM_MAX];

syscall wait(semaphore sem)
{

	return OK;
}

syscall semsignal(semaphore sem)
{

	return OK;

}

syscall signaln(semaphore sem, int count)
{

    return OK;
}

semaphore semcreate(int count)
{



	return OK;
}

syscall semfree(semaphore sem)
{

	return OK;
}

syscall semcount(semaphore sem)
{

    return OK;
}


uint8 UsbGetSem(semaphore sem)
{
	uint8 res = E_OK;
	irqmask im;

	im = disable();

	if(semlist_usb[sem].val > 0){
		res = E_NOT_OK;
	}else{
		semlist_usb[sem].val = 1;
	}
	restore(im);
	return res;
}

/*-----------------------------------------------------------------------------------*/
//  Creates and returns a new semaphore. The "count" argument specifies
//  the initial state of the semaphore. TBD finish and test
sem_err usb_sem_new(semaphore *sem, uint count)
{
	uint8 i;
	irqmask im;

	im = disable();

	// use semaphore pool
	for(i = 0; i < USB_SEM_MAX; i++)
	{
		if(semlist_usb[i].used == FALSE)
		{

			semlist_usb[i].used = TRUE;
			if(count == 0)
			{
				if(UsbGetSem(i) != E_OK)
				{
					restore(im);
					printf("infor: fail to get semaphore.\r\n");
					return SEM_NOT_OK;
				}
			}
			restore(im);
			*sem = i;
			return SEM_OK;
		}
    }
    restore(im);
	return SEM_NOT_OK;
}


/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread while waiting for the semaphore to be
  signaled. If the "timeout" argument is non-zero, the thread should
  only be blocked for the specified time (measured in
  milliseconds).

  If the timeout argument is non-zero, the return value is the number of
  milliseconds spent waiting for the semaphore to be signaled. If the
  semaphore wasn't signaled within the specified time, the return value is
  SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
  (i.e., it was already signaled), the function may return zero.
*/
uint32 usb_sem_wait(semaphore *sem, uint32 timeout)
{
	TickType StartTime, EndTime, Elapsed;
	irqmask im;
	StartTime = GetOsTick();


	if(	timeout != 0)
	{
		while( UsbGetSem( *sem) != E_OK )
		{
//			Sleep(1);
			EndTime = GetOsTick();
			Elapsed = EndTime - StartTime;
			if(Elapsed > timeout)
			{
				return SEM_TIMEOUT;
			}
		}
		EndTime = GetOsTick();
		Elapsed = EndTime - StartTime;
		if( Elapsed == 0 )
		{
			Elapsed = 1;
		}
		return (Elapsed); // return time blocked TBD test
	}
	else // must block without a timeout
	{
		while( UsbGetSem(*sem) != E_OK )
		{
			TaskType TaskID;
			im = disable();
//			int intNestCnt = OS_SYS_PTR->intNestCnt;
//			if(intNestCnt > 0){
//				OS_SYS_PTR->intNestCnt = 0;
//			}
            GetTaskID(&TaskID);
			semlist_usb[*sem].task[semlist_usb[*sem].taskIndex++] = TaskID;
//			if(intNestCnt > 0){
//				OS_SYS_PTR->intNestCnt = intNestCnt;
//			}
			restore(im);
			WaitEvent(semlist_usb[*sem].event);
			ClearEvent(semlist_usb[*sem].event);
		}
		EndTime = GetOsTick();
		Elapsed = EndTime - StartTime;
		if( Elapsed == 0 )
		{
			Elapsed = 1;
		}
		return ( Elapsed ); // return time blocked
	}

}

/*-----------------------------------------------------------------------------------*/
// Signals a semaphore
void usb_sem_signal(semaphore *sem)
{

	irqmask im;
	im = disable();

//	printf("infor: Before GetResource\r\n");
//	GetResource(RES_SCHEDULER);
//	printf("infor: After GetResource\r\n");
	if(semlist_usb[*sem].val > 0){
		semlist_usb[*sem].val = 0;
		for(int i=0;i<semlist_usb[*sem].taskIndex;i++){
			SetEvent(semlist_usb[*sem].task[i] , semlist_usb[*sem].event);
		}
        semlist_usb[*sem].taskIndex = 0;
	}
//	printf("infor: Before ReleaseResource\r\n");
//	ReleaseResource(RES_SCHEDULER);
//	printf("infor: After ReleaseResource\r\n");
	restore(im);
}

void usb_sem_free(semaphore *sem)
{
//	sys_prot_t val = sys_arch_protect();
	irqmask im;
    im = disable();
	if(semlist_usb[*sem].used == TRUE)
	{
		usb_sem_signal(sem); // Just to be sure its released
		semlist_usb[*sem].used = FALSE;
		semlist_usb[*sem].taskIndex = 0;
//		sys_arch_unprotect(val);
		restore(im);
		return;
    }
//    ErrorHook(E_OS_VALUE);
//	sys_arch_unprotect(val);
    restore(im);
	return;
}


void usb_sem_init(void){

	uint8 i;

	//initialize semaphore pool
	for (i = 0; i < USB_SEM_MAX; i++) {
		semlist_usb[i].used = FALSE;
		semlist_usb[i].val = 0;
		semlist_usb[i].taskIndex = 0;
	}

	i = 0;
	semlist_usb[i++].event = EVENT_MASK_UsbEvent1;
	semlist_usb[i++].event = EVENT_MASK_UsbEvent2;
	semlist_usb[i++].event = EVENT_MASK_UsbEvent3;
	semlist_usb[i++].event = EVENT_MASK_UsbEvent4;
	semlist_usb[i++].event = EVENT_MASK_UsbEvent5;
	semlist_usb[i++].event = EVENT_MASK_UsbEvent6;
	semlist_usb[i++].event = EVENT_MASK_UsbEvent7;
	semlist_usb[i++].event = EVENT_MASK_UsbEvent8;
	semlist_usb[i++].event = EVENT_MASK_UsbEvent9;
//	pi_printf("infor: usb sem init.\r\n");

}
