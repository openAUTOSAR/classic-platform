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
#include <stdarg.h>
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
	ramlog_curr = 0;
#endif
}

static int Ramlog_Write(  uint8_t *data, size_t nbytes)
{
	for (int i = 0; i < nbytes; i++) {
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




