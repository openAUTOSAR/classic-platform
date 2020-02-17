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


#if defined(__GNUC__) || defined(__DCC__)  || defined(__ghs__) || defined(__ARMCC_VERSION) || defined(__IAR_SYSTEMS_ICC__)
#if defined(__APPLE__)
    #define SECTION_RAMLOG      __attribute__ ((section ("0,.ramlog")))
    #define SECTION_POSTBUILD_DATA
    #define SECTION_POSTBUILD_HEADER
#else
   #define SECTION_RAMLOG       __attribute__ ((section (".ramlog")))
   #define SECTION_RAM_NO_CACHE_BSS __attribute__ ((section (".ram_no_cache_bss")))
   #define SECTION_RAM_NO_CACHE_DATA    __attribute__ ((section (".ram_no_cache_data")))
   #define SECTION_RAM_NO_INIT  __attribute__ ((section (".ram_no_init")))
#if defined(CFG_POSTBUILD)
   #define SECTION_POSTBUILD_DATA __attribute__ ((section (".postbuild_data")))
   #define SECTION_POSTBUILD_HEADER __attribute__ ((section (".postbuild_header")))
#else
   #define SECTION_POSTBUILD_DATA
   #define SECTION_POSTBUILD_HEADER
#endif
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
   #define SECTION_RAMLOG   __declspec(section ".ramlog_data")

   #pragma section RW ".ram_no_cache_data" ".ram_no_cache_bss"
   #define SECTION_RAM_NO_CACHE_BSS __declspec(section ".ram_no_cache_bss")
   #define SECTION_RAM_NO_CACHE_DATA    __declspec(section ".ram_no_cache_data")

   #pragma section RW ".ram_no_init_data" ".ram_no_init_bss"
   #define SECTION_RAM_NO_INIT  __declspec(section ".ram_no_init_data")

#if defined(CFG_POSTBUILD)
   #pragma section RW ".postbuild_data" ".postbuild_bss"
   #define SECTION_POSTBUILD_DATA  __declspec(section ".postbuild_data")

   #pragma section RW ".postbuild_header" ".postbuild_header_bss"
   #define SECTION_POSTBUILD_HEADER __declspec(section ".postbuild_header")
#else
   #define SECTION_POSTBUILD_DATA
   #define SECTION_POSTBUILD_HEADER
#endif
#else
#error Compiler not set
#endif

#if defined(MCU_LP_START_SEC_CLEARED_8)
#if defined(__GNUC__) || defined(__ghs__) || defined(__DCC__) || defined(__IAR_SYSTEMS_ICC__)
__attribute__ ((section (".lowpower_bss"))) __attribute__ ((aligned (8)))
#elif defined(__CWCC__)
#pragma section RW ".lowpower_data" ".lowpower_bss"
__attribute__ ((section (".lowpower_data"))) __attribute__ ((aligned (8)))
#else
#error Compiler not supported
#endif
#undef MCU_LP_START_SEC_CLEARED_8
#endif

#if defined(MCU_LP_STOP_SEC_CLEARED_8)
#undef MCU_LP_START_SEC_CLEARED_8
#endif

#if defined(ETH_DESC_START_SEC_CLEARED_4)
#if defined(__GNUC__) || defined(__ARMCC_VERSION) || defined(__DCC__) || defined(__IAR_SYSTEMS_ICC__)
__attribute__ ((section(".eth_desc")))
#else
#error Compiler not supported
#endif
#undef ETH_DESC_START_SEC_CLEARED_4
#endif
#if defined(ETH_DESC_STOP_SEC_CLEARED_4)
#undef ETH_DESC_START_SEC_CLEARED_4
#endif

#if defined(ETH_BUF_START_SEC_CLEARED_4)
#if defined(__GNUC__) || defined(__ARMCC_VERSION) || defined(__ghs__) || defined(__IAR_SYSTEMS_ICC__)
__attribute__ ((section(".eth_ahb_buf")))
#elif defined(__DCC__)
__attribute__ ((section(".eth_ahb_buf"))) __attribute__ ((aligned (4)))
#else
#error Compiler not supported
#endif
#undef ETH_BUF_START_SEC_CLEARED_4
#endif
#if defined(ETH_BUF_STOP_SEC_CLEARED_4)
#undef ETH_BUF_START_SEC_CLEARED_4
#endif




