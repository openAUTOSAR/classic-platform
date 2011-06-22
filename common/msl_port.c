/*
 * msl_port.c
 *
 *  Created on: 14 feb 2011
 *      Author: mahi
 */

/*
Methods called by MW MSL libraries to perform console IO:
*/

#include "Os.h"
#include "stddef.h"

#ifdef USE_TTY_WINIDEA

#define TWBUFF_SIZE 0x100
#define TRBUFF_SIZE 0x100
#define TBUFF_PTR 2

#define TWBUFF_LEN		(TWBUFF_SIZE+TBUFF_PTR)
#define TRBUFF_LEN		(TRBUFF_SIZE+TBUFF_PTR)
#define TWBUFF_TPTR		(g_TWBuffer[TWBUFF_SIZE+0])
#define TWBUFF_CPTR		(g_TWBuffer[TWBUFF_SIZE+1])
#define TWBUFF_INC(n)	((n + 1)&(TWBUFF_SIZE-1))
#define TWBUFF_FULL()	(TWBUFF_TPTR==((TWBUFF_CPTR-1)&(TWBUFF_SIZE-1)))

volatile char g_TConn;
volatile unsigned char g_TWBuffer[TWBUFF_LEN] __attribute__ ((aligned (0x100))); // Transmit to WinIDEA terminal
volatile unsigned char g_TRBuffer[TRBUFF_LEN] __attribute__ ((aligned (0x100)));


#endif

int  InitializeUART(void)
{
	return 0;
}

int ReadUARTN( char* buf, int cnt )
{
#ifdef USE_TTY_WINIDEA
	(void)g_TRBuffer[0];
#endif
	(void)buf;
	(void)cnt;
	return 0; // No error
}

int ReadUART1(char* c) {
	return ReadUARTN( c, 1 );
}

int WriteUARTN( char* buf, int cnt )
{
#ifdef USE_TTY_WINIDEA
	if (g_TConn)
	{
		unsigned char nCnt,nLen;
		for(nCnt = 0; nCnt < cnt; nCnt++)
		{
			while( TWBUFF_FULL() ) {}
			nLen = TWBUFF_TPTR;
			g_TWBuffer[nLen] = buf[nCnt];
			nLen = TWBUFF_INC(nLen);
			TWBUFF_TPTR = nLen;
		}
	}
#endif
#if defined(USE_RAMLOG)
		{
			char *pbuf = buf;
			for (int i = 0; i < cnt; i++) {
				ramlog_chr (*(pbuf + i));
			}
		}
#endif

	(void)buf;
	(void)cnt;
	
	return 0; // No error
}

int WriteUART1(char c) {
	return WriteUARTN( &c, 1 );
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
	/* !! WARNING !! !! WARNING !! !! WARNING !!
	 *
	 * ENDING UP HERE CAN BE VERY BAD:
	 * 1. You got an assert() (calls exit())
	 * 2. For some other reason (deliberately exited main() ?)
	 *
	 * TODO: Call reset of card?
	 * */
	DisableAllInterrupts();
	while(1);
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
