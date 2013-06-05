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

#ifndef RAMLOG_H_
#define RAMLOG_H_

#include <stdint.h>
#include "device_serial.h"
#include "xtoa.h"

#if !defined(USE_RAMLOG)
#define ramlog_str( _x)
#define ramlog_dec( _x)
#define ramlog_hex( _x)

#else

void ramlog_fputs( char *str );
void ramlog_puts( char *str );
void ramlog_chr( char c );

/*
 * Fast ramlog functions
 */
static inline void ramlog_str( char *str ) {
  ramlog_fputs(str);
}

static inline void ramlog_dec( int val ) {
  char str[10]; // include '-' and \0
  ultoa(val,str,10);
  ramlog_str(str);
}

static inline void ramlog_hex( uint32_t val ) {
  char str[10]; // include '-' and \0
  ultoa(val,str,16);
  ramlog_str(str);
}
#endif


/*
 * var args ramlog functions
 */
#if defined(USE_RAMLOG)
int ramlog_printf(const char *format, ...);
#else
#define ramlog_printf(format,...)
#endif

void ramlog_init( void );

extern DeviceSerialType Ramlog_Device;

#endif /* RAMLOG_H_ */

