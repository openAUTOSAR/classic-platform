/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"

#include "Os.h"
#include "Std_Types.h"
#include "arch/sys_arch.h"
#include "Mcu.h"
#include <string.h>

static void (*tcpip_thread)(void *arg) = NULL;

#if !NO_SYS


struct lwip_port_statsS {
    uint32 sem_new_err;
    uint32 mbox_trypost_err;
    uint32 mbox_tryfetch_empty;
    uint32 sem_new;
    uint32 sem_free;
    uint32 sem_wait;
    uint32 sem_wait_tmo;
    uint32 sem_signal;
    uint32 mbox_new;
    uint32 mbox_new_err;
    uint32 mbox_post;
    uint32 mbox_trypost;
    uint32 mbox_tryfetch;
    uint32 mbox_fetch;
    uint32 mbox_fetch_tmo;
    uint32 malloc;
    uint32 free;
};


struct lwip_port_statsS lwip_port_stats;


void __malloc_lock ( struct _reent *_r )
{
    Irq_Disable();
    lwip_port_stats.malloc++;
    return;
}

void __malloc_unlock ( struct _reent *_r )
{
    lwip_port_stats.free++;
    Irq_Enable();
    return;
}

#include "lwip/tcpip.h"

/* This is the number of threads that can be started with sys_thread_new()
 * Cannot be modified at the moment. No need to support slip/ppp */
#define SYS_THREAD_MAX 1


/*
 This optional function does a "fast" critical region protection and returns
 the previous protection level. This function is only called during very short
 critical regions. An embedded system which supports ISR-based drivers might
 want to implement this function by disabling interrupts. Task-based systems
 might want to implement this by using a mutex or disabling tasking. This
 function should support recursive calls from the same task or interrupt. In
 other words, sys_arch_protect() could be called while already protected. In
 that case the return value indicates that it is already protected.

 sys_arch_protect() is only required if your port is supporting an operating
 system.
 */
sys_prot_t sys_arch_protect(void)
{
    uint32_t val;
    Irq_Save(val);
    return val;
}

/*
 This optional function does a "fast" set of critical region protection to the
 value specified by pval. See the documentation for sys_arch_protect() for
 more information. This function is only required if your port is supporting
 an operating system.
 */
void sys_arch_unprotect(sys_prot_t pval)
{
    Irq_Restore(pval);
}

u32_t sys_now(void)
{
	return (u32_t)GetOsTick();
}
/*-----------------------------------------------------------------------------------*/
//  Creates an empty mailbox.
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    err_t rv;
    lwip_port_stats.mbox_new++;

    if(size==0){size=20;}
    *mbox = malloc( sizeof(OsMBoxType) + sizeof(OsMBoxMsg) * size );

    if( mbox == NULL ) {
        rv = ERR_MEM;
        lwip_port_stats.mbox_new_err++;
    } else {
        memset(*mbox, 0, sizeof(OsMBoxType));
        MBoxInit(   *mbox,
                    (OsMBoxMsg *)(((uint8 *)*mbox) + sizeof(OsMBoxType)),
                    size);
        rv = ERR_OK;
    }

    return rv;
}

/*-----------------------------------------------------------------------------------*/
/*
 Deallocates a mailbox. If there are messages still present in the
 mailbox when the mailbox is deallocated, it is an indication of a
 programming error in lwIP and the developer should be notified.
 */
void sys_mbox_free(sys_mbox_t *mbox)
{
    lwip_port_stats.sem_free++;
    free(*mbox);
    *mbox = SYS_MBOX_NULL;
}

/*-----------------------------------------------------------------------------------*/
/* void sys_mbox_post(sys_mbox_t mbox, void *msg)

 Posts the "msg" to the mailbox. This function have to block until
 the "msg" is really posted.
 */
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    StatusType rv;
    lwip_port_stats.mbox_post++;
    rv = MBoxPost(*mbox,(OsMBoxMsg)msg);
    (void)rv;
}

/* err_t sys_mbox_trypost(sys_mbox_t mbox, void *msg)

 Try to post the "msg" to the mailbox. Returns ERR_MEM if this one
 is full, else, ERR_OK if the "msg" is posted.
 */
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    err_t err = ERR_OK;
    StatusType rv;
    lwip_port_stats.mbox_trypost++;
    rv = MBoxPost(*mbox,(OsMBoxMsg)msg);
    if( rv == E_OS_FULL ) {
        lwip_port_stats.mbox_trypost_err++;
        err = ERR_MEM;
    }

    return err;
}

/* This is similar to sys_arch_mbox_fetch, however if a message is not
 present in the mailbox, it immediately returns with the code
 SYS_MBOX_EMPTY. On success 0 is returned.

 To allow for efficient implementations, this can be defined as a
 function-like macro in sys_arch.h instead of a normal function. For
 example, a naive implementation could be:
 #define sys_arch_mbox_tryfetch(mbox,msg) \
      sys_arch_mbox_fetch(mbox,msg,1)
 although this would introduce unnecessary delays. */
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    StatusType mRv;
    u32_t rv;

    lwip_port_stats.mbox_tryfetch++;
    mRv = MBoxFetchTmo(*mbox, (OsMBoxMsg *)msg, TMO_MIN );
    if( mRv == E_OS_TIMEOUT  ) {
        lwip_port_stats.mbox_tryfetch_empty++;
        rv = SYS_MBOX_EMPTY;
    } else {
        rv = 0;
    }

    return rv;
}

/*-----------------------------------------------------------------------------------*/
/*
 Blocks the thread until a message arrives in the mailbox, but does
 not block the thread longer than "timeout" milliseconds (similar to
 the sys_arch_sem_wait() function). The "msg" argument is a result
 parameter that is set by the function (i.e., by doing "*msg =
 ptr"). The "msg" parameter maybe NULL to indicate that the message
 should be dropped.

 The return values are the same as for the sys_arch_sem_wait() function:
 Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
 timeout.

 Note that a function with a similar name, sys_mbox_fetch(), is
 implemented by lwIP.
 */


u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout_ms)
{
    StatusType rv;
    u32_t diff;
    TickType start;
    imask_t flags;
    uint32 tmo;

    /* TODO: Not really sure about the locking here.... only needed to get the timeout correct */
    Irq_Save(flags);
    lwip_port_stats.mbox_fetch++;

    start = GetOsTick();
    /* TODO: MBox timeout is in ticks... */

    tmo = (timeout_ms == 0) ? TMO_INFINITE : timeout_ms;

    rv = MBoxFetchTmo(*mbox,(OsMBoxMsg *)msg, tmo);
    if( rv == E_OS_TIMEOUT ) {
        return SYS_ARCH_TIMEOUT;
        lwip_port_stats.mbox_fetch_tmo++;
    }
    diff = GetOsTick() - start;
    Irq_Restore(flags);
    return diff;
}

/*-----------------------------------------------------------------------------------*/
//  Creates and returns a new semaphore. The "count" argument specifies
//  the initial state of the semaphore.
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    lwip_port_stats.sem_new++;
    *sem = malloc( sizeof(OsSemType) );

    if( *sem == NULL ) {
        lwip_port_stats.sem_new_err++;
        return ERR_MEM;
    }

    memset(*sem, 0, sizeof(OsSemType));
    BSemInit(*sem,count);
    return ERR_OK;
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

 Notice that lwIP implements a function with a similar name,
 sys_sem_wait(), that uses the sys_arch_sem_wait() function.
 */

/**
 * Wait for a semaphore for the specified timeout.
 * @param sem           the semaphore to wait for
 * @param timeout_ms    timeout in milliseconds to wait (0 = wait forever)
 *
 * @return
 */
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout_ms)
{
    StatusType rv;
    u32_t diff;
    TickType start;
    imask_t flags;
    uint32 tmo;

    lwip_port_stats.sem_wait++;

    /* TODO: Not really sure about the locking here.... only needed to get the timeout correct */
    Irq_Save(flags);
    start = GetOsTick();
    tmo = (timeout_ms == 0) ? TMO_INFINITE : timeout_ms;

    /* TODO: MBox timeout is in ticks... */
    rv = BSemWaitTmo(*sem, tmo);
    if( rv == E_OS_TIMEOUT ) {
        lwip_port_stats.sem_wait_tmo++;
        return SYS_ARCH_TIMEOUT;
    }
    diff = GetOsTick() - start;
    Irq_Restore(flags);
    return diff;
}

/*-----------------------------------------------------------------------------------*/
// Signals a semaphore
void sys_sem_signal(sys_sem_t *sem)
{
    lwip_port_stats.sem_signal++;
    BSemSignal(*sem);
}

/*-----------------------------------------------------------------------------------*/
// Deallocates a semaphore
void sys_sem_free(sys_sem_t *sem)
{
    lwip_port_stats.sem_free++;
    free(*sem);
    *sem = SYS_SEM_NULL;
}

/*-----------------------------------------------------------------------------------*/
// Initialize sys arch
void sys_init(void)
{
}

// Preinitialize sys arch
void pre_sys_init(void)
{
}

// TBD memory leak with invalid mbox?
int sys_mbox_valid(sys_mbox_t *mbox)
{
    return (*mbox == SYS_MBOX_NULL) ? 0 : 1;
}

void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
    *mbox = SYS_MBOX_NULL;
}

int sys_sem_valid(sys_sem_t *sem)
{
    return (*sem != SYS_SEM_NULL);
}

void sys_sem_set_invalid(sys_sem_t *sem)
{
    *sem = SYS_SEM_NULL;
}

/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
// TBD 
/*-----------------------------------------------------------------------------------*/
/*
 Starts a new thread with priority "prio" that will begin its execution in the
 function "thread()". The "arg" argument will be passed as an argument to the
 thread() function. The id of the new thread is returned. Both the id and
 the priority are system dependent.
 */
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
    sys_thread_t CreatedTask = TASK_ID_tcpip_task;
    static uint8 iCall = 0;

    if( iCall == 0 )
    {
        tcpip_thread = thread;
    }
    else
    {
        // We only support creating tcpip thread at the moment
        ErrorHook(E_OS_LIMIT);
        return 0;
    }

    // ActiveTask(TASK_ID_tcpip_task);

    /* Why not ActivateTask here ?? */
    SetEvent(TASK_ID_tcpip_task,EVENT_MASK_START_TCPIP); // Start tcpip stack
    return CreatedTask;
}

#endif /* !NO_SYS */

// TCPIP task shell
void tcpip_task(void) {
    for (;;) {
        WaitEvent(EVENT_MASK_START_TCPIP);
        ClearEvent(EVENT_MASK_START_TCPIP);
        if (tcpip_thread != NULL) {
            tcpip_thread(NULL); // Will never return
        }
    }
}





