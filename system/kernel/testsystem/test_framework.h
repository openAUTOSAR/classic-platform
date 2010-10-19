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

#include <stdio.h>

/* Test flags */
#define TEST_FLG_OK				1
#define TEST_FLG_ASSERT			(1<<7)
#define TEST_FLG_DONE				(1<<2)
#define TEST_FLG_RUNNING			(1<<3)
#define TEST_FLG_NOT_IMPLEMENTED	(1<<4)
#define TEST_FLG_TOUCHED			(1<<5)
#define TEST_FLG_SEQ_ERROR			(1<<6)

#define TEST_VALUE_NC			(-1)

#define TASK_ID_ILL			99
#define RES_ID_ILL				99
#define ALARM_ID_ILL			99
#define SCHTBL_ID_ILL 	99
#define COUNTER_ID_ILL			99



#define TEST_INIT()			printf("Test init\n");
#define TEST_FAIL(_text)		TestFail((_text),  __FILE__,  __LINE__, __FUNCTION__ )
#define TEST_OK()				TestOk();
#define TEST_ASSERT(_cond)     TestTouch(); \
								if(!(_cond)) { \
									TEST_FAIL(#_cond); \
								}

/* Start to run a test */
#define TEST_RUN()				printf("Running test %d\n",test_nr);
/* Indicate that a test is done */
//#define TEST_DONE()
#define TEST_START(_str,_nr)		TestStart(_str,_nr)
#define TEST_NEXT(_str,_next_nr)	TestEnd(); TestStart(_str,_next_nr);
#define TEST_END()					TestEnd()
#define TEST_NOT_IMPLEMENTED()		TestNotImplemented();

typedef struct TestFixture {
	const char *description;
	int nr;
} TestFixtureType;

typedef struct TestWorld {
	uint16_t fixtureNr;
	uint16_t testNr;
	TestFixtureType fixtures[];
} TestWorldType;

extern TestWorldType TestWorld;

void TestDone( void );

void TestFail( const char *text,char *file,  int line, const char *function );
void TestOk( void );
void TestTouch( void );

void TestStart( const char *str, int testNr );
void TestInc( void );
void TestEnd( void );
void TestExit( int rv );
void TestSetFixture( uint32_t nextTestFixture, uint32_t nextTestNr,
		char *file,  int line, const char *function );
#define TEST_SET_FIXTURE( _nextTestFixture, _nextTestNr ) TestSetFixture( _nextTestFixture, _nextTestNr, __FILE__, __LINE__, __FUNCTION__ )

#if 0
// void TestSetFixture( uint32_t testFixture, uint32_t testNr );
#define TestSetFixture( _nextTestFixture, _nextTestNr )    \
	if( (_nextTestNr) != (TestWorld.testNr +1) ) {            \
		printf("%s %d FAILURE, seq failed\n",__FILE__,__LINE__); \
	}  \
	TestWorld.testNr  = _nextTestNr;          \
    TestWorld.fixtureNr = _nextTestFixture;
#endif

enum TestSeq {
	TEST_NR_01 = 1,
	TEST_NR_02,
	TEST_NR_03,
	TEST_NR_04,
	TEST_NR_05,
	TEST_NR_06,
	TEST_NR_07,
	TEST_NR_08,
	TEST_NR_09,
	TEST_NR_10,
	TEST_NR_11,
	TEST_NR_12,
	TEST_NR_13,
	TEST_NR_14,
	TEST_NR_15,
	TEST_NR_16,
	TEST_NR_17,
	TEST_NR_18,
	TEST_NR_19,
	TEST_NR_20,
	TEST_NR_21,
	TEST_NR_22,
	TEST_NR_23,
	TEST_NR_24,
	TEST_NR_25,
	TEST_NR_26,
	TEST_NR_27,
	TEST_NR_28,
	TEST_NR_29,
	TEST_NR_30,
	TEST_NR_31,
	TEST_NR_32,
	TEST_NR_33,
	TEST_NR_34,
	TEST_NR_35,
	TEST_NR_36,
	TEST_NR_37,
	TEST_NR_38,
	TEST_NR_39,
};

enum SubTest {
	SEQ_NR_01 = 1,
	SEQ_NR_02,
	SEQ_NR_03,
	SEQ_NR_04,
	SEQ_NR_05,
	SEQ_NR_06,
	SEQ_NR_07,
	SEQ_NR_08,
};


typedef void (*test_func_t)( void );

extern int test_suite;
extern int test_nr;

/* TODO: Move to a better place */
#if defined(CFG_MPC55XX)
/* On INTC first 8 interrupt are softtriggered */
#define IRQ_SOFTINT_0		INTC_SSCIR0_CLR0
#define IRQ_SOFTINT_1		INTC_SSCIR0_CLR1
#elif defined(CFG_ARM_CM3)
/* Cortex-M3 can softtrigger any interrupt. Use external here. */
#define IRQ_SOFTINT_0		EXTI0_IRQn
#define IRQ_SOFTINT_1		EXTI1_IRQn
#endif

#if 1
#define SECTION_SUP
#define SECTION_USER
#else
#define SECTION_SUP			__attribute__ ((section(".text_app_sup")))
#define SECTION_USER		__attribute__ ((section(".text_app_user")))
#endif

#define SECTION_BSS_SUPER	__attribute__ ((aligned (16),section(".bss")))
#define SECTION_BSS_USER	__attribute__ ((aligned (16),section(".bss")))

#define OS_STR__(x)		#x
#define OS_STRSTR__(x) 	OS_STR__(x)

#define DECLARE_TEST_BTASK(_nr, _task1, _task2, _task3 ) \
		__attribute__ ((section (".test_btask"))) const test_func_t btask_sup_matrix_ ## _nr[3] = { _task1, _task2, _task3 }

#define DECLARE_TEST_ETASK(_nr, _task1, _task2, _task3 ) \
		__attribute__ ((section (".test_etask"))) const test_func_t etask_sup_matrix_ ## _nr[3]  = { _task1, _task2, _task3 }

#define DECLARE_TASKS(_nr) \
	void etask_sup_l_##_nr( void ); \
	void etask_sup_m_##_nr( void ); \
	void etask_sup_h_##_nr( void ); \
	void btask_sup_l_##_nr( void ); \
	void btask_sup_m_##_nr( void ); \
	void btask_sup_h_##_nr( void );


/*
 * Declare tests
 */

// Test master processes
void OsIdle(void );
void etask_master( void );
void etask_sup_l( void ) SECTION_SUP;
void etask_sup_m( void ) SECTION_SUP;
void etask_sup_h( void ) SECTION_SUP;

void btask_sup_l( void ) SECTION_SUP;
void btask_sup_m( void ) SECTION_SUP;
void btask_sup_h( void ) SECTION_SUP;


// Tests
DECLARE_TASKS(01)
DECLARE_TASKS(02)
DECLARE_TASKS(03)
DECLARE_TASKS(04)

#endif /* TEST_FRAMEWORK_H_ */
