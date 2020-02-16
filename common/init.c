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
#if !defined(CFG_BRD_MPC5XXX_CC)
#include "Mcu.h"
#endif

/* ----------------------------[private define]------------------------------*/
#define TEST_DATA  0x12345
#define TEST_SDATA2	0x3344

#define BAD_LINK_FILE() 	while(1) {}

/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
volatile uint32_t test_data = TEST_DATA;
volatile uint32_t test_bss = 0;
volatile uint32_t test_bss_array[3];
volatile uint32_t test_data_array[3] = {TEST_DATA,TEST_DATA,TEST_DATA};


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
extern char __DATA_ROM[];
extern char __DATA_RAM[];
extern char __DATA_END[];
extern char __BSS_START[];
extern char __BSS_END[];

#if defined(__DCC__)
extern void __init( void );
#endif


/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/


void noooo( void ) {
	while(1);
}



/**
 * NOTE
 * This function will be called before BSS and DATA are initialized.
 * Ensure that you do not access any global or static variables before
 * BSS and DATA is initialized
 */
void init(void) {

#if defined(USE_TTY_WINIDEA)
	/* Store winIDEA terminal connected status */
	extern volatile char g_TConn;
	char g_TconTemp = g_TConn;
#endif

#if !defined(CFG_BRD_MPC5XXX_CC)
	McuE_InitZero();
#endif

	memcpy(__DATA_RAM, __DATA_ROM, __DATA_END - __DATA_RAM);
	memset(__BSS_START, 0, __BSS_END - __BSS_START);

#if defined(USE_TTY_WINIDEA)
	/* Restore winIDEA terminal connected status */
	g_TConn = g_TconTemp;
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

}



