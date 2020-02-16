/*
 * memget.c
 *
 *  Created on: 2013-10-31
 *      Author: Zhang Shuzhou
 */


#include "Cpu.h"
#include "pi_memory.h"
#include "usb_stddef.h"
#include "bcm2835.h"
#include "semaphore.h"
#include "Uart.h"
#include "Std_Types.h"

#define USB_MEM_SIZE  20971520
struct memblock USB_MEM[USB_MEM_SIZE];

struct memblock *memlist;        /* List of free memory blocks     */
struct bfpentry bfptab[NPOOL];  /* List of memory buffer pools    */
void *memheap;

int BUF_INITIALIZE = 0;
int MEM_INITIALIZE = 0;

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
	struct memblock *pmblock;     /* memory block pointer          */
    register struct memblock *prev, *curr, *leftover;
    char* s = NULL;

    if (MEM_INITIALIZE == 0) {

    	memheap = roundmb(&USB_MEM[0]);
    	memlist->next = pmblock = (struct memblock *)memheap;
		memlist->length = (uint) USB_MEM_SIZE;
		pmblock->next = NULL;
		pmblock->length = (uint) USB_MEM_SIZE;
		MEM_INITIALIZE = 1;
	}


    if (0 == nbytes)
    {
    	s = "0 byte\r\n";
    	mini_uart_sendstr(s);
        return (void *)SYSERR;
    }

    /* round to multiple of memblock size   */
    nbytes = (ulong)roundmb(nbytes);

    Irq_Disable();

    prev = memlist;
    curr = memlist->next;

    while (curr != NULL)
    {
        if (curr->length == nbytes)
        {
            prev->next = curr->next;
            memlist->length -= nbytes;
            Irq_Enable();
            return (void *)(curr);
        }
        else if (curr->length > nbytes)
        {
            /* split block into two */
            leftover = (struct memblock *)((ulong)curr + nbytes);
            prev->next = leftover;
            leftover->next = curr->next;
            leftover->length = curr->length - nbytes;
            memlist->length -= nbytes;

            Irq_Enable();
            return (void *)(curr);
        }
        prev = curr;
        curr = curr->next;
    }
    Irq_Enable();
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
    //irqmask im;
    ulong top;

    /* make sure block is in heap */
    if ((0 == nbytes)
        || ((ulong)memptr < (ulong)memheap))
    {
        return SYSERR;
    }

    block = (struct memblock *)memptr;
    nbytes = (ulong)roundmb(nbytes);

    Irq_Disable();

    prev = memlist;
    next = memlist->next;
    while ((next != NULL) && (next < block))
    {
        prev = next;
        next = next->next;
    }

    /* find top of previous memblock */
    if (prev == memlist)
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
       	Irq_Enable();
        return SYSERR;
    }

    memlist->length += nbytes;

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
    Irq_Enable();
    return E_OK;
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
	int i, id, buffer;

	char* s = NULL;

    //initialize once
	if (BUF_INITIALIZE == 0) {
		for (i = 0; i < NPOOL; i++) {
			bfpptr = &bfptab[i];
			bfpptr->state = BFPFREE;
		}
		BUF_INITIALIZE = 1;
	}

	bufsize = roundword(bufsize) + sizeof(struct poolbuf);

	if (bufsize > POOL_MAX_BUFSIZE || bufsize < POOL_MIN_BUFSIZE
			|| nbuf > POOL_MAX_NBUFS || nbuf < 1) {

		s = "bufsize wrong\r\n";
		mini_uart_sendstr(s);
		return SYSERR;
	}


	Irq_Disable();
	for (id = 0; id < NPOOL; id++) {
		bfpptr = &bfptab[id];
		if (BFPFREE == bfpptr->state) {
			break;
		}
	}
	if (NPOOL == id) {
		 Irq_Enable();
		return SYSERR;
	}
	bfpptr->state = BFPUSED;
	Irq_Enable();

	bfpptr->freebuf = semcreate(0);
	if (SYSERR == (int) bfpptr->freebuf) {
		bfpptr->state = BFPFREE;
		return SYSERR;
	}

	bfpptr->nbuf = nbuf;
	bfpptr->bufsize = bufsize;
	bufptr = (struct poolbuf *) memget(nbuf * bufsize);
	if ((void *) SYSERR == bufptr) {
		semfree(bfpptr->freebuf);
		bfpptr->state = BFPFREE;

		s = "memget wrong\r\n";
	    mini_uart_sendstr(s);
		return SYSERR;
	}
	bfpptr->next = bufptr;
	bfpptr->head = bufptr;
	for (buffer = 0; buffer < nbuf; buffer++) {
		bufptr->poolid = id;
		bufptr->next = (struct poolbuf *) ((ulong) bufptr + bufsize);
		bufptr = bufptr->next;
	}
	signaln(bfpptr->freebuf, nbuf);

	return id;
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

	if (isbadpool(poolid)) {
		return (void *) SYSERR;
	}

	bfpptr = &bfptab[poolid];

	Irq_Disable();
	wait(bfpptr->freebuf);
	bufptr = bfpptr->next;
	bfpptr->next = bufptr->next;
	Irq_Enable();

	bufptr->next = bufptr;
	return (void *) (bufptr + 1); /* +1 to skip past accounting structure */
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
	char* s = NULL;
	bufptr = ((struct poolbuf *) buffer) - 1;

	if (isbadpool(bufptr->poolid)) {
		s = "is bad pool\r\n";
		mini_uart_sendstr(s);
		return SYSERR;
	}

	if (bufptr->next != bufptr) {
		s = "bufptr next wrong\r\n";
	    mini_uart_sendstr(s);
		return SYSERR;
	}

	bfpptr = &bfptab[bufptr->poolid];

	Irq_Disable();
	bufptr->next = bfpptr->next;
	bfpptr->next = bufptr;
	Irq_Enable();
	signaln(bfpptr->freebuf, 1);
	return E_OK;
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

    if (isbadpool(poolid))
    {
        return SYSERR;
    }

    bfpptr = &bfptab[poolid];

    Irq_Disable();
    bfpptr->state = BFPFREE;
    if (SYSERR == memfree(bfpptr->head, bfpptr->nbuf * bfpptr->bufsize))
    {
        Irq_Enable();
        return SYSERR;
    }
    if (SYSERR == semfree(bfpptr->freebuf))
    {
    	Irq_Enable();
        return SYSERR;
    }
    Irq_Enable();
    return E_OK;
}


/**
 * @ingroup libxc
 *
 * Copy the specified number of bytes of memory to another location.  The memory
 * locations must not overlap.
 *
 * @param dest
 *      Pointer to the destination memory.
 * @param src
 *      Pointer to the source memory.
 * @param n
 *      The amount of data (in bytes) to copy.
 *
 * @return
 *      @p dest
 */
void *pi_memcpy(void *dest, const void *src, unsigned long n)
{
    unsigned char *dest_p = dest;
    const unsigned char *src_p = src;
    unsigned long i;

    for (i = 0; i < n; i++)
    {
        dest_p[i] = src_p[i];
    }

    return dest;
}


/**
 * @ingroup libxc
 *
 * Compares two memory regions of a specified length.
 *
 * @param s1
 *      Pointer to the first memory location.
 * @param s2
 *      Pointer to the second memory location.
 * @param n
 *      Length, in bytes, to compare.
 *
 * @return
 *      A negative value, 0, or a positive value if the @p s1 region of memory
 *      is less than, equal to, or greater than the @p s2 region of memory,
 *      respectively.
 */
int pi_memcmp(const void *s1, const void *s2, unsigned long n)
{
    const unsigned char *p1 = s1, *p2 = s2;
    unsigned long i;

    for (i = 0; i < n; i++)
    {
        if (p1[i] != p2[i])
        {
            return (int)p1[i] - (int)p2[i];
        }
    }
    return 0;
}
