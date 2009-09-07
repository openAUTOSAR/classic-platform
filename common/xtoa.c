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
 * Ansi C "itoa" based on Kernighan & Ritchie's "Ansi C"
 * with slight modification to optimize for specific architecture:
 */

void strreverse(char* begin, char* end) {

  char aux;

  while (end > begin) {
    aux = *end, *end-- = *begin, *begin++ = aux;
  }
}


// int to string
void xtoa(unsigned long val, char* str, int base,int negative) {
  static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  char* wstr = str;
  int value = (int)val;

  div_t res;

  // Validate base
  if (base < 2 || base > 35) {
    *wstr = '\0';
    return;
  }

  // Check sign
  if ( negative ) {
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


// unsigned long to string
void ultoa(unsigned long value, char* str, int base) {
  xtoa(value,str,base,0);
}

// int to string
void itoa(int value, char* str, int base) {
  xtoa(value,str,base,(value<0));
}

