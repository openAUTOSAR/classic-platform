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
 * Itoa based on K&R itoa
 */


#include <stdlib.h>



/**
 * Convert a number to a string
 *
 * @param val		The value to convert
 * @param str		Pointer to a space where to put the string
 * @param base		The base
 * @param negative	If negative or not.
 */
void xtoa( unsigned long val, char* str, int base, int negative) {
	int i;
	char *oStr = str;
	char c;

	if (negative) {
		val = -val;
	}

	if( base < 10 && base > 16 ) {
		*str = '0';
		return;
	}
    i = 0;

    do {
      str[i++] = "0123456789abcdef"[ val % base ];
	} while ((val /= base) > 0);


    if (negative) {
        str[i++] = '-';
    }

    str[i] = '\0';
    str = &str[i]-1;
    while(str > oStr) {
    	c = *str;
    	*str-- = *oStr;
    	*oStr++=c;
    }
}


#if defined(TEST_XTOA)
/* Some very limited testing */
int main( void ) {
	char str[20];

	xtoa(123,str,10,0);
	printf("%s\n",str);
	xtoa(-123,str,10,1);
	printf("%s\n",str);
	xtoa(0xa123,str,16,0);
	printf("%s\n",str);
	xtoa(-0xa123,str,16,1);
	printf("%s\n",str);


	return 0;
}
#endif


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
char * itoa(int value, char* str, int base) {
	xtoa(value, str, base, (value < 0));
	return str;
}

