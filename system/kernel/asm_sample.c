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
 * Just a sample file to generate some assembler constructs if you are
 * really bad at it.
 *
 * Add it to makefile and then examin it with objdump.
 *
 * Example:
 * ${CROSS_COMPILE}objdump -S system/kernel/obj_et_stm32_stamp/asm_sample.o
 *
 */


#include "Os.h"

void apa(void *);

void interrupt(void) __attribute__((__interrupt__));

void interrupt( void ) {

//	apa((void *)0x123);
}


void func1( int a ) {
	(void)a;
}

uint32_t func3( int a ) {
	if( a == 0) {
		return 5;
	}
	return (*(uint32_t *)0xE000ED04 ) & 0x1f7;
}

int func2( void ) {
	int a;
	a = 3;

	func1(5);
	func3(0);

	return 2;
}


void func( void ) {
	func2();
}


