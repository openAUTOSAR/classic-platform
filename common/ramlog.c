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
 *
 * Assumes some c-lib support:
 * - The clib support should be able to open a file called "ramlog".
 * - Printing to that file will print to the ramlog.
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include "simple_printf.h"


#ifndef CFG_RAMLOG_SIZE
#define CFG_RAMLOG_SIZE  2000
#endif

static unsigned char ramlog[CFG_RAMLOG_SIZE] __attribute__ ((section (".ramlog")));
static unsigned ramlog_curr __attribute__ ((section (".ramlog")));
static unsigned ramlog_session __attribute__ ((section (".ramlog")));

static FILE *ramlogFile = 0;


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

/**
 * Print a string to the ramlog
 * @param str
 */
void ramlog_puts( char *str ) {

  while(*str!=0) {
	ramlog_chr(*str++);
  }
  ramlog_chr('\n');
}

/**
 * Formatted print for the ramlog.
 *
 * @param format The format string.
 */
void ramlog_printf( const char *format, ... ) {

	// Fast and ugly ramlog support.
	volatile int rv;
	va_list args;
	va_start(args,format);

	rv = vfprintf(ramlogFile,format, args);
	va_end(args);
}


/**
 * Initialize the ramlog. Must be called before any other ramlog functions.
 */
void ramlog_init()
{
	char buf[32];
    if( ramlog_curr>CFG_RAMLOG_SIZE)
    {
      ramlog_curr = 0;
      ramlog_session = 0;
    }

    ramlogFile = fopen("ramlog","a");

    ramlog_session++;

    simple_sprintf(buf, "Session (%d)\n", ramlog_session);
    ramlog_puts(buf);
}
