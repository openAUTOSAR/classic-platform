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

/* @req SWS_Cal_00001 The CAL shall be able to incorporate cryptographic library modules specified in chapter 8.4*/
/* @req SWS_Cal_00003 The header file structure shall contain a configuration header Cal_Cfg.h*/
/* @req SWS_Cal_00006 The code file structure shall contain one or more MISRA-C 2004 conform source files Cal_<xxx>.c */
/* @req SWS_Cal_00008 Include file structure for Cal and Cpl*/
/* @req SWS_Cal_00021 The CAL shall not require initialization phase */
/* @req SWS_Cal_00024 Structure of Cal and the states */
/* @req SWS_Cal_00025 Each service shall have a constant structure Cal_<Service>ConfigType (implemented by generator) */
/* @req SWS_Cal_00026 Each configuration shall have a configurable name */
/* @req SWS_Cal_00029 Structure of Cal and the states */
/* @req SWS_Cal_00028 Configure all available and allowed schemes and underlying cryptographic primitives*/
/* @req SWS_Cal_00030 It shall be checked during configuration that only valid service configurations are chosen */
/* @req SWS_Cal_00063 CAL functions should check error when input parameters if there are error cases*/
/* @req SWS_Cal_00064 The API parameters shall be checked in the order in which they are passed*/
/* @req SWS_Cal_00489 Errors should be evaluated for each API call according to the table */
/* @req SWS_Cal_00506 The CAL shall use the interfaces of the incorporated cryptographic library modules to calculate the result of a cryptographic service */
/* @req SWS_Cal_00539 Return API return value if error conditions are met*/
/* @req SWS_Cal_00738 Each .c file should include MemMap.h*/
/* @req SWS_Cal_00739 Each implementation <library>*.c, that uses AUTOSAR integer data types, shall include the header file Std_Types.h */
/* !req SWS_Cal_00740 All the AUTOSAR library Modules should use the AUTOSAR data types insetad of native C data types*/
/* !req SWS_Cal_00741 All the AUTOSAR library Modules should avoid direct use of compiler and platform specific keyword*/

#include "Cal.h"
#include "Cpl_Mac.h"
#include "MemMap.h"

extern const Cal_MacGenerateConfigType CalMacGenerateConfig[ ];
extern const Cal_MacVerifyConfigType CalMacVerifyConfig[ ];

/*
 * Public functions
 */

/* @req SWS_Cal_00046 Cal_<Service>Start shall sheck the validity of the configuration it receives */
/* @req SWS_Cal_00047 Cal_<Service>Start shall configure CAL, set the status and store the status in the context buffer */
/* @req SWS_Cal_00488 If an error is detected, return CAL_E_NOT_OK */
Cal_ReturnType Cal_MacGenerateStart(Cal_ConfigIdType cfgId, Cal_MacGenerateCtxBufType contextBuffer, const Cal_SymKeyType* keyPtr) {
    // Check the cfgId

    Cal_ReturnType returnVal;
    returnVal = CAL_E_OK;
    if ((cfgId >= CalSizeOfGenerateConfig) || (keyPtr == NULL)) {
        returnVal = CAL_E_NOT_OK;
    }else {
        //Set state to active
        contextBuffer[0] = 1;

        // Call the Cpl_MacGenerateStart
        returnVal = CalMacGenerateConfig[cfgId].MacGenerateStartFunction((const void *)CalMacGenerateConfig[cfgId].MacGenerateConfigPtr, contextBuffer, keyPtr);
    }
    return returnVal;
}

/* @req SWS_Cal_00050 Cal_<Service>Update shall check whether the service is initialized */
/* @req SWS_Cal_00051 The CAL shall assume that the data provided to Cal_<Service>Update will not change until it returns */
/* @req SWS_Cal_00052 If the service is initialized, process the data, set the status active, store data in context buffer and return status */
/* @req SWS_Cal_00054 The CAL shall allow the application to call the update function arbitrarily often */
/* @req SWS_Cal_00488 If an error is detected, return CAL_E_NOT_OK */
Cal_ReturnType Cal_MacGenerateUpdate(Cal_ConfigIdType cfgId, Cal_MacGenerateCtxBufType contextBuffer, const uint8* dataPtr, uint32 dataLength) {
    //check if contextBuffer is "idle"

    Cal_ReturnType returnVal;
    returnVal = CAL_E_OK;
    if ((cfgId >= CalSizeOfGenerateConfig) || (contextBuffer[0] == 0) || (dataPtr == NULL)) {
        returnVal = CAL_E_NOT_OK;
    } else {
        // Call the Cpl_MacGenerateUpdate
        returnVal =  CalMacGenerateConfig[cfgId].MacGenerateUpdateFunction((const void *)CalMacGenerateConfig[cfgId].MacGenerateConfigPtr, contextBuffer, dataPtr, dataLength);
    }
    return returnVal;
}
/* @req SWS_Cal_00056 Cal_<Service>Finish shall check whether the service is initialized */
/* @req SWS_Cal_00057 The CAL shall assume that the data provided to Cal_<Service>Finish will not change until it returns */
/* @req SWS_Cal_00058 If the service is initialized, process the data, set the status to idle, store result in result buffer and return status */
/* @req SWS_Cal_00662 The CAL shall check if the provided buffer is large enough to hold the result of the computation */
/* @req SWS_Cal_00488 If an error is detected, return CAL_E_NOT_OK */
Cal_ReturnType Cal_MacGenerateFinish(Cal_ConfigIdType cfgId, Cal_MacGenerateCtxBufType contextBuffer, uint8* resultPtr, uint32* resultLengthPtr, boolean TruncationIsAllowed) {

    Cal_ReturnType returnVal;
    returnVal = CAL_E_OK;
    if ((cfgId >= CalSizeOfGenerateConfig) || (contextBuffer[0] == 0) || (resultPtr == NULL) || (resultLengthPtr == NULL)) {
        returnVal = CAL_E_NOT_OK;
    } else {
        uint32 tempResultLength = *resultLengthPtr;

        // Call the Cpl_MacGenerateFinish
        returnVal = CalMacGenerateConfig[cfgId].MacGenerateFinishFunction((const void *)CalMacGenerateConfig[cfgId].MacGenerateConfigPtr, contextBuffer, resultPtr, resultLengthPtr, TruncationIsAllowed);

        if (returnVal == CAL_E_OK) {
            contextBuffer[0] = 0;
        }
        if((*resultLengthPtr > tempResultLength) && (FALSE == TruncationIsAllowed)) {
            returnVal = CAL_E_SMALL_BUFFER;
        }
    }
    return returnVal;
}

/* @req SWS_Cal_00046 Cal_<Service>Start shall sheck the validity of the configuration it receives */
/* @req SWS_Cal_00047 Cal_<Service>Start shall configure CAL, set the status and store the status in the context buffer */
/* @req SWS_Cal_00488 If an error is detected, return CAL_E_NOT_OK */
Cal_ReturnType Cal_MacVerifyStart(Cal_ConfigIdType cfgId, Cal_MacVerifyCtxBufType contextBuffer, const Cal_SymKeyType* keyPtr) {

    Cal_ReturnType returnVal;
    returnVal = CAL_E_OK;
    if ((cfgId >= CalSizeOfGenerateConfig) || (keyPtr == NULL)) {
        returnVal = CAL_E_NOT_OK;
    } else {
        //Set status to active
        contextBuffer[0] = 1;
        // Call the Cpl_MacVerifyStart
        returnVal =  CalMacVerifyConfig[cfgId].MacVerifyStartFunction((const void *)CalMacVerifyConfig[cfgId].MacVerifyConfigPtr, contextBuffer, keyPtr);
    }
    return returnVal;
}

/* @req SWS_Cal_00050 Cal_<Service>Update shall check whether the service is initialized */
/* @req SWS_Cal_00051 The CAL shall assume that the data provided to Cal_<Service>Update will not change until it returns */
/* @req SWS_Cal_00052 If the service is initialized, process the data, set the status active, store data in context buffer and return status */
/* @req SWS_Cal_00054 The CAL shall allow the application to call the update function arbitrarily often */
/* @req SWS_Cal_00478 As the CAL is a library, all functions have to be reentrant */
/* @req SWS_Cal_00488 If an error is detected, return CAL_E_NOT_OK */
Cal_ReturnType Cal_MacVerifyUpdate(Cal_ConfigIdType cfgId, Cal_MacVerifyCtxBufType contextBuffer, const uint8* dataPtr, uint32 dataLength) {

    Cal_ReturnType returnVal;
    returnVal = CAL_E_OK;
    if ((cfgId >= CalSizeOfGenerateConfig) || (contextBuffer[0] == 0) || (dataPtr == NULL)) {
        returnVal = CAL_E_NOT_OK;
    } else {
        // Call the Cpl_MacVerifyUpdate
        returnVal =  CalMacVerifyConfig[cfgId].MacVerifyUpdateFunction((const void *)CalMacVerifyConfig[cfgId].MacVerifyConfigPtr, contextBuffer,dataPtr,dataLength);
    }
    return returnVal;
}

/* @req SWS_Cal_00056 Cal_<Service>Finish shall check whether the service is initialized */
/* @req SWS_Cal_00057 The CAL shall assume that the data provided to Cal_<Service>Finish will not change until it returns */
/* @req SWS_Cal_00058 If the service is initialized, process the data, set the status to idle, store result in result buffer and return status */
/* @req SWS_Cal_00488 If an error is detected, return CAL_E_NOT_OK */
Cal_ReturnType Cal_MacVerifyFinish(Cal_ConfigIdType cfgId, Cal_MacVerifyCtxBufType contextBuffer, const uint8* MacPtr, uint32 MacLength, Cal_VerifyResultType* resultPtr) {

    Cal_ReturnType returnVal;
    returnVal = CAL_E_OK;
    if ((cfgId >= CalSizeOfGenerateConfig)|| (contextBuffer[0] == 0) || (resultPtr == NULL)) {
        returnVal = CAL_E_NOT_OK;
    } else {
        // Call the Cpl_MacVerifyFinish
        returnVal =  CalMacVerifyConfig[cfgId].MacVerifyFinishFunction((const void *)CalMacVerifyConfig[cfgId].MacVerifyConfigPtr, contextBuffer,MacPtr,MacLength,resultPtr);

        //Set status to "idle"
        contextBuffer[0] = 0;
    }
    return returnVal;
}


