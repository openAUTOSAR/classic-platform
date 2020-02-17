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


#ifndef CANNM_CONFIGTYPES_H_
#define CANNM_CONFIGTYPES_H_

#define CANNM_UNUSED_CHANNEL 0xFFu

typedef enum {
    CANNM_PDU_BYTE_0 = 0x00,
    CANNM_PDU_BYTE_1 = 0x01,
    CANNM_PDU_OFF = 0xFF
} CanNm_PduBytePositionType;

/** @req CANNM202 */
typedef struct {
    const boolean					Active;
    const NetworkHandleType			NmNetworkHandle;
    const uint8						NodeId;
    const uint32					MainFunctionPeriod;
    const uint32					TimeoutTime;			/** @req CANNM246 */
    const uint32					RepeatMessageTime;      /** @req CANNM247 */
    const uint32					WaitBusSleepTime;       /** @req CANNM248 */
    const uint32					MessageCycleTime;
    const uint32					MessageCycleOffsetTime;
    const uint32                    MessageTimeoutTime;
    const PduIdType					CanIfPduId;
    const PduLengthType				PduLength;
    const CanNm_PduBytePositionType	NidPosition;            /**< @req CANNM074 */
    const CanNm_PduBytePositionType	CbvPosition;            /**< @req CANNM075 */
    const uint32					ImmediateNmCycleTime;	/**< @req CANNM335 */
    const uint32					ImmediateNmTransmissions;
    const boolean                   CanNmPnEnable;
    const boolean					CanNmPnEraCalcEnabled;
    const boolean                   CanNmAllNmMsgKeepAwake;
#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
    const PduIdType					CanNmUserDataTxPduId;
#endif
#if ((CANNM_PNC_COUNT > 0) && (CANNM_PNC_ERA_CALC_ENABLED == STD_ON))
    PduIdType	CanNmERARxNSduId; /* SDU carrying ERA */
#endif /* ((CANNM_PNC_COUNT > 0) && (CANNM_PNC_ERA_CALC_ENABLED == STD_ON)) */
} CanNm_ChannelType;

typedef struct {
    uint8 CanNmPnInfoOffset; /* Offset Index */
    PduLengthType CanNmPnInfoLen; /* PnInfo Length */
    const uint8 *CanNmPnInfoMask; /* Pointer PNC mask bytes */
    const uint8 *CanNmPnIndexToTimerMap; /* Mapping from PN index to reset timer index */
    const uint8 *CanNmTimerIndexToPnMap; /* Mapping from timer index to PN index */
    PduIdType	CanNmEIRARxNSduId; /* SDU carrying EIRA */
}CanNm_PnInfoType;

typedef struct {
    const CanNm_ChannelType*    Channels;       /* Pointer to the CanNm channels */
    const NetworkHandleType*    ChannelLookups; /* Pointer to lookup from NetworkHandle to index in CanNm channels */
    const NetworkHandleType     ChannelLookupsSize;/* Size of ChannelLookups */
    const CanNm_PnInfoType * 	CanNmPnInfo; /* Pointer to Pn Info */
} CanNm_ConfigType;											/** @req CANNM202 */

#endif /* CANNM_CONFIGTYPES_H_ */
