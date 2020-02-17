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


#ifndef UDPNM_CONFIGTYPES_H_
#define UDPNM_CONFIGTYPES_H_

#if defined(USE_DEM)
#include "Dem.h"
#endif


/** @req SWS_UdpNm_00026 */
/** @req SWS_UdpNm_00201 */
/** @req SWS_UdpNm_00202 */
/** @req SWS_UdpNm_00203 */


/** @req SWS_UdpNm_00304 */
typedef enum {
    UDPNM_PDU_BYTE_0 = 0x00,
    UDPNM_PDU_BYTE_1 = 0x01,
    UDPNM_PDU_OFF = 0xFF
} UdpNm_PduPositionType;

/** @req SWS_UdpNm_00202 */
/** @req SWS_UdpNm_00203 */
typedef struct {
    const uint8					    UdpNmNodeId;
    const UdpNm_PduPositionType	    UdpNmPduCbvPosition;          /**< @req SWS_UdpNm_00075 */
    const uint8						UdpNmPduLength;               /**< @req SWS_UdpNm_00076 */
    const UdpNm_PduPositionType     UdpNmPduNidPosition;          /**< @req SWS_UdpNm_00074 */
    const uint8                     UdpNmUserDataLength;          /* Not used */
    const uint32					UdpNmMainFunctionPeriod;
    const uint32					UdpNmMsgCycleOffset;
    const uint32					UdpNmMsgCycleTime;
    const uint32					UdpNmMsgTimeoutTime;
    const uint32					UdpNmRemoteSleepIndTime;      /**< @req SWS_UdpNm_00249 */
    const uint32					UdpNmRepeatMessageTime;       /**< @req SWS_UdpNm_00247 */
    const uint32					UdpNmTimeoutTime;             /**< @req SWS_UdpNm_00246 */
    const uint32					UdpNmWaitBusSleepTime;        /**< @req SWS_UdpNm_00248 */
    const NetworkHandleType         UdpNmComMChannelIdRef;
    const PduIdType                 UdpNmRxPduId;                 /* Not used */
    const PduIdType                 UdpNmTxPduId;
    const boolean                   UdpNmPnEnable;
    const boolean					UdpNmPnEraCalcEnabled;
#if (UDPNM_COM_USER_DATA_SUPPORT == STD_ON)
    const PduIdType					UdpNmUserDataTxPduId;
#endif
#if ((UDPNM_PNC_COUNT > 0) && (UDPNM_PNC_ERA_CALC_ENABLED == STD_ON))
    PduIdType						UdpNmERARxNSduId; /* SDU carrying ERA */
#endif /* ((UDPNM_PNC_COUNT > 0) && (UDPNM_PNC_ERA_CALC_ENABLED == STD_ON)) */
} UdpNm_ChannelType;


#if defined(USE_DEM)
typedef struct {
    Dem_EventIdType                 UdpNmTcpIpTransmitErrorDemEventId;
    Dem_EventIdType                 UdpNmNetworkTimeoutDemEventId;
} UdpNm_DemEventReferencesType;
#endif

typedef struct {
    uint8							UdpNmPnInfoOffset; /* Offset Index */
    uint8 							UdpNmPnInfoLen; /* PnInfo Length */
    const uint8 					*UdpNmPnInfoMask; /* Pointer PNC mask bytes */
    const uint8 					*UdpNmPnIndexToTimerMap; /* Mapping from PN index to reset timer index */
    const uint8 					*UdpNmTimerIndexToPnMap; /* Mapping from timer index to PN index */
#if UDPNM_PNC_EIRA_CALC_ENABLED == STD_ON
    PduIdType						UdpNmEIRARxNSduId; /* SDU carrying EIRA */
#endif
} UdpNm_PnInfoType;

/** @req SWS_UdpNm_00308 */
typedef struct {
    const UdpNm_ChannelType*		Channels;
    const NetworkHandleType*        ChannelMap;
    const NetworkHandleType         ChannelMapSize;
    const UdpNm_PnInfoType* 		UdpNmPnInfo;
#if defined(USE_DEM)
    const UdpNm_DemEventReferencesType*     DemReferences;
#endif
} UdpNm_ConfigType;

#endif /* UDPNM_CONFIGTYPES_H_ */
