/*
 * sempphore.c
 *
 *  Created on: 2013-10-31
 *      Author: Zhang Shuzhou
 */

#include "semaphore.h"
#include "usb_stddef.h"
#include "bcm2835.h"
#include "Uart.h"
#include "usb_hcdi.h"
#include "usb_core_driver.h"
#include "usb_hub_driver.h"

int SEMA_INIT = 0;

static semaphore semalloc(void);
static semaphore seminit(void);
char* s = NULL;


syscall wait(semaphore sem)
{
//    char* s;
//
//	while(1){
//		if(msg_done_flag == TRUE){
//			msg_done_flag = FALSE;
//			break;
//		}
//
//		if(chfree_sema_flag == TRUE){
//			chfree_sema_flag = FALSE;
//			break;
//		}
//
//		s = "god help me\r\n";
//	    mini_uart_sendstr(s);
//
//	}

	return OK;
}

syscall pi_signal(semaphore sem)
{
//	register struct sement *semptr;
//    char* s = NULL;
//
//	__disable_irq();
//	if (isbadsem(sem)) {
//		__enable_irq();
//		return SYSERR;
//	}
//	semptr = &semtab[sem];
//	if ((semptr->count++) < 0) {
//
//		while(1){
//		  s = "pi_signal\r\n";
//	      mini_uart_sendstr(s);
//	      __enable_irq();
//		}
//
//	}
//	__enable_irq();
	return OK;

}

syscall signaln(semaphore sem, int count)
{

    return OK;
}

semaphore semcreate(int count)
{
	semaphore sem;
//    if (SEMA_INIT == 0){
//    	seminit();
//    	SEMA_INIT = 1;
//    }
//
//	if (count < 0) /* Don't allow starting with negative count.  */
//	{
//		return SYSERR;
//	}
//
//	__disable_irq();
//	sem = semalloc(); /* Allocate semaphore.  */
//	if (SYSERR != sem) /* If semaphore was allocated, set count.  */
//	{
//		semtab[sem].count = count;
//	}
//	/* Restore interrupts and return either the semaphore or SYSERR.  */
//	__enable_irq();
	return OK;
}

syscall semfree(semaphore sem)
{
//	register struct sement *semptr;
//
//	__disable_irq();
//	if (isbadsem(sem)) {
//		__enable_irq();
//		return SYSERR;
//	}
//	semptr = &semtab[sem];
//	semptr->count = 0;
//	semptr->state = SFREE;
//	__enable_irq();
	return OK;
}

syscall semcount(semaphore sem)
{

    return OK;
}


/**
 * Allocate an unused semaphore and return its ID.
 * Scan the global semaphore table for a free entry, mark the entry
 * used, and return the new semaphore
 * @return available semaphore ID on success, SYSERR on failure
 */
static semaphore semalloc(void)
{
//    int i;
//    static int nextsem = 0;
//
//    /* check all NSEM slots, starting at 1 past the last slot searched.  */
//    for (i = 0; i < NSEM; i++)
//    {
//        nextsem = (nextsem + 1) % NSEM;
//        if (SFREE == semtab[nextsem].state)
//        {
//            semtab[nextsem].state = SUSED;
//            return nextsem;
//        }
//    }
//    return SYSERR;
}


static semaphore seminit(void)
{
//	int i;
//	struct sement *semptr;      /* semaphore entry pointer       */
//    /* Initialize semaphores */
//    for (i = 0; i < NSEM; i++)
//    {
//        semptr = &semtab[i];
//        semptr->state = SFREE;
//        semptr->count = 0;
//    }
}


//syscall wait(semaphore sem)
//{
////	register struct sement *semptr;
////	struct usb_xfer_request *req;
////
////	__disable_irq();
////
////	/*s = "receive mailmsg:";
////	mini_uart_sendstr(s);
////	mini_uart_sendhex(hcd_xfer_mailbox_flag, 1);*/
////
////	if(hcd_xfer_mailbox_flag == TRUE){
////		hcd_xfer_mailbox_flag = FALSE;
////		/*s = "clear\r\n";
////	    mini_uart_sendstr(s);*/
////		req = (struct usb_xfer_request*) hcd_xfer_req;
////		dwc_schedule_xfer_requests(req);
////		hcd_xfer_req = NULL;
////    }
////
////	if(hub_status_change_flag == TRUE){
////		hub_status_change_flag = FALSE;
////		hub_thread();
////	}
////
////	if (isbadsem(sem)) {
////		__enable_irq();
////		return SYSERR;
////	}
////	semptr = &semtab[sem];
////	if (--(semptr->count) < 0) {
////		while (1) {
////			s = "wait sem\r\n";
////			mini_uart_sendstr(s);
////			__enable_irq();
////		}
////
////	}
////	//__enable_irq();
//	return OK;
//}
