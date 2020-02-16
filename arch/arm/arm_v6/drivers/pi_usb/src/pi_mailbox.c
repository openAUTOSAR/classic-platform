/*
 * pi_mailbox.c
 *
 *  Created on: 2013-11-18
 *      Author: Zhang Shuzhou
 */

#include "Cpu.h"
#include "usb_stddef.h"
#include "pi_mailbox.h"
#include "bcm2835.h"
#include "pi_memory.h"
#include "Uart.h"
#include "semaphore.h"
//#include "Platform_Types.h"
int MAIL_INIT = 0;

int hcd_xfer_mailbox_flag = FALSE;
int hcd_xfer_req = 0;
struct mbox mboxtab[NMAILBOX];
semaphore mboxtabsem;

/**
 * @ingroup mailbox
 *
 * Initialize mailbox structures.
 * @return OK if all mailboxes are initialized successfully
 */
syscall mailboxInit(void)
{
//    int i = 0;
//
//    mboxtabsem = semcreate(1);
//
//    /* set all mailbox states to MAILBOX_FREE */
//    for (i = 0; i < NMAILBOX; i++)
//    {
//        mboxtab[i].state = MAILBOX_FREE;
//    }

    return OK;
}


/**
 * @ingroup mailbox
 *
 * Send a mailmsg to a mailbox.
 * @param box the number of the mailbox to send to
 * @param mailmsg the mailmsg to send
 * @return OK if the mailmsg was enqueued, otherwise SYSERR
 */
syscall mailboxSend(mailbox box, int mailmsg)
{
	char* s = NULL;
	Irq_Disable();
	hcd_xfer_mailbox_flag = TRUE;
	hcd_xfer_req = mailmsg;
	Irq_Enable();
	/*s = "set\r\n";
	mini_uart_sendstr(s);*/

    return OK;
}


/**
 * @ingroup mailbox
 *
 * Receive a mailmsg from a mailbox.
 * @param box the number of the mailbox to receive from
 * @return the mailmsg that was dequeued, otherwise SYSERR
 */
syscall mailboxReceive(mailbox box)
{
//    struct mbox *mbxptr;
//    int mailmsg;
//    //irqmask im;
//
//
//    if (box >= NMAILBOX)
//    {
//        return SYSERR;
//    }
//
//    mbxptr = &mboxtab[box];
//    if (MAILBOX_ALLOC != mbxptr->state)
//    {
//        return SYSERR;
//    }
//
//    /* wait until there is a mailmsg in the mailmsg queue */
//    wait(mbxptr->receiver);
//
//    //im = disable();
//    Irq_Disable();
//    /* recieve the first mailmsg in the mailmsg queue */
//    mailmsg = mbxptr->msgs[mbxptr->start];
//
//    mbxptr->start = (mbxptr->start + 1) % mbxptr->max;
//    mbxptr->count--;
//
//    //restore(im);
//    Irq_Enable();
//    /* signal that there is another empty space in the mailmsg queue */
//    pi_signal(mbxptr->sender);
//
//    return mailmsg;
}


/**
 * @ingroup mailbox
 *
 * Allocate a mailbox that allows count outstanding messages.
 * @param count maximum number of messages allowed for the mailbox
 * @return the number of the mailbox, SYSERR if none are available
 */
syscall mailboxAlloc(uint count)
{
//    static int nextmbx = 0;
//    struct mbox *mbxptr;
//    ushort i = 0;
//
//    if(MAIL_INIT == 0){
//		mailboxInit();
//		MAIL_INIT = 1;
//    }
//
//    /* wait until other threads are done editing the mailbox table */
//    wait(mboxtabsem);
//
//    /* run through all mailboxes until we find a free one */
//    for (i = 0; i < NMAILBOX; i++)
//    {
//        nextmbx = (nextmbx + 1) % NMAILBOX;
//        /* when we find a free mailbox set that one up and return it */
//        if (MAILBOX_FREE == mboxtab[nextmbx].state)
//        {
//            mbxptr = &mboxtab[nextmbx];
//
//            /* get memory space for the message queue */
//            mbxptr->msgs = memget(sizeof(int) * count);
//
//            /* check if memory was allocated correctly */
//            if (SYSERR == (int)mbxptr->msgs)
//            {
//                /* signal and return SYSERR */
//                pi_signal(mboxtabsem);
//                return SYSERR;
//            }
//
//            /* initialize mailbox details and semaphores */
//            mbxptr->max = count;
//
//            mbxptr->sender = semcreate(count);
//            mbxptr->receiver = semcreate(0);
//            if ((SYSERR ==
//                 (int)mbxptr->sender)
//                || (SYSERR == (int)mbxptr->receiver))
//            {
//                memfree(mbxptr->msgs, sizeof(int) * (mbxptr->max));
//                semfree(mbxptr->sender);
//                semfree(mbxptr->receiver);
//                return SYSERR;
//            }
//            mbxptr->count = 0;
//            mbxptr->start = 0;
//
//            /* mark this mailbox as being used */
//            mbxptr->state = MAILBOX_ALLOC;
//
//            /* signal this thread is done editing the mbox tab */
//            pi_signal(mboxtabsem);
//
//            /* return the this entry in the mboxtab */
//            return nextmbx;
//        }
//    }
//
//    /* signal and return SYSERR because we didn't find a free spot */
//    pi_signal(mboxtabsem);
//    return SYSERR;
	return OK;
}

/**
 * @ingroup mailbox
 *
 * Free a mailbox.
 * @param box the number of the mailbox to delete
 * @return OK if the mailbox was deleted successfully, otherwise SYSERR
 */
syscall mailboxFree(mailbox box)
{
//    struct mbox *mbxptr;
//
//    if (box >= NMAILBOX)
//    {
//        return SYSERR;
//    }
//
//    mbxptr = &mboxtab[box];
//    if (MAILBOX_ALLOC != mbxptr->state)
//    {
//        return SYSERR;
//    }
//
//    /* wait until other threads are done editing the mailbox table */
//    wait(mboxtabsem);
//
//    /* free semaphores related to this mailbox */
//    if ((SYSERR == (int)semfree(mbxptr->sender))
//        || (SYSERR == (int)semfree(mbxptr->receiver)))
//    {
//        /* signal and return SYSERR */
//        pi_signal(mboxtabsem);
//        return SYSERR;
//    }
//
//    /* free memory that was used for the message queue */
//    if (SYSERR == memfree(mbxptr->msgs, sizeof(int) * (mbxptr->max)))
//    {
//        /* signal and return SYSERR */
//        pi_signal(mboxtabsem);
//        return SYSERR;
//    }
//
//    /* free this mailbox for use */
//    mbxptr->state = MAILBOX_FREE;
//
//    /* signal that this thread is done editing the mailbox table */
//    pi_signal(mboxtabsem);

    return OK;
}
