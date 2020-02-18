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
 * init.c
 *
 *  Created on: 12 dec 2013
 *      Author: mahi
 */

/* ----------------------------[includes]------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#if !defined(CFG_BRD_MPC5XXX_CC)
#include "Mcu.h"
#endif
#if defined(__ARMCC_VERSION)
#include "rt_heap.h"
#endif

/* ----------------------------[private define]------------------------------*/
#define TEST_DATA  0x12345
#define TEST_SDATA2	0x3344

#define BAD_LINK_FILE() /*lint -save -e527 */ 	for(;;) {}		/*lint -restore */

/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
//lint -e9003 MISRA 2012 8.9 (adv)   For validation purposes, variables are defined out of block scope
//lint -e729 For validation purposes, data are not explicitly initialized
//lint -e843 For validation purposes, data not to be declared const
//lint -e970 Allow basic types (like int and char) in init file
//lint -e752 Allow not referenced symbols init file
volatile uint32 test_data = TEST_DATA;
volatile uint32 test_bss = 0;
volatile uint32 test_bss_array[3];
volatile uint32 test_data_array[3] = { TEST_DATA, TEST_DATA, TEST_DATA };

/*
 * Small data validation (PPC specific)
 */
/* Note! It does not matter if the data is initialized to 0, it still sbss2 */
volatile const int test_sbss2;
/* Initialized small data */
volatile const int test_sdata2 = TEST_SDATA2;

/*
 * Linker variables
 */
#if defined(__ARMCC_VERSION)
int errno = 0;
volatile int *__aeabi_errno_addr(void);

/* ARMCC scatter loading symbols */
extern int Load$$EXEC_ROM$$RW$$Base[];
extern char Image$$SRAM$$RW$$Base[];
extern char Image$$SRAM$$RW$$Limit[];
extern char Image$$SRAM$$ZI$$Base[];
extern char Image$$SRAM$$ZI$$Limit[];
extern char Image$$APP$$ZI$$Base[];
extern char Image$$APP$$ZI$$Limit[];
extern char Image$$ARM_LIB_HEAP$$ZI$$Base[];
extern char Image$$ARM_LIB_HEAP$$ZI$$Limit[];
#else
extern char __DATA_ROM[];
extern char __DATA_RAM[];
extern char __DATA_END[];
extern char __BSS_START[];
extern char __BSS_END[];
#if defined(CFG_ZYNQ) || defined (CFG_ARM_CR4) || defined (CFG_ARM_CR5)
#if defined(__GNUC__) || defined(__ghs__)
extern char _sdata[];
extern char _edata[];
extern char _sbss[];
extern char _ebss[];
extern char _sidata[];
#endif
#endif
#endif

#if defined(__DCC__)
extern void __init( void );
extern void _dcc_exit( void );
#endif

#if defined(CFG_BRD_MPC5XXX_CC)
extern void Mcu_Arc_InitZero( void );
#endif


/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/

//lint -esym(957, init)  MISRA 2004 8.1 (req) init is called from assembly. Allow definition without prototype.
//lint -e946 -e947  MISRA 2004 17.2 (req), 17.3 (req) MISRA 2012 18.2 (req), 18.3 (req)
//    Allow subtract operator and relational operators applied to pointers used in init function
/**
 * NOTE
 * This function will be called before BSS and DATA are initialized.
 * Ensure that you do not access any global or static variables before
 * BSS and DATA is initialized
 */
void init(void) {

#if !defined(CFG_BRD_MPC5XXX_CC)
    Mcu_Arc_InitZero();
#endif

#if defined(CFG_ZYNQ) || defined (CFG_ARM_CR4) || defined (CFG_ARM_CR5)
#if defined(__GNUC__) || defined(__ghs__)

    memcpy(_sdata, _sidata, _edata - _sdata);
    memset(_sbss, 0, _ebss - _sbss);
#elif defined(__ARMCC_VERSION)
#if 0
    memcpy(Image$$SRAM$$RW$$Base, Load$$EXEC_ROM$$RW$$Base, Image$$SRAM$$RW$$Limit - Image$$SRAM$$RW$$Base);
#endif
    memset(Image$$SRAM$$ZI$$Base, 0, Image$$SRAM$$ZI$$Limit - Image$$SRAM$$ZI$$Base);
    memset(Image$$APP$$ZI$$Base, 0, Image$$APP$$ZI$$Limit - Image$$APP$$ZI$$Base);

#endif
#elif defined(CFG_AURIX)
    /* Already done */
#else
    memcpy(__DATA_RAM, __DATA_ROM, __DATA_END - __DATA_RAM); //lint !e732 Allow loss of sign. Expected positive.
    memset(__BSS_START, 0, __BSS_END - __BSS_START); //lint !e732 Allow loss of sign. Expected positive.
#endif

#if defined(__DCC__)
    atexit(_dcc_exit);
#endif
#if defined(__ARMCC_VERSION)
    errno = *__aeabi_errno_addr();
    /* Setup Heap */
    _init_alloc((unsigned)Image$$ARM_LIB_HEAP$$ZI$$Base,(unsigned)Image$$ARM_LIB_HEAP$$ZI$$Limit);
#endif

#if defined(USE_TTY_WINIDEA)
    /* Autoconnect WINIDEA */
    extern volatile char g_TConn;
    g_TConn = (char)0xff;
#endif


#if defined(CFG_SHARED_BSS_DATA)
    extern uint8 __OS_START_SEC_shared_bss_shared[];
    extern uint8 __OS_STOP_SEC_shared_bss_shared[];
    extern uint8 __OS_START_SEC_shared_data_shared[];
    extern uint8 __OS_STOP_SEC_shared_data_shared[];
    extern uint8 __SHARED_DATA_ROM[];

    memcpy(__OS_START_SEC_shared_data_shared, __SHARED_DATA_ROM, __OS_STOP_SEC_shared_data_shared - __OS_START_SEC_shared_data_shared); //lint !e732 Allow loss of sign. Expected positive.
    memset(__OS_START_SEC_shared_bss_shared, 0, __OS_STOP_SEC_shared_bss_shared - __OS_START_SEC_shared_bss_shared ); //lint !e732 Allow loss of sign. Expected positive.
#endif

    /* Check link file */

    /* .data */
    for (int i = 0; i < 3; i++) {
        if (test_data_array[i] != TEST_DATA) {
            BAD_LINK_FILE();
        }
    }

    /* .sdata */
    if (TEST_DATA != test_data) {
        BAD_LINK_FILE();
    }

    /* .bss */
    for (int i = 0; i < 3; i++) {
        if (test_bss_array[i] != 0) {
            BAD_LINK_FILE();
        }
    }

    /* .sbss */
    if (test_bss != 0) {
        BAD_LINK_FILE();
    }

    /* check .sdata2 (PPC)*/
    if (test_sdata2 != TEST_SDATA2) {
        BAD_LINK_FILE();
    }

    /* check .sbss (PPC)*/
    if (test_sbss2 != 0) {
        BAD_LINK_FILE();
    }

#if defined(__DCC__)
    /* Runtime init */
    __init();
#endif

#if defined(CFG_TMS570)
    extern void _init(void);
#if defined(CFG_GCOV)
    _init();
#endif
#endif
}

