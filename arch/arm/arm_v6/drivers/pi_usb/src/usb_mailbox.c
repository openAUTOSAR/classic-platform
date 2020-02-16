/*
 * usb_mailbox.c
 *
 *  Created on: 13 feb 2014
 *      Author: zsz
 */

#include "usb_mailbox.h"
#include "bcm2835.h"
#include "Uart.h"
#include "usb_semaphore.h"
#include "usb_memory.h"
struct mbox mboxtab[NMAILBOX];
semaphore mboxtabsem;

boolean MASSAGE_ARRIVE_FLAG = false;
/**
 * @ingroup mailbox
 *
 * Initialize mailbox structures.
 *
 * @return
 *      ::OK if all mailboxes were initialized successfully, otherwise ::SYSERR.
 */
syscall mailboxInit(void)
{
    uint i;

    /* set all mailbox states to MAILBOX_FREE */
    for (i = 0; i < NMAILBOX; i++)
    {
        mboxtab[i].state = MAILBOX_FREE;
    }

    mboxtabsem = semcreate(1);
    if (SYSERR == mboxtabsem)
    {
    	pi_printf("error: fail to mailbox Init\r\n");
        return SYSERR;
    }

    return OK;
}

/* @ingroup mailbox
 *
 * Allocate a mailbox that allows up to the specified number of outstanding
 * messages.
 *
 * @param count
 *      Maximum number of messages allowed for the mailbox.
 *
 * @return
 *      The index of the newly allocated mailbox, or ::SYSERR if all mailboxes
 *      are already in use or other resources could not be allocated.
 */
syscall mailboxAlloc(uint count)
{
    static uint nextmbx = 0;
    uint i;
    struct mbox *mbxptr;
    int retval = SYSERR;

    /* wait until other threads are done editing the mailbox table */
    wait(mboxtabsem);

    /* run through all mailboxes until we find a free one */
    for (i = 0; i < NMAILBOX; i++)
    {
        nextmbx = (nextmbx + 1) % NMAILBOX;
        mbxptr = &mboxtab[nextmbx];

        /* when we find a free mailbox set that one up and return it */
        if (MAILBOX_FREE == mbxptr->state)
        {
            /* get memory space for the message queue */
            mbxptr->msgs = memget(sizeof(int) * count);

            /* check if memory was allocated correctly */
            if (SYSERR == (int)mbxptr->msgs)
            {
            	pi_printf("error: fail to mailbox memory\r\n");
                break;
            }

            /* initialize mailbox details and semaphores */
            mbxptr->count = 0;
            mbxptr->start = 0;
            mbxptr->max = count;
            usb_sem_new(&mbxptr->sender, 1024);
            usb_sem_new(&mbxptr->receiver, 0);
            if ((SYSERR == (int)mbxptr->sender) ||
                (SYSERR == (int)mbxptr->receiver))
            {
                memfree(mbxptr->msgs, sizeof(int) * (mbxptr->max));
                semfree(mbxptr->sender);
                semfree(mbxptr->receiver);
                pi_printf("error: fail to allocate mailbox\r\n");
                break;
            }

            /* mark this mailbox as being used */
            mbxptr->state = MAILBOX_ALLOC;

            /* return value is index of the allocated mailbox */
            retval = nextmbx;
            break;
        }
    }

    /* signal this thread is done editing the mbox tab */
    semsignal(mboxtabsem);

    /* return either SYSERR or the index of the allocated mailbox */
    return retval;
}

/**
 * @ingroup mailbox
 *
 * Send a message to the specified mailbox.
 *
 * @param box
 *      The index of the mailbox to send the message to.
 *
 * @param mailmsg
 *      The message to send.
 *
 * @return ::OK if the message was successfully enqueued, otherwise ::SYSERR.
 *         ::SYSERR is returned only if @p box did not specify a valid allocated
 *         mailbox or if the mailbox was freed while waiting for room in the
 *         queue.
 */
syscall mailboxSend(mailbox box, int mailmsg)
{
    struct mbox *mbxptr;
    irqmask im;
    int retval;

    if (!(0 <= box && box < NMAILBOX))
    {
    	pi_printf("error: mailbox invalid");
        return SYSERR;
    }
    mbxptr = &mboxtab[box];
    im = disable();
    retval = SYSERR;
    if (MAILBOX_ALLOC == mbxptr->state)
    {
        /* wait until there is room in the mailmsg queue */
//        usb_sem_wait(&mbxptr->sender, 0);
        /* only continue if the mailbox hasn't been freed  */
        if (MAILBOX_ALLOC == mbxptr->state)
        {
            /* write mailmsg to this mailbox's mailmsg queue */
            mbxptr->msgs[((mbxptr->start + mbxptr->count) % mbxptr->max)] = mailmsg;
            mbxptr->count++;
            /* signal that there is another mailmsg in the mailmsg queue */
//            usb_sem_signal(&mbxptr->receiver);
            MASSAGE_ARRIVE_FLAG = true;
            retval = OK;
        }
    }

    restore(im);
    return retval;
}


/**
 * @ingroup mailbox
 *
 * Receive a message from the specified mailbox.
 *
 * @param box
 *      The index of the mailbox to receive the message from.
 *
 * @return
 *      On success, returns the message that was dequeued; on failure (@p box
 *      did not specify an allocated mailbox, or the mailbox was freed while
 *      waiting for a message) returns ::SYSERR.  Note that it may be impossible
 *      to disambiguate ::SYSERR from a successful return value.
 */
syscall mailboxReceive(mailbox box)
{
    struct mbox *mbxptr;
    irqmask im;
    int retval;

    if (!(0 <= box && box < NMAILBOX))
    {
    	pi_printf("error: fail to mailbox receiver\r\n");
        return SYSERR;
    }
    mbxptr = &mboxtab[box];
    im = disable();
    retval = SYSERR;
    if (MAILBOX_ALLOC == mbxptr->state)
    {
		/* wait until there is a mailmsg in the mailmsg queue */
//    	printf("infor: wait mailbox receiver\r\n");
//		usb_sem_wait(&mbxptr->receiver, 0);
		/* only continue if the mailbox hasn't been freed  */
		if (MAILBOX_ALLOC == mbxptr->state) {
			/* recieve the first mailmsg in the mailmsg queue */
			retval = mbxptr->msgs[mbxptr->start];
			mbxptr->start = (mbxptr->start + 1) % mbxptr->max;
			mbxptr->count --;
			/* signal that there is another empty space in the mailmsg queue */
//			printf("infor: signal mailbox sender\r\n");
//			usb_sem_signal(&mbxptr->sender);
			MASSAGE_ARRIVE_FLAG = false;
		}
    }
    restore(im);
    return retval;
}

/**
 * @ingroup mailbox
 *
 * Free the specified mailbox.
 *
 * @param box
 *      The index of the mailbox to free.
 *
 * @return
 *      ::OK if the mailbox was successfully freed, or ::SYSERR if @p box did
 *      not specify a valid allocated mailbox.
 */
syscall mailboxFree(mailbox box)
{
    struct mbox *mbxptr;
    int retval;

    if (!(0 <= box && box < NMAILBOX))
    {
    	pi_printf("error: fail to mailbox free\r\n");
        return SYSERR;
    }

    mbxptr = &mboxtab[box];

    /* wait until other threads are done editing the mailbox table */
    wait(mboxtabsem);

    if (MAILBOX_ALLOC == mbxptr->state)
    {
        /* mark mailbox as no longer allocated  */
        mbxptr->state = MAILBOX_FREE;

        /* free semaphores related to this mailbox */
        semfree(mbxptr->sender);
        semfree(mbxptr->receiver);

        /* free memory that was used for the message queue */
        memfree(mbxptr->msgs, sizeof(int) * (mbxptr->max));

        retval = OK;
    }
    else
    {
        /* mailbox was not allocated  */
    	pi_printf("error: mailbox was not allocated\r\n");
        retval = SYSERR;
    }

    /* signal that this thread is done editing the mailbox table */
    semsignal(mboxtabsem);

    return retval;
}

/**
 * @ingroup mailbox
 *
 * Retrieve the number of outstanding messages in the specified mailbox.
 *
 * @param box
 *      The index of the mailbox for which to retrieve the number of outstanding
 *      messages.
 *
 * @return
 *      The number of messages in the mailbox, or ::SYSERR if @p box did not
 *      specify a valid allocated mailbox.
 */
syscall mailboxCount(mailbox box)
{
    const struct mbox *mbxptr;
    irqmask im;
    int retval;

    if (!(0 <= box && box < NMAILBOX))
    {
    	pi_printf("error: fail to mailbox count 1\r\n");
        return SYSERR;
    }

    mbxptr = &mboxtab[box];
    im = disable();
    if (MAILBOX_ALLOC == mbxptr->state)
    {
        retval = mbxptr->count;
    }
    else
    {
    	pi_printf("error: fail to mailbox count 2\r\n");
        retval = SYSERR;
    }
    restore(im);
    return retval;
}
