
/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/

/* ----------------------------[information]----------------------------------
 * Author: mahi
 *
 * Description:
 *   Implements the glue between different clibs and ArcCore "system"
 */

/* ----------------------------[includes]------------------------------------*/

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "Std_Types.h"

/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/


/* Errno is made reentrant by using malloc and we don't want this. This is usually fixed
 * by undef errno and declaring it as extern int. It does not work when using GCC for
 * HC1X and this ifdef takes care of that.
 */
#undef errno
#if  defined(__GNUC__) && defined(CFG_HC1X)
int errno;
#else
extern int errno;
#endif

/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/



int open(const char *name, int flags, int mode){
	(void)name;
	(void)flags;
	(void)mode;

    return -1;
}



#include <sys/stat.h>
int fstat(int file, struct stat *st) {
	(void)file;
  	st->st_mode = S_IFCHR;
  	return 0;
}

pid_t getpid() {
  return 1;
}

int kill(int pid, int sig){
	(void)pid;
	(void)sig;
  	errno=EINVAL;
  	return(-1);
}

/* Do nothing */
int close( int fd ) {
	(void)fd;
  	return (-1);
}

/**
 *
 * @param fd
 * @return 1 if connected to a terminal
 */
int isatty( int fd )
{
	(void)fd;
	return 1;
}

/* If we use malloc and it runs out of memory it calls sbrk()
 */

#if defined(CFG_PPC)

/* linker symbols */
extern char _heap_start[];  // incomplete array to ensure not placed in small-data
extern char _heap_end[];

void * sbrk( ptrdiff_t incr )
{
    char *prevEnd;
    static char *nextAvailMemPtr = _heap_start;

    if( nextAvailMemPtr + incr >  _heap_end) {
		write( 2, "Heap overflow!\n", 15 );
		abort();
	}
    prevEnd = nextAvailMemPtr;
    nextAvailMemPtr += incr;
    return prevEnd;
}
#else
extern char _end[];

//static char *curbrk = _end;

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
		write( 2, "Heap overflow!\n", 15 );
		abort();
	}

   heap_end += incr;

   return (caddr_t) prev_heap_end;
}
#endif



int read( int fd, void *buf, size_t nbytes )
{
	(void)fd;
	(void)buf;
	(void)nbytes;

	return 0;
}
int write(  int fd, const void *_buf, size_t nbytes) {

	return (nbytes);
}

/* reposition read/write file offset
 * We can't seek, return error.*/
off_t lseek( int fd, off_t offset,int whence)
{
	(void)fd;
	(void)offset;
	(void)whence;

  	errno = ESPIPE;
  	return ((off_t)-1);
}

void __init( void )
{
}


#if defined(CFG_ARM)
void _exit( int status ) {
#ifdef USE_TTY_CODE_COMPOSER
	__asm("        .global C$$EXIT");
	__asm("C$$EXIT: nop");
#endif

	ShutdownOS( E_OS_EXIT_ABORT );

	while(1) ;
}
#endif

