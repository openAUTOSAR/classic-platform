
#ifndef POSTBUILD_TYPES_H_
#define POSTBUILD_TYPES_H_


#include "Std_Types.h"

#if defined(USE_CANTP)
#include "CanTp.h"
#endif

#if defined(USE_COM)
#include "Com.h"
#endif

#if defined(USE_PDUR)
#include "PduR.h"
#endif

#if defined(USE_CANIF)
#include "CanIf.h"
#endif

#if defined(USE_CANNM)
#include "CanNm.h"
#endif


typedef struct {

	const uint32 startPattern;
	const uint32 postBuildVariant;
	const uint64 preCompileHashLow; /* Lower 8 bytes of the MD5 */
	const uint64 preCompileHashHigh; /* Upper 8 bytes of the MD5 */

#if defined(USE_CANTP)
	const CanTp_ConfigType* const CanTp_ConfigPtr;
#endif

#if defined(USE_COM)
	const Com_ConfigType* const ComConfigurationPtr;
#endif

#if defined(USE_PDUR)
	const PduR_PBConfigType* const PduR_ConfigPtr;
#endif

#if defined(USE_CANIF)
	const CanIf_ConfigType* const CanIf_ConfigPtr;
#endif

#if defined(USE_CANNM)
	const CanNm_ConfigType* const CanNm_ConfigPtr;
#endif


} PostbuildConfigType;


#endif
