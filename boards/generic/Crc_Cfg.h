/*
 * Generator version: 2.0.0
 * AUTOSAR version:   4.3.0
 */

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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0 */

#if !(((CRC_SW_MAJOR_VERSION == 2) && (CRC_SW_MINOR_VERSION == 0)) )
#error Crc: Configuration file expected BSW module version to be 2.0.*
#endif
#if !(((CRC_AR_RELEASE_MAJOR_VERSION == 4) && (CRC_AR_RELEASE_MINOR_VERSION == 3)) )
#error Crc: Configuration file expected AUTOSAR version to be 4.3.*
#endif

#ifndef CRC_CFG_H_
#define CRC_CFG_H_

/* @req SWS_CRC_00022 The Crc module shall comply with the following include file structure */
#include "Crc_MemMap.h"

/* @req SWS_CRC_00040 Requirement was removed from ASR4.3.0, but kept to check,that only VARIANT-PRE-COMPILE is supported */

/* CRC 8 Configuration
 * Possible values and the mode decides what method to be used
 */
#define CRC_8_HARDWARE (0x01) /* Not supported */
#define CRC_8_RUNTIME  (0x02)
#define CRC_8_TABLE    (0x04) /* Default value */


#define Crc_8_Mode			CRC_8_TABLE


/* CRC 8H2F Configuration
 * Possible values and the mode decides what method to be used
 */
#define CRC_8H2F_HARDWARE (0x01) /* Not supported */
#define CRC_8H2F_RUNTIME  (0x02)
#define CRC_8H2F_TABLE    (0x04) /* Default value */


#define Crc_8_8H2FMode		CRC_8H2F_TABLE


/* CRC 16 Configuration
 * Possible values and the mode decides what method to be used
 */
#define CRC_16_HARDWARE (0x01) /* Not supported */
#define CRC_16_RUNTIME  (0x02)
#define CRC_16_TABLE    (0x04) /* Default value */

#define SAFELIB_VERSIONINFO_API  STD_OFF // Enable/Diable version info API

#define Crc_16_Mode			CRC_16_TABLE


/* CRC 32 Configuration
 * Possible values and the mode decides what method to be used
 */
#define CRC_32_HARDWARE (0x01)  /* Not supported */
#define CRC_32_RUNTIME  (0x02)
#define CRC_32_TABLE    (0x04)  /* Default value */


#define CRC_32_MODE      CRC_32_TABLE


/* CRC 32P4 Configuration
 * Possible values and the mode decides what method to be used
 */
#define CRC_32P4_HARDWARE (0x01)  /* Not supported */
#define CRC_32P4_RUNTIME  (0x02)
#define CRC_32P4_TABLE    (0x04)  /* Default value */


#define CRC_32P4_MODE      CRC_32P4_TABLE


#endif /* CRC_CFG_H_ */

