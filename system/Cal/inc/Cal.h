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

/* @req SWS_Cal_00005 The header file structure shall contain an application interface header file Cal.h */
/* @req SWS_Cal_00016 The CAL shall support reentrant access to all services */
/* @req SWS_Cal_00021 The CAL shall not require initialization phase */
/* @req SWS_Cal_00022 The CAL shall allow parallel access to different services */
/* @req SWS_Cal_00023 CAL shall use a streaming aproach with start, update and finish functions */
/* @req SWS_Cal_00027 The CAL shall not require a shutdown operation phase */
/* @req SWS_Cal_00035 The interface functions shall immediately compute the result */
/* @req SWS_Cal_00067 CAL shall not call the DET in case of error */
/* @req SWS_Cal_00728 CAL services, which do not expect arbitrary amounts of data, only have to provide Cal_<Service>() */
/* @req SWS_Cal_00729 CAL services, which expect arbitrary amounts of data, shall provide Cal_<Service>Start(), Cal_<Service>Update() and Cal_<Service>Finish() */
/* @req SWS_Cal_00731 CAL API can be directly called from BSW modules or SWC. No port definition is required */
/* @req SWS_Cal_00736 A library function shall not call any BSW modules functions */
/* @req SWS_Cal_00015 CAL library and the underlying Crypto Library shall only provide necessary services and algorithms */

#ifndef CAL_H_
#define CAL_H_

#include "Cal_Types.h"

/**
 * Function to initialize the Mac generate service
 * @param cfgId Identifier of the CAL module configuration to be used for the MAC computation
 * @param contextBuffer Pointer to the buffer in which the contect of the service is stored
 * @param keyPtr Pointer to the key to be used for the MAC generation
 * @return CAL_E_OK if request successful, CAL_E_NOT_OK of request failed
 */
/* @req SWS_Cal_00108 Cal_MacGenerateStart */
Cal_ReturnType Cal_MacGenerateStart(Cal_ConfigIdType cfgId, Cal_MacGenerateCtxBufType contextBuffer, const Cal_SymKeyType* keyPtr);

/**
 * Function to feed input data to the MAC generate service
 * @param cfgPtr Identifier of the CAL module configuration to be used for the MAC computation
 * @param contextBuffer Pointer to the buffer in which the contect of the service is stored
 * @param dataPtr Pointer to the data for which the MAC shall be computed
 * @param dataLength Number of bytes for which the MAC shall be computed
 * @return CAL_E_OK if request successful, CAL_E_NOT_OK of request failed
 */
/* @req SWS_Cal_00114 Cal_MacGenerateUpdate */
Cal_ReturnType Cal_MacGenerateUpdate(Cal_ConfigIdType cfgId, Cal_MacGenerateCtxBufType contextBuffer, const uint8* dataPtr, uint32 dataLength);

/**
 * Function to finish the MAC generate service
 * @param cfgPtr Identifier of the CAL module configuration to be used for the MAC computation
 * @param contextBuffer Pointer to the buffer in which the contect of the service is stored
 * @param resultPtr Pointer to the memory location to store the generated MAC in.
 * @param resultLengthPtr On calling this variable contain the size of the resultbuffer, on returning it contains the length of the computed MAC
 * @param TruncationIsAllowed
 * @return CAL_E_OK if request successful, CAL_E_NOT_OK of request failed
 */
/* @req SWS_Cal_00121 Cal_MacGenerateFinish */
Cal_ReturnType Cal_MacGenerateFinish(Cal_ConfigIdType cfgId, Cal_MacGenerateCtxBufType contextBuffer, uint8* resultPtr, uint32* resultLengthPtr, boolean TruncationIsAllowed);

/**
 * Function to initialize the Mac verify service
 * @param cfgPtr Identifier of the CAL module configuration to be used for the MAC computation
 * @param contextBuffer Pointer to the buffer in which the contect of the service is stored
 * @param keyPtr Pointer to the key to be used for the MAC generation
 * @return CAL_E_OK if request successful, CAL_E_NOT_OK of request failed
 */
/* @req SWS_Cal_00128 Cal_MacVerifyStart */
Cal_ReturnType Cal_MacVerifyStart(Cal_ConfigIdType cfgId, Cal_MacVerifyCtxBufType contextBuffer, const Cal_SymKeyType* keyPtr);

/**
 * Function to feed input data to the MAC verify service
 * @param cfgPtr Identifier of the CAL module configuration to be used for the MAC computation
 * @param contextBuffer Pointer to the buffer in which the contect of the service is stored
 * @param dataPtr Pointer to the data for which the MAC shall be computed
 * @param datalength Number of bytes for which the MAC shall be computed
 * @return CAL_E_OK if request successful, CAL_E_NOT_OK of request failed
 */
/* @req SWS_Cal_00134 Cal_MacVerifyUpdate */
Cal_ReturnType Cal_MacVerifyUpdate(Cal_ConfigIdType cfgPtr,Cal_MacVerifyCtxBufType contextBuffer, const uint8* dataPtr,uint32 dataLength);

/**
 * Function to finish the MAC verify service
 * @param cfgPtr Identifier of the CAL module configuration to be used for the MAC computation
 * @param contextBuffer Pointer to the buffer in which the contect of the service is stored
 * @param MacPtr Pointer to the MAC to be verified
 * @param MacLength Length of the MAC to be verified
 * @param resultPtr Pointer to the result of the MAC verification
 * @return CAL_E_OK if request successful, CAL_E_NOT_OK of request failed
 */
/* @req SWS_Cal_00141 Cal_MacVerifyFinish */
Cal_ReturnType Cal_MacVerifyFinish(Cal_ConfigIdType cfgId, Cal_MacVerifyCtxBufType contextBuffer, const uint8* MacPtr, uint32 MacLength, Cal_VerifyResultType* resultPtr);

#endif /* CAL_H_ */
