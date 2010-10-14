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
	uint16_t testSuite;
	uint16_t testNr;
	uint16_t status;
	uint16_t pad;
//	const char *description;
//	uint32_t expectedErrMask;
};



struct test testTable[50] __attribute__ ((aligned(8))) = { {0} };

void TestInit( void ) {

}

void TestDone( void ) {
	printf(	"\nTest summary\n"
				"Total: %d\n"
				"OK   : %d\n"
				"FAIL : %d\n", _test_ok + _test_failed, _test_ok, _test_failed);

}

void TestSetFixture( uint32_t nextTestFixture, uint32_t nextTestNr,
		char *file,  int line, const char *function ) {
	_Bool error = 0;

	testTable[nextTestFixture].testSuite = nextTestFixture;
	testTable[nextTestFixture].testNr = nextTestNr;
	testTable[nextTestFixture].status |= TEST_FLG_RUNNING;
	testTable[nextTestFixture].pad = 0x0;

	/* For a new sequence should start with TEST_SET_FIXTURE( OSEK_TM_04, SEQ_NR_01 ) */
	if( nextTestNr == 1) {
		if( (nextTestFixture) != (TestWorld.fixtureNr +1) ) {
			printf("## Expected fixture=%u, found=%u\n",
					(unsigned)nextTestFixture,
					(unsigned)TestWorld.fixtureNr +1);
			error = 1;
		}

	} else {
		if( (nextTestFixture) != (TestWorld.fixtureNr ) ) {
			printf("## Expected same fixture=%u, found=%u\n",
					(unsigned)nextTestFixture,
					(unsigned)TestWorld.fixtureNr);
			error = 1;
		}
		if( nextTestNr  != ( TestWorld.testNr + 1 ) ) {
			printf("## Sequence is wrong. Found %u, expected %u in fixture %u\n",
					(unsigned)nextTestNr,
					(unsigned)TestWorld.testNr + 1,
					(unsigned)nextTestFixture);
			error = 1;
		}
	}

	if( error == 1 ) {
		printf("## Info: %s %d %s \n",file,line,function);
		testTable[nextTestFixture].status |= TEST_FLG_SEQ_ERROR;
	}
#if 0
	if( (nextTestNr) != (TestWorld.testNr +1) ) {
		printf("%s %d %s FAILURE, seq failed\n",file,line,function);
	}
#endif

	printf("Testing fixture %u and sub seq:%u\n",(unsigned)nextTestFixture, (unsigned)nextTestNr);
	TestWorld.testNr  = nextTestNr;
    TestWorld.fixtureNr = nextTestFixture;
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
	testTable[TestWorld.fixtureNr].testSuite = 0x0;
	testTable[TestWorld.fixtureNr].testNr = TestWorld.testNr;
	testTable[TestWorld.fixtureNr].status |= TEST_FLG_ASSERT;
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
//	testTable[testCnt].description = str;
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
			// testTable[testCnt].status &= TEST_FLG_RUNNING;
			testTable[testCnt].status |= TEST_FLG_OK;
			printf("OK\n");
		}
	} else {
		printf("testEnd() on a test that is not running\n");
	}
	testCnt++;
}

/**
 * Exit from the test system, no try to be graceful here.
 * @param rv
 */
void TestExit( int rv ) {
	printf("---- Done ----\n");
	Irq_Disable();
	exit(rv);
}

void TestTouch( void ) {
	testTable[TestWorld.fixtureNr].status |= TEST_FLG_TOUCHED;
}

void TestNotImplemented( void ) {
	testTable[TestWorld.fixtureNr].status |= TEST_FLG_NOT_IMPLEMENTED;
}


void TestOk( void ) {
	printf("%02d %02d OK\n",test_suite, test_nr);
	testTable[TestWorld.fixtureNr].testSuite = TestWorld.fixtureNr;
	testTable[TestWorld.fixtureNr].testNr = TestWorld.testNr;
	testTable[TestWorld.fixtureNr].status = 1;
	testCnt++;
	_test_ok++;
}

