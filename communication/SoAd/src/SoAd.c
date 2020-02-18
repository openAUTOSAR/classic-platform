
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

/*lint -w2 */
/*lint -e534 */


/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.2.2 */
/** @req SWS_SOAD_00001 All configuration items shall be located outside the kernel of the module. */
/** @req SWS_SOAD_00093 SoAd_Init() */
/** @req SWS_SOAD_00096 SoAd_GetVersionInfo() */
/** @req SWS_SOAD_00106 <Up>_[SoAd][If]RxIndication() */
/** @req SWS_SOAD_00107 <Up>_[SoAd][If]TxConfirmation() */
/** @req SWS_SOAD_00505 The SoAd shall provide sufficient buffers for rx. */
/** @req SWS_SOAD_00599 The SoAd shall provide sufficient buffers for tx. */
/** @req SWS_SOAD_00513 <Up>_LocalIpAddrAssignmentChg() */
/** @req SWS_SOAD_00514 <Up>_SoConModeChg() */
/** @req SWS_SOAD_00538 Use SoAdBSWModules to determine upper layer function names */
/** @req SWS_SOAD_00663 <Up>_[SoAd][If]TriggerTransmit() */


/** @req SWS_SOAD_00503 */

#include "SoAd.h"

#include <string.h>

#include "cirq_buffer.h"
#include "ComStack_Types.h"
#include "Platform_Types.h"
#include "Std_Types.h"
#include "TcpIp.h"
#include "TcpIp_Types.h"
#include "SoAd_Cfg.h"
#include "SchM_SoAd.h"

#if defined(USE_DET) && (SOAD_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif


#include "Os.h"


/* ----------------------------[private macro]-------------------------------*/


/*lint -emacro(904,VALIDATE_NO_RV,VALIDATE_RV)*/ /*904 PC-Lint exception to MISRA 14.7 (validate DET macros)*/
#if defined(USE_DET) && (SOAD_DEV_ERROR_DETECT == STD_ON)
#define VALIDATE_RV(_exp,_api,_err,_rv) \
        if( !(_exp) ) { \
          (void)Det_ReportError(SOAD_MODULE_ID,0,_api,_err); \
          return _rv; \
        }

#define VALIDATE(_exp,_api,_err ) \
        VALIDATE_RV(_exp,_api,_err, (E_NOT_OK))

#define VALIDATE_NO_RV(_exp,_api,_err ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(SOAD_MODULE_ID,0,_api,_err); \
          return; \
        }

#define DET_REPORTERROR(_x,_y,_z,_q) (void)Det_ReportError(_x, _y, _z, _q)
#else
#define VALIDATE_RV(_exp,_api,_err,_rv)
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif


#define IS_UDP_GRP(_x) ((ConnectionGroupAdminList[_x].SocketConnectionGroupRef->GroupType == SOAD_GROUPTYPE_LONELY_UDP) || (ConnectionGroupAdminList[_x].SocketConnectionGroupRef->GroupType == SOAD_GROUPTYPE_MULTI_UDP))
#define SOAD_PDU_ROUTE_FIRST_DESTINATION_IDX 0u /*Index of the fist PDU route destination */

/* Structure to keep run time paraemeters for TP transmission */
typedef struct {
    TcpIp_SocketIdType  socketId;
    PduLengthType       totalLen;
    boolean             tpTxRequest;
    boolean             firstCopyCalled;
    PduLengthType       remainingLength;
}SoadTptransmitType;
/* Structure to keep run time parameters for Tp reception */
typedef struct {

    PduLengthType       remaingLen;
    uint8               socketRouteIndex;
    boolean             tpRxRequest;
    boolean             firstCopyCalled;
}SoadTpReceiveType;


typedef struct {
    TcpIp_SocketIdType 						SocketId;
    SoAd_SoConModeType						mode;	// Maybe not needed? Used in one place at the moment
    TcpIp_IpAddrStateType					IpAddrState;
    uint16									localPortUsed;
    const SoAd_SocketConnectionGroupType* 	SocketConnectionGroupRef;
}ConnectionGroupAdminType;

typedef struct {
    TcpIp_SocketIdType 						SocketId;
    SoAd_SoConModeType						mode;
    boolean                             	openRequest;
    boolean                             	closeRequest;
    boolean									abort;	// From SoAd_CloseSoCon()
    boolean                             	opened;
    boolean 								closedByCloseSoCon;	// closed by SoAd_CloseSoCon()?
    uint32									bytesInPduUdpBuffer;
    CirqBufferType							rxBuffer;
    /** @req SWS_SOAD_00696 */
    uint32									udpBufTimer;
    uint32									udpAliveTimer;
    SoAd_SocketRemoteAddressType			remAddrInUse;
    ConnectionGroupAdminType*				groupAdminRef;
    const SoAd_SocketConnectionType*		SocketConnectionRef;
}ConnectionAdminType;

typedef enum {
    SOAD_TRIGGERTRANSMITMODE_OFF = 0,
    SOAD_TRIGGERTRANSMITMODE_ROUTINGGROUP,
    SOAD_TRIGGERTRANSMITMODE_ROUTINGGROUP_AND_SOCON,
} TriggerTransmitMode;

typedef struct {
    TriggerTransmitMode	mode;
    uint32 		RoutingGroup;
    uint32 		SoConId;
}TriggerTransmitInfoType;

typedef struct {
    TriggerTransmitInfoType	trigger;
    TxStateType	TxState;
    uint32		unconfirmedMessages;
}PduRouteAdminType;

typedef struct {
    const SoAd_PduRouteDestType* PduRouteDest;
    uint32 RoutingGroup;
    uint32 SoConId;
    boolean Active;
    boolean TriggerTransmit;
}PduRoutingGroupAdminType;

typedef struct {
    const SoAd_SocketRouteDestType* SocketRouteDest;
    uint32 RoutingGroup;
    uint32 SoConId;
    boolean Active;
}SocketRoutingGroupAdminType;

typedef struct {
    const  uint8* TxBufCopy;
    uint32 TxPduHeaderId;
    /* uint32 TxBufLen; */
    SoAd_SoConIdType 	SoConId;
}SocketTxAdminType;

typedef struct {
    boolean SoAd_TpCancelTranStatus;
    boolean SoAd_TpCancelReceiveStatus;
}SoAd_TpCancelStatusType;

const SoAd_ConfigType *SoAdCfgPtr = NULL;

typedef enum {
  SOAD_UNINITIALIZED = 0,
  SOAD_INITIALIZED
} SoadStateType;
#define TCP_TP_RECEIVE_BUFFER_SIZE    1500u
#define SOAD_INVALID_CON_ID           0xFFFFu
#define SOAD_GROUP_INDEX_VALUE_ZERO   0x0u
#define SOCONID_VALUE_ZERO            0x0u
static SoadStateType SoAd_ModuleStatus = SOAD_UNINITIALIZED;
//static SoAd_TpCancelStatusType SoAd_TpCancelStatus;
static SoadTptransmitType SoadTpTransStatus[SOAD_NR_OF_PDU_ROUTES]; /* Array contianing run time parameters for Tp transmit */
static SoadTpReceiveType SoAdTpReceiveStatus[SOAD_NR_OF_SOCKET_CONNECTIONS]; /* Array contianing run time parameters for Tp transmit */


static ConnectionGroupAdminType ConnectionGroupAdminList[SOAD_NR_OF_SOCKET_CONNECTION_GROUPS];
static ConnectionAdminType ConnectionAdminList[SOAD_NR_OF_SOCKET_CONNECTIONS];
static PduRouteAdminType PduRouteAdminList[SOAD_NR_OF_PDU_ROUTES];
#if SOAD_NR_OF_PDU_ROUTING_GROUP_CONNECTIONS > 0
static PduRoutingGroupAdminType PduRoutingGroupAdminList[SOAD_NR_OF_PDU_ROUTING_GROUP_CONNECTIONS];
#endif
static SocketRoutingGroupAdminType SocketRoutingGroupAdminList[SOAD_NR_OF_SOCKET_ROUTING_GROUP_CONNECTIONS];
static SocketTxAdminType   SocketCurrentTx[OS_TASK_CNT];

/* ----------------------------[private functions]---------------------------*/
static void changeMode( SoAd_SoConIdType SoConId, SoAd_SoConModeType mode);
static TcpIp_ProtocolType convertSoAdProtType( const SoAd_SocketProtocolType SoAdProtocol );
static void changeParams( SoAd_SoConIdType SoConId, TcpIp_SocketIdType socketId );
static TcpIp_SockAddrType makeTcpIp_SockAddr( SoAd_SoConIdType SoConId );
static boolean hasWildcardsInAddress(SoAd_SoConIdType SoConId);
static boolean getGroupIndexFromSocketId(TcpIp_SocketIdType SocketId, uint32* index);
static boolean getSoConIdFromSocketId(TcpIp_SocketIdType SocketId, SoAd_SoConIdType* SoConId);
static Std_ReturnType openSocket(SoAd_SoConIdType SoConId);
static Std_ReturnType closeSocket(SoAd_SoConIdType SoConId, boolean initiatedBySoConClose);
static boolean runMessageAcceptancePolicy(void);
static boolean runBestMatch(const SoAd_SocketConnectionGroupType* grp, const TcpIp_SockAddrType* RemoteAddrPtr, uint32* ConnIndexInGroup);
static void setRemoteAddress(SoAd_SoConIdType SoConId, const TcpIp_SockAddrType* RemoteAddrPtr);
static Std_ReturnType transmit(uint16 connIndex, uint32 length,uint8 *dataPtr,uint32 pduHeaderId);
#if SOAD_NR_OF_PDU_ROUTING_GROUP_CONNECTIONS > 0
static Std_ReturnType findActiveRoutingGroup(const SoAd_PduRouteDestType* pduRouteDest, uint16 connIndex );
#endif
static void copyPduToBuffer(uint16 connIndex, const PduInfoType* SoAdSrcPduInfoPtr, boolean useHeader, uint32 headerId);
static Std_ReturnType ifTransmitSubFunction(uint16 connIndex, PduIdType SoAdSrcPduId, const SoAd_PduRouteDestType* pduRouteDest, const PduInfoType* SoAdSrcPduInfoPtr, boolean* atLeastOneDestSent);
static void tpTransmitSubFunction(PduIdType SoAdSrcPduId);
static void setBufTimer(uint16 connIndex, const SoAd_PduRouteDestType* pduRouteDest);
static boolean getPduHeader(const uint8* BufPtr, uint32 Length, uint32* headerId, uint32* PduLength);
static boolean soConInSocketRoute(uint16 SoConId, const SoAd_SocketRouteType* SocketRoute);
static boolean routingReceptionNoHeader(uint16 SoConId, PduInfoType* rxPDU);
static boolean routingReceptionPduHeader(uint16 SoConId, uint32 headerId, uint8* pduBuf, PduLengthType pduLength);
static void setRemoteAddressFromCfg(SoAd_SoConIdType SoConId);
static void processTpTcpRxCommunication (TcpIp_SocketIdType SocketId, SoAd_SoConIdType SoConId, uint8* BufPtr, uint16 Length);

/**
 * Converts a SoAdProtocolType into a TcpIpProtocolType
 * @param SoAdProtocol The variable to convert
 * @return The Protocol as an TcpIp_ProtocolType
 */
static TcpIp_ProtocolType convertSoAdProtType(
        const SoAd_SocketProtocolType SoAdProtocol) {

    TcpIp_ProtocolType TcpIpProt;

    if (SoAdProtocol == SOAD_SOCKET_PROT_TCP) {
        TcpIpProt = TCPIP_IPPROTO_TCP;
    } else if (SoAdProtocol == SOAD_SOCKET_PROT_UDP) {
        TcpIpProt = TCPIP_IPPROTO_UDP;
    } else {
        /* IMPROVEMENT: Add error handling */
        TcpIpProt = TCPIP_IPPROTO_UDP;
    }

    return TcpIpProt;
}

/**
 * Changes TCP/IP parameters for TCP/IP socket according to config data in SoAd Socket Connection structs.
 *
 * @param SoConId 	Identifies the SoAd socket connection with info about the Tcp/Ip param values.
 * @param socketId	Which Tcp/Ip socket.
 */
static void changeParams(SoAd_SoConIdType SoConId, TcpIp_SocketIdType socketId){
    const SoAd_SocketConnectionGroupType* grp = ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef;

    uint8 TCP_NAGLE_ON = 1;

    /** @req SWS_SOAD_00689 */
    if(grp->SocketProtocol == SOAD_SOCKET_PROT_TCP){
        TcpIp_ChangeParameter(socketId, TCPIP_PARAMID_TCP_RXWND_MAX, (const uint8*)(&(grp->SocketTpRxBufferMin)) );
        if(grp->SocketProtocolTcp->SocketTcpNoDelay == TRUE){
            TcpIp_ChangeParameter(socketId, TCPIP_PARAMID_TCP_NAGLE, NULL );
        }else{
            TcpIp_ChangeParameter(socketId, TCPIP_PARAMID_TCP_NAGLE, (const uint8*)(&TCP_NAGLE_ON) );
        }
        TcpIp_ChangeParameter(socketId, TCPIP_PARAMID_TCP_KEEPALIVE,            (const uint8*)(&(grp->SocketProtocolTcp->SocketTcpKeepAlive)) );
        TcpIp_ChangeParameter(socketId, TCPIP_PARAMID_TCP_KEEPALIVE_TIME,       (const uint8*)(&(grp->SocketProtocolTcp->SocketTcpKeepAliveTime)) );
        TcpIp_ChangeParameter(socketId, TCPIP_PARAMID_TCP_KEEPALIVE_PROBES_MAX, (const uint8*)(&(grp->SocketProtocolTcp->SocketTcpKeepAliveProbesMax)) );
        TcpIp_ChangeParameter(socketId, TCPIP_PARAMID_TCP_KEEPALIVE_INTERVAL,   (const uint8*)(&(grp->SocketProtocolTcp->SocketTcpKeepAliveInterval)) );
    }
    if(grp->SocketFramePriorityUsed == TRUE){
        TcpIp_ChangeParameter(socketId, TCPIP_PARAMID_FRAMEPRIO, (const uint8*)(&(grp->SocketFramePriority)) );
    }
}

/**
 * Makes a TCP/IP Socket Address from info in an SoAd Socket Connection struct.
 *
 * @param SoConId	Identifies which SoAd Socket Connection
 * @return	The TCP/IP socket address
 */
static TcpIp_SockAddrType makeTcpIp_SockAddr(SoAd_SoConIdType SoConId){
    //IMPROVEMENT Should be able to signal an error!!!
    TcpIp_SockAddrType SockAddr = {
            .addr = {TCPIP_IPADDR_ANY, TCPIP_IPADDR_ANY, TCPIP_IPADDR_ANY, TCPIP_IPADDR_ANY},
            .port = TCPIP_PORT_ANY,
            .domain = TCPIP_AF_INET
    };

    if(ConnectionAdminList[SoConId].remAddrInUse.Set == FALSE){
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability */
        return SockAddr;
    }

    for(uint32 i=0;i<4;i++){
        SockAddr.addr[i]	= ConnectionAdminList[SoConId].remAddrInUse.SocketRemoteIpAddress[i];
    }
    SockAddr.port		= ConnectionAdminList[SoConId].remAddrInUse.SocketRemotePort;
    SockAddr.domain = TCPIP_AF_INET;

    return SockAddr;
}

/**
 * Checks if a SoAd Socket Connection has wildcards in some part of the address, for example if port==TCPIP_PORT_ANY
 * @param soCon The SoCon to test
 * @return	true if wildcard found
 */
static boolean hasWildcardsInAddress(SoAd_SoConIdType SoConId){

    boolean retVal = FALSE;
    if( (ConnectionAdminList[SoConId].remAddrInUse.Set == TRUE) &&
            ( ( (ConnectionAdminList[SoConId].remAddrInUse.SocketRemoteIpAddress[0]==TCPIP_IPADDR_ANY) &&
                (ConnectionAdminList[SoConId].remAddrInUse.SocketRemoteIpAddress[1]==TCPIP_IPADDR_ANY) &&
                (ConnectionAdminList[SoConId].remAddrInUse.SocketRemoteIpAddress[2]==TCPIP_IPADDR_ANY) &&
                (ConnectionAdminList[SoConId].remAddrInUse.SocketRemoteIpAddress[3]==TCPIP_IPADDR_ANY) ) ||
                (ConnectionAdminList[SoConId].remAddrInUse.SocketRemotePort==TCPIP_PORT_ANY)) ){
        retVal = TRUE;
    }
    return retVal;
}

/**
 * Finds which SoAd Socket Connection Group is connected with the TcpIp Socket, if any, and gives the groupIndex.
 *
 * @param SocketId	The TcpIp socket
 * @param index		Index to the Socket Connection Group which is connected to the TcpIp socket.
 * @return	true if group found, false otherwise.
 */
static boolean getGroupIndexFromSocketId(TcpIp_SocketIdType SocketId, uint32* indx){

    boolean status;
    status = FALSE;
    for(uint32 i=0;i<SOAD_NR_OF_SOCKET_CONNECTION_GROUPS;i++){
        if( ConnectionGroupAdminList[i].SocketId== SocketId){
            *indx = i;
            status = TRUE;
            break;
        }
    }
    return status;
}

/**
 * Finds which SoAd Socket Connection is connected with the TcpIp Socket, if any, and returns the index.
 * @param SocketId	The TcpIp socket
 * @param SoConId	Index to the Socket Connection which is connected to the TcpIp socket.
 * @return	true if Socket Connection found, false otherwise.
 */
static boolean getSoConIdFromSocketId(TcpIp_SocketIdType SocketId, SoAd_SoConIdType* SoConId){

    boolean status;
    status = FALSE;
    for(int i=0;i<SOAD_NR_OF_SOCKET_CONNECTIONS;i++){
        if( ConnectionAdminList[i].SocketId == SocketId){
            *SoConId = i;
            // Finds the first, there might be more.
            status = TRUE;
        }
    }
    return status;
}

/**
 * Opens a SoAd Socket Connection by opening TcpIp Socket, changing TcpIp parameters, binds the socket and calls TcpConnect() as needed.
 *
 * @param SoConId	The SoAd Socket Connection
 * @return E_OK	if successful, E_NOT_OK otherwise.
 */
static Std_ReturnType openSocket(SoAd_SoConIdType SoConId){
    const SoAd_SocketConnectionGroupType* grp = ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef;

    if(grp == NULL){
        // IMPROVEMENT Dev error
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability*/
        return E_NOT_OK;
    }

    /** @req SWS_SOAD_00590 */
    if( ((grp->SocketProtocol == SOAD_SOCKET_PROT_UDP) && (grp->NrOfSocketConnections == 1))
            || ((grp->SocketProtocol == SOAD_SOCKET_PROT_TCP) &&
                    (grp->SocketProtocolTcp->SocketTcpInitiate == TRUE)) ){
        // IMPROVEMENT Maybe also IPv6 domains?
        TcpIp_SoAdGetSocket(TCPIP_AF_INET, convertSoAdProtType(grp->SocketProtocol), &(ConnectionAdminList[SoConId].SocketId));
        changeParams(SoConId, ConnectionAdminList[SoConId].SocketId);
        TcpIp_Bind(ConnectionAdminList[SoConId].SocketId, grp->SocketLocalAddressRef, (uint16*)&(ConnectionAdminList[SoConId].groupAdminRef->localPortUsed) );
        if( grp->SocketProtocol == SOAD_SOCKET_PROT_TCP ){
            TcpIp_SockAddrType remoteAddress = makeTcpIp_SockAddr(SoConId);
            TcpIp_TcpConnect(ConnectionAdminList[SoConId].SocketId, &remoteAddress);
        }
    }
    /** @req SWS_SOAD_00638 */
    else if(((grp->SocketProtocol == SOAD_SOCKET_PROT_TCP) &&
            (grp->SocketProtocolTcp->SocketTcpInitiate == FALSE))){
        if(ConnectionAdminList[SoConId].groupAdminRef->SocketId == TCPIP_SOCKETID_INVALID){
            // IMPROVEMENT Maybe also IPv6 domains?
            TcpIp_SoAdGetSocket(TCPIP_AF_INET, convertSoAdProtType(grp->SocketProtocol), &(ConnectionAdminList[SoConId].groupAdminRef->SocketId));
            // IMPROVEMENT Assigned the tcpip group socket to the SoCon as well, is this right?
            ConnectionAdminList[SoConId].SocketId = ConnectionAdminList[SoConId].groupAdminRef->SocketId;
            changeParams(SoConId, ConnectionAdminList[SoConId].groupAdminRef->SocketId);
            TcpIp_Bind(ConnectionAdminList[SoConId].groupAdminRef->SocketId, grp->SocketLocalAddressRef, (uint16*)&(ConnectionAdminList[SoConId].groupAdminRef->localPortUsed) );
            // IMPROVEMENT 638 (e)??
            TcpIp_TcpListen(ConnectionAdminList[SoConId].groupAdminRef->SocketId, grp->NrOfSocketConnections);
        }
        else{
            //IMPROVEMENT (2)(a) Activate the socket connection to accept connections from remote nodes???
            // IMPROVEMENT This indicates "Activate the socket connection to accept..."
            ConnectionAdminList[SoConId].SocketId = ConnectionAdminList[SoConId].groupAdminRef->SocketId;
        }
    }
    /** @req SWS_SOAD_00639 */
    else if( (grp->SocketProtocol == SOAD_SOCKET_PROT_UDP) && (grp->NrOfSocketConnections > 1) ){
        if(ConnectionAdminList[SoConId].groupAdminRef->SocketId == TCPIP_SOCKETID_INVALID){
            // IMPROVEMENT Maybe also IPv6 domains?
            TcpIp_SoAdGetSocket(TCPIP_AF_INET, convertSoAdProtType(grp->SocketProtocol), &(ConnectionAdminList[SoConId].groupAdminRef->SocketId));
            // IMPROVEMENT Assigned the tcpip group socket to the SoCon as well, is this right?
            ConnectionAdminList[SoConId].SocketId = ConnectionAdminList[SoConId].groupAdminRef->SocketId;
            changeParams(SoConId, ConnectionAdminList[SoConId].groupAdminRef->SocketId);
            TcpIp_Bind(ConnectionAdminList[SoConId].groupAdminRef->SocketId, grp->SocketLocalAddressRef, (uint16*)&(ConnectionAdminList[SoConId].groupAdminRef->localPortUsed) );
        }
        else{
            //IMPROVEMENT (2)(a) Activate the socket connection for communication via the shared UDP socket of the socket connection group???
            // IMPROVEMENT This indicates "Activate the socket connection for communication..."
            ConnectionAdminList[SoConId].SocketId = ConnectionAdminList[SoConId].groupAdminRef->SocketId;
        }
    }
    else{
        //Should never end up here. The above alternatives should cover every possibility
    }

    /** @req SWS_SOAD_00591 */
    if((grp->SocketProtocol == SOAD_SOCKET_PROT_UDP) && ((grp->SocketProtocolUdp->SocketUdpListenOnly==TRUE) || (hasWildcardsInAddress(SoConId) == FALSE) )){
        changeMode(SoConId, SOAD_SOCON_ONLINE);
    }

    /** @req SWS_SOAD_00686 */
    if( (grp->SocketProtocol == SOAD_SOCKET_PROT_TCP) || (hasWildcardsInAddress(SoConId) == TRUE ) ){
        changeMode(SoConId, SOAD_SOCON_RECONNECT);
    }

    return E_OK;
}

/**
 * Closes a SoAd Socket Connection and realted TcpIp socket as needed.
 *
 * @param SoConId	The SoAd Socket Connection to close
 * @param initiatedBySoConClose	Indicates whether the closing was initiated by SoCon_Close() or not
 * @return E_OK	if successful, E_NOT_OK otherwise.
 */
static Std_ReturnType closeSocket(SoAd_SoConIdType SoConId, boolean initiatedBySoConClose){
    const SoAd_SocketConnectionGroupType* grp = ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef;
    uint32 nrOfClosedConnections = 0;

    if(grp == NULL){
        // IMPROVEMENT Dev error
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability */
        return E_NOT_OK;
    }

    /** @req SWS_SOAD_00637 */
    //(1) only applicable to TP connections
    //(2) IMPROVEMENT How to do this?

    /** @req SWS_SOAD_00642 */

    ConnectionAdminList[SoConId].closedByCloseSoCon = TRUE; // IMPROVEMENT where to set this false again?

    // IMPROVEMENT Use SoAd_GroupTypeType instead
    if( !((grp->SocketProtocol == SOAD_SOCKET_PROT_UDP) && (grp->NrOfSocketConnections == 1))  && ((grp->SocketProtocol == SOAD_SOCKET_PROT_TCP) && (grp->SocketProtocolTcp->SocketTcpInitiate == FALSE))){
        if( (ConnectionAdminList[SoConId].SocketId != TCPIP_SOCKETID_INVALID)&& (ConnectionAdminList[SoConId].SocketId == ConnectionAdminList[SoConId].groupAdminRef->SocketId)){

        }
        else{
            TcpIp_Close( ConnectionAdminList[SoConId].SocketId, (ConnectionAdminList[SoConId].abort && initiatedBySoConClose) );

        }
        for(int i=0;i<SOAD_NR_OF_SOCKET_CONNECTIONS;i++){
            if( (ConnectionAdminList[i].groupAdminRef == ConnectionAdminList[SoConId].groupAdminRef) && (ConnectionAdminList[i].closedByCloseSoCon == TRUE) ){
                nrOfClosedConnections++;
            }
        }
        if(nrOfClosedConnections == grp->NrOfSocketConnections){
            // Close listen socket as well if there is one
            TcpIp_Close( ConnectionAdminList[SoConId].groupAdminRef->SocketId, (ConnectionAdminList[SoConId].abort && initiatedBySoConClose) );
            ConnectionAdminList[SoConId].groupAdminRef->SocketId = TCPIP_SOCKETID_INVALID;
            ConnectionAdminList[SoConId].groupAdminRef->localPortUsed = ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SocketLocalPort;
        }
    }
    else{
        TcpIp_Close(ConnectionAdminList[SoConId].SocketId, (ConnectionAdminList[SoConId].abort && initiatedBySoConClose) );
        ConnectionAdminList[SoConId].groupAdminRef->localPortUsed = ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SocketLocalPort;
    }

    SchM_Enter_SoAd_EA_0();
    if( initiatedBySoConClose == TRUE ){
        changeMode(SoConId, SOAD_SOCON_OFFLINE);
    }
    else{
        changeMode(SoConId, SOAD_SOCON_RECONNECT);
    }

    ConnectionAdminList[SoConId].SocketId = TCPIP_SOCKETID_INVALID;
    SchM_Exit_SoAd_EA_0();

    return E_OK;
}

/**
 * The Message Acceptance Policy.
 *
 * @return
 */
static boolean runMessageAcceptancePolicy(void){
    //IMPROVEMENT Implement
    return TRUE;
}

/**
 * Runs the Best Match Algorithm.
 *
 * @param grp	The Socket Connection Group
 * @param RemoteAddrPtr		Remote Address
 * @param ConnIndexInGroup	Index in the Socket Connection Group for the best matching Socket Connection
 * @return true if a match is found, false otherwise.
 */
static boolean runBestMatch(const SoAd_SocketConnectionGroupType* grp, const TcpIp_SockAddrType* RemoteAddrPtr, uint32* ConnIndexInGroup){
    typedef struct{
        SoAd_SoConIdType index;
        uint8 prio;
    } BestMatchType;

    BestMatchType BestMatch = {.index =0, .prio = 0};
    const SoAd_SocketRemoteAddressType* cfgAddress;
    /** @req SWS_SOAD_00680 */
    for(SoAd_SoConIdType i=0;i<grp->NrOfSocketConnections;i++){
        cfgAddress = &(ConnectionAdminList[grp->SoAdSocketConnection[i].SocketId].remAddrInUse);

        if(cfgAddress->Set){

            /** @req SWS_SOAD_00525 */
            if( (cfgAddress->SocketRemoteIpAddress[0] == RemoteAddrPtr->addr[0]) &&
                    (cfgAddress->SocketRemoteIpAddress[1] == RemoteAddrPtr->addr[1]) &&
                    (cfgAddress->SocketRemoteIpAddress[2] == RemoteAddrPtr->addr[2]) &&
                    (cfgAddress->SocketRemoteIpAddress[3] == RemoteAddrPtr->addr[3]) &&
                    (cfgAddress->SocketRemotePort == RemoteAddrPtr->port)) {
                BestMatch.index = i;
                BestMatch.prio = 4;
            }
            else if( (cfgAddress->SocketRemoteIpAddress[0] == RemoteAddrPtr->addr[0]) &&
                    (cfgAddress->SocketRemoteIpAddress[1] == RemoteAddrPtr->addr[1]) &&
                    (cfgAddress->SocketRemoteIpAddress[2] == RemoteAddrPtr->addr[2]) &&
                    (cfgAddress->SocketRemoteIpAddress[3] == RemoteAddrPtr->addr[3])&&
                    (cfgAddress->SocketRemotePort == TCPIP_PORT_ANY) &&
                    (BestMatch.prio < 4) ){
                BestMatch.index = i;
                BestMatch.prio = 3;
            }
            else if( (cfgAddress->SocketRemotePort == RemoteAddrPtr->port) &&
                    (cfgAddress->SocketRemoteIpAddress[0] == TCPIP_IPADDR_ANY) &&
                    (cfgAddress->SocketRemoteIpAddress[1] == TCPIP_IPADDR_ANY) &&
                    (cfgAddress->SocketRemoteIpAddress[2] == TCPIP_IPADDR_ANY) &&
                    (cfgAddress->SocketRemoteIpAddress[3] == TCPIP_IPADDR_ANY) &&
                    (BestMatch.prio < 3) ){
                BestMatch.index = i;
                BestMatch.prio = 2;
            }
            else if( (cfgAddress->SocketRemoteIpAddress[0] == TCPIP_IPADDR_ANY) &&
                    (cfgAddress->SocketRemoteIpAddress[1] == TCPIP_IPADDR_ANY) &&
                    (cfgAddress->SocketRemoteIpAddress[2] == TCPIP_IPADDR_ANY) &&
                    (cfgAddress->SocketRemoteIpAddress[3] == TCPIP_IPADDR_ANY) &&
                    (cfgAddress->SocketRemotePort == TCPIP_PORT_ANY) &&
                    (BestMatch.prio < 2) ){
                BestMatch.index = i;
                BestMatch.prio = 1;
            }
            else{
                //Nothing
            }
        }
    }
    *ConnIndexInGroup = BestMatch.index;
    return (boolean)(BestMatch.prio>0);
}

/**
 * Sets the Remote Address of a Socket Connection according to the supplied TcpIp Address.
 *
 * @param SocketConnectionRef	The socket connection
 * @param RemoteAddrPtr			Remote address.
 */
static void setRemoteAddress(SoAd_SoConIdType SoConId, const TcpIp_SockAddrType* RemoteAddrPtr) {
    //IMPROVEMENT Check if IP4 or IP6?
    /** @req SWS_SOAD_00533 */
    for (uint32 i = 0; i < 4; i++) {
        ConnectionAdminList[SoConId].remAddrInUse.SocketRemoteIpAddress[i] = RemoteAddrPtr->addr[i];
    }
    ConnectionAdminList[SoConId].remAddrInUse.SocketRemotePort =
            RemoteAddrPtr->port;
    ConnectionAdminList[SoConId].remAddrInUse.Set = TRUE;
}


/**
 * Calls TcpIp_Udp/TcpTransmit() according to group belonging for the SoAd Socket Connection.
 * It can be called from other task contexts and allows for context switches
 * @param connIndex	The SoAd Socket Connection index
 * @param length	Length of data to be sent.
 * @return			E_OK if TcpIp accepted the sending.
 */
static Std_ReturnType transmit(uint16 connIndex, uint32 length, uint8 *dataPtr, uint32 pduHeaderId){
    Std_ReturnType retVal = E_OK;
#if defined(HOST_TEST)
    // Must be word aligned in Intel architecture for host test to pass
    TcpIp_SockAddrType remAddress __attribute__ ((aligned (32)));
#else
    TcpIp_SockAddrType remAddress;
#endif
    TaskType task_id = 0;

    // Since we cannot disable interrupts in TcpIp calls, task_id is used as an array index to protect
    // SocketCurrentTx from possible task switches occurring between assignment of SocketCurrentTx and the transmission.
    // TcpIp_UdpTransmit uses callback SoAd_CopyTxData where the right version of SocketCurrentTx will be used
    // even if a context switch has occurred.
    retVal = GetTaskID(&task_id);
    if ((retVal == E_OK) && (task_id < OS_TASK_CNT)){
		SchM_Enter_SoAd_EA_0();
		SocketCurrentTx[task_id].SoConId = connIndex;
		SocketCurrentTx[task_id].TxBufCopy = dataPtr;
		SocketCurrentTx[task_id].TxPduHeaderId = pduHeaderId;
		SchM_Exit_SoAd_EA_0();

		/** @req SWS_SOAD_00540 */
		/** @req SWS_SOAD_00542 */
		switch ( ConnectionAdminList[connIndex].groupAdminRef->SocketConnectionGroupRef->GroupType ) {
		case SOAD_GROUPTYPE_LONELY_UDP:
			remAddress = makeTcpIp_SockAddr(connIndex);
			retVal = TcpIp_UdpTransmit(ConnectionAdminList[connIndex].SocketId, NULL, &remAddress, length);
			break;
		case SOAD_GROUPTYPE_MULTI_UDP:
			remAddress = makeTcpIp_SockAddr(connIndex);
			retVal = TcpIp_UdpTransmit(ConnectionAdminList[connIndex].groupAdminRef->SocketId, NULL, &remAddress, length);
			break;
		case SOAD_GROUPTYPE_LISTEN_TCP:// Intentional fall through
		case SOAD_GROUPTYPE_INIT_TCP:
			retVal = TcpIp_TcpTransmit(ConnectionAdminList[connIndex].SocketId, NULL, length, TRUE);
			break;
		default:
			break;
		}
    }
    return retVal;
}

#if SOAD_NR_OF_PDU_ROUTING_GROUP_CONNECTIONS > 0
/**
 * Finds if Pdu Route Dest pduRouteDest should be used to send Pdus. If either
 * it has no routing groups or if at least one of its routing groups is active
 * for the supplied Socket Connection id it should be ok.
 *
 * @param pduRouteDest	Pdu Route Dest to check
 * @param connIndex		Socket Connection id
 * @return				True if it's clear to send
 */
static Std_ReturnType findActiveRoutingGroup(const SoAd_PduRouteDestType* pduRouteDest, uint16 connIndex ){
    Std_ReturnType retVal = E_NOT_OK;
    /** @req SWS_SOAD_00560 */
    // Find if there is at least one routing group active for this PduRouteDest
    if(pduRouteDest->NrOfRoutingGroups == 0){
        retVal = E_OK;
    }
    else{
        /*lint -save -e845 CONFIGURATION */
        for(uint32 k=0;k<SOAD_NR_OF_PDU_ROUTING_GROUP_CONNECTIONS;k++){
            if(pduRouteDest == PduRoutingGroupAdminList[k].PduRouteDest ){
                if( (PduRoutingGroupAdminList[k].Active == TRUE) && (PduRoutingGroupAdminList[k].SoConId == connIndex) ){
                    retVal = E_OK;
                    break;
                }
            }
        }
        /*lint -restore */
    }
    return retVal;
}
#endif

/**
 * Copies Pdu data pointed to by SoAdSrcPduInfoPtr for the Socket Connection
 * with id connIndex into its nPduUdpTxBuffer taking into account if Pdu Headers
 * should be used or not.
 *
 * @param connIndex			Indicates the Socket Connection
 * @param SoAdSrcPduInfoPtr	Pointer to the Pdu to be copied
 * @param useHeader			True if PDU header is to be copied as well
 * @param headerId			What headerId to copy
 */
static void copyPduToBuffer(uint16 connIndex, const PduInfoType* SoAdSrcPduInfoPtr, boolean useHeader, uint32 headerId){
    uint32 bufIndex= ConnectionAdminList[connIndex].bytesInPduUdpBuffer;

    SchM_Enter_SoAd_EA_0();
    if(useHeader==TRUE){
        /** @req SWS_SOAD_00197 */
        /** @req SWS_SOAD_00198 */
        ConnectionAdminList[connIndex].SocketConnectionRef->nPduUdpTxBuffer[bufIndex++] = (uint8)((headerId & 0xFF000000u)>>24);
        ConnectionAdminList[connIndex].SocketConnectionRef->nPduUdpTxBuffer[bufIndex++] = (uint8)((headerId & 0x00FF0000u)>>16);
        ConnectionAdminList[connIndex].SocketConnectionRef->nPduUdpTxBuffer[bufIndex++] = (uint8)((headerId & 0x0000FF00u)>>8);
        ConnectionAdminList[connIndex].SocketConnectionRef->nPduUdpTxBuffer[bufIndex++] = (uint8) (headerId & 0x000000FFu);
        ConnectionAdminList[connIndex].SocketConnectionRef->nPduUdpTxBuffer[bufIndex++] = 0;
        ConnectionAdminList[connIndex].SocketConnectionRef->nPduUdpTxBuffer[bufIndex++] = 0;
        ConnectionAdminList[connIndex].SocketConnectionRef->nPduUdpTxBuffer[bufIndex++] = (uint8)((SoAdSrcPduInfoPtr->SduLength & 0x0000FF00u)>>8);
        ConnectionAdminList[connIndex].SocketConnectionRef->nPduUdpTxBuffer[bufIndex++] = (uint8) (SoAdSrcPduInfoPtr->SduLength & 0x000000FFu);
        ConnectionAdminList[connIndex].bytesInPduUdpBuffer = bufIndex;
    }
    uint32 i=0;
    for(;bufIndex<ConnectionAdminList[connIndex].bytesInPduUdpBuffer+SoAdSrcPduInfoPtr->SduLength;bufIndex++){
        ConnectionAdminList[connIndex].SocketConnectionRef->nPduUdpTxBuffer[bufIndex] = SoAdSrcPduInfoPtr->SduDataPtr[i++];
    }
    ConnectionAdminList[connIndex].bytesInPduUdpBuffer = bufIndex;
	SchM_Exit_SoAd_EA_0();
}

/**
 * Sets the Tx Udp Trigger Timeout for the Socket Connection connIndex using
 * config data from pduRouteDest. This timer controls the Udp Tx Buffer
 * sending.
 *
 * @param connIndex		Indicates which Socket Connection
 * @param pduRouteDest	Here the timeout values are kept.
 */
static void setBufTimer(uint16 connIndex, const SoAd_PduRouteDestType* pduRouteDest){
    if(ConnectionAdminList[connIndex].udpBufTimer==0){	// Timer not running
        /** @req SWS_SOAD_00697 */
        if( pduRouteDest->TxUdpTriggerTimeout > 0 ){
        	SchM_Enter_SoAd_EA_0();
            ConnectionAdminList[connIndex].udpBufTimer = pduRouteDest->TxUdpTriggerTimeout;
        	SchM_Exit_SoAd_EA_0();
        } /** @req SWS_SOAD_00683 */
        else if( ConnectionAdminList[connIndex].groupAdminRef->SocketConnectionGroupRef->SocketProtocolUdp->SocketUdpTriggerTimeout > 0 ){
        	SchM_Enter_SoAd_EA_0();
            ConnectionAdminList[connIndex].udpBufTimer = ConnectionAdminList[connIndex].groupAdminRef->SocketConnectionGroupRef->SocketProtocolUdp->SocketUdpTriggerTimeout;
        	SchM_Exit_SoAd_EA_0();
        }
        else{

        }
    }else{// Timer is already running
        /** @req SWS_SOAD_00697 */
        if( (pduRouteDest->TxUdpTriggerTimeout > 0) && (ConnectionAdminList[connIndex].udpBufTimer > pduRouteDest->TxUdpTriggerTimeout)){
        	SchM_Enter_SoAd_EA_0();
            ConnectionAdminList[connIndex].udpBufTimer = pduRouteDest->TxUdpTriggerTimeout;
        	SchM_Exit_SoAd_EA_0();
        } /** @req SWS_SOAD_00683 */
        else if( (pduRouteDest->TxUdpTriggerTimeout == 0) &&
                (ConnectionAdminList[connIndex].groupAdminRef->SocketConnectionGroupRef->SocketProtocolUdp->SocketUdpTriggerTimeout > 0) &&
                (ConnectionAdminList[connIndex].udpBufTimer > ConnectionAdminList[connIndex].groupAdminRef->SocketConnectionGroupRef->SocketProtocolUdp->SocketUdpTriggerTimeout) ){
        	SchM_Enter_SoAd_EA_0();
            ConnectionAdminList[connIndex].udpBufTimer = ConnectionAdminList[connIndex].groupAdminRef->SocketConnectionGroupRef->SocketProtocolUdp->SocketUdpTriggerTimeout;
        	SchM_Exit_SoAd_EA_0();
        }
        else{

        }
    }
}

/**
 * If routing groups allow Pdu is either sent using transmit() subfunction or
 * copied into buffer for later transmission.
 *
 * @param connIndex				SoAd Socket Connection
 * @param SoAdSrcPduId			To keep track of unconfirmed TCP messages.
 * @param pduRouteDest			The Pdu Route Dest to match against connIndex
 * @param SoAdSrcPduInfoPtr		The Pdu to send.
 * @param atLeastOneDestSent	Outgoing parameter that indicates if at least one transmission was done.
 * @return	The return value of transmit() subfunction
 */
static Std_ReturnType ifTransmitSubFunction(uint16 connIndex, PduIdType SoAdSrcPduId, const SoAd_PduRouteDestType* pduRouteDest, const PduInfoType* SoAdSrcPduInfoPtr, boolean* atLeastOneDestSent){
    Std_ReturnType retVal = E_OK;
    boolean activeRoutingFound;
#if SOAD_NR_OF_PDU_ROUTING_GROUP_CONNECTIONS > 0
    activeRoutingFound=FALSE;
    if( findActiveRoutingGroup(pduRouteDest, connIndex )==E_OK){
        activeRoutingFound = TRUE;
        *atLeastOneDestSent = TRUE;
    }
#else
    activeRoutingFound=TRUE;
    *atLeastOneDestSent = TRUE;
#endif
    if(activeRoutingFound==TRUE){
        /* @req SWS_SOAD_00690 */
        /* @req SWS_SOAD_00691 */
        if(ConnectionAdminList[connIndex].SocketConnectionRef->nPduUdpTxBufferSize>0){
            uint32 bufferSpace = ConnectionAdminList[connIndex].SocketConnectionRef->nPduUdpTxBufferSize - ConnectionAdminList[connIndex].bytesInPduUdpBuffer;
            if(pduRouteDest->TxUdpTriggerMode == SOAD_TRIGGER_NEVER){
                if( bufferSpace >= SoAdSrcPduInfoPtr->SduLength){
                    /** @req SWS_SOAD_00547 */
                    copyPduToBuffer(connIndex, SoAdSrcPduInfoPtr, ConnectionAdminList[connIndex].groupAdminRef->SocketConnectionGroupRef->PduHeaderEnable, pduRouteDest->TxPduHeaderId);
                }
                else{
                    /** @req SWS_SOAD_00549 */
                    retVal = transmit(connIndex, ConnectionAdminList[connIndex].bytesInPduUdpBuffer,ConnectionAdminList[connIndex].SocketConnectionRef->nPduUdpTxBuffer,0);
                    ConnectionAdminList[connIndex].bytesInPduUdpBuffer=0;
                    copyPduToBuffer(connIndex, SoAdSrcPduInfoPtr, ConnectionAdminList[connIndex].groupAdminRef->SocketConnectionGroupRef->PduHeaderEnable, pduRouteDest->TxPduHeaderId);
                }
                if(retVal == E_OK){
                    setBufTimer(connIndex, pduRouteDest);
                }
            }
            else{
                if( bufferSpace < SoAdSrcPduInfoPtr->SduLength){
                    /** @req SWS_SOAD_00685 */
                    retVal = transmit(connIndex, ConnectionAdminList[connIndex].bytesInPduUdpBuffer,ConnectionAdminList[connIndex].SocketConnectionRef->nPduUdpTxBuffer,0);
                    ConnectionAdminList[connIndex].bytesInPduUdpBuffer=0;
                }
                /** @req SWS_SOAD_00548 */
                copyPduToBuffer(connIndex, SoAdSrcPduInfoPtr, ConnectionAdminList[connIndex].groupAdminRef->SocketConnectionGroupRef->PduHeaderEnable, pduRouteDest->TxPduHeaderId);
                retVal = transmit(connIndex, ConnectionAdminList[connIndex].bytesInPduUdpBuffer,ConnectionAdminList[connIndex].SocketConnectionRef->nPduUdpTxBuffer,0);
                ConnectionAdminList[connIndex].bytesInPduUdpBuffer=0;
                /** @req SWS_SOAD_00684 */
                ConnectionAdminList[connIndex].udpBufTimer=0;
            }
        }
        else{
            PduRouteAdminList[SoAdSrcPduId].unconfirmedMessages++;
            if(ConnectionAdminList[connIndex].groupAdminRef->SocketConnectionGroupRef->PduHeaderEnable==TRUE){
                retVal = transmit(connIndex, (SoAdSrcPduInfoPtr->SduLength+SOAD_PDU_HEADER_LENGTH),SoAdSrcPduInfoPtr->SduDataPtr, pduRouteDest->TxPduHeaderId);
            }
            else{
                retVal = transmit(connIndex, SoAdSrcPduInfoPtr->SduLength,SoAdSrcPduInfoPtr->SduDataPtr, pduRouteDest->TxPduHeaderId);
            }
            // IMPROVEMENT Must check that we only do this for TCP groups!
            if(retVal != E_OK){
                PduRouteAdminList[SoAdSrcPduId].unconfirmedMessages--;	// This is only for TCP not UDP
            }
        }
    }
    return retVal;
}

/**
 * @brief Function to start a Tp transmit function by invoking TcpIp_TcpTransmit
 * @param SoAdSrcPduId PDU used for transmission
 * @return
 */
static void tpTransmitSubFunction(PduIdType SoAdSrcPduId){

    const SoAd_PduRouteType* pduRoute;
    SoadTptransmitType *tpRunTimeParm;
    PduInfoType pduInfo;
    uint16 socketIndex;
    PduLengthType totalLength;
    totalLength = 0u;
    BufReq_ReturnType bufAvRetVal;
    bufAvRetVal = BUFREQ_OK;
    Std_ReturnType retVal;
    TcpIp_SockAddrType remAddress;
    SoAd_SoConIdType SoConId;

    pduRoute = &(SoAdCfgPtr->PduRoute[SoAdSrcPduId]);
    tpRunTimeParm = &SoadTpTransStatus[SoAdSrcPduId];
    pduInfo.SduDataPtr = &retVal; //Set a valid address. Not used by UL.
    pduInfo.SduLength = 0;
    retVal = E_OK;

    bufAvRetVal = pduRoute->SoAdTpCopyTxData(pduRoute->TxPduRef, &pduInfo, NULL, &totalLength);

    /** @req SWS_SoAd_00552 *//* Check what is the total length in bytes to be transmitted */
    if (FALSE == tpRunTimeParm->firstCopyCalled){
        if ((pduRoute->PduRouteDest[SOAD_PDU_ROUTE_FIRST_DESTINATION_IDX].TxPduHeaderId) != 0) {
            /* @req SWS_SoAd_00197 */
            totalLength += SOAD_PDU_HEADER_LENGTH;
        }

        tpRunTimeParm->totalLen = totalLength;
    }

    if(bufAvRetVal == BUFREQ_OK){

        socketIndex = pduRoute->PduRouteDest[SOAD_PDU_ROUTE_FIRST_DESTINATION_IDX].Index;
        tpRunTimeParm->socketId = ConnectionAdminList[socketIndex].SocketId; /* Save socket Id needed for later parsing */

        if(ConnectionAdminList[socketIndex].groupAdminRef-> \
                SocketConnectionGroupRef->SocketProtocol == SOAD_SOCKET_PROT_UDP){

            PduRouteAdminList[SoAdSrcPduId].unconfirmedMessages++;
            /** @req SWS_SoAd_00553 */
            (void)getSoConIdFromSocketId(ConnectionAdminList[socketIndex].SocketId, &SoConId);
            remAddress = makeTcpIp_SockAddr(SoConId);
            retVal = TcpIp_UdpTransmit(tpRunTimeParm->socketId, NULL, &remAddress, totalLength);
        }
        else if (ConnectionAdminList[socketIndex].groupAdminRef-> \
                SocketConnectionGroupRef->SocketProtocol == SOAD_SOCKET_PROT_TCP){
            PduRouteAdminList[SoAdSrcPduId].unconfirmedMessages++;
            /** @req SWS_SoAd_00554 */
            retVal = TcpIp_TcpTransmit(tpRunTimeParm->socketId, NULL, totalLength, FALSE);
        }else{
            /*To avoid lint error*/
        }
    } else {
        retVal = E_NOT_OK;
    }

    if (E_NOT_OK == retVal){
        /* Something went wrong in upper/lower layer. Discard transmission */
        PduRouteAdminList[SoAdSrcPduId].unconfirmedMessages--;
        pduRoute->TpTxConfirmationFunction(pduRoute->TxPduRef, E_NOT_OK);
        tpRunTimeParm->tpTxRequest = FALSE;
    }
}

/**
 * Returns the Pdu Header (headerId) and Pdu Length (PduLength) from data
 * in the buffer pointed to by BufPtr. It is assumed that the buffer data
 * starts with the Header at the first byte.
 *
 * @param BufPtr	Pointer to data buffer where HeaderId and PduLength are.
 * @param Length	Length of buffer.
 * @param headerId	The returned Header Id. The first 4 bytes are
 * 					converted into an uint32 big endian.
 * @param PduLength	The returned Pdu Length. The first 4 bytes are
 * 					converted into an uint32 big endian.
 * @return			True as long as Length>SOAD_PDU_HEADER_LENGTH
 */
static boolean getPduHeader(const uint8* BufPtr, uint32 Length, uint32* headerId, uint32* PduLength){
    if(Length<SOAD_PDU_HEADER_LENGTH){
        return FALSE;
    }
    *headerId =  0x00000000u;
    *headerId =  *headerId | (BufPtr[0]<<24);
    *headerId =	 *headerId | (BufPtr[1]<<16);
    *headerId =	 *headerId | (BufPtr[2]<<8);
    *headerId =	 *headerId | (BufPtr[3]);

    *PduLength = 0x00000000u;
    *PduLength = *PduLength | (BufPtr[4]<<24);
    *PduLength = *PduLength | (BufPtr[5]<<16);
    *PduLength = *PduLength | (BufPtr[6]<<8);
    *PduLength = *PduLength | (BufPtr[7]);

    return TRUE;
}

/**
 * If Socket Connection with id SoConId is referenced from SocektRoute function
 * returns TRUE.
 *
 * @param SoConId		Identifies the SoAd socket connection.
 * @param SocketRoute	Socket Route
 * @return				True if SocektRoute references Socket Connection with
 * 						id SoConId.
 */
static boolean soConInSocketRoute(uint16 SoConId, const SoAd_SocketRouteType* SocketRoute){
    boolean retVal = FALSE;
    if(SocketRoute->GroupUsed==FALSE){
        if(SocketRoute->Index == SoConId){
            retVal = TRUE;
        }
    }
    else{
        for(uint32 i=0;i<ConnectionGroupAdminList[SocketRoute->Index].SocketConnectionGroupRef->NrOfSocketConnections;i++){
            if(ConnectionGroupAdminList[SocketRoute->Index].SocketConnectionGroupRef->SoAdSocketConnection[i].SocketId == SoConId){
                retVal = TRUE;
                break;
            }
        }
    }
    return retVal;
}

/**
 * If SoConId is referenced from SocketRoute and routing groups allow,
 * RxIndication() is called with RxPduId and rxPdu.
 *
 * @param SoConId		Identifies the SoAd socket connection.
 * @param rxPDU			The Pdu payload to send to upper layer.
 * @return				True if upper layer was called.
 */
static boolean routingReceptionNoHeader(uint16 SoConId, PduInfoType* rxPDU){
    const SoAd_SocketRouteDestType*  socketDest;
    uint32 j=0;
    uint32 SocketRoute;
    PduLengthType bufferSizePtr;
    BufReq_ReturnType ret;
    boolean retVal = FALSE;

    for(SocketRoute=0;SocketRoute<SOAD_NR_OF_SOCKET_ROUTES;SocketRoute++){

        if(soConInSocketRoute(SoConId, &(SoAdCfgPtr->SocketRoute[SocketRoute])) == TRUE){
            socketDest = SoAdCfgPtr->SocketRoute[SocketRoute].SocketRouteDest;

            for(j=0;j<SOAD_NR_OF_SOCKET_ROUTING_GROUP_CONNECTIONS;j++){
                if( (socketDest->NrOfRoutingGroups==0) || ((socketDest == SocketRoutingGroupAdminList[j].SocketRouteDest) &&
                        (SocketRoutingGroupAdminList[j].SoConId == SoConId) &&
                        (SocketRoutingGroupAdminList[j].Active == TRUE))){

                    if (SOAD_RX_UPPER_LAYER_TP == socketDest->RxUpperLayerType ){
                        if(ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SocketProtocol ==SOAD_SOCKET_PROT_UDP){
                            ret = socketDest->SoAdTpStartofReception(socketDest->RxPduRef ,\
                                         rxPDU,rxPDU->SduLength, &bufferSizePtr);
                            if(ret == BUFREQ_OK){
                                ret = socketDest->SoAdTpCopyRxData(socketDest->RxPduRef,rxPDU,&bufferSizePtr);

                                if(ret == BUFREQ_NOT_OK){
                                    socketDest->TpRxIndicationFunction(socketDest->RxPduRef, E_NOT_OK);
                                }else {
                                    socketDest->TpRxIndicationFunction(socketDest->RxPduRef, E_OK);
                                    retVal = TRUE;
                                    break;
                                }
                            }
                        }else {
                            if (FALSE == SoAdTpReceiveStatus[SoConId].tpRxRequest){
                                ret = socketDest->SoAdTpStartofReception(socketDest->RxPduRef ,\
                                     rxPDU,rxPDU->SduLength, &bufferSizePtr);
                                SoAdTpReceiveStatus[SoConId].tpRxRequest = TRUE;

                                if(ret == BUFREQ_OK){
                                    ret = socketDest->SoAdTpCopyRxData(socketDest->RxPduRef,rxPDU,&bufferSizePtr);
                                    if(ret == BUFREQ_NOT_OK){
                                        socketDest->TpRxIndicationFunction(socketDest->RxPduRef, E_NOT_OK);
                                        SoAdTpReceiveStatus[SoConId].tpRxRequest = FALSE;
                                        break;
                                    }else{
                                        retVal = TRUE;
                                        break;
                                    }
                                }
                            }else {
                                ret = socketDest->SoAdTpCopyRxData(socketDest->RxPduRef,rxPDU,&bufferSizePtr);
                                if(ret == BUFREQ_NOT_OK){
                                    socketDest->TpRxIndicationFunction(socketDest->RxPduRef, E_NOT_OK);
                                    SoAdTpReceiveStatus[SoConId].tpRxRequest = FALSE;
                                    break;
                                }else{
                                    retVal = TRUE;
                                    break;
                                }
                            }
                        }
                    }else {
                        SoAdCfgPtr->SocketRoute[SocketRoute].SocketRouteDest->RxIndicationFunction( socketDest->RxPduRef ,(PduInfoType*)rxPDU);
                        retVal = TRUE;
                        break;
                    }
                }
            }
        }
        if (retVal == TRUE){
            break;
        }
    }
    return retVal;
}
/**
 * This function is called from SoAd_RxIndication().
 * Enables Reception through TP-TCP communication
 *
 *
 * @param SocketId     The TcpIp socket
 * @param SoConId      Identifies the SoAd socket connection.
 * @param BufPtr       Pointer to the received data
 * @param Length       Data length of the received TCP segment
 */
static void processTpTcpRxCommunication (TcpIp_SocketIdType SocketId, SoAd_SoConIdType SoConId, uint8* BufPtr, uint16 Length) {

    const SoAd_SocketRouteDestType* socketDest;
    uint32 headerId = 0u;
    uint32 pduLength = 0u;
    /*lint -e578*/
    PduInfoType rxPDU;
    PduLengthType availablBufSize;
    PduLengthType lenmin;
    boolean tcpSegmented;
    BufReq_ReturnType retValCopydata;
    if (FALSE == SoAdTpReceiveStatus[SoConId].tpRxRequest){
        //extract header
        if (getPduHeader(BufPtr, Length, &headerId, &pduLength) == TRUE) {
            for(uint32 i=0;i<SOAD_NR_OF_SOCKET_ROUTES;i++){
                if( (SoAdCfgPtr->SocketRoute[i].RxPduHeaderId == headerId) && (soConInSocketRoute(SoConId, &(SoAdCfgPtr->SocketRoute[i])) == TRUE )){
                    socketDest = SoAdCfgPtr->SocketRoute[i].SocketRouteDest;
                    /** @req SWS_SOAD_00600 */
                    for(uint32 j=0;j<SOAD_NR_OF_SOCKET_ROUTING_GROUP_CONNECTIONS;j++){
                        if((socketDest->NrOfRoutingGroups==0) || (SocketRoutingGroupAdminList[j].SocketRouteDest==socketDest &&
                                SocketRoutingGroupAdminList[j].Active == TRUE)){
                            tcpSegmented = Length < (pduLength  + SOAD_PDU_HEADER_LENGTH);
                            lenmin =  (tcpSegmented == TRUE)? (Length-SOAD_PDU_HEADER_LENGTH): pduLength;

                            rxPDU.SduDataPtr = BufPtr;
                            rxPDU.SduLength = lenmin;

                            retValCopydata = socketDest->SoAdTpStartofReception(socketDest->RxPduRef ,\
                                   &rxPDU, pduLength,&availablBufSize); //Call start of reception
                            //It is assumed that upper layer will reject if it does not have a buffer to hold pduLength
                            if (BUFREQ_OK == retValCopydata) {

                                retValCopydata = socketDest->SoAdTpCopyRxData(socketDest->RxPduRef ,\
                                       &rxPDU, &availablBufSize);

                                if (tcpSegmented == FALSE) {
                                    if (BUFREQ_OK == retValCopydata) {
                                        socketDest->TpRxIndicationFunction(socketDest->RxPduRef, E_OK);
                                        SoAdTpReceiveStatus[SoConId].tpRxRequest = FALSE;
                                    } else {
                                        //Since copying failed we terminate with the indication /* @req SWS_SoAd_00573 */
                                        socketDest->TpRxIndicationFunction(socketDest->RxPduRef, E_NOT_OK);
                                        SoAdTpReceiveStatus[SoConId].tpRxRequest = FALSE;
                                    }
                                } else {
                                    if (BUFREQ_OK == retValCopydata) {
                                        SoAdTpReceiveStatus[SoConId].tpRxRequest = TRUE;
                                        SoAdTpReceiveStatus[SoConId].remaingLen = pduLength -lenmin;
                                        SoAdTpReceiveStatus[SoConId].socketRouteIndex = i;
                                        SoAdTpReceiveStatus[SoConId].firstCopyCalled = TRUE;
                                    } else {
                                        //Since copying failed we terminate with the indication /* @req SWS_SoAd_00573 */
                                        socketDest->TpRxIndicationFunction(socketDest->RxPduRef, E_NOT_OK);
                                        SoAdTpReceiveStatus[SoConId].tpRxRequest = FALSE;
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
            }

        }else {
            /*lint -e{904} PERFORMANCE, Return statement is necessary to avoid multiple if loops (less cyclomatic complexity) and hence increase readability */
            return;
        }

    } else {
        socketDest = SoAdCfgPtr->SocketRoute[SoAdTpReceiveStatus[SoConId].socketRouteIndex].SocketRouteDest;
        lenmin =  Length > SoAdTpReceiveStatus[SoConId].remaingLen ? SoAdTpReceiveStatus[SoConId].remaingLen : Length;
        rxPDU.SduDataPtr = BufPtr;
        rxPDU.SduLength = lenmin;

        retValCopydata = socketDest->SoAdTpCopyRxData(socketDest->RxPduRef ,\
                &rxPDU, &availablBufSize);
        if (BUFREQ_OK == retValCopydata) {
            SoAdTpReceiveStatus[SoConId].remaingLen -= lenmin;
            if (SoAdTpReceiveStatus[SoConId].remaingLen == 0){
                SoAdTpReceiveStatus[SoConId].tpRxRequest = FALSE;
                socketDest->TpRxIndicationFunction(socketDest->RxPduRef, E_OK);
            }
        } else {
            SoAdTpReceiveStatus[SoConId].tpRxRequest = FALSE;
            SoAdTpReceiveStatus[SoConId].firstCopyCalled = FALSE;
            //Since copying failed we terminate with the indication /* @req SWS_SoAd_00573 */
            socketDest->TpRxIndicationFunction(socketDest->RxPduRef, E_NOT_OK);
        }
    }
}

/**
 * Check which SocketRoute matches the HeaderId and calls the corresponding
 * RxIndication() function with the PDU in pduBuf with length pduLength.
 * It also checks that the routing groups connected to this SocketRouteDest
 * allows this.
 * @param headerId	The PDU Header Id of the received PDU.
 * @param pduBuf	Pointer to the buffer with PDU data.
 * @param pduLength	Length of the PDU data as indicated by the
 * 					PDU Header length field.
 * @return			True if something is sent on a SocketRoute.
 */
static boolean routingReceptionPduHeader(uint16 SoConId, uint32 headerId, uint8* pduBuf, PduLengthType pduLength){
    const SoAd_SocketRouteDestType*  socketDest;
    PduInfoType rxPDU;
    PduLengthType availablBufSize;
    BufReq_ReturnType retValCopydata;
    boolean retVal = FALSE;

    for(uint32 i=0;i<SOAD_NR_OF_SOCKET_ROUTES;i++){
        if( (SoAdCfgPtr->SocketRoute[i].RxPduHeaderId == headerId) && (soConInSocketRoute(SoConId, &(SoAdCfgPtr->SocketRoute[i])) == TRUE )){
            socketDest = SoAdCfgPtr->SocketRoute[i].SocketRouteDest;
            /** @req SWS_SOAD_00600 */
            for(uint32 j=0;j<SOAD_NR_OF_SOCKET_ROUTING_GROUP_CONNECTIONS;j++){
                if((socketDest->NrOfRoutingGroups==0) || (SocketRoutingGroupAdminList[j].SocketRouteDest==socketDest &&
                        SocketRoutingGroupAdminList[j].Active == TRUE)){
                    rxPDU.SduDataPtr = pduBuf;
                    rxPDU.SduLength = pduLength;
                    if (SOAD_RX_UPPER_LAYER_TP == socketDest->RxUpperLayerType ) {
                        //Check if it is a Tp reception

                        retValCopydata = socketDest->SoAdTpStartofReception(socketDest->RxPduRef ,\
                               &rxPDU, pduLength,&availablBufSize); //Call start of reception
                        //It is assumed that upper layer will reject if it does not have a buffer to hold pduLength
                        if (BUFREQ_OK == retValCopydata) {
                           /* In UDP communication we cannot get fragmented messages for the same PDU header
                            * on different SoAd_RxIndication(). This is because UDP does not ascertain delivery
                            * in order they have been transmitted.
                            * i.e A SoAD PDU header indicating 600B cannot be split into two UDP messages
                            * 1. First meesage =  PDU Header + 300 B
                            * 2. Second message = 300B (remaining bytes)
                            * We can thus further deduce that this results in only one call <Up>_[SoAd][Tp]CopyTxData()
                            */
                           retValCopydata = socketDest->SoAdTpCopyRxData(socketDest->RxPduRef ,\
                                   &rxPDU, &availablBufSize);

                           if (BUFREQ_OK == retValCopydata) {
                               socketDest->TpRxIndicationFunction(socketDest->RxPduRef, E_OK);
                           } else {
                               //Since copying failed we terminate with the indication /* @req SWS_SoAd_00573 */
                               socketDest->TpRxIndicationFunction(socketDest->RxPduRef, E_NOT_OK);
                           }
                           retVal = TRUE;
                           break;
                        }// @req SWS_SoAd_00572


                    } else {
                        socketDest->RxIndicationFunction( socketDest->RxPduRef , &rxPDU);
                    }

                    retVal = TRUE;
                    break;
                }
            }
        }
        if (retVal == TRUE) {
            break;
        }
    }
    return retVal;
}

/**
 * Resets the Remote address from config for the Socket Connection indicated by SoConId
 *
 * @param SoConId	Identifies the SoAd socket connection.
 */
static void setRemoteAddressFromCfg(SoAd_SoConIdType SoConId){
    ConnectionAdminList[SoConId].remAddrInUse.Set = ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SoAdSocketConnection[ConnectionAdminList[SoConId].SocketConnectionRef->IndexInGroup].SoAdSocketRemoteAddress->Set;
    for(uint32 j=0;j<4;j++){
        ConnectionAdminList[SoConId].remAddrInUse.SocketRemoteIpAddress[j] = ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SoAdSocketConnection[ConnectionAdminList[SoConId].SocketConnectionRef->IndexInGroup].SoAdSocketRemoteAddress->SocketRemoteIpAddress[j];
    }
    ConnectionAdminList[SoConId].remAddrInUse.SocketRemotePort = ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SoAdSocketConnection[ConnectionAdminList[SoConId].SocketConnectionRef->IndexInGroup].SoAdSocketRemoteAddress->SocketRemotePort;
}

/**
 * Changes the SoConModeType for the Socket Connection with id SoConId and
 * calls the upper layer notofication function if configured.
 *
 * @param SoConId	Identifies the SoAd socket connection.
 * @param mode		The mode to change to.
 */
static void changeMode( SoAd_SoConIdType SoConId, SoAd_SoConModeType mode){
    ConnectionAdminList[SoConId].mode = mode;
    if(ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SocketSoConModeChgNotification == TRUE){
        /** @req SWS_SOAD_00597 */
        ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SocketSoConModeChgNotificationFunction(SoConId, mode);
    }
}

/**
 * Updates the timer and checks if it has expired. Uses the main function period in milliseconds to update the timer.
 *
 * @param timer	Pointer to uint32 downcounting timer.
 * @return	true if the timer expired this cycle. False otherwise.
 */
static boolean timerExpiredEvent(uint32* timer){
    boolean retVal = FALSE;

    if( *timer > 0 ){
        if( *timer > SOAD_MAIN_FUNCTION_PERIOD_MS ){
            *timer = *timer - SOAD_MAIN_FUNCTION_PERIOD_MS;
        }else{
            *timer = 0;
            retVal = TRUE;
        }
    }

    return retVal;
}

/* ----------------------------[public functions]----------------------------*/

void SoAd_Init( const SoAd_ConfigType* SoAdConfigPtr ){
    uint32 soCon=0;
    uint32 soConGroup=0;
    uint32 soConInGroup=0;
    uint32 rtGrpConnIdx=0;
    uint32 pduCnt=0;
    uint32 socketCnt=0;

    /** @req SWS_SOAD_00216 */
    VALIDATE_NO_RV(SoAdConfigPtr!=NULL, SOAD_INIT_ID, SOAD_E_INIT_FAILED)
    VALIDATE_NO_RV(SoAdConfigPtr->PduRoute!=NULL, SOAD_INIT_ID, SOAD_E_INIT_FAILED)
    //IMPROVEMENT Check rest of config, how much do we have to check?

    SchM_Enter_SoAd_EA_0();
    /** @req SWS_SOAD_00211 */
    SoAdCfgPtr = SoAdConfigPtr;

    /** @req SWS_SOAD_00723 */
    for(soConGroup=0;soConGroup<SoAdConfigPtr->NrOfSocketConnectionGroups;soConGroup++){
        ConnectionGroupAdminList[soConGroup].SocketId = TCPIP_SOCKETID_INVALID;
        ConnectionGroupAdminList[soConGroup].SocketConnectionGroupRef = &(SoAdConfigPtr->SocketConnectionGroup[soConGroup]);
        ConnectionGroupAdminList[soConGroup].mode = SOAD_SOCON_OFFLINE;
        ConnectionGroupAdminList[soConGroup].IpAddrState = TCPIP_IPADDR_STATE_UNASSIGNED;
        ConnectionGroupAdminList[soConGroup].localPortUsed = SoAdConfigPtr->SocketConnectionGroup[soConGroup].SocketLocalPort;
        for(soConInGroup=0;soConInGroup<SoAdConfigPtr->SocketConnectionGroup[soConGroup].NrOfSocketConnections;soConInGroup++){
            ConnectionAdminList[soCon].SocketId = TCPIP_SOCKETID_INVALID;
            ConnectionAdminList[soCon].SocketConnectionRef = &(SoAdConfigPtr->SocketConnectionGroup[soConGroup].SoAdSocketConnection[soConInGroup]);
            ConnectionAdminList[soCon].groupAdminRef = &(ConnectionGroupAdminList[soConGroup]);
            ConnectionAdminList[soCon].opened = FALSE;
            ConnectionAdminList[soCon].openRequest = FALSE;
            ConnectionAdminList[soCon].closeRequest = FALSE;
            ConnectionAdminList[soCon].closedByCloseSoCon = FALSE;
            ConnectionAdminList[soCon].abort = FALSE;
            ConnectionAdminList[soCon].mode = SOAD_SOCON_OFFLINE;
            for(uint32 bufIndex=0;bufIndex<SoAdConfigPtr->SocketConnectionGroup[soConGroup].SoAdSocketConnection[soConInGroup].nPduUdpTxBufferSize;bufIndex++){
                SoAdConfigPtr->SocketConnectionGroup[soConGroup].SoAdSocketConnection[soConInGroup].nPduUdpTxBuffer[bufIndex] = 0;
            }
            ConnectionAdminList[soCon].bytesInPduUdpBuffer = 0;
            ConnectionAdminList[soCon].rxBuffer = CirqBuffStatCreate(SoAdConfigPtr->SocketConnectionGroup[soConGroup].SoAdSocketConnection[soConInGroup].TcpRxBuffer, SoAdConfigPtr->SocketConnectionGroup[soConGroup].SoAdSocketConnection[soConInGroup].TcpRxBufferSize, sizeof(uint8));
            ConnectionAdminList[soCon].udpBufTimer = 0;
            ConnectionAdminList[soCon].udpAliveTimer = 0;

            ConnectionAdminList[soCon].remAddrInUse.Set = SoAdConfigPtr->SocketConnectionGroup[soConGroup].SoAdSocketConnection[soConInGroup].SoAdSocketRemoteAddress->Set;
            ConnectionAdminList[soCon].remAddrInUse.Domain = SoAdConfigPtr->SocketConnectionGroup[soConGroup].SoAdSocketConnection[soConInGroup].SoAdSocketRemoteAddress->Domain;
            ConnectionAdminList[soCon].remAddrInUse.SocketRemoteIpAddress[0] = SoAdConfigPtr->SocketConnectionGroup[soConGroup].SoAdSocketConnection[soConInGroup].SoAdSocketRemoteAddress->SocketRemoteIpAddress[0];
            ConnectionAdminList[soCon].remAddrInUse.SocketRemoteIpAddress[1] = SoAdConfigPtr->SocketConnectionGroup[soConGroup].SoAdSocketConnection[soConInGroup].SoAdSocketRemoteAddress->SocketRemoteIpAddress[1];
            ConnectionAdminList[soCon].remAddrInUse.SocketRemoteIpAddress[2] = SoAdConfigPtr->SocketConnectionGroup[soConGroup].SoAdSocketConnection[soConInGroup].SoAdSocketRemoteAddress->SocketRemoteIpAddress[2];
            ConnectionAdminList[soCon].remAddrInUse.SocketRemoteIpAddress[3] = SoAdConfigPtr->SocketConnectionGroup[soConGroup].SoAdSocketConnection[soConInGroup].SoAdSocketRemoteAddress->SocketRemoteIpAddress[3];
            ConnectionAdminList[soCon].remAddrInUse.SocketRemotePort = SoAdConfigPtr->SocketConnectionGroup[soConGroup].SoAdSocketConnection[soConInGroup].SoAdSocketRemoteAddress->SocketRemotePort;
            soCon++;
        }
    }

    for (pduCnt=0; pduCnt < SOAD_NR_OF_PDU_ROUTES; pduCnt++ ){
        SoadTpTransStatus[pduCnt].firstCopyCalled = FALSE;
        SoadTpTransStatus[pduCnt].tpTxRequest = FALSE;
        SoadTpTransStatus[pduCnt].socketId = TCPIP_SOCKETID_INVALID;
        SoadTpTransStatus[pduCnt].totalLen = 0;
        SoadTpTransStatus[pduCnt].remainingLength = 0;
    }
    for (socketCnt=0; socketCnt < SOAD_NR_OF_SOCKET_CONNECTIONS; socketCnt++ ){
        SoAdTpReceiveStatus[socketCnt].firstCopyCalled = FALSE;
        SoAdTpReceiveStatus[socketCnt].tpRxRequest = FALSE;
        SoAdTpReceiveStatus[socketCnt].remaingLen = 0;
        SoAdTpReceiveStatus[socketCnt].socketRouteIndex = 0;
    }
#if SOAD_NR_OF_PDU_ROUTING_GROUP_CONNECTIONS > 0
    /** @req SWS_SOAD_00601 */
    /** @req SWS_SOAD_00721 */
    rtGrpConnIdx=0;
    for(uint32 pduRouteIndex=0;pduRouteIndex<SOAD_NR_OF_PDU_ROUTES;pduRouteIndex++){
        for(uint32 pduRouteDestIndex=0;pduRouteDestIndex<SoAdCfgPtr->PduRoute[pduRouteIndex].NrOfPduRouteDest;pduRouteDestIndex++){
            for(uint32 routingGrpIndex=0;routingGrpIndex<SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].NrOfRoutingGroups;routingGrpIndex++){
                if(SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].GroupUsed == TRUE){
                    for(uint32 indexInGroup=0;indexInGroup<ConnectionGroupAdminList[SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].Index].SocketConnectionGroupRef->NrOfSocketConnections;indexInGroup++){
                        uint16 SocketId = ConnectionGroupAdminList[SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].Index].SocketConnectionGroupRef->SoAdSocketConnection[indexInGroup].SocketId;
                        PduRoutingGroupAdminList[rtGrpConnIdx].PduRouteDest = &(SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex]);
                        PduRoutingGroupAdminList[rtGrpConnIdx].RoutingGroup = SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].TxRoutingGroupRef[routingGrpIndex]->RoutingGroupId;
                        PduRoutingGroupAdminList[rtGrpConnIdx].SoConId = SocketId;
                        PduRoutingGroupAdminList[rtGrpConnIdx].Active = SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].TxRoutingGroupRef[routingGrpIndex]->RoutingGroupIsEnabledAtInit;
                        PduRoutingGroupAdminList[rtGrpConnIdx].TriggerTransmit = FALSE;
                        rtGrpConnIdx++;
                    }
                }
                else{
                    uint16 SocketId = SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].Index;
                    PduRoutingGroupAdminList[rtGrpConnIdx].PduRouteDest = &(SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex]);
                    PduRoutingGroupAdminList[rtGrpConnIdx].RoutingGroup = SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].TxRoutingGroupRef[routingGrpIndex]->RoutingGroupId;
                    PduRoutingGroupAdminList[rtGrpConnIdx].SoConId = SocketId;
                    PduRoutingGroupAdminList[rtGrpConnIdx].Active = SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].TxRoutingGroupRef[routingGrpIndex]->RoutingGroupIsEnabledAtInit;
                    PduRoutingGroupAdminList[rtGrpConnIdx].TriggerTransmit = FALSE;
                    rtGrpConnIdx++;
                }
            }
        }
    }
#endif

    rtGrpConnIdx=0;
    for(uint32 socketRouteIndex=0;socketRouteIndex<SOAD_NR_OF_SOCKET_ROUTES;socketRouteIndex++){
        if(SoAdConfigPtr->SocketRoute[socketRouteIndex].GroupUsed == TRUE){
            for(uint32 indexInGroup=0;indexInGroup<ConnectionGroupAdminList[SoAdConfigPtr->SocketRoute[socketRouteIndex].Index].SocketConnectionGroupRef->NrOfSocketConnections;indexInGroup++){
                for(uint32 routingGroupIndex=0;routingGroupIndex<SoAdConfigPtr->SocketRoute[socketRouteIndex].SocketRouteDest->NrOfRoutingGroups;routingGroupIndex++){
                    uint16 SocketId = ConnectionGroupAdminList[SoAdConfigPtr->SocketRoute[socketRouteIndex].Index].SocketConnectionGroupRef->SoAdSocketConnection[indexInGroup].SocketId;
                    SocketRoutingGroupAdminList[rtGrpConnIdx].SocketRouteDest = SoAdConfigPtr->SocketRoute[socketRouteIndex].SocketRouteDest;
                    SocketRoutingGroupAdminList[rtGrpConnIdx].RoutingGroup = SoAdConfigPtr->SocketRoute[socketRouteIndex].SocketRouteDest->RxRoutingGroupRef[routingGroupIndex]->RoutingGroupId;
                    SocketRoutingGroupAdminList[rtGrpConnIdx].SoConId = SocketId;
                    SocketRoutingGroupAdminList[rtGrpConnIdx].Active = SoAdConfigPtr->SocketRoute[socketRouteIndex].SocketRouteDest->RxRoutingGroupRef[routingGroupIndex]->RoutingGroupIsEnabledAtInit;
                    rtGrpConnIdx++;
                }
            }
        }
        else{
            for(uint32 routingGroupIndex=0;routingGroupIndex<SoAdConfigPtr->SocketRoute[socketRouteIndex].SocketRouteDest->NrOfRoutingGroups;routingGroupIndex++){
                uint16 SocketId = SoAdConfigPtr->SocketRoute[socketRouteIndex].Index;
                SocketRoutingGroupAdminList[rtGrpConnIdx].SocketRouteDest = SoAdConfigPtr->SocketRoute[socketRouteIndex].SocketRouteDest;
                SocketRoutingGroupAdminList[rtGrpConnIdx].RoutingGroup = SoAdConfigPtr->SocketRoute[socketRouteIndex].SocketRouteDest->RxRoutingGroupRef[routingGroupIndex]->RoutingGroupId;
                SocketRoutingGroupAdminList[rtGrpConnIdx].SoConId = SocketId;
                SocketRoutingGroupAdminList[rtGrpConnIdx].Active = SoAdConfigPtr->SocketRoute[socketRouteIndex].SocketRouteDest->RxRoutingGroupRef[routingGroupIndex]->RoutingGroupIsEnabledAtInit;
                rtGrpConnIdx++;
            }
        }
    }
    for (TaskType task_id=0;task_id<OS_TASK_CNT;task_id++){
    	SocketCurrentTx[task_id].SoConId = SOAD_INVALID_CON_ID; /* No transfer going on */
    }
    SoAd_ModuleStatus = SOAD_INITIALIZED;
    SchM_Exit_SoAd_EA_0();
}

Std_ReturnType SoAd_OpenSoCon( SoAd_SoConIdType SoConId ){
    /** @req SWS_SOAD_00615 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_OPEN_SOCON_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00611 */
    VALIDATE_RV(SoConId<SOAD_NR_OF_SOCKET_CONNECTIONS, SOAD_OPEN_SOCON_ID, SOAD_E_INV_ARG, E_NOT_OK)
    /** @req SWS_SOAD_00528 */
    VALIDATE_RV(ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SocketAutomaticSoConSetup==FALSE, SOAD_OPEN_SOCON_ID, SOAD_E_INV_ARG, E_NOT_OK)

    /** @req SWS_SOAD_00588 */
    if( ((ConnectionAdminList[SoConId].opened == TRUE) && (ConnectionAdminList[SoConId].closeRequest == TRUE)) ||
        (ConnectionAdminList[SoConId].opened == FALSE)){
        ConnectionAdminList[SoConId].openRequest = TRUE;
    }

    return E_OK;
}

Std_ReturnType SoAd_CloseSoCon( SoAd_SoConIdType SoConId, boolean abort ){
    /** @req SWS_SOAD_00616 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_CLOSE_SOCON_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00612 */
    VALIDATE_RV(SoConId<SOAD_NR_OF_SOCKET_CONNECTIONS, SOAD_CLOSE_SOCON_ID, SOAD_E_INV_ARG, E_NOT_OK)
    /** @req SWS_SOAD_00529 */
    VALIDATE_RV(ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SocketAutomaticSoConSetup==FALSE, SOAD_CLOSE_SOCON_ID, SOAD_E_INV_ARG, E_NOT_OK)

    uint32 SocketRoute;
    SoAd_SocketRouteDestType socketDest1;
    const SoAd_SocketRouteDestType* socketDest;
    socketDest = &socketDest1;


    // IMPROVEMENT Investigate abort
    ConnectionAdminList[SoConId].abort = abort;

    /** @req SWS_SOAD_00588 */
    if( ConnectionAdminList[SoConId].opened == TRUE ){
        ConnectionAdminList[SoConId].closeRequest = TRUE;
    }else{
        ConnectionAdminList[SoConId].openRequest = FALSE;
    }

    for(SocketRoute=0;SocketRoute<SOAD_NR_OF_SOCKET_ROUTES;SocketRoute++){
        if(soConInSocketRoute(SoConId, &(SoAdCfgPtr->SocketRoute[SocketRoute])) == TRUE){
            socketDest = SoAdCfgPtr->SocketRoute[SocketRoute].SocketRouteDest;
            break;
        }
    }
    /** req This part of code is implemented based on sequence diagram 9.2 Rx TCP message of DoIP SWS */
    if ((SOAD_RX_UPPER_LAYER_TP == socketDest->RxUpperLayerType )&& \
            (SoAdCfgPtr->SocketRoute[SocketRoute].RxPduHeaderId == 0)&&\
            (ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SocketProtocol == SOAD_SOCKET_PROT_TCP)) {

        socketDest->TpRxIndicationFunction(socketDest->RxPduRef, E_OK);
        SoAdTpReceiveStatus[SoConId].tpRxRequest = FALSE;
    }

    return E_OK;
}

void SoAd_LocalIpAddrAssignmentChg( TcpIp_LocalAddrIdType IpAddrId, TcpIp_IpAddrStateType State ){

    /** @req SWS_SOAD_00279 */
    VALIDATE_NO_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_LOCAL_IP_ADDR_ASSIGNMENT_CHG_ID, SOAD_E_NOTINIT)
#if defined(USE_DET) && (SOAD_DEV_ERROR_DETECT == STD_ON)
    boolean found = FALSE;
#endif


    for(uint32 soConGroup=0;soConGroup<SoAdCfgPtr->NrOfSocketConnectionGroups;soConGroup++){
        if(ConnectionGroupAdminList[soConGroup].SocketConnectionGroupRef->SocketLocalAddressRef == IpAddrId){
            ConnectionGroupAdminList[soConGroup].IpAddrState = State;
#if defined(USE_DET) && (SOAD_DEV_ERROR_DETECT == STD_ON)
            found = TRUE;
#endif
            if(ConnectionGroupAdminList[soConGroup].SocketConnectionGroupRef->SocketIpAddrAssignmentChgNotification == TRUE && ConnectionGroupAdminList[soConGroup].SocketConnectionGroupRef->SocketIpAddrAssignmentChgNotificationFunction != NULL){
                for(uint32 soConIndex=0;soConIndex<ConnectionGroupAdminList[soConGroup].SocketConnectionGroupRef->NrOfSocketConnections;soConIndex++){
                    /** @req SWS_SOAD_00598 */
                    ConnectionGroupAdminList[soConGroup].SocketConnectionGroupRef->SocketIpAddrAssignmentChgNotificationFunction(ConnectionGroupAdminList[soConGroup].SocketConnectionGroupRef->SoAdSocketConnection[soConIndex].SocketId, State);
                }
            }
        }
    }
#if defined(USE_DET) && (SOAD_DEV_ERROR_DETECT == STD_ON)
    /** @req SWS_SOAD_00280 */
    VALIDATE_NO_RV(found==TRUE, SOAD_LOCAL_IP_ADDR_ASSIGNMENT_CHG_ID, SOAD_E_INV_ARG)
#endif
}

Std_ReturnType SoAd_SetRemoteAddr( SoAd_SoConIdType SoConId, const TcpIp_SockAddrType* RemoteAddrPtr ){

    Std_ReturnType status;
    status = E_OK;
    /** @req SWS_SOAD_00628 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_SET_REMOTE_ADDRESS_ID, SOAD_E_NOTINIT, E_NOT_OK)
    // Should be a req here as for every other public function using SoConId
    VALIDATE_RV(SoConId<SOAD_NR_OF_SOCKET_CONNECTIONS, SOAD_SET_REMOTE_ADDRESS_ID, SOAD_E_INV_ARG, E_NOT_OK)
    /** @req SWS_SOAD_00531 */
    VALIDATE_RV(ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SocketAutomaticSoConSetup==FALSE, SOAD_SET_REMOTE_ADDRESS_ID, SOAD_E_INV_ARG, E_NOT_OK)
    /* ArcCore req */
    VALIDATE_RV(RemoteAddrPtr!=NULL, SOAD_SET_REMOTE_ADDRESS_ID, SOAD_E_INV_ARG, E_NOT_OK)

    /** @req SWS_SOAD_00532 */ // IMPROVEMENT (2) of req SWS_SOAD_00532
    if((ConnectionAdminList[SoConId].mode != SOAD_SOCON_OFFLINE) && (ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SocketProtocol!=SOAD_SOCKET_PROT_UDP)){
        status = E_NOT_OK;
    } else {
        /** @req SWS_SoAd_00699 */
        if(RemoteAddrPtr->domain != ConnectionAdminList[SoConId].remAddrInUse.Domain){
            DET_REPORTERROR(SOAD_MODULE_ID, 0, SOAD_SET_REMOTE_ADDRESS_ID, SOAD_E_INV_ARG);
            /*lint -e{904} Return statement is necessary in case of reporting a DET error */
            return E_NOT_OK;
        }
        SchM_Enter_SoAd_EA_0();
        setRemoteAddress(SoConId, RemoteAddrPtr);
        /** @req SWS_SoAd_00687 */
        if( (ConnectionAdminList[SoConId].mode == SOAD_SOCON_ONLINE) && (hasWildcardsInAddress(SoConId)) ){
            changeMode(SoConId, SOAD_SOCON_RECONNECT);
        }
        SchM_Exit_SoAd_EA_0();
    }
    return status;
}

void SoAd_MainFunction( void ){
    PduIdType pduCnt;
    SoAd_SoConIdType SoConId;

    /** @req SWS_SOAD_00283 */
    VALIDATE_NO_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_MAIN_FUNCTION_ID, SOAD_E_NOTINIT)

    for (pduCnt=0; pduCnt < SOAD_NR_OF_PDU_ROUTES; pduCnt++ ){
        if(SoadTpTransStatus[pduCnt].tpTxRequest == TRUE){
            tpTransmitSubFunction(pduCnt);
        }
    }

    for(uint16 i=0;i<SOAD_NR_OF_SOCKET_CONNECTIONS;i++){
        Std_ReturnType socModified = E_NOT_OK;

        /** @req SWS_SOAD_00588 */
        if( (ConnectionAdminList[i].opened == TRUE) ){
            if( (ConnectionAdminList[i].closeRequest == TRUE) && (ConnectionAdminList[i].mode != SOAD_SOCON_OFFLINE) ) {/** @req SWS_SOAD_00604 */
                socModified = closeSocket(i, TRUE);
                ConnectionAdminList[i].abort = FALSE;
                ConnectionAdminList[i].opened = FALSE;
                ConnectionAdminList[i].closeRequest = FALSE; // IMPROVEMENT Should this be done even if other criterias aren't fullfilled?
            }
        }else{
            /** @req SWS_SOAD_00589 */
            if( (ConnectionAdminList[i].openRequest == TRUE || (ConnectionAdminList[i].groupAdminRef->SocketConnectionGroupRef->SocketAutomaticSoConSetup == TRUE)) &&  // SWS_SoAd_00589 (2)
                (ConnectionAdminList[i].SocketId == TCPIP_SOCKETID_INVALID) && // SWS_SoAd_00589 (1)
                ((ConnectionAdminList[i].SocketConnectionRef != NULL) && (ConnectionAdminList[i].remAddrInUse.Set == TRUE) ) &&// SWS_SoAd_00589 (3)
                (ConnectionAdminList[i].groupAdminRef->IpAddrState == TCPIP_IPADDR_STATE_ASSIGNED ) ){            // SWS_SoAd_00589 (4)
                            socModified = openSocket(i);
                            ConnectionAdminList[i].opened = TRUE;
                            ConnectionAdminList[i].openRequest = FALSE;
            }
        }
        /** @req SWS_SOAD_00586 */
        /* "Reestablish" connection. What does that mean???. The SoCon can
         * be in state Reconnect with or without a TcpIp Socket. If it has
         *  no TcpIp Socket it should try to get one, but what if it
         *  already has one? */
        if( (socModified == E_NOT_OK) && (ConnectionAdminList[i].mode == SOAD_SOCON_RECONNECT) ){
            switch (ConnectionAdminList[i].groupAdminRef->SocketConnectionGroupRef->GroupType) {
            case SOAD_GROUPTYPE_INIT_TCP:
            case SOAD_GROUPTYPE_LISTEN_TCP:
            case SOAD_GROUPTYPE_LONELY_UDP:
                if( ConnectionAdminList[i].SocketId == TCPIP_SOCKETID_INVALID ){
                    openSocket(i);
                }
                break;
            case SOAD_GROUPTYPE_MULTI_UDP:
                if( ConnectionAdminList[i].groupAdminRef->SocketId == TCPIP_SOCKETID_INVALID ){
                    openSocket(i);
                }
                break;
            default:
                break;
            }

        }
        /** @req SWS_SOAD_00695 */
        if( timerExpiredEvent( &(ConnectionAdminList[i].udpAliveTimer)) == TRUE ){ /* Cannot control the timer , lock it */
            SchM_Enter_SoAd_EA_0();
            changeMode(i, SOAD_SOCON_RECONNECT);
            ConnectionAdminList[i].remAddrInUse.Set = ConnectionAdminList[i].SocketConnectionRef->SoAdSocketRemoteAddress->Set;
            ConnectionAdminList[i].remAddrInUse.Domain = ConnectionAdminList[i].SocketConnectionRef->SoAdSocketRemoteAddress->Domain;
            ConnectionAdminList[i].remAddrInUse.SocketRemoteIpAddress[0] = ConnectionAdminList[i].SocketConnectionRef->SoAdSocketRemoteAddress->SocketRemoteIpAddress[0];
            ConnectionAdminList[i].remAddrInUse.SocketRemoteIpAddress[1] = ConnectionAdminList[i].SocketConnectionRef->SoAdSocketRemoteAddress->SocketRemoteIpAddress[1];
            ConnectionAdminList[i].remAddrInUse.SocketRemoteIpAddress[2] = ConnectionAdminList[i].SocketConnectionRef->SoAdSocketRemoteAddress->SocketRemoteIpAddress[2];
            ConnectionAdminList[i].remAddrInUse.SocketRemoteIpAddress[3] = ConnectionAdminList[i].SocketConnectionRef->SoAdSocketRemoteAddress->SocketRemoteIpAddress[3];
            ConnectionAdminList[i].remAddrInUse.SocketRemotePort = ConnectionAdminList[i].SocketConnectionRef->SoAdSocketRemoteAddress->SocketRemotePort;
            SchM_Exit_SoAd_EA_0();
        }

    }// end loop

    /** @req SWS_SOAD_00544 */
    for(uint32 i=0;i<SOAD_NR_OF_PDU_ROUTES;i++){
        if(PduRouteAdminList[i].TxState == SOAD_TX_STATE_UDP_SENDING_DONE  ){
            if(SoAdCfgPtr->PduRoute[i].TxConfirmationFunction!=NULL){
                SoAdCfgPtr->PduRoute[i].TxConfirmationFunction(SoAdCfgPtr->PduRoute[i].TxPduRef);
            }
            PduRouteAdminList[i].TxState = SOAD_TX_STATE_INACTIVE;
        }
    }

    /** @req SWS_SOAD_00550 */
    for(uint16 i=0;i<SOAD_NR_OF_SOCKET_CONNECTIONS;i++){
        if( timerExpiredEvent( &(ConnectionAdminList[i].udpBufTimer)) == TRUE ){
            transmit(i, ConnectionAdminList[i].bytesInPduUdpBuffer, ConnectionAdminList[i].SocketConnectionRef->nPduUdpTxBuffer, 0);
        }
    }

    /** @req SWS_SOAD_00665 */
    for(uint16 pduRouteIndex=0;pduRouteIndex<SOAD_NR_OF_PDU_ROUTES;pduRouteIndex++){
        if( PduRouteAdminList[pduRouteIndex].trigger.mode != SOAD_TRIGGERTRANSMITMODE_OFF ){

            /** @req SWS_SOAD_00728 */
            PduInfoType Pdu = {.SduDataPtr=SoAd_triggerTransmitBuffer, .SduLength=TRIGGER_TRANMSIT_BUFFER_SIZE};
            if(SoAdCfgPtr->PduRoute[pduRouteIndex].TriggerTransmitFunction!=NULL){
                SoAdCfgPtr->PduRoute[pduRouteIndex].TriggerTransmitFunction(pduRouteIndex, &Pdu );
            }

            for(uint32 pduRouteDestIndex=0;pduRouteDestIndex<SoAdCfgPtr->PduRoute[pduRouteIndex].NrOfPduRouteDest;pduRouteDestIndex++){
                for(uint32 routingGrpIndex=0;routingGrpIndex<SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].NrOfRoutingGroups;routingGrpIndex++){
                    if( PduRouteAdminList[pduRouteIndex].trigger.RoutingGroup == SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].TxRoutingGroupRef[routingGrpIndex]->RoutingGroupId){

                        if(SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].GroupUsed == TRUE){
                            for(uint32 indexInGroup=0;indexInGroup<ConnectionGroupAdminList[SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].Index].SocketConnectionGroupRef->NrOfSocketConnections;indexInGroup++){
                                uint16 SocketId = ConnectionGroupAdminList[SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].Index].SocketConnectionGroupRef->SoAdSocketConnection[indexInGroup].SocketId;
                                if( (PduRouteAdminList[pduRouteIndex].trigger.mode == SOAD_TRIGGERTRANSMITMODE_ROUTINGGROUP_AND_SOCON && SocketId == PduRouteAdminList[pduRouteIndex].trigger.SoConId) ||
                                        (PduRouteAdminList[pduRouteIndex].trigger.mode == SOAD_TRIGGERTRANSMITMODE_ROUTINGGROUP)){

                                    boolean notUsed;
                                    SoConId = ConnectionGroupAdminList[SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].Index].SocketConnectionGroupRef->SoAdSocketConnection[indexInGroup].SocketId;

                                    if(ifTransmitSubFunction(SoConId, pduRouteIndex, &(SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex]), (const PduInfoType*)&Pdu, &notUsed)==E_NOT_OK){
                                        // error
                                    }

                                    PduRouteAdminList[pduRouteIndex].trigger.RoutingGroup = 0;
                                    PduRouteAdminList[pduRouteIndex].trigger.SoConId = 0;
                                }
                            }
                        }
                        else{
                            if( (PduRouteAdminList[pduRouteIndex].trigger.mode == SOAD_TRIGGERTRANSMITMODE_ROUTINGGROUP_AND_SOCON && SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].Index == PduRouteAdminList[pduRouteIndex].trigger.SoConId) ||
                                    (PduRouteAdminList[pduRouteIndex].trigger.mode == SOAD_TRIGGERTRANSMITMODE_ROUTINGGROUP)	){

                                boolean notUsed;
                                SoConId = SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].Index;

                                if(ifTransmitSubFunction(SoConId, pduRouteIndex, &(SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex]), (const PduInfoType*)&Pdu, &notUsed)==E_NOT_OK){
                                    // error
                                }

                                PduRouteAdminList[pduRouteIndex].trigger.RoutingGroup = 0;
                                PduRouteAdminList[pduRouteIndex].trigger.SoConId = 0;
                            }
                        }
                    }
                }
            }
            PduRouteAdminList[pduRouteIndex].trigger.mode = SOAD_TRIGGERTRANSMITMODE_OFF;
        }
    }
}

/*lint -e{818} Pointer parameter 'PduInfoPtr' could be declared as pointing to const.  */
/* No re-entrancy issue here , since called from TCPIP layer */
/* IMPROVEMENT - reduce the work in this function as much as possible */
void SoAd_RxIndication( TcpIp_SocketIdType SocketId, const TcpIp_SockAddrType* RemoteAddrPtr, uint8* BufPtr, uint16 Length ){
    /** @req SWS_SOAD_00264 */
    VALIDATE_NO_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_RX_INDICATION_ID, SOAD_E_NOTINIT)
    /** @req SWS_SOAD_00268 */
    VALIDATE_NO_RV(RemoteAddrPtr!=NULL, SOAD_RX_INDICATION_ID, SOAD_E_INV_ARG)

    uint32 groupIndex;
    uint32 connIndexInGroup;
    const SoAd_SocketConnectionGroupType* grp;
    SoAd_SoConIdType SoConId = 0;
    SoConId = SOCONID_VALUE_ZERO;
    boolean groupSocket;
    PduInfoType rxPDU;
    TcpIp_SockAddrType previousAddr = {.domain = TCPIP_AF_INET, .port = 0, .addr = {0}};
    boolean remAddressOverwritten = FALSE;

    /** @req SWS_SOAD_00562 */

    groupSocket = getGroupIndexFromSocketId(SocketId, &groupIndex);
    if(groupSocket == FALSE){
        if(getSoConIdFromSocketId(SocketId, &SoConId)==FALSE){
            /** @req SWS_SOAD_00267 */
            DET_REPORTERROR(SOAD_MODULE_ID, 0, SOAD_RX_INDICATION_ID, SOAD_E_INV_SOCKETID);
        }
        groupIndex = ConnectionAdminList[SoConId].SocketConnectionRef->GroupNr;
    }
    else{
        SoConId = 0;
    }

    grp = ConnectionGroupAdminList[groupIndex].SocketConnectionGroupRef;
    // IMPROVEMENT IS THIS CORRECT!!! We now run the bestMatch algo first to get only one SoCon.
    if(ConnectionGroupAdminList[groupIndex].SocketConnectionGroupRef->GroupType == SOAD_GROUPTYPE_MULTI_UDP){
        if(runBestMatch(ConnectionGroupAdminList[groupIndex].SocketConnectionGroupRef, RemoteAddrPtr, &connIndexInGroup) == FALSE){
            // IMPROVEMENT Set DET error
            /*lint -e{904} PERFORMANCE, Return statement is necessary to avoid multiple if loops (less cyclomatic complexity) and hence increase readability */
            return;
        }else{
            SoConId = ConnectionGroupAdminList[groupIndex].SocketConnectionGroupRef->SoAdSocketConnection[connIndexInGroup].SocketId;
        }
    }

	/**
	 * Workaround for specification oversight, reported in: https://arccore.atlassian.net/browse/CORE-3115
	 *
	 * This workaround will become ineffective if req SWS_SOAD_00582 is implemented.
	 */
#if (SOAD_XCP_SUPPORT == STD_ON) // If XCP module is linked in the SoAd module
    for(uint32 i = 0; i < SOAD_NR_OF_SOCKET_ROUTES; i++){
		// If current message is meant for XCP
		if (SoAdCfgPtr->SocketRoute[i].SocketRouteDest->RxIndicationFunction == Xcp_SoAdIfRxIndication) {
			if (soConInSocketRoute(SoConId, &(SoAdCfgPtr->SocketRoute[i])) == TRUE) {
				// Then reset current remote address to its configured remote address
				// if it's an automatic UDP connection configured to accept any remote address.
				uint32 grpIdx = ConnectionAdminList[SoConId].SocketConnectionRef->IndexInGroup;
				SoAd_SocketRemoteAddressType* setSoAdSocketRemoteAddress = grp->SoAdSocketConnection[grpIdx].SoAdSocketRemoteAddress;
				if ((grp->SocketProtocol == SOAD_SOCKET_PROT_UDP) && (grp->SocketAutomaticSoConSetup == TRUE) &&
					(((setSoAdSocketRemoteAddress->SocketRemoteIpAddress[0] == TCPIP_IPADDR_ANY) &&
					(setSoAdSocketRemoteAddress->SocketRemoteIpAddress[1] == TCPIP_IPADDR_ANY) &&
					(setSoAdSocketRemoteAddress->SocketRemoteIpAddress[2] == TCPIP_IPADDR_ANY) &&
					(setSoAdSocketRemoteAddress->SocketRemoteIpAddress[3] == TCPIP_IPADDR_ANY)) ||
					(setSoAdSocketRemoteAddress->SocketRemotePort == TCPIP_PORT_ANY))) {
					SchM_Enter_SoAd_EA_0();
					changeMode(SoConId, SOAD_SOCON_RECONNECT);
					setRemoteAddressFromCfg(SoConId);
					SchM_Exit_SoAd_EA_0();
				}
			}
		}
	}
#endif

    /** @req SWS_SOAD_00592 */
    if( (ConnectionAdminList[SoConId].mode != SOAD_SOCON_ONLINE) &&
            (grp->SocketProtocol == SOAD_SOCKET_PROT_UDP) &&
            (grp->SocketProtocolUdp->SocketUdpListenOnly == FALSE) &&
            (grp->SocketMsgAcceptanceFilterEnabled == TRUE) &&
            (hasWildcardsInAddress(SoConId)==TRUE) &&
            (runMessageAcceptancePolicy() == TRUE)){
        SchM_Enter_SoAd_EA_0();
        for(uint32 i=0;i<4;i++){
            previousAddr.addr[i] = ConnectionAdminList[SoConId].remAddrInUse.SocketRemoteIpAddress[i];
            ConnectionAdminList[SoConId].remAddrInUse.SocketRemoteIpAddress[i] = RemoteAddrPtr->addr[i];
        }
        previousAddr.port = ConnectionAdminList[SoConId].remAddrInUse.SocketRemotePort;
        ConnectionAdminList[SoConId].remAddrInUse.SocketRemotePort = RemoteAddrPtr->port;
        changeMode(SoConId, SOAD_SOCON_ONLINE);
        SchM_Exit_SoAd_EA_0();
        remAddressOverwritten = TRUE;
    }


    /** @req SWS_SOAD_00694 */
    if( (grp->SocketProtocol == SOAD_SOCKET_PROT_UDP) && (grp->SocketProtocolUdp->UdpAliveSupervisionTimeout>0) &&
        ( (ConnectionAdminList[SoConId].udpAliveTimer > 0) || (remAddressOverwritten == TRUE) ) ){
        SchM_Enter_SoAd_EA_0();
        ConnectionAdminList[SoConId].udpAliveTimer = grp->SocketProtocolUdp->UdpAliveSupervisionTimeout;
        SchM_Exit_SoAd_EA_0();
    }

    /** @req SWS_SOAD_00657 */

    // runMessageAcceptancePolicy();

    /** @req SWS_SOAD_00559 */
    /** @req SWS_SOAD_00567 */
    if( ConnectionGroupAdminList[groupIndex].SocketConnectionGroupRef->PduHeaderEnable == TRUE ){
        /*Header Enable */

        uint32 headerId = 0u;
        uint32 pduLength = 0u;
        uint32 indexInBuf = 0u;

        /* @req SWS_SOAD_00690 *//* With Header IF - TP : UDP */
        if(IS_UDP_GRP(groupIndex)){
            /* @req SWS_SOAD_00709 */
            if(ConnectionGroupAdminList[groupIndex].SocketConnectionGroupRef->SocketProtocolUdp->SocketUdpStrictHeaderLenCheckEnabled == TRUE){
                uint32 tempIndex = indexInBuf;
                while( (tempIndex < Length) && (getPduHeader(&BufPtr[tempIndex], Length-tempIndex, &headerId, &pduLength) == TRUE) ){
                    tempIndex = tempIndex + SOAD_PDU_HEADER_LENGTH + pduLength;
                }
                if(tempIndex != Length){
                    /*lint -e{904} PERFORMANCE, Return statement is necessary to avoid multiple if loops (less cyclomatic complexity) and hence increase readability */
                    return;
                }
            }

            while(indexInBuf<Length){
                if( getPduHeader(&BufPtr[indexInBuf], Length-indexInBuf, &headerId, &pduLength) == TRUE ){
                    indexInBuf+=SOAD_PDU_HEADER_LENGTH;
                    routingReceptionPduHeader(SoConId, headerId, &(BufPtr[indexInBuf]), pduLength);
                    indexInBuf += pduLength;
                }else{
                    /*lint -e{904} PERFORMANCE, Return statement is necessary to avoid multiple if loops (less cyclomatic complexity) and hence increase readability */
                    return;
                }
            }

        } else { /* With Header IF - TP : TCP */

            uint8 SocketRoute1;
            const SoAd_SocketRouteDestType*  socketDest;
            socketDest = NULL;
            for(SocketRoute1=0; SocketRoute1<SOAD_NR_OF_SOCKET_ROUTES; SocketRoute1++){
                if(soConInSocketRoute(SoConId, &(SoAdCfgPtr->SocketRoute[SocketRoute1])) == TRUE ){
                    socketDest = SoAdCfgPtr->SocketRoute[SocketRoute1].SocketRouteDest;
                    break;
                }
            }
            if (socketDest != NULL) {
                if (SOAD_RX_UPPER_LAYER_TP == socketDest->RxUpperLayerType ) { /* With header TP - TCP*/
                    processTpTcpRxCommunication(SocketId, SoConId, BufPtr, Length);

                } else { /* With header IF - TCP*/

                    while(indexInBuf < Length && (CirqBuff_Full(&(ConnectionAdminList[SoConId].rxBuffer)) == FALSE)){
                        // Copy to buffer
                        /** @req SWS_SOAD_00566 */
                        while( (indexInBuf < Length) && (CirqBuffPush(&(ConnectionAdminList[SoConId].rxBuffer), (void *)&(BufPtr[indexInBuf])) == 0) ){
                            indexInBuf++;
                        }
                        if( CirqBuff_Size(&(ConnectionAdminList[SoConId].rxBuffer)) > SOAD_PDU_HEADER_LENGTH ){
                            headerId = 0;
                            pduLength = 0;
                            headerId =  headerId | (*((uint8*)CirqBuff_Peek(&(ConnectionAdminList[SoConId].rxBuffer), 0 ))<<24);
                            headerId =  headerId | (*((uint8*)CirqBuff_Peek(&(ConnectionAdminList[SoConId].rxBuffer), 1 ))<<16);
                            headerId =  headerId | (*((uint8*)CirqBuff_Peek(&(ConnectionAdminList[SoConId].rxBuffer), 2 ))<<8);
                            headerId =  headerId | (*((uint8*)CirqBuff_Peek(&(ConnectionAdminList[SoConId].rxBuffer), 3 )));
                            pduLength = pduLength | (*((uint8*)CirqBuff_Peek(&(ConnectionAdminList[SoConId].rxBuffer), 4 ))<<24);
                            pduLength = pduLength | (*((uint8*)CirqBuff_Peek(&(ConnectionAdminList[SoConId].rxBuffer), 5 ))<<16);
                            pduLength = pduLength | (*((uint8*)CirqBuff_Peek(&(ConnectionAdminList[SoConId].rxBuffer), 6 ))<<8);
                            pduLength = pduLength | (*((uint8*)CirqBuff_Peek(&(ConnectionAdminList[SoConId].rxBuffer), 7 )));

                            if( CirqBuff_Size(&(ConnectionAdminList[SoConId].rxBuffer)) >= (sint32)(SOAD_PDU_HEADER_LENGTH + pduLength) ){
                                uint8 tempByte;

                                for(uint32 i=0;i<8;i++){
                                    CirqBuffPop( &(ConnectionAdminList[SoConId].rxBuffer), (void*)&tempByte );
                                }
                                for(uint32 i=0;i<pduLength;i++){
                                    if( CirqBuffPop( &(ConnectionAdminList[SoConId].rxBuffer), (void*)(&(SoAd_tmpTcpRxBuf[i])) ) > 0){
                                        // IMPROVEMENT Set DET error
                                        /*lint -e{904} PERFORMANCE, Return statement is necessary to avoid multiple if loops (less cyclomatic complexity) and hence increase readability */
                                        return;
                                    }
                                }
                                routingReceptionPduHeader(SoConId, headerId, &(SoAd_tmpTcpRxBuf[0]), pduLength);
                            }
                        }
                    }
                    if((CirqBuff_Full(&(ConnectionAdminList[SoConId].rxBuffer)) == TRUE) && (indexInBuf < Length)){
                        /** @req SWS_SOAD_00693 */
                        DET_REPORTERROR(SOAD_MODULE_ID, 0, SOAD_RX_INDICATION_ID, SOAD_E_NOBUFS);
                        /*lint -e{904} PERFORMANCE, Return statement is necessary in case of reporting a DET error */
                        return;
                    }
                }
            }
        }
    }else{ /* Without header IF - TP : UDP & TCP */

        /** @req SWS_SOAD_00563 */
        rxPDU.SduLength = Length;
        rxPDU.SduDataPtr = (uint8* )BufPtr;

        if((rxPDU.SduLength == 0) && (ConnectionGroupAdminList[groupIndex].SocketConnectionGroupRef->PduHeaderEnable==FALSE)){
            // Skip further processing
            /** @req SWS_SOAD_00710 */
            if(remAddressOverwritten==TRUE){
                SchM_Enter_SoAd_EA_0();
                for(uint32 i=0;i<4;i++){
                    ConnectionAdminList[SoConId].remAddrInUse.SocketRemoteIpAddress[i] = previousAddr.addr[i];
                }
                ConnectionAdminList[SoConId].remAddrInUse.SocketRemotePort = previousAddr.port;
                changeMode(SoConId, SOAD_SOCON_RECONNECT);
                SchM_Exit_SoAd_EA_0();
            }
            /** @req SWS_SOAD_00564 */
            if(ConnectionGroupAdminList[groupIndex].SocketConnectionGroupRef->SocketProtocol == SOAD_SOCKET_PROT_TCP){
                TcpIp_TcpReceived(SocketId, Length);
            }
            /*lint -e{904} PERFORMANCE, Return statement is necessary to avoid multiple if loops (less cyclomatic complexity) and hence increase readability */
            return;
        }

        /** @req SWS_SOAD_00600 */
        routingReceptionNoHeader(SoConId, &rxPDU);
    }
    /** @req SWS_SOAD_00564 */
    if(!IS_UDP_GRP(groupIndex) ){
        TcpIp_TcpReceived(SocketId, Length);
    }

}

void SoAd_TcpConnected( TcpIp_SocketIdType SocketId ){
    /** @req SWS_SOAD_00274 */
    VALIDATE_NO_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_TCP_CONNECTED_ID, SOAD_E_NOTINIT)
    const SoAd_SocketConnectionGroupType* grp;
    SoAd_SoConIdType SoConId;
    SoConId = SOCONID_VALUE_ZERO;

    if(getSoConIdFromSocketId(SocketId, &SoConId)==FALSE){
        /** @req SWS_SOAD_00275 */
        DET_REPORTERROR(SOAD_MODULE_ID, 0, SOAD_TCP_CONNECTED_ID, SOAD_E_INV_SOCKETID);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }
    grp = ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef;

    /** @req SWS_SOAD_00593 */
    if( (ConnectionAdminList[SoConId].mode != SOAD_SOCON_ONLINE) &&
            (grp->SocketProtocol == SOAD_SOCKET_PROT_TCP) &&
            (grp->SocketProtocolTcp->SocketTcpInitiate == TRUE)){
    	SchM_Enter_SoAd_EA_0();
        changeMode(SoConId, SOAD_SOCON_ONLINE);
    	SchM_Exit_SoAd_EA_0();
    }
}

Std_ReturnType SoAd_TcpAccepted( TcpIp_SocketIdType SocketId, TcpIp_SocketIdType SocketIdConnected, const TcpIp_SockAddrType* RemoteAddrPtr ){
    /** @req SWS_SOAD_00272 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_TCP_ACCEPTED_ID, SOAD_E_NOTINIT, E_NOT_OK)

    boolean retVal = E_NOT_OK;
    const SoAd_SocketConnectionGroupType* grp;
    SoAd_SoConIdType SoConId;
    uint32 GroupIndex;
    uint32 ConnIndexInGroup;
    //boolean bestMatchFound = FALSE;

    if(getGroupIndexFromSocketId(SocketId, &GroupIndex)==FALSE){
        /** @req SWS_SOAD_00273 */
        DET_REPORTERROR(SOAD_MODULE_ID, 0, SOAD_TCP_ACCEPTED_ID, SOAD_E_INV_SOCKETID);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return E_NOT_OK;
    }
    grp = ConnectionGroupAdminList[GroupIndex].SocketConnectionGroupRef;

    /** @req SWS_SOAD_00636 */
    if( (grp->SocketProtocolTcp->SocketTcpInitiate == FALSE) ){
        if( (grp->SocketMsgAcceptanceFilterEnabled == FALSE) && (ConnectionGroupAdminList[GroupIndex].mode != SOAD_SOCON_ONLINE) ){
            // IMPROVEMENT Maybe generate a cross reference from the grouplist to the connectionlist so we don't have to loop
            for(SoConId=0;SoConId<SOAD_NR_OF_SOCKET_CONNECTIONS;SoConId++){
                // MsgAccepFilter==FALSE implies: One and only one connection in the group
                if(&(grp->SoAdSocketConnection[0]) == ConnectionAdminList[SoConId].SocketConnectionRef){
                	SchM_Enter_SoAd_EA_0();
                    ConnectionAdminList[SoConId].SocketId = SocketIdConnected;
                    changeMode(SoConId, SOAD_SOCON_ONLINE);
                	SchM_Exit_SoAd_EA_0();
                    retVal = E_OK;
                    break;
                }
            }
        }
        /** @req SWS_SOAD_00594 */
        else if(runBestMatch(grp, RemoteAddrPtr, &ConnIndexInGroup) == TRUE){
            // IMPROVEMENT Maybe generate a cross reference from the grouplist to the connectionlist so we don't have to loop
            for(SoConId=0;SoConId<SOAD_NR_OF_SOCKET_CONNECTIONS;SoConId++){
                if(&(grp->SoAdSocketConnection[ConnIndexInGroup]) == ConnectionAdminList[SoConId].SocketConnectionRef){
                	SchM_Enter_SoAd_EA_0();
                    setRemoteAddress(SoConId, RemoteAddrPtr);
                    ConnectionAdminList[SoConId].SocketId = SocketIdConnected;
                    changeMode(SoConId, SOAD_SOCON_ONLINE);
                	SchM_Exit_SoAd_EA_0();
                    retVal = E_OK;
                    break;
                }
            }
        }
        else {
            /* MISRA */
        }
    }
    return retVal;
}

void SoAd_TcpIpEvent( TcpIp_SocketIdType SocketId, TcpIp_EventType Event ){

    boolean status;
    status = TRUE;
    /** @req SWS_SOAD_00276 */
    VALIDATE_NO_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_TCP_IP_EVENT, SOAD_E_NOTINIT)
    /** @req SWS_SOAD_00278 */
    /*lint -e{645, 685} CONFIGURATION */
    VALIDATE_NO_RV(Event<=TCPIP_UDP_CLOSED, SOAD_TCP_IP_EVENT, SOAD_E_INV_ARG)

    const SoAd_SocketConnectionGroupType* grp;
    SoAd_SoConIdType SoConId;
    uint32 GroupIndex;
    boolean groupSocketUsed;
    SoConId = SOCONID_VALUE_ZERO;
    GroupIndex= SOAD_GROUP_INDEX_VALUE_ZERO;

    uint32 SocketRoute;
    SoAd_SocketRouteDestType socketDest1;
    const SoAd_SocketRouteDestType* socketDest;
    socketDest = &socketDest1;


    if(getSoConIdFromSocketId(SocketId, &SoConId)==FALSE){
        /** @req SWS_SOAD_00277 */
        DET_REPORTERROR(SOAD_MODULE_ID, 0, SOAD_TCP_IP_EVENT, SOAD_E_INV_SOCKETID);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }

    groupSocketUsed = getGroupIndexFromSocketId(SocketId, &GroupIndex);

    grp = ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef;

    switch (Event) {
    /** @req SWS_SOAD_00643 */
    case TCPIP_UDP_CLOSED:
        SchM_Enter_SoAd_EA_0();
        if( grp->NrOfSocketConnections == 1 ){
            ConnectionAdminList[SoConId].SocketId = TCPIP_SOCKETID_INVALID;
            // Reset the address. This is not in the SoAd Requirements but it should be!
            setRemoteAddressFromCfg(SoConId);
        }else{
            if(groupSocketUsed == FALSE){
                //IMPROVEMENT Det error. An UDP group with more than one connection should use group socket.
                status = FALSE;
            } else {
                ConnectionGroupAdminList[GroupIndex].SocketId = TCPIP_SOCKETID_INVALID;
                for(uint16 i=0;i<SOAD_NR_OF_SOCKET_CONNECTIONS;i++){
                    if( (ConnectionAdminList[i].groupAdminRef == &(ConnectionGroupAdminList[GroupIndex])) && (ConnectionAdminList[i].mode == SOAD_SOCON_ONLINE) ){
                        //Close this connection
                        if( ConnectionAdminList[SoConId].closedByCloseSoCon == TRUE ){
                            changeMode(i, SOAD_SOCON_OFFLINE);
                        }
                        else{
                            changeMode(i, SOAD_SOCON_RECONNECT);
                        }
                        // Reset the address. This is not in the SoAd Requirements but it should be!
                        setRemoteAddressFromCfg(i);
                    }
                }
            }
        }
        if (status == TRUE) {
            // Should be able to reset this now.
            ConnectionAdminList[SoConId].closedByCloseSoCon = FALSE;
        }
        SchM_Exit_SoAd_EA_0();
        break;
    case TCPIP_TCP_CLOSED:

        for(SocketRoute=0;SocketRoute<SOAD_NR_OF_SOCKET_ROUTES;SocketRoute++){
            if(soConInSocketRoute(SoConId, &(SoAdCfgPtr->SocketRoute[SocketRoute])) == TRUE){
                socketDest = SoAdCfgPtr->SocketRoute[SocketRoute].SocketRouteDest;
                break;
            }
        }

        /** req This part of code is implemented based on sequence diagram 9.2 Rx TCP message of DoIP SWS */
        if ((SOAD_RX_UPPER_LAYER_TP == socketDest->RxUpperLayerType )&& \
                (SoAdCfgPtr->SocketRoute[SocketRoute].RxPduHeaderId == 0)&&\
                (ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SocketProtocol == SOAD_SOCKET_PROT_TCP)) {

            socketDest->TpRxIndicationFunction(socketDest->RxPduRef, E_OK);
            SoAdTpReceiveStatus[SoConId].tpRxRequest = FALSE;
        }
        break;
    case TCPIP_TCP_RESET:
        /** @req SWS_SOAD_00645 */
        SchM_Enter_SoAd_EA_0();
        if(groupSocketUsed==TRUE){
            ConnectionGroupAdminList[GroupIndex].SocketId = TCPIP_SOCKETID_INVALID;
        }
        else{
            /** @req SWS_SOAD_00646 */
            ConnectionAdminList[SoConId].SocketId = TCPIP_SOCKETID_INVALID;
            if(ConnectionAdminList[SoConId].mode == SOAD_SOCON_ONLINE){
                if( ConnectionAdminList[SoConId].closedByCloseSoCon == TRUE ){
                    changeMode(SoConId, SOAD_SOCON_OFFLINE);
                }
                else{
                    changeMode(SoConId, SOAD_SOCON_RECONNECT);
                }
            }
            // Reset the address. This is not in the SoAd Requirements but it should be!
            setRemoteAddressFromCfg(SoConId);
        }
        SchM_Exit_SoAd_EA_0();
        break;
    case TCPIP_TCP_FIN_RECEIVED:
        /** @req SWS_SOAD_00688 */
        TcpIp_Close(ConnectionAdminList[SoConId].SocketId, FALSE );
        break;
    default:
        break;
    }
    if (status == TRUE) {
        // IMPROVEMENT Should be able to reset this now.
        SchM_Enter_SoAd_EA_0();
        ConnectionAdminList[SoConId].closedByCloseSoCon = FALSE;
        SchM_Exit_SoAd_EA_0();
    }
}

Std_ReturnType SoAd_IfTransmit( PduIdType SoAdSrcPduId, const PduInfoType* SoAdSrcPduInfoPtr ){
    /** @req SWS_SOAD_00213 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_IF_TRANSMIT_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00214 */
    VALIDATE_RV(SoAdSrcPduId<SOAD_NR_OF_TX_PDUS, SOAD_IF_TRANSMIT_ID, SOAD_E_INV_PDUID, E_NOT_OK)

    boolean atLeastOneDestSent = FALSE;
    uint16 connIndex=0;
    Std_ReturnType retVal=E_NOT_OK;

    const SoAd_PduRouteType* pduRoute;

    pduRoute = &(SoAdCfgPtr->PduRoute[SoAdSrcPduId]);

    /** @req SWS_SOAD_00602 */
    for(uint32 i=0;i<pduRoute->NrOfPduRouteDest;i++){

        /** @req SWS_SOAD_00539 */
        if(pduRoute->PduRouteDest[i].GroupUsed == FALSE){
            if( (SoAdSrcPduInfoPtr->SduLength==0) && (ConnectionAdminList[pduRoute->PduRouteDest[i].Index].groupAdminRef->SocketConnectionGroupRef->PduHeaderEnable == FALSE) ){
                /*lint -e{904} Return statement is necessary to avoid multiple if loops (less cyclomatic complexity) and hence increase readability */
                return E_NOT_OK;
            }


        }
        else{
            if( (SoAdSrcPduInfoPtr->SduLength==0) && (ConnectionGroupAdminList[pduRoute->PduRouteDest[i].Index].SocketConnectionGroupRef->PduHeaderEnable == FALSE) ){
                /*lint -e{904} Return statement is necessary to avoid multiple if loops (less cyclomatic complexity) and hence increase readability */
                return E_NOT_OK;
            }
        }


        /* @req SWS_SOAD_00648 */
        if(pduRoute->PduRouteDest[i].GroupUsed == FALSE){
            connIndex = pduRoute->PduRouteDest[i].Index;
            if(ifTransmitSubFunction(connIndex, SoAdSrcPduId, &(pduRoute->PduRouteDest[i]), SoAdSrcPduInfoPtr, &atLeastOneDestSent)==E_NOT_OK){
                /*lint -e{904} Return statement is necessary to avoid multiple if loops (less cyclomatic complexity) and hence increase readability */
                return E_NOT_OK;
            }
        }
        else{
            /** @req SWS_SOAD_00722 */
            for(uint32 j=0;j<ConnectionGroupAdminList[pduRoute->PduRouteDest[i].Index].SocketConnectionGroupRef->NrOfSocketConnections;j++){
                connIndex = ConnectionGroupAdminList[pduRoute->PduRouteDest[i].Index].SocketConnectionGroupRef->SoAdSocketConnection[j].SocketId;
                if(ifTransmitSubFunction(connIndex, SoAdSrcPduId, &(pduRoute->PduRouteDest[i]), SoAdSrcPduInfoPtr, &atLeastOneDestSent)==E_NOT_OK){
                    /*lint -e{904} Return statement is necessary to avoid multiple if loops (less cyclomatic complexity) and hence increase readability */
                    return E_NOT_OK;
                }
            }
        }
    } /* for(i=0;i<pduRoute->NrOfPduRouteDest;i++) */

    /* @req SWS_SOAD_00647 */
    SchM_Enter_SoAd_EA_0();
    if( pduRoute->PduRouteDest[0].GroupUsed == TRUE ){
        if(ConnectionGroupAdminList[pduRoute->PduRouteDest[0].Index].SocketConnectionGroupRef->SocketProtocol == SOAD_SOCKET_PROT_UDP){
            PduRouteAdminList[SoAdSrcPduId].TxState = SOAD_TX_STATE_UDP_SENDING_DONE;
        }
    }else{
        if(ConnectionAdminList[pduRoute->PduRouteDest[0].Index].groupAdminRef->SocketConnectionGroupRef->SocketProtocol == SOAD_SOCKET_PROT_UDP){
            PduRouteAdminList[SoAdSrcPduId].TxState = SOAD_TX_STATE_UDP_SENDING_DONE;
        }
    }
    SchM_Exit_SoAd_EA_0();

	/** @req SWS_SOAD_00560 */
    if(atLeastOneDestSent == TRUE){
        retVal = E_OK;
    }
    else{
        retVal = E_NOT_OK;
    }
    return retVal;
}

BufReq_ReturnType SoAd_CopyTxData( TcpIp_SocketIdType SocketId, uint8* BufPtr, uint16 BufLength ){

    const SoAd_PduRouteType* pduRoute;
    SoadTptransmitType *tpRunTimeParm;
    uint32 pduHeaderId;
    PduInfoType pduInfo;
    PduLengthType remaingLength;
    PduIdType pduCnt;
    TaskType task_id;
    uint16 lenNoHeader;
    BufReq_ReturnType retValCopydata;
    Std_ReturnType retVal;
    boolean isTpSocket;

    retValCopydata = BUFREQ_NOT_OK;
    retVal = E_OK;
    isTpSocket = FALSE;

    // task_id is used as an array index to protect SocketCurrentTx from possible task switches.
    // The instance of SocketCurrentTx for the current task will then be used, even if a context switch has occurred.
    retVal = GetTaskID(&task_id);
    if ((retVal != E_OK) || (task_id >= OS_TASK_CNT)){
        return BUFREQ_NOT_OK;
    }

    /** @req SWS_SOAD_00632 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_COPY_TX_DATA_ID, SOAD_E_NOTINIT, BUFREQ_E_NOT_OK)

    /* Check if Socket is used for Tp communication */
    for (pduCnt = 0; pduCnt < SOAD_NR_OF_PDU_ROUTES; pduCnt++ ){
        tpRunTimeParm = &SoadTpTransStatus[pduCnt];
        //There is active Tp transmit request which has locked this socket
        if((tpRunTimeParm->socketId == SocketId) && (TRUE == tpRunTimeParm->tpTxRequest)){

            pduRoute = &(SoAdCfgPtr->PduRoute[pduCnt]);
            pduHeaderId = pduRoute->PduRouteDest[SOAD_PDU_ROUTE_FIRST_DESTINATION_IDX].TxPduHeaderId;
            if ((tpRunTimeParm->firstCopyCalled == FALSE) && (pduHeaderId != 0)) {
                /* Copy PDU header */
                lenNoHeader = (tpRunTimeParm->totalLen - SOAD_PDU_HEADER_LENGTH);
                BufPtr[0] =(uint8)((pduHeaderId & 0xFF000000u)>>24);
                BufPtr[1] =(uint8)((pduHeaderId & 0x00FF0000u)>>16);
                BufPtr[2] = (uint8)((pduHeaderId & 0x0000FF00u)>>8);
                BufPtr[3] = (uint8) (pduHeaderId & 0x000000FFu);
                BufPtr[4] = 0u;
                BufPtr[5] = 0u;
                BufPtr[6] = (uint8)((lenNoHeader & 0x0000FF00u)>>8);
                BufPtr[7] = (uint8) (lenNoHeader & 0x000000FFu);
                pduInfo.SduDataPtr = &BufPtr[SOAD_PDU_HEADER_LENGTH];
                pduInfo.SduLength = BufLength - SOAD_PDU_HEADER_LENGTH;
            } else {
                pduInfo.SduDataPtr = BufPtr;
                pduInfo.SduLength = BufLength;
            }
            retValCopydata = pduRoute->SoAdTpCopyTxData(pduRoute->TxPduRef, &pduInfo, NULL, &remaingLength);
            tpRunTimeParm->remainingLength = remaingLength;
            if (BUFREQ_OK == retValCopydata) {
                tpRunTimeParm->firstCopyCalled = TRUE; //First copy done
            }
            isTpSocket = TRUE;
            break; //Exit loop as we found that socket belongs to Tp type
        }
    }

    if (FALSE == isTpSocket) {
        /** @req SWS_SOAD_00633 */
        VALIDATE_RV(((SocketCurrentTx[task_id].SoConId != SOAD_INVALID_CON_ID) && (ConnectionAdminList[SocketCurrentTx[task_id].SoConId].SocketId == SocketId)),SOAD_COPY_TX_DATA_ID, SOAD_E_INV_SOCKETID, BUFREQ_E_NOT_OK)

        /** @req SWS_SOAD_00543 */
        if( (ConnectionAdminList[SocketCurrentTx[task_id].SoConId].SocketConnectionRef->nPduUdpTxBufferSize > 0) ||
                (ConnectionAdminList[SocketCurrentTx[task_id].SoConId].groupAdminRef->SocketConnectionGroupRef->PduHeaderEnable == FALSE)){
                memcpy(BufPtr,SocketCurrentTx[task_id].TxBufCopy,BufLength);
        }
        else{
            /** @req SWS_SOAD_00197 */
            /** @req SWS_SOAD_00198 */
            lenNoHeader = (BufLength - SOAD_PDU_HEADER_LENGTH);
            BufPtr[0] =(uint8)((SocketCurrentTx[task_id].TxPduHeaderId & 0xFF000000u)>>24);
            BufPtr[1] =(uint8)((SocketCurrentTx[task_id].TxPduHeaderId & 0x00FF0000u)>>16);
            BufPtr[2]= (uint8)((SocketCurrentTx[task_id].TxPduHeaderId & 0x0000FF00u)>>8);
            BufPtr[3]= (uint8) (SocketCurrentTx[task_id].TxPduHeaderId & 0x000000FFu);
            BufPtr[4]= 0u;
            BufPtr[5]= 0u;
            BufPtr[6]= (uint8)((lenNoHeader & 0x0000FF00u)>>8);
            BufPtr[7]= (uint8) (lenNoHeader & 0x000000FFu);
            memcpy(&BufPtr[SOAD_PDU_HEADER_LENGTH],SocketCurrentTx[task_id].TxBufCopy,lenNoHeader);
        }
        SchM_Enter_SoAd_EA_0();
        SocketCurrentTx[task_id].SoConId = SOAD_INVALID_CON_ID;
        SchM_Exit_SoAd_EA_0();

        retValCopydata = BUFREQ_OK;

    }

    return retValCopydata;
}

void SoAd_TxConfirmation( TcpIp_SocketIdType SocketId, uint16 Length ){
    /** @req SWS_SOAD_00269 */
    VALIDATE_NO_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_TX_CONFIRMATION_ID, SOAD_E_NOTINIT)
    /** @req SWS_SOAD_00271 */
    VALIDATE_NO_RV(Length>0, SOAD_TX_CONFIRMATION_ID, SOAD_E_INV_ARG)

    SoAd_SoConIdType SoConId;
    SoadTptransmitType *tpRunTimeParm;

    if(getSoConIdFromSocketId(SocketId, &SoConId)==FALSE){
        /** @req SWS_SOAD_00270 */
        DET_REPORTERROR(SOAD_MODULE_ID, 0, SOAD_TX_CONFIRMATION_ID, SOAD_E_INV_SOCKETID);
        return;
    }
    /** @req SWS_SOAD_00545 */
    for(uint32 pduRouteIndex=0;pduRouteIndex<SOAD_NR_OF_PDU_ROUTES;pduRouteIndex++){
        if(PduRouteAdminList[pduRouteIndex].unconfirmedMessages>0){
            for(uint32 pduRouteDestIndex=0;pduRouteDestIndex<SoAdCfgPtr->PduRoute[pduRouteIndex].NrOfPduRouteDest;pduRouteDestIndex++){
                if(SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].GroupUsed == TRUE){
                    for(uint32 indexInGroup=0;indexInGroup<ConnectionGroupAdminList[SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].Index].SocketConnectionGroupRef->NrOfSocketConnections;indexInGroup++){
                        uint32 TmpSocketId = ConnectionGroupAdminList[SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].Index].SocketConnectionGroupRef->SoAdSocketConnection[indexInGroup].SocketId;
                        if(ConnectionAdminList[TmpSocketId].SocketId == SocketId ){
                            PduRouteAdminList[pduRouteIndex].unconfirmedMessages--;
                            if(PduRouteAdminList[pduRouteIndex].unconfirmedMessages==0){
                                tpRunTimeParm = &SoadTpTransStatus[pduRouteIndex];
                                if(SoAdCfgPtr->PduRoute[pduRouteIndex].TxConfirmationFunction!=NULL){
                                    SoAdCfgPtr->PduRoute[pduRouteIndex].TxConfirmationFunction(SoAdCfgPtr->PduRoute[pduRouteIndex].TxPduRef);
                                }
                                else if((SoAdCfgPtr->PduRoute[pduRouteIndex].TpTxConfirmationFunction!=NULL)&&(tpRunTimeParm->remainingLength == 0)){
                                    SoAdCfgPtr->PduRoute[pduRouteIndex].TpTxConfirmationFunction(SoAdCfgPtr->PduRoute[pduRouteIndex].TxPduRef, E_OK);
                                    SoadTpTransStatus[pduRouteIndex].tpTxRequest = FALSE; //Reset any Tp request on this route
                                }
                                return;
                            }
                        }
                    }
                }else{
                    uint32 TmpSocketId = SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].Index;
                    if(ConnectionAdminList[TmpSocketId].SocketId == SocketId ){
                        PduRouteAdminList[pduRouteIndex].unconfirmedMessages--;
                        if(PduRouteAdminList[pduRouteIndex].unconfirmedMessages==0){
                            tpRunTimeParm = &SoadTpTransStatus[pduRouteIndex];
                            if(SoAdCfgPtr->PduRoute[pduRouteIndex].TxConfirmationFunction!=NULL){
                                SoAdCfgPtr->PduRoute[pduRouteIndex].TxConfirmationFunction(SoAdCfgPtr->PduRoute[pduRouteIndex].TxPduRef);
                            }
                            else if((SoAdCfgPtr->PduRoute[pduRouteIndex].TpTxConfirmationFunction!=NULL)&&(tpRunTimeParm->remainingLength == 0)){
                                SoAdCfgPtr->PduRoute[pduRouteIndex].TpTxConfirmationFunction(SoAdCfgPtr->PduRoute[pduRouteIndex].TxPduRef, E_OK);
                                SoadTpTransStatus[pduRouteIndex].tpTxRequest = FALSE; //Reset any Tp request on this route
                            }
                            return;
                        }
                    }
                }
            }
        }
    }
}

Std_ReturnType SoAd_GetRemoteAddr( SoAd_SoConIdType SoConId, TcpIp_SockAddrType* IpAddrPtr ){
    /** @req SWS_SOAD_00659 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_GET_REMOTE_ADDR_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00660 */
    VALIDATE_RV(SoConId<SOAD_NR_OF_SOCKET_CONNECTIONS, SOAD_GET_REMOTE_ADDR_ID, SOAD_E_INV_ARG, E_NOT_OK)
    /* ArcCore req */
    VALIDATE_RV(IpAddrPtr!=NULL, SOAD_GET_REMOTE_ADDR_ID, SOAD_E_INV_ARG, E_NOT_OK)

    /** @req SWS_SOAD_00666 */
    if(ConnectionAdminList[SoConId].SocketConnectionRef->SoAdSocketRemoteAddress->Set==FALSE){
        return E_NOT_OK;
    }
    /** @req SWS_SoAd_00698 */
    if(IpAddrPtr->domain != ConnectionAdminList[SoConId].remAddrInUse.Domain){
        DET_REPORTERROR(SOAD_MODULE_ID, 0, SOAD_GET_REMOTE_ADDR_ID, SOAD_E_INV_ARG);
        return E_NOT_OK;
    }
    /** @req SWS_SOAD_00664 */
    for (uint32 i = 0; i < 4; i++) {
        IpAddrPtr->addr[i] = ConnectionAdminList[SoConId].remAddrInUse.SocketRemoteIpAddress[i];
    }
    IpAddrPtr->port = ConnectionAdminList[SoConId].remAddrInUse.SocketRemotePort;
    return E_OK;
}

#if ( SOAD_VERSION_INFO_API == STD_ON)
void SoAd_GetVersionInfo( Std_VersionInfoType* versioninfo ){

    VALIDATE_NO_RV( ( NULL != versioninfo ), SOAD_GET_VERSION_INFO_ID, SOAD_E_PARAM_POINTER);

    versioninfo->vendorID = SOAD_VENDOR_ID;
    versioninfo->moduleID = SOAD_MODULE_ID;
    versioninfo->sw_major_version = SOAD_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = SOAD_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = SOAD_SW_PATCH_VERSION;
}
#endif

Std_ReturnType SoAd_EnableRouting( SoAd_RoutingGroupIdType id ){
    /** @req SWS_SOAD_00624 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_ENABLE_ROUTING_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00625 */
    VALIDATE_RV(id<SOAD_NR_OF_ROUTING_GROUPS, SOAD_ENABLE_ROUTING_ID, SOAD_E_INV_ARG, E_NOT_OK)

#if SOAD_NR_OF_PDU_ROUTING_GROUP_CONNECTIONS > 0
    /*lint -save -e845 CONFIGURATION */
    for(uint32 i=0;i<SOAD_NR_OF_PDU_ROUTING_GROUP_CONNECTIONS;i++){
        if(PduRoutingGroupAdminList[i].RoutingGroup == id){
            PduRoutingGroupAdminList[i].Active = TRUE;
        }
    }
    /*lint -restore */
#endif
    for(uint32 i=0;i<SOAD_NR_OF_SOCKET_ROUTING_GROUP_CONNECTIONS;i++){
        if(SocketRoutingGroupAdminList[i].RoutingGroup == id){
            SocketRoutingGroupAdminList[i].Active = TRUE;
        }
    }

    return E_OK;
}

Std_ReturnType SoAd_EnableSpecificRouting( SoAd_RoutingGroupIdType id, SoAd_SoConIdType SoConId ){
    /** @req SWS_SOAD_00715 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_ENABLE_SPECIFIC_ROUTING_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00716 */
    VALIDATE_RV(id<SOAD_NR_OF_ROUTING_GROUPS, SOAD_ENABLE_SPECIFIC_ROUTING_ID, SOAD_E_INV_ARG, E_NOT_OK)
    /* Added by ArcCore */
    VALIDATE_RV(SoConId<SOAD_NR_OF_SOCKET_CONNECTIONS, SOAD_ENABLE_SPECIFIC_ROUTING_ID, SOAD_E_INV_ARG, E_NOT_OK)

#if SOAD_NR_OF_PDU_ROUTING_GROUP_CONNECTIONS > 0
    /*lint -save -e845 CONFIGURATION */
    for(uint32 i=0;i<SOAD_NR_OF_PDU_ROUTING_GROUP_CONNECTIONS;i++){
        if( (PduRoutingGroupAdminList[i].RoutingGroup == id) && (PduRoutingGroupAdminList[i].SoConId == SoConId) ){
            PduRoutingGroupAdminList[i].Active = TRUE;
        }
    }
    /*lint -restore */
#endif

    for(uint32 i=0;i<SOAD_NR_OF_SOCKET_ROUTING_GROUP_CONNECTIONS;i++){
        if( (SocketRoutingGroupAdminList[i].RoutingGroup == id) && (SocketRoutingGroupAdminList[i].SoConId == SoConId)){
            SocketRoutingGroupAdminList[i].Active = TRUE;
        }
    }
    return E_OK;
}

Std_ReturnType SoAd_DisableRouting( SoAd_RoutingGroupIdType id ){
    /** @req SWS_SOAD_00627 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_DISABLE_ROUTING_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00626 */
    VALIDATE_RV(id<SOAD_NR_OF_ROUTING_GROUPS, SOAD_DISABLE_ROUTING_ID, SOAD_E_INV_ARG, E_NOT_OK)

#if SOAD_NR_OF_PDU_ROUTING_GROUP_CONNECTIONS > 0
    /*lint -save -e845 CONFIGURATION */
    for(uint32 i=0;i<SOAD_NR_OF_PDU_ROUTING_GROUP_CONNECTIONS;i++){
        if(PduRoutingGroupAdminList[i].RoutingGroup == id){
            PduRoutingGroupAdminList[i].Active = FALSE;
        }
    }
    /*lint -restore */
#endif
    for(uint32 i=0;i<SOAD_NR_OF_SOCKET_ROUTING_GROUP_CONNECTIONS;i++){
        if(SocketRoutingGroupAdminList[i].RoutingGroup == id){
            SocketRoutingGroupAdminList[i].Active = FALSE;
        }
    }

    return E_OK;
}

Std_ReturnType SoAd_DisableSpecificRouting( SoAd_RoutingGroupIdType id, SoAd_SoConIdType SoConId ){
    /** @req SWS_SOAD_00718 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_DISABLE_SPECIFIC_ROUTING_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00719 */
    VALIDATE_RV(id<SOAD_NR_OF_ROUTING_GROUPS, SOAD_DISABLE_SPECIFIC_ROUTING_ID, SOAD_E_INV_ARG, E_NOT_OK)
    /* Added by ArcCore */
    VALIDATE_RV(SoConId<SOAD_NR_OF_SOCKET_CONNECTIONS, SOAD_DISABLE_SPECIFIC_ROUTING_ID, SOAD_E_INV_ARG, E_NOT_OK)

#if SOAD_NR_OF_PDU_ROUTING_GROUP_CONNECTIONS > 0
    /*lint -save -e845 CONFIGURATION */
    for(uint32 i=0;i<SOAD_NR_OF_PDU_ROUTING_GROUP_CONNECTIONS;i++){
        if( (PduRoutingGroupAdminList[i].RoutingGroup == id) && (PduRoutingGroupAdminList[i].SoConId == SoConId) ){
            PduRoutingGroupAdminList[i].Active = FALSE;
        }
    }
    /*lint -restore */
#endif
    for(uint32 i=0;i<SOAD_NR_OF_SOCKET_ROUTING_GROUP_CONNECTIONS;i++){
        if( (SocketRoutingGroupAdminList[i].RoutingGroup == id) && (SocketRoutingGroupAdminList[i].SoConId == SoConId)){
            SocketRoutingGroupAdminList[i].Active = FALSE;
        }
    }
    return E_OK;
}

Std_ReturnType SoAd_GetSoConId( PduIdType TxPduId, SoAd_SoConIdType* SoConIdPtr ){
    /** @req SWS_SOAD_00609 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_GET_SOCON_ID_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00610 */
    VALIDATE_RV(TxPduId<SOAD_NR_OF_PDU_ROUTES, SOAD_GET_SOCON_ID_ID, SOAD_E_INV_PDUID, E_NOT_OK)

    Std_ReturnType retVal = E_NOT_OK;
    const SoAd_PduRouteType* pduRoute = &(SoAdCfgPtr->PduRoute[TxPduId]);

    /** @req SWS_SOAD_00724 */
    if(pduRoute->NrOfPduRouteDest >1){
        return E_NOT_OK;
    }

    if(pduRoute->PduRouteDest[0].GroupUsed == FALSE){
        *SoConIdPtr = ConnectionAdminList[pduRoute->PduRouteDest[0].Index].SocketConnectionRef->SocketId;
        retVal = E_OK;
    }
    else{
        //IMPROVEMENT What to do if we have a group?
        retVal = E_NOT_OK;
    }

    return retVal;
}

Std_ReturnType SoAd_Arc_GetSoConIdFromRxPdu( PduIdType RxPduId, SoAd_SoConIdType* SoConIdPtr ){
    //Improvement: Guards

    Std_ReturnType retVal = E_NOT_OK;

    for(uint32 i=0;i<SOAD_NR_OF_SOCKET_ROUTES;i++){
        //IMPROVEMENT What to do if we have a group?
        if( (SoAdCfgPtr->SocketRoute[i].SocketRouteDest->RxPduRef == RxPduId) && (SoAdCfgPtr->SocketRoute[i].GroupUsed == FALSE)){
            retVal = E_OK;
            *SoConIdPtr =  ConnectionAdminList[SoAdCfgPtr->SocketRoute[i].Index].SocketConnectionRef->SocketId;
            break;
        }
    }

    return retVal;
}

/** @req SWS_SOAD_00520 */
Std_ReturnType SoAd_GetLocalAddr( SoAd_SoConIdType SoConId, TcpIp_SockAddrType* LocalAddrPtr, uint8* NetmaskPtr, TcpIp_SockAddrType* DefaultRouterPtr ){
    Std_ReturnType result;
    /** @req SWS_SOAD_00621 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_GET_LOCAL_ADDR_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00620 */
    VALIDATE_RV(SoConId<SOAD_NR_OF_SOCKET_CONNECTIONS, SOAD_GET_LOCAL_ADDR_ID, SOAD_E_INV_ARG, E_NOT_OK)

    result = TcpIp_GetIpAddr(ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SocketLocalAddressRef, LocalAddrPtr, NetmaskPtr, DefaultRouterPtr);
    LocalAddrPtr->port = ConnectionAdminList[SoConId].groupAdminRef->localPortUsed;
    return result;
}

Std_ReturnType SoAd_GetPhysAddr( SoAd_SoConIdType SoConId, uint8* PhysAddrPtr ){
    /** @req SWS_SOAD_00623 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_GET_PHYS_ADDR_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00622 */
    VALIDATE_RV(SoConId<SOAD_NR_OF_SOCKET_CONNECTIONS, SOAD_GET_PHYS_ADDR_ID, SOAD_E_INV_ARG, E_NOT_OK)

    return TcpIp_GetPhysAddr(ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SocketLocalAddressRef, PhysAddrPtr);
}

Std_ReturnType SoAd_RequestIpAddrAssignment( SoAd_SoConIdType SoConId, TcpIp_IpAddrAssignmentType Type, const TcpIp_SockAddrType* LocalIpAddrPtr, uint8 Netmask, const TcpIp_SockAddrType* DefaultRouterPtr ){
    /** @req SWS_SOAD_00613 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_REQUEST_IP_ADDR_ASSIGNMENT_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00617 */
    VALIDATE_RV(SoConId<SOAD_NR_OF_SOCKET_CONNECTIONS, SOAD_REQUEST_IP_ADDR_ASSIGNMENT_ID, SOAD_E_INV_ARG, E_NOT_OK)

    return TcpIp_RequestIpAddrAssignment( ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SocketLocalAddressRef, Type, LocalIpAddrPtr, Netmask, DefaultRouterPtr );
}

Std_ReturnType SoAd_ReleaseIpAddrAssignment( SoAd_SoConIdType SoConId ){
    /** @req SWS_SOAD_00618 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_RELEASE_IP_ADDR_ASSIGNMENT_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00619 */
    VALIDATE_RV(SoConId<SOAD_NR_OF_SOCKET_CONNECTIONS, SOAD_RELEASE_IP_ADDR_ASSIGNMENT_ID, SOAD_E_INV_ARG, E_NOT_OK)
    Std_ReturnType retVal = E_NOT_OK;
    retVal = TcpIp_ReleaseIpAddrAssignment( ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SocketLocalAddressRef );
    return retVal;
}

Std_ReturnType SoAd_SetUniqueRemoteAddr( SoAd_SoConIdType SoConId, const TcpIp_SockAddrType* RemoteAddrPtr, SoAd_SoConIdType* AssignedSoConIdPtr ){
    /** @req SWS_SOAD_00672 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_SET_UNIQUE_REMOTE_ADDR_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00673 */
    VALIDATE_RV(SoConId<SOAD_NR_OF_SOCKET_CONNECTIONS, SOAD_SET_UNIQUE_REMOTE_ADDR_ID, SOAD_E_INV_ARG, E_NOT_OK)

    /** @req SWS_SOAD_00678 */
    Std_ReturnType retVal = E_NOT_OK;
    /** @req SWS_SoAd_00700 */
    if(RemoteAddrPtr->domain != ConnectionAdminList[SoConId].remAddrInUse.Domain){
        DET_REPORTERROR(SOAD_MODULE_ID, 0, SOAD_SET_UNIQUE_REMOTE_ADDR_ID, SOAD_E_INV_ARG);
        return E_NOT_OK;
    }
    for(uint16 i=0;i<ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->NrOfSocketConnections;i++){
        uint16 connectionIndex = ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SoAdSocketConnection[i].SocketId;
        /** @req SWS_SOAD_00675 */
        if( (ConnectionAdminList[connectionIndex].remAddrInUse.Set == TRUE) &&
                    (ConnectionAdminList[connectionIndex].remAddrInUse.SocketRemoteIpAddress[0] == RemoteAddrPtr->addr[0]) &&
                    (ConnectionAdminList[connectionIndex].remAddrInUse.SocketRemoteIpAddress[1] == RemoteAddrPtr->addr[1]) &&
                    (ConnectionAdminList[connectionIndex].remAddrInUse.SocketRemoteIpAddress[2] == RemoteAddrPtr->addr[2]) &&
                    (ConnectionAdminList[connectionIndex].remAddrInUse.SocketRemoteIpAddress[3] == RemoteAddrPtr->addr[3]) &&
                    (ConnectionAdminList[connectionIndex].remAddrInUse.SocketRemotePort == RemoteAddrPtr->port)){
            *AssignedSoConIdPtr = connectionIndex;
            retVal = E_OK;
        }
    }
    if(retVal==E_NOT_OK){ // No SocketConnection found for the RemoteAddr given
        uint32 connIndexInGroup=0;
        /** @req SWS_SOAD_00676 */
        if(TRUE == runBestMatch(ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef, RemoteAddrPtr, &connIndexInGroup)){
            *AssignedSoConIdPtr = ConnectionAdminList[SoConId].groupAdminRef->SocketConnectionGroupRef->SoAdSocketConnection[connIndexInGroup].SocketId;
            // Assign address
            SchM_Enter_SoAd_EA_0();
            for(uint32 i=0;i<4;i++){
                ConnectionAdminList[*AssignedSoConIdPtr].remAddrInUse.SocketRemoteIpAddress[i] = RemoteAddrPtr->addr[i];
            }
            ConnectionAdminList[*AssignedSoConIdPtr].remAddrInUse.SocketRemotePort = RemoteAddrPtr->port;
            SchM_Exit_SoAd_EA_0();
            retVal=E_OK;
        }
    }
    return retVal;
}

Std_ReturnType SoAd_IfRoutingGroupTransmit( SoAd_RoutingGroupIdType id ){
    /** @req SWS_SOAD_00661 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_IF_ROUTING_GROUP_TRANSMIT_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00658 */
    VALIDATE_RV( (id < SOAD_NR_OF_ROUTING_GROUPS) && (SoAdCfgPtr->RoutingGroup[id].RoutingGroupTxTriggerable == TRUE), SOAD_IF_ROUTING_GROUP_TRANSMIT_ID, SOAD_E_INV_ARG, E_NOT_OK)

    /** @req SWS_SOAD_00662 */
    for(uint32 pduRouteIndex=0;pduRouteIndex<SOAD_NR_OF_PDU_ROUTES;pduRouteIndex++){
        for(uint32 pduRouteDestIndex=0;pduRouteDestIndex<SoAdCfgPtr->PduRoute[pduRouteIndex].NrOfPduRouteDest;pduRouteDestIndex++){
            for(uint32 routingGrpIndex=0;routingGrpIndex<SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].NrOfRoutingGroups;routingGrpIndex++){
                if( id == SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].TxRoutingGroupRef[routingGrpIndex]->RoutingGroupId){
                    PduRouteAdminList[pduRouteIndex].trigger.mode = SOAD_TRIGGERTRANSMITMODE_ROUTINGGROUP;
                    PduRouteAdminList[pduRouteIndex].trigger.RoutingGroup = id;
                }
            }
        }
    }
    return E_OK;
}

Std_ReturnType SoAd_IfSpecificRoutingGroupTransmit( SoAd_RoutingGroupIdType id, SoAd_SoConIdType SoConId ){
    /** @req SWS_SOAD_00712 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_IF_SPECIFIC_ROUTING_GROUP_TRANSMIT_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00713 */
    VALIDATE_RV( (id < SOAD_NR_OF_ROUTING_GROUPS) && (SoAdCfgPtr->RoutingGroup[id].RoutingGroupTxTriggerable == TRUE), SOAD_IF_SPECIFIC_ROUTING_GROUP_TRANSMIT_ID, SOAD_E_INV_ARG, E_NOT_OK)

    /** @req SWS_SOAD_00720 */
    for(uint32 pduRouteIndex=0;pduRouteIndex<SOAD_NR_OF_PDU_ROUTES;pduRouteIndex++){
        for(uint32 pduRouteDestIndex=0;pduRouteDestIndex<SoAdCfgPtr->PduRoute[pduRouteIndex].NrOfPduRouteDest;pduRouteDestIndex++){
            for(uint32 routingGrpIndex=0;routingGrpIndex<SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].NrOfRoutingGroups;routingGrpIndex++){
                if( id == SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].TxRoutingGroupRef[routingGrpIndex]->RoutingGroupId){
                    if(SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].GroupUsed == TRUE){
                        for(uint32 indexInGroup=0;indexInGroup<ConnectionGroupAdminList[SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].Index].SocketConnectionGroupRef->NrOfSocketConnections;indexInGroup++){
                            uint16 SocketId = ConnectionGroupAdminList[SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].Index].SocketConnectionGroupRef->SoAdSocketConnection[indexInGroup].SocketId;
                            if(SocketId == SoConId){
                                PduRouteAdminList[pduRouteIndex].trigger.mode = SOAD_TRIGGERTRANSMITMODE_ROUTINGGROUP_AND_SOCON;
                                PduRouteAdminList[pduRouteIndex].trigger.RoutingGroup = id;
                                PduRouteAdminList[pduRouteIndex].trigger.SoConId = SoConId;
                            }
                        }
                    }
                    else{
                        if(SoAdCfgPtr->PduRoute[pduRouteIndex].PduRouteDest[pduRouteDestIndex].Index == SoConId){
                            PduRouteAdminList[pduRouteIndex].trigger.mode = SOAD_TRIGGERTRANSMITMODE_ROUTINGGROUP_AND_SOCON;
                            PduRouteAdminList[pduRouteIndex].trigger.RoutingGroup = id;
                            PduRouteAdminList[pduRouteIndex].trigger.SoConId = SoConId;
                        }
                    }
                }
            }
        }
    }
    return E_OK;
}


Std_ReturnType SoAd_TpTransmit( PduIdType SoAdSrcPduId, const PduInfoType* SoAdSrcPduInfoPtr ){

    SoadTptransmitType *tpRunTimeParm;
    Std_ReturnType retVal;

    /* @req SWS_SOAD_00224 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_TP_TRANSMIT_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /* @req SWS_SOAD_00237 */
    VALIDATE_RV(SoAdSrcPduId < SOAD_NR_OF_TX_PDUS, SOAD_TP_TRANSMIT_ID, SOAD_E_INV_PDUID, E_NOT_OK)

    tpRunTimeParm = &SoadTpTransStatus[SoAdSrcPduId];

    /* @req SWS_SOAD_00551 */
    if(SoAdSrcPduInfoPtr->SduLength == 0){
        retVal = E_NOT_OK;
    } else if (SOAD_RX_UPPER_LAYER_TP != SoAdCfgPtr->PduRoute[SoAdSrcPduId].TxUpperLayerType) {
        retVal = E_NOT_OK; /* Reject if PDU route is not Tp type */
    } else if (TRUE == tpRunTimeParm->tpTxRequest) {
        /* Reject request for ongoing Tp transmission case */
        /* @req SWS_SOAD_00650 */
        retVal = E_NOT_OK;
    }
    else if ((SoAdCfgPtr->PduRoute[SoAdSrcPduId].NrOfPduRouteDest > 1) ||
            (SoAdCfgPtr->PduRoute[SoAdSrcPduId]. \
                    PduRouteDest[SOAD_PDU_ROUTE_FIRST_DESTINATION_IDX].GroupUsed == TRUE)){
        /* Discard Tp transmit on a route which contains multiple destinations. In case
         * of only one destination the first destination should map to a socket connection */
        retVal = E_NOT_OK;
    } else {
        tpRunTimeParm->tpTxRequest = TRUE;
        tpRunTimeParm->firstCopyCalled = FALSE;
        retVal = E_OK;
    }

    return retVal;
}

// ----------------- UNIMPLEMENTED FUNCTIONS ----------------------------------
Std_ReturnType SoAd_TpCancelTransmit( PduIdType PduId ){

    //SoAd_TpCancelStatus.SoAd_TpCancelTranStatus = FALSE;
    /** @req SWS_SOAD_00605 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_TP_CANCEL_TRANSMIT_ID, SOAD_E_NOTINIT, E_NOT_OK)


    return E_OK;
}

Std_ReturnType SoAd_TpCancelReceive( PduIdType PduId ){
    /** @req SWS_SOAD_00607 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_TP_CANCEL_RECEIVE_ID, SOAD_E_NOTINIT, E_NOT_OK)
    return E_OK;
}

Std_ReturnType SoAd_TpChangeParameter( PduIdType id, TPParameterType parameter, uint16 value ){
    /** @req SWS_SOAD_00629 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_TP_CHANGE_PARAMETER_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00630 */
    VALIDATE_RV(parameter<=TP_BC, SOAD_TP_CHANGE_PARAMETER_ID, SOAD_E_INV_ARG, E_NOT_OK)
    /** @req SWS_SOAD_00631 */
    VALIDATE_RV(id<SOAD_NR_OF_PDU_ROUTES, SOAD_TP_CHANGE_PARAMETER_ID, SOAD_E_INV_ARG, E_NOT_OK)

    return E_OK;
}

Std_ReturnType SoAd_ReadDhcpHostNameOption( SoAd_SoConIdType SoConId, uint8* length, uint8* data ){
    /** @req SWS_SOAD_00701 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_READ_DHCP_HOST_NAME_OPTION_ID, SOAD_E_NOTINIT, E_NOT_OK)
    /** @req SWS_SOAD_00702 */
    VALIDATE_RV(SoConId<SOAD_NR_OF_SOCKET_CONNECTIONS, SOAD_READ_DHCP_HOST_NAME_OPTION_ID, SOAD_E_INV_ARG, E_NOT_OK)

    return E_OK;
}

Std_ReturnType SoAd_WriteDhcpHostNameOption( SoAd_SoConIdType SoConId, uint8 length, const uint8* data ){
    /** @req SWS_SOAD_00705 */
    VALIDATE_RV(SoAd_ModuleStatus==SOAD_INITIALIZED, SOAD_WRITE_DHCP_HOST_NAME_OPTION_ID, SOAD_E_NOTINIT, E_NOT_OK)

    (void)length;


    return E_OK;
}

#if defined(HOST_TEST)
/**
 * Get the mode for a specific Socket Connection. Only used for testing purposes
 */
SoAd_SoConModeType SoAd_Arc_GetMode(SoAd_SoConIdType SoConId){
    return ConnectionAdminList[SoConId].mode;
}
#endif
