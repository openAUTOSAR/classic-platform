/*
 * usb_memory.c
 *
 *  Created on: 31 jan 2014
 *      Author: zsz
 */
#include <string.h>
#include "usb_memory.h"
#include "bcm2835.h"
#include "Uart.h"
#include "usb_semaphore.h"


#define usb_heap_size  5242880*4    //10M

int usb_heap[usb_heap_size];
struct memblock memlist;        /* List of free memory blocks     */
struct bfpentry bfptab[NPOOL];  /* List of memory buffer pools    */

void *memheap;


/**
 * @ingroup libxc
 *
 * Zeroes a block of memory.
 *
 * @param s
 *      Pointer to the start of the block of memory to zero.
 * @param n
 *      Length of the block of memory, in bytes.
 */
void bzero(void *s, size_t n)
{
    memset(s, 0, n);
}


/*
 * this function is used for initialization the
 * memory of usb will be used, God bless Shuzhou!!
 *
 */
void usb_heap_init(void){

	struct memblock *pmblock;   /* memory block pointer          */
	memheap = roundmb(&usb_heap[0]);
//	pi_printf("infor: heap=");pi_print((uint32)memheap,1);
	memlist.next = pmblock = (struct memblock *) memheap;
	memlist.length = (uint) usb_heap_size;
	pmblock->next = NULL;
	pmblock->length = (uint) usb_heap_size;

}


/**
 * @ingroup memory_mgmt
 *
 * Allocate heap memory.
 *
 * @param nbytes
 *      Number of bytes requested.
 *
 * @return
 *      ::SYSERR if @p nbytes was 0 or there is no memory to satisfy the
 *      request; otherwise returns a pointer to the allocated memory region.
 *      The returned pointer is guaranteed to be 8-byte aligned.  Free the block
 *      with memfree() when done with it.
 */
void *memget(uint nbytes)
{
    register struct memblock *prev, *curr, *leftover;
    irqmask im;
    if (0 == nbytes)
    {
    	pi_printf("error: fail to memget1\r\n");
        return (void *)SYSERR;
    }

    /* round to multiple of memblock size   */
    nbytes = (ulong)roundmb(nbytes);

    im = disable();

    prev = &memlist;
    curr = memlist.next;
    while (curr != NULL)
    {
        if (curr->length == nbytes)
        {
            prev->next = curr->next;
            memlist.length -= nbytes;

            restore(im);
            return (void *)(curr);
        }
        else if (curr->length > nbytes)
        {
            /* split block into two */
            leftover = (struct memblock *)((ulong)curr + nbytes);
            prev->next = leftover;
            leftover->next = curr->next;
            leftover->length = curr->length - nbytes;
            memlist.length -= nbytes;
            restore(im);
//            printf("infor: memory address = %d\r\n", curr);
            return (void *)(curr);
        }
        prev = curr;
        curr = curr->next;
    }
    restore(im);
    pi_printf("error: fail to memget2\r\n");
    return (void *)SYSERR;
}


/**
 * @ingroup memory_mgmt
 *
 * Frees a block of heap-allocated memory.
 *
 * @param memptr
 *      Pointer to memory block allocated with memget().
 *
 * @param nbytes
 *      Length of memory block, in bytes.  (Same value passed to memget().)
 *
 * @return
 *      ::OK on success; ::SYSERR on failure.  This function can only fail
 *      because of memory corruption or specifying an invalid memory block.
 */
syscall memfree(void *memptr, uint nbytes)
{
    register struct memblock *block, *next, *prev;
    irqmask im;
    ulong top;

    /* make sure block is in heap */
    if ((0 == nbytes) || ((ulong)memptr < (ulong)memheap))
    {
    	pi_printf("error: fail to memfree 1\r\n");
        return SYSERR;
    }

    block = (struct memblock *)memptr;
    nbytes = (ulong)roundmb(nbytes);

    im = disable();

    prev = &memlist;
    next = memlist.next;
    while ((next != NULL) && (next < block))
    {
        prev = next;
        next = next->next;
    }

    /* find top of previous memblock */
    if (prev == &memlist)
    {
        top = NULL;
    }
    else
    {
        top = (ulong)prev + prev->length;
    }

    /* make sure block is not overlapping on prev or next blocks */
    if ((top > (ulong)block)
        || ((next != NULL) && ((ulong)block + nbytes) > (ulong)next))
    {
    	pi_printf("error: fail to memfree 2\r\n");
        restore(im);
        return SYSERR;
    }

    memlist.length += nbytes;

    /* coalesce with previous block if adjacent */
    if (top == (ulong)block)
    {
        prev->length += nbytes;
        block = prev;
    }
    else
    {
        block->next = next;
        block->length = nbytes;
        prev->next = block;
    }

    /* coalesce with next block if adjacent */
    if (((ulong)block + block->length) == (ulong)next)
    {
        block->length += next->length;
        block->next = next->next;
    }
    restore(im);
    return OK;
}



void buffer_init(void){

	for (int i = 0; i < NPOOL; i++) {
		bfptab[i].state = BFPFREE;
	}

}
/**
 * @ingroup memory_mgmt
 *
 * Allocate a buffer from a buffer pool.  If no buffers are currently available,
 * this function wait until one is, usually rescheduling the thread.  The
 * returned buffer must be freed with buffree() when the calling code is
 * finished with it.
 *
 * @param poolid
 *      Identifier of the buffer pool, as returned by bfpalloc().
 *
 * @return
 *      If @p poolid does not specify a valid buffer pool, returns ::SYSERR;
 *      otherwise returns a pointer to the resulting buffer.
 */
void *bufget(int poolid)
{
    struct bfpentry *bfpptr;
    struct poolbuf *bufptr;
    irqmask im;

    if (isbadpool(poolid))
    {
    	pi_printf("error: fail to buf get\r\n");
        return (void *)SYSERR;
    }

    bfpptr = &bfptab[poolid];

    im = disable();
//    usb_sem_wait(&bfpptr->freebuf, 0);
    bufptr = bfpptr->next;
    bfpptr->next = bufptr->next;
    restore(im);
    bufptr->next = bufptr;
    return (void *)(bufptr + 1);        /* +1 to skip past accounting structure */
}

/**
 * @ingroup memory_mgmt
 *
 * Return a buffer to its buffer pool.
 *
 * @param buffer
 *      Address of buffer to free, as returned by bufget().
 *
 * @return
 *      ::OK if buffer was successfully freed; otherwise ::SYSERR.  ::SYSERR can
 *      only be returned as a result of memory corruption or passing an invalid
 *      @p buffer argument.
 */
syscall buffree(void *buffer)
{
    struct bfpentry *bfpptr;
    struct poolbuf *bufptr;
    irqmask im;

    bufptr = ((struct poolbuf *)buffer) - 1;

    if (isbadpool(bufptr->poolid))
    {
    	pi_printf("error: fail to buffer free1\r\n");
        return SYSERR;
    }

    if (bufptr->next != bufptr)
    {
    	pi_printf("error: fail to buffer free2\r\n");
        return SYSERR;
    }

    bfpptr = &bfptab[bufptr->poolid];

    im = disable();
    bufptr->next = bfpptr->next;
    bfpptr->next = bufptr;
    restore(im);
//    usb_sem_signal(&bfpptr->freebuf);
    return OK;
}

/**
 * @ingroup memory_mgmt
 *
 * Acquire heap storage and subdivide into buffers.
 *
 * @param bufsize
 *      Size of individual buffers, in bytes.
 *
 * @param nbuf
 *      Number of buffers in the pool.
 *
 * @return
 *      On success, returns an identifier for the buffer pool that can be passed
 *      to bufget() or bfpfree().  On failure, returns ::SYSERR.
 */
int bfpalloc(uint bufsize, uint nbuf)
{
    struct bfpentry *bfpptr;
    struct poolbuf *bufptr;
    int id, buffer;
    irqmask im;
    sem_err err;
    bufsize = roundword(bufsize) + sizeof(struct poolbuf);

    if (bufsize > POOL_MAX_BUFSIZE ||
        bufsize < POOL_MIN_BUFSIZE || nbuf > POOL_MAX_NBUFS || nbuf < 1)
    {
    	pi_printf("error: fail to bfpalloc1\r\n");
        return SYSERR;
    }

    im = disable();
    for (id = 0; id < NPOOL; id++)
    {
        bfpptr = &bfptab[id];
        if (BFPFREE == bfpptr->state)
        {
            break;
        }
    }
    if (NPOOL == id)
    {
    	pi_printf("error: the NPOOL is full\r\n");
        restore(im);
        return SYSERR;
    }
    bfpptr->state = BFPUSED;
    restore(im);
//    err = usb_sem_new(&bfpptr->freebuf,0);
//    if (err != SEM_OK) {
//		pi_printf("error: fail to create freebuf sema\r\n");
//		return SYSERR;
//    }
//    if (SEM_NOT_OK == (int)bfpptr->freebuf)
//    {
//        bfpptr->state = BFPFREE;
//        pi_printf("error: fail to bfpalloc3\r\n");
//        return SYSERR;
//    }

    bfpptr->nbuf = nbuf;
    bfpptr->bufsize = bufsize;
    bufptr = (struct poolbuf *)memget(nbuf * bufsize);
    if ((void *)SYSERR == bufptr)
    {
    	pi_printf("error: fail to bfpalloc4\r\n");
//        usb_sem_free(&bfpptr->freebuf);
        bfpptr->state = BFPFREE;
        return SYSERR;
    }
    bfpptr->next = bufptr;
    bfpptr->head = bufptr;
    for (buffer = 0; buffer < nbuf; buffer++)
    {
        bufptr->poolid = id;
        bufptr->next = (struct poolbuf *)((ulong)bufptr + bufsize);
        bufptr = bufptr->next;
    }
//    usb_sem_signal(&bfpptr->freebuf);
    return id;
}

/**
 * @ingroup memory_mgmt
 *
 * Frees the memory allocated for a buffer pool.
 *
 * @param poolid
 *      Identifier of the buffer pool to free, as returned by bfpalloc().
 *
 * @return
 *      ::OK if the buffer pool was valid and was successfully freed; otherwise
 *      ::SYSERR.  If @p poolid specified a valid buffer pool, then this
 *      function can only return ::SYSERR as a result of memory corruption.
 */
syscall bfpfree(int poolid)
{
    struct bfpentry *bfpptr;
    irqmask im;

    if (isbadpool(poolid))
    {
    	pi_printf("error: fail to bfpfree1\r\n");
        return SYSERR;
    }

    bfpptr = &bfptab[poolid];

    im = disable();
    bfpptr->state = BFPFREE;
    if (SYSERR == memfree(bfpptr->head, bfpptr->nbuf * bfpptr->bufsize))
    {
    	pi_printf("error: fail to bfpfree2\r\n");
        restore(im);
        return SYSERR;
    }
//    if (SYSERR == semfree(bfpptr->freebuf))
//    {
//        restore(im);
//        return SYSERR;
//    }
//    usb_sem_free(&bfpptr->freebuf);
    restore(im);

    return OK;
}
