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


#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Std_Types.h"
#include "Ramlog.h"

#if defined(CFG_ARM_CM3)
#include "irq_types.h"
#include "core_cm3.h"
#endif


#if defined(CFG_ARM)
#define open	_open
#define exit	_exit
#define fstat	_fstat
#define getpid	_getpid
#define kill	_kill
#define close	_close
#define isatty	_isatty
#define sbrk	_sbrk
#define read	_read
#define write	_write
#define lseek	_lseek
#endif

// Operation on Winidea terminal buffer


#define TWBUFF_SIZE 0x100
#define TRBUFF_SIZE 0x100


#define TBUFF_PTR 2

#define TWBUFF_LEN (TWBUFF_SIZE+TBUFF_PTR)
#define TRBUFF_LEN (TRBUFF_SIZE+TBUFF_PTR)
#define TWBUFF_TPTR (g_TWBuffer[TWBUFF_SIZE+0])
#define TWBUFF_CPTR (g_TWBuffer[TWBUFF_SIZE+1])
#define TWBUFF_INC(n) ((n + 1)&(TWBUFF_SIZE-1))
#define TWBUFF_FULL() (TWBUFF_TPTR==((TWBUFF_CPTR-1)&(TWBUFF_SIZE-1)))

#ifdef USE_TTY_WINIDEA

#if defined(MC912DG128A)
static volatile unsigned char g_TWBuffer[TWBUFF_LEN];
static volatile unsigned char g_TRBuffer[TRBUFF_LEN];
static volatile char g_TConn __attribute__ ((section (".winidea_port")));

#else
static volatile unsigned char g_TWBuffer[TWBUFF_LEN] __attribute__ ((aligned (0x100))); // Transmit to WinIDEA terminal
static volatile unsigned char g_TRBuffer[TRBUFF_LEN] __attribute__ ((aligned (0x100)));
static volatile char g_TConn __attribute__ ((section (".winidea_port")));

#endif

#endif

#define FILE_RAMLOG		3

/*
 * T32 stuff
 */

// This must be in un-cached space....
#ifdef USE_TTY_T32
static volatile char t32_outport __attribute__ ((section (".t32_outport")));

void t32_writebyte(char c)
{
	/* T32 can hang here for several reasons;
	 * - term.view e:address.offset(v.address(t32_outport)) e:0
	 */

	while (t32_outport != 0 ) ; /* wait until port is free */
	t32_outport = c; /* send character */
}
#endif
/*
 * clib support
 */

/* Do nothing */
int close( int fd ) {
	(void)fd;
  	return (-1);
}

char *__env[1] = { 0 };
char **environ = __env;


#include <errno.h>
#undef errno
extern int errno;


int execve(const char *path, char * const argv[], char * const envp[] ) {
//int execve(char *name, char **argv, char **env){
	(void)path;
	(void)argv;
	(void)envp;
  	errno=ENOMEM;
  	return -1;
}

int fork() {
  errno=EAGAIN;
  return -1;
}

#include <sys/stat.h>
int fstat(int file, struct stat *st) {
	(void)file;
  	st->st_mode = S_IFCHR;
  	return 0;
}

/* Returns 1 if connected to a terminal. T32 can be a terminal
 */

int isatty( int fd )
{
	(void)fd;
	return 1;
}

/*
int fstat( int fd,  struct stat *buf )
{
  buf->st_mode = S_IFCHR;
  buf->st_blksize = 0;

  return (0);
}
*/

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

int open(const char *name, int flags, int mode){
	(void)name;
	(void)flags;
	(void)mode;

#if defined(USE_RAMLOG)
	if( strcmp(name,"ramlog") == 0 ) {
		return FILE_RAMLOG;
	}
#endif

    return -1;
}

int read( int fd, void *buf, size_t nbytes )
{
	(void)fd;
	(void)buf;
	(void)nbytes;
#ifdef USE_TTY_WINIDEA
	(void)g_TRBuffer[0];
#endif

	/* Only support write for now, return 0 read */
	return 0;
}


int write(  int fd, const void *_buf, size_t nbytes)
{
	char *buf = (char *)_buf;
  	//(void)fd;  // Normally 0- ?, 1-stdout, 2-stderr,
				// Added 3-ramlog,


	if( fd <= STDERR_FILENO ) {
#ifdef USE_TTY_WINIDEA
  	if (g_TConn)
  	{
  	  unsigned char nCnt,nLen;
  	  for(nCnt=0; nCnt<nbytes; nCnt++)
	    {
  	    while(TWBUFF_FULL());
  	    nLen=TWBUFF_TPTR;
  	    g_TWBuffer[nLen]=buf[nCnt];
  	    nLen=TWBUFF_INC(nLen);
  	    TWBUFF_TPTR=nLen;
	    }
  	}
#endif

#ifdef USE_TTY_T32
  	for (int i = 0; i < nbytes; i++) {
    	if (*(buf + i) == '\n') {
      		t32_writebyte ('\r');
//      		t32_writebyte ('\n');
    	}
    	t32_writebyte (*(buf + i));
  	}
#endif
#ifdef USE_TTY_ARM_ITM
  	for (int i = 0; i < nbytes; i++) {
  	  	ITM_SendChar(*(buf + i));
  	}
#endif

	}
	else
	{
#if defined(USE_RAMLOG)
		/* RAMLOG support */
		if(fd == FILE_RAMLOG) {
		  	for (int i = 0; i < nbytes; i++) {
				ramlog_chr (*(buf + i));
		  	}
		}
#endif
	}

	return (nbytes);
}

int arc_putchar(int fd, int c) {
	char cc = c;
	write( fd,&cc,1);

	return 0;
}

/* If we use malloc and it runs out of memory it calls sbrk()
 */
#if 1

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
    if( heap_end == 0 )
    	heap_end = _heap;

	prev_heap_end = heap_end;

	if( heap_end + incr - _heap > HEAPSIZE ) {
	/* heap overflow - announce on stderr */
		write( 2, "Heap overflow!\n", 15 );
		abort();
	}

   heap_end += incr;

   return (caddr_t) prev_heap_end;
}
#else
void *sbrk(int inc )
{
	/* We use our own malloc */
	return (void *)(-1);
}
#endif

int stat( const char *file, struct stat *st ) {
//int stat(char *file, struct stat *st) {
	(void)file;
  	st->st_mode = S_IFCHR;
  	return 0;
}


int getpid() {
  return 1;
}

#include <errno.h>
#undef errno
extern int errno;
int kill(int pid, int sig){
	(void)pid;
	(void)sig;
  	errno=EINVAL;
  	return(-1);
}


/* Should not really be here, but .. */

void _fini( void )
{

}


void __init( void )
{

}
#if defined(CFG_ARM)
void _exit( int status ) {
	while(1);
}
#endif

