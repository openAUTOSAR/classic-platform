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

#include <stdlib.h>
/**
 * K & R itoa
 */

void strreverse(char* begin, char* end) {

	char c;

	while (end > begin) {
		c = *end;
		*end-- = *begin;
		*begin++ = c;
	}
}

/**
 * Convert a number to a string
 *
 * @param val		The value to convert
 * @param str		Pointer to a space where to put the string
 * @param base		The base
 * @param negative	If negative or not.
 */
void xtoa( unsigned long val, char* str, int base, int negative) {
	static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char* wstr = str;
	int value = (int) val;

	div_t res;

	// Validate base
	if (base < 2 || base > 35) {
		*wstr = '\0';
		return;
	}

	// Check sign
	if (negative) {
		value = -value;
	}

	do {
		res = div(value, base);
		*wstr++ = num[res.rem];
	} while ((value = res.quot));

	if (negative)
		*wstr++ = '-';

	*wstr = '\0';

	// Reverse string
	strreverse(str, wstr - 1);
}

/**
 * Converts an unsigned long to a string
 *
 * @param value  The value to convert
 * @param str    Pointer to the string
 * @param base   The base
 */
void ultoa(unsigned long value, char* str, int base) {
	xtoa(value, str, base, 0);
}

/**
 * Converts an integer to a string
 *
 * @param value The value to convert
 * @param str   Pointer to the string to write to
 * @param base  The base
 */
void itoa(int value, char* str, int base) {
	xtoa(value, str, base, (value < 0));
}

