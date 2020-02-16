/*
 * semaphore.h
 *
 *  Created on: 2013-10-31
 *      Author: Zhang Shuzhou
 */

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include "usb_util.h"
#include "Os.h"
#include "internal.h"
#include "task_i.h"
#include "sys.h"

/* Semaphore state definitions */
#define SFREE 		0x01 /**< this semaphore is free */
#define SUSED 		0x02 /**< this semaphore is used */
#define NSEM      	100           /* number of semaphores             */
/* type definition of "semaphore" */
typedef unsigned int semaphore;

/**
 * Semaphore table entry
 */
struct sement                   /* semaphore table entry      */
{
    char state;                 /**< the state SFREE or SUSED */
    int count;                  /**< count for this semaphore */
    //qid_typ queue;              /**< requires queue.h.        */
};




//extern struct sement semtab[];
struct sement semtab[NSEM];     /* Semaphore table                */

/* isbadsem - check validity of reqested semaphore id and state */
#define isbadsem(s) ((s >= NSEM) || (SFREE == semtab[s].state))

/* Semaphore function prototypes */
syscall wait(semaphore);
syscall semsignal(semaphore);
syscall signaln(semaphore, int);
semaphore semcreate(int);
syscall semfree(semaphore);
syscall semcount(semaphore);

/* This is the number of threads that can be started with sys_thread_new()
 * Cannot be modified at the moment. No need to support slip/ppp */

#define SEM_OK             0    /* No error, everything OK. */
#define SEM_NOT_OK        -1
#define SEM_TIMEOUT 	   0xffffffffUL




typedef uint sem_err;

void usb_sem_init(void);
void usb_sem_free(semaphore *sem);
void usb_sem_signal(semaphore *sem);
uint32 usb_sem_wait(semaphore *sem, uint32 timeout);
sem_err usb_sem_new(semaphore *sem, uint count);
uint8 UsbGetSem(semaphore sem);
#endif /* SEMAPHORE_H_ */
