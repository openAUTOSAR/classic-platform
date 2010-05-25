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

#ifndef OS_TEST_H_
#define OS_TEST_H_

#include "test_framework.h"
#include "debug.h"

typedef void (*test_func_t)( void );

#define TEST_FAIL(_text)		test_fail((_text),  __FILE__,  __LINE__, __FUNCTION__ )
#define TEST_OK()				test_ok();
#define TEST_ASSERT(_cond) 			if(!(_cond)) { TEST_FAIL(#_cond); }
#define TEST_RUN()				printf("Running test %d\n",test_nr);


extern int test_suite;
extern int test_nr;

#define TASK_ID_ILL			99
#define RES_ID_ILL				99
#define ALARM_ID_ILL			99
#define SCHTBL_ID_ILL 	99
#define COUNTER_ID_ILL			99

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
DECLARE_TASKS(01);
DECLARE_TASKS(02);
DECLARE_TASKS(03);
DECLARE_TASKS(04);

#endif /* OS_TEST_H_ */
