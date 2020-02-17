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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.3 */

#ifndef XCP_CALLOUT_STUBS_H_
#define XCP_CALLOUT_STUBS_H_

#include "Xcp_Internal.h"

/**
 * Function is used to initialize MTA (Memory Transfer Address) and its proper functions
 * which shall be used when XCP reads and writes ceratin memory sections
 * @param mta (output) Memory Transfer Address
 * @param address (input) This value is sent by XCP Master tool
 * @param extension (input) This value is sent by XCP Master tool
 */
extern void Xcp_Arc_MtaInit(
        Xcp_MtaType* mta,
        uint32 address,
        uint8 extension);

/**
 * This function is responsible to calculate the intended checksum algorithm, at the given example
 * XCP_CHECKSUM_ADD_11 is used
 * @param mtaPtr
 * @param block (input) length of the block where the CRC is calculated
 * @param type (output) type of the CRC calculation.
 * @param response (output) the result of the CRC calculation over the block
 */
extern void Xcp_Arc_BuildChecksum(
        Xcp_MtaType*      mtaPtr,
        uint32            block,
        Xcp_ChecksumType* type,
        uint32*           response);

#if (XCP_FEATURE_PROTECTION == STD_ON)

/**
 * Function called to retrieve seed that should be
 * sent to master in a GetSeed/Unlock exchange
 * @param res is the resource requested
 * @param seed pointer to buffer that will hold seed (255 bytes max)
 * @return number of bytes in seed
 */
extern  uint8 Xcp_Arc_GetSeed(
        Xcp_ProtectType res,
        uint8*          seed);

/**
 * Function used for Seed & Key unlock
 * @param res is the resource requested to be unlocked
 * @param seed is the seed that was sent to the master
 * @param seed_len is the length of @param seed
 * @param key is the key sent from master
 * @param key_len is the length of @param key
 * @return E_OK for success, E_ERR for failure
 */
extern Std_ReturnType Xcp_Arc_Unlock(
        Xcp_ProtectType res,
        const uint8*    seed,
        uint8           seed_len,
        const uint8*    key,
        uint8           key_len);

#endif /* XCP_FEATURE_PROTECTION == STD_ON */

#if(XCP_FEATURE_CALPAG == STD_ON)

/**
 *
 * @param mode
 * @param segm
 * @param page
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_SetCalPage(
        const uint8 mode,
        const uint8 segm,
        const uint8 page,
        Xcp_ErrorType *error);

/**
 *
 * @param mode
 * @param segm
 * @param page
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_GetCalPage(
        const uint8 mode,
        const uint8 segm,
              uint8 *page,
              Xcp_ErrorType *error);

/**
 *
 * @param sourceSegm
 * @param sourcePage
 * @param destSegm
 * @param destPage
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_CopyCalPage(
        const uint8 sourceSegm,
        const uint8 sourcePage,
        const uint8 destSegm,
        const uint8 destPage,
        Xcp_ErrorType *error);

/**
 *
 * @param maxSegment
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_GetPagProcessorInfo(
              uint8 *maxSegment,
              Xcp_ErrorType *error);

/**
 *
 * @param segmentNumber
 * @param segmentInfo
 * @param basicInfo
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_GetSegmentInfo_GetBasicAddress(
        const uint8 segmentNumber,
        const uint8 segmentInfo,
              uint32 *basicInfo,
              Xcp_ErrorType *error);

/**
 *
 * @param segmentNumber
 * @param responseData
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_GetSegmentInfo_GetStandardInfo(
        const uint8 segmentNumber,
              Xcp_SegmentStandardInfoReturnType *responseData,
              Xcp_ErrorType *error);

/**
 *
 * @param segmentNumber
 * @param segmentInfo
 * @param mappingInfo
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_GetSegmentInfo_GetAddressMapping(
        const uint8 segmentNumber,
        const uint8 segmentInfo,
              uint32 *mappingInfo,
              Xcp_ErrorType *error);

/**
 *
 * @param segmentNumber
 * @param pageNumber
 * @param pageProperties
 * @param initSegment
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_GetPageInfo(
        const uint8 segmentNumber,
        const uint8 pageNumber,
              uint8 *pageProperties,
              uint8 *initSegment,
              Xcp_ErrorType *error);

/**
 *
 * @param mode
 * @param segmentNumber
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_SetSegmentMode(
        const uint8 mode,
        const uint8 segmentNumber,
        Xcp_ErrorType *error);

/**
 *
 * @param segmentNumber
 * @param mode
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_GetSegmentMode(
        const uint8 segmentNumber,
              uint8 *mode,
              Xcp_ErrorType *error);

#endif /*XCP_FEATURE_CALPAG == STD_ON*/

#if(XCP_FEATURE_PGM == STD_ON)
/**
 *
 * @param clearRange
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_ProgramClear_FunctionalAccess(
        const uint32 clearRange,
        Xcp_ErrorType *error);

/**
 *
 * @param mta
 * @param clearRange
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_ProgramClear_AbsoluteAccess(
        Xcp_MtaType* mta,
        const uint32 clearRange,
        Xcp_ErrorType *error);
/**
 *
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_ProgramReset(
        Xcp_ErrorType *error);

/**
 *
 * @param pgmProperties
 * @param maxSector
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_GetPgmProcessorInfo(
        uint8 *pgmProperties,
        uint8 *maxSector,
        Xcp_ErrorType *error);

/**
 *
 * @param mode
 * @param sectorNumber
 * @param responseData
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_GetSectorInfo_Mode0_1(
        const uint8 mode,
        const uint8 sectorNumber,
        Xcp_GetSectorInfo_Mode0_1_ReturnType *responseData,
        Xcp_ErrorType *error);

/**
 *
 * @param sectorNameLength
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_GetSectorInfo_Mode2(
        uint8 *sectorNameLength,
        Xcp_ErrorType *error);

/**
 *
 * @param mta
 * @param codeSize
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_ProgramPrepare(
        Xcp_MtaType* mta,
        uint16 codeSize,
        Xcp_ErrorType *error);

/**
 *
 * @param compressionMethod
 * @param encryptionMethod
 * @param programmingMethod
 * @param accessMethod
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_ProgramFormat(
        const uint8 compressionMethod,
        const uint8 encryptionMethod,
        const uint8 programmingMethod,
        const uint8 accessMethod,
        Xcp_ErrorType *error);

/**
 *
 * @param mode
 * @param type
 * @param value
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_ProgramVerify(
        const uint8 mode,
        const uint8 type,
        const uint8 value,
        Xcp_ErrorType *error);

/**
 * Flash algorithm must be place here, Post increment of the MTA shall be performed here
 * @param mta
 * @param data
 * @param len
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_Program(
        Xcp_MtaType* mta,
        uint8* data,
        uint32 len,
        Xcp_ErrorType *error);

#endif /*XCP_FEATURE_PGM == STD_ON*/

/**
 * This function together with Xcp_Arc_RecievePduAllowed is used to dynamically allow certain Pdus to receive and transmit messages.
 * With the help of these functions the user can enable, disable certain communication channels, and in this way different session handling of
 * Xcp can be performed.
 * @param XcpTxPduId  This Pdu Id is intended to use to send out Xcp message
 * @return TRUE  XcpTxPduId is allowed to use in this session
 * 		   FALSE XcpTxPduId is not allowed to use in this session
 */
boolean Xcp_Arc_TransmitPduAllowed(PduIdType XcpTxPduId);

/**
 * This function together with Xcp_Arc_TransmitPduAllowed is used to dynamically allow certain Pdus to receive and transmit messages.
 * With the help of these functions the user can enable, disable certain communication channels, and in this way different session handling of
 * Xcp can be performed.
 * @param XcpRxPduId  This Pdu Id is intended to use to process incoming Xcp message
 * @param XcpRxPduPtr   Pointer to the incoming Xcp messages
 * @return TRUE  XcpRxPduId is allowed to use in this session
 * 		   FALSE XcpRxPduId is not allowed to use in this session
 */
boolean Xcp_Arc_RecievePduAllowed(PduIdType XcpRxPduId, PduInfoType* XcpRxPduPtr);

#endif /* XCP_CALLOUT_STUBS_H_ */
