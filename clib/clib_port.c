/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/


/* ----------------------------[information]----------------------------------
 * Author: mahi
 *
 * Description:
 *   Implements the glue between different clibs and ArcCore "system"
 *
 * Implementation Notes:
 *   Heavily inspired by http://neptune.billgatliff.com/newlib.html
 *
 *   If a module should be used is controlled by USE_TTY_T32, USE_TTY_WINIDEA, etc
 *   The console is controlled by SELECT_CONSOLE=TTY_T32|TTY_WINIDEA|RAMLOG|....
 *
 *   To build the device
 *
 *
 *
 *
 *
 *
 *
 */

/* ----------------------------[includes]------------------------------------*/

#if defined(__GNUC__)
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "Std_Types.h"
#if defined(USE_KERNEL)
#include "Os.h"
#endif

#include "device_serial.h"

#if defined(USE_TTY_UDE)
#include "serial_dbg_ude.h"
#endif
#if defined(USE_TTY_T32)
#include "serial_dbg_t32.h"
#endif
#if defined(USE_TTY_WINIDEA)
#include "serial_dbg_winidea.h"
#endif

#include "Ramlog.h"


#if defined(CFG_ARM_CM3)
#include "irq_types.h"
#include "stm32f10x.h"
#endif


/* ----------------------------[private define]------------------------------*/


#if defined(CFG_ARM)
#define _OPEN		_open
#define _EXIT		_exit
#define _FSTAT		_fstat
#define _GETPID		_getpid
#define _KILL		_kill
#define _CLOSE		_close
#define _ISATTY		_isatty
#define _SBRK		_sbrk
#define _READ		_read
#define _WRITE		_write
#define _LSEEK		_lseek
#else
#define _OPEN		open
#define _EXIT		exit
#define _FSTAT		fstat
#define _GETPID		getpid
#define _KILL		kill
#define _CLOSE		close
#define _ISATTY		isatty
#define _SBRK		sbrk
#define _READ		read
#define _WRITE		write
#define _LSEEK		lseek
#endif


/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/


DeviceSerialType *deviceList[] = {
#if defined(USE_TTY_T32)
		&T32_Device,
#endif
#if defined(USE_TTY_UDE)
		&UDE_Device,
#endif
#if defined(USE_RAMLOG)
		&Ramlog_Device,
#endif
#if defined(USE_TTY_WINIDEA)
		&WinIdea_Device,
#endif
};


/* Global file descriptor to device list */
DeviceSerialType *fileList[] = {
#if defined(USE_TTY_T32)
	[0] = &T32_Device,		/* stdin  */
	[1] = &T32_Device,		/* stdout */
	[2] = &T32_Device,		/* stderr */
#elif defined(USE_TTY_UDE)
	[0] = &UDE_Device,		/* stdin  */
	[1] = &UDE_Device,		/* stdout */
	[2] = &UDE_Device,		/* stderr */
#elif defined(USE_TTY_WINIDEA)
	[0] = &WinIdea_Device,		/* stdin  */
	[1] = &WinIdea_Device,		/* stdout */
	[2] = &WinIdea_Device,		/* stderr */
#endif


#if defined(USE_RAMLOG)
	&Ramlog_Device,		/* stdin  */
	&Ramlog_Device,		/* stdout */
	&Ramlog_Device,		/* stderr */
#endif
};



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


/**
 * Get a file number from a file
 * @param file
 * @return
 */
#ifndef SIMPLE_STDIO
int fileno( FILE *file ) {
	return file->fileNo;
}
#endif

/**
 * POSIX open function
 *
 * Should probably support some O and S flags here
 *   See http://pubs.opengroup.org/onlinepubs/009695399/functions/open.html
 *
 * @param name  Name of the file to open
 * @param flags O_xx flags
 * @param mode  S_xx modes.
 * @return	The file descriptor or -1 if failed to open.
 */
int _OPEN(const char *name, int flags, int mode){
	int i;
	int fd = -1;

	for( i=0; i<sizeof(fileList)/sizeof(fileList[0]); i++ ) {
		if( strcmp(name,fileList[i]->name) == 0 ) {
			fd = i;
		}
	}

	if( fd != -1) {
		if( fileList[fd]->open != NULL ) {
			fileList[fd]->open(name,flags,mode);
		}
	} else {
		/* TODO: Set errno?! */
	}

    return fd;
}


#if defined(__GNUC__)
int _FSTAT(int file, struct stat *st) {
	(void)file;
  	st->st_mode = S_IFCHR;
  	return 0;
}


pid_t _GETPID() {
  return 1;
}
#endif

int _KILL(int pid, int sig){
	(void)pid;
	(void)sig;
  	errno=EINVAL;
  	return(-1);
}

/* Do nothing */
int _CLOSE( int fd ) {
	(void)fd;
  	return (-1);
}

/**
 *
 * @param fd
 * @return 1 if connected to a terminal
 */
int _ISATTY( int fd )
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

void * _SBRK( ptrdiff_t incr )
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

#if !defined(SQUAWK)
void * _SBRK( ptrdiff_t incr )
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
#endif



int _READ( int fd, void *buf, size_t nbytes )
{

	fileList[fd]->read(buf,nbytes);

	return 0;
}

/**
 * Write data to
 *
 * @param fd      The file descriptor
 * @param _buf
 * @param nbytes
 * @return
 */
int _WRITE(  int fd, const void *buf, size_t nbytes) {

	fileList[fd]->write((void *)buf,nbytes);

	return (nbytes);
}

#if defined(__GNUC__)
/* reposition read/write file offset
 * We can't seek, return error.*/
off_t _LSEEK( int fd, off_t offset,int whence)
{
	(void)fd;
	(void)offset;
	(void)whence;

  	errno = ESPIPE;
  	return ((off_t)-1);
}
#endif


#if defined(__GNUC__)
void __init( void )
{
}
#endif


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

#if defined(__CWCC__)

void exit(int exit ) {
	(void)exit;
}

#endif
#if 0

#include "../../MSL_Common_Embedded/Include/UART.h"

UARTError InitializeUART(UARTBaudRate baudRate)
{
	(void)baudRate;
	return 0;
}

UARTError ReadUARTN(void* bytes, unsigned long length) {
	(void)bytes;
	(void)length;
	return 0;
}

UARTError WriteUART1(char c) {
	(void)c;
	return 0;
}

UARTError WriteUARTN(const void* buf, unsigned long cnt)
{
	(void)buf;
	(void)cnt;
	return 0;
}

UARTError ReadUART1(char* c) {
	(void)c;
	return 0;
}


#endif



