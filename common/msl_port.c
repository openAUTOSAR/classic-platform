/*
 * msl_port.c
 *
 *  Created on: 14 feb 2011
 *      Author: mahi
 */

/*
Methods called by MW MSL libraries to perform console IO:
*/

#include "stddef.h"


int  InitializeUART(void)
{
	return 0;
}

int ReadUARTN( char* buf, int cnt )
{
	(void)buf;
	(void)cnt;
	return 0;
}

int WriteUARTN( char* buf, int cnt )
{
	(void)buf;
	(void)cnt;
	return 0;
}


#if 0
void __init_hardware(void)
{
}


void __flush_cache(register void *address, register unsigned int size)
{
	(void)address;
	(void)size;

}

void __init_user(void)
{

}
#endif


void exit(int exit ) {
	(void)exit;
}



extern char _end[];

//static char *curbrk = _end;
#if 0

#ifndef HEAPSIZE
#define HEAPSIZE 16000
#endif

/*
 * The heap sadly have alignment that depends on the pagesize that
 * you compile malloc newlib with. From what I can tell from the
 * code that is a pagesize of 4096.
 */

unsigned char _heap[HEAPSIZE] __attribute__((aligned (4)));
//__attribute__((section(".heap")));

#else

/* The linker will allocate a heap for us
 * Importing variables set by the linker is never pretty... */
extern unsigned char _heap_addr[];
extern void _heap_size(void);

const ptrdiff_t _heap_size_proper = (const ptrdiff_t) &_heap_size; // Casting this to get a proper type

#define HEAPSIZE 	_heap_size_proper
#define _heap 		_heap_addr

#endif


void * sbrk( ptrdiff_t incr )
{
    static unsigned char *heap_end;
    unsigned char *prev_heap_end;

/* initialize */
    if( heap_end == 0 ){
    	heap_end = _heap;
    }
    prev_heap_end = heap_end;

	if( heap_end + incr - _heap > HEAPSIZE ) {
	/* heap overflow - announce on stderr */
		abort();
	}

   heap_end += incr;

   return prev_heap_end;
}
