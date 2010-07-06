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
 * Testsystem Requirements:
 * - Similar to EmbUnit():
 *   - EmbUnit is built around methods..this does not work well for an OS so
 *     use macros, TEST_START() and TEST_END() start and end testcases.
 *   - TEST_ASSERT() macro the same
 *   - XML output should be the same
 *   - Same statistict output
 * - Be able to survive a known crash and restore to the current testcase
 *   This would be very useful when testing exception behaviour.
 * - It would have the test-cases grouped, so it will be easy the remove test that don't work
 *   - Invent a way to do this pretty for different ARCH's. Today ARCH dependent
 *     test-cases are #ifdef'd.
 *     ALT_1: Have different header files for each ARCH that disable certain test-cases ?
 *
 * Bad stuff in current implementation:
 * - The initial thought (see suite_01 ) was to build a rather large testsystem
 *   that whould have few configurations. Howevent, this does not work well at all for
 *   systems with that is very low on ROM/RAM.
 * - I wish there was one place to enable/disable tests. Now it's spread out all over the place.
 * - I guess the DECLARE_TEST_ETASK() macros was good for a large test-system,
 *   but it should go away (suite_01)
 * - Hooks and error handling should be unitfied into one file.
 * - That test-cases is dependent on the former testcase to increase "test_nr"
 * - Should be able to see what tests are not run?!
 *
 *
 * Total tests:   150
 * Not run:       5    (not touched by assert)
 * Failures:      10
 * Success:       135
 * Not Implented: 0
 *
 * Next:
 * 1. Fix statistics
 * 2. Cleanup of testCnt and other "indexes"
 * 3. Fix statistics over severaral test_suites. (move to NOLOAD section)
 *
 */



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


struct testStats {
	int ok;
	int fail;
	int notRun;
	int notImplemented;
};

struct test {
	uint8_t testSuite;
	uint8_t testNr;
	uint16_t status;
	const char *description;
	uint32_t expectedErrMask;
};

struct test testTable[50] = { {0} };


void TestInit( void ) {

}

void TestDone( void ) {
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
void TestFail( const char *text,char *file,  int line, const char *function ) {
	printf("%02d %02d FAILED, %s , %d, %s\n",test_suite, test_nr, file, line, function);
	testTable[testCnt].testSuite = test_suite;
	testTable[testCnt].testNr = test_nr;
	testTable[testCnt].status |= TEST_FLG_ASSERT;
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
void TestStart( const char *str, int testNr ) {
	testTable[testCnt].status = TEST_FLG_RUNNING;
	testTable[testCnt].testNr = testNr;
	testTable[testCnt].description = str;
	printf("%3d %3d %s\n",testCnt,testNr,str);
}

void TestInc( void ) {
	testCnt++;
}

/**
 * End a testcase.
 */
void TestEnd( void ) {
	uint16_t status = testTable[testCnt].status;

	if( status & TEST_FLG_NOT_IMPLEMENTED ) {
		printf("Not Implemented\n");
	} else 	if( (status & TEST_FLG_TOUCHED) == 0 ) {
		printf("NOT touched\n");
	} else 	if( status & TEST_FLG_RUNNING ) {
		if( status & TEST_FLG_ASSERT ) {

		} else {
			/* All is OK */
			testTable[testCnt].status &= TEST_FLG_RUNNING;
			testTable[testCnt].status |= TEST_FLG_OK;
			printf("OK\n");
		}
	} else {
		printf("testEnd() on a test that is not running\n");
	}
	testCnt++;
}

void TestExit( int rv ) {
	Irq_Disable();
	exit(rv);
}

void TestTouch( void ) {
	testTable[testCnt].status |= TEST_FLG_TOUCHED;
}

void TestNotImplemented( void ) {
	testTable[testCnt].status |= TEST_FLG_NOT_IMPLEMENTED;
}


void TestOk( void ) {
	printf("%02d %02d OK\n",test_suite, test_nr);
	testTable[testCnt].testSuite = test_suite;
	testTable[testCnt].testNr = test_nr;
	testTable[testCnt].status = 1;
	testCnt++;
	_test_ok++;
}

