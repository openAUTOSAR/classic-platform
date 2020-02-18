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

/* @req SWS_E2E_00115 E2E library files (i.e. E2E_*.*) shall not include any RTE files */
/* @req SWS_E2E_00216
 * The E2E Library shall not call BSW modules for error reporting (in particular DEM and DET), nor for any other purpose.
 * The E2E Library shall not call RTE. */

#define E2E_AR_RELEASE_MAJOR_VERSION_INT        4u
#define E2E_AR_RELEASE_MINOR_VERSION_INT        3u
#define E2E_AR_RELEASE_REVISION_VERSION_INT     0u

#define E2E_SW_MAJOR_VERSION_INT        2
#define E2E_SW_MINOR_VERSION_INT        0

#include "E2E.h"

/* @req SWS_E2E_00327
 * The implementer of the E2E Library shall avoid the integration of incompatible files. Minimum implementation is the version check of the header files.
 */
#if (E2E_AR_RELEASE_MAJOR_VERSION != E2E_AR_RELEASE_MAJOR_VERSION_INT) || (E2E_AR_RELEASE_MINOR_VERSION != E2E_AR_RELEASE_MINOR_VERSION_INT)
#error "SW Version mismatch between E2E.c and E2E.h"
#endif

#if (E2E_AR_RELEASE_MAJOR_VERSION != E2E_AR_RELEASE_MAJOR_VERSION_INT) || (E2E_AR_RELEASE_MINOR_VERSION != E2E_AR_RELEASE_MINOR_VERSION_INT) || \
    (E2E_AR_RELEASE_REVISION_VERSION != E2E_AR_RELEASE_REVISION_VERSION_INT)
#error "AR Version mismatch between E2E.c and E2E.h"
#endif

/* @req SWS_E2E_00032 API: E2E_GetVersionInfo */
/* @req SWS_E2E_00033 The function E2E_GetVersionInfo shall return the version information of the E2E module */
/* @req SWS_BSW_00051*/
/** @brief Return the version information of E2E module.
 *
 * The function E2E_GetVersionInfo shall return the version
 * information of the E2E module.
 * The version information includes:
 * - Module Id
 * - Vendor Id
 * - sw_major_version
 * - sw_minor_version
 * - sw_patch_version
 *
 *  @param Std_VersionInfoType The type including Module and Vendor ID for the E2E Module.
 *  @return void.
 */
#if (SAFELIB_VERSIONINFO_API == STD_ON)
void E2E_GetVersionInfo(Std_VersionInfoType* versioninfo) {

	if (versioninfo != NULL_PTR) {
            versioninfo->vendorID = E2E_VENDOR_ID;
            versioninfo->moduleID = E2E_MODULE_ID;
            versioninfo->sw_major_version = E2E_SW_MAJOR_VERSION;
            versioninfo->sw_minor_version = E2E_SW_MINOR_VERSION;
            versioninfo->sw_patch_version = E2E_SW_PATCH_VERSION;
	}
}
#endif
