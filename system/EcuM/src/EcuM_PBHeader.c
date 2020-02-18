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

/* The post build area has the following structure.
 *
 * PostbuildConfig_Type - contains general data about the post
 * build data and contains pointers to the actual configurations
 * that are stored afterwards.
 *
 * ComM - ComM_Config
 * Com  - ComConfiguration
 * PduR  - PduR_Config
 * CanIf - CanIf_Config
 * CanTp Not supported yet
 * LinIf (N/A)
 * LinTp (N/A)
 * FrIf (N/A)
 * FrTp (N/A)
 *
 */


#include "EcuM_PBTypes.h"
#include "PreCompiledDataHash.h"
#include "MemMap.h"

#if defined(USE_CANTP)
extern const CanTp_ConfigType CanTpConfig;
#endif

#if defined(USE_PDUR)
extern const PduR_PBConfigType PduR_Config;
#endif

#if defined(USE_CANIF)
extern const CanIf_ConfigType CanIf_Config;
#endif

#if defined(USE_COM)
extern const Com_ConfigType ComConfiguration;
#endif

#if defined(USE_FIM)
#include "FiM.h"
#endif

/*lint -save -e785 MISRA:FALSE_POSITIVE:Partial array initialization is allowed because this can be rewritten in integration:[MISRA 2012 Rule 9.3, required] */

SECTION_POSTBUILD_HEADER const PostbuildConfigType Postbuild_Config = {
    .startPattern = 0x5A5A5A5A,
    .postBuildVariant = 1,
    .preCompileHashLow = PRE_COMPILED_DATA_HASH_LOW,
    .preCompileHashHigh = PRE_COMPILED_DATA_HASH_HIGH,
#if defined (USE_BSWM)
    .BswMConfigPtr = &BswM_Config,
#endif
#if defined (USE_CAN)
    .CanConfigPtr = &CanConfigData,
#endif
#if defined(USE_CANIF)
    .CanIfConfigPtr = &CanIf_Config,
#endif
#if defined (USE_CANNM)
    .CanNmConfigPtr = &CanNm_Config,
#endif
#if defined(USE_CANTP)
    .CanTpConfigPtr = &CanTpConfig,
#endif
#if defined(USE_COM)
    .ComConfigPtr = &ComConfiguration,
#endif
#if defined (USE_COMM)
    .ComMConfigPtr = &ComM_Config,
#endif
#if defined(USE_PDUR)
    .PduRConfigPtr = &PduR_Config,
#endif
#if defined(USE_CANTRCV)
	.CanTrcvConfigPtr = &CanTrcv_Config,
#endif
#if defined(USE_FIM) && (FIM_POSTBUILD_MODE == STD_ON)
    .FiMConfigPtr = &FiM_Config,
#endif
};
/*lint -restore */
