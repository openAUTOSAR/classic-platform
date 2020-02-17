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

#ifndef SEQTEST_H_
#define SEQTEST_H_

#include <stdio.h>
#include <stdint.h>


struct STest_World;
typedef struct STest_World * STestPtr;

#include "seqtest_output.h"


#define STEST_E_NOT_RUNNING		(1<<0)  /* Test was not running, although expected */
#define STEST_E_RUNNING			(1<<1)  /* Test was running, not expected */

/* Test flags */
#define TEST_FLG_OK				1
#define TEST_FLG_ASSERT			(1<<7)
#define TEST_FLG_DONE				(1<<2)
#define TEST_FLG_RUNNING			(1<<3)
#define TEST_FLG_NOT_IMPLEMENTED	(1<<4)
#define TEST_FLG_TOUCHED			(1<<5)
#define TEST_FLG_SEQ_ERROR			(1<<6)


struct STest_PrintCallbacks {
	void (*header)(STestPtr );
	void (*start)(STestPtr );
	void (*end)(STestPtr );
	void (*ok)(STestPtr);
	void (*fail)(STestPtr, char *test, char* file ,int line ,char* function);
//	void (*ok)(STestPtr);
	void (*statistics)(STestPtr);
};


extern struct STest_PrintCallbacks STest_XmlCallbacks;
extern struct STest_PrintCallbacks STest_TextCallbacks;

#if defined(STEST_XML_OUTPUT)
#define STEST_OUTPUT_CALLBACKS &STest_XmlCallbacks
#else
#define STEST_OUTPUT_CALLBACKS &STest_TextCallbacks
#endif


struct test {
	uint16_t testSuite;
	uint16_t testNr;
	uint16_t status;
	uint16_t pad;
	const char *name;
//	const char *description;
//	uint32_t expectedErrMask;
};

typedef struct STest_World {
	uint16_t 	testMajor;
	uint16_t 	testMinor;
	const char *name;
	uint16_t 	testOk;
	uint16_t 	testFail;
	uint16_t    testNotRun;

	struct STest_PrintCallbacks *printCb;
	struct test *testTable;
} STest_WorldType;


extern STest_WorldType STest_World;

//#define STEST_START( _nr, _name )			STest_Start(_nr, _name)
#define STEST_GET_TEST()					STest_World.testMajor
#define STEST_GET_MINOR()					STest_World.testMinor
#define STEST_FAIL(_msg) 					STest_Fail( _msg, __FILE__, __LINE__, __FUNCTION__ );
#define STEST_CHECK_ADD( _major, _minor )	STest_Check_Add( _major, _minor, __FILE__, __LINE__, __FUNCTION__ )
#define STEST_ASSERT( _x )					if( !(_x) ) { STest_Assert( #_x, __FILE__, __LINE__, __FUNCTION__ ); }


void STest_Start( int testMajor, const char *str );
void STest_End( uint16_t majorNr  );
void STest_Assert ( char *msg, char *file,  int line, const char *function );
void STest_Check_Add( uint32_t testMajor, uint32_t testMinor, char *file,  int line, const char *function );
void STest_Fail( const char *text, char *file,  int line, const char *function );
void STest_Init( struct STest_PrintCallbacks *callbacks );
void STest_Exit( void  );

// void STest_Ok( int testMajor );

static inline const char * STest_GetMajorName( STestPtr ptr ) {
	return ptr->testTable[ptr->testMajor].name;
}
static inline uint16_t STest_GetFailCount( STestPtr ptr ) {
	return ptr->testFail;
}

static inline uint16_t STest_GetNotRunCount( STestPtr ptr ) {
    return ptr->testNotRun;
}

static inline uint16_t STest_GetOkCount( STestPtr ptr ) {
	return ptr->testOk;
}
static inline uint16_t STest_GetMajor( STestPtr ptr ) {
	return ptr->testMajor;
}




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
	SEQ_NR_09,
	SEQ_NR_10,
	SEQ_NR_11,
	SEQ_NR_12,
	SEQ_NR_13,
};


#endif /* TEST_FRAMEWORK_H_ */
