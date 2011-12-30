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

/* REFERENCE
 *   MemoryMapping.pdf
 *
 * DESCRIPTION
 *
 *
 *
 */

#if defined(__GNUC__) || defined(__DCC__)

   #define SECTION_RAMLOG	__attribute__ ((section (".ramlog")))


#elif defined(__CWCC__)

/* The compiler manual states:
 *   The section name specified in the
 *   __declspec(section <section_name>) statement must be the
 *   name of an initialized data section.  It is an error to use the uninitialized
 *   data section name.
 *
 * NOTE!! The initialized data section name is __declspec() does not mean that
 *        it will end up in that section, if its BSS data it will end-up in
 *        ramlog_bss instead.
 *
 * NOTE!! Naming the initialized and uninitialized data section to the same
 *        name will generate strange linker warnings (sometimes)
 */

   #pragma section RW ".ramlog_data" ".ramlog_bss"
   #define SECTION_RAMLOG	__declspec(section ".ramlog_data")


#endif

