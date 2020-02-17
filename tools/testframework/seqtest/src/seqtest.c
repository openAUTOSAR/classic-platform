/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with
 * the terms contained in the written license agreement between you and ArcCore,
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as
 * published by the Free Software Foundation and appearing in the file
 * LICENSE.GPL included in the packaging of this file or here
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/


/*
 * A test system that tests sequences. embUnit is a based on calling test functions
 * to the job and the failure exits the test method with return. This test-system
 * is made to check sequences in a simple way that may also be over several modules.
 *
 * Evaluation of the test
 *
 *
 *  STest_Init(...)
 *  STEST_CHECK_AND_ADD(.. )
 *  STEST_CHECK_AND_ADD(.. )
 *  STest_End( ... )
 *
 */


/* ----------------------------[includes]------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "Std_Types.h"
#include "seqtest.h"
#include "Cpu.h"
#include "MemMap.h"
#include "Os.h"

#ifdef USE_TTY_TMS570_KEIL
#include "GLCD.h"
#endif


/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/


struct STest_PrintCallbacks STest_XmlCallbacks = {
	STest_XML_PrintHeader,
	STest_XML_PrintStart,
	STest_XML_PrintEnd,
	STest_XML_PrintOk,
	STest_XML_PrintFail,
	STest_XML_PrintStatistics
};

struct STest_PrintCallbacks STest_TextCallbacks = {
	STest_Text_PrintHeader,
	STest_Text_PrintStart,
	STest_Text_PrintEnd,
	STest_Text_PrintOk,
	STest_Text_PrintFail,
	STest_Text_PrintStatistics
};



/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/

#define SeqTest_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "MemMap.h"
struct test testTable[50] = { {0} };
#define SeqTest_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "MemMap.h"

#define SeqTest_START_SEC_VAR_INIT_UNSPECIFIED
#include "MemMap.h"
STest_WorldType STest_World = { .testTable = testTable };
#define SeqTest_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "MemMap.h"

#define SeqTest_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "MemMap.h"
static char STest_Buff[100];
#define SeqTest_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "MemMap.h"

/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/


/*
 * XML Stuff:
 * - XML_OUTPUTTER_SKIP_HEADER
 * - XML_OUTPUTTER_HTML_ESCAPE
 * - STEST_XML_OUTPUT
 * - CFG_MINIMAL_OUTPUT
 *   If defined do not print out OK
 */


void STest_Init( struct STest_PrintCallbacks *callbacks ) {
	STest_World.printCb = callbacks;
}


/**
 * Start a test
 */
void STest_Start( int testMajor, const char *str ) {

	if( (testTable[testMajor].status) != 0 ) {
		STEST_FAIL("Already started");
		return;
	}

	testTable[testMajor].name = str;
	testTable[testMajor].status = TEST_FLG_RUNNING;
	STest_World.testMajor = testMajor;
	STest_World.testMinor = SEQ_NR_01;
}

/**
 *
 * @param msg
 * @param file
 * @param line
 * @param function
 */
void STest_Assert ( char *msg, char *file,  int line, const char *function ) {

	/* Add ASSERT failure */
	testTable[STest_World.testMajor].status |= TEST_FLG_ASSERT;

	STest_Fail(msg,file,line,function );
}


/**
 *
 * @param testMajor
 * @param testMinor
 * @param file          Pointer to string with the filename (__FILE__)
 * @param line          Pointer to string with the filename (__LINE__)
 * @param function      Pointer to string with the function-name (__FUNC__)
 */
void STest_Check_Add( uint32_t testMajor, uint32_t testMinor,
		              char *file,         int line,
		              const char *function ) {


	if( (testTable[testMajor].status & TEST_FLG_RUNNING) == 0 ) {
		STest_Assert( "Test not running" , file, line, function );
	} else if ( (testMajor != STest_World.testMajor) || ((testMinor != STest_World.testMinor)) ) {
		snprintf(STest_Buff, 100, "Sequence Error %u/%u, expected %d/%d\n",
				(unsigned int)testMajor, (unsigned int)testMinor,   STest_World.testMajor, STest_World.testMinor  );
		STest_Assert( STest_Buff , file, line, function );
	}

	STest_World.testMinor++;
}


/**
 * Function that is called when a test fails
 *
 * @param text          Pointer with information text from the testcase.
 * @param file          Pointer to string with the filename (__FILE__)
 * @param line          Pointer to string with the filename (__LINE__)
 * @param function      Pointer to string with the function-name (__FUNC__)
 */
void STest_Fail( const char *text,char *file,  int line, const char *function ) {
	STest_World.printCb->fail(&STest_World,(char *)text,file,line,(char *)function);
}

/**
 * Function that ends a testcase.
 * @param majorNr
 */
void STest_End( uint16_t majorNr  ) {
	uint16_t status = testTable[majorNr].status;

	if( status & TEST_FLG_RUNNING ) {
		if( status & TEST_FLG_ASSERT ) {
			/* We have already printed stuff */
		    STest_World.testFail++;
		} else {
			/* All is OK */
			testTable[majorNr].status = TEST_FLG_OK;
			STest_World.printCb->ok(&STest_World);
			STest_World.testOk++;
		}
	} else {

	}
}

/**
 * Exit from the test system, no try to be graceful here.
 * @param rv
 */
void STest_Exit( void  ) {

	int i;
	uint16 status;

	/* Collect a report for the test suite */
	STest_World.testNotRun = 0;

	for ( i=TEST_NR_01 ; i<= STest_World.testMajor;i++ ) {

	    status = testTable[i].status;
		if( status == TEST_FLG_OK ) {
		} else if ( status == 0 ) {
		    STest_World.testNotRun++;
		}  else {
		}
	}

	STest_World.printCb->statistics(&STest_World);

	SYS_CALL_DisableAllInterrupts();
	exit( (STest_World.testFail == 0) ? 0 : 1);
}






