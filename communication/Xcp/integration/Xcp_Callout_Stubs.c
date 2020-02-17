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

#include "Xcp_Callout_Stubs.h"
#include <string.h>

/** This type encapsulate a possible Address Extension Type, normally this information is coming from Xcp Master */
typedef enum {
    /** These are freely customizable */
    XCP_MTA_EXTENSION_MEMORY   = 0u,
    XCP_MTA_EXTENSION_FLASH    = 1u,
    XCP_MTA_EXTENSION_DIO_PORT = 2u,
    XCP_MTA_EXTENSION_DIO_CHAN = 3u,

    /** This kind of memory extension is used by XCP.c for reading or writing some general information
     * from XCP reserved memory (Ram), or reading from XCP Config (Flash) */
    XCP_MTA_EXTENSION_MEMORY_INTERNAL    = 0xFFu,
} Xcp_MtaExtentionType;

/** These example functions are used to read or write different memory sections */
static uint8 Xcp_MtaGetMemory(Xcp_MtaType* mta);
static void Xcp_MtaReadMemory(Xcp_MtaType* mta, uint8* data, uint32 len);
static void Xcp_MtaWriteMemory(Xcp_MtaType* mta, uint8* data, uint32 len);

/** Read a character from MTA */
static uint8 Xcp_MtaGetMemory(Xcp_MtaType* mta)
{
    /** Post increment of the MTA shall be performed here*/
    return *(uint8*)(mta->address++);
}

/** Read a character from memory */
static void Xcp_MtaReadMemory(Xcp_MtaType* mta, uint8* data, uint32 len)
{
    Xcp_MemCpy(data, (void*)mta->address, len);

    /** Post increment of the MTA shall be performed here*/
    mta->address += len;
}

/** Write a character to memory */
static void Xcp_MtaWriteMemory(Xcp_MtaType* mta, uint8* data, uint32 len)
{
    Xcp_MemCpy((void*)mta->address, data, len);

    /** Post increment of the MTA shall be performed here*/
    mta->address += len;
}

/**
 * Function is used to initialize MTA (Memory Transfer Address) and its proper functions
 * which shall be used when XCP reads and writes ceratin memory sections
 * @param mta (output) Memory Transfer Address
 * @param address (input) This value is sent by XCP Master tool
 * @param extension (input) This value is sent by XCP Master tool
 */
void Xcp_Arc_MtaInit(
        Xcp_MtaType* mta,
        uint32 address,
        uint8 extension)
{
	mta->address   = address;
	mta->extension = extension;

	if( (extension == (uint8)XCP_MTA_EXTENSION_MEMORY)          ||
	    (extension == (uint8)XCP_MTA_EXTENSION_MEMORY_INTERNAL)  )
	{
		mta->get   = Xcp_MtaGetMemory;
		mta->read  = Xcp_MtaReadMemory;
		mta->write = Xcp_MtaWriteMemory;
		mta->flush = NULL;
	}
	else if(extension == (uint8)XCP_MTA_EXTENSION_FLASH)
	{
		mta->get   = Xcp_MtaGetMemory;
		mta->read  = Xcp_MtaReadMemory;
		mta->write = NULL;
		mta->flush = NULL;
	}
	else
	{
		mta->get   = NULL;
		mta->read  = NULL;
		mta->write = NULL;
		mta->flush = NULL;
	}

}

/**
 * This function is responsible to calculate the intended checksum algorithm, at the given example
 * XCP_CHECKSUM_ADD_11 is used
 * @param mtaPtr
 * @param block (input) length of the block where the CRC is calculated
 * @param type (output) type of the CRC calculation.
 * @param response (output) the result of the CRC calculation over the block
 */
void Xcp_Arc_BuildChecksum(
        Xcp_MtaType*      mtaPtr,
        uint32            block,
        Xcp_ChecksumType* type,
        uint32*           response)
{
    *type = XCP_CHECKSUM_ADD_11;
    *response = 0u;

    for (uint32 i = 0; i < block; i++)
    {
        *response += mtaPtr->get(mtaPtr);
    }

    /** Post increment of the MTA shall be performed here*/
    mtaPtr->address += block;
}

#if (XCP_FEATURE_PROTECTION == STD_ON)

/**
 * Function called to retrieve seed that should be
 * sent to master in a GetSeed/Unlock exchange
 * @param res is the resource requested
 * @param seed pointer to buffer that will hold seed (255 bytes max)
 * @return number of bytes in seed
 */
uint8 Xcp_Arc_GetSeed(
        Xcp_ProtectType res,
        uint8*          seed)
{
    (void)(res);
    strcpy((char*)seed, "HELLO_BELLO");
    return strlen((const char*)seed);
}

/**
 * Function used for Seed & Key unlock
 * @param res is the resource requested to be unlocked
 * @param seed is the seed that was sent to the master
 * @param seed_len is the length of @param seed
 * @param key is the key sent from master
 * @param key_len is the length of @param key
 * @return E_OK for success, E_ERR for failure
 */
Std_ReturnType Xcp_Arc_Unlock(
        Xcp_ProtectType res,
        const uint8*    seed,
        uint8           seed_len,
        const uint8*    key,
        uint8 key_len)
{
    (void)(res);
    if(seed_len != key_len)
        return E_NOT_OK; /*lint !e904 allow multiple exit */
    if(memcmp(seed, key, seed_len))
        return E_NOT_OK; /*lint !e904 allow multiple exit */
    return E_OK;
}

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
        Xcp_ErrorType *error)
{
    XCP_UNUSED(mode);
    XCP_UNUSED(segm);
    XCP_UNUSED(page);
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}

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
              Xcp_ErrorType *error)
{
    XCP_UNUSED(mode);
    XCP_UNUSED(segm);
    XCP_UNUSED(page); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}

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
        Xcp_ErrorType *error)
{
    XCP_UNUSED(sourceSegm);
    XCP_UNUSED(sourcePage);
    XCP_UNUSED(destSegm);
    XCP_UNUSED(destPage);
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}

/**
 *
 * @param maxSegment
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_GetPagProcessorInfo(
         uint8 *maxSegment,
         Xcp_ErrorType *error)
{
    XCP_UNUSED(maxSegment); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}

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
              Xcp_ErrorType *error)
{
    XCP_UNUSED(segmentNumber);
    XCP_UNUSED(segmentInfo);
    XCP_UNUSED(basicInfo); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}


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
              Xcp_ErrorType *error)
{
    XCP_UNUSED(segmentNumber);
    XCP_UNUSED(responseData); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}

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
              Xcp_ErrorType *error)
{
    XCP_UNUSED(segmentNumber);
    XCP_UNUSED(segmentInfo);
    XCP_UNUSED(mappingInfo); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}

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
              Xcp_ErrorType *error)
{
    XCP_UNUSED(segmentNumber);
    XCP_UNUSED(pageNumber);
    XCP_UNUSED(pageProperties); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
    XCP_UNUSED(initSegment); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}


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
        Xcp_ErrorType *error)
{
    XCP_UNUSED(mode);
    XCP_UNUSED(segmentNumber);
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}


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
              Xcp_ErrorType *error)
{
    XCP_UNUSED(segmentNumber);
    XCP_UNUSED(mode); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}

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
        Xcp_ErrorType *error)
{
    XCP_UNUSED(clearRange);
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}

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
        Xcp_ErrorType *error)
{
    XCP_UNUSED(mta); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
    XCP_UNUSED(clearRange);
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}
/**
 *
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_ProgramReset(
        Xcp_ErrorType *error)
{
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}

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
        Xcp_ErrorType *error)
{
    XCP_UNUSED(pgmProperties); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
    XCP_UNUSED(maxSector); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}

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
        Xcp_ErrorType *error)
{
    XCP_UNUSED(mode);
    XCP_UNUSED(sectorNumber);
    XCP_UNUSED(responseData); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}

/**
 *
 * @param sectorNameLength
 * @param error
 * @return
 */
Std_ReturnType Xcp_Arc_GetSectorInfo_Mode2(
        uint8 *sectorNameLength,
        Xcp_ErrorType *error)
{
    XCP_UNUSED(sectorNameLength); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}

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
        Xcp_ErrorType *error)
{
    XCP_UNUSED(mta); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
    XCP_UNUSED(codeSize);
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}

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
        Xcp_ErrorType *error)
{
    XCP_UNUSED(compressionMethod);
    XCP_UNUSED(encryptionMethod);
    XCP_UNUSED(programmingMethod);
    XCP_UNUSED(accessMethod);
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}

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
        Xcp_ErrorType *error)
{
    XCP_UNUSED(mode);
    XCP_UNUSED(type);
    XCP_UNUSED(value);
    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}

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
        Xcp_ErrorType *error)
{
    XCP_UNUSED(mta); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
    XCP_UNUSED(data); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
    XCP_UNUSED(len);

    /** Post increment of the MTA shall be performed here*/
    //mta->address += len;

    *error = XCP_ERR_CMD_UNKNOWN;
    return E_NOT_OK;
}

#endif /*XCP_FEATURE_PGM == STD_ON*/

/**
 * This function together with Xcp_Arc_RecievePduAllowed is used to dynamically allow certain Pdus to receive and transmit messages.
 * With the help of these functions the user can enable, disable certain communication channels, and in this way different session handling of
 * Xcp can be performed.
 * @param XcpTxPduId  This Pdu Id is intended to use to send out Xcp message
 * @return TRUE  XcpTxPduId is allowed to use in this session
 * 		   FALSE XcpTxPduId is not allowed to use in this session
 */
boolean Xcp_Arc_TransmitPduAllowed(PduIdType XcpTxPduId) {
    XCP_UNUSED(XcpTxPduId);
    return TRUE;
}

/**
 * This function together with Xcp_Arc_TransmitPduAllowed is used to dynamically allow certain Pdus to receive and transmit messages.
 * With the help of these functions the user can enable, disable certain communication channels, and in this way different session handling of
 * Xcp can be performed.
 * @param XcpRxPduId  This Pdu Id is intended to use to process incoming Xcp message
 * @param XcpRxPduPtr   Pointer to the incoming Xcp messages
 * @return TRUE  XcpRxPduId is allowed to use in this session
 * 		   FALSE XcpRxPduId is not allowed to use in this session
 */
boolean Xcp_Arc_RecievePduAllowed(PduIdType XcpRxPduId, PduInfoType* XcpRxPduPtr) {
    XCP_UNUSED(XcpRxPduId);
    XCP_UNUSED(XcpRxPduPtr); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
    return TRUE;
}
