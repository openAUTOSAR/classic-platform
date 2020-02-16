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


#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "Std_Types.h"
#include "Ramlog.h"
#include "Os.h"

#if defined(CFG_ARM_CM3)
#include "irq_types.h"
#include "stm32f10x.h"
#endif

#ifdef USE_TTY_TCF_STREAMS
#include "streams.h"
#endif

#if defined(USE_TTY_UDE)
#include "serial_dbg_ude.h"
#endif


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
volatile char g_TConn __attribute__ ((section (".winidea_port")));

#else
static volatile unsigned char g_TWBuffer[TWBUFF_LEN] __attribute__ ((aligned (0x100))); // Transmit to WinIDEA terminal
static volatile unsigned char g_TRBuffer[TRBUFF_LEN] __attribute__ ((aligned (0x100)));
volatile char g_TConn __attribute__ ((section (".winidea_port")));

#endif

#endif

#ifdef USE_TTY_CODE_COMPOSER

#define _DTOPEN    (0xF0)
#define _DTCLOSE   (0xF1)
#define _DTREAD    (0xF2)
#define _DTWRITE   (0xF3)
#define _DTLSEEK   (0xF4)
#define _DTUNLINK  (0xF5)
#define _DTGETENV  (0xF6)
#define _DTRENAME  (0xF7)
#define _DTGETTIME (0xF8)
#define _DTGETCLK  (0xF9)
#define _DTSYNC    (0xFF)

#define LOADSHORT(x,y,z)  { x[(z)]   = (unsigned short) (y); \
                            x[(z)+1] = (unsigned short) (y) >> 8;  }

#define UNLOADSHORT(x,z) ((short) ( (short) x[(z)] +             \
				   ((short) x[(z)+1] << 8)))

#define PACKCHAR(val, base, byte) ( (base)[(byte)] = (val) )

#define UNPACKCHAR(base, byte)    ( (base)[byte] )


static unsigned char parmbuf[8];
#define CC_BUFSIZ 512
#define CC_BUFFER_SIZE ((CC_BUFSIZ)+32)
volatile unsigned int _CIOBUF_[CC_BUFFER_SIZE] __attribute__ ((section (".cio")));
static unsigned char CIOTMPBUF[CC_BUFSIZ];
static uint16 cio_tmp_buf_index = 0;

/***************************************************************************/
/*                                                                         */
/*  WRITEMSG()  -  Sends the passed data and parameters on to the host.    */
/*                                                                         */
/***************************************************************************/
void writemsg(unsigned char  command,
              register const unsigned char *parm,
              register const          char *data,
              unsigned int            length)
{
   volatile unsigned char *p = (volatile unsigned char *)(_CIOBUF_+1);
   unsigned int i;

   /***********************************************************************/
   /* THE LENGTH IS WRITTEN AS A TARGET INT                               */
   /***********************************************************************/
   _CIOBUF_[0] = length;

   /***********************************************************************/
   /* THE COMMAND IS WRITTEN AS A TARGET BYTE                             */
   /***********************************************************************/
   *p++ = command;

   /***********************************************************************/
   /* PACK THE PARAMETERS AND DATA SO THE HOST READS IT AS BYTE STREAM    */
   /***********************************************************************/
   for (i = 0; i < 8; i++)      PACKCHAR(*parm++, p, i);
   for (i = 0; i < length; i++) PACKCHAR(*data++, p, i+8);

   /***********************************************************************/
   /* THE BREAKPOINT THAT SIGNALS THE HOST TO DO DATA TRANSFER            */
   /***********************************************************************/
   __asm("	 .global C$$IO$$");
   __asm("C$$IO$$: nop");
}

/***************************************************************************/
/*                                                                         */
/*  READMSG()   -  Reads the data and parameters passed from the host.     */
/*                                                                         */
/***************************************************************************/
void readmsg(register unsigned char *parm,
	     register char          *data)
{
   volatile unsigned char *p = (volatile unsigned char *)(_CIOBUF_+1);
   unsigned int   i;
   unsigned int   length;

   /***********************************************************************/
   /* THE LENGTH IS READ AS A TARGET INT                                  */
   /***********************************************************************/
   length = _CIOBUF_[0];

   /***********************************************************************/
   /* UNPACK THE PARAMETERS AND DATA                                      */
   /***********************************************************************/
   for (i = 0; i < 8; i++) *parm++ = UNPACKCHAR(p, i);
   if (data != NULL)
      for (i = 0; i < length; i++) *data++ = UNPACKCHAR(p, i+8);
}

/****************************************************************************/
/* HOSTWRITE()  -  Pass the write command and its arguments to the host.    */
/****************************************************************************/
int HOSTwrite(int dev_fd, const char *buf, unsigned count)
{
   int result;

   // WARNING. Can only handle count == 1!
   if (count != 1) _exit(1);

   if (count > CC_BUFSIZ) count = CC_BUFSIZ;

   if (cio_tmp_buf_index < CC_BUFSIZ) {
	   CIOTMPBUF[cio_tmp_buf_index++] = *buf;

	   if (*buf != 0xA) { // Only flush if newline
		   return 0;
	   }
   }


   LOADSHORT(parmbuf,dev_fd,0);
   LOADSHORT(parmbuf,cio_tmp_buf_index,2);
   writemsg(_DTWRITE,parmbuf,(char *)CIOTMPBUF,cio_tmp_buf_index);
   readmsg(parmbuf,NULL);

   result = UNLOADSHORT(parmbuf,0);

   cio_tmp_buf_index = 0;

   return result;
}

#endif

#ifdef USE_TTY_TMS570_KEIL
#include "GLCD.h"
#endif

#define FILE_RAMLOG		3

/* Location MUST match NoICE configuration */
#ifdef USE_TTY_NOICE
static volatile char VUART_TX __attribute__ ((section (".noice_port")));
static volatile char VUART_RX __attribute__ ((section (".noice_port")));
volatile unsigned char START_VUART = 0;
#endif

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




int execve(const char *path, char * const argv[], char * const envp[] ) {
//int execve(char *name, char **argv, char **env){
	(void)path;
	(void)argv;
	(void)envp;
  	errno=ENOMEM;
  	return -1;
}

pid_t fork( void ) {
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

#ifdef USE_TTY_NOICE
	// Not tested at all
    int retval;
    while (VUART_RX != 0)
    {
    }

    retval = VUART_RX;
    VUART_RX = 0;
#endif

	/* Only support write for now, return 0 read */
	return 0;
}


int write(  int fd, const void *_buf, size_t nbytes)
{
  	//(void)fd;  // Normally 0- ?, 1-stdout, 2-stderr,
				// Added 3-ramlog,


	if( fd <= STDERR_FILENO ) {
#ifdef USE_TTY_NOICE
	char *buf1 = (char *)_buf;
	if (START_VUART)
	{
   	   for (int i = 0; i < nbytes; i++) {
   		   char c = buf1[i];
   		   if (c == '\n')
   		   {
   	   		   while (VUART_TX != 0)
   	   		   {
   	   		   }

   	   		   VUART_TX = '\r';
   		   }

   		   while (VUART_TX != 0)
   		   {
   		   }

   		   VUART_TX = c;
   	   }
	}
#endif

#ifdef USE_TTY_WINIDEA
		if (g_TConn)
		{
          char *buf = (char *)_buf;
		  unsigned char nCnt,nLen;
		  for(nCnt=0; nCnt<nbytes; nCnt++)
			{
			while(TWBUFF_FULL()) ;
			nLen=TWBUFF_TPTR;
			g_TWBuffer[nLen]=buf[nCnt];
			nLen=TWBUFF_INC(nLen);
			TWBUFF_TPTR=nLen;
			}
		}
#endif

#ifdef USE_TTY_T32
		char *buf = (char *)_buf;
		for (int i = 0; i < nbytes; i++) {
			if (*(buf + i) == '\n') {
				t32_writebyte ('\r');
//      		t32_writebyte ('\n');
			}
			t32_writebyte (*(buf + i));
		}
#endif

#ifdef USE_TTY_ARM_ITM
		char *buf = (char *)_buf;
		for (int i = 0; i < nbytes; i++) {
			ITM_SendChar(*(buf + i));
		}
#endif

#ifdef USE_TTY_TCF_STREAMS
		char *buf = (char *)_buf;
		for (int i = 0; i < nbytes; i++) {
			TCF_TTY_SendChar(*(buf + i));
		}
#endif

#ifdef USE_TTY_CODE_COMPOSER
	HOSTwrite(fd, _buf, nbytes);
#endif

#ifdef USE_TTY_TMS570_KEIL
	for (int i = 0; i < nbytes; i++) {
		GLCD_PrintChar((char *)(_buf + i));
	}
#endif

#ifdef USE_TTY_UDE
	UDE_write(fd,(char *)_buf,nbytes);
#endif


#if defined(USE_RAMLOG)
		{
			char *buf = (char *)_buf;
			for (int i = 0; i < nbytes; i++) {
				ramlog_chr (*(buf + i));
			}
		}
#endif

	}
	else
	{
#if defined(USE_RAMLOG)
		/* RAMLOG support */
		if(fd == FILE_RAMLOG) {
			char *buf = (char *)_buf;
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


int stat( const char *file, struct stat *st ) {
//int stat(char *file, struct stat *st) {
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


/* Should not really be here, but .. */

void _fini( void )
{

}


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

