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
 *   This file is used to map memory areas to specific sections, for example
 *   a calibration variable to a specific place in ROM.
 *
 *
 * DISCUSSION
 *
 *   The Autosar way to do this is
 *
 *       #define XXX_START_SEC_YYY
 *       #include "MemMap.h"
 *       int hello_1 = 123;
 *       int hello_2 = 345;
 *       #define XXX_STOP_SEC_YYY
 *       #include "MemMap.h"
 *
 *     and in the MemMap.h file you then do
 *       #if defined(XXX_START_SEC_YYY)
 *       #pragma DoStuff
 *       #endif
 *
 *       #if defined(XXX_STOP_SEC_YYY)
 *       #pragma DoStuff
 *       #endif
 *
 *    This have the following good/bad things going for it:
 *      - Increases compilation time (all the #include takes time for the compiler)
 *      - Not portable (since this assumes that #pragma for a code/data section exist)
 *      +- works for "all" compilers except GCC (if you place also place hello_2 in within
 *         #includes it will work
 *
 *    Since ArcCore only support compilers with C99 support all the old (and often crappy) C89/C90
 *    compilers is not supported. "New" compilers often support the C99 _Pragma keyword or uses GCCs
 *    __attribute__ to set attributes on variables and functions. The really good things
 *    with using the latter keywords is that it can be pre-processed.
 *
 *    For example:
 *       SECTION_XXX_YYY int hello_1 = 123;
 *       SECTION_XXX_YYY int hello_2 = 345;
 *
 *    and in Memmap.h
 *      #define SECTION_XXX_YYY  __attribute__ ((section (".xxx_yyy")))
 *
 *    Soo, what should we support? Support both but remove the START and STOP from section.
 *
 *   Autosar <INIT_POLICY>
 *      NO_INIT           - Never cleared or initialized
 *      CLEARED           - Cleared at every reset (.bss)
 *      POWER_ON_CLEARED  - Cleared only on POR
 *      INIT              - Initialized at every reset (.data)
 *      POWER_ON_INIT     - Initialized only on POR
 *
 *   Of these NO_INIT and possible the POWER_ON_xxx makes sense, since the the rest
 *   is redundant information and you can make mistakes, e.g.
 *     #define XXX_START_SEC_CLEARED
 *     int apa = 123;
 *
 *   ops.. I have should have placed in a INIT instead....
 *
 *
 *   Examples:
 *
 *     Autosar                              ArcCore                      Section
 *     -------------------------------------------------------------------------------
 *     CAN_60_START_SEC_CLEARED             SECTION_CAN_60         		.bss
 *     ....
 *
 *
 *  ArcCore examples: (no vendor or other stuff for arc modules..)
 *  - ramlog              - SECTION_RAM_NO_INIT
 *  - winidea_port        - SECTION_RAM_NO_CACHE
 *  - dem_eventmemory_pri - SECTION_DEM_60_NO_INIT
 *
 */



#if defined(__GNUC__) || defined(__DCC__)
#if defined(__APPLE__)
	#define SECTION_RAMLOG	    __attribute__ ((section ("0,.ramlog")))
#else
   #define SECTION_RAMLOG	    __attribute__ ((section (".ramlog")))
   #define SECTION_RAM_NO_CACHE	__attribute__ ((section (".ram_no_cache")))
   #define SECTION_RAM_NO_INIT	__attribute__ ((section (".ram_no_init")))
#endif

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

   #pragma section RW ".ram_no_cache_data" ".ram_no_cache_bss"
   #define SECTION_RAM_NO_CACHE	__declspec(section ".ram_no_cache_data")

   #pragma section RW ".ram_no_init_data" ".ram_no_init_bss"
   #define SECTION_RAM_NO_INIT	__declspec(section ".ram_no_init_data")

#elif defined(__ICCHCS12__)
   #define SECTION_RAMLOG   __no_init
#else
#error Compiler not set
#endif

