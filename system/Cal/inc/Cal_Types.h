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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */
/** @tagSettings DEFAULT_ARCHITECTURE=GENERIC */

/* @req SWS_Cal_00004 The header file structure shall contain a type header Cal_Types.h */
/* @req SWS_Cal_00068 Only standard AUTOSAR types shall be imported */
/* @req SWS_Cal_00069 Each CPL library shall provide a header file */
/* @req SWS_Cal_00730 All context buffers shall be aligned according to the maximum alignment of all scalar types on the given platform */

#ifndef CAL_TYPES_H_
#define CAL_TYPES_H_

#include "Std_Types.h"
#include "Cal_Cfg.h"

typedef enum {
   CAL_E_OK = 0,
   CAL_E_NOT_OK = 1,
   CAL_E_SMALL_BUFFER = 3,
   CAL_E_ENTROPY_EXHAUSTION = 4
} Cal_ReturnType;

/* @req SWS_Cal_00075 Cal_VerifyResultType */
typedef enum {
   CAL_E_VER_OK = 0,
   CAL_E_VER_NOT_OK = 1
} Cal_VerifyResultType;

/* @req SWS_Cal_00073 Cal_ConfigIdType */
typedef uint16 Cal_ConfigIdType;

/* @req SWS_Cal_00743 Cal_AlignType */
typedef CalMaxAlignScalarType Cal_AlignType;

/* @req SWS_Cal_00742 Cal_MacGenerateCtxBufType */
typedef Cal_AlignType Cal_MacGenerateCtxBufType[CAL_MACGENERATE_CONTEXT_BUFFER_SIZE];
/* @req SWS_Cal_00742 Cal_MacVerifyCtxBufType */
typedef Cal_AlignType Cal_MacVerifyCtxBufType[CAL_MACGENERATE_CONTEXT_BUFFER_SIZE];

/* @req SWS_Cal_00082 Cal_SymKeyType */
typedef struct {
    uint32 length;
    Cal_AlignType data[CAL_SYM_KEY_MAX_SIZE];
} Cal_SymKeyType;

typedef Cal_ReturnType (*CplMacGenerateStartType)(const void* cfgPtr, Cal_MacGenerateCtxBufType contextBuffer, const Cal_SymKeyType* keyPtr);
typedef Cal_ReturnType (*CplMacGenerateUpdateType)(const void* cfgPtr, Cal_MacGenerateCtxBufType contextBuffer, const uint8* dataPtr, uint32 dataLength);
typedef Cal_ReturnType (*CplMacGenerateFinishType)(const void* cfgPtr, Cal_MacGenerateCtxBufType contextBuffer, uint8* resultPtr, uint32* resultLengthPtr, boolean TruncationIsAllowed);

typedef Cal_ReturnType (*CplMacVerifyStartType)(const void* cfgPtr, Cal_MacVerifyCtxBufType contextBuffer, const Cal_SymKeyType* keyPtr);
typedef Cal_ReturnType (*CplMacVerifyUpdateType)(const void* cfgPtr,Cal_MacVerifyCtxBufType contextBuffer, const uint8* dataPtr,uint32 dataLength);
typedef Cal_ReturnType (*CplMacVerifyFinishType)(const void* cfgPtr, Cal_MacVerifyCtxBufType contextBuffer, const uint8* MacPtr, uint32 MacLength, Cal_VerifyResultType* resultPtr);

/* @req SWS_Cal_00074 Cal_MacGenerateConfigType*/
typedef struct {
    Cal_ConfigIdType ConfigId;
    CplMacGenerateStartType MacGenerateStartFunction;
    CplMacGenerateUpdateType MacGenerateUpdateFunction;
    CplMacGenerateFinishType MacGenerateFinishFunction;
    void *MacGenerateConfigPtr;
} Cal_MacGenerateConfigType;

/* @req SWS_Cal_00074 Cal_MacVerifyConfigType*/
typedef struct {
    Cal_ConfigIdType ConfigId;
    CplMacVerifyStartType MacVerifyStartFunction;
    CplMacVerifyUpdateType MacVerifyUpdateFunction;
    CplMacVerifyFinishType MacVerifyFinishFunction;
    void *MacVerifyConfigPtr;
} Cal_MacVerifyConfigType;

/* @req SWS_Cal_00544 Cpl_MacGenerateConfigType */
typedef struct {
    Cal_ConfigIdType ConfigId;
} Cpl_MacGenerateConfigType;

/* @req SWS_Cal_00544 Cpl_MacVerifyConfigType */
typedef struct {
    Cal_ConfigIdType ConfigId;
} Cpl_MacVerifyConfigType;

#endif /* CAL_TYPES_H_ */
