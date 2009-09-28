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

#include <stdint.h>
#include "Platform_Types.h"
#include "simple_printf.h"

#define USE_DEBUG
#include "Trace.h"

int test_suite = 1;
int test_nr = 1;
int _test_ok = 0;
int _test_failed = 0;

int testCnt = 0;
struct test {
	uint8_t testSuite;
	uint8_t testNr;
	uint16_t status;
};

struct test testTable[50] = { {0} };


void test_done( void ) {
	dbg_printf(	"Test summary\n"
				"Total: %d\n"
				"OK   : %d\n"
				"FAIL : %d\n", _test_ok + _test_failed, _test_ok, _test_failed);

}

void test_fail( const char *text,char *file,  int line, const char *function ) {
	dbg_printf("%02d %02d FAILED, %s , %d, %s\n",test_suite, test_nr, file, line, function);
	testTable[testCnt].testSuite = test_suite;
	testTable[testCnt].testNr = test_nr;
	testTable[testCnt].status = 0;
	testCnt++;
	_test_failed++;
}


void test_ok( void ) {
	dbg_printf("%02d %02d OK\n",test_suite, test_nr);
	testTable[testCnt].testSuite = test_suite;
	testTable[testCnt].testNr = test_nr;
	testTable[testCnt].status = 1;
	testCnt++;
	_test_ok++;
}
