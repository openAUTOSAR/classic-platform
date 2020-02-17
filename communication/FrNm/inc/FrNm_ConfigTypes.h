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

#ifndef FRNM_CONFIGTYPES_H_
#define FRNM_CONFIGTYPES_H_

#define FRNM_UNUSED_CHANNEL 0xFFu

typedef enum {
    FRNM_PDU_SCHEDULE_VARIANT_1 = 0x00u,
    FRNM_PDU_SCHEDULE_VARIANT_2 = 0x01u,
    FRNM_PDU_SCHEDULE_VARIANT_3 = 0x02u,
    FRNM_PDU_SCHEDULE_VARIANT_4 = 0x03u,
    FRNM_PDU_SCHEDULE_VARIANT_5 = 0x04u,
    FRNM_PDU_SCHEDULE_VARIANT_6 = 0x05u,
    FRNM_PDU_SCHEDULE_VARIANT_7 = 0x06u
} FrNm_PduScheduleVariantType;

/* @req FRNM195 */ /* @req FRNM196 */
typedef enum {
    FRNM_CYCLE_VALUE_1 = 1u,
    FRNM_CYCLE_VALUE_2 = 2u,
    FRNM_CYCLE_VALUE_4 = 4u,
    FRNM_CYCLE_VALUE_8 = 8u,
    FRNM_CYCLE_VALUE_16 = 16u,
    FRNM_CYCLE_VALUE_32 = 32u,
    FRNM_CYCLE_VALUE_64 = 64u
} FrNm_FrNmCycleType;

typedef struct {
    const boolean FrNmRxContainsData;
    const boolean FrNmRxConatainsVote;
    const PduIdType FrNmRxPduId;
    const uint8 FrNmRxPduLength;
}FrNm_RxPduConfigType;

typedef struct {
    const boolean FrNmTxContainsData;
    const boolean FrNmTxConatainsVote;
    const PduIdType FrNmTxConfPduId;
    const uint8 FrNmTxPduLength;
    const PduIdType FrIfTxPduId;
}FrNm_TxPduConfigType;

typedef struct {
    const PduIdType FrNmUserDataTxPduId;
    const uint8 FrNmUserDataTxPduLength;
}FrNm_UserDataTxPduConfigType;


typedef struct {
    const FrNm_RxPduConfigType *FrNmRxPduList;
    const uint8 FrNmRxPduCount;
    const uint8 FrNmRxPduLength;
    const FrNm_TxPduConfigType *FrNmTxPduList;
    const uint8 FrNmTxPduCount;
    const uint8 FrNmTxPduLength;
    const FrNm_UserDataTxPduConfigType *FrNmUserDataConfig;
    const NetworkHandleType FrNmChannelHandle;
    const NetworkHandleType FrNmComMNetworkHandleRef;
    const boolean FrNmControlBitVectorActive;
    const uint8 FrNmNodeId;
    const FrNm_PduScheduleVariantType FrNmPduScheduleVariant;
    const boolean FrNmRepeatMessageBitActive;
    const boolean FrNmSyncPointEnabled;
    const boolean FrNmPnEnabled;
}FrNm_ChannelIdentifiersInfoType;

typedef struct {
    const FrNm_FrNmCycleType FrNmDataCycle; /* @req FRNM195 */
    const uint32 FrNmMainFunctionPeriod;
    const uint32 FrNmMsgTimeoutTime;
    const uint16 FrNmReadySleepCnt; /* @req FRNM309 */
    const uint32 FrNmRepeatMessageTime;
    const FrNm_FrNmCycleType FrNmRepetitionCycle; /* @req FRNM195 */
    const uint32 FrNmSyncLossTimer;
    const boolean FrNmVoteInhibitionEnabled;
    const FrNm_FrNmCycleType FrNmVotingCycle; /* @req FRNM195 */
}FrNm_ChannelTiminginfoType;


typedef struct {
    const FrNm_ChannelIdentifiersInfoType *FrNmChannelIdentifiersConfig;
    const FrNm_ChannelTiminginfoType *FrNmTimingConfig;
}FrNm_ChannelInfoType;

typedef struct {
    uint8 FrNmPnInfoOffset; /* Offset Index */
    PduLengthType FrNmPnInfoLen; /* PnInfo Length */
    const uint8 *FrNmPnInfoMask; /* Pointer PNC mask bytes */
    const uint8 *FrNmPnIndexToTimerMap; /* Mapping from PN index to reset timer index */
    const uint8 *FrNmTimerIndexToPnMap; /* Mapping from timer index to PN index */
    PduIdType   FrNmEIRARxNSduId; /* SDU carrying EIRA */
}FrNm_PnInfoType;

typedef struct {
    const FrNm_ChannelInfoType* FrNmChannels;
    const uint8* FrNmChannelLookups; /* Pointer to lookup from NetworkHandle to index in FrNm channels */
    const uint8  FrNmChnlCount; /* No. of FrNm Channels */
    const uint8* FrNmChnlRxpduMaps;
    const uint8* FrNmChnlTxpduMaps;
    const uint8* FrNmChnlFrIfCtrlIdMap;
    const FrNm_PnInfoType *FrNmPnInfo;
}FrNm_ConfigType;


#endif /* FRNM_CONFIGTYPES_H_ */
