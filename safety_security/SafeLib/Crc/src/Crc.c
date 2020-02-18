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

/* @req SWS_CRC_00022 The Crc module shall comply with the following include file structure */
#include "Crc.h"

#define CRC_AR_RELEASE_MAJOR_VERSION_INT        4u
#define CRC_AR_RELEASE_MINOR_VERSION_INT        3u
#define CRC_AR_RELEASE_REVISION_VERSION_INT     0u

#define CRC_SW_MAJOR_VERSION_INT        2
#define CRC_SW_MINOR_VERSION_INT        0


/* @req SWS_CRC_00005 Version check SWS_BSWGeneral, chapter 5.1.8 */
/* @req SWS_BSW_00036 Inter Module Checks to avoid integration of incompatible files */
#if (CRC_SW_MAJOR_VERSION != CRC_SW_MAJOR_VERSION_INT) || (CRC_SW_MINOR_VERSION != CRC_SW_MINOR_VERSION_INT)
#error "SW Version mismatch between Crc.c and Crc.h"
#endif

#if (CRC_AR_RELEASE_MAJOR_VERSION != CRC_AR_RELEASE_MAJOR_VERSION_INT) || (CRC_AR_RELEASE_MINOR_VERSION != CRC_AR_RELEASE_MINOR_VERSION_INT) || \
    (CRC_AR_RELEASE_REVISION_VERSION != CRC_AR_RELEASE_REVISION_VERSION_INT)
#error "AR Version mismatch between Crc.c and Crc.h"
#endif

/* @req SWS_CRC_00011 The function Crc_GetVersionInfo shall return the version information of the Crc module */
/* @req SWS_CRC_00021 API for Crc_GetVersionInfo */
/* @req SWS_BSW_00064 Get Version Information function shall be executed synchronously to its call and shall be reentrant */
/* @req SWS_BSW_00052 Get Version Information function shall have only one parameter. */
/* @req SWS_BSW_00164 Ther should be no restriction to Get Version Information calling context */
/* @req SWS_BSW_00051*/
/** @brief Return the version information of Crc module.
 *
 * The function Crc_GetVersionInfo shall return the version
 * information of the Crc module.
 * The version information includes:
 * - Module Id
 * - Vendor Id
 * - sw_major_version
 * - sw_minor_version
 * - sw_patch_version
 *
 *  @param Std_VersionInfoType The type including Module and Vendor ID for the Crc Module.
 *  @return void.
 */
#if (SAFELIB_VERSIONINFO_API == STD_ON)
void Crc_GetVersionInfo(Std_VersionInfoType* versioninfo) {
	
	/* @req SWS_BSW_00212 NULL pointer checking */
	if (versioninfo != NULL_PTR) {
            versioninfo->vendorID = CRC_VENDOR_ID;
            versioninfo->moduleID = CRC_MODULE_ID;
            versioninfo->sw_major_version = CRC_SW_MAJOR_VERSION;
            versioninfo->sw_minor_version = CRC_SW_MINOR_VERSION;
            versioninfo->sw_patch_version = CRC_SW_PATCH_VERSION;
	}
}
#endif
