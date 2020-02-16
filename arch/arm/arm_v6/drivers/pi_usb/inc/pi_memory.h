/*
 * memory.h
 *
 *  Created on: 2013-10-31
 *      Author: Zhang Shuzhou
 */

#ifndef MEMORY_H_
#define MEMORY_H_

#include "Platform_Types.h"
#include "usb_stddef.h"
#include "bcm2835.h"
#include "semaphore.h"


/* roundmb - round address up to size of memblock  */
#define roundmb(x)  (void *)( (7 + (ulong)(x)) & ~0x07 )
/* truncmb - truncate address down to size of memblock */
#define truncmb(x)  (void *)( ((ulong)(x)) & ~0x07 )

/**
 * @ingroup memory_mgmt
 *
 * Frees memory allocated with stkget().
 */
#define stkfree(p, len) memfree((void *)((ulong)(p)         \
                                - (ulong)roundmb(len)       \
                                + (ulong)sizeof(ulong)),    \
                                (ulong)roundmb(len))


/**
 * Structure for a block of memory.
 */
struct memblock
{
    struct memblock *next;          /**< pointer to next memory block       */
    uint length;                    /**< size of memory block (with struct) */
};

extern struct memblock* memlist;     /**< head of free memory list           */

/* Other memory data */

extern void *_end;              /**< linker provides end of image       */
extern void *memheap;           /**< bottom of heap                     */
extern struct memblock USB_MEM[];
/* Memory function prototypes */
void *memget(uint);
syscall memfree(void *, uint);
void *stkget(uint);



/* Buffer pool state definitions */
#define BFPFREE  1
#define BFPUSED  2
#define NPOOL    8
#define POOL_MAX_BUFSIZE 2048   /* max size of a buffer in a pool   */
#define POOL_MIN_BUFSIZE 8      /* min size of a buffer in a pool   */
#define POOL_MAX_NBUFS   8192   /* max number of buffers in a pool  */

/**
 * Individual pool buffer
 */
struct poolbuf
{
    struct poolbuf *next;
    int poolid;
};

/**
 * Buffer pool table entry
 */
struct bfpentry
{
    uchar state;
    uint bufsize;
    uint nbuf;
    void *head;
    struct poolbuf *next;
    semaphore freebuf;
};

/**
 * isbadpool - check validity of reqested buffer pool id and state
 * @param p id number to test
 */
#define isbadpool(p) ((p >= NPOOL)||(p < 0)||(BFPFREE == bfptab[p].state))

/** roundword - round byte sized request to word size
 *  @param b size in bytes
 */
#define roundword(b) ((3 + b) & ~0x03)

extern struct bfpentry bfptab[];

/* function prototypes */
void *bufget(int);
syscall buffree(void *);
int bfpalloc(uint, uint);
syscall bfpfree(int);


void *pi_memcpy(void *dest, const void *src, unsigned long n);

int pi_memcmp(const void *s1, const void *s2, unsigned long n);


#endif /* MEMORY_H_ */
