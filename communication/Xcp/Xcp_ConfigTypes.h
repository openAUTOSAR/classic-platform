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

#ifndef XCP_CONFIGTYPES_H_
#define XCP_CONFIGTYPES_H_

#include "Std_Types.h"

typedef enum {
	XCP_TIMESTAMP_UNIT_1NS = 0x00,
	XCP_TIMESTAMP_UNIT_10NS = 0x01,
	XCP_TIMESTAMP_UNIT_100NS = 0x02,
	XCP_TIMESTAMP_UNIT_1US = 0x03,
	XCP_TIMESTAMP_UNIT_10US = 0x04,
	XCP_TIMESTAMP_UNIT_100US = 0x05,
	XCP_TIMESTAMP_UNIT_1MS = 0x06,
	XCP_TIMESTAMP_UNIT_10MS = 0x07,
	XCP_TIMESTAMP_UNIT_100MS = 0x08,
	XCP_TIMESTAMP_UNIT_1S = 0x09,
    XCP_TIMESTAMP_UNIT_1PS = 0x0A,
    XCP_TIMESTAMP_UNIT_10PS = 0x0B,
    XCP_TIMESTAMP_UNIT_100PS = 0x0C,
} Xcp_TimestampUnitType;

#define XCP_IDENTIFICATION_ABSOLUTE              0x0
#define XCP_IDENTIFICATION_RELATIVE_BYTE         0x1
#define XCP_IDENTIFICATION_RELATIVE_WORD         0x2
#define XCP_IDENTIFICATION_RELATIVE_WORD_ALIGNED 0x3

#define XCP_PROTOCOL_ETHERNET 0x0
#define XCP_PROTOCOL_CAN      0x1
#define XCP_PROTOCOL_USB      0x2
#define XCP_PROTOCOL_FLEXRAY  0x3

typedef struct {
	const uint16 XcpRxPduId;
	void* XcpRxPduRef;
} Xcp_RxPduType;

typedef struct {
	const uint16 XcpTxConfirmationPduId;
} Xcp_TxConfirmationType;

typedef struct {
	const uint16 XcpTxPduId;
	void* XcpTxPduRef;
	Xcp_TxConfirmationType* XcpTxConfirmation;
} Xcp_TxPduType;

typedef struct {
	const Xcp_RxPduType* XcpRxPdu;
	const Xcp_TxPduType* XcpTxPdu;
} Xcp_PduType;

typedef struct {
	uint8 XcpDtoPid; /* 0 .. 251 */
	const Xcp_PduType* XcpDto2PduMapping; /* XcpRxPdu, XcpTxPdu */
} Xcp_DtoType;

typedef struct Xcp_OdtEntryType {
	void* XcpOdtEntryAddress;
	uint8 XcpOdtEntryLength;
	uint8 XcpOdtEntryNumber; /* 0 .. 254 */

	/* Implementation defined */

	struct Xcp_OdtEntryType *XcpNextOdtEntry;
	uint8 BitOffSet;
	uint8 XcpOdtEntryExtension;
} Xcp_OdtEntryType;

struct Xcp_BufferType;

typedef struct Xcp_OdtType {
	uint8 XcpMaxOdtEntries; /* XCP_MAX_ODT_ENTRIES */
	uint8 XcpOdtEntriesCount; /* 0 .. 255 */
	uint8 XcpOdtEntryMaxSize; /* 0 .. 254 */
	uint8 XcpOdtNumber; /* 0 .. 251 */
	Xcp_DtoType XcpOdt2DtoMapping;
	Xcp_OdtEntryType* XcpOdtEntry; /* 0 .. * */

	/* Implementation defined */
	int XcpOdtEntriesValid; /* Number of non zero entries */
	struct Xcp_OdtType *XcpNextOdt;
	struct Xcp_BufferType *XcpStim;
} Xcp_OdtType;

typedef enum {
	XCP_DAQLIST_MODE_SELECTED = 1 << 0,
	XCP_DAQLIST_MODE_STIM = 1 << 1,
	XCP_DAQLIST_MODE_TIMESTAMP = 1 << 4,
	XCP_DAQLIST_MODE_PIDOFF = 1 << 5,
	XCP_DAQLIST_MODE_RUNNING = 1 << 6,
	XCP_DAQLIST_MODE_RESUME = 1 << 7,
} Xcp_DaqListModeEnum;

typedef enum {
	XCP_DAQLIST_PROPERTY_PREDEFINED = 1 << 0,
	XCP_DAQLIST_PROPERTY_EVENTFIXED = 1 << 1,
	XCP_DAQLIST_PROPERTY_DAQ = 1 << 2,
	XCP_DAQLIST_PROPERTY_STIM = 1 << 3
} Xcp_DaqListPropertyEnum;

struct Xcp_DaqListType;
typedef struct {
	Xcp_DaqListModeEnum Mode; /**< bitfield for the current mode of the DAQ list */
	uint16 EventChannel; /*TODO: Fixed channel vs current */
	uint8 Prescaler; /* */
	uint8 Priority; /* */
	Xcp_DaqListPropertyEnum Properties; /**< bitfield for the properties of the DAQ list */
	/**
	 * Pointer to next allocated DAQ list
	 *   [INTERNAL]
	 */
	const struct Xcp_DaqListType *XcpNextDaq;
	/**
	 * Index number of DAQ list
	 *   [INTERNAL]
	 *
	 * 0 .. 65534
	 */
	uint16 XcpDaqListNumber; /* 0 .. 65534 */
} Xcp_DaqListParams;

typedef struct Xcp_DaqListType {
	/**
	 * Maximum number of ODT's in XcpOdt array
	 *   [USER]    : When static DAQ configuration
	 *   [INTERNAL]: Dynamic DAQ configuration.
	 * 0 .. 252
	 */
	uint8 XcpMaxOdt;

	/**
	 * Number of currently configured ODT's
	 *   [USER]    : If you have predefined DAQ lists
	 *   [INTERNAL]: If daq lists are configured by master
	 */
	uint8 XcpOdtCount; /* 0 .. 252 */

	/**
	 * Pointer to an array of ODT structures this DAQ list will use
	 *   [USER]: With static DAQ lists, this needs to be set
	 *           to an array of XcpMaxOdt size.
	 *   [INTERNAL]: With dynamic DAQ configuration.
	 */
	Xcp_OdtType *XcpOdt; /**< reference to an array of Odt's configured for this Daq list */

	/**
	 * Holds parameters for the DAQ list
	 *   [INTERNAL/USER]
	 *   TODO: Move the parameters into the DAQ list structure instead
	 */
	Xcp_DaqListParams *XcpParams;
} Xcp_DaqListType;

typedef enum {
	XCP_EVENTCHANNEL_PROPERTY_DAQ = 1 << 2, XCP_EVENTCHANNEL_PROPERTY_STIM = 1
			<< 3, XCP_EVENTCHANNEL_PROPERTY_ALL = XCP_EVENTCHANNEL_PROPERTY_DAQ
			| XCP_EVENTCHANNEL_PROPERTY_STIM,
} Xcp_EventChannelPropertyEnum;

typedef struct {
	/**
	 * Event channel number.
	 *   [USER]
	 *
	 * Should match the order in the array of event channels
	 */
	const uint16 XcpEventChannelNumber; /**< 0 .. 65534 */

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
	 * Pointer to an array of pointers to daqlists
	 *   [USER]
	 */
	const Xcp_DaqListType** XcpEventChannelTriggeredDaqListRef;

	/**
	 * Set to the name of the eventchannel or NULL
	 *   [USER]
	 */
	const char* XcpEventChannelName;

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

	/**
	 * Counter used to implement prescaling
	 *   [INTERNAL]
	 */
	uint8 XcpEventChannelCounter;

	/**
	 * Number of daq lists currently assigned to event channel
	 *   [INTERNAL]
	 */
	uint8 XcpEventChannelDaqCount;

} Xcp_EventChannelType;

typedef enum {
	XCP_ACCESS_ECU_ACCESS_WITHOUT_XCP = 1 << 0,
	XCP_ACCESS_ECU_ACCESS_WITH_XCP = 1 << 1,
	XCP_ACCESS_XCP_READ_ACCESS_WITHOUT_ECU = 1 << 2,
	XCP_ACCESS_READ_ACCESS_WITH_ECU = 1 << 3,
	XCP_ACCESS_XCP_WRITE_ACCESS_WITHOUT_ECU = 1 << 4,
	XCP_ACCESS_XCP_WRITE_ACCESS_WITH_ECU = 1 << 5,
	XCP_ACCESS_ALL = 0x3f
} Xcp_AccessFlagsType;

typedef enum {
	XCP_PROTECT_NONE = 0,
	XCP_PROTECT_CALPAG = 1 << 0,
	XCP_PROTECT_DAQ = 1 << 2,
	XCP_PROTECT_STIM = 1 << 3,
	XCP_PROTECT_PGM = 1 << 4,
} Xcp_ProtectType;

typedef enum {
	XCP_COMPRESSION_METHOD_NONE = 0,
} Xcp_CompressType;

typedef enum {
	XCP_ENCRYPTION_METHOD_NONE = 0,
} Xcp_EncryptionType;

typedef struct {
	uint32 XcpSrc;
	uint32 XcpDst;
	uint32 XcpLen;
} Xcp_MemoryMappingType;

typedef struct {
	const Xcp_AccessFlagsType XcpAccessFlags;
	uint8 XcpMaxPage;
	uint8 XcpPageXcp;
	uint8 XcpPageEcu;

	uint32 XcpAddress;
	uint32 XcpLength;
	uint8 XcpExtension;

	Xcp_CompressType XcpCompression;
	Xcp_EncryptionType XcpEncryption;

	uint32 XcpMaxMapping;
	Xcp_MemoryMappingType* XcpMapping;
} Xcp_SegmentType;

typedef struct {
	const char* XcpCaption; /**< ASCII text describing device [USER] */
	const char* XcpMC2File; /**< ASAM-MC2 filename without path and extension [USER] */
	const char* XcpMC2Path; /**< ASAM-MC2 filename with path and extension [USER] */
	const char* XcpMC2Url; /**< ASAM-MC2 url to file [USER] */
	const char* XcpMC2Upload; /**< ASAM-MC2 file to upload [USER] */
} Xcp_InfoType;

/** @req XCP845 *//*Xcp_ConfigType definition - can not be tested with conventional module tests*/
typedef struct {
	const Xcp_DaqListType *XcpDaqList;
	Xcp_EventChannelType *XcpEventChannel;
	const Xcp_PduType *XcpPdu;

	/* Implementation defined */

	Xcp_SegmentType *XcpSegment;
	const uint16 XcpMaxSegment;

	const Xcp_InfoType XcpInfo;

	const uint16 XcpMaxEventChannel; /* 0 .. 65535, XCP_MAX_EVENT_CHANNEL */
	const uint16 XcpMinDaq; /* 0 .. 255  , XCP_MIN_DAQ */

	Xcp_ProtectType XcpProtect; /**< Bitfield with currently locked features (Xcp_ProtectType) */

	/**
	 * Function used for Seed & Key unlock
	 * @param res is the resource requested to be unlocked
	 * @param seed is the seed that was sent to the master
	 * @param seed_len is the length of @param seed
	 * @param key is the key sent from master
	 * @param key_len is the length of @param key
	 * @return E_OK for success, E_ERR for failure
	 */
	Std_ReturnType (*XcpUnlockFn)(Xcp_ProtectType res, const uint8* seed,
			uint8 seed_len, const uint8* key, uint8 key_len);
	/**
	 * Function called to retrieve seed that should be
	 * sent to master in a GetSeed/Unlock exchange
	 * @param res is the resource requested
	 * @param seed pointer to buffer that will hold seed (255 bytes max)
	 * @return number of bytes in seed
	 */
	uint8 (*XcpSeedFn)(Xcp_ProtectType res, uint8* seed);

	/**
	 * Function called for a XCP user defined call from the master
	 * @param data data recieved from master (excluding the preceding 0xF1 for user defined)
	 * @param len  length of the data in buffer
	 * @return
	 */
	Std_ReturnType (*XcpUserFn)(void* data, int len);

#if(XCP_DAQ_CONFIG_TYPE == DAQ_DYNAMIC)

	Xcp_DaqListParams* ptrDynamicDaqParams;
	/** Daq      references for dynamic Daq allocation */
	Xcp_DaqListType*  ptrDynamicDaq;

	/** Odt      references for dynamic Daq allocation */
	Xcp_OdtType*      ptrDynamicOdt;

	/** OdtEntry references for dynamic Daq allocation */
	Xcp_OdtEntryType* ptrDynamicOdtEntry;

#endif /*XCP_DAQ_CONFIG_TYPE == DAQ_DYNAMIC*/

} Xcp_ConfigType;

#endif /* XCP_CONFIGTYPES_H_ */
