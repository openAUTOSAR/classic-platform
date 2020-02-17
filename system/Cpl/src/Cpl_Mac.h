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

/* @req SWS_Cal_00536 Each CPL library shall provide a header file */
/* @req SWS_Cal_00461 For every API function there should be a coresponding Cpl function */

#ifndef CPL_MAC_H_
#define CPL_MAC_H_

#include "Cal_Types.h"

/**
 * Function to initialize the Mac generate service
 * @param cfgPtr Identifier of the CAL module configuration to be used for the MAC computation
 * @param contextBuffer Pointer to the buffer in which the contect of the service is stored
 * @param keyPtr Pointer to the key to be used for the MAC generation
 * @return CAL_E_OK if request successful, CAL_E_NOT_OK of request failed
 */
Cal_ReturnType Cpl_MacGenerateStart(const void* cfgPtr, Cal_MacGenerateCtxBufType contextBuffer, const Cal_SymKeyType* keyPtr);


/**
 * Function to feed input data to the MAC generate service
 * @param cfgPtr Identifier of the CAL module configuration to be used for the MAC computation
 * @param contextBuffer Pointer to the buffer in which the contect of the service is stored
 * @param dataPtr Pointer to the data for which the MAC shall be computed
 * @param dataLength Number of bytes for which the MAC shall be computed
 * @return CAL_E_OK if request successful, CAL_E_NOT_OK of request failed
 */
Cal_ReturnType Cpl_MacGenerateUpdate(const void* cfgPtr, Cal_MacGenerateCtxBufType contextBuffer, const uint8* dataPtr, uint32 dataLength);

/**
 * Function to finish the MAC generate service
 * @param cfgPtr Identifier of the CAL module configuration to be used for the MAC computation
 * @param contextBuffer Pointer to the buffer in which the contect of the service is stored
 * @param resultPtr Pointer to the memory location to store the generated MAC in.
 * @param resultLengthPtr On calling this variable contain the size of the resultbuffer, on returning it contains the length of the computed MAC
 * @param TruncationIsAllowed
 * @return CAL_E_OK if request successful, CAL_E_NOT_OK of request failed
 */
Cal_ReturnType Cpl_MacGenerateFinish(const void* cfgPtr, Cal_MacGenerateCtxBufType contextBuffer, uint8* resultPtr, uint32* resultLengthPtr, boolean TruncationIsAllowed);

/**
 * Function to initialize the Mac verify service
 * @param cfgPtr Identifier of the CAL module configuration to be used for the MAC computation
 * @param contextBuffer Pointer to the buffer in which the contect of the service is stored
 * @param keyPtr Pointer to the key to be used for the MAC generation
 * @return CAL_E_OK if request successful, CAL_E_NOT_OK of request failed
 */
Cal_ReturnType Cpl_MacVerifyStart(const void* cfgPtr, Cal_MacVerifyCtxBufType contextBuffer, const Cal_SymKeyType* keyPtr);

/**
 * Function to feed input data to the MAC verify service
 * @param cfgPtr Identifier of the CAL module configuration to be used for the MAC computation
 * @param contextBuffer Pointer to the buffer in which the contect of the service is stored
 * @param dataPtr Pointer to the data for which the MAC shall be computed
 * @param datalength Number of bytes for which the MAC shall be computed
 * @return CAL_E_OK if request successful, CAL_E_NOT_OK of request failed
 */
Cal_ReturnType Cpl_MacVerifyUpdate(const void* cfgPtr, Cal_MacVerifyCtxBufType contextBuffer, const uint8* dataPtr, uint32 datalength);

/**
 * Function to finish the MAC verify service
 * @param cfgPtr Identifier of the CAL module configuration to be used for the MAC computation
 * @param contextBuffer Pointer to the buffer in which the contect of the service is stored
 * @param MacPtr Pointer to the MAC to be verified
 * @param MacLength Length of the MAC to be verified
 * @param resultPtr Pointer to the result of the MAC verification
 * @return CAL_E_OK if request successful, CAL_E_NOT_OK of request failed
 */
Cal_ReturnType Cpl_MacVerifyFinish(const void* cfgPtr, Cal_MacVerifyCtxBufType contextBuffer, const uint8* MacPtr, uint32 MacLength, Cal_VerifyResultType* resultPtr);


#endif /* CPL_MAC_H_ */
