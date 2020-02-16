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

/**
 * A very simple ramlog.
 *
 * Features:
 * - Prints to a ram space in section ".ramlog"
 * - The size is configurable using CFG_RAMLOG_SIZE (default is 1000)
 * - The ramlog section should not be cleared by the linkfile if one wants
 *   to have a ramlog that survives reset.
 * - Session support
 *
 * Assumes some c-lib support:
 * - The clib support should be able to open a file called "ramlog".
 * - Printing to that file will print to the ramlog.
 *
 * HEADER
 *
 *   The ramlog uses a very simple header.
 *
 *  Byte   0    1    2    3
 *       +----+----+----+----+
 *       | 01 |CNT | X  |    |
 *       +----+----+----+----+
 *
 *   Byte 0 is 01
 *   Byte 1 is a counter that incremented for each time the ramlog is started.
 *   Byte 2 is not defined yet.
 *   Byte 3 is 01
 *
 *   01 is used since it's control sign and not a printable char.
 *
 * STRATEGY FOR SESSION
 *   Cases:
 *   1. The ramlog is empty, ie the header is not there at first position
 *   2. The header is in first poistion
 *      - look for the next?
 *      - Use saved inforation?
 *
 *
 */

#include <stdio.h>

/*lint -save -e451 */
#include <stdarg.h>
/*lint -restore */
#include <assert.h>
#include "Ramlog.h"
#include "MemMap.h"
#include "device_serial.h"

#ifndef CFG_RAMLOG_SIZE
#define CFG_RAMLOG_SIZE  2000
#endif

#define RAMLOG_MAGIC	  1


SECTION_RAMLOG static unsigned char ramlog[CFG_RAMLOG_SIZE];

#if defined(CFG_RAMLOG_SESSION)
SECTION_RAMLOG static unsigned ramlog_curr;
SECTION_RAMLOG static unsigned ramlog_session;
#else
static unsigned ramlog_curr = 0;
#endif

static FILE *rFile;

//#define RAMLOG_FILENO  (FILE *)3


/**
 * Print a char to the ramlog
 * @param c
 */
void ramlog_chr( char c ) {
  ramlog[ramlog_curr++] = c;
  if( ramlog_curr >= CFG_RAMLOG_SIZE ) {
	  ramlog_curr = 0;
  }
}


void ramlog_fputs(char *str) {

	while (*str != 0) {
		ramlog_chr(*str++);
	}
}

/**
 * Print a string to the ramlog
 * @param str
 */
void ramlog_puts( char *str ) {

	ramlog_fputs(str);
	ramlog_chr('\n');
}

/**
 * Formatted print for the ramlog.
 *
 * @param format The format string.
 */
int ramlog_printf( const char *format, ... ) {

	// Fast and ugly ramlog support.
    /*lint -save -e451 -e438 -e530 -e550 -e551 */
	volatile int rv;
	va_list args;
	va_start(args,format);

	assert( rFile != NULL );
	rv = vfprintf(rFile, format, args);
	va_end(args);
	return rv;
}


/**
 * Initialize the ramlog. Must be called before any other ramlog functions.
 */
void ramlog_init( void )
{
	rFile = fopen("ramlog","r");

#if defined(CFG_RAMLOG_SESSION)
	char buf[32];
	/* Check for existing session */
	if( (ramlog[0] != RAMLOG_MAGIC) || (ramlog[3] != RAMLOG_MAGIC) ) {
		ramlog_curr = 0;
		ramlog_session = 0;
	} else {
		/*  Search the ramlog */
	}
    ramlog_session++;
    simple_sprintf(buf, "Session (%d)\n", ramlog_session);
    ramlog_puts(buf);

#else
    (void)ramlog[0]; /* To avoid lint warning */
	ramlog_curr = 0;
#endif
}

static int Ramlog_Write(  uint8_t *data, size_t nbytes)
{
	for (unsigned int i = 0; i < nbytes; i++) {
		ramlog_chr(*data++);
	}
	return nbytes;
}

static int Ramlog_Open( const char *path, int oflag, int mode ) {
	(void)path;
	(void)oflag;
	(void)mode;

	return 0;
}



DeviceSerialType Ramlog_Device = {
	.name = "ramlog",
	.read = NULL,
	.write = Ramlog_Write,
	.open = Ramlog_Open,
};




