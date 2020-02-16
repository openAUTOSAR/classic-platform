/*
 * semaphore.h
 *
 *  Created on: 2013-10-31
 *      Author: Zhang Shuzhou
 */

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include "usb_stddef.h"

/* Semaphore state definitions */
#define SFREE 0x01 /**< this semaphore is free */
#define SUSED 0x02 /**< this semaphore is used */
#define NSEM      100           /* number of semaphores             */
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
syscall pi_signal(semaphore);
syscall signaln(semaphore, int);
semaphore semcreate(int);
syscall semfree(semaphore);
syscall semcount(semaphore);


#endif /* SEMAPHORE_H_ */
