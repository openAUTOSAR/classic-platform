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

#include "Xcp_Cfg.h"

#ifndef XCP_CONFIGTYPES_H_
#define XCP_CONFIGTYPES_H_

#define XCP_IDENTIFICATION_ABSOLUTE              0x0u
#define XCP_IDENTIFICATION_RELATIVE_BYTE         0x1u
#define XCP_IDENTIFICATION_RELATIVE_WORD         0x2u
#define XCP_IDENTIFICATION_RELATIVE_WORD_ALIGNED 0x3u

/* ERROR CODES */
typedef enum {
    XCP_ERR_CMD_SYNCH         = 0x00u,
    XCP_ERR_CMD_BUSY          = 0x10u,
    XCP_ERR_DAQ_ACTIVE        = 0x11u,
    XCP_ERR_PGM_ACTIVE        = 0x12u,

    XCP_ERR_CMD_UNKNOWN       = 0x20u,
    XCP_ERR_CMD_SYNTAX        = 0x21u,
    XCP_ERR_OUT_OF_RANGE      = 0x22u,
    XCP_ERR_WRITE_PROTECTED   = 0x23u,
    XCP_ERR_ACCESS_DENIED     = 0x24u,
    XCP_ERR_ACCESS_LOCKED     = 0x25u,
    XCP_ERR_PAGE_NOT_VALID    = 0x26u,
    XCP_ERR_MODE_NOT_VALID    = 0x27u,
    XCP_ERR_SEGMENT_NOT_VALID = 0x28u,
    XCP_ERR_SEQUENCE          = 0x29u,
    XCP_ERR_DAQ_CONFIG        = 0x2Au,

    XCP_ERR_MEMORY_OVERFLOW   = 0x30u,
    XCP_ERR_GENERIC           = 0x31u,
    XCP_ERR_VERIFY            = 0x32u,
} Xcp_ErrorType;

typedef enum {
    XCP_TIMESTAMP_UNIT_1NS = 0x00u,
    XCP_TIMESTAMP_UNIT_10NS = 0x01u,
    XCP_TIMESTAMP_UNIT_100NS = 0x02u,
    XCP_TIMESTAMP_UNIT_1US = 0x03u,
    XCP_TIMESTAMP_UNIT_10US = 0x04u,
    XCP_TIMESTAMP_UNIT_100US = 0x05u,
    XCP_TIMESTAMP_UNIT_1MS = 0x06u,
    XCP_TIMESTAMP_UNIT_10MS = 0x07u,
    XCP_TIMESTAMP_UNIT_100MS = 0x08u,
    XCP_TIMESTAMP_UNIT_1S = 0x09u,
    XCP_TIMESTAMP_UNIT_1PS = 0x0Au,
    XCP_TIMESTAMP_UNIT_10PS = 0x0Bu,
    XCP_TIMESTAMP_UNIT_100PS = 0x0Cu,
} Xcp_TimestampUnitType;

typedef enum  {
    XCP_CHECKSUM_ADD_11      = 0x01u,
    XCP_CHECKSUM_ADD_12      = 0x02u,
    XCP_CHECKSUM_ADD_14      = 0x03u,
    XCP_CHECKSUM_ADD_22      = 0x04u,
    XCP_CHECKSUM_ADD_24      = 0x05u,
    XCP_CHECKSUM_ADD_44      = 0x06u,
    XCP_CHECKSUM_CRC_16      = 0x07u,
    XCP_CHECKSUM_CRC_16_CITT = 0x08u,
    XCP_CHECKSUM_CRC_32      = 0x09u,
    XCP_CHECKSUM_USERDEFINE  = 0xFFu,
} Xcp_ChecksumType;


typedef struct Xcp_SegmentStandardInfoReturnType {
    uint8 maxPages;
    uint8 addressExtension;
    uint8 maxMapping;
    uint8 compressionMethod;
    uint8 encryptionMethod;
} Xcp_SegmentStandardInfoReturnType;

typedef struct Xcp_GetSectorInfo_Mode0_1_ReturnType {
    uint8 clearSeqNum;
    uint8 programSeqNum;
    uint8 programMethod;
    uint32 sectorInfo;
} Xcp_GetSectorInfo_Mode0_1_ReturnType;

typedef struct Xcp_MtaType {
    uint8   (*get)  (struct Xcp_MtaType* mta);
    void    (*write)(struct Xcp_MtaType* mta, uint8* data, uint32 len);
    void    (*read) (struct Xcp_MtaType* mta, uint8* data, uint32 len);
    void    (*flush)(struct Xcp_MtaType* mta);
    uint32   address;
    uint8    extension;
} Xcp_MtaType;

typedef struct Xcp_OdtEntryType {
    uint32 XcpOdtEntryAddress;
    uint8  XcpOdtEntryExtension;
    uint8  XcpOdtEntryLength;
    uint8  BitOffSet;
} Xcp_OdtEntryType;

struct Xcp_BufferType;

typedef struct Xcp_OdtType {
    Xcp_OdtEntryType* XcpOdtEntry; /* 0 .. * */
    uint8 XcpOdtEntriesCount; /* 0 .. 255 */
} Xcp_OdtType;

#define XCP_DAQLIST_MODE_SELECTED  (1u << 0u)
#define XCP_DAQLIST_MODE_STIM      (1u << 1u)
#define XCP_DAQLIST_MODE_TIMESTAMP (1u << 4u)
#define XCP_DAQLIST_MODE_PIDOFF    (1u << 5u)
#define XCP_DAQLIST_MODE_RUNNING   (1u << 6u)
#define XCP_DAQLIST_MODE_RESUME    (1u << 7u)
typedef uint8 Xcp_DaqListModeEnum;


#define XCP_DAQLIST_PROPERTY_EVENTFIXED (1u << 0u)
#define XCP_DAQLIST_PROPERTY_PREDEFINED (1u << 1u)
#define XCP_DAQLIST_PROPERTY_DAQ        (1u << 2u)
#define XCP_DAQLIST_PROPERTY_STIM       (1u << 3u)
#define XCP_DAQLIST_PROPERTY_DAQ_STIM   ((1u << 2u)|(1u << 3u))
typedef uint8 Xcp_DaqListPropertyEnum;

typedef const struct {
    /**
     * Pointer to an array of ODT structures this DAQ list will use
     *   [USER]: With static DAQ lists, this needs to be set
     *           to an array of XcpMaxOdt size.
     *   [INTERNAL]: With dynamic DAQ configuration.
     */
    Xcp_OdtType *XcpOdt; /**< reference to an array of Odt's configured for this Daq list */
    /**
     * Fixed eventchannel for this daq or 0xFFFF
     * 0 .. NumEvents-1
     */
    uint16 FixedEventChannel;
    /**
     * Maximum number of ODT's in XcpOdt array
     *   [USER]    : When static DAQ configuration
     *   [INTERNAL]: Dynamic DAQ configuration.
     * 0 .. 252
     */
    uint8 XcpMaxOdt;

    Xcp_DaqListPropertyEnum  Properties; /**< bitfield for the properties of the DAQ list */

} Xcp_DaqListCfgType;

typedef struct Xcp_DaqListType {
    /**
     * Pointer to an array of ODT structures this DAQ list will use
     *   [USER]: With static DAQ lists, this needs to be set
     *           to an array of XcpMaxOdt size.
     *   [INTERNAL]: With dynamic DAQ configuration.
     */
    Xcp_OdtType *XcpOdt; /**< reference to an array of Odt's configured for this Daq list */

    /**
     * Used to create list of ongoing transmissions
     *   [INTERNAL/USER]
     */
    struct Xcp_DaqListType *TxDaqListNext;

    /**
     * Next ptr of the event daq list
     */
    struct Xcp_DaqListType *EventDaqListNext;

    /**
     * Used to create list of ongoing transmissions
     *   [INTERNAL/USER]
     */
    Xcp_OdtType *NextOdtToSend;

    /**
     * Maximum number of ODT's in XcpOdt array
     *   [USER]    : When static DAQ configuration
     *   [INTERNAL]: Dynamic DAQ configuration.
     * 0 .. 252
     */
    uint8 XcpMaxOdt;

    Xcp_DaqListModeEnum     Mode; /**< bitfield for the current mode of the DAQ list */
    uint16 EventChannel; /* NOTE: Fixed channel vs current */
    uint8 Prescaler;
    uint8 Timer; // to be used with prescaler
    uint8 Priority;
    uint8 FirstPid;
} Xcp_DaqListType;

typedef enum {
    XCP_EVENTCHANNEL_PROPERTY_DAQ  = 1u << 2u,
    XCP_EVENTCHANNEL_PROPERTY_STIM = 1u << 3u,
    XCP_EVENTCHANNEL_PROPERTY_ALL  = XCP_EVENTCHANNEL_PROPERTY_DAQ | XCP_EVENTCHANNEL_PROPERTY_STIM,
} Xcp_EventChannelPropertyEnum;

typedef struct {
    /**
     * Set to the name of the eventchannel or NULL
     *   [USER]
     */
    const char* XcpEventChannelName;

    /**
     * Priority of event channel (0 .. 255)
     *   [IGNORED]
     */
    const uint8 XcpEventChannelPriority;

    /**
     * Maximum number of entries in XcpEventChannelTriggeredDaqListRef
     *   [USER]
     *
     * 1 .. 255
     * 0 = Unlimited
     */
    const uint8 XcpEventChannelMaxDaqList;

    /**
     * Bitfield defining supported features
     *   [USER]
     */
    const Xcp_EventChannelPropertyEnum XcpEventChannelProperties;

    /**
     * Cycle unit of event channel
     *   [USER]
     *
     * Set to 0 (XCP_TIMESTAMP_UNIT_1NS) if channel is not
     * cyclic.
     */
    const Xcp_TimestampUnitType XcpEventChannelUnit;

    /**
     * Number of cycle units between each trigger of event
     *   [USER]
     *
     * 0 .. 255
     * 0 -> non cyclic
     */
    const uint8 XcpEventChannelRate;
} Xcp_EventChannelType;


#define	XCP_ACCESS_ECU_ACCESS_WITHOUT_XCP       (1u << 0u)
#define	XCP_ACCESS_ECU_ACCESS_WITH_XCP          (1u << 1u)
#define	XCP_ACCESS_XCP_READ_ACCESS_WITHOUT_ECU  (1u << 2u)
#define	XCP_ACCESS_READ_ACCESS_WITH_ECU         (1u << 3u)
#define	XCP_ACCESS_XCP_WRITE_ACCESS_WITHOUT_ECU (1u << 4u)
#define	XCP_ACCESS_XCP_WRITE_ACCESS_WITH_ECU    (1u << 5u)
#define	XCP_ACCESS_ALL                          0x3fu
typedef uint8  Xcp_AccessFlagsType;


#define XCP_PROTECT_NONE    (uint8)(0u)
#define XCP_PROTECT_CALPAG  (uint8)(1u << 0u)
#define XCP_PROTECT_DAQ     (uint8)(1u << 2u)
#define XCP_PROTECT_STIM    (uint8)(1u << 3u)
#define XCP_PROTECT_PGM     (uint8)(1u << 4u)
typedef uint8 Xcp_ProtectType;


typedef enum {
    XCP_COMPRESSION_METHOD_NONE = 0u,
} Xcp_CompressType;

typedef enum {
    XCP_ENCRYPTION_METHOD_NONE = 0u,
} Xcp_EncryptionType;

typedef struct {
    uint32 XcpSrc;
    uint32 XcpDst;
    uint32 XcpLen;
} Xcp_MemoryMappingType;

typedef struct {
    Xcp_MemoryMappingType* XcpMapping;
    uint32 XcpAddress;
    uint32 XcpLength;
    uint32 XcpMaxMapping;
    uint8  XcpMaxPage;
    uint8  XcpPageXcp;
    uint8  XcpPageEcu;
    uint8  XcpExtension;
    const  Xcp_AccessFlagsType XcpAccessFlags;
    Xcp_CompressType   XcpCompression;
    Xcp_EncryptionType XcpEncryption;
} Xcp_SegmentType;

typedef struct {
    const char* XcpCaption; /**< ASCII text describing device [USER] */
    const char* XcpMC2File; /**< ASAM-MC2 filename without path and extension [USER] */
    const char* XcpMC2Path; /**< ASAM-MC2 filename with path and extension [USER] */
    const char* XcpMC2Url; /**< ASAM-MC2 url to file [USER] */
    const char* XcpMC2Upload; /**< ASAM-MC2 file to upload [USER] */
} Xcp_InfoType;

/** Collection of runtime parameter*/
typedef struct {

    /** Runtime parameter*/
    Xcp_ProtectType XcpProtect;

    /** Runtime parameter*/
    uint16 XcpMaxDaq; /* 0 .. 65535, XCP_MAX_DAQ */

#if(XCP_DAQ_CONFIG_TYPE == DAQ_DYNAMIC)
    /** Runtime parameter*/
    uint32 XcpNumDynWordsAllocated;
#endif /*XCP_DAQ_CONFIG_TYPE == DAQ_DYNAMIC*/

} Xcp_RunTimeType;

/** State of the communication tx channels*/
typedef enum{
    XCP_TX_CH_EMPTY = 0x00u,
    XCP_TX_CH_RESERVED = 0x01u,
} Xcp_TxChStateType;

/** Collection of the configured communication tx channels*/
typedef struct{
#if (XCP_PROTOCOL == XCP_PROTOCOL_CAN)
    uint8 data[XCP_MAX_DTO > XCP_MAX_CTO ? XCP_MAX_DTO : XCP_MAX_CTO];
#elif (XCP_PROTOCOL == XCP_PROTOCOL_ETHERNET)
    uint8 data[XCP_MAX_DTO > XCP_MAX_CTO ? XCP_MAX_DTO + 4u : XCP_MAX_CTO + 4u];
#endif
    PduLengthType len;
    Xcp_TxChStateType state;
    const PduIdType pduId;
} Xcp_TxChType;

/** @req 4.1.3/SWS_Xcp_00845 *//*Xcp_ConfigType definition - can not be tested with conventional module tests*/
typedef struct {
    /** Collection of the configured communication tx channels*/
    Xcp_TxChType *XcpTxCh;

    /** Collection of runtime parameter*/
    Xcp_RunTimeType *rt;

#if(XCP_DAQ_CONFIG_TYPE == DAQ_DYNAMIC)

    /** references for dynamic allocation */
    uint32 *ptrDynamicRamBuffer;

    uint32 dynamicRamBufferSize;

#endif /*XCP_DAQ_CONFIG_TYPE == DAQ_DYNAMIC*/


    /** Pointer to the statically allocated DAQ Lists*/
    Xcp_DaqListType *XcpDaqList;

    /** Pointer to the statically allocated DAQ Lists*/
    const Xcp_DaqListCfgType *XcpDaqListCfg;

    /** Number of statically allocated DAQ Lists*/
    uint16 XcpDaqListSize;

    /**
     * Function is used to initialize MTA (Memory Transfer Address) and its proper functions to read and write
     * that memory sections
     * @param mta (output) Memory Transfer Address
     * @param address (input) This value is sent by XCP Master tool
     * @param extension (input) This value is sent by XCP Master tool
     */
    void (*XcpMtaInit)(
            Xcp_MtaType* mta,
            uint32 address,
            uint8 extension);

    /**
     * Function used for Seed & Key unlock
     * @param res is the resource requested to be unlocked
     * @param seed is the seed that was sent to the master
     * @param seed_len is the length of @param seed
     * @param key is the key sent from master
     * @param key_len is the length of @param key
     * @return E_OK for success, E_ERR for failure
     */
    Std_ReturnType (*XcpUnlockFn)(
            Xcp_ProtectType res,
            const uint8* seed,
            uint8 seed_len,
            const uint8* key,
            uint8 key_len);
    /**
     * Function called to retrieve seed that should be
     * sent to master in a GetSeed/Unlock exchange
     * @param res is the resource requested
     * @param seed pointer to buffer that will hold seed (255 bytes max)
     * @return number of bytes in seed
     */
    uint8 (*XcpSeedFn)(
            Xcp_ProtectType res,
            uint8* seed);

    /**
     * Function called for a XCP user defined call from the master
     * @param data data recieved from master (excluding the preceding 0xF1 for user defined)
     * @param len  length of the data in buffer
     * @return
     */
    Std_ReturnType (*XcpUserFn)(
            void* data,
            PduLengthType len);


#if(XCP_FEATURE_CALPAG == STD_ON)

    /**
     *
     * @param mode
     * @param segm
     * @param page
     * @param error
     * @return
     */
    Std_ReturnType (*XcpSetCalPage)(
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
    Std_ReturnType (*XcpGetCalPage)(
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
    Std_ReturnType (*XcpCopyCalPage)(
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
    Std_ReturnType (*XcpGetPagProcessorInfo)(
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
    Std_ReturnType (*XcpGetSegmentInfo_GetBasicAddress)(
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
    Std_ReturnType (*XcpGetSegmentInfo_GetStandardInfo)(
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
    Std_ReturnType (*XcpGetSegmentInfo_GetAddressMapping)(
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
    Std_ReturnType (*XcpGetPageInfo)(
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
    Std_ReturnType (*XcpSetSegmentMode)(
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
    Std_ReturnType (*XcpGetSegmentMode)(
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
    Std_ReturnType (*XcpProgramClear_FunctionalAccess)(
            const uint32 clearRange,
            Xcp_ErrorType *error);

    /**
     *
     * @param mta
     * @param clearRange
     * @param error
     * @return
     */
    Std_ReturnType (*XcpProgramClear_AbsoluteAccess)(
            Xcp_MtaType* mta,
            uint32 clearRange,
            Xcp_ErrorType *error);
    /**
     *
     * @param error
     * @return
     */
    Std_ReturnType (*XcpProgramReset)(
            Xcp_ErrorType *error);

    /**
     *
     * @param pgmProperties
     * @param maxSector
     * @param error
     * @return
     */
    Std_ReturnType (*XcpGetPgmProcessorInfo)(
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
    Std_ReturnType (*XcpGetSectorInfo_Mode0_1)(
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
    Std_ReturnType (*XcpGetSectorInfo_Mode2)(
            uint8 *sectorNameLength,
            Xcp_ErrorType *error);

    /**
     *
     * @param mta
     * @param codeSize
     * @param error
     * @return
     */
    Std_ReturnType (*XcpProgramPrepare)(
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
    Std_ReturnType (*XcpProgramFormat)(
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
    Std_ReturnType (*XcpProgramVerify)(
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
    Std_ReturnType (*XcpProgram)(
            Xcp_MtaType* mta,
            uint8* data,
            uint32 len,
            Xcp_ErrorType *error);

#endif /*XCP_FEATURE_PGM == STD_ON*/



    /**
     * This function is responsible to calculate the intended checksum algorithm, at the given example
     * XCP_CHECKSUM_ADD_11 is used
     * @param mtaPtr
     * @param block (input) length of the block where the CRC is calculated
     * @param type (output) type of the CRC calculation.
     * @param response (output) the result of the CRC calculation over the block
     */
    void (*XcpBuildChecksum)(
            Xcp_MtaType*      mtaPtr,
            uint32            block,
            Xcp_ChecksumType* type,
            uint32*           response);

    /**
     * This function together with Xcp_Arc_RecievePduAllowed is used to dynamically allow certain Pdus to receive and transmit messages.
     * With the help of these functions the user can enable, disable certain communication channels, and in this way different session handling of
     * Xcp can be performed.
     * @param XcpTxPduId  This Pdu Id is intended to use to send out Xcp message
     * @return TRUE  XcpTxPduId is allowed to use in this session
     * 		   FALSE XcpTxPduId is not allowed to use in this session
     */
    boolean (*XcpTransmitPduAllowed)(PduIdType XcpTxPduId);

    /**
     * This function together with Xcp_Arc_TransmitPduAllowed is used to dynamically allow certain Pdus to receive and transmit messages.
     * With the help of these functions the user can enable, disable certain communication channels, and in this way different session handling of
     * Xcp can be performed.
     * @param XcpRxPduId  This Pdu Id is intended to use to process incoming Xcp message
     * @param XcpRxPduPtr   Pointer to the incoming Xcp messages
     * @return TRUE  XcpRxPduId is allowed to use in this session
     * 		   FALSE XcpRxPduId is not allowed to use in this session
     */
    boolean (*XcpRecievePduAllowed)(PduIdType XcpRxPduId, PduInfoType* XcpRxPduPtr);

    /** Config and Runtime parameter*/
    const Xcp_EventChannelType *XcpEventChannel;

    /** Config parameter*/
    const Xcp_InfoType XcpInfo;

    /** Config parameter*/
    const Xcp_ProtectType XcpOriginalProtect; /**< Bitfield with currently locked features (Xcp_ProtectType) */

} Xcp_ConfigType;


extern const Xcp_ConfigType XcpConfig;

#endif /* XCP_CONFIGTYPES_H_ */
