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
 * test_framework.h
 *
 *  Created on: 23 aug 2009
 *      Author: mahi
 */

#ifndef TEST_FRAMEWORK_H_
#define TEST_FRAMEWORK_H_

/* Test flags */
#define TEST_FLG_RUNNING		1
#define TEST_FLG_ASSERT		(1<<1)
#define TEST_FLG_DONE			(1<<2)
#define TEST_FLG_OK			(1<<3)

#define TEST_VALUE_NC			(-1)
void test_done( void );

void test_fail( const char *text,char *file,  int line, const char *function );
void test_ok( void );

void testStart( const char *str, int testNr );
void testInc( void );
void testEnd( void );
void testExit( int rv );

#endif /* TEST_FRAMEWORK_H_ */
