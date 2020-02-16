/*
 * usb_memory.h
 *
 *  Created on: 31 jan 2014
 *      Author: zsz
 */

#ifndef USB_MEMORY_H_
#define USB_MEMORY_H_
#include "usb_util.h"
#include "usb_semaphore.h"
/* roundmb - round address up to size of memblock  */
#define roundmb(x)  (void *)( (7 + (ulong)(x)) & ~0x07 )
/* truncmb - truncate address down to size of memblock */
#define truncmb(x)  (void *)( ((ulong)(x)) & ~0x07 )

/**
 * @ingroup memory_mgmt
 *
 * Frees memory allocated with stkget().
 *
 * @param p
 *      Pointer to the topmost (highest address) word of the allocated stack (as
 *      returned by stkget()).
 * @param len
 *      Size of the allocated stack, in bytes.  (Same value passed to stkget().)
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

extern struct memblock memlist;     /**< head of free memory list           */

/* Other memory data */

extern void *_end;              /**< linker provides end of image       */
extern void *memheap;           /**< bottom of heap                     */

/* Memory function prototypes */
void usb_heap_init(void);
void *memget(uint);
syscall memfree(void *, uint);
void *stkget(uint);
void bzero(void *s, size_t n);



/* Buffer pool state definitions */
#define BFPFREE  1
#define BFPUSED  2

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

void buffer_init(void);
/* function prototypes */
void *bufget(int);
syscall buffree(void *);
int bfpalloc(uint, uint);
syscall bfpfree(int);

#define NPOOL    8
#define POOL_MAX_BUFSIZE 2048   /* max size of a buffer in a pool   */
#define POOL_MIN_BUFSIZE 8      /* min size of a buffer in a pool   */
#define POOL_MAX_NBUFS   8192   /* max number of buffers in a pool  */

#ifndef NPOOL
#  define NPOOL 0
#endif

/* Fill in dummy definitions if no pools are defined  */
#if NPOOL == 0
#  ifndef POOL_MAX_BUFSIZE
#    define POOL_MAX_BUFSIZE 0
#  endif
#  ifndef POOL_MIN_BUFSIZE
#    define POOL_MIN_BUFSIZE 0
#  endif
#  ifndef POOL_MAX_NBUFS
#    define POOL_MAX_NBUFS 0
#  endif
#endif



#endif /* USB_MEMORY_H_ */
