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

#ifndef SOAD_CONFIGTYPES_H
#define SOAD_CONFIGTYPES_H
#include "Std_Types.h"
#include "ComStack_Types.h"
#include "SoAd_Types.h"

/*
 * Callback function prototypes
 */

typedef void (*SoAd_CallbackTpCopyRxDataFncType)(PduIdType rxPduId, PduInfoType* pduInfoPtr, PduLengthType* rxBufferSizePtr);	/** @req SOAD139 */
typedef void (*SoAd_CallbackTpStartofReceptionFncType)(PduIdType rxPduId, PduLengthType sduLength, PduLengthType* rxBufferSizePtr);	/** @req SOAD138 */
typedef void (*SoAd_CallbackTpGetAvailableTxBufferFncType)(PduIdType txPduId, PduLengthType, PduLengthType* txBufferSizePtr);	/** @req SOAD175 */
typedef void (*SoAd_CallbackTpCopyTxDataFncType)(PduIdType txPduId, PduInfoType** txPduPtr, RetryInfoType* retryInfoPtr, PduLengthType txDataCntPtr);	/** @req SOAD137 */

typedef void (*SoAd_CallbackPdurIfRxIndicationFncType)(PduIdType rxPduId, PduInfoType* pduInfoPtr);	/** @req SOAD106 */
typedef void (*SoAd_CallbackPdurTpRxIndicationFncType)(PduIdType rxPduId, NotifResultType result);	/** @req SOAD180 */
typedef void (*SoAd_CallbackUdpnmIfRxIndicationFncType)(PduIdType rxPduId, const uint8* udpSduPtr);

typedef void (*SoAd_CallbackPdurIfTxConfirmationFncType)(PduIdType txPduId);	/** @req SOAD107 */
typedef void (*SoAd_CallbackPdurTpTxConfirmationFncType)(PduIdType txPduId, NotifResultType result);	/** @req SOAD181 */
typedef void (*SoAd_CallbackUdpnmIfTxConfirmationFncType)(PduIdType txPduId);

typedef Std_ReturnType (*SoAd_CallbackDoIpPowerModeFncType)(SoAd_DoIp_PowerMode* powermode);

typedef union {
	SoAd_CallbackPdurIfRxIndicationFncType	PdurIfRxIndicationFnc;
	SoAd_CallbackPdurTpRxIndicationFncType	PdurTpRxIndicationFnc;
	SoAd_CallbackUdpnmIfRxIndicationFncType	UdpnmIfRxIndicationFnc;
} SoAd_CallbackRxIndicationFncType;

typedef union {
	SoAd_CallbackPdurIfTxConfirmationFncType	PdurIfTxConfirmationFnc;
	SoAd_CallbackPdurTpTxConfirmationFncType	PdurTpTxConfirmationFnc;
	SoAd_CallbackUdpnmIfTxConfirmationFncType	UdpnmIfTxConfirmationFnc;
} SoAd_CallbackTxConfirmationFncType;

// TODO: Where shall these enums be defined?
typedef enum {
	SOAD_SOCKET_PROT_TCP,
	SOAD_SOCKET_PROT_UDP
} SoAd_SocketProtocolType;

typedef enum {
	SOAD_AUTOSAR_CONNECTOR_CDD,
	SOAD_AUTOSAR_CONNECTOR_DOIP,
	SOAD_AUTOSAR_CONNECTOR_PDUR,
	SOAD_AUTOSAR_CONNECTOR_XCP
} SoAd_AutosarConnectorType;

typedef enum {
	SOAD_UL_CDD,
	SOAD_UL_PDUR,
	SOAD_UL_UDPNM,
	SOAD_UL_XCP
} SoAd_ULType;

// 10.2.5 SoAdSocketConnection
// Information required to receive and transmit data via the TCP/IP stack on a particular connection.
typedef struct {
	uint16						SocketId;					/** @req SOAD016_Conf */ // Static
	char*						SocketLocalIpAddress;		/** @req SOAD017_Conf */
	uint16						SocketLocalPort;			/** @req SOAD018_Conf */ // Static
	char*						SocketRemoteIpAddress;		/** @req SOAD019_Conf */
	uint16						SocketRemotePort;			/** @req SOAD020_Conf */
	SoAd_SocketProtocolType		SocketProtocol;				/** @req SOAD021_Conf */ // Static
	boolean						SocketTcpInitiate;			/** @req SOAD022_Conf */ // Static
	boolean						SocketTcpNoDelay;			/** @req SOAD023_Conf */ // Static
	boolean						SocketUdpListenOnly;		/** @req SOAD024_Conf */ // Static
	SoAd_AutosarConnectorType	AutosarConnectorType;		/** @req SOAD025_Conf */ // Static
	boolean						PduHeaderEnable;			/** @req SOAD027_Conf */ // Static
	boolean						SocketAutosarApi;			/** @req SOAD026_Conf */ // Static
	boolean 					ResourceManagementEnable;	/** @req SOAD067_Conf */ // Static
	boolean						PduProvideBufferEnable; 	/** @req SOAD029_Conf */ // Static
} SoAd_SocketConnectionType;	/** @req SOAD009_Conf */

// 10.2.6 SoAdSocketRoute (RX direction)
// Describes the path of a PDU from a socket in the TCP/IP stack to the PDU Router after reception in the TCP/IP Stack
typedef struct {
	const SoAd_SocketConnectionType*	SourceSocketRef;		/** @req SOAD035_Conf */
	uint32								SourceId;				/** @req SOAD036_Conf */	 /* Only used when having PDU header enabled */
//	???									DestinationPduRef;		/** @req SOAD038_Conf */	// TODO: Replaced by following line, ok?
	uint16								DestinationPduId;									// TODO: Ok?
	uint64								DestinationSduLength;	/** @req SOAD037_Conf */
	SoAd_ULType							UserRxIndicationUL;		/** @req SOAD068_Conf */
//	SoAd_CallbackRxIndicationFncType	RxIndicationUL;			/** @req SOAD069_Conf */	// Only used when UserRxIndicationUL = CDD
} SoAd_SocketRouteType;	/** @req SOAD008_Conf */

// 10.2.7 SoAdPduRoute (TX direction)
// Describes the path of a PDU from the PDU Router to the socket in the TCP/IP stack for transmission.
typedef struct {
	uint16									SourcePduId;			/** @req SOAD031_Conf */
	uint16									SourceSduLength;		/** @req SOAD032_Conf */
//	???										SourcePduRef;			/** @req SOAD030_Conf */	// TODO: Is not SOAD031 enough?
	const SoAd_SocketConnectionType*		DestinationSocketRef;	/** @req SOAD034_Conf */
	uint32									DestinationId;			/** @req SOAD033_Conf */	/* Only used when having PDU header enabled */
	SoAd_ULType								UserTxConfirmationUL;	/** @req SOAD070_Conf */
//	SoAd_CallbackTxConfirmationFncType		TxConfirmationUL;		/** @req SOAD071_Conf */	// Only used when UserTxConfirmationUL = CDD
} SoAd_PduRouteType;	/** @req SOAD007_Conf */

// 10.2.8 SoAdDoIpConfig
// This container contains all global configuration parameters of the DoIP plug-in.
typedef struct {
	uint32		DoIpAliveCheckresponseTimeMs;	/** @req SOAD051_Conf */
	uint32		DoIpControlTimeoutMs;			/** @req SOAD065_Conf */
	uint32		DoIpGenericInactiveTimeMs;		/** @req SOAD052_Conf */
	uint8*		DoIpHostNameOpt;				/** @req SOAD053_Conf */
	uint32		DoIpInitialInactiveTimeMs;		/** @req SOAD054_Conf */
	uint32		DoIpResponseTimeoutMs;			/** @req SOAD066_Conf */
	uint32		DoIpVidAnnounceIntervalMs;		/** @req SOAD055_Conf */
	uint32		DoIpVidAnnounceMaxWaitMs;		/** @req SOAD056_Conf */
	uint32		DoIpVidAnnounceMinWaitMs;		/** @req SOAD057_Conf */
	uint32		DoIpVidAnnounceNum;				/** @req SOAD058_Conf */
	SoAd_CallbackDoIpPowerModeFncType DoipPowerModeCallback;
} SoAd_DoIpConfigType;	/** @req SOAD050_Conf */

// 10.2.9 SoAdDoIpRoute
// A SoAd_DoIP_Route allocates a PDU ID to a combination of a DoIP source and a DoIP target address.
typedef struct {
	uint64								DoIpSourceAddress;			/** @req SOAD041_Conf */
	uint64								DoIpTargetAddress;			/** @req SOAD042_Conf */
	const SoAd_SocketConnectionType*	DoIpSocketConnectionRef;	/** @req SOAD043_Conf */
	uint16								PduRouteIndex;
	uint16								SocketRouteIndex;
} SoAd_DoIpRouteType;	/** @req SOAD040_Conf */


typedef void (*DoIp_AuthenticationCallbackType)(void);
typedef void (*DoIp_ConfirmationCallbackType)(void);

typedef struct {
	uint16 addressValue;
	uint16 txPdu;
	uint16 rxPdu;
} DoIp_TargetAddressConfigType;

typedef struct {
	uint8 activationNumber;
	DoIp_AuthenticationCallbackType authenticationCallback;
	DoIp_ConfirmationCallbackType confirmationCallback;
} DoIp_RoutingActivationConfigType;

typedef struct {
	uint16 routingActivation;
	uint16 target;
} DoIp_RoutingActivationToTargetAddressMappingType;

typedef struct {
	uint16 address;
	uint16 numBytes;
} DoIp_TesterConfigType;

typedef struct {
	uint16 tester;
	uint16 routingActivation;
} DoIp_TesterToRoutingActivationMapType;

typedef struct {
	const SoAd_SocketConnectionType*	SocketConnection;
	const SoAd_SocketRouteType*			SocketRoute;
	const SoAd_PduRouteType*			PduRoute;
	const SoAd_DoIpConfigType*			DoIpConfig;

	const SoAd_DoIp_ArcNodeTypeType 	DoIpNodeType;
	const uint16 						DoIpNodeLogicalAddress;

	const DoIp_TargetAddressConfigType* 					DoIpTargetAddresses;
	const DoIp_RoutingActivationConfigType*					DoIpRoutingActivations;
	const DoIp_RoutingActivationToTargetAddressMappingType* DoIpRoutingActivationToTargetAddressMap;
	const DoIp_TesterConfigType*							DoIpTesters;
	const DoIp_TesterToRoutingActivationMapType*			DoIpTesterToRoutingActivationMap;
} SoAd_ConfigType;



/*
 * Make the SoAd_Config visible for others.
 */
extern const SoAd_ConfigType SoAd_Config;


#endif
