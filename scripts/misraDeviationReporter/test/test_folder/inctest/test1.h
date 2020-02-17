/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file if you have a valid commercial ArcCore license 
 * and then in accordance with the terms contained in the written license 
 * agreement between you and ArcCore.
 *-------------------------------- Arctic Core -----------------------------*/

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0 */

#include "embUnit/embUnit.h"
#include <string.h>

/*lint -w2 Warning level two in test files */
/*lint -e632 -e639 -e634 -e638 No strong typing for tests */


#include "Crc.h"


// Check values as described in the CRC specification
#define CRC8_CHECK_VALUE  			0x4B
#define CRC8_MAGIC_CHECK_VALUE  	0xC4

#define CRC8_H2F_CHECK_VALUE        0xDF
#define CRC8_H2F_MAGIC_CHECK_VALUE  0x42

#define CRC16_CHECK_VALUE           0x29B1
#define CRC16_MAGIC_CHECK_VALUE     0x0000

#define CRC32_CHECK_VALUE           0xCBF43926
#define CRC32_MAGIC_CHECK_VALUE     0xDEBB20E3

#define CRC32P4_CHECK_VALUE         0x1697D06A
#define CRC32P4_MAGIC_CHECK_VALUE   0x904CDDBF

// Test variables
// ------------------------------------------------------------------------------------------------
#define NBR_REF_VALUES   4
#define REF_VALUE_LENGTH 16
static const uint8 Crc_u8RefValues[NBR_REF_VALUES][REF_VALUE_LENGTH] =
    {{'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'},
    {'a','z','b','y','c','x','d','v','e','u','f','t','g','s','h','r'},
    {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
    {0,255,1,254,2,253,3,252,4,251,5,250,6,249,7,248}};

// Stubbed functions
// ------------------------------------------------------------------------------------------------


// Calculation of 8-bit CRC, used to compare result with the table lookup in the actual implementation
// in the CRC module.
uint8 CrcCalc(const uint8* message, uint32 nBytes, uint8 start, uint8 poly, uint8 finalXor)
{
    uint8  remainder = start;
    uint32 byte;
    uint8  bit;
    uint8  topbit = 0x80;


    // Perform modulo-2 division, a byte at a time.
    for (byte = 0; byte < nBytes; byte++) {
        // Bring the next byte into the remainder.
        remainder ^= message[byte];

        // Perform modulo-2 division, a bit at a time.
        for (bit = 8; bit > 0; bit--) {

            //Try to divide the current data bit.
            if (remainder & topbit) {
                remainder = (remainder << 1) ^ poly;
            }
            else {
                remainder = (remainder << 1);
            }
        }
    }


    return remainder ^ finalXor;
}


// Setup / Teardown
// ------------------------------------------------------------------------------------------------
static void Setup(void) {
}

static void Teardown(void) {
}

// Test functions
// ------------------------------------------------------------------------------------------------



/* @tnum CRC_001
 * @testname Crc_tests_VersionInfo
 * @desc Tests the version info functionality, it includes sw major, minor, patch version
 *       and vendor, modules ID's.
 * @treq SWS_CRC_00011
 * @treq SWS_CRC_00021
 */
void Crc_tests_VersionInfo() {

#if  (__GNUC__ == 4 && (__GNUC_MINOR__ > 5))
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Waddress"
#endif
    Std_VersionInfoType versioninfo;
    /*lint -save -e506 */
    Crc_GetVersionInfo(&versioninfo);
    /*lint -restore */
#if  (__GNUC__ == 4 && (__GNUC_MINOR__ > 5))
#pragma GCC diagnostic pop
#endif

    TEST_ASSERT(versioninfo.moduleID == 201u);
    TEST_ASSERT(versioninfo.vendorID == 60u);
    TEST_ASSERT(versioninfo.sw_major_version == 2);
    TEST_ASSERT(versioninfo.sw_minor_version == 0);
    TEST_ASSERT(versioninfo.sw_patch_version == 0);
}


/* @tnum CRC_002
 * @testname Crc_tests_NullPointer
 * @desc Tests using NULL pointer arguments
 *       case1:Pass *Crc_DataPtr = NULL to Crc_CalculateCRC8 function and expect return crc value = 0
 *       case2:Pass *Crc_DataPtr = NULL to Crc_CalculateCRC8H2F function and expect return crc value = 0
 *       case3:Pass *Crc_DataPtr = NULL to Crc_CalculateCRC16 function and expect return crc value = 0
 *       case4:Pass *Crc_DataPtr = NULL to Crc_CalculateCRC32 function and expect return crc value = 0
 *       case5:Pass *Crc_DataPtr = NULL to Crc_CalculateCRC32P4 function and expect return crc value = 0
 */
void Crc_tests_NullPointer() {
    uint8 crc = 0;
    Std_VersionInfoType* versioninfo;

    crc = Crc_CalculateCRC8(NULL, 30, 0, TRUE);
    TEST_ASSERT(crc == 0);

    crc = Crc_CalculateCRC8H2F(NULL, 30, 0, TRUE);
    TEST_ASSERT(crc == 0);

    crc = Crc_CalculateCRC16(NULL, 30, 0, TRUE);
    TEST_ASSERT(crc == 0);

    crc = Crc_CalculateCRC32(NULL, 30, 0, TRUE);
    TEST_ASSERT(crc == 0);

    crc = Crc_CalculateCRC32P4(NULL, 30, 0, TRUE);
    TEST_ASSERT(crc == 0);
    
    /*lint -save -e506 */
    versioninfo = NULL_PTR;
    Crc_GetVersionInfo(versioninfo);
    /*lint -restore */
}


/* @tnum CRC_003
 * @testname Crc_tests_CheckValueCRC8
 * @desc Tests check value for SAE-1850 on predefined data {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39}. Tests init CRC values.
 *       case 1: Calculate CRC on above data with Crc_IsFirstCall = TRUE and Crc_StartValue8 = 0 and expect crc == 0x4B
 *       case 2: Calculate CRC on above mentioned data with Crc_IsFirstCall = TRUE and Crc_StartValue8 = 14 to make sure start value
 *               is not used and expect crc == 0x4B
 *       case 3: Calculate CRC on above mentioned data with Crc_IsFirstCall = FALSE and Crc_StartValue8 = 0 to
 *               make sure check Value is 0xFF and expect crc == 0x4B
 *       case 4: Calculate CRC on above mentioned data with Crc_IsFirstCall = FALSE and Crc_StartValue8 = 0xFF this makes check value
 *               miss-match so expect crc != 0x4B
 *       case 5: Calculate CRC on above data with Crc_IsFirstCall = FALSE and Crc_StartValue8 = 0xEB and expect crc = 0x5F as calculated
 * @treq SWS_CRC_00014
 * @treq SWS_CRC_00030
 * @treq SWS_CRC_00031
 * @treq SWS_CRC_00032
 * @treq SWS_CRC_00041
 */
void Crc_tests_CheckCRC8() {

    uint8 checkData[] =  {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
    uint8 crc;

    /* Verify the check value */
    crc = Crc_CalculateCRC8(checkData, 9, 0, TRUE);
    TEST_ASSERT(crc == CRC8_CHECK_VALUE);

    /* Make sure the start value is not used when CRC_isFirstCall is TRUE */
    crc = Crc_CalculateCRC8(checkData, 9, 0x14, TRUE);
    TEST_ASSERT(crc == CRC8_CHECK_VALUE);

    /* Verify the check value when manually setting start value
     * Need to start with 0x00 since XOR:er in the call*/
    crc = Crc_CalculateCRC8(checkData, 9, 0x00, FALSE);
    TEST_ASSERT(crc == CRC8_CHECK_VALUE);

    /* Verify the check value mismatch when starting at another value */
    crc = Crc_CalculateCRC8(checkData, 9, 0xFF, FALSE);
    TEST_ASSERT(crc !=  CRC8_CHECK_VALUE);

    /* Verify CRC check with a different start value */
    crc = Crc_CalculateCRC8(checkData, 9, 0xEB , FALSE);
    TEST_ASSERT(crc == 0x5Fu); /* 0x5F has been calcuated */


}


/* @tnum CRC_004
 * @testname Crc_tests_MagicCheckCRC8
 * @desc Tests magic check value for SAE-1850.
 *       case 1: step 1: calculate CRC on data 00h 00h 00h 00h, start value FFh.
 *                       expect CRC-Result == 0x59
 *               step 2: calculate CRC on data 00h 00h 00h 00h 59h, start value FFh.
 *                       expect CRC-Result == 0x3B and expect magic check == 0xC4
 *       Case2 : Fault injection test
 *               Step 1: calculate CRC on data 00h 00h 00h 00h, start value FFh.
 *                       expect CRC-Result == 0x59
 *               Step 2: calculate CRC on data 00h 00h 00h 00h <59h+1>, start value FFh and XOR CRC-Result
 *                       with 0xFF and expect magic check !=  0xC4
 *       Case 3: Repeat step 1 and 2 of case 1 with different data block and expect same magic check == 0xC4
 * @treq SWS_CRC_00014
 * @treq SWS_CRC_00030
 * @treq SWS_CRC_00031
 * @treq SWS_CRC_00032
 * @treq SWS_CRC_00041
 */
void Crc_tests_MagicCheckCRC8() {

    uint8 checkData[] =  {0x00, 0x00, 0x00, 0x00, 0xFF};
    uint8 crc;

    /* Verify first step of magic check */
    crc = Crc_CalculateCRC8(checkData, 4, 0, TRUE);
    TEST_ASSERT(crc == 0x59);

    checkData[4] = crc;

    /* Verify the magic check value */
    crc = Crc_CalculateCRC8(checkData, 5, 0, TRUE);
    TEST_ASSERT((crc ^ 0xFF) == CRC8_MAGIC_CHECK_VALUE);

    /* Fault injection test */ /* Verify first step of magic check */
    crc = Crc_CalculateCRC8(checkData, 4, 0, TRUE);
    TEST_ASSERT(crc == 0x59);

    /* Modified CRC to introduce fault */
    checkData[4] = crc + 1;

    /* Verify the magic check value */
    crc = Crc_CalculateCRC8(checkData, 5, 0, TRUE);
    TEST_ASSERT((crc ^ 0xFF) != CRC8_MAGIC_CHECK_VALUE);

    /* Verify first step of magic check */
    crc = Crc_CalculateCRC8(checkData, 4, 0, TRUE);
    TEST_ASSERT(crc == 0x59);

    checkData[4] = crc;

    /* Verify the magic check value */
    crc = Crc_CalculateCRC8(checkData, 5, 0, TRUE);
    TEST_ASSERT((crc ^ 0xFF) == CRC8_MAGIC_CHECK_VALUE);

    /* Fault injection test */
    /* Verify first step of magic check */
    crc = Crc_CalculateCRC8(checkData, 4, 0, TRUE);
    TEST_ASSERT(crc == 0x59);

    /* Modified resulted CRC by adding one to introduce fault */
    checkData[4] = crc + 1;

    /* Verify the magic check value */
    crc = Crc_CalculateCRC8(checkData, 5, 0, TRUE);
    TEST_ASSERT((crc ^ 0xFF) != CRC8_MAGIC_CHECK_VALUE);

    for (uint8 i=0;i<200;i+=3) {

        checkData[1] = i;
        /* Verify first step of magic check */
        crc = Crc_CalculateCRC8(checkData, 4, 0, TRUE);

        checkData[4] = crc;

        /* Verify the magic check value */
        crc = Crc_CalculateCRC8(checkData, 5, 0, TRUE);
        TEST_ASSERT((crc ^ 0xFF) == CRC8_MAGIC_CHECK_VALUE);
    }

}


/* @tnum CRC_005
 * @testname Crc_tests_SpecDefinedCheckCRC8
 * @desc Tests from the 4.3.0 specification
 *       case 1: calculate CRC on data 00 00 00 00 and expect CRC-result == 0x59(Value defined in spec)
 *       case 2: calculate CRC on data F2 01 83 and expect CRC-result == 0x37
 *       case 3: Calculate CRC on data 0F AA 00 55 and expect CRC-result == 0x79
 *       case 4: Calculate CRC on data 00 FF 55 11 and expect CRC_result == 0xB8
 *       case 5: Calculate CRC on data 33 22 55 AA BB CC DD EE FF and CRC result == 0xCB
 *       case 6: Calculate CRC on data 92 6B 55 and expect CRC-Result == 0x8C
 *       case 7: Calculate CRC on data FF FF FF FF and expect CRC-Result == 0x74
 * @treq SWS_CRC_00014
 * @treq SWS_CRC_00030
 * @treq SWS_CRC_00031
 * @treq SWS_CRC_00032
 * @treq SWS_CRC_00041
 * @treq SWS_CRC_00052
 */
void Crc_tests_SpecDefinedCheckCRC8() {

    uint8 checkData1[] =  {0x00, 0x00, 0x00, 0x00};
    uint8 checkData2[] =  {0xF2, 0x01, 0x83};
    uint8 checkData3[] =  {0x0F, 0xAA, 0x00, 0x55};
    uint8 checkData4[] =  {0x00, 0xFF, 0x55, 0x11};
    uint8 checkData5[] =  {0x33, 0x22, 0x55, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    uint8 checkData6[] =  {0x92, 0x6B, 0x55};
    uint8 checkData7[] =  {0xFF, 0xFF, 0xFF, 0xFF};
    uint8 crc;

    crc = Crc_CalculateCRC8(checkData1, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0x59);  /* Value defined in spec */

    crc = Crc_CalculateCRC8(checkData2, 3, 0, TRUE);
    TEST_ASSERT( crc ==  0x37);  /* Value defined in spec */

    crc = Crc_CalculateCRC8(checkData3, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0x79);  /* Value defined in spec */

    crc = Crc_CalculateCRC8(checkData4, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0xB8);  /* Value defined in spec */

    crc = Crc_CalculateCRC8(checkData5, 9, 0, TRUE);
    TEST_ASSERT( crc ==  0xCB);  /* Value defined in spec */

    crc = Crc_CalculateCRC8(checkData6, 3, 0, TRUE);
    TEST_ASSERT( crc ==  0x8C);  /* Value defined in spec */

    crc = Crc_CalculateCRC8(checkData7, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0x74);  /* Value defined in spec */

}


/* @tnum CRC_006
 * @testname Crc_tests_RefValuesCRC8
 * @desc Tests that the SAE-1850 CRC implementation (table based) results in same result as alternative implementation in test
 *       step 1: Calculate CRC on specific data using Crc_CalculateCRC8 function
 *       Step 2: Calculate CRC on same data as above step1 but using Table created for test manually and compare the result.
 */
void Crc_tests_RefValuesCRC8() {

	uint8 i = 0;
	uint8 j = 0;
	uint8 crc = 0;
	uint8 crcRef = 0;

	for (i=0;i<NBR_REF_VALUES;i++) {
		for (j=0;j<REF_VALUE_LENGTH;j++) {
			crc = Crc_CalculateCRC8(Crc_u8RefValues[i], j, 0xFF, FALSE);
			crcRef = CrcCalc(Crc_u8RefValues[i], j, 0, 0x1D,  0xFF);
			TEST_ASSERT(crc == crcRef);
		}
	}
}


/* @tnum CRC_007
 * @testname Crc_tests_CheckCRC8H2F
 * @desc Tests check value for H2F on predefined data {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39}. Tests init CRC values.
 *       case 1: Calculate CRC on above mentioned data with Crc_IsFirstCall = TRUE
 *               and Crc_StartValue8 = 0 to make sure it uses Init start value = 0xFF and expect crc == 0xDF
 *       case 2: Calculate CRC on above mentioned data with Crc_IsFirstCall = TRUE
 *               and Crc_StartValue8 = 14 to make sure init start value is not used and expect crc == 0xDF
 *       case 3: Calculate CRC on above mentioned data with Crc_IsFirstCall = FALSE
 *               and Crc_StartValue8 = 0 to make sure start Value is 0xFF due to Xored with FF and expect crc == 0xDF
 *       case 4: Calculate CRC on above mentioned data with Crc_IsFirstCall = FALSE
 *               and Crc_StartValue8 = 0xFF this makes check value miss-match so expect crc != 0xDF
 *       case 5: Calculate CRC on above mentioned data with Crc_IsFirstCall = FALSE and Crc_StartValue8 = 0xEB and
 *               expect crc == 0xAB as calculated
 * @treq SWS_CRC_00014
 * @treq SWS_CRC_00041
 * @treq SWS_CRC_00042
 * @treq SWS_CRC_00043
 * @treq SWS_CRC_00044
 */
void Crc_tests_CheckCRC8H2F() {

    uint8 checkData[] =  {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
    uint8 crc;

    /* Verify the check value */
    crc = Crc_CalculateCRC8H2F(checkData, 9, 0, TRUE);
    TEST_ASSERT(crc == CRC8_H2F_CHECK_VALUE);

    /* Make sure the start value is not used when CRC_isFirstCall is TRUE */
    crc = Crc_CalculateCRC8H2F(checkData, 9, 0x14, TRUE);
    TEST_ASSERT(crc == CRC8_H2F_CHECK_VALUE);

    /* Verify the check value when manually setting start value
     * Must use 0x00 since value is XOR:ed in the call */
    crc = Crc_CalculateCRC8H2F(checkData, 9, 0x00, FALSE);
    TEST_ASSERT(crc == CRC8_H2F_CHECK_VALUE);

    /* Verify the check value mismatch when starting at another value */
    crc = Crc_CalculateCRC8H2F(checkData, 9, 0xFF, FALSE);
    TEST_ASSERT(crc !=  CRC8_H2F_CHECK_VALUE);

    /* Verify the CRC with a different start value  */
    crc = Crc_CalculateCRC8H2F(checkData, 9, 0xEB, FALSE);
    TEST_ASSERT(crc ==  0xABu); /* 0xAB has been calcuated */


}


/* @tnum CRC_008
 * @testname Crc_tests_MagicCheckCRC8H2F
 * @desc Tests magic check value for H2F.
 *       step 1: calculate CRC on data 00h 00h 00h 00h, start value FFh.
 *               and append calculated CRC as 5th byte in data at below step 2
 *       step 2: calculate CRC on data 00h 00h 00h 00h <CRC>, start value FFh.
 *               and then XOR CRC-Result with 0xFF and
 *               expect magic check value == 0x42
 *       step 3: Repeat step 1 and 2 for different data block(ex:- 00h <i>h 00h 00h) and
 *               expect same Magic check value == 0x42
 *       step 4: Fault injection test
 *               calculate CRC on data 00h 00h 00h 00h, start value FFh and append calculated <CRC+1>
 *               as 5th byte in above data to introduce fault and after calculation of CRC, XOR CRC-Result
 *               with 0xFF and expect Magic check != 0x42
 * @treq SWS_CRC_00014
 * @treq SWS_CRC_00041
 * @treq SWS_CRC_00042
 * @treq SWS_CRC_00043
 * @treq SWS_CRC_00044
 */
void Crc_tests_MagicCheckCRC8H2F() {

    uint8 checkData[] =  {0x00, 0x00, 0x00, 0x00, 0xFF};
    uint8 crc;

    for (uint8 i=0;i<200;i+=3) {

        checkData[1] =  i;

        /* Verify first step of magic check */
        crc = Crc_CalculateCRC8H2F(checkData, 4, 0, TRUE);
        checkData[4] = crc;

        /* Verify the magic check value */
        crc = Crc_CalculateCRC8H2F(checkData, 5, 0, TRUE);
        TEST_ASSERT((crc ^ 0xFF) == CRC8_H2F_MAGIC_CHECK_VALUE);
    }

    /* Fault Injection test */
    /* Verify first step of magic check */
    crc = Crc_CalculateCRC8H2F(checkData, 4, 0, TRUE);
    checkData[4] = (crc+1); /* Fault introduced */

    /* Verify the magic check value */
    crc = Crc_CalculateCRC8H2F(checkData, 5, 0, TRUE);
    TEST_ASSERT((crc ^ 0xFF) != CRC8_H2F_MAGIC_CHECK_VALUE);

}

/* @tnum CRC_009
 * @testname Crc_tests_SpecDefinedCheckCRC8H2F
 * @desc Tests from the 4.3.0 specification
 *       case 1: calculate CRC on data 00 00 00 00  and expect CRC-result == 0x12(Value defined in spec)
 *       case 2: calculate CRC on data F2 01 83 and expect CRC-result == 0xC2
 *       case 3: Calculate CRC on data 0F AA 00 55 and expect CRC-result == 0xC6
 *       case 4: Calculate CRC on data 00 FF 55 11 and expect CRC_result == 0x77
 *       case 5: Calculate CRC on data 33 22 55 AA BB CC DD EE FF and CRC result == 0x11
 *       case 6: Calculate CRC on data 92 6B 55 and expect CRC-Result == 0x33
 *       case 7: Calculate CRC on data FF FF FF FF and expect CRC-Result == 0x6C
 * @treq SWS_CRC_00014
 * @treq SWS_CRC_00041
 * @treq SWS_CRC_00042
 * @treq SWS_CRC_00043
 * @treq SWS_CRC_00044
 * @treq SWS_CRC_00053
 */
void Crc_tests_SpecDefinedCheckCRC8H2F() {

    uint8 checkData1[] =  {0x00, 0x00, 0x00, 0x00};
    uint8 checkData2[] =  {0xF2, 0x01, 0x83};
    uint8 checkData3[] =  {0x0F, 0xAA, 0x00, 0x55};
    uint8 checkData4[] =  {0x00, 0xFF, 0x55, 0x11};
    uint8 checkData5[] =  {0x33, 0x22, 0x55, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    uint8 checkData6[] =  {0x92, 0x6B, 0x55};
    uint8 checkData7[] =  {0xFF, 0xFF, 0xFF, 0xFF};

    uint8 crc;

    crc = Crc_CalculateCRC8H2F(checkData1, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0x12);  /* Value defined in spec */

    crc = Crc_CalculateCRC8H2F(checkData2, 3, 0, TRUE);
    TEST_ASSERT( crc ==  0xC2);  /* Value defined in spec */

    crc = Crc_CalculateCRC8H2F(checkData3, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0xC6);  /* Value defined in spec */

    crc = Crc_CalculateCRC8H2F(checkData4, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0x77);  /* Value defined in spec */

    crc = Crc_CalculateCRC8H2F(checkData5, 9, 0, TRUE);
    TEST_ASSERT( crc ==  0x11);  /* Value defined in spec */

    crc = Crc_CalculateCRC8H2F(checkData6, 3, 0, TRUE);
    TEST_ASSERT( crc ==  0x33);  /* Value defined in spec */

    crc = Crc_CalculateCRC8H2F(checkData7, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0x6C);  /* Value defined in spec */

}


/* @tnum CRC_010
 * @testname Crc_tests_RefValuesCRC8H2F
 * @desc Tests that the H2F CRC implementation (table based) results in same result as alternative implementation in test.
 *       step 1: Calculate CRC on specific data using Crc_CalculateCRC8H2F function
 *       Step 2: Calculate CRC on same data as above step1 but using Table created by manually for test and compare the results.
 */
void Crc_tests_RefValuesCRC8H2F() {

	uint8 i = 0;
	uint8 j = 0;
	uint8 crc = 0;
	uint8 crcRef = 0;

	for (i=0;i<NBR_REF_VALUES;i++) {
		for (j=0;j<REF_VALUE_LENGTH;j++) {
			crc = Crc_CalculateCRC8H2F(Crc_u8RefValues[i], j, 0xFF, FALSE);
			crcRef = CrcCalc(Crc_u8RefValues[i], j, 0, 0x2F,  0xFF);
			TEST_ASSERT(crc == crcRef);
		}
	}
}


/* @tnum CRC_011
 * @testname Crc_tests_CheckCRC16
 * @desc Tests check value for CRC16 on predefined data {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39}. Tests init CRC values.
 *       case 1: Calculate CRC on above mentioned data with Crc_IsFirstCall = FALSE and Crc_StartValue16 = FFFFh to make sure
 *               initial value is FFFFh after XORed with 0000h and expect crc == 0x29B1
 *       case 2: Calculate CRC on above mentioned data with Crc_IsFirstCall = FALSE
 *               and Crc_StartValue16 = FFF0h to make sure start value is not same as 0xFFFF and expect crc != 0x29B1
 *       case 3: Calculate CRC on above mentioned data with Crc_IsFirstCall = TRUE
 *               and Crc_StartValue16 = 0 to make sure start value is not used and expect crc == 0x29B1
 *       case 5: Calculate CRC on above mentioned data with Crc_IsFirstCall = FALSE
 *               and Crc_StartValue16 = 0x0014 other than start value and expect crc == 0x031A as calculated
 * @treq SWS_CRC_00002
 * @treq SWS_CRC_00014
 * @treq SWS_CRC_00015
 * @treq SWS_CRC_00019
 * @treq SWS_CRC_00041
 */
void Crc_tests_CheckCRC16() {

    uint8 checkData[] =  {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
    uint16 crc;

    crc = Crc_CalculateCRC16(checkData, 9, 0xFFFF, FALSE);
    TEST_ASSERT(crc == CRC16_CHECK_VALUE);

    crc = Crc_CalculateCRC16(checkData, 9, 0xFFF0, FALSE);
    TEST_ASSERT(crc !=  CRC16_CHECK_VALUE);

    crc = Crc_CalculateCRC16(checkData, 9, 0x0000, TRUE);
    TEST_ASSERT(crc == CRC16_CHECK_VALUE);

    /* Verify CRC with a different start value */
    crc = Crc_CalculateCRC16(checkData, 9, 0x0014, FALSE);
    TEST_ASSERT(crc == 0x031Au); /* 0x031A has been calcuated */


}


/* @tnum CRC_012
 * @testname Crc_tests_MagicCheckCRC16
 * @desc Tests magic check value for CRC16.
 *       step 1: calculate CRC on data 00h 00h 00h 00h, start value FFFFh.
 *               and append calculated CRC as 5th & 6th byte in data at below step 2
 *       step 2: calculate CRC on data 00h 00h 00h 00h <crc >> 8> <crc & 0xFF>, start value FFFFh.
 *               and expect magic check value == 0x0000
 *       step 3: Repeat step 1 and 2 for different data block(ex:- 00h <i>h 00h 00h) and
 *               expect same Magic check value == 0x0000
 *       step 4: Fault injection test
 *               calculate CRC on data 00h 00h 00h 00h, start value FFh and append calculated <CRC+1>
 *               as 5th byte in above data to introduce fault and after calculation of CRC, XOR CRC-Result
 *               with 0xFF and expect Magic check != 0x0000
 * @treq SWS_CRC_00002
 * @treq SWS_CRC_00014
 * @treq SWS_CRC_00015
 * @treq SWS_CRC_00019
 * @treq SWS_CRC_00041
 */
void Crc_tests_MagicCheckCRC16() {

    uint8 checkData[] =  {0x00, 0x00, 0x00, 0x00, 0x5A, 0xA5};
    uint16 crc;

    for (uint8 i=0;i<200;i+=3) {

        checkData[1] =  i;

        /* Verify first step of magic check */
        crc = Crc_CalculateCRC16(checkData, 4, 0, TRUE);
        checkData[4]  = (uint8)(crc >> 8);
        checkData[5]  = (uint8)(crc & 0xFF);

        /* Verify the magic check value */
        crc = Crc_CalculateCRC16(checkData, 6, 0, TRUE);
        TEST_ASSERT(crc == CRC16_MAGIC_CHECK_VALUE);
    }

    /* Fault injection Test */
    /* Verify first step of magic check */
    crc = Crc_CalculateCRC16(checkData, 4, 0, TRUE);
    checkData[4]  = (((uint8)(crc >> 8)) + 1); /* Fault introduced */
    checkData[5]  = (uint8)(crc & 0xFF);

    /* Verify the magic check value */
    crc = Crc_CalculateCRC16(checkData, 6, 0, TRUE);
    TEST_ASSERT(crc != CRC16_MAGIC_CHECK_VALUE);
}

/* @tnum CRC_013
 * @testname Crc_tests_SpecDefinedCheckCRC16
 * @desc Tests from the 4.3.0 specification
 *       case 1: calculate CRC on data 00 00 00 00  and expect CRC-result == 0x84C0(Value defined in spec)
 *       case 2: calculate CRC on data F2 01 83 and expect CRC-result == 0xD374
 *       case 3: Calculate CRC on data 0F AA 00 55 and expect CRC-result == 0x2023
 *       case 4: Calculate CRC on data 00 FF 55 11 and expect CRC_result == 0xB8F9
 *       case 5: Calculate CRC on data 33 22 55 AA BB CC DD EE FF and CRC result == 0xF53F
 *       case 6: Calculate CRC on data 92 6B 55 and expect CRC-Result == 0x0745
 *       case 7: Calculate CRC on data FF FF FF FF and expect CRC-Result == 0x1D0F
 * @treq SWS_CRC_00002
 * @treq SWS_CRC_00014
 * @treq SWS_CRC_00015
 * @treq SWS_CRC_00019
 * @treq SWS_CRC_00041
 * @treq SWS_CRC_00054
 */
void Crc_tests_SpecDefinedCheckCRC16() {

    uint8 checkData1[] =  {0x00, 0x00, 0x00, 0x00};
    uint8 checkData2[] =  {0xF2, 0x01, 0x83};
    uint8 checkData3[] =  {0x0F, 0xAA, 0x00, 0x55};
    uint8 checkData4[] =  {0x00, 0xFF, 0x55, 0x11};
    uint8 checkData5[] =  {0x33, 0x22, 0x55, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    uint8 checkData6[] =  {0x92, 0x6B, 0x55};
    uint8 checkData7[] =  {0xFF, 0xFF, 0xFF, 0xFF};

    uint16 crc;

    crc = Crc_CalculateCRC16(checkData1, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0x84C0);  /* Value defined in spec */

    crc = Crc_CalculateCRC16(checkData2, 3, 0, TRUE);
    TEST_ASSERT( crc ==  0xD374);  /* Value defined in spec */

    crc = Crc_CalculateCRC16(checkData3, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0x2023);  /* Value defined in spec */

    crc = Crc_CalculateCRC16(checkData4, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0xB8F9);  /* Value defined in spec */

    crc = Crc_CalculateCRC16(checkData5, 9, 0, TRUE);
    TEST_ASSERT( crc ==  0xF53F);  /* Value defined in spec */

    crc = Crc_CalculateCRC16(checkData6, 3, 0, TRUE);
    TEST_ASSERT( crc ==  0x0745);  /* Value defined in spec */

    crc = Crc_CalculateCRC16(checkData7, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0x1D0F);  /* Value defined in spec */

}


/* @tnum CRC_014
 * @testname Crc_tests_CheckCRC32
 * @desc Tests check value for CRC32 on predefined data {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39}. Tests init CRC values.
 *       case 1: Calculate CRC on above mentioned data with Crc_IsFirstCall = FALSE and Crc_StartValue32 = 00000000h to make sure
 *               initial value is FFFFFFFFh after XORed with 00h and expect crc == 0xCBF43926
 *       case 2: Calculate CRC on above mentioned data with Crc_IsFirstCall = FALSE and Crc_StartValue32 = 0xFFFFFFFF
 *               to make sure initial start value is not same as 0xFFFFFFFF and expect crc != 0xCBF43926
 *       case 3: Calculate CRC on above mentioned data with Crc_IsFirstCall = TRUE and Crc_StartValue32 = 0xFFFFFFFF
 *               and expect crc == 0xCBF43926
 *       case 4: Calculate CRC on above mentioned data with Crc_IsFirstCall = FALSE
 *               and Crc_StartValue32 = 0x00000000 other than start value and expect crc == 0xCBF43926
 * @treq SWS_CRC_00003
 * @treq SWS_CRC_00014
 * @treq SWS_CRC_00016
 * @treq SWS_CRC_00020
 * @treq SWS_CRC_00041
 */
void Crc_tests_CheckCRC32() {
    uint8 checkData[] =  {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
    uint32 crc;

    /* Must use 0 since XOR:ed in the call */
    crc = Crc_CalculateCRC32(checkData, 9, 0x00000000, FALSE);
    TEST_ASSERT(crc == CRC32_CHECK_VALUE);

    crc = Crc_CalculateCRC32(checkData, 9, 0xFFFFFFFF, FALSE);
    TEST_ASSERT(crc != CRC32_CHECK_VALUE);

    crc = Crc_CalculateCRC32(checkData, 9, 0xFFFFFFFF, TRUE);
    TEST_ASSERT(crc == CRC32_CHECK_VALUE);

    crc = Crc_CalculateCRC32(checkData, 9, 0x00000000, TRUE);
    TEST_ASSERT(crc ==  CRC32_CHECK_VALUE);


}


/* @tnum CRC_015
 * @testname Crc_tests_MagicCheckCRC32
 * @desc Tests magic check value for CRC32.
 *       step 1: calculate CRC on data 00h 00h 00h 00h, start value FFFFFFFFh.
 *               and append calculated CRC as 5th,6th,7th and 8th byte in data at below step 2
 *       step 2: calculate CRC on data 00h 00h 00h 00h <CRC-LSB><CRC><CRC><CRC-MSB>, start value FFFFFFFFh.
 *               and expect magic check == 0xDEBB20E3
 *       step 3: Repeat step 1 and 2 for different data block(ex:- 00h <i>h 00h 00h) and
 *               expect same Magic check == 0xDEBB20E3
 *       step 4: Fault injection test
 *               calculate CRC on data 00h 00h 00h 00h, start value FFh and append calculated <CRC+1>
 *               as 5th byte in above data to introduce fault and after calculation of CRC, XOR CRC-Result
 *               with 0xFF and expect Magic check != 0xDEBB20E3
 * @treq SWS_CRC_00003
 * @treq SWS_CRC_00014
 * @treq SWS_CRC_00016
 * @treq SWS_CRC_00020
 * @treq SWS_CRC_00041
 */
void Crc_tests_MagicCheckCRC32() {

    uint8 checkData[] =  {0x00, 0x00, 0x00, 0x00, 0x5A, 0xA5, 0xFF, 0xFF};
    uint32 crc;

    for (uint8 i=0;i<200;i+=3) {

        checkData[0] =  i;
        checkData[1] =  i + 5;
        checkData[2] =  i + 10;
        checkData[3] =  i + 13;

        /* Verify first step of magic check */
        crc = Crc_CalculateCRC32(checkData, 4, 0, TRUE);
        checkData[4]  = (uint8)(crc & 0xFF);
        checkData[5]  = (uint8)((crc >> 8) & 0xFF);
        checkData[6]  = (uint8)((crc >> 16) & 0xFF);
        checkData[7]  = (uint8)((crc >> 24) & 0xFF);

        /* Verify the magic check value */
        crc = Crc_CalculateCRC32(checkData, 8, 0, TRUE);
        TEST_ASSERT((crc ^ 0xFFFFFFFF) ==  CRC32_MAGIC_CHECK_VALUE);
    }

    /* Fault injection test */
    /* Verify first step of magic check */
    crc = Crc_CalculateCRC32(checkData, 4, 0, TRUE);
    checkData[4]  = (((uint8)(crc & 0xFF)) + 1); /* Fault introduced by adding 1 */
    checkData[5]  = (uint8)((crc >> 8) & 0xFF);
    checkData[6]  = (uint8)((crc >> 16) & 0xFF);
    checkData[7]  = (uint8)((crc >> 24) & 0xFF);

    /* Verify the magic check value */
    crc = Crc_CalculateCRC32(checkData, 8, 0, TRUE);
    TEST_ASSERT((crc ^ 0xFFFFFFFF) !=  CRC32_MAGIC_CHECK_VALUE);

}


/* @tnum CRC_016
 * @testname Crc_tests_SpecDefinedCheckCRC32
 * @desc Tests from the 4.3.0 specification
 *       case 1: calculate CRC on data 00 00 00 00  and expect CRC-result == 0x2144DF1C
 *       case 2: calculate CRC on data F2 01 83 and expect CRC-result == 0x24AB9D77
 *       case 3: Calculate CRC on data 0F AA 00 55 and expect CRC-result == 0xB6C9B287
 *       case 4: Calculate CRC on data 00 FF 55 11 and expect CRC_result == 0x32A06212
 *       case 5: Calculate CRC on data 33 22 55 AA BB CC DD EE FF and CRC result == 0xB0AE863D
 *       case 6: Calculate CRC on data 92 6B 55 and expect CRC-Result == 0x9CDEA29B
 *       case 7: Calculate CRC on data FF FF FF FF and expect CRC-Result == 0xFFFFFFFF
 *       case 8: Calculate CRC on data ranging 1..2..to 255 with Crc_IsFirstCall == TRUE
 *               and Crc_StartValue32 = 00000000h and expect CRC-Result == 0x29058c73 as calculated
 *       case 9: Calculate CRC on data ranging 1..2..to 255 with Crc_IsFirstCall = FALSE
 *               and Crc_StartValue32 = FFFFFFFFh and expect CRC-Result == 0xdb6cf6d4 as calculated
 *       case 10: Calculate CRC on data ranging 1..2..to 255 with Crc_IsFirstCall = FALSE
 *               and Crc_StartValue32 = FFFFFFA5h and expect CRC-Result == 0x81b831c8 as calculated
 *       case 11: Calculate CRC on data ranging 1..2..to 8 with Crc_IsFirstCall = TRUE
 *               and Crc_StartValue32 = 00h and expect CRC-Result == 0x3FCA88C5
 *       case 12: Step 1:- Calculate CRC on data ranging 1..2..3..4 with Crc_IsFirstCall = TRUE
 *                         and Crc_StartValue32 = 00h and expect CRC-Result == 0xB63CFBCD
 *                Step 2:- Calculate CRC on data 5..6..7..8 with Crc_IsFirstCall = TRUE and
 *                         Crc_StartValue32 = calculated CRC at Step1(0xB63CFBCD) and expect CRC result == 0x3FCA88C5
 * @treq SWS_CRC_00003
 * @treq SWS_CRC_00014
 * @treq SWS_CRC_00016
 * @treq SWS_CRC_00020
 * @treq SWS_CRC_00041
 * @treq SWS_CRC_00055
 */
void Crc_tests_SpecDefinedCheckCRC32() {

    uint8 checkData1[] =  {0x00, 0x00, 0x00, 0x00};
    uint8 checkData2[] =  {0xF2, 0x01, 0x83};
    uint8 checkData3[] =  {0x0F, 0xAA, 0x00, 0x55};
    uint8 checkData4[] =  {0x00, 0xFF, 0x55, 0x11};
    uint8 checkData5[] =  {0x33, 0x22, 0x55, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    uint8 checkData6[] =  {0x92, 0x6B, 0x55};
    uint8 checkData7[] =  {0xFF, 0xFF, 0xFF, 0xFF};

    uint8 checkData8[] =  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                           17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
                           33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
                           49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
                           65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
                           81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
                           97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112,
                           113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128,
                           129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144,
                           145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160,
                           161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176,
                           177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192,
                           193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208,
                           209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224,
                           225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240,
                           241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255};

    uint8 checkData9[] =  {1, 2, 3, 4, 5, 6, 7, 8};

    uint32 crc;

    crc = Crc_CalculateCRC32(checkData1, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0x2144DF1C);  /* Value defined in spec */

    crc = Crc_CalculateCRC32(checkData2, 3, 0, TRUE);
    TEST_ASSERT( crc ==  0x24AB9D77);  /* Value defined in spec */

    crc = Crc_CalculateCRC32(checkData3, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0xB6C9B287);  /* Value defined in spec */

    crc = Crc_CalculateCRC32(checkData4, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0x32A06212);  /* Value defined in spec */

    crc = Crc_CalculateCRC32(checkData5, 9, 0, TRUE);
    TEST_ASSERT( crc ==  0xB0AE863D);  /* Value defined in spec */

    crc = Crc_CalculateCRC32(checkData6, 3, 0, TRUE);
    TEST_ASSERT( crc ==  0x9CDEA29B);  /* Value defined in spec */

    crc = Crc_CalculateCRC32(checkData7, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0xFFFFFFFF);  /* Value defined in spec */

    crc = Crc_CalculateCRC32(checkData8, 256, 0, TRUE);
    TEST_ASSERT( crc ==  0x29058c73);  /* Value calculated */

    crc = Crc_CalculateCRC32(checkData8, 256, 0xFFFFFFFF, FALSE);
    TEST_ASSERT( crc ==  0xdb6cf6d4);  /* Value calculated */

    crc = Crc_CalculateCRC32(checkData8, 256, 0xFFFFFFA5, FALSE);
    TEST_ASSERT( crc ==  0x81b831c8);  /* Value calculated */

    crc = Crc_CalculateCRC32(checkData9, 8, 0, TRUE);
    TEST_ASSERT( crc ==  0x3FCA88C5);  /* Value defined in spec */

    crc = Crc_CalculateCRC32(checkData9, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0xB63CFBCD);  /* Value defined in spec */

    crc = Crc_CalculateCRC32(&checkData9[4], 4, crc, FALSE);
    TEST_ASSERT( crc ==  0x3FCA88C5);  /* Value defined in spec */
}

/* @tnum CRC_017
 * @testname Crc_tests_CheckCRC32P4
 * @desc Tests check value for CRC32P4 on predefined data {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39}. Tests init CRC values .
 *       case 1: Calculate CRC on above mentioned data with Crc_IsFirstCall = FALSE and Crc_StartValue32 = 00000000h
 *               to make sure initial value is FFFFFFFFh after XORed with 00h and expect crc == 0xCBF43926
 *       case 2: Calculate CRC on above mentioned data with Crc_IsFirstCall = FALSE
 *               and Crc_StartValue32 = 0xFFFFFFFF to make sure start value is not same as 0xFFFFFFFF and expect crc != 0xCBF43926
 *       case 3: Calculate CRC on above mentioned data with Crc_IsFirstCall = TRUE and Crc_StartValue32 = 0xFFFFFFFF
 *               and expect crc == 0xCBF43926
 *       case 4: Calculate CRC on above mentioned data with Crc_IsFirstCall = FALSE
 *               and Crc_StartValue32 = 0x00000000 other than start value and expect crc == 0xCBF43926
 * @treq SWS_CRC_00014
 * @treq SWS_CRC_00041
 * @treq SWS_CRC_00056
 * @treq SWS_CRC_00058
 * @treq SWS_CRC_00059
 */
void Crc_tests_CheckCRC32P4() {
    uint8 checkData[] =  {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
    uint32 crc;

    /* Must use 0 since XOR:ed in the call */
    crc = Crc_CalculateCRC32P4(checkData, 9, 0x00000000, FALSE);
    TEST_ASSERT(crc == CRC32P4_CHECK_VALUE);

    crc = Crc_CalculateCRC32P4(checkData, 9, 0xFFFFFFFF, FALSE);
    TEST_ASSERT(crc != CRC32P4_CHECK_VALUE);

    crc = Crc_CalculateCRC32P4(checkData, 9, 0xFFFFFFFF, TRUE);
    TEST_ASSERT(crc == CRC32P4_CHECK_VALUE);

    crc = Crc_CalculateCRC32P4(checkData, 9, 0x00000000, TRUE);
    TEST_ASSERT(crc ==  CRC32P4_CHECK_VALUE);


}


/* @tnum CRC_018
 * @testname Crc_tests_MagicCheckCRC32P4
 * @desc Tests magic check value for CRC32P4.
 *       step 1: calculate CRC on data 00h 00h 00h 00h, start value FFFFFFFFh.
 *               and append calculated CRC as 5th,6th,7th and 8th byte in data at below step 2
 *       step 2: calculate CRC on data 00h 00h 00h 00h <CRC-LSB><CRC><CRC><CRC-MSB>, start value FFFFFFFFh.
 *               and expect magic check == 0x904CDDBF
 *       step 3: Repeat step 1 and 2 for different data block(ex:- 00h <i>h 00h 00h) and
 *               expect same Magic check == 0x904CDDBF
 *       step 4: Fault injection test
 *               calculate CRC on data 00h 00h 00h 00h, start value FFh and append calculated <CRC+1>
 *               as 5th byte in above data to introduce fault and after calculation of CRC, XOR CRC-Result
 *               with 0xFF and expect Magic check != 0x904CDDBF
 * @treq SWS_CRC_00014
 * @treq SWS_CRC_00041
 * @treq SWS_CRC_00056
 * @treq SWS_CRC_00058
 * @treq SWS_CRC_00059
 */
void Crc_tests_MagicCheckCRC32P4() {

    uint8 checkData[] =  {0x00, 0x00, 0x00, 0x00, 0x5A, 0xA5, 0xFF, 0xFF};
    uint32 crc;

    for (uint8 i=0;i<200;i+=3) {

        checkData[0] =  i;
        checkData[1] =  i + 5;
        checkData[2] =  i + 10;
        checkData[3] =  i + 13;

        /* Verify first step of magic check */
        crc = Crc_CalculateCRC32P4(checkData, 4, 0, TRUE);
        checkData[4]  = (uint8)(crc & 0xFF);
        checkData[5]  = (uint8)((crc >> 8) & 0xFF);
        checkData[6]  = (uint8)((crc >> 16) & 0xFF);
        checkData[7]  = (uint8)((crc >> 24) & 0xFF);

        /* Verify the magic check value */
        crc = Crc_CalculateCRC32P4(checkData, 8, 0, TRUE);
        TEST_ASSERT((crc ^ 0xFFFFFFFF) ==  CRC32P4_MAGIC_CHECK_VALUE);
    }

    /* Fault injection test */
    /* Verify first step of magic check */
    crc = Crc_CalculateCRC32P4(checkData, 4, 0, TRUE);
    checkData[4]  = (((uint8)(crc & 0xFF)) + 1); /* Fault introduced by adding 1 */
    checkData[5]  = (uint8)((crc >> 8) & 0xFF);
    checkData[6]  = (uint8)((crc >> 16) & 0xFF);
    checkData[7]  = (uint8)((crc >> 24) & 0xFF);

    /* Verify the magic check value */
    crc = Crc_CalculateCRC32P4(checkData, 8, 0, TRUE);
    TEST_ASSERT((crc ^ 0xFFFFFFFF) !=  CRC32P4_MAGIC_CHECK_VALUE);

}


/* @tnum CRC_019
 * @testname Crc_tests_SpecDefinedCheckCRC32P4
 * @desc Tests from the 4.3.0 specification
 *       case 1: calculate CRC on data 00 00 00 00  and expect CRC-result == 0x6FB32240
 *       case 2: calculate CRC on data F2 01 83 and expect CRC-result == 0x4F721A25
 *       case 3: Calculate CRC on data 0F AA 00 55 and expect CRC-result == 0x20662DF8
 *       case 4: Calculate CRC on data 00 FF 55 11 and expect CRC_result == 0x9BD7996E
 *       case 5: Calculate CRC on data 33 22 55 AA BB CC DD EE FF and CRC result == 0xA65A343D
 *       case 6: Calculate CRC on data 92 6B 55 and expect CRC-Result == 0xEE688A78
 *       case 7: Calculate CRC on data FF FF FF FF and expect CRC-Result == 0xFFFFFFFF
 * @treq SWS_CRC_00014
 * @treq SWS_CRC_00041
 * @treq SWS_CRC_00056
 * @treq SWS_CRC_00057
 * @treq SWS_CRC_00058
 * @treq SWS_CRC_00059
 */
void Crc_tests_SpecDefinedCheckCRC32P4() {

    uint8 checkData1[] =  {0x00, 0x00, 0x00, 0x00};
    uint8 checkData2[] =  {0xF2, 0x01, 0x83};
    uint8 checkData3[] =  {0x0F, 0xAA, 0x00, 0x55};
    uint8 checkData4[] =  {0x00, 0xFF, 0x55, 0x11};
    uint8 checkData5[] =  {0x33, 0x22, 0x55, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    uint8 checkData6[] =  {0x92, 0x6B, 0x55};
    uint8 checkData7[] =  {0xFF, 0xFF, 0xFF, 0xFF};
    uint32 crc;

    crc = Crc_CalculateCRC32P4(checkData1, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0x6FB32240);  /* Value defined in spec */

    crc = Crc_CalculateCRC32P4(checkData2, 3, 0, TRUE);
    TEST_ASSERT( crc ==  0x4F721A25);  /* Value defined in spec */

    crc = Crc_CalculateCRC32P4(checkData3, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0x20662DF8);  /* Value defined in spec */

    crc = Crc_CalculateCRC32P4(checkData4, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0x9BD7996E);  /* Value defined in spec */

    crc = Crc_CalculateCRC32P4(checkData5, 9, 0, TRUE);
    TEST_ASSERT( crc ==  0xA65A343D);  /* Value defined in spec */

    crc = Crc_CalculateCRC32P4(checkData6, 3, 0, TRUE);
    TEST_ASSERT( crc ==  0xEE688A78);  /* Value defined in spec */

    crc = Crc_CalculateCRC32P4(checkData7, 4, 0, TRUE);
    TEST_ASSERT( crc ==  0xFFFFFFFF);  /* Value defined in spec */

}

/* @tnum CRC_020
 * @testname Test case provided by vendor
 * @desc Test case to check CRC on Specified data 0F AA 00 55
 *       case 1: Calculated CRC on data 0F AA 00 55 with Crc_IsFirstCall = TRUE
 *               and expect CRC result == 0xB6C9B287
 *       case 2: Calculate CRC by passing 0u as data and length as zero with Crc_IsFirstCall = TRUE and
 *               append resulted CRC as Start value with Crc_IsFirstCall = FALSE for data 0F AA 00 55 and
 *               expect CRC result == 0xB6C9B287
 *       case 3: Calculate CRC by passing 0u as data and length as zero with Crc_IsFirstCall = TRUE and
 *               append resulted CRC as Start value with Crc_IsFirstCall = FALSE for single byte data each time and
 *               expect final CRC result == 0xB6C9B287
 */
void Crc_tests_VendorDefinedCheckCRC32() {

    /* 0F AA 00 55    B6C9B287 */
    const uint8 test1[4]={0x0F,0xAA,0x00,0x55};
    uint32 crcValue1;
    uint32 crcValue2_tmp;
    uint32 crcValue2;

    uint32 crcValue3_tmp;
    uint32 crcValue3;

    crcValue1 = Crc_CalculateCRC32 ( &test1[0],4u, 0u, TRUE);

    crcValue2_tmp = Crc_CalculateCRC32 ( 0u,0u, 0u, TRUE);
    crcValue2     = Crc_CalculateCRC32 ( &test1[0],4u, crcValue2_tmp, FALSE);

    crcValue3_tmp = Crc_CalculateCRC32 ( 0u,0u, 0u, TRUE);
    crcValue3_tmp = Crc_CalculateCRC32 ( &test1[0],1u, crcValue3_tmp, FALSE);
    crcValue3_tmp = Crc_CalculateCRC32 ( &test1[1],1u, crcValue3_tmp, FALSE);
    crcValue3_tmp = Crc_CalculateCRC32 ( &test1[2],1u, crcValue3_tmp, FALSE);
    crcValue3     = Crc_CalculateCRC32 ( &test1[3],1u, crcValue3_tmp, FALSE);

    TEST_ASSERT(crcValue1 == 0xB6C9B287);
    TEST_ASSERT(crcValue2 == 0xB6C9B287);
    TEST_ASSERT(crcValue3 == 0xB6C9B287);
}



// Test composition
// ----------------

TestRef Crc_tests(void) {

    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture("CRC_001", Crc_tests_VersionInfo),
        new_TestFixture("CRC_002", Crc_tests_NullPointer),
        new_TestFixture("CRC_003", Crc_tests_CheckCRC8),
        new_TestFixture("CRC_004", Crc_tests_RefValuesCRC8),
        new_TestFixture("CRC_005", Crc_tests_MagicCheckCRC8),
        new_TestFixture("CRC_006", Crc_tests_SpecDefinedCheckCRC8),
        new_TestFixture("CRC_007", Crc_tests_CheckCRC8H2F),
        new_TestFixture("CRC_008", Crc_tests_MagicCheckCRC8H2F),
        new_TestFixture("CRC_009", Crc_tests_SpecDefinedCheckCRC8H2F),
        new_TestFixture("CRC_010", Crc_tests_RefValuesCRC8H2F),
        new_TestFixture("CRC_011", Crc_tests_CheckCRC16),
        new_TestFixture("CRC_012", Crc_tests_MagicCheckCRC16),
        new_TestFixture("CRC_013", Crc_tests_SpecDefinedCheckCRC16),
        new_TestFixture("CRC_014", Crc_tests_CheckCRC32),
        new_TestFixture("CRC_015", Crc_tests_MagicCheckCRC32),
        new_TestFixture("CRC_016", Crc_tests_SpecDefinedCheckCRC32),
        new_TestFixture("CRC_017", Crc_tests_CheckCRC32P4),
        new_TestFixture("CRC_018", Crc_tests_MagicCheckCRC32P4),
        new_TestFixture("CRC_019", Crc_tests_SpecDefinedCheckCRC32P4),
        new_TestFixture("CRC_020", Crc_tests_VendorDefinedCheckCRC32),
    };

	EMB_UNIT_TESTCALLER(CrcTests, "CrcTests", Setup, Teardown, fixtures);

	return (TestRef)&CrcTests;
}

