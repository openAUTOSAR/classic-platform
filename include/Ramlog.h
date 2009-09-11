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



/*
 * Ramlog.h
 *
 *  Created on: 2009-apr-19
 *      Author: mahi
 */

#ifndef RAMLOG_H_
#define RAMLOG_H_

#include "xtoa.h"

void ramlog_puts( char *str );
void ramlog_chr( char c );

/*
 * Fast ramlog functions
 */
static inline void ramlog_str( char *str ) {
  ramlog_puts(str);
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

/*
 * var args ramlog functions
 */
int ramlog_printf(const char *format, ...);

#endif /* RAMLOG_H_ */
