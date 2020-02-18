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

#include "SafeIoHwAb.h"
#include "SafeIoHwAb_Internal.h"

#if ( SAFEIOHWAB_VERSION_INFO_API == STD_ON )
/* @req SWS_BSW_00064 GetVersionInfo shall execute synchonously */
/* @req SWS_BSW_00052 GetVersion info shall only have one parameter */
/* @req SWS_BSW_00164 No restriction on calling context */
void SafeIoHwAb_GetVersionInfo( Std_VersionInfoType* VersionInfo )
{
    /*lint -e904 MISRA:OTHER:argument check:[MISRA 2012 Rule 15.5, advisory]*/
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */

	/* @req SWS_BSW_00212 NULL pointer check */	
    SAFEIOHWAB_VALIDATE_NO_RV((NULL_PTR != VersionInfo), SAFEIOHWAB_GETVERSIONINFO_ID, SAFEIOHWAB_E_PARAM_PTR);


    VersionInfo->vendorID =  SAFEIOHWAB_VENDOR_ID;
    VersionInfo->moduleID =  SAFEIOHWAB_MODULE_ID;
    VersionInfo->sw_major_version =  SAFEIOHWAB_SW_MAJOR_VERSION;
    VersionInfo->sw_minor_version =  SAFEIOHWAB_SW_MINOR_VERSION;
    VersionInfo->sw_patch_version =  SAFEIOHWAB_SW_PATCH_VERSION;

}
#endif



