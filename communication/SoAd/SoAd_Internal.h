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


/*
 * NB! This file is for SOAD internal use only and may only be included from SOAD C-files!
 */



#ifndef SOAD_INTERNAL_H_
#define SOAD_INTERNAL_H_
#include "SoAd.h"
#include "TcpIp.h"


#if  ( SOAD_DEV_ERROR_DETECT == STD_ON )
#include "Det.h"
#define VALIDATE_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_SOAD, 0, _api, _err); \
          return _rv; \
        }

#define VALIDATE_NO_RV(_exp,_api,_err ) \
  if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_SOAD, 0, _api, _err); \
          return; \
        }
#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(_x, _y, _z, _q)

#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_RV(_exp,_api,_err,_rv )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif

typedef enum {
	SOCKET_UNINIT = 0,
	SOCKET_DUPLICATE,
	SOCKET_INIT,
	SOCKET_TCP_LISTENING,
	SOCKET_TCP_READY,
	SOCKET_UDP_READY

} SocketStateType;

typedef struct {
	uint8								SocketNr;
	SocketStateType						SocketState;
	boolean								SocketProtocolIsTcp;
	const SoAd_SocketConnectionType*	SocketConnectionRef;
	const SoAd_SocketRouteType*			SocketRouteRef;
//	const SoAd_PduRouteType*			PduRouteRef;
	uint32								RemoteIpAddress;
	uint16								RemotePort;
	int									SocketHandle;
	int									ConnectionHandle;
} SocketAdminType;

typedef enum {
	PDU_IDLE,
	PDU_IF_SENDING,
	PDU_TP_REQ_BUFFER,
	PDU_TP_SENDING
} PduStatusType;

typedef enum {
	SOAD_ARC_DOIP_IDENTIFICATIONREQUEST_ALL,
	SOAD_ARC_DOIP_IDENTIFICATIONREQUEST_BY_EID,
	SOAD_ARC_DOIP_IDENTIFICATIONREQUEST_BY_VIN,
} SoAd_Arc_DoIp_VehicleIdentificationRequestType;

typedef struct {
	PduStatusType		PduStatus;
} PduAdminListType;

typedef enum {
	DOIP_ARC_CONNECTION_INVALID,
	DOIP_ARC_CONNECTION_REGISTERED,
} SoAd_ArcDoIpSocketStateType;

typedef struct {
	uint16 sockNr;
	uint16 sa;
	uint8 activationType;
	SoAd_ArcDoIpSocketStateType socketState;
	uint32 initialInactivityTimer;
	uint32 generalInactivityTimer;
	uint32 aliveCheckTimer;
	boolean authenticated;
	boolean confirmed;
	boolean awaitingAliveCheckResponse;

	/*
	 * Index of the last/active routing target in the generated DoIp_Cfg_TargetList
	 */
	uint16 targetIndex;

	/*
	 * Status of last/active pdu routing request.
	 */
	PduStatusType pduStatus;


} DoIp_ArcDoIpSocketStatusType;

typedef enum {
	DOIP_SOCKET_ASSIGNMENT_FAILED,
	DOIP_SOCKET_ASSIGNMENT_SUCCESSFUL,
	DOIP_SOCKET_ASSIGNMENT_PENDING,
} DoIp_Arc_SocketAssignmentResultType;

typedef enum {
	/* We do not currently have a valid IP */
	SOAD_ARC_LINKDOWN = 0,

	/* We have a valid IP to send from */
	SOAD_ARC_LINKUP,
} SoadArcLinkStatusType;


extern SocketAdminType SocketAdminList[SOAD_SOCKET_COUNT];
extern PduAdminListType PduAdminList[SOAD_PDU_ROUTE_COUNT];

boolean SoAd_BufferGet(uint32 size, uint8** buffPtr);
void SoAd_BufferFree(uint8* buffPtr);
void SoAd_SocketClose(uint16 sockNr);
void SoAd_SocketStatusCheck(uint16 sockNr, int sockHandle);
uint16 SoAd_SendIpMessage(uint16 sockNr, uint32 msgLen, uint8* buff);

SoadArcLinkStatusType SoAd_GetLinkStatus(void);

void DoIp_HandleTcpRx(uint16 sockNr);
void DoIp_HandleUdpRx(uint16 sockNr);
Std_ReturnType DoIp_HandleTpTransmit(PduIdType SoAdSrcPduId, const PduInfoType* SoAdSrcPduInfoPtr);

void DoIp_SendVehicleAnnouncement(uint16 sockNr);

void DoIp_LocalIpAddrAssignmentChg(SoAd_SoConIdType linkId, TcpIp_IpAddrStateType state);

uint8 SoAd_GetNofCurrentlyUsedTcpSockets();


#endif /* SOAD_INTERNAL_H_ */
