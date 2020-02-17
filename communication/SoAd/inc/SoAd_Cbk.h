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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.2.2 */

#ifndef SOAD_CBK_H_
#define SOAD_CBK_H_

/** @req SWS_SOAD_00097 */
void SoAd_RxIndication( TcpIp_SocketIdType SocketId, const TcpIp_SockAddrType* RemoteAddrPtr, uint8* BufPtr, uint16 Length );

/** @req SWS_SOAD_00523 */
BufReq_ReturnType SoAd_CopyTxData( TcpIp_SocketIdType SocketId, uint8* BufPtr, uint16 BufLength );

/** @req SWS_SOAD_00098 */
void SoAd_TxConfirmation( TcpIp_SocketIdType SocketId, uint16 Length );

/** @req SWS_SOAD_00099 */
Std_ReturnType SoAd_TcpAccepted( TcpIp_SocketIdType SocketId, TcpIp_SocketIdType SocketIdConnected, const TcpIp_SockAddrType* RemoteAddrPtr );

/** @req SWS_SOAD_00100 */
void SoAd_TcpConnected( TcpIp_SocketIdType SocketId );

/** @req SWS_SOAD_00146 */
void SoAd_TcpIpEvent( TcpIp_SocketIdType SocketId, TcpIp_EventType Event );

/** @req SWS_SOAD_00209 */
void SoAd_LocalIpAddrAssignmentChg( TcpIp_LocalAddrIdType IpAddrId, TcpIp_IpAddrStateType State );

#endif /* SOAD_CBK_H_ */
