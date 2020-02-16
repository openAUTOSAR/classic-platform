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

#include <string.h>
#include "Os.h"
#include "Mcu.h"
#include "SoAd.h"
#include "SoAd_Internal.h"
#include "PduR_SoAd.h"
#include "TcpIp.h"
#include "Bsd.h"
//#include "SchM_SoAd.h"
#include "MemMap.h"

#include "Det.h"
#if defined(USE_DEM)
#include "Dem.h"
#endif
#if defined(USE_UDPNM)
#include "UdpNm.h"
#endif
#include "bcm2835.h"
#include "Uart.h"
typedef enum {
  SOAD_UNINITIALIZED = 0,
  SOAD_INITIALIZED
} SoadStateType;



static SoadStateType ModuleStatus = SOAD_UNINITIALIZED;
static SoadArcLinkStatusType LinkStatus = SOAD_ARC_LINKDOWN;

SocketAdminType SocketAdminList[SOAD_SOCKET_COUNT];

PduAdminListType PduAdminList[SOAD_PDU_ROUTE_COUNT];

#if 1
#include "lwip/mem.h"
boolean SoAd_BufferGet(uint32 size, uint8** buffPtr)
{
	boolean res;

	*buffPtr = mem_malloc(size);
	res = (*buffPtr != NULL) ? TRUE : FALSE;

	return res;
}
void SoAd_BufferFree(uint8* buffPtr)
{
	mem_free(buffPtr);
}

#else
typedef struct  {
	boolean	bufferInUse;
	uint16	bufferLen;
	uint8*	bufferPtr;
} AdminBufferType;

static uint8 buffer1[42];
static uint8 buffer2[SOAD_RX_BUFFER_SIZE];
static uint8 buffer3[SOAD_RX_BUFFER_SIZE];
static uint8 buffer4[SOAD_RX_BUFFER_SIZE];
#define BUFFER_COUNT	4
static AdminBufferType	adminBuffer[BUFFER_COUNT] = {
		{
				.bufferInUse = FALSE,
				.bufferLen = sizeof(buffer1),
				.bufferPtr = buffer1,
		},
		{
				.bufferInUse = FALSE,
				.bufferLen = sizeof(buffer2),
				.bufferPtr = buffer2,
		},
		{
				.bufferInUse = FALSE,
				.bufferLen = sizeof(buffer3),
				.bufferPtr = buffer3,
		},
		{
				.bufferInUse = FALSE,
				.bufferLen = sizeof(buffer4),
				.bufferPtr = buffer4,
		},
};


boolean SoAd_BufferGet(uint32 size, uint8** buffPtr)
{
	boolean returnCode = FALSE;
	uint16 i;
	imask_t state;
	Irq_Save(state);

	*buffPtr = NULL;
	for (i = 0; i < BUFFER_COUNT; i++) {
		if ((!adminBuffer[i].bufferInUse) && (adminBuffer[i].bufferLen >= size)) {
			adminBuffer[i].bufferInUse = TRUE;
			*buffPtr = adminBuffer[i].bufferPtr;
			returnCode = TRUE;
			break;
		}
	}

	Irq_Restore(state);
	return returnCode;
}

void SoAd_BufferFree(uint8* buffPtr)
{
	uint16 i;
	imask_t state;
	Irq_Save(state);

	for (i = 0; i < BUFFER_COUNT; i++) {
		if (adminBuffer[i].bufferPtr == buffPtr) {
			adminBuffer[i].bufferInUse = FALSE;
			uint16 j;

			for (j = 0; j < adminBuffer[i].bufferLen; j++) {
				adminBuffer[i].bufferPtr[j] = 0xaa;
			}
			break;
		}
	}

    Irq_Restore(state);
}
#endif

void SoAd_SocketClose(uint16 sockNr)
{
	uint16 i;

	switch (SocketAdminList[sockNr].SocketState) {
	case SOCKET_UDP_READY:
	case SOCKET_TCP_LISTENING:
		lwip_close(SocketAdminList[sockNr].SocketHandle);
		SocketAdminList[sockNr].SocketHandle = -1;
		SocketAdminList[sockNr].SocketState = SOCKET_INIT;
		break;


	case SOCKET_TCP_READY:
		lwip_close(SocketAdminList[sockNr].ConnectionHandle);
    	SocketAdminList[sockNr].ConnectionHandle = -1;
		SocketAdminList[sockNr].RemoteIpAddress = inet_addr(SoAd_Config.SocketConnection[sockNr].SocketRemoteIpAddress);
		SocketAdminList[sockNr].RemotePort = htons(SoAd_Config.SocketConnection[sockNr].SocketRemotePort);

		SocketAdminList[sockNr].SocketState = SOCKET_TCP_LISTENING;
		for (i = 0; i < SOAD_SOCKET_COUNT; i++) {
			if (i == sockNr) continue;

			if (SocketAdminList[sockNr].SocketHandle == SocketAdminList[i].SocketHandle) {
				if (SocketAdminList[i].SocketState == SOCKET_TCP_LISTENING) {
					SocketAdminList[sockNr].SocketState = SOCKET_DUPLICATE;
					break;
				}
			}
		}

		break;

	default:
		/* This should never happen! */
		DET_REPORTERROR(MODULE_ID_SOAD, 0, SOAD_SOCKET_CLOSE_ID, SOAD_E_SHALL_NOT_HAPPEN);
		break;

	}

}


void SoAd_SocketStatusCheck(uint16 sockNr, int sockHandle)
{
	int sockErr;
	socklen_t sockErrLen = sizeof(sockErr);

	lwip_getsockopt(sockHandle, SOL_SOCKET, SO_ERROR, &sockErr, &sockErrLen);
	if ((sockErr != 0) && (sockErr != EWOULDBLOCK)) {
		SoAd_SocketClose(sockNr);
	}
}


uint16 SoAd_SendIpMessage(uint16 sockNr, uint32 msgLen, uint8* buff)
{
	uint16 bytesSent;

	if (SocketAdminList[sockNr].SocketProtocolIsTcp) {
	    printf("lwip_send: %c,%c,%d,%d,%d,%d\r\n", buff[0],buff[1], buff[2], buff[3], buff[4], buff[5]);
	    printf("lwip_send: %d,%d,%d,%d,%d,%d\r\n", buff[6],buff[7], buff[8], buff[9], buff[10], buff[11]);
	    printf("lwip_send: %d,%d,%d,%d,%d,%d\r\n", buff[12],buff[13], buff[14], buff[15], buff[16], buff[17]);
	    printf("lwip_send: %d,%d,%d,%d,%d,%d\r\n", buff[18],buff[19], buff[20], buff[21], buff[22], buff[23]);
	    printf("lwip_send: %d,%d,%d,%d,%d,%d,%d\r\n", buff[24],buff[25], buff[26], buff[27], buff[28], buff[29], buff[30]);
//	    printf("lwip_send: %d,%d\r\n", buff[6],buff[7]);
		bytesSent = lwip_send(SocketAdminList[sockNr].ConnectionHandle, buff, msgLen, 0);
	} else {
	    struct sockaddr_in toAddr;
	    socklen_t toAddrLen = sizeof(toAddr);
	    toAddr.sin_family = AF_INET;
	    toAddr.sin_len = sizeof(toAddr);

	    toAddr.sin_addr.s_addr = SocketAdminList[sockNr].RemoteIpAddress;
	    toAddr.sin_port = SocketAdminList[sockNr].RemotePort;
		bytesSent = lwip_sendto(SocketAdminList[sockNr].SocketHandle, buff, msgLen, 0, (struct sockaddr *)&toAddr, toAddrLen);
	}

	return bytesSent;
}


static void socketCreate(uint16 sockNr)
{
    int sockFd;
    int sockType;
    struct sockaddr_in sLocalAddr;

    if (SocketAdminList[sockNr].SocketProtocolIsTcp) {
    	sockType = SOCK_STREAM;
    } else {
    	sockType = SOCK_DGRAM;
    }


    sockFd = lwip_socket(AF_INET, sockType, 0);
    if (sockFd >= 0) {
    	memset((char *)&sLocalAddr, 0, sizeof(sLocalAddr));

    	int on = 1;
    	lwip_ioctl(sockFd, FIONBIO, &on);
//    	lwip_setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int) );		// shuzhou add
    	lwip_setsockopt(sockFd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(int));	// Set socket to no delay

    	/*Source*/
    	sLocalAddr.sin_family = AF_INET;
    	sLocalAddr.sin_len = sizeof(sLocalAddr);

    	sLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);		// TODO: Use IP from configuration instead
    	sLocalAddr.sin_port = htons(SocketAdminList[sockNr].SocketConnectionRef->SocketLocalPort);

    	if(lwip_bind(sockFd, (struct sockaddr *)&sLocalAddr, sizeof(sLocalAddr)) >= 0) {

            if (!SocketAdminList[sockNr].SocketProtocolIsTcp) {
            	// Now the UDP socket is ready for receive/transmit
            	SocketAdminList[sockNr].SocketHandle = sockFd;
            	SocketAdminList[sockNr].SocketState = SOCKET_UDP_READY;
            } else {
                if  ( lwip_listen(sockFd, 20) == 0 ){	// TODO: What number of the backlog?
                	// Now the TCP socket is ready for receive/transmit
                	SocketAdminList[sockNr].SocketHandle = sockFd;
                	SocketAdminList[sockNr].SocketState = SOCKET_TCP_LISTENING;
                } else {
                	lwip_close(sockFd);
                }
            }
    	} else {
    		lwip_close(sockFd);
    	}
    } else {
    	// Socket creation failed
    	// Do nothing, try again later
    }
}


static void socketAccept(uint16 sockNr)
{
	uint16 i;
	int clientFd;
	struct sockaddr_in client_addr;
	int addrlen = sizeof(client_addr);

	clientFd = lwip_accept(SocketAdminList[sockNr].SocketHandle, (struct sockaddr*)&client_addr, (socklen_t *)&addrlen);

	if( clientFd != (-1))
	{
		// Check that remote port and ip match
		// TODO: Check remote port and ip with SocketAdminList and select first matching

		// New connection established
		int on = 1;
    	lwip_ioctl(clientFd, FIONBIO, &on);	// Set socket to non block mode

    	lwip_setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(int));	// Set socket to no delay

    	SocketAdminList[sockNr].ConnectionHandle = clientFd;
		SocketAdminList[sockNr].RemotePort = client_addr.sin_port;
		SocketAdminList[sockNr].RemoteIpAddress = client_addr.sin_addr.s_addr;
		SocketAdminList[sockNr].SocketState = SOCKET_TCP_READY;

		// Check if there is any free duplicate of this socket
		for (i = 0; i < SOAD_SOCKET_COUNT; i++) {
			if ((SocketAdminList[i].SocketState == SOCKET_DUPLICATE)
				&& (SoAd_Config.SocketConnection[i].SocketProtocol == SoAd_Config.SocketConnection[sockNr].SocketProtocol)
				&& (SoAd_Config.SocketConnection[i].SocketLocalPort == SoAd_Config.SocketConnection[sockNr].SocketLocalPort)) {
				// Yes, move the old socket to this
				SocketAdminList[i].SocketHandle = SocketAdminList[sockNr].SocketHandle;
				SocketAdminList[i].SocketState = SOCKET_TCP_LISTENING;
//				pi_printf("infor: socket accept "); mini_uart_sendDec(i); pi_printf(" \r\n");
				// SocketAdminList[sockNr].SocketHandle = -1;
				break;
			}
		}
		network_up = true;
	}
}

uint8 SoAd_GetNofCurrentlyUsedTcpSockets() {
	uint8 count = 0;
	uint16 i;

	for (i = 0; i < SOAD_SOCKET_COUNT; i++) {
		if ((SOCKET_TCP_READY == SocketAdminList[i].SocketState) && SocketAdminList[i].SocketProtocolIsTcp) {
			count++;
		}
	}

	return count;
}

static void socketTcpRead(uint16 sockNr)
{
    BufReq_ReturnType result;

	switch (SocketAdminList[sockNr].SocketConnectionRef->AutosarConnectorType) {
	case SOAD_AUTOSAR_CONNECTOR_PDUR:
		if (SocketAdminList[sockNr].SocketRouteRef != NULL) {
		    int nBytes;
		    PduInfoType pduInfo;
//		    pi_printf("infor: tcp read\r\n");
			if (SoAd_BufferGet(SOAD_RX_BUFFER_SIZE, &pduInfo.SduDataPtr)) {
				nBytes = lwip_recv(SocketAdminList[sockNr].ConnectionHandle, pduInfo.SduDataPtr, SOAD_RX_BUFFER_SIZE, MSG_PEEK);
//				pi_printf("infor: peek data "); mini_uart_sendDec(nBytes); pi_printf("\r\n");
				SoAd_SocketStatusCheck(sockNr, SocketAdminList[sockNr].ConnectionHandle);

				if ((nBytes > 0) && (nBytes >= SocketAdminList[sockNr].SocketRouteRef->DestinationSduLength)) {
					if  (!SocketAdminList[sockNr].SocketConnectionRef->PduProvideBufferEnable) {
						// IF-type
//						pi_printf(" infor: plan to read\r\n");
						pduInfo.SduLength = lwip_recv(SocketAdminList[sockNr].ConnectionHandle, pduInfo.SduDataPtr, SocketAdminList[sockNr].SocketRouteRef->DestinationSduLength, 0);
//						pi_printf(" infor: value = "); mini_uart_sendDec(*pduInfo.SduDataPtr); pi_printf("\r\n");
						New_Data_Flag = true;
						(void)PduR_SoAdIfRxIndication(SocketAdminList[sockNr].SocketRouteRef->DestinationPduId, &pduInfo);
					} else {
						// TP-type
					    PduLengthType len;
					    result = PduR_SoAdTpStartOfReception(SocketAdminList[sockNr].SocketRouteRef->DestinationPduId, SocketAdminList[sockNr].SocketRouteRef->DestinationSduLength, &len);
						if (result == BUFREQ_OK && len>0) {
							pduInfo.SduLength = SocketAdminList[sockNr].SocketRouteRef->DestinationSduLength;
							nBytes = lwip_recv(SocketAdminList[sockNr].SocketHandle, pduInfo.SduDataPtr, SocketAdminList[sockNr].SocketRouteRef->DestinationSduLength,0);

							/* Let pdur copy received data */
							if(len < SocketAdminList[sockNr].SocketRouteRef->DestinationSduLength)
							{
							    PduInfoType pduInfoChunk;
							    PduLengthType lenToSend = SocketAdminList[sockNr].SocketRouteRef->DestinationSduLength;
								/* We need to copy in smaller parts */
							    pduInfoChunk.SduDataPtr = pduInfo.SduDataPtr;

							    while(lenToSend > 0)
							    {
							    	if(lenToSend >= len){
							    		PduR_SoAdTpCopyRxData(SocketAdminList[sockNr].SocketRouteRef->DestinationPduId, &pduInfoChunk, &len);
							    		lenToSend -= len;
							    	}else{
							    		PduR_SoAdTpCopyRxData(SocketAdminList[sockNr].SocketRouteRef->DestinationPduId, &pduInfoChunk, &lenToSend);
							    		lenToSend = 0;
							    	}
							    }
							}else{
								PduR_SoAdTpCopyRxData(SocketAdminList[sockNr].SocketRouteRef->DestinationPduId, &pduInfo, &len);
							}
							/* Finished reception */
							(void)PduR_SoAdTpRxIndication(SocketAdminList[sockNr].SocketRouteRef->DestinationPduId, NTFRSLT_OK);
						} else if (result != BUFREQ_BUSY) {
							DET_REPORTERROR(MODULE_ID_SOAD, 0, SOAD_SOCKET_TCP_READ_ID, SOAD_E_UL_RXBUFF);
						}
					}
				}

				SoAd_BufferFree(pduInfo.SduDataPtr);
			} else {
				// No buffer available: continue poll() Report error SOAD_E_UPPERBUFF to DEM
//				DET_REPORTERROR(MODULE_ID_SOAD, 0, SOAD_SOCKET_TCP_READ_ID, SOAD_E_UPPERBUFF);
				(void)PduR_SoAdTpRxIndication(SocketAdminList[sockNr].SocketRouteRef->DestinationPduId, NTFRSLT_E_NO_BUFFER);
			}
		} else {
			DET_REPORTERROR(MODULE_ID_SOAD, 0, SOAD_SOCKET_TCP_READ_ID, SOAD_E_CONFIG_INVALID);
		}
		break;	// SOAD_AUTOSAR_CONNECTOR_PDUR

	case SOAD_AUTOSAR_CONNECTOR_DOIP:
		DoIp_HandleTcpRx(sockNr);
		break;	// SOAD_AUTOSAR_CONNECTOR_DOIP

	default:
		DET_REPORTERROR(MODULE_ID_SOAD, 0, SOAD_SOCKET_TCP_READ_ID, SOAD_E_NOCONNECTOR);
		break;
	}
}


static void socketUdpRead(uint16 sockNr)
{
    BufReq_ReturnType result;

	switch (SocketAdminList[sockNr].SocketConnectionRef->AutosarConnectorType) {
	case SOAD_AUTOSAR_CONNECTOR_PDUR:
		if (SocketAdminList[sockNr].SocketRouteRef != NULL) {
			struct sockaddr_in fromAddr;
		    socklen_t fromAddrLen = sizeof(fromAddr);
		    int nBytes;
		    PduInfoType pduInfo;

			if (SoAd_BufferGet(SOAD_RX_BUFFER_SIZE, &pduInfo.SduDataPtr)) {
			    nBytes = lwip_recvfrom(SocketAdminList[sockNr].SocketHandle, pduInfo.SduDataPtr, SOAD_RX_BUFFER_SIZE, MSG_PEEK, (struct sockaddr*)&fromAddr, &fromAddrLen);
				SoAd_SocketStatusCheck(sockNr, SocketAdminList[sockNr].SocketHandle);

				if (nBytes > 0){
					if(nBytes >= SocketAdminList[sockNr].SocketRouteRef->DestinationSduLength) {
						if  (!SocketAdminList[sockNr].SocketConnectionRef->PduProvideBufferEnable) {
							// IF-type
							pduInfo.SduLength = lwip_recvfrom(SocketAdminList[sockNr].SocketHandle, pduInfo.SduDataPtr, SocketAdminList[sockNr].SocketRouteRef->DestinationSduLength,0, (struct sockaddr*)&fromAddr, &fromAddrLen);
							SocketAdminList[sockNr].RemotePort = fromAddr.sin_port;
							SocketAdminList[sockNr].RemoteIpAddress = fromAddr.sin_addr.s_addr;
							/* TODO Find out how autosar connector and user really shall be used. This is just one interpretation
							 * support for XCP, CDD will have to be added later when supported */
							switch(SocketAdminList[sockNr].SocketRouteRef->UserRxIndicationUL){
	#if defined(USE_UDPNM)
							case SOAD_UL_UDPNM:
								(void)UdpNm_SoAdIfRxIndication(SocketAdminList[sockNr].SocketRouteRef->DestinationPduId, &pduInfo);
								break;
	#endif
							default:
								(void)PduR_SoAdIfRxIndication(SocketAdminList[sockNr].SocketRouteRef->DestinationPduId, &pduInfo);
								break;
							}
						} else {
							// TP-type
							PduLengthType len;
							result = PduR_SoAdTpStartOfReception(SocketAdminList[sockNr].SocketRouteRef->DestinationPduId, SocketAdminList[sockNr].SocketRouteRef->DestinationSduLength, &len);
							if (result == BUFREQ_OK && len > 0) {
								pduInfo.SduLength = SocketAdminList[sockNr].SocketRouteRef->DestinationSduLength;
								nBytes = lwip_recvfrom(SocketAdminList[sockNr].SocketHandle, pduInfo.SduDataPtr, SocketAdminList[sockNr].SocketRouteRef->DestinationSduLength,0, (struct sockaddr*)&fromAddr, &fromAddrLen);
								SocketAdminList[sockNr].RemotePort = fromAddr.sin_port;
								SocketAdminList[sockNr].RemoteIpAddress = fromAddr.sin_addr.s_addr;

								/* Let pdur copy received data */
								if(len < SocketAdminList[sockNr].SocketRouteRef->DestinationSduLength)
								{
									PduInfoType pduInfoChunk;
									PduLengthType lenToSend = SocketAdminList[sockNr].SocketRouteRef->DestinationSduLength;
									/* We need to copy in smaller parts */
									pduInfoChunk.SduDataPtr = pduInfo.SduDataPtr;

									while(lenToSend > 0)
									{
										if(lenToSend >= len){
											PduR_SoAdTpCopyRxData(SocketAdminList[sockNr].SocketRouteRef->DestinationPduId, &pduInfoChunk, &len);
											lenToSend -= len;
										}else{
											PduR_SoAdTpCopyRxData(SocketAdminList[sockNr].SocketRouteRef->DestinationPduId, &pduInfoChunk, &lenToSend);
											lenToSend = 0;
										}
									}
								}else{
									PduR_SoAdTpCopyRxData(SocketAdminList[sockNr].SocketRouteRef->DestinationPduId, &pduInfo, &len);
								}
								/* Finished reception */
								(void)PduR_SoAdTpRxIndication(SocketAdminList[sockNr].SocketRouteRef->DestinationPduId, NTFRSLT_OK);
							} else if (result != BUFREQ_BUSY) {
								DET_REPORTERROR(MODULE_ID_SOAD, 0, SOAD_SOCKET_UDP_READ_ID, SOAD_E_UL_RXBUFF);
								(void)PduR_SoAdTpRxIndication(SocketAdminList[sockNr].SocketRouteRef->DestinationPduId, NTFRSLT_E_NO_BUFFER);
							}
						}
					}
				}

				SoAd_BufferFree(pduInfo.SduDataPtr);
			} else {
				// No buffer available: continue poll() Report error SOAD_E_UPPERBUFF to DEM
//				DET_REPORTERROR(MODULE_ID_SOAD, 0, SOAD_SOCKET_UDP_READ_ID, SOAD_E_UPPERBUFF);
				(void)PduR_SoAdTpRxIndication(SocketAdminList[sockNr].SocketRouteRef->DestinationPduId, NTFRSLT_E_NO_BUFFER);
			}
		} else {
			DET_REPORTERROR(MODULE_ID_SOAD, 0, SOAD_SOCKET_UDP_READ_ID, SOAD_E_CONFIG_INVALID);
		}
		break;	// SOAD_AUTOSAR_CONNECTOR_PDUR

	case SOAD_AUTOSAR_CONNECTOR_DOIP:
		DoIp_HandleUdpRx(sockNr);
		break;	// SOAD_AUTOSAR_CONNECTOR_DOIP

	default:
		DET_REPORTERROR(MODULE_ID_SOAD, 0, SOAD_SOCKET_UDP_READ_ID, SOAD_E_NOCONNECTOR);
		break;
	}
}


static void scanSockets(void)
{
	uint16 i;

	for (i = 0; i < SOAD_SOCKET_COUNT; i++) {
//		pi_printf("infor: scan sockets "); mini_uart_sendDec(i); pi_printf(" \r\n");
		switch (SocketAdminList[i].SocketState) {
		case SOCKET_INIT:

			socketCreate(i);
//			pi_printf("infor: socket create"); mini_uart_sendDec(i); pi_printf(" \r\n");
			break;

		case SOCKET_TCP_LISTENING:
			socketAccept(i);
//			pi_printf("infor: socket accept"); mini_uart_sendDec(i); pi_printf(" \r\n");
			break;

		case SOCKET_TCP_READY:

//			pi_printf("infor: socket read "); mini_uart_sendDec(i); pi_printf(" \r\n");
			socketTcpRead(i);
			break;

		case SOCKET_UDP_READY:
			socketUdpRead(i);
			break;

		case SOCKET_DUPLICATE:
			/* Do nothing */
			break;

		default:
			/* This should never happen! */
			DET_REPORTERROR(MODULE_ID_SOAD, 0, SOAD_SCAN_SOCKETS_ID, SOAD_E_SHALL_NOT_HAPPEN);
			break;
		} /* End of switch */
	} /* End of for */


}


static void handleTx()
{

}


/** @req SOAD093 */
void SoAd_Init(void)
{
	sint16 i, j;

	// Initiate the socket administration list
	for (i = 0; i < SOAD_SOCKET_COUNT; i++) {
		SocketAdminList[i].SocketNr = i;
		SocketAdminList[i].SocketState = SOCKET_INIT;
		SocketAdminList[i].SocketConnectionRef = &SoAd_Config.SocketConnection[i];
		SocketAdminList[i].RemoteIpAddress = inet_addr(SoAd_Config.SocketConnection[i].SocketRemoteIpAddress);
		SocketAdminList[i].RemotePort = htons(SoAd_Config.SocketConnection[i].SocketRemotePort);
		SocketAdminList[i].SocketHandle = -1;
		SocketAdminList[i].ConnectionHandle = -1;

		SocketAdminList[i].SocketProtocolIsTcp = FALSE;
		if (SoAd_Config.SocketConnection[i].SocketProtocol == SOAD_SOCKET_PROT_TCP) {
			SocketAdminList[i].SocketProtocolIsTcp = TRUE;
		} else if (SoAd_Config.SocketConnection[i].SocketProtocol != SOAD_SOCKET_PROT_UDP) {
			// Configuration error!!!
			DET_REPORTERROR(MODULE_ID_SOAD, 0, SOAD_INIT_ID, SOAD_E_CONFIG_INVALID);
		}

		// Check if several connections are expected on this port.
		for (j = 0; j < i; j++) {
			if ((SoAd_Config.SocketConnection[i].SocketProtocol == SoAd_Config.SocketConnection[j].SocketProtocol)
				&& (SoAd_Config.SocketConnection[i].SocketLocalPort == SoAd_Config.SocketConnection[j].SocketLocalPort)) {
				SocketAdminList[i].SocketState = SOCKET_DUPLICATE;
				break;
			}
		}
	}

	// Cross reference from SocketAdminList to SocketRoute
	for (i = 0; i < SOAD_SOCKET_ROUTE_COUNT; i++) {
		if (SoAd_Config.SocketRoute[i].SourceSocketRef != NULL) {
			if (SoAd_Config.SocketRoute[i].SourceSocketRef->SocketId < SOAD_SOCKET_COUNT) {
				SocketAdminList[SoAd_Config.SocketRoute[i].SourceSocketRef->SocketId].SocketRouteRef = &SoAd_Config.SocketRoute[i];
			} else {
				DET_REPORTERROR(MODULE_ID_SOAD, 0, SOAD_INIT_ID, SOAD_E_CONFIG_INVALID);
			}
		} else {
			DET_REPORTERROR(MODULE_ID_SOAD, 0, SOAD_INIT_ID, SOAD_E_CONFIG_INVALID);
		}
	}
	
	// Initialize PduStatus of PduAdminList
	for (i = 0; i < SOAD_PDU_ROUTE_COUNT; i++) {
		PduAdminList[i].PduStatus = PDU_IDLE;
	}


	DoIp_Init();
	TcpIp_Init();

	ModuleStatus = SOAD_INITIALIZED;
}


/** @req SOAD092 */
Std_ReturnType SoAd_Shutdown(void)
{
	return E_OK;
}


/** @req SOAD121 */
void SoAd_MainFunction(void)
{
	if (ModuleStatus != SOAD_UNINITIALIZED) {
		scanSockets();
		handleTx();

	}
}


/** @req SOAD209 */
void SoAd_Cbk_LocalIpAssignmentChg( uint8 Index, boolean Valid, SoAd_SockAddrType Address ) {
	// TODO: use Index (and make sure it's valid)
	if (TRUE == Valid) {
		// Link is up.
		// Start timer so that we can send out vehicle announcements.
		LinkStatus = SOAD_ARC_LINKUP;
#if SOAD_DOIP_ACTIVE == STD_ON
		DoIp_LocalIpAddrAssignmentChg((SoAd_SoConIdType)Index, TCPIP_IPADDR_STATE_ASSIGNED);

#endif

	} else {
		// Stop timer so that we can stop sending out vehicle announcements.
		LinkStatus = SOAD_ARC_LINKDOWN;

#if SOAD_DOIP_ACTIVE == STD_ON
		DoIp_LocalIpAddrAssignmentChg((SoAd_SoConIdType)Index, TCPIP_IPADDR_STATE_ASSIGNED);
#endif

	}
}

/** @req SOAD127 */
void SoAd_SocketReset(void)
{

}


/** @req SOAD091 */
Std_ReturnType SoAdIf_Transmit(PduIdType SoAdSrcPduId, const PduInfoType* SoAdSrcPduInfoPtr)
{
	Std_ReturnType returnCode = E_OK;
	uint16 socketNr;

	VALIDATE_RV(ModuleStatus != SOAD_UNINITIALIZED, SOAD_IF_TRANSMIT_ID, SOAD_E_NOTINIT, E_NOT_OK);	/** @req SOAD213 *//** @req SOAD217 */
	VALIDATE_RV(SoAdSrcPduInfoPtr != NULL, SOAD_IF_TRANSMIT_ID, SOAD_E_NULL_PTR, E_NOT_OK);	/** @req SOAD235 */
	VALIDATE_RV(SoAdSrcPduId < SOAD_PDU_ROUTE_COUNT, SOAD_IF_TRANSMIT_ID, SOAD_E_INVAL, E_NOT_OK);	/** @req SOAD214 */

//	pi_printf("infor: SoAdIf = ");mini_uart_sendDec(*SoAdSrcPduInfoPtr->SduDataPtr);pi_printf("\r\n");

	if (SoAd_Config.PduRoute[SoAdSrcPduId].SourceSduLength == SoAdSrcPduInfoPtr->SduLength) {
//		pi_printf("infor: SoAd1\r\n");
		if (PduAdminList[SoAdSrcPduId].PduStatus == PDU_IDLE ) {
//			pi_printf("infor: SoAd2\r\n");
			if (SoAd_Config.PduRoute[SoAdSrcPduId].DestinationSocketRef != NULL ) {
//				pi_printf("infor: SoAd3\r\n");
				socketNr = SoAd_Config.PduRoute[SoAdSrcPduId].DestinationSocketRef->SocketId;
				if ((SocketAdminList[socketNr].SocketState == SOCKET_TCP_LISTENING)
					|| (SocketAdminList[socketNr].SocketState == SOCKET_TCP_READY)
					|| (SocketAdminList[socketNr].SocketState == SOCKET_UDP_READY)) {
						PduAdminList[SoAdSrcPduId].PduStatus = PDU_IF_SENDING;
//						pi_printf("infor: SoAdIp = ");mini_uart_sendDec(*SoAdSrcPduInfoPtr->SduDataPtr);pi_printf("\r\n");
						SoAd_SendIpMessage(socketNr, SoAdSrcPduInfoPtr->SduLength, SoAdSrcPduInfoPtr->SduDataPtr);
						switch (SoAd_Config.PduRoute[SoAdSrcPduId].UserTxConfirmationUL)
						{
#if defined(USE_UDPNM)
						case SOAD_UL_UDPNM:
							UdpNm_SoAdIfTxConfirmation(SoAd_Config.PduRoute[SoAdSrcPduId].SourcePduId);
							break;
#endif
						default:
							PduR_SoAdIfTxConfirmation(SoAd_Config.PduRoute[SoAdSrcPduId].SourcePduId);
							break;
						}
						PduAdminList[SoAdSrcPduId].PduStatus = PDU_IDLE;
				} else {
					/* Socket not ready */
					returnCode = E_NOT_OK;
				}
			} else {
				/* SocketAdminRef is NULL */
				returnCode = E_NOT_OK;
			}
		} else {
			/* A PDU is already in progress */
			returnCode = E_NOT_OK;
		}
	} else {
		/* Attached length and length from PduRoutingTable does not match */
		returnCode = E_NOT_OK;
	}

	return returnCode;
}


/** @req SOAD105 */
Std_ReturnType SoAdTp_Transmit(PduIdType SoAdSrcPduId, const PduInfoType* SoAdSrcPduInfoPtr)
{
	Std_ReturnType returnCode = E_OK;
	PduInfoType txPduInfo;
	uint16 socketNr;

	VALIDATE_RV(ModuleStatus != SOAD_UNINITIALIZED, SOAD_TP_TRANSMIT_ID, SOAD_E_NOTINIT, E_NOT_OK);	/** @req SOAD224 *//** @req SOAD223 */
	VALIDATE_RV(SoAdSrcPduInfoPtr != NULL, SOAD_TP_TRANSMIT_ID, SOAD_E_NULL_PTR, E_NOT_OK);	/** @req SOAD225 */
	VALIDATE_RV(SoAdSrcPduId < SOAD_PDU_ROUTE_COUNT, SOAD_TP_TRANSMIT_ID, SOAD_E_INVAL, E_NOT_OK);	/** @req SOAD237 */

	if (SoAd_Config.PduRoute[SoAdSrcPduId].DestinationSocketRef != NULL ) {
//		pi_printf("infor: destination\r\n");
		switch (SoAd_Config.PduRoute[SoAdSrcPduId].DestinationSocketRef->AutosarConnectorType) {
		case SOAD_AUTOSAR_CONNECTOR_PDUR:
//			pi_printf("infor: soad autosar pdur\r\n");
			if (SoAd_Config.PduRoute[SoAdSrcPduId].SourceSduLength == SoAdSrcPduInfoPtr->SduLength) {	// TODO: Shall this test be done or all length be allowed?
//				pi_printf("infor: length\r\n");
				if (PduAdminList[SoAdSrcPduId].PduStatus == PDU_IDLE ) {
//					pi_printf("infor: pdu idle\r\n");
					socketNr = SoAd_Config.PduRoute[SoAdSrcPduId].DestinationSocketRef->SocketId;
					if ((SocketAdminList[socketNr].SocketState == SOCKET_TCP_LISTENING)
						|| (SocketAdminList[socketNr].SocketState == SOCKET_TCP_READY)
						|| (SocketAdminList[socketNr].SocketState == SOCKET_UDP_READY)) {
							PduAdminList[SoAdSrcPduId].PduStatus = PDU_TP_REQ_BUFFER;
//							pi_printf("infor: tcp ready\r\n");
							txPduInfo.SduLength = SoAdSrcPduInfoPtr->SduLength;

							if(SoAd_BufferGet(txPduInfo.SduLength, &txPduInfo.SduDataPtr))
							{
								PduLengthType availableData;
								PduR_SoAdTpCopyTxData(SoAd_Config.PduRoute[SoAdSrcPduId].SourcePduId, &txPduInfo, /* retry */NULL, &availableData );
							}else{
								// No free buffer. Inform PduR..
								PduR_SoAdTpTxConfirmation(SoAd_Config.PduRoute[SoAdSrcPduId].SourcePduId, NTFRSLT_E_NO_BUFFER);
								PduAdminList[SoAdSrcPduId].PduStatus = PDU_TP_SENDING;
//								pi_printf("infor: soad ip message\r\n");
								SoAd_SendIpMessage(socketNr, txPduInfo.SduLength, txPduInfo.SduDataPtr);
								PduR_SoAdTpTxConfirmation(SoAd_Config.PduRoute[SoAdSrcPduId].SourcePduId, NTFRSLT_OK);
							}
							SoAd_BufferFree(txPduInfo.SduDataPtr);
							PduAdminList[SoAdSrcPduId].PduStatus = PDU_IDLE;
					} else {
						/* Socket not ready */
						returnCode = E_NOT_OK;
					}
				} else {
					/* A PDU is already in progress */
					returnCode = E_NOT_OK;
				}
			} else {
				/* Attached length and length from PduRoutingTable does not match */
				returnCode = E_NOT_OK;
			}
			break;

		case SOAD_AUTOSAR_CONNECTOR_DOIP:
			returnCode = DoIp_HandleTpTransmit(SoAdSrcPduId, SoAdSrcPduInfoPtr);
			break;

		default:
			// Not supported connector type
			returnCode = E_NOT_OK;
			break;
		} /* End of switch */

	} else {
		/* SocketAdminRef is NULL */
		returnCode = E_NOT_OK;
	}

	return returnCode;
}


/** @req SOAD193 */
void TcpIp_Init(void)
{
    LwIP_Init();
    pi_printf("infor: lwip initialized\r\n");
}


/** @req SOAD194 */
void TcpIp_Shutdown(void)
{

}


/** @req SOAD143 */
void TcpIp_MainFunctionCyclic(void)
{

}


/** @req SOAD196*/
Std_ReturnType TcpIp_SetDhcpHostNameOption(uint8* HostNameOption, uint8 HostNameLen)
{
	return E_OK;
}

