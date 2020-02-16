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

/** @req UDPNM026 */
/** @req UDPNM201 */
/** @req UDPNM202 */
/** @req UDPNM203 */


/** @req UDPNM304 */
typedef enum {
	UDPNM_PDU_BYTE_0 = 0x00,
	UDPNM_PDU_BYTE_1 = 0x01,
	UDPNM_PDU_OFF = 0xFF
} UdpNm_PduPositionType;

/** @req UDPNM202 */
/** @req UDPNM203 */
typedef struct {
	const uint8					    UdpNmNodeId;
	const UdpNm_PduPositionType	    UdpNmPduCbvPosition;          /**< @req UDPNM075 */
	const uint8						UdpNmPduLength;               /**< @req UDPNM076 */
	const UdpNm_PduPositionType     UdpNmPduNidPosition;          /**< @req UDPNM074 */
	const uint8                     UdpNmUserDataLength;          /* Not used */
	const uint32					UdpNmMainFunctionPeriod;
	const uint32					UdpNmMsgCycleOffset;
	const uint32					UdpNmMsgCycleTime;
	const uint32					UdpNmMsgTimeoutTime;
	const uint32					UdpNmRemoteSleepIndTime;      /**< @req UDPNM249 */
	const uint32					UdpNmRepeatMessageTime;       /**< @req UDPNM247 */
	const uint32					UdpNmTimeoutTime;             /**< @req UDPNM246 */
	const uint32					UdpNmWaitBusSleepTime;        /**< @req UDPNM248 */
	const NetworkHandleType         UdpNmChannelIdRef;
	const uint32                    UdpNmRxPduId;                 /* Not used */
	const uint32                    UdpNmTxPduId;
} UdpNm_ChannelType;

/** @req UDPNM308 */
typedef struct {
	const UdpNm_ChannelType* 			Channels;
} UdpNm_ConfigType;

#endif /* UDPNM_CONFIGTYPES_H_ */
