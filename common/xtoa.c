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

