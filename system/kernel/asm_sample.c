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


