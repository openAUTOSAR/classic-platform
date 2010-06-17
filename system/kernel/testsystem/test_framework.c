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
#include <stdio.h>
#include <stdlib.h>
#include "Platform_Types.h"
#include "test_framework.h"
#include "Cpu.h"

int test_suite = 1;
int test_nr = 1;
int _test_ok = 0;
int _test_failed = 0;


int testCnt = 0;



struct test {
	uint8_t testSuite;
	uint8_t testNr;
	uint16_t status;
	const char *description;
	uint32_t expectedErrMask;
};

struct test testTable[50] = { {0} };


void testInit( void ) {

}

void test_done( void ) {
	printf(	"\nTest summary\n"
				"Total: %d\n"
				"OK   : %d\n"
				"FAIL : %d\n", _test_ok + _test_failed, _test_ok, _test_failed);

}

/**
 *
 * @param text
 * @param file
 * @param line
 * @param function
 */
void test_fail( const char *text,char *file,  int line, const char *function ) {
	printf("%02d %02d FAILED, %s , %d, %s\n",test_suite, test_nr, file, line, function);
	testTable[testCnt].testSuite = test_suite;
	testTable[testCnt].testNr = test_nr;
	testTable[testCnt].status = TEST_FLG_ASSERT;
//	testCnt++;
//	_test_failed++;
}


/**
 * Set errors that are expected during the test
 * @param errMask
 */
void testSetErrorMask( uint32_t errMask ) {

}


void testValidateHook( void ) {

}
/**
 * Start a test
 */
void testStart( const char *str, int testNr ) {
	testTable[testCnt].status = TEST_FLG_RUNNING;
	testTable[testCnt].testNr = testNr;
	testTable[testCnt].description = str;
	printf("%3d %3d %s\n",testCnt,testNr,str);
}

void testInc( void ) {
	testCnt++;
}

/**
 * End a testcase.
 */
void testEnd( void ) {
	uint16_t status = testTable[testCnt].status;

	if( status & TEST_FLG_RUNNING ) {
		if( status & TEST_FLG_ASSERT ) {

		} else {
			/* All is OK */
			testTable[testCnt].status &= TEST_FLG_RUNNING;
			testTable[testCnt].status |= TEST_FLG_OK;
		}
	} else {
		printf("testEnd() on a test that is not running\n");
	}
	testCnt++;
}

void testExit( int rv ) {
	Irq_Disable();
	exit(rv);
}


void test_ok( void ) {
	printf("%02d %02d OK\n",test_suite, test_nr);
	testTable[testCnt].testSuite = test_suite;
	testTable[testCnt].testNr = test_nr;
	testTable[testCnt].status = 1;
	testCnt++;
	_test_ok++;
}

