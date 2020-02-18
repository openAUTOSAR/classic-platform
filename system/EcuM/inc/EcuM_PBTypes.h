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

#ifndef POSTBUILD_TYPES_H_
#define POSTBUILD_TYPES_H_


#include "Std_Types.h"

#if defined(USE_BSWM)
#include "BswM.h"
#endif

#if defined(USE_CAN)
#include "Can.h"
#endif

#if defined(USE_CANIF)
#include "CanIf.h"
#endif

#if defined(USE_CANNM)
#include "CanNm.h"
#endif

#if defined(USE_CANTP)
#include "CanTp.h"
#endif

#if defined (USE_CANTRCV)
#include "CanTrcv.h"
#endif

#if defined(USE_COM)
#include "Com.h"
#endif

#if defined(USE_COMM)
#include "ComM.h"
#endif

#if defined(USE_PDUR)
#include "PduR.h"
#endif

#ifdef CFG_POSTBUILD_PARTNUMBER
#include "partno.h"
#endif

#if defined(USE_FIM)
#include "FiM.h"
#endif

typedef struct {

    const uint32 startPattern;
    const uint32 postBuildVariant;
#ifdef CFG_POSTBUILD_PARTNUMBER
	const PostBuildPartNoType partNumber;
#endif
    const uint64 preCompileHashLow; /* Lower 8 bytes of the MD5 */
    const uint64 preCompileHashHigh; /* Upper 8 bytes of the MD5 */

#if defined(USE_BSWM)
    const BswM_ConfigType* const BswMConfigPtr;
#endif

#if defined(USE_CAN)
    const Can_ConfigType* const CanConfigPtr;
#endif

#if defined(USE_CANIF)
    const CanIf_ConfigType* const CanIfConfigPtr;
#endif

#if defined(USE_CANNM)
    const CanNm_ConfigType* const CanNmConfigPtr;
#endif

#if defined(USE_CANTP)
    const CanTp_ConfigType* const CanTpConfigPtr;
#endif

#if defined (USE_CANTRCV)
    const CanTrcv_ConfigType* const CanTrcvConfigPtr;
#endif

#if defined(USE_COM)
    const Com_ConfigType* const ComConfigPtr;
#endif

#if defined(USE_COMM)
    const ComM_ConfigType* const ComMConfigPtr;
#endif

#if defined(USE_PDUR)
    const PduR_PBConfigType* const PduRConfigPtr;
#endif

#if defined(USE_FIM)
    const FiM_ConfigType* const FiMConfigPtr;
#endif

const uint32 * const endPattern;

} PostbuildConfigType;


#endif
