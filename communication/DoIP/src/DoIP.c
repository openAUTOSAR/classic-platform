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
#include "DoIP.h"

#include "DoIP_Cfg.h"
#include "DoIP_Cbk.h"
#include "DoIP_Types.h"
#include "DoIP_Internal.h"
#include "DoIP_MemMap.h"
#if defined(USE_RTE)
#include "Rte_DoIP.h"
#endif
#if (DOIP_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif
#include "SoAd.h"
#include "PduR_DoIP.h"
#include "SchM_DoIP.h"
#include <string.h>

//lint -emacro(904,VALIDATE) MISRA:OTHER:Readiability:[MISRA 2004 Rule 14.7, required]
//lint -emacro(904,VALIDATE_W_RV) MISRA:OTHER:Readiability:[MISRA 2004 Rule 14.7, required]
#if (DOIP_DEV_ERROR_DETECT == STD_ON)
    
#define VALIDATE(_exp, _api, _err ) \
    if( !(_exp) ) {\
        (void)Det_ReportError(DOIP_MODULE_ID, 0, _api, _err); \
        return; \
    }

#define VALIDATE_W_RV(_exp, _api, _err, _rv ) \
    if( !(_exp) ) { \
        (void)Det_ReportError(DOIP_MODULE_ID, 0, _api, _err); \
        return (_rv); \
    }

#define DOIP_DET_REPORTERROR(_api, _err) (void)Det_ReportError(DOIP_MODULE_ID, 0, _api, _err)
    
#else

#define VALIDATE(_exp, _api, _err ) \
    if( !(_exp) ) {\
        return; \
    }

#define VALIDATE_W_RV(_exp, _api, _err, _rv ) \
    if( !(_exp) ) { \
        return (_rv); \
    }
    
#define DOIP_DET_REPORTERROR(_api, _err)
#endif


/***** Macro - Definition *****/
/* Macro to get the payload length from the DoIP message */
/** @req SWS_DoIP_000010 */
#define GET_PL_LEN_FROM_DOIP_MSG_PTR(_rxBuf)    (((DoIPPayloadType)_rxBuf[PL_LEN_INDEX] << SHIFT_BY_THREE_BYTES) | \
                                                ((DoIPPayloadType)_rxBuf[PL_LEN_INDEX+1u] << SHIFT_BY_TW0_BYTES) | \
                                                ((DoIPPayloadType)_rxBuf[PL_LEN_INDEX+2u] << SHIFT_BY_ONE_BYTE) | \
                                                ((DoIPPayloadType)_rxBuf[PL_LEN_INDEX+3u]))

/** @req SWS_DoIP_00007 */
#define GET_PL_TYPE_FROM_DOIP_MSG_PTR(_rxBuf)   (((DoIPPayloadType)_rxBuf[PL_TYPE_INDEX] << SHIFT_BY_ONE_BYTE) | \
                                                ((DoIPPayloadType)_rxBuf[PL_TYPE_INDEX+1u]))

#define GET_SA_FROM_DOIP_MSG_PTR(_rxBuf)        (((uint16)_rxBuf[SA_INDEX] << SHIFT_BY_ONE_BYTE) | \
                                                ((uint16)_rxBuf[SA_INDEX+1u]))

#define GET_TA_FROM_DOIP_MSG_PTR(_rxBuf)        (((uint16)_rxBuf[TA_INDEX] << SHIFT_BY_ONE_BYTE) | \
                                                ((uint16)_rxBuf[TA_INDEX+1u]))

#define GET_ACT_TYPE_FROM_DOIP_MSG_PTR(_rxBuf)  ((uint8)_rxBuf[ROUT_ACTIV_TYPE_INDEX])


/***** Local variables *****/                                                
static DoIP_Internal_TcpConRxBufAdType DoIP_TcpConRxBufferAdmin;
static DoIP_Internal_TcpConAdminType DoIP_TcpConAdmin[DOIP_TCP_CON_NUM];
static DoIP_Internal_TcpQueueAdminType DoIP_TcpQueueAdmin[DOIP_TCP_CON_NUM];
static DoIP_Internal_UdpConAdminType DoIP_UdpConAdmin[DOIP_UDP_CON_NUM];
static DoIP_Internal_StatusType DoIP_Status = DOIP_UNINIT;
static uint32 DoIP_TcpRxRemainingBufferSize;

/** @req SWS_DoIP_00201 */
static DoIP_Internal_ActnLineStsType DoIP_ActivationLineStatus;
static const DoIP_ConfigType* DoIP_ConfigPtr = NULL;

static uint8 DoIP_Vin[PL_LEN_VID_VIN_REQ_RES];
static boolean DoIP_VinUpdated;
static uint8 DoIP_Eid[PL_LEN_VID_EID_REQ_RES];
static boolean DoIP_EidUpdated;

static boolean DoIP_PendingRoutActivFlag;
static uint16 DoIP_PendingRoutActivConIndex;
static uint16 DoIP_PendingRoutActivSa;
static uint16 DoIP_pendingRoutActivActType;


#if (DOIPTGRGIDSYNCCALLBACK_CONFIGURED == TRUE) && (DOIP_VIN_GID_MASTER == STD_ON)
static boolean DoIP_GidSyncDone = FALSE;
#endif
    
/***** Local function - Prototypes *****/
static void freeUdpTxBuffer(SoAd_SoConIdType conIndex);
static void freeUdpRxBuffer(SoAd_SoConIdType conIndex);
static void freeTcpTxBuffer(SoAd_SoConIdType conIndex);
static void freeTcpRxBuffer(void);

static void resetUdpConnection(SoAd_SoConIdType conIndex);
static void resetTcpConnection(SoAd_SoConIdType conIndex);

static Std_ReturnType getVin(uint8* bufferPtr);
static Std_ReturnType getEid(uint8* bufferPtr, SoAd_SoConIdType conIndex);
static void getGid(uint8* txBufferPtr, SoAd_SoConIdType conIndex);

static uint8 getChIndexFromTargetIndex(uint8 targetIndex);
static Std_ReturnType getConTypeConIndexFromSoConId(SoAd_SoConIdType SoConId, DoIP_Internal_ConType* conTypePtr, uint16* conIndexPtr);
static uint8 getNofCurrentlyUsedTcpSockets(void);
static void getFurtherActionRequired(uint8* txBufferPtr);

static boolean isSourceAddressKnown(uint16 sa, uint8 *testerIndex);
static boolean isRoutingTypeSupported(uint16 activationType, uint8 testerIndex, uint16* routActRefIndex);
#if 0
/* Routing and confirmation callbacks not supported */ 
static boolean isAuthenticationRequired(uint16 routActRefIndex);
static boolean isAuthenticated(uint16 routActRefIndex);
static boolean isConfirmationRequired(uint16 routActRefIndex);
static boolean isConfirmed(uint16 routActRefIndex);
#endif
static void registerSocket(SoAd_SoConIdType conIndex, uint16 sa, uint16 activationType);
static void closeSocket(DoIP_Internal_ConType conType, SoAd_SoConIdType conIndex, uint16 txPdu);

static void handleTimeout(SoAd_SoConIdType conIndex);

static void updateRemoteAddrAsBroadcast(SoAd_SoConIdType conIndex);
static void createAndSendNackIf(SoAd_SoConIdType conIndex, uint8 nackCode);
static void createAndSendNackTp(SoAd_SoConIdType conIndex, uint8 nackCode);
static void createAndSendDiagnosticAck(SoAd_SoConIdType conIndex, uint16 sa, uint16 ta);
static void createAndSendDiagnosticNack(SoAd_SoConIdType conIndex, uint16 sa, uint16 ta, uint8 nackCode);
static void createVehicleIdentificationResponse(uint8* txBuffer, SoAd_SoConIdType conIndex);
static void sendVehicleAnnouncement(SoAd_SoConIdType conIndex);
static void fillRoutingActivationResponseData(uint8 *txBuffer, uint16 sa, uint8 routingActivationResponseCode);

static void createAndSendAliveCheck(SoAd_SoConIdType conIndex);
static void startSingleSaAliveCheck(SoAd_SoConIdType conIndex);
static void startAllSocketsAliveCheck(void);

static DoIP_Internal_LookupResType lookupSaTa(SoAd_SoConIdType conIndex, uint16 sa, uint16 ta, uint8* targetIndex);
static DoIP_Internal_SockAssigResType socketHandle(SoAd_SoConIdType conIndex, uint16 activationType, uint16 sa, uint8* routingActivationResponseCode);

static void handleAliveCheckResp(SoAd_SoConIdType conIndex, const uint8* rxBuffer);
static void handleRoutingActivationReq(SoAd_SoConIdType conIndex, const uint8 *rxBuffer);
static void handleDiagnosticMessage(SoAd_SoConIdType conIndex, uint8 *rxBuffer);
static void handleVehicleIdentificationReq(SoAd_SoConIdType conIndex, const uint8 *rxBuffer, DoIP_Internal_VehReqType type);
static void handleEntityStatusReq(SoAd_SoConIdType conIndex,const uint8 *rxBuffer);
static void handlePowerModeCheckReq(SoAd_SoConIdType conIndex, const uint8 *rxBuffer);

static void handleTcpRx(SoAd_SoConIdType conIndex, uint8* rxBuffer);
static void handleUdpRx(SoAd_SoConIdType conIndex,const uint8* rxBuffer);

/***** Local function - Definition *****/
static void freeUdpTxBuffer(SoAd_SoConIdType conIndex) {
    DoIP_UdpConAdmin[conIndex].txBufferState = BUFFER_IDLE;
}

static void freeUdpRxBuffer(SoAd_SoConIdType conIndex) {
    DoIP_UdpConAdmin[conIndex].rxBufferStartIndex = UDP_RX_BUFFER_RESET_INDEX;
    DoIP_UdpConAdmin[conIndex].rxBufferEndIndex = UDP_RX_BUFFER_RESET_INDEX;
    DoIP_UdpConAdmin[conIndex].rxBufferPresent = FALSE;
    DoIP_UdpConAdmin[conIndex].rxPduIdUnderProgress = INVALID_PDU_ID;    
}

static void freeTcpTxBuffer(SoAd_SoConIdType conIndex) {
    DoIP_TcpConAdmin[conIndex].txBufferState = BUFFER_IDLE;
    DoIP_TcpConAdmin[conIndex].txBytesToTransmit = 0u;
    DoIP_TcpConAdmin[conIndex].txBytesCopied = 0u;
    DoIP_TcpConAdmin[conIndex].txBytesTransmitted = 0u;
    DoIP_TcpConAdmin[conIndex].txPduIdUnderProgress = INVALID_PDU_ID;    
}   

static void freeTcpRxBuffer(void) {
    DoIP_TcpConRxBufferAdmin.bufferState = BUFFER_IDLE;
    DoIP_TcpConRxBufferAdmin.pduIdUnderProgress = INVALID_PDU_ID;     
}   

static void resetUdpConnection(SoAd_SoConIdType conIndex) {
    DoIP_UdpConAdmin[conIndex].sockNr = INVALID_SOCKET_NUMBER;
    DoIP_UdpConAdmin[conIndex].socketState = CONNECTION_INVALID;

    DoIP_UdpConAdmin[conIndex].vehicleAnnounceTgr = FALSE;
    DoIP_UdpConAdmin[conIndex].vehicleAnnounceInProgress = FALSE;
    DoIP_UdpConAdmin[conIndex].vehicleAnnounceInitialTime = 0u;
    DoIP_UdpConAdmin[conIndex].vehicleAnnounceTimeInterval = 0u;
    DoIP_UdpConAdmin[conIndex].vehicleAnnounceRepetition = 0u;

    DoIP_UdpConAdmin[conIndex].txBufferState = BUFFER_IDLE;
    
    DoIP_UdpConAdmin[conIndex].rxBufferStartIndex = UDP_RX_BUFFER_RESET_INDEX;
    DoIP_UdpConAdmin[conIndex].rxBufferEndIndex = UDP_RX_BUFFER_RESET_INDEX;
    DoIP_UdpConAdmin[conIndex].rxBufferPresent = FALSE;
    DoIP_UdpConAdmin[conIndex].rxPduIdUnderProgress = INVALID_PDU_ID;
}

static void resetTcpConnection(SoAd_SoConIdType conIndex) {
    DoIP_TcpConAdmin[conIndex].sockNr = INVALID_SOCKET_NUMBER;
    DoIP_TcpConAdmin[conIndex].socketState = CONNECTION_INVALID;
    DoIP_TcpConAdmin[conIndex].sa = INVALID_SOURCE_ADDRESS;
    DoIP_TcpConAdmin[conIndex].generalInactivityTimer = 0u;
    DoIP_TcpConAdmin[conIndex].initialInactivityTimer = 0u;
    DoIP_TcpConAdmin[conIndex].aliveCheckTimer = 0u;
    DoIP_TcpConAdmin[conIndex].authenticated = FALSE;
    DoIP_TcpConAdmin[conIndex].confirmed = FALSE;
    DoIP_TcpConAdmin[conIndex].awaitingAliveCheckResponse = FALSE;
    DoIP_TcpConAdmin[conIndex].closeSocketIndication = FALSE;
    DoIP_TcpConAdmin[conIndex].uLMsgTxInProgress = FALSE;
    DoIP_TcpConAdmin[conIndex].txBufferState = BUFFER_IDLE;
    DoIP_TcpConAdmin[conIndex].txBytesToTransmit = 0u;
    DoIP_TcpConAdmin[conIndex].txBytesCopied = 0u;
    DoIP_TcpConAdmin[conIndex].txBytesTransmitted = 0u;
    DoIP_TcpConAdmin[conIndex].txPduIdUnderProgress = INVALID_PDU_ID;
    DoIP_TcpConAdmin[conIndex].channelIndex = INVALID_CHANNEL_INDEX;
    
    DoIP_TcpQueueAdmin[conIndex].diagAckQueueActive = FALSE;
    DoIP_TcpQueueAdmin[conIndex].tpTransmitQueueActive = FALSE;
}

static Std_ReturnType getVin(uint8* bufferPtr) {
    Std_ReturnType retVal;
    uint8 i;
    /** @req SWS_DoIP_00070 */ /** @req SWS_DoIP_00072 */
    /* Dcm_GetVin to be called from the DoIP_Arc_GetVin() */
    retVal = DoIP_Arc_GetVin(bufferPtr, PL_LEN_VID_VIN_REQ_RES);
    if (E_OK != retVal) {
        for (i = 0u; i < PL_LEN_VID_VIN_REQ_RES; i++) {
        	bufferPtr[i] = DOIP_VIN_INVALIDITY_PATTERN;
        }
    }
    return retVal;
}

static Std_ReturnType getEid(uint8* bufferPtr, SoAd_SoConIdType conIndex) {
    Std_ReturnType retVal;

#if (DOIP_USE_MAC_ADD_FOR_ID == STD_ON)    
	PduIdType soadTxPduId;
    SoAd_SoConIdType SoConId;
    uint8 phyAddrPtr[PL_LEN_VID_EID_REQ_RES];
    uint8 i;
    soadTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpSoADTxPduRef;
    (void)SoAd_GetSoConId(soadTxPduId, &SoConId);
    
    /** @req SWS_DoIP_00065 */ /** @req SWS_DoIP_00074 */
    if (E_OK == SoAd_GetPhysAddr(SoConId, phyAddrPtr)) {
        bufferPtr[0] = phyAddrPtr[0];
        bufferPtr[1] = phyAddrPtr[1];
        bufferPtr[2] = phyAddrPtr[2];
        bufferPtr[3] = phyAddrPtr[3];
        bufferPtr[4] = phyAddrPtr[4];
        bufferPtr[5] = phyAddrPtr[5];
        
        retVal = E_OK;
    } else {
        for (i = 0u; i < PL_LEN_VID_EID_REQ_RES; i++) {
            bufferPtr[19u+i] = 0xEE;
        }
        DOIP_DET_REPORTERROR(DOIP_GET_EID_ID, DOIP_E_UNEXPECTED_EXECUTION);
        retVal = E_NOT_OK;
    }
#else    
    (void)conIndex;
/** @req SWS_DoIP_00075 */
#if (DOIP_EID_CONFIGURED == TRUE)
    /*lint -e{572} MISRA:FALSE_POSITIVE:Inspite of keeping the define DOIP_EID with suffix uLL PC lint throws this warning Excessive shift value (precision 33 shifted right by 40):[MISRA 2004 Rule 12.8, required] */
    /*lint -e{778} MISRA:FALSE_POSITIVE::False positive info - Constant expression evaluates to 0 in operation '>>' */
    bufferPtr[0] = (uint8)(DOIP_EID >> SHIFT_BY_FIVE_BYTES);
    bufferPtr[1] = (uint8)(DOIP_EID >> SHIFT_BY_FOUR_BYTES);
    bufferPtr[2] = (uint8)(DOIP_EID >> SHIFT_BY_THREE_BYTES);
    bufferPtr[3] = (uint8)(DOIP_EID >> SHIFT_BY_TW0_BYTES);
    bufferPtr[4] = (uint8)(DOIP_EID >> SHIFT_BY_ONE_BYTE);
    bufferPtr[5] = (uint8)(DOIP_EID);
    
    retVal = E_OK;
#else 
#error "EID is not configured"
#endif

#endif    

    return retVal;
}

static void getGid(uint8* bufferPtr, SoAd_SoConIdType conIndex) {

#if (DOIP_USE_EID_AS_GID == TRUE) 
    uint8 i;
    /** @req SWS_DoIP_00077 */
    if (E_OK != getEid(bufferPtr, conIndex)) {
        for (i = 0u; i < PL_LEN_VID_EID_REQ_RES; i++) {
            bufferPtr[25u+i] = 0xEE;
        }
    }
#else
    (void)conIndex;
#if (DOIP_GID_CONFIGURED == TRUE) 
    /** @req SWS_DoIP_00078 */
    /*lint -e{572} MISRA:FALSE_POSITIVE:Inspite of keeping the define DOIP_EID with suffix uLL PC lint throws this warning Excessive shift value (precision 33 shifted right by 40):[MISRA 2004 Rule 12.8, required] */
    /*lint -e{778} MISRA:FALSE_POSITIVE::False positive info - Constant expression evaluates to 0 in operation '>>' */
    bufferPtr[0] = (uint8)(DOIP_GID >> SHIFT_BY_FIVE_BYTES);
    bufferPtr[1] = (uint8)(DOIP_GID >> SHIFT_BY_FOUR_BYTES);
    bufferPtr[2] = (uint8)(DOIP_GID >> SHIFT_BY_THREE_BYTES);
    bufferPtr[3] = (uint8)(DOIP_GID >> SHIFT_BY_TW0_BYTES);
    bufferPtr[4] = (uint8)(DOIP_GID >> SHIFT_BY_ONE_BYTE);
    bufferPtr[5] = (uint8)(DOIP_GID);
#else
    uint8 i;
    /** @req SWS_DoIP_00079 */ /** @req SWS_DoIP_00080 */
    if (E_OK != DOIP_GETGIDCALLBACK_FUNCTION()) {
        for (i = 0u; i < PL_LEN_GID; i++) {
            /** @req SWS_DoIP_00081 */
            bufferPtr[i] = DOIP_GID_INVALIDITY_PATTERN;
        }
    }
#endif    
#endif
}

static uint8 getChIndexFromTargetIndex(uint8 targetIndex) {
    uint8 chIndex;
    uint8 retVal;

    retVal = INVALID_CHANNEL_INDEX;
    for (chIndex = 0u; chIndex < DOIP_CHANNEL_COUNT; chIndex++) {
        if (DoIP_ConfigPtr->DoIP_Channel[chIndex].DoIP_ChannelTARef->DoIP_TargetAddrValue == DoIP_ConfigPtr->DoIP_TargetAddress[targetIndex].DoIP_TargetAddrValue) {
            retVal = chIndex;
            break;
        }
    } 

    return retVal;
}

static Std_ReturnType getConTypeConIndexFromSoConId(SoAd_SoConIdType SoConId, DoIP_Internal_ConType* conTypePtr, uint16* conIndexPtr) {
    SoAd_SoConIdType conIndex;
    Std_ReturnType retVal;
    SoAd_SoConIdType tempSoConId;
    boolean found;

    found = FALSE;
    retVal = E_NOT_OK;
    for (conIndex = 0u; conIndex < DOIP_TCP_CON_NUM; conIndex++) {
        (void)SoAd_GetSoConId(DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpSoADTxPduRef, &tempSoConId);
        if (tempSoConId == SoConId) {
            found = TRUE;
            *conTypePtr = TCP_TYPE;
            *conIndexPtr = conIndex;
            retVal = E_OK;            
            break;
        }
    }
    
    if (FALSE == found) {
        for (conIndex = 0u; conIndex < DOIP_UDP_CON_NUM; conIndex++) {
            (void)SoAd_GetSoConId(DoIP_ConfigPtr->DoIP_UdpMsg[conIndex].DoIP_UdpSoADTxPduRef, &tempSoConId);
            if (tempSoConId == SoConId) {
                found = TRUE;
                *conTypePtr = UDP_TYPE;
                *conIndexPtr = conIndex;
                retVal = E_OK;                
                break;
            }
        }
        
        if (FALSE == found) {
            retVal = E_NOT_OK;
        }
    }
    return retVal;
}
 
static uint8 getNofCurrentlyUsedTcpSockets(void) {
    SoAd_SoConIdType conIndex;
    uint8 openSocketCount = 0u;
    
    for (conIndex = 0u; conIndex < DOIP_TCP_CON_NUM; conIndex++) {
        if (DoIP_TcpConAdmin[conIndex].socketState == CONNECTION_REGISTERED) {
            openSocketCount++;
        }
    }  
    return openSocketCount;
}

/** @req SWS_DoIP_00082 */ /** @req SWS_DoIP_00083 */ /** @req SWS_DoIP_00084 */
static void getFurtherActionRequired(uint8* txBufferPtr) {
    SoAd_SoConIdType conIndex;
    uint8 i;
    
    for (i = 0; i < DOIP_NUM_ROUTING_ACTIVATION; i++) {
        if (0xE0 == DoIP_ConfigPtr->DoIP_RoutingActivation[i].DoIP_RoutActNum) {
            break;
        }
    }
    
    if (i != DOIP_NUM_ROUTING_ACTIVATION) {
        for (conIndex = 0u; conIndex < DOIP_TCP_CON_NUM; conIndex++) {
            if (CONNECTION_REGISTERED == DoIP_TcpConAdmin[conIndex].socketState) {
                if (0xE0 == DoIP_TcpConAdmin[conIndex].activationType) {
                    *txBufferPtr = 0x00;
                    break;
                } else {
                    *txBufferPtr = 0x10;
                }
            }
        }
    } else {
        *txBufferPtr = 0x00;
    }
}

static boolean isSourceAddressKnown(uint16 sa, uint8 *testerIndex) {
    boolean saKnown;
    uint8 i;

    saKnown = FALSE;
    for (i = 0u; (i < DOIP_TESTER_COUNT); i++) {
        if (DoIP_ConfigPtr->DoIP_Tester[i].DoIP_TesterSA == sa) {
            saKnown = TRUE;
            *testerIndex =i;
            break; 
        }
    }
    
    return saKnown;
}

static boolean isRoutingTypeSupported(uint16 activationType, uint8 testerIndex, uint16* routActRefIndex) {
    boolean supported;
    uint8 i;

    supported = FALSE;
    for (i = 0u; i < DoIP_ConfigPtr->DoIP_Tester[testerIndex].DoIp_RoutRefSize; i++) {
        if (DoIP_ConfigPtr->DoIP_Tester[testerIndex].DoIP_RoutActRef[i]->DoIP_RoutActNum == activationType) {
            supported = TRUE;
            *routActRefIndex = i;
        }
    }

    return supported;
}

#if 0
/* Routing and confirmation callbacks not supported */ 
static boolean isAuthenticationRequired(uint16 routActRefIndex) {
    boolean required;

    required = FALSE;
    if (NULL == DoIP_ConfigPtr->DoIP_RoutingActivation[routActRefIndex].DoIP_RoutActAuthRef) {
        required = FALSE;
    } else {
        required = TRUE;
    }            
    return required;
}

static boolean isAuthenticated(uint16 routActRefIndex) {
    return E_OK;
}

static boolean isConfirmationRequired(uint16 routActRefIndex) {
    boolean required;

    required = FALSE;
    if (NULL == DoIP_ConfigPtr->DoIP_RoutingActivation[routActRefIndex].DoIP_RoutActConfRef) {
        required = FALSE;
    } else {
        required = TRUE;
    }            
    return required;
}

static boolean isConfirmed(uint16 routActRefIndex) {
    return E_OK;
}
#endif

static void registerSocket(SoAd_SoConIdType conIndex, uint16 sa, uint16 activationType) {
    DoIP_TcpConAdmin[conIndex].socketState = CONNECTION_REGISTERED;
    DoIP_TcpConAdmin[conIndex].sa = sa;
    DoIP_TcpConAdmin[conIndex].activationType = activationType;
    DoIP_TcpConAdmin[conIndex].generalInactivityTimer = 0u;
    DoIP_TcpConAdmin[conIndex].initialInactivityTimer = 0u;
    DoIP_TcpConAdmin[conIndex].aliveCheckTimer = 0u;
    DoIP_TcpConAdmin[conIndex].authenticated = FALSE;
    DoIP_TcpConAdmin[conIndex].confirmed = FALSE;
    DoIP_TcpConAdmin[conIndex].awaitingAliveCheckResponse = FALSE;
}

static void closeSocket(DoIP_Internal_ConType conType, SoAd_SoConIdType conIndex, uint16 txPdu) {
    SoAd_SoConIdType SoConId;
    Std_ReturnType ret;
    
    if (TCP_TYPE == conType) {
        DoIP_TcpConAdmin[conIndex].closeSocketIndication = TRUE;
    } else {
        /** @req SWS_DoIP_00058 */
        ret = SoAd_GetSoConId(txPdu, &SoConId);
        if (E_OK == ret) {
            (void)SoAd_CloseSoCon(SoConId, TRUE);
            /** @req SWS_DoIP_00115 */
            resetTcpConnection(conIndex);
        }
    }

}

static void updateRemoteAddrAsBroadcast(SoAd_SoConIdType conIndex) {
    TcpIp_SockAddrType ad;
    ad.domain = TCPIP_AF_INET;
    (void)SoAd_GetRemoteAddr(DoIP_UdpConAdmin[conIndex].sockNr, &ad);
    ad.addr[0u] = 255u;
    ad.addr[1u] = 255u;
    ad.addr[2u] = 255u;
    ad.addr[3u] = 255u;
    ad.port = DOIP_PORT_NUM;
    (void)SoAd_SetRemoteAddr(DoIP_UdpConAdmin[conIndex].sockNr, &ad);
}

static void handleTimeout(SoAd_SoConIdType conIndex) {
    PduIdType soadTxPduId;
    PduInfoType txPduInfo;
    PduIdType doipLoTxPduId;
    Std_ReturnType ret;
    uint8 routingActivationResponseCode;
    
    soadTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpTxPduRef;

    if (TRUE == DoIP_PendingRoutActivFlag) {

        registerSocket(conIndex, DoIP_PendingRoutActivSa, DoIP_pendingRoutActivActType);

        soadTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[DoIP_PendingRoutActivConIndex].DoIP_TcpSoADTxPduRef;
        txPduInfo.SduLength = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_ROUT_ACTIV_RES;
        
        ret = SoAd_TpTransmit(soadTxPduId, &txPduInfo);
        
        if (E_OK == ret) {
            doipLoTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[DoIP_PendingRoutActivConIndex].DoIP_TcpTxPduRef;

            /* Routing successfully activated */
            routingActivationResponseCode = 0x10u;
            fillRoutingActivationResponseData(DoIP_TcpConAdmin[DoIP_PendingRoutActivConIndex].txBuffer, DoIP_PendingRoutActivSa, routingActivationResponseCode);
            DoIP_TcpConAdmin[DoIP_PendingRoutActivConIndex].txBytesToTransmit = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_ROUT_ACTIV_RES;
            DoIP_TcpConAdmin[DoIP_PendingRoutActivConIndex].txBytesCopied = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_ROUT_ACTIV_RES;
            DoIP_TcpConAdmin[DoIP_PendingRoutActivConIndex].txBytesTransmitted = 0u;
            DoIP_TcpConAdmin[DoIP_PendingRoutActivConIndex].txPduIdUnderProgress = doipLoTxPduId;
            DoIP_TcpConAdmin[DoIP_PendingRoutActivConIndex].txBufferState = BUFFER_LOCK;
        } else {
            DOIP_DET_REPORTERROR(DOIP_HANDLE_TIMEOUT_ID, DOIP_E_TP_TRANSMIT_ERROR);
        }
        DoIP_PendingRoutActivFlag = FALSE;
            
    } else {
        /* Close current connection (which has timed out anyway)
           and register a new connection with the pending routing activation */
        closeSocket(TCP_FORCE_CLOSE_TYPE, conIndex, soadTxPduId);
    }
}

/** @req SWS_DoIP_00009 */
static void createAndSendNackIf(SoAd_SoConIdType conIndex, uint8 nackCode) {
    PduInfoType txPduInfo;
    PduIdType soadTxPduId;
    Std_ReturnType ret;
    
    if (BUFFER_IDLE == DoIP_UdpConAdmin[conIndex].txBufferState) {
        /** @req SWS_DoIP_00004 */ /** @req SWS_DoIP_00005 */ /** @req SWS_DoIP_00006 */
        DoIP_UdpConAdmin[conIndex].txBuffer[0u] = PROTOCOL_VERSION;
        DoIP_UdpConAdmin[conIndex].txBuffer[1u] = ~(uint8)PROTOCOL_VERSION;
        /*lint -e9053 MISRA:FALSE_POSITIVE:lint operates on left operand which define (values) and provides false Note - the shift value is at least the precision of the essential type of the left hand side:[MISRA 2012 Rule 12.2, mandatory] */
        /*lint -e572 MISRA:FALSE_POSITIVE:lint operates on define and if define is zero false warning - Excessive shift value (precision 0 shifted right by 8):[MISRA 2004 Rule 12.8, required] */
        DoIP_UdpConAdmin[conIndex].txBuffer[2u] = (uint8)(PL_TYPE_GENERIC_N_ACK >> SHIFT_BY_ONE_BYTE);
        DoIP_UdpConAdmin[conIndex].txBuffer[3u] = (uint8)PL_TYPE_GENERIC_N_ACK;
        /* Length < 255, hence 2nd, 3rd and 4th byte of payload length field is 0 */
        DoIP_UdpConAdmin[conIndex].txBuffer[4u] = 0u;   
        DoIP_UdpConAdmin[conIndex].txBuffer[5u] = 0u;
        DoIP_UdpConAdmin[conIndex].txBuffer[6u] = 0u;
        DoIP_UdpConAdmin[conIndex].txBuffer[7u] = PL_LEN_GENERIC_N_ACK;
        DoIP_UdpConAdmin[conIndex].txBuffer[8u+0u] = nackCode;
        
        txPduInfo.SduDataPtr = DoIP_UdpConAdmin[conIndex].txBuffer;
        txPduInfo.SduLength = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_GENERIC_N_ACK;
        soadTxPduId = DoIP_ConfigPtr->DoIP_UdpMsg[conIndex].DoIP_UdpSoADTxPduRef;

        ret = SoAd_IfTransmit(soadTxPduId, &txPduInfo);
        
        if (E_OK == ret) {
            DoIP_UdpConAdmin[conIndex].txBufferState = BUFFER_LOCK;
        } else {
            DOIP_DET_REPORTERROR(DOIP_CREATE_AND_SEND_NACK_ID, DOIP_E_IF_TRANSMIT_ERROR);
        }
    } else {
        DOIP_DET_REPORTERROR(DOIP_CREATE_AND_SEND_NACK_ID, DOIP_E_BUFFER_BUSY);
    }
}

/** @req SWS_DoIP_00009 */
static void createAndSendNackTp(SoAd_SoConIdType conIndex, uint8 nackCode) {
    PduInfoType txPduInfo;
    PduIdType soadTxPduId;
    PduIdType doipLoTxPduId;
    Std_ReturnType ret;
    
    SchM_Enter_DoIP_EA_0();
    if (BUFFER_IDLE == DoIP_TcpConAdmin[conIndex].txBufferState) {
        
        txPduInfo.SduLength = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_GENERIC_N_ACK;
        soadTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpSoADTxPduRef;

        ret = SoAd_TpTransmit(soadTxPduId, &txPduInfo);
        
        if (E_OK == ret) {
            doipLoTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpTxPduRef;
            
            DoIP_TcpConAdmin[conIndex].txBuffer[0u] = PROTOCOL_VERSION;
            DoIP_TcpConAdmin[conIndex].txBuffer[1u] = ~(uint8)PROTOCOL_VERSION;
            /*lint -e9053 MISRA:FALSE_POSITIVE:lint operates on left operand which define (values) and provides false Note - the shift value is at least the precision of the essential type of the left hand side:[MISRA 2012 Rule 12.2, mandatory] */
            /*lint -e572 MISRA:FALSE_POSITIVE:lint operates on define and if define is zero false warning - Excessive shift value (precision 0 shifted right by 8):[MISRA 2004 Rule 12.8, required] */
            DoIP_TcpConAdmin[conIndex].txBuffer[2u] = (uint8)(PL_TYPE_GENERIC_N_ACK >> SHIFT_BY_ONE_BYTE);
            DoIP_TcpConAdmin[conIndex].txBuffer[3u] = (uint8)PL_TYPE_GENERIC_N_ACK;
            /* Length < 255, hence 2nd, 3rd and 4th byte of payload length field is 0 */
            DoIP_TcpConAdmin[conIndex].txBuffer[4u] = 0u;
            DoIP_TcpConAdmin[conIndex].txBuffer[5u] = 0u;
            DoIP_TcpConAdmin[conIndex].txBuffer[6u] = 0u;
            DoIP_TcpConAdmin[conIndex].txBuffer[7u] = PL_LEN_GENERIC_N_ACK;
            DoIP_TcpConAdmin[conIndex].txBuffer[8u+0u] = nackCode;
            
            DoIP_TcpConAdmin[conIndex].txBytesToTransmit = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_GENERIC_N_ACK;
            DoIP_TcpConAdmin[conIndex].txBytesCopied = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_GENERIC_N_ACK;
            DoIP_TcpConAdmin[conIndex].txBytesTransmitted = 0u;
            DoIP_TcpConAdmin[conIndex].txPduIdUnderProgress = doipLoTxPduId;            
            DoIP_TcpConAdmin[conIndex].txBufferState = BUFFER_LOCK;
            DoIP_TcpConAdmin[conIndex].channelIndex = INVALID_CHANNEL_INDEX;
        } else {
            DOIP_DET_REPORTERROR(DOIP_CREATE_AND_SEND_D_ACK_ID, DOIP_E_TP_TRANSMIT_ERROR);
        }
    } else {
        DOIP_DET_REPORTERROR(DOIP_CREATE_AND_SEND_D_ACK_ID, DOIP_E_BUFFER_BUSY);
    }
    SchM_Exit_DoIP_EA_0();
}

/** @req SWS_DoIP_00009 */
static void createAndSendDiagnosticAck(SoAd_SoConIdType conIndex, uint16 sa, uint16 ta) {
    PduInfoType txPduInfo;
    PduIdType soadTxPduId;
    PduIdType doipLoTxPduId;
    Std_ReturnType ret;
    
    SchM_Enter_DoIP_EA_0();
    if (BUFFER_IDLE == DoIP_TcpConAdmin[conIndex].txBufferState) {
        
        txPduInfo.SduLength = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_DIAG_MSG_ACK;
        soadTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpSoADTxPduRef;

        ret = SoAd_TpTransmit(soadTxPduId, &txPduInfo);
        
        if (E_OK == ret) {
            doipLoTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpTxPduRef;
            
            /** @req SWS_DoIP_00132 */ /** @req SWS_DoIP_00133 */ /** @req SWS_DoIP_00134 */
            DoIP_TcpConAdmin[conIndex].txBuffer[0u] = PROTOCOL_VERSION;
            DoIP_TcpConAdmin[conIndex].txBuffer[1u] = ~(uint8)PROTOCOL_VERSION;
            DoIP_TcpConAdmin[conIndex].txBuffer[2u] = (uint8)(PL_TYPE_DIAG_MSG_P_ACK >> SHIFT_BY_ONE_BYTE);
            DoIP_TcpConAdmin[conIndex].txBuffer[3u] = (uint8)PL_TYPE_DIAG_MSG_P_ACK;
            /* Length < 255, hence 2nd, 3rd and 4th byte of payload length field is 0 */
            DoIP_TcpConAdmin[conIndex].txBuffer[4u] = 0u;
            DoIP_TcpConAdmin[conIndex].txBuffer[5u] = 0u;
            DoIP_TcpConAdmin[conIndex].txBuffer[6u] = 0u;
            DoIP_TcpConAdmin[conIndex].txBuffer[7u] = PL_LEN_DIAG_MSG_ACK;
            DoIP_TcpConAdmin[conIndex].txBuffer[8u+0u] = ta >> SHIFT_BY_ONE_BYTE;
            DoIP_TcpConAdmin[conIndex].txBuffer[8u+1u] = (uint8) ta;
            DoIP_TcpConAdmin[conIndex].txBuffer[8u+2u] = sa >> SHIFT_BY_ONE_BYTE;
            DoIP_TcpConAdmin[conIndex].txBuffer[8u+3u] = (uint8) sa;
            DoIP_TcpConAdmin[conIndex].txBuffer[8u+4u] = 0u;      
            
            DoIP_TcpConAdmin[conIndex].txBytesToTransmit = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_DIAG_MSG_ACK;
            DoIP_TcpConAdmin[conIndex].txBytesCopied = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_DIAG_MSG_ACK;
            DoIP_TcpConAdmin[conIndex].txBytesTransmitted = 0u;
            DoIP_TcpConAdmin[conIndex].txPduIdUnderProgress = doipLoTxPduId;            
            DoIP_TcpConAdmin[conIndex].txBufferState = BUFFER_LOCK;
            DoIP_TcpConAdmin[conIndex].channelIndex = INVALID_CHANNEL_INDEX;
        } else {
            DOIP_DET_REPORTERROR(DOIP_CREATE_AND_SEND_D_ACK_ID, DOIP_E_TP_TRANSMIT_ERROR);
        }
    } else {
        /* Push the transmission related data into the queue */
        if ((FALSE == DoIP_TcpQueueAdmin[conIndex].tpTransmitQueueActive) &&
        		(FALSE ==  DoIP_TcpQueueAdmin[conIndex].diagAckQueueActive)) {
            DoIP_TcpQueueAdmin[conIndex].diagAckQueueActive = TRUE;
            DoIP_TcpQueueAdmin[conIndex].sa = sa;
            DoIP_TcpQueueAdmin[conIndex].ta = ta;
        } else {
            DOIP_DET_REPORTERROR(DOIP_CREATE_AND_SEND_D_ACK_ID, DOIP_E_BUFFER_BUSY);
        }
    }
    SchM_Exit_DoIP_EA_0();
}

static void createAndSendDiagnosticNack(SoAd_SoConIdType conIndex, uint16 sa, uint16 ta, uint8 nackCode) {
    PduInfoType txPduInfo;
    PduIdType soadTxPduId;
    PduIdType doipLoTxPduId;
    Std_ReturnType ret;
    
    SchM_Enter_DoIP_EA_0();
    
    soadTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpSoADTxPduRef;
    
    if (BUFFER_IDLE == DoIP_TcpConAdmin[conIndex].txBufferState) {
        
        txPduInfo.SduLength = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_DIAG_MSG_ACK;
    
        ret = SoAd_TpTransmit(soadTxPduId, &txPduInfo);
        
        if (E_OK == ret) {
            doipLoTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpTxPduRef;
            
            /** @req SWS_DoIP_00135 */ /** @req SWS_DoIP_00136 */ /** @req SWS_DoIP_00137 */
            DoIP_TcpConAdmin[conIndex].txBuffer[0u] = PROTOCOL_VERSION;
            DoIP_TcpConAdmin[conIndex].txBuffer[1u] = ~(uint8)PROTOCOL_VERSION;
            DoIP_TcpConAdmin[conIndex].txBuffer[2u] = (uint8)(PL_TYPE_DIAG_MSG_N_ACK >> SHIFT_BY_ONE_BYTE);
            DoIP_TcpConAdmin[conIndex].txBuffer[3u] = (uint8)PL_TYPE_DIAG_MSG_N_ACK;
            /* Length < 255, hence 2nd, 3rd and 4th byte of payload length field is 0 */
            DoIP_TcpConAdmin[conIndex].txBuffer[4u] = 0u;
            DoIP_TcpConAdmin[conIndex].txBuffer[5u] = 0u;
            DoIP_TcpConAdmin[conIndex].txBuffer[6u] = 0u;
            DoIP_TcpConAdmin[conIndex].txBuffer[7u] = PL_LEN_DIAG_MSG_ACK;
            DoIP_TcpConAdmin[conIndex].txBuffer[8u+0u] = ta >> SHIFT_BY_ONE_BYTE;
            DoIP_TcpConAdmin[conIndex].txBuffer[8u+1u] = (uint8) ta;
            DoIP_TcpConAdmin[conIndex].txBuffer[8u+2u] = sa >> SHIFT_BY_ONE_BYTE;
            DoIP_TcpConAdmin[conIndex].txBuffer[8u+3u] = (uint8) sa;
            DoIP_TcpConAdmin[conIndex].txBuffer[8u+4u] = nackCode;

            DoIP_TcpConAdmin[conIndex].txBytesToTransmit = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_DIAG_MSG_ACK;
            DoIP_TcpConAdmin[conIndex].txBytesCopied = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_DIAG_MSG_ACK;
            DoIP_TcpConAdmin[conIndex].txBytesTransmitted = 0u;
            DoIP_TcpConAdmin[conIndex].txPduIdUnderProgress = doipLoTxPduId;            
            DoIP_TcpConAdmin[conIndex].txBufferState = BUFFER_LOCK;
            DoIP_TcpConAdmin[conIndex].channelIndex = INVALID_CHANNEL_INDEX;
            
        } else {
            DOIP_DET_REPORTERROR(DOIP_CREATE_AND_SEND_D_NACK_ID, DOIP_E_TP_TRANSMIT_ERROR);
        }
    } else {
        DOIP_DET_REPORTERROR(DOIP_CREATE_AND_SEND_D_NACK_ID, DOIP_E_BUFFER_BUSY);
    }
    SchM_Exit_DoIP_EA_0();
}

static void createVehicleIdentificationResponse(uint8* txBuffer, SoAd_SoConIdType conIndex) {
    
    boolean getVinResult;
    
    getVinResult = FALSE;
    txBuffer[0u] = PROTOCOL_VERSION;
    txBuffer[1u] = ~(uint8)PROTOCOL_VERSION;
    /*lint -e9053 MISRA:FALSE_POSITIVE:lint operates on left operand which define (values) and provides false Note - the shift value is at least the precision of the essential type of the left hand side:[MISRA 2012 Rule 12.2, mandatory] */
    /*lint -e{572,778} MISRA:FALSE_POSITIVE:lint operates on define and if define is zero false warning - Excessive shift value (precision 0 shifted right by 8):[MISRA 2004 Rule 12.8, required] */
    txBuffer[2u] = (uint8)(PL_TYPE_VID_RES >> SHIFT_BY_ONE_BYTE);
    txBuffer[3u] = (uint8)PL_TYPE_VID_RES;
    /* Length < 255, hence 2nd, 3rd and 4th byte of payload length field is 0 */
    txBuffer[4u] = 0u;
    txBuffer[5u] = 0u;
    txBuffer[6u] = 0u;
    txBuffer[7u] = PL_LEN_VID_RES;

    /* VIN field */
    if (FALSE == DoIP_VinUpdated) {
        getVinResult = getVin(DoIP_Vin);
    }
    memcpy(&txBuffer[VID_VIN_INDEX], DoIP_Vin, sizeof(DoIP_Vin));

    /** @req SWS_DoIP_00073 */
    /* Logical address field */
    txBuffer[VID_LA_INDEX+0u] = (uint8)(DoIP_LOGICAL_ADDRESS >> SHIFT_BY_ONE_BYTE);
    txBuffer[VID_LA_INDEX+1u] = (uint8)DoIP_LOGICAL_ADDRESS;

    /* EID field */
    if (FALSE == DoIP_EidUpdated) {
        (void)getEid(DoIP_Eid, conIndex);
    }
    memcpy(&txBuffer[VID_EID_INDEX], DoIP_Eid, sizeof(DoIP_Eid));

    /* GID field */
    getGid(&txBuffer[VID_GID_INDEX], conIndex);

    /* Further action required field */
    if (E_OK != getVinResult) {
        txBuffer[VID_FUR_ACT_REQ_INDEX] = 0x00u;
    } else {
        getFurtherActionRequired(&txBuffer[VID_FUR_ACT_REQ_INDEX]);
    }

#if (DOIP_USE_VEHICLE_ID_SYNC_STATUS == STD_ON)  
    /** @req SWS_DoIP_00086 */   
    /* VIN/GID Sync Status field */
    if (E_OK != getVinResult) {
        txBuffer[VID_VIN_GID_STS_INDEX] = 0x10u;
    } else {
        txBuffer[VID_VIN_GID_STS_INDEX] = 0x00;
    }
#endif
}

static void sendVehicleAnnouncement(SoAd_SoConIdType conIndex) {
    PduInfoType txPduInfo;
    PduIdType soadTxPduId;
    Std_ReturnType ret;
    
    if (BUFFER_IDLE == DoIP_UdpConAdmin[conIndex].txBufferState) {

        createVehicleIdentificationResponse(DoIP_UdpConAdmin[conIndex].txBuffer, conIndex);

        txPduInfo.SduDataPtr = DoIP_UdpConAdmin[conIndex].txBuffer;
        txPduInfo.SduLength = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_VID_RES;
        soadTxPduId = DoIP_ConfigPtr->DoIP_UdpMsg[conIndex].DoIP_UdpSoADTxPduRef;

        updateRemoteAddrAsBroadcast(conIndex);
        ret = SoAd_IfTransmit(soadTxPduId, &txPduInfo);
        
        if (E_OK == ret) {
            DoIP_UdpConAdmin[conIndex].txBufferState = BUFFER_LOCK;
        } else {
            DOIP_DET_REPORTERROR(DOIP_SEND_VEHICLE_ANNOUNCE_REQ_ID, DOIP_E_IF_TRANSMIT_ERROR);
        }
    } else {
        DOIP_DET_REPORTERROR(DOIP_SEND_VEHICLE_ANNOUNCE_REQ_ID, DOIP_E_BUFFER_BUSY);
    }
}

static void fillRoutingActivationResponseData(uint8 *txBuffer, uint16 sa, uint8 routingActivationResponseCode) {
    txBuffer[0u] = PROTOCOL_VERSION;
    txBuffer[1u] = ~(uint8)PROTOCOL_VERSION;
    /*lint -e{572,778} MISRA:FALSE_POSITIVE:lint operates on define and if define is zero false warning - Excessive shift value (precision 0 shifted right by 8):[MISRA 2004 Rule 12.8, required] */
    txBuffer[2u] = (uint8)PL_TYPE_ROUT_ACTIV_RES >> SHIFT_BY_ONE_BYTE;
    txBuffer[3u] = (uint8)PL_TYPE_ROUT_ACTIV_RES;
    /* Length < 255, hence 2nd, 3rd and 4th byte of payload length field is 0 */
    txBuffer[4u] = 0u;
    txBuffer[5u] = 0u;
    txBuffer[6u] = 0u;
    txBuffer[7u] = PL_LEN_ROUT_ACTIV_RES;

    txBuffer[8u+0u] = sa >> SHIFT_BY_ONE_BYTE;
    txBuffer[8u+1u] = (uint8)sa;
    txBuffer[8u+2u] = (uint8)(DoIP_LOGICAL_ADDRESS >> SHIFT_BY_ONE_BYTE);
    txBuffer[8u+3u] = (uint8)DoIP_LOGICAL_ADDRESS;
    txBuffer[8u+4u] = routingActivationResponseCode;
    /* Reserved */
    txBuffer[8u+5u] = 0u;
    txBuffer[8u+6u] = 0u;
    txBuffer[8u+7u] = 0u;
    txBuffer[8u+8u] = 0u;
#if 0
    /* OEM Specific not supported */
    txBuffer[8u+9u] = 0u;
    txBuffer[8u+10u] = 0u;
    txBuffer[8u+11u] = 0u;
    txBuffer[8u+12u] = 0u;
#endif
}

static void createAndSendAliveCheck(SoAd_SoConIdType conIndex) {
    PduInfoType txPduInfo;
    PduIdType soadTxPduId;
    PduIdType doipLoTxPduId;
    Std_ReturnType ret;
    
    soadTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpTxPduRef;

    if (BUFFER_IDLE == DoIP_TcpConAdmin[conIndex].txBufferState) {

        txPduInfo.SduLength = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_ALIVE_CHK_REQ;
    
        ret = SoAd_TpTransmit(soadTxPduId, &txPduInfo);
        
        if (E_OK == ret) {
            doipLoTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpTxPduRef;
            
            DoIP_TcpConAdmin[conIndex].txBuffer[0u] = PROTOCOL_VERSION;
            DoIP_TcpConAdmin[conIndex].txBuffer[1u] = ~(uint8)PROTOCOL_VERSION;
            /*lint -e{572,778} MISRA:FALSE_POSITIVE:lint operates on define and if define is zero false warning - Excessive shift value (precision 0 shifted right by 8):[MISRA 2004 Rule 12.8, required] */
            DoIP_TcpConAdmin[conIndex].txBuffer[2u] = (uint8)(PL_TYPE_ALIVE_CHK_REQ >> SHIFT_BY_ONE_BYTE);
            DoIP_TcpConAdmin[conIndex].txBuffer[3u] = (uint8)PL_TYPE_ALIVE_CHK_REQ;
            /* Length < 255, hence 2nd, 3rd and 4th byte of payload length field is 0 */
            DoIP_TcpConAdmin[conIndex].txBuffer[4u] = 0u;
            DoIP_TcpConAdmin[conIndex].txBuffer[5u] = 0u;
            DoIP_TcpConAdmin[conIndex].txBuffer[6u] = 0u;
            DoIP_TcpConAdmin[conIndex].txBuffer[7u] = PL_LEN_ALIVE_CHK_REQ;
            
            DoIP_TcpConAdmin[conIndex].txBytesToTransmit = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_ALIVE_CHK_REQ;
            DoIP_TcpConAdmin[conIndex].txBytesCopied = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_ALIVE_CHK_REQ;
            DoIP_TcpConAdmin[conIndex].txBytesTransmitted = 0u;
            DoIP_TcpConAdmin[conIndex].txPduIdUnderProgress = doipLoTxPduId;
            DoIP_TcpConAdmin[conIndex].txBufferState = BUFFER_LOCK;
            
        } else {
            /* Could not send data - socket probably broken, so let's close the socket. */
            /* If there are pending routing activations waiting, activate that one. */
            if (TRUE == DoIP_PendingRoutActivFlag) {            
                handleTimeout(conIndex);
            } else {
                closeSocket(TCP_FORCE_CLOSE_TYPE, conIndex, soadTxPduId);
            }
        }
    } else {
        /* No TX buffer available. Report a Det error. */
        DOIP_DET_REPORTERROR(DOIP_CREATE_AND_SEND_ALIVE_CHECK_ID, DOIP_E_BUFFER_BUSY);
    }
}

static void startSingleSaAliveCheck(SoAd_SoConIdType conIndex) {
    
    if (FALSE == DoIP_TcpConAdmin[conIndex].awaitingAliveCheckResponse) {
        DoIP_TcpConAdmin[conIndex].awaitingAliveCheckResponse = TRUE;
        DoIP_TcpConAdmin[conIndex].generalInactivityTimer = 0u;
        DoIP_TcpConAdmin[conIndex].aliveCheckTimer = 0u;

        createAndSendAliveCheck(conIndex);
    }
}

static void startAllSocketsAliveCheck() {
    SoAd_SoConIdType conIndex;
    
    for (conIndex = 0u; conIndex < DOIP_TCP_CON_NUM; conIndex++) {
        /* No need to check connection states as this method is only called
           when all sockets are in registered state. */
        DoIP_TcpConAdmin[conIndex].awaitingAliveCheckResponse = TRUE;
        DoIP_TcpConAdmin[conIndex].generalInactivityTimer = 0u;
        DoIP_TcpConAdmin[conIndex].aliveCheckTimer = 0u;
        createAndSendAliveCheck(conIndex);
    }
}

static DoIP_Internal_LookupResType lookupSaTa(SoAd_SoConIdType conIndex, uint16 sa, uint16 ta, uint8* targetIndex) {
    uint8 j;
    DoIP_Internal_LookupResType retVal;
    uint8 testerIndex;
    uint8 i;
    boolean found;

    testerIndex = 0;
    /* Check whether the tester(source address) is configured */
    for (i = 0u; (i < DOIP_TESTER_COUNT); i++) {
        if (DoIP_ConfigPtr->DoIP_Tester[i].DoIP_TesterSA  == sa) {
            testerIndex = (uint8)i;
            break;
        }
    }
    
    found = FALSE;
    /* Tester(source address) is configured, check further .... */
    if (i != DOIP_TESTER_COUNT) {
        /* Check whether tester is linked to the connection handler */
        if (sa == DoIP_TcpConAdmin[conIndex].sa) {
            /* Is the connection handler registered ? */
            if (CONNECTION_REGISTERED == DoIP_TcpConAdmin[conIndex].socketState) {
                /* Is Target address linked with the Tester ?
                   Check all the Target addresses linked with each of the 
                   routing activations which are assigned to the tester */
                for (i = 0u; i < DoIP_ConfigPtr->DoIP_Tester[testerIndex].DoIp_RoutRefSize; i++) {
                    for (j = 0u; j < DoIP_ConfigPtr->DoIP_RoutingActivation[i].DoIP_chnlTARef_Size; j++) {
                        if (DoIP_ConfigPtr->DoIP_RoutingActivation[i].DoIP_TargetRef[j] == ta) {
                            *targetIndex = i;
                            found = TRUE;
                            break;
                        }
                    }    
                }
                
                if (TRUE == found) {
                    retVal = LOOKUP_SA_TA_OK;
                } else {
                    retVal = LOOKUP_SA_TA_TAUNKNOWN;
                }
            } else {
            	retVal = LOOKUP_SA_TA_ROUTING_ERR;
            }
        } else {
            retVal = LOOKUP_SA_TA_SAERR;
        }
    } else {
        retVal = LOOKUP_SA_TA_SAERR;
    }
    
    return retVal;
}

static DoIP_Internal_SockAssigResType socketHandle(SoAd_SoConIdType conIndex, uint16 activationType, uint16 sa, uint8* routingActivationResponseCode) {
    SoAd_SoConIdType numRegisteredSockets;
    SoAd_SoConIdType tempConIndex;
    /* This method is intended to implement Figure 13 in ISO/FDIS 13400-2:2012(E) */
    DoIP_Internal_SockAssigResType socketHandleResult;

    numRegisteredSockets = 0u;
    socketHandleResult = SOCKET_ASSIGNMENT_FAILED;
    for (tempConIndex = 0u; tempConIndex < DOIP_TCP_CON_NUM; tempConIndex++) {
        if (CONNECTION_REGISTERED == DoIP_TcpConAdmin[tempConIndex].socketState) {
            numRegisteredSockets++;
        }
    }

    if (0u == numRegisteredSockets) {
        /* No registered sockets, so we pick a slot for this connection: */
        registerSocket(conIndex, sa, activationType);

        /* No need to set *routingActivationResponseCode when socket assignment is successful. */
        socketHandleResult = SOCKET_ASSIGNMENT_SUCCESSFUL;
    } else {
        /* We found the TCP socket. Is it registered? */
        if (CONNECTION_REGISTERED == DoIP_TcpConAdmin[conIndex].socketState) {
            /* We found the registered TCP socket. Is it assigned to this tester (SA)? */
            if (DoIP_TcpConAdmin[conIndex].sa == sa) {
                socketHandleResult = SOCKET_ASSIGNMENT_SUCCESSFUL;

            } else {
                /** @req SWS_DoIP_00106 */
                /* Routing activation denied because an SA different from the table connection entry 
                   was received on the already activated TCP_DATA socket. */
                *routingActivationResponseCode = 0x02u;
                socketHandleResult = SOCKET_ASSIGNMENT_FAILED;
            }
        } else {
            /** @req SWS_DoIP_00103 */
            /* Next up: Check if SA is already registered to another socket... */
            for (tempConIndex = 0u; tempConIndex < DOIP_TCP_CON_NUM; tempConIndex++) {
                if ((DoIP_TcpConAdmin[tempConIndex].sa == sa) && (CONNECTION_REGISTERED == DoIP_TcpConAdmin[tempConIndex].socketState)) {
                    /** @req SWS_DoIP_00105 */
                    /* Yes, the SA is already registered on another socket! */
                    /* perform alive check single SA */
                    startSingleSaAliveCheck(tempConIndex);
                    socketHandleResult = SOCKET_ASSIGNMENT_PENDING;
                    break;
                }
            }
            
            /** @req SWS_DoIP_00107 */
            if (DOIP_TCP_CON_NUM == tempConIndex) {
                /* Next up: Check to see that there is a free socket slot available.. */
                for (tempConIndex = 0u; tempConIndex < DOIP_TCP_CON_NUM; tempConIndex++) {
                    if (CONNECTION_INITIALIZED == DoIP_TcpConAdmin[tempConIndex].socketState) {
                        /* Yes, we found a free slot */
                        registerSocket(tempConIndex, activationType, sa);
                        socketHandleResult = SOCKET_ASSIGNMENT_SUCCESSFUL;
                        break;
                    }
                }
                
                if (DOIP_TCP_CON_NUM == tempConIndex) {
                    /* For loop terminated; that means that there are no free slots.
                    Perform alive check on all registered sockets... */
                    startAllSocketsAliveCheck();
                    socketHandleResult = SOCKET_ASSIGNMENT_PENDING;
                }
            }
        }
    }
    
    return socketHandleResult;
}

static void handleAliveCheckResp(SoAd_SoConIdType conIndex,const uint8* rxBuffer) {
    DoIPPayloadType payloadLength;
    PduInfoType txPduInfo;
    PduIdType soadTxPduId;
    PduIdType doipLoTxPduId;
    SoAd_SoConIdType i;
    uint16 remainingConnections;
    uint16 sa;

    Std_ReturnType ret;
    uint8 routingActivationResponseCode;

    routingActivationResponseCode = 0;
    payloadLength = GET_PL_LEN_FROM_DOIP_MSG_PTR(rxBuffer);
    soadTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpSoADTxPduRef;

    if (PL_LEN_ALIVE_CHK_RES == payloadLength) {

        remainingConnections = 0u;
        /* Connections remaining to receive alive check responses */
        for (i = 0u; i < DOIP_TCP_CON_NUM; i++) {
            if (TRUE == DoIP_TcpConAdmin[i].awaitingAliveCheckResponse) {
                remainingConnections++;
            }
        }

        if (remainingConnections != 0x00u) {

            remainingConnections--;
            
            DoIP_TcpConAdmin[conIndex].generalInactivityTimer = 0u;
            DoIP_TcpConAdmin[conIndex].awaitingAliveCheckResponse = FALSE;
            
            sa = GET_SA_FROM_DOIP_MSG_PTR(rxBuffer);
            
            if (CONNECTION_REGISTERED == DoIP_TcpConAdmin[conIndex].socketState) {
                if (DoIP_TcpConAdmin[conIndex].sa == sa) {
                    /* 0x03: Routing activation denied because the SA is already registered 
                            and active on a different TCP_DATA socket */
                    routingActivationResponseCode = 0x03u;
                } else {
                    if (0x00u == remainingConnections) {
                        /* 0x01: Routing activation denied because all concurrently supported
                                TCP_DATA sockets are registered and active. */
                        routingActivationResponseCode = 0x01u;
                    }
                }
            }
            
            if ((0x03u == routingActivationResponseCode) || (0x00u == routingActivationResponseCode)) { 
                soadTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[DoIP_PendingRoutActivConIndex].DoIP_TcpSoADTxPduRef;
                txPduInfo.SduLength = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_ROUT_ACTIV_RES;
                
                /** @req SWS_DoIP_00220 */
                ret = SoAd_TpTransmit(soadTxPduId, &txPduInfo);
                
                if (E_OK == ret) {
                    doipLoTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[DoIP_PendingRoutActivConIndex].DoIP_TcpTxPduRef;
    
                    fillRoutingActivationResponseData(DoIP_TcpConAdmin[DoIP_PendingRoutActivConIndex].txBuffer, DoIP_PendingRoutActivSa, routingActivationResponseCode);
                    DoIP_TcpConAdmin[DoIP_PendingRoutActivConIndex].txBytesToTransmit = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_ROUT_ACTIV_RES;
                    DoIP_TcpConAdmin[DoIP_PendingRoutActivConIndex].txBytesCopied = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_ROUT_ACTIV_RES;
                    DoIP_TcpConAdmin[DoIP_PendingRoutActivConIndex].txBytesTransmitted = 0u;
                    DoIP_TcpConAdmin[DoIP_PendingRoutActivConIndex].txPduIdUnderProgress = doipLoTxPduId;
                    DoIP_TcpConAdmin[DoIP_PendingRoutActivConIndex].txBufferState = BUFFER_LOCK;
                } else {
                    /** @req SWS_DoIP_00014 */ /** @req SWS_DoIP_00223 */
                    DOIP_DET_REPORTERROR(DOIP_HANDLE_ALIVECHECK_RESP_ID, DOIP_E_TP_TRANSMIT_ERROR);
                }
                DoIP_PendingRoutActivFlag = FALSE;
            }
        }
    } else {
        /* Invalid payload length! */
        createAndSendNackTp(conIndex, ERROR_INVALID_PAYLOAD_LENGTH);
        closeSocket(TCP_TYPE, conIndex, soadTxPduId);
    }
}

static void handleRoutingActivationReq(SoAd_SoConIdType conIndex, const uint8 *rxBuffer) {
    DoIPPayloadType payloadLength;
    PduInfoType txPduInfo;
    PduIdType soadTxPduId;
    PduIdType doipLoTxPduId;
    SoAd_SoConIdType SoConId;
    uint16 sa;
    uint16 activationType;
    uint16 routActRefIndex;
    Std_ReturnType ret;
    DoIP_Internal_SockAssigResType socketHandleResult;
    uint8 testerIndex;
    uint8 routingActivationResponseCode;
    
    payloadLength = GET_PL_LEN_FROM_DOIP_MSG_PTR(rxBuffer);
    soadTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpSoADTxPduRef;
    
    /** @req SWS_DoIP_00117  is invalid (refers to the wrong service and does not match
     * ISO13400-2:2012 issued on 2012-06-18)
     *
     * Decision made together with Customer's diagnostic tester group that activation
     * type is only one byte, thus payload length may be 11 or 7. (Not 12 or 8 as
     * AUTOSAR requirement states)
     */
     /** @req SWS_DoIP_00117 */
    if ((PL_LEN_ROUT_ACTIV_REQ == payloadLength) || (PL_LEN_ROUT_ACTIV_OEM_REQ == payloadLength)) {

        sa = GET_SA_FROM_DOIP_MSG_PTR(rxBuffer);
        activationType = GET_ACT_TYPE_FROM_DOIP_MSG_PTR(rxBuffer);

        if (BUFFER_IDLE == DoIP_TcpConAdmin[conIndex].txBufferState) {
            /* Vehicle manufacturer-specific */
            routingActivationResponseCode = 0x7eu; 

            if (TRUE == isSourceAddressKnown(sa, &testerIndex)) {

                /** @req SWS_DoIP_00108 */
                if (TRUE == isRoutingTypeSupported(activationType, testerIndex, &routActRefIndex)) {

                    (void)SoAd_GetSoConId(soadTxPduId, &SoConId);

                    socketHandleResult = socketHandle(conIndex, activationType, sa, &routingActivationResponseCode);
                    
                    if (SOCKET_ASSIGNMENT_SUCCESSFUL == socketHandleResult) {
                        /* Routing successfully activated */
                        /** @req SWS_DoIP_00113 */
                        routingActivationResponseCode = 0x10u;
                        DoIP_Arc_TcpConnectionNotification(SoConId);

#if 0
                        boolean authenticated;

                        /* Routing and confirmation callbacks not supported */ 
                        if (TRUE == isAuthenticationRequired(routActRefIndex)) {
                            authenticated = isAuthenticated(routActRefIndex);
                        
                        } else {
                            authenticated = TRUE;
                        }

                        if (authenticated) {
                            if (TRUE == isConfirmationRequired(routActRefIndex)) {
                                if (TRUE == isConfirmed(routActRefIndex)) {
                                    /* Routing successfully activated */
                                    /** @req SWS_DoIP_00113 */
                                    routingActivationResponseCode = 0x10u;

                                } else {
                                    /* Routing will be activated; confirmation required */
                                    routingActivationResponseCode = 0x11u;
                                }
                            } else {
                                /* Routing successfully activated */
                                routingActivationResponseCode = 0x10u;
                            }
                        } else {
                            /* Routing activation rejected due to missing authentication */
                            routingActivationResponseCode = 0x04u;
                        }
#endif
                    } else if (SOCKET_ASSIGNMENT_FAILED == socketHandleResult) { /* socketHandle */
                        /* Routing activation denied because:
                         *  0x01 - all concurrently supported TCP_DATA sockets are
                         *         registered and active
                         *  0x02 - an SA different from the table connection entry
                         *         was received on the already activated TCP_DATA
                         *         socket
                         *  0x03 - the SA is already registered and active on a
                         *         different TCP_DATA socket
                         *
                         *  socketHandle() shall already have written the corresponding response code to
                         *  routingActivationResponseCode
                         */

                        /* Validate response code */
                        if (0x02 != routingActivationResponseCode) {
                            /* Socket will be closed */
                        } else {
                            /* Unsupported response code at this level */
                            routingActivationResponseCode = 0x7eu;
                            DOIP_DET_REPORTERROR(DOIP_ROUTING_ACTIVATION_REQ_ID, DOIP_E_UNEXPECTED_EXECUTION);
                        }
                        
                    } else if (SOCKET_ASSIGNMENT_PENDING == socketHandleResult) { /* socketHandle */
                        /*
                         * Trying to assign a connection slot, but pending
                         * alive check responses before continuing.
                         * Continuation handled from DoIP_MainFunction (if a
                         * connection times out and thus becomes free) or from
                         * handleAliveCheckResp (if all connections remain
                         * active)
                         */
                        if (FALSE == DoIP_PendingRoutActivFlag) {
                            DoIP_PendingRoutActivFlag = TRUE;
                            DoIP_PendingRoutActivConIndex = conIndex;
                            DoIP_PendingRoutActivSa = sa;
                            DoIP_pendingRoutActivActType = activationType;
                        } else {
                            /* Socket assignment pending; alive check response already pending
                               This should not happen - the connection attempt should not have been accepted. */
                            DOIP_DET_REPORTERROR(DOIP_ROUTING_ACTIVATION_REQ_ID, DOIP_E_UNEXPECTED_EXECUTION);
                        }
                    } else {
                        /* This cannot happen */
                        DOIP_DET_REPORTERROR(DOIP_ROUTING_ACTIVATION_REQ_ID, DOIP_E_UNEXPECTED_EXECUTION);
                    }

                } else {
                    /* @req SWS_DoIP_00160 */
                    /* Routing activation denied due to unsupported routing activation type */
                    routingActivationResponseCode = 0x06u;
                }

            } else {
                /** @req SWS_DoIP_00104 */
                /* Routing activation rejected due to unknown source address */
                routingActivationResponseCode = 0x00u;
            }

            txPduInfo.SduLength = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_ROUT_ACTIV_RES;
           
            ret = SoAd_TpTransmit(soadTxPduId, &txPduInfo);
            if (E_OK == ret) {
                doipLoTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpTxPduRef;
                
                fillRoutingActivationResponseData(DoIP_TcpConAdmin[conIndex].txBuffer, sa, routingActivationResponseCode);
                DoIP_TcpConAdmin[conIndex].txBytesToTransmit = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_ROUT_ACTIV_RES;
                DoIP_TcpConAdmin[conIndex].txBytesCopied = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_ROUT_ACTIV_RES;
                DoIP_TcpConAdmin[conIndex].txBytesTransmitted = 0u;
                DoIP_TcpConAdmin[conIndex].txPduIdUnderProgress = doipLoTxPduId;
                DoIP_TcpConAdmin[conIndex].txBufferState = BUFFER_LOCK;
            } else {
                DOIP_DET_REPORTERROR(DOIP_ROUTING_ACTIVATION_REQ_ID, DOIP_E_TP_TRANSMIT_ERROR);
            }

            /* Negative response code --> Close socket on which the current message was received */
            switch (routingActivationResponseCode) {
            /* Routing activated. */     
            case 0x10:
            /* Confirmation pending */
            case 0x11:
            /* Missing authentication */
            case 0x04:
                break;
            default:
                closeSocket(TCP_TYPE, conIndex, soadTxPduId);
                break;
            }

        } else {
            DOIP_DET_REPORTERROR(DOIP_ROUTING_ACTIVATION_REQ_ID, DOIP_E_BUFFER_BUSY);
        }
    } else {
        createAndSendNackTp(conIndex, ERROR_INVALID_PAYLOAD_LENGTH);
        closeSocket(TCP_TYPE, conIndex, soadTxPduId);
    }
}

static void handleDiagnosticMessage(SoAd_SoConIdType conIndex, uint8 *rxBuffer) {
    DoIPPayloadType payloadLength;
    PduInfoType pduInfo;
    PduIdType soadTxPduId;
    PduIdType pduRRxPduId;
    PduLengthType bufferSize;
    PduIdType diagMessageLengthToRecv;
    uint16 sa;
    uint16 ta;
    DoIP_Internal_LookupResType lookupResult;
    BufReq_ReturnType result;
    uint8 chIndex;
    uint8 targetIndex;

    payloadLength = GET_PL_LEN_FROM_DOIP_MSG_PTR(rxBuffer);
    soadTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpSoADTxPduRef;

    SchM_Enter_DoIP_EA_0();

    /** @req SWS_DoIP_00122 */
    if (payloadLength >= PL_LEN_DIAG_MIN_REQ) {
        diagMessageLengthToRecv = (PduIdType) (payloadLength - SA_AND_TA_LEN);
        
        sa = GET_SA_FROM_DOIP_MSG_PTR(rxBuffer);
        ta = GET_TA_FROM_DOIP_MSG_PTR(rxBuffer);
    
        /** @req SWS_DoIP_00125 */
        if (diagMessageLengthToRecv <= DOIP_MAX_REQUEST_BYTES) {
            
            lookupResult = lookupSaTa(conIndex, sa, ta, &targetIndex);
    
            if (lookupResult == LOOKUP_SA_TA_OK) {
                    /* Send diagnostic message to PduR */
                    chIndex = getChIndexFromTargetIndex(targetIndex);
                    
                    if (chIndex != INVALID_CHANNEL_INDEX) {
                        
                        pduRRxPduId = DoIP_ConfigPtr->DoIP_Channel[chIndex].DoIP_UpperLayerRxPduId; 
    
                        pduInfo.SduDataPtr = &(rxBuffer[MSG_LEN_INCL_PL_LEN_FIELD + SA_AND_TA_LEN]);
                        /** @req SWS_DoIP_00212 */
                        result = PduR_DoIPTpStartOfReception(pduRRxPduId, &pduInfo, diagMessageLengthToRecv, &bufferSize);
                        if (result == BUFREQ_OK) {
                            
                            /** @req SWS_DoIP_00260 */
                            /* Let PduR copy received data */
                            /** @req SWS_DoIP_00128 */
                            if (diagMessageLengthToRecv <= bufferSize) {
                                pduInfo.SduLength = diagMessageLengthToRecv;
                                (void)PduR_DoIPTpCopyRxData(pduRRxPduId, &pduInfo, &bufferSize);
                            }
                
                            /** @req SWS_DoIP_00221 */
                            /* Finished reception */
                            PduR_DoIPTpRxIndication(pduRRxPduId, E_OK);
                
                            /** @req SWS_DoIP_00129 */
                            /* Send diagnostic message positive ack */
                            createAndSendDiagnosticAck(conIndex, sa, ta);
                        }
                        else {
                            /** @req SWS_DoIP_00174 */
                            createAndSendDiagnosticNack(conIndex, sa, ta, ERROR_DIAG_TP_ERROR);
                        }
                    } else {
                        DOIP_DET_REPORTERROR(DOIP_HANDLE_DIAG_MSG_ID, DOIP_E_UNEXPECTED_EXECUTION);
                    }
                    
    
            } else if (lookupResult == LOOKUP_SA_TA_SAERR) {
                /** @req SWS_DoIP_00123 */ /** @req SWS_DoIP_00104 */
                /* SA not registered on receiving socket */
                createAndSendDiagnosticNack(conIndex, sa, ta, ERROR_DIAG_INVALID_SA);
                closeSocket(TCP_TYPE, conIndex, soadTxPduId);
            } else if (lookupResult == LOOKUP_SA_TA_TAUNKNOWN) {
                /** @req SWS_DoIP_00124 */
                createAndSendDiagnosticNack(conIndex, sa, ta, ERROR_DIAG_UNKNOWN_TA);
            } else if (lookupResult == LOOKUP_SA_TA_ROUTING_ERR) {
                /** @req SWS_DoIP_00127 */
                createAndSendDiagnosticNack(conIndex, sa, ta, ERROR_DIAG_TARGET_UNREACHABLE);
            } else {
                
            }
        } else {
            createAndSendDiagnosticNack(conIndex, sa, ta, ERROR_DIAG_MESSAGE_TO_LARGE);
        }            
    }  else {
        createAndSendNackTp(conIndex, ERROR_INVALID_PAYLOAD_LENGTH);
        closeSocket(TCP_TYPE, conIndex, soadTxPduId);
    }
    SchM_Exit_DoIP_EA_0(); 
}

static void handleVehicleIdentificationReq(SoAd_SoConIdType conIndex, const  uint8 *rxBuffer, DoIP_Internal_VehReqType type) {
    DoIPPayloadType payloadLength;
    PduInfoType txPduInfo;
    PduIdType soadTxPduId;
    Std_ReturnType ret;
    boolean doRespond;
    
    soadTxPduId = DoIP_ConfigPtr->DoIP_UdpMsg[conIndex].DoIP_UdpSoADTxPduRef;
    payloadLength = GET_PL_LEN_FROM_DOIP_MSG_PTR(rxBuffer);
    
    if (ID_REQUEST_ALL == type) {
        /** @req SWS_DoIP_00059 */
        if (PL_LEN_VID_REQ == payloadLength) {
            doRespond = TRUE;
    
        } else {
            /* Invalid payload length! */
            createAndSendNackIf(conIndex, ERROR_INVALID_PAYLOAD_LENGTH);
            closeSocket(UDP_TYPE, conIndex, soadTxPduId);
            doRespond = FALSE;
        }
    } else if (ID_REQUEST_BY_EID == type) {
        /** @req SWS_DoIP_00063 */
        if (PL_LEN_VID_EID_REQ_RES == payloadLength) {
            
            ret = E_OK;
            if (FALSE == DoIP_EidUpdated) {
                ret = getEid(DoIP_Eid, conIndex);
            }
            
            if (E_OK == ret) {
                /** @req SWS_DoIP_00066 */
                /*lint -e737 MISRA:FALSE_POSITIVE:Loss of sign in promotion from into to unsigned int:Other */
               if (E_OK == memcmp(DoIP_Eid, &rxBuffer[REQ_PAYLOAD_INDEX], sizeof(DoIP_Eid))) {
                    doRespond = TRUE;
                } else {
                    doRespond = FALSE;
                }
            } else {
                doRespond = FALSE;
            }
        } else {
            /* Invalid payload length! */
            createAndSendNackIf(conIndex, ERROR_INVALID_PAYLOAD_LENGTH);
            closeSocket(UDP_TYPE, conIndex, soadTxPduId);
            doRespond = FALSE;
        }
    } else if (type == ID_REQUEST_BY_VIN) {
        /** @req SWS_DoIP_00068 */
        if (PL_LEN_VID_VIN_REQ_RES == payloadLength) {

            ret = E_OK;
            if (FALSE == DoIP_VinUpdated) {
                ret = getVin(DoIP_Vin);
            }
            
            if (E_OK == ret) {
                /*lint -e737 MISRA:FALSE_POSITIVE:Loss of sign in promotion from into to unsigned int:Other */
                if (E_OK == memcmp(DoIP_Vin, &rxBuffer[REQ_PAYLOAD_INDEX], sizeof(DoIP_Vin))) {
                    doRespond = TRUE;
                } else {
                    doRespond = FALSE;
                }
            } else {
                doRespond = FALSE;
            }
        } else {
            /* Invalid payload length! */
            createAndSendNackIf(conIndex, ERROR_INVALID_PAYLOAD_LENGTH);
            closeSocket(UDP_TYPE, conIndex, soadTxPduId);
            doRespond = FALSE;
        }
    } else {
        /* Not reachable code */
        /* Unknown payload type! */
        createAndSendNackIf(conIndex, ERROR_UNKNOWN_PAYLOAD_TYPE);
        doRespond = FALSE;
    }

    /** @req SWS_DoIP_00060 */
    if (TRUE == doRespond) {
        if (BUFFER_IDLE == DoIP_UdpConAdmin[conIndex].txBufferState) {
    
            createVehicleIdentificationResponse(DoIP_UdpConAdmin[conIndex].txBuffer, conIndex);
    
            updateRemoteAddrAsBroadcast(conIndex);
            
            txPduInfo.SduDataPtr = DoIP_UdpConAdmin[conIndex].txBuffer;
            txPduInfo.SduLength = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_VID_RES;
            
            ret = SoAd_IfTransmit(soadTxPduId, &txPduInfo);
            
            if (E_OK == ret) {
                DoIP_UdpConAdmin[conIndex].txBufferState = BUFFER_LOCK;
            } else {
                DOIP_DET_REPORTERROR(DOIP_HANDLE_VEHICLE_ID_REQ_ID, DOIP_E_IF_TRANSMIT_ERROR);
            }
        } else {
            /* Not TX buffer available. Report a Det error */
            DOIP_DET_REPORTERROR(DOIP_HANDLE_VEHICLE_ID_REQ_ID, DOIP_E_BUFFER_BUSY);
        }
    
    } else {
        /* Do nothing */
    }
}

static void handleEntityStatusReq(SoAd_SoConIdType conIndex, const uint8 *rxBuffer) {
    DoIPPayloadType payloadLength;
    PduInfoType txPduInfo;
    PduIdType soadTxPduId;
    Std_ReturnType ret;
    uint8 curNofOpenSockets;


    soadTxPduId = DoIP_ConfigPtr->DoIP_UdpMsg[conIndex].DoIP_UdpSoADTxPduRef;    
    payloadLength = GET_PL_LEN_FROM_DOIP_MSG_PTR(rxBuffer);

    /** @req SWS_DoIP_00095 */
    if (PL_LEN_ENT_STATUS_REQ == payloadLength) {
        
        if (BUFFER_IDLE == DoIP_UdpConAdmin[conIndex].txBufferState) {
            curNofOpenSockets = getNofCurrentlyUsedTcpSockets();

            /** @req SWS_DoIP_00096 */
            DoIP_UdpConAdmin[conIndex].txBuffer[0u] = PROTOCOL_VERSION;
            DoIP_UdpConAdmin[conIndex].txBuffer[1u] = ~(uint8)PROTOCOL_VERSION;
            DoIP_UdpConAdmin[conIndex].txBuffer[2u] = (uint8)(PL_TYPE_ENT_STATUS_RES >> SHIFT_BY_ONE_BYTE);
            DoIP_UdpConAdmin[conIndex].txBuffer[3u] = (uint8)PL_TYPE_ENT_STATUS_RES;
            /* Length < 255, hence 2nd, 3rd and 4th byte of payload length field is 0 */
            DoIP_UdpConAdmin[conIndex].txBuffer[4u] = 0u;
            DoIP_UdpConAdmin[conIndex].txBuffer[5u] = 0u;
            DoIP_UdpConAdmin[conIndex].txBuffer[6u] = 0u;
            DoIP_UdpConAdmin[conIndex].txBuffer[7u] = PL_LEN_ENT_STATUS_RES;
            /** @req SWS_DoIP_00097 */
            DoIP_UdpConAdmin[conIndex].txBuffer[8u+0u] = DOIP_NODE_TYPE;
            /** @req SWS_DoIP_00098 */ 
            DoIP_UdpConAdmin[conIndex].txBuffer[8u+1u] = DOIP_MAX_TESTER_CONNECTIONS;
            /** @req SWS_DoIP_00099 */
            DoIP_UdpConAdmin[conIndex].txBuffer[8u+2u] = curNofOpenSockets;
#if (DOIP_ENTITY_MAX_BYTE_USE == STD_ON) 
            /** @req SWS_DoIP_00100 */
            /*lint -e778 MISRA:FALSE_POSITIVE:Macro value DOIP_MAX_REQUEST_BYTES leads to false errors - Constant expression evaluates to 0 in operation '>>:Other */
            DoIP_UdpConAdmin[conIndex].txBuffer[8u+3u] = (uint8)(DOIP_MAX_REQUEST_BYTES >> SHIFT_BY_THREE_BYTES);
            /*lint -e778 MISRA:FALSE_POSITIVE:Macro value DOIP_MAX_REQUEST_BYTES leads to false errors - Constant expression evaluates to 0 in operation '>>:Other */
            DoIP_UdpConAdmin[conIndex].txBuffer[8u+4u] = (uint8)(DOIP_MAX_REQUEST_BYTES >> SHIFT_BY_TW0_BYTES);
            /*lint -e778 MISRA:FALSE_POSITIVE:Macro value DOIP_MAX_REQUEST_BYTES leads to false errors - Constant expression evaluates to 0 in operation '>>:Other */
            DoIP_UdpConAdmin[conIndex].txBuffer[8u+5u] = (uint8)(DOIP_MAX_REQUEST_BYTES >> SHIFT_BY_ONE_BYTE);
            DoIP_UdpConAdmin[conIndex].txBuffer[8u+6u] = (uint8)DOIP_MAX_REQUEST_BYTES;
#endif
            txPduInfo.SduDataPtr = DoIP_UdpConAdmin[conIndex].txBuffer;
            txPduInfo.SduLength = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_ENT_STATUS_RES;
        
            /** @req SWS_DoIP_00198 */
            ret = SoAd_IfTransmit(soadTxPduId, &txPduInfo);
            
            if (E_OK == ret) {
                DoIP_UdpConAdmin[conIndex].txBufferState = BUFFER_LOCK;
            } else {
                DOIP_DET_REPORTERROR(DOIP_HANDLE_ENTITY_STATUS_REQ_ID, DOIP_E_IF_TRANSMIT_ERROR);
            }
        } else {
            DOIP_DET_REPORTERROR(DOIP_HANDLE_ENTITY_STATUS_REQ_ID, DOIP_E_BUFFER_BUSY);
        }
    } else {
        /* Invalid payload length! */
        createAndSendNackIf(conIndex, ERROR_INVALID_PAYLOAD_LENGTH);
        closeSocket(UDP_TYPE, conIndex, soadTxPduId);
    }
}

static void handlePowerModeCheckReq(SoAd_SoConIdType conIndex,const uint8 *rxBuffer) {
    DoIPPayloadType payloadLength;
    PduInfoType txPduInfo;
    PduIdType soadTxPduId;
    DoIP_PowerStateType powerMode;
    Std_ReturnType ret;
    
    powerMode = DOIP_POWER_STATE_NOT_READY;
    soadTxPduId = DoIP_ConfigPtr->DoIP_UdpMsg[conIndex].DoIP_UdpSoADTxPduRef;    
    payloadLength = GET_PL_LEN_FROM_DOIP_MSG_PTR(rxBuffer);
   
    /** @req SWS_DoIP_00091 */
    if (PL_LEN_POWER_MODE_REQ == payloadLength) {
        if (BUFFER_IDLE == DoIP_UdpConAdmin[conIndex].txBufferState) {
            
#if (DOIPPWRMODECALLBACK_CONFIGURED == TRUE)
            if (E_OK == DOIP_GETPWRMODECALLBACK_FUNCTION(&powerMode)) {
                /** @req SWS_DoIP_00093 */
                powerMode = DOIP_POWER_STATE_READY;
            } else {
                powerMode = DOIP_POWER_STATE_NOT_READY;
            }
#endif            
            /** @req SWS_DoIP_00092 */
            DoIP_UdpConAdmin[conIndex].txBuffer[0u] = PROTOCOL_VERSION;
            DoIP_UdpConAdmin[conIndex].txBuffer[1u] = ~(uint8)PROTOCOL_VERSION;
            DoIP_UdpConAdmin[conIndex].txBuffer[2u] = (uint8)(PL_TYPE_POWER_MODE_RES >> SHIFT_BY_ONE_BYTE);
            DoIP_UdpConAdmin[conIndex].txBuffer[3u] = (uint8)PL_TYPE_POWER_MODE_RES;
            /* Length < 255, hence 2nd, 3rd and 4th byte of payload length field is 0 */
            DoIP_UdpConAdmin[conIndex].txBuffer[4u] = 0u;
            DoIP_UdpConAdmin[conIndex].txBuffer[5u] = 0u;
            DoIP_UdpConAdmin[conIndex].txBuffer[6u] = 0u;
            DoIP_UdpConAdmin[conIndex].txBuffer[7u] = PL_LEN_POWER_MODE_RES;
            /** @req SWS_DoIP_00093 */
            DoIP_UdpConAdmin[conIndex].txBuffer[8u+0u] = (uint8)powerMode;

            txPduInfo.SduDataPtr = DoIP_UdpConAdmin[conIndex].txBuffer;
            txPduInfo.SduLength = MSG_LEN_INCL_PL_LEN_FIELD + PL_LEN_POWER_MODE_RES;

            ret = SoAd_IfTransmit(soadTxPduId, &txPduInfo);
            
            if (E_OK == ret) {
                DoIP_UdpConAdmin[conIndex].txBufferState = BUFFER_LOCK;
            } else {
                DOIP_DET_REPORTERROR(DOIP_HANDLE_POWER_MODE_REQ_ID, DOIP_E_IF_TRANSMIT_ERROR);
            }
        } else {
            DOIP_DET_REPORTERROR(DOIP_HANDLE_POWER_MODE_REQ_ID, DOIP_E_BUFFER_BUSY);
        }
    } else {
        /* Invalid payload length! */
        createAndSendNackIf(conIndex, ERROR_INVALID_PAYLOAD_LENGTH);
        closeSocket(UDP_TYPE, conIndex, soadTxPduId);
    }
}

static void handleTcpRx(SoAd_SoConIdType conIndex, uint8* rxBuffer) {
    DoIPPayloadType payloadType;

    payloadType = GET_PL_TYPE_FROM_DOIP_MSG_PTR(rxBuffer);
    
    /** @req SWS_DoIP_0008 */  /** @req SWS_DoIP_00101 */ /** @req SWS_DoIP_00121 */
    switch (payloadType) {
    /*
     * Vehicle identification requests are not supported over TCP
     * 0x0001: Vehicle Identification Request
     * 0x0002: Vehicle Identification Request with EID
     * 0x0003: Vehicle Identification Request with VIN
     */
    /* Routing Activation request */      
    case PL_TYPE_ROUT_ACTIV_REQ:
        handleRoutingActivationReq(conIndex, rxBuffer);
        break;
    /* Alive check response */
    case PL_TYPE_ALIVE_CHK_RES:
        handleAliveCheckResp(conIndex, rxBuffer);
        break;
    
    /* Diagnostic message */
    case PL_TYPE_DIAG_MSG:
        handleDiagnosticMessage(conIndex, rxBuffer);
        break;
    
    default:
        /* Unknown payload type! */
        createAndSendNackTp(conIndex, ERROR_UNKNOWN_PAYLOAD_TYPE);
        break;
    }
}

static void handleUdpRx(SoAd_SoConIdType conIndex, const uint8* rxBuffer) {
    DoIPPayloadType payloadType;
    
    payloadType = GET_PL_TYPE_FROM_DOIP_MSG_PTR(rxBuffer);
    
    /** @req SWS_DoIP_0008 */ /** @req SWS_DoIP_00061 */ /** @req SWS_DoIP_00062 */
    /** @req SWS_DoIP_00064 */ /** @req SWS_DoIP_00067 */ /** @req SWS_DoIP_00069 */
    /** @req SWS_DoIP_00090 */ /** @req SWS_DoIP_00094 */
    switch (payloadType) {
    /*
     * 0x0005: Routing Activation request is not supported over UDP
     * 0x8001: Diagnostic messages is not to be supported over UDP
     */        
    /* Vehicle Identification Request */
    case PL_TYPE_VID_REQ:         
        handleVehicleIdentificationReq(conIndex, rxBuffer, ID_REQUEST_ALL);
        break;
    /* Vehicle Identification Request with EID */
    case PL_TYPE_VID_EID_REQ:    
        handleVehicleIdentificationReq(conIndex, rxBuffer, ID_REQUEST_BY_EID);
        break;

    /* Vehicle Identification Request with VIN */
    case PL_TYPE_VID_VIN_REQ:    
        handleVehicleIdentificationReq(conIndex, rxBuffer, ID_REQUEST_BY_VIN);
        break;

    /* DoIP entity status request */
    case PL_TYPE_ENT_STATUS_REQ: 
        handleEntityStatusReq(conIndex, rxBuffer);
        break;

    /* DoIP power mode check request */
    case PL_TYPE_POWER_MODE_REQ: 
        handlePowerModeCheckReq(conIndex, rxBuffer);
        break;

    default:
        /* Unknown payload type! */
        createAndSendNackIf(conIndex, ERROR_UNKNOWN_PAYLOAD_TYPE);
        break;
    }
}

/***** Global function - Definition *****/

/**
 * @brief The service DoIP_TpTransmit Requests transmission of a PDU
 * @note Reentrant for different PduIds. Non reentrant for the same PduId.
 * @param[in] DoIPPduRTxId - Identifier of the PDU to be transmitted
 * @param[in] DoIPPduRTxInfoPtr - Length of and pointer to the PDU data and pointer to MetaData
 * @return Result of the function
 */
/** @req SWS_DoIP_00022 */
Std_ReturnType DoIP_TpTransmit(PduIdType DoIPPduRTxId, const PduInfoType* DoIPPduRTxInfoPtr) {
    DoIPPayloadType bytesToTransmit;
    PduInfoType txPdu;
    PduIdType doipLoTxPduId;
    PduIdType soadTxPduId;
    DoIP_PduType pduType;
    uint16 conIndex;
    uint16 sourceAddress;
    uint16 targetAddress;
    Std_ReturnType retVal;
    uint8 chIndex;


    /** @req SWS_DoIP_00162 */
    VALIDATE_W_RV((DoIP_Status == DOIP_INIT), DOIP_TP_TRANSMIT_SERVICE_ID, DOIP_E_UNINIT, E_NOT_OK);

    for (chIndex = 0u; chIndex < DOIP_CHANNEL_COUNT; chIndex++) {
        if (DoIP_ConfigPtr->DoIP_Channel[chIndex].DoIP_PduTransmitRef != NULL) {
            if (DoIP_ConfigPtr->DoIP_Channel[chIndex].DoIP_PduTransmitRef->DoIP_TxPduRef == DoIPPduRTxId) {
                break;
            }
        }
    }
    /** @req SWS_DoIP_00163 */
    VALIDATE_W_RV((chIndex != DOIP_CHANNEL_COUNT), DOIP_TP_TRANSMIT_SERVICE_ID, DOIP_E_INVALID_PDU_SDU_ID, E_NOT_OK);

    /** @req SWS_DoIP_00164 */
    VALIDATE_W_RV((DoIPPduRTxInfoPtr != NULL_PTR) ,DOIP_TP_TRANSMIT_SERVICE_ID, DOIP_E_PARAM_POINTER, E_NOT_OK);

    for (conIndex = 0u; conIndex < DOIP_TCP_CON_NUM; conIndex++) {
        /* This is required since SBL is sending routine control OK without a request */
        /** @req SWS_DoIP_00130 */
        if ((CONNECTION_REGISTERED == DoIP_TcpConAdmin[conIndex].socketState)) {
            break;
        }
    }
    /** @req SWS_DoIP_00226 */
    VALIDATE_W_RV((conIndex != DOIP_TCP_CON_NUM), DOIP_TP_TRANSMIT_SERVICE_ID, DOIP_E_INVALID_SOCKET_STATE, E_NOT_OK);

    SchM_Enter_DoIP_EA_0();

    /** @req SWS_DoIP_00230 */
    if (BUFFER_IDLE == DoIP_TcpConAdmin[conIndex].txBufferState) {

        pduType = DoIP_ConfigPtr->DoIP_Channel[chIndex].DoIP_PduTransmitRef->DoIP_TxPduType;

        if (DOIP_TPPDU == pduType) {

            bytesToTransmit = MSG_LEN_INCL_PL_LEN_FIELD + SA_AND_TA_LEN + DoIPPduRTxInfoPtr->SduLength;

            txPdu.SduLength = (PduLengthType)bytesToTransmit;

            soadTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpSoADTxPduRef;
            /** @req SWS_DoIP_00228 */
            retVal = SoAd_TpTransmit(soadTxPduId, &txPdu);
            if (E_OK == retVal) {
                sourceAddress = DoIP_ConfigPtr->DoIP_Channel[chIndex].DoIP_ChannelTARef->DoIP_TargetAddrValue;
                targetAddress = DoIP_ConfigPtr->DoIP_Channel[chIndex].DoIP_ChannelSARef->DoIP_TesterSA;
                doipLoTxPduId = DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpTxPduRef;

                /** @req SWS_DoIP_00131 */ /** @req SWS_DoIP_00173 */
                /** @req SWS_DoIP_00284 */
                DoIP_TcpConAdmin[conIndex].txBuffer[0u] = PROTOCOL_VERSION;
                DoIP_TcpConAdmin[conIndex].txBuffer[1u] = ~(uint8)(PROTOCOL_VERSION);
                DoIP_TcpConAdmin[conIndex].txBuffer[2u] = (uint8)(PL_TYPE_DIAG_MSG >> SHIFT_BY_ONE_BYTE);
                DoIP_TcpConAdmin[conIndex].txBuffer[3u] = (uint8)PL_TYPE_DIAG_MSG;
                DoIP_TcpConAdmin[conIndex].txBuffer[4u] = 0u;
                DoIP_TcpConAdmin[conIndex].txBuffer[5u] = 0u;
                DoIP_TcpConAdmin[conIndex].txBuffer[6u] = (SA_AND_TA_LEN + DoIPPduRTxInfoPtr->SduLength) >> SHIFT_BY_ONE_BYTE;
                DoIP_TcpConAdmin[conIndex].txBuffer[7u] = (uint8) (SA_AND_TA_LEN + DoIPPduRTxInfoPtr->SduLength);
                DoIP_TcpConAdmin[conIndex].txBuffer[8u+0u] = sourceAddress >> SHIFT_BY_ONE_BYTE;
                DoIP_TcpConAdmin[conIndex].txBuffer[8u+1u] = (uint8) sourceAddress;
                DoIP_TcpConAdmin[conIndex].txBuffer[8u+2u] = targetAddress >> SHIFT_BY_ONE_BYTE;
                DoIP_TcpConAdmin[conIndex].txBuffer[8u+3u] = (uint8) targetAddress;

                DoIP_TcpConAdmin[conIndex].txBytesToTransmit = bytesToTransmit;
                DoIP_TcpConAdmin[conIndex].txBytesCopied = MSG_LEN_INCL_PL_LEN_FIELD + SA_AND_TA_LEN;
                DoIP_TcpConAdmin[conIndex].txBytesTransmitted = 0u;
                DoIP_TcpConAdmin[conIndex].txPduIdUnderProgress = doipLoTxPduId;
                DoIP_TcpConAdmin[conIndex].txBufferState = BUFFER_LOCK;
                DoIP_TcpConAdmin[conIndex].channelIndex = chIndex;

                DoIP_TcpConAdmin[conIndex].uLMsgTxInProgress = TRUE;

            }
        }else {
            retVal = E_NOT_OK;
        }
    } else {

        /** @req SWS_DoIP_00230 */
        /* Push the transmission related data into the queue */
        if ((FALSE == DoIP_TcpQueueAdmin[conIndex].tpTransmitQueueActive) &&
                (FALSE ==  DoIP_TcpQueueAdmin[conIndex].diagAckQueueActive)) {
            DoIP_TcpQueueAdmin[conIndex].tpTransmitQueueActive = TRUE;
            DoIP_TcpQueueAdmin[conIndex].txPduId = DoIPPduRTxId;
            DoIP_TcpQueueAdmin[conIndex].SduDataPtr = DoIPPduRTxInfoPtr->SduDataPtr;
            DoIP_TcpQueueAdmin[conIndex].SduLength = DoIPPduRTxInfoPtr->SduLength;
            retVal = E_OK;
        } else {
            DOIP_DET_REPORTERROR(DOIP_CREATE_AND_SEND_D_ACK_ID, DOIP_E_BUFFER_BUSY);
            retVal = E_NOT_OK;
        }
    }

    SchM_Exit_DoIP_EA_0();

    return retVal;
}

/**
 * @brief The service DoIP_TpCancelTransmit Requests cancellation of an ongoing transmission of a PDU in a lower layer
 * @param[in] DoIPPduRTxId - Identification of the PDU to be canceled
 * @return Result of the function
 */
/** @req SWS_DoIP_00023 */
Std_ReturnType DoIP_TpCancelTransmit(PduIdType DoIPPduRTxId) {
    SoAd_SoConIdType conIndex;
    Std_ReturnType retVal = FALSE;
    uint8 chIndex;

    /** @req SWS_DoIP_00166 */
    VALIDATE_W_RV((DoIP_Status == DOIP_INIT), DOIP_TP_CANCEL_TRANSMIT_SERVICE_ID, DOIP_E_UNINIT, E_NOT_OK);

    for (chIndex = 0u; chIndex < DOIP_CHANNEL_COUNT; chIndex++) {
        if (DoIP_ConfigPtr->DoIP_Channel[chIndex].DoIP_PduTransmitRef->DoIP_TxPduRef == DoIPPduRTxId) {
            break;
        }
    }

    /** @req SWS_DoIP_00167 */
    VALIDATE_W_RV((chIndex != DOIP_CHANNEL_COUNT), DOIP_TP_CANCEL_TRANSMIT_SERVICE_ID, DOIP_E_INVALID_PDU_SDU_ID, E_NOT_OK);

    for (conIndex = 0u; conIndex < DOIP_TCP_CON_NUM;conIndex++) {
        if((CONNECTION_REGISTERED == DoIP_TcpConAdmin[conIndex].socketState) && (DoIP_TcpConAdmin[conIndex].channelIndex == chIndex)){
            break;
        }
    }
    VALIDATE_W_RV((conIndex != DOIP_TCP_CON_NUM), DOIP_TP_CANCEL_TRANSMIT_SERVICE_ID, DOIP_E_INVALID_PDU_SDU_ID, E_NOT_OK);

    /** @req SWS_DoIP_00257 */
    if (E_OK == SoAd_TpCancelTransmit(DoIP_TcpConAdmin[conIndex].txPduIdUnderProgress)) {
        freeTcpTxBuffer(conIndex);
        retVal = TRUE;
    }

    return retVal;
}


/**
 * @brief The service DoIP_IfTransmit Requests transmission of a PDU
 * @note Non-Reentrant
 * @param[in] id - Identification of the PDU to be canceled
 * @param[in] info - Identification of the PDU to be canceled
 * @return Result of the function
 */
/** @req SWS_DoIP_00277 */
/*lint --e{715} MISRA:OTHER:unsupported argument:[MISRA 2012 Rule 2.7, advisory] */
Std_ReturnType DoIP_IfTransmit(PduIdType id, const PduInfoType* info) {
    VALIDATE_W_RV((DoIP_Status == DOIP_INIT), DOIP_IF_TRANSMIT_SERVICE_ID, DOIP_E_UNINIT, E_NOT_OK);

    (void)id;
    /* Not implemented */
    return E_OK;
}


/**
 * @brief The service DoIP_IfCancelTransmit Requests cancellation of an ongoing transmission of a PDU in
 *        a lower layer communication module
 * @note Non-Reentrant
 * @param[in] id - Identification of the PDU to be canceled
 * @return Result of the function
 */
/** @req SWS_DoIP_00278*/
Std_ReturnType DoIP_IfCancelTransmit(PduIdType id) {
    VALIDATE_W_RV((DoIP_Status == DOIP_INIT), DOIP_IF_CANCEL_TRANSMIT_SERVICE_ID, DOIP_E_UNINIT, E_NOT_OK);

    /* Not implemented */
    return E_OK;
}

/**
 * @brief The service DoIP_Init Initializes all global variables of the DoIP module.
 *        After return of this service,  the DoIP module is operational
 * @note Non-Reentrant
 * @param[in] DoIPConfigPtr - Pointer to the configuration data of the DoIP module
 */
/** @req SWS_DoIP_00026 */
void DoIP_Init(const DoIP_ConfigType* DoIPConfigPtr) {
    SoAd_SoConIdType conIndex;

    VALIDATE((DoIPConfigPtr != NULL_PTR) ,DOIP_INIT_SERVICE_ID, DOIP_E_PARAM_POINTER);

    DoIP_ConfigPtr = DoIPConfigPtr;

    DoIP_Status = DOIP_INIT;
    DoIP_ActivationLineStatus = ACTIVATION_LINE_INACTIVE;

    for (conIndex = 0u; conIndex < DOIP_TCP_CON_NUM; conIndex++) {
        resetTcpConnection(conIndex);
    }

    for (conIndex = 0u; conIndex < DOIP_UDP_CON_NUM; conIndex++) {
        resetUdpConnection(conIndex);
    }

    DoIP_VinUpdated = FALSE;
    DoIP_EidUpdated = FALSE;
#if (DOIPTGRGIDSYNCCALLBACK_CONFIGURED == TRUE) && (DOIP_VIN_GID_MASTER == STD_ON)
    DoIP_GidSyncDone = FALSE;
#endif

    DoIP_PendingRoutActivFlag = FALSE;
}

/**
 * @brief The service DoIP_GetVersionInfo Returns the version information of this module
 * @note Reentrant
 * @param[out] versioninfo - Pointer to where to store the version information of this module
 */
/** @req SWS_DoIP_00027 */
void DoIP_GetVersionInfo(Std_VersionInfoType* versioninfo) {
    /** @req SWS_DoIP_00172 */
    VALIDATE((NULL != versioninfo),DOIP_GET_VERSION_INFO_SERVICE_ID,DOIP_E_PARAM_POINTER);

    versioninfo->vendorID = DOIP_VENDOR_ID;
    versioninfo->moduleID = DOIP_MODULE_ID;
    versioninfo->sw_major_version = DOIP_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = DOIP_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = DOIP_SW_PATCH_VERSION;
}

/**
 * @brief The service DoIP_SoAdTpCopyTxData Acquires the transmit data of an I-PDU segment
 * @note Reentrant
 * @param[in] id - Identifier of the PDU to be transmitted
 * @param[in] info - Provides the destination buffer and the number of bytes to be copied
 * @param[in] retry - Used to acknowledge transmitted data or to retransmit data after transmission problems
 * @param[out] availableDataPtr - Indicates the remaining number of bytes that are available in the upper layer module's Tx buffer
 * @return Result of the function
 */
/** @req SWS_DoIP_00031 */
/*lint --e{818} MISRA:STANDARDIZED_INTERFACE:Pointer parameter 'retry' could be declared as pointing to const:[MISRA 2004 Rule 16.7, advisory] */
BufReq_ReturnType DoIP_SoAdTpCopyTxData(PduIdType id, const PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr) {
    PduInfoType pudInfo;
    PduLengthType sduLenghtToTransmit;
    SoAd_SoConIdType conIndex;
    PduIdType pduRTxId;
    DoIP_PduType pduType;
    BufReq_ReturnType retVal;
    uint8 chIndex;

    pudInfo.SduDataPtr = info->SduDataPtr;
    pudInfo.SduLength = info->SduLength;
    retVal = BUFREQ_E_NOT_OK;

    /** @req SWS_DoIP_00175 */
    VALIDATE_W_RV((DoIP_Status == DOIP_INIT), DOIP_SOAD_TP_COPY_TX_DATA_SERVICE_ID, DOIP_E_UNINIT, BUFREQ_E_NOT_OK);

    for (conIndex = 0u; conIndex < DOIP_TCP_CON_NUM; conIndex++) {
        if (DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpTxPduRef == id) {
            /* Match! */
            break;
        }
    }

    /** @req SWS_DoIP_00176 */
    VALIDATE_W_RV(conIndex != DOIP_TCP_CON_NUM, DOIP_SOAD_TP_COPY_TX_DATA_SERVICE_ID, DOIP_E_INVALID_PDU_SDU_ID, BUFREQ_E_NOT_OK);

    /** @req SWS_DoIP_00177 */
    VALIDATE_W_RV((info != NULL_PTR), DOIP_SOAD_TP_COPY_TX_DATA_SERVICE_ID, DOIP_E_PARAM_POINTER, BUFREQ_E_NOT_OK);

    VALIDATE_W_RV((availableDataPtr != NULL_PTR), DOIP_SOAD_TP_COPY_TX_DATA_SERVICE_ID, DOIP_E_PARAM_POINTER, BUFREQ_E_NOT_OK);

    SchM_Enter_DoIP_EA_0();
    if ((DoIP_TcpConAdmin[conIndex].uLMsgTxInProgress != TRUE) ||
        ((TRUE == DoIP_TcpConAdmin[conIndex].uLMsgTxInProgress) && (CONNECTION_REGISTERED == DoIP_TcpConAdmin[conIndex].socketState))) {

        if (BUFFER_LOCK == DoIP_TcpConAdmin[conIndex].txBufferState) {

            if (DoIP_TcpConAdmin[conIndex].txPduIdUnderProgress == id) {

                sduLenghtToTransmit = pudInfo.SduLength;

                if (sduLenghtToTransmit == 0u) {
                    /** @req SWS_DoIP_00224 */
                    *availableDataPtr = (PduLengthType) ( DoIP_TcpConAdmin[conIndex].txBytesToTransmit - DoIP_TcpConAdmin[conIndex].txBytesTransmitted );
                    retVal = BUFREQ_OK;
                } else if (sduLenghtToTransmit > (DoIP_TcpConAdmin[conIndex].txBytesToTransmit - DoIP_TcpConAdmin[conIndex].txBytesTransmitted)) {
                    retVal = BUFREQ_NOT_OK;
                } else {
                    /** @req SWS_DoIP_00225 */
                    if (0 == DoIP_TcpConAdmin[conIndex].txBytesTransmitted) {
                        memcpy(pudInfo.SduDataPtr, DoIP_TcpConAdmin[conIndex].txBuffer, DoIP_TcpConAdmin[conIndex].txBytesCopied);
                        DoIP_TcpConAdmin[conIndex].txBytesTransmitted += DoIP_TcpConAdmin[conIndex].txBytesCopied;
                        /*lint -e9016 MISRA:OTHER:pointer arithmetic other than array indexing used:[MISRA 2012 Rule 18.4, advisory] */
                        pudInfo.SduDataPtr += DoIP_TcpConAdmin[conIndex].txBytesTransmitted;
                        pudInfo.SduLength  -= (PduLengthType)DoIP_TcpConAdmin[conIndex].txBytesTransmitted;
                        retVal = BUFREQ_OK;
                    }

                    if (DoIP_TcpConAdmin[conIndex].txBytesTransmitted < DoIP_TcpConAdmin[conIndex].txBytesToTransmit) {

                        retVal = BUFREQ_NOT_OK;

                        chIndex = DoIP_TcpConAdmin[conIndex].channelIndex;

                        if (chIndex != INVALID_CHANNEL_INDEX) {

                            if (DoIP_ConfigPtr->DoIP_Channel[chIndex].DoIP_PduTransmitRef != NULL) {

                                pduType = DoIP_ConfigPtr->DoIP_Channel[chIndex].DoIP_PduTransmitRef->DoIP_TxPduType;

                                if (DOIP_TPPDU == pduType) {

                                    pduRTxId = DoIP_ConfigPtr->DoIP_Channel[chIndex].DoIP_UpperLayerTxPduId;
                                    /** @req SWS_DoIP_00232 */ /** @req SWS_DoIP_00233 */
                                    retVal = PduR_DoIPTpCopyTxData(pduRTxId, &pudInfo, /* retry */NULL, availableDataPtr);

                                    if (retVal == BUFREQ_OK) {
                                        DoIP_TcpConAdmin[conIndex].txBytesTransmitted += pudInfo.SduLength;
                                    }
                                } else {
                                    DOIP_DET_REPORTERROR(DOIP_HANDLE_VEHICLE_ID_REQ_ID, DOIP_E_UNEXPECTED_EXECUTION);
                                }
                            } else {
                                retVal = BUFREQ_OK;
                            }
                        } else {
                            /* We should not end up here */
                            DOIP_DET_REPORTERROR(DOIP_HANDLE_VEHICLE_ID_REQ_ID, DOIP_E_UNEXPECTED_EXECUTION);
                        }
                    }

                    if (BUFREQ_OK == retVal) {
                        /** @req SWS_DoIP_00254 */
                        *availableDataPtr = (PduLengthType)(DoIP_TcpConAdmin[conIndex].txBytesToTransmit - DoIP_TcpConAdmin[conIndex].txBytesTransmitted);
                    }
                }
            } else {
                retVal = BUFREQ_NOT_OK;
            }
        } else {
            retVal = BUFREQ_NOT_OK;
        }
    } else {
        retVal = BUFREQ_NOT_OK;
    }

    SchM_Exit_DoIP_EA_0();
    return retVal;

}

/**
 * @brief The service DoIP_SoAdTpTxConfirmation Is called after the I-PDU has been transmitted on its network.
 *        The result indicates whether the transmission was successful or not
 * @note Reentrant
 * @param[in] id - Identification of the transmitted I-PDU
 * @param[in] result - Result of the transmission of the I-PDU
 */
/** @req SWS_DoIP_00032 */
void DoIP_SoAdTpTxConfirmation(PduIdType id, Std_ReturnType result) {
    PduIdType pduRTxId;
    SoAd_SoConIdType SoConId;
    SoAd_SoConIdType conIndex;
    uint8 chIndex;

    /** @req SWS_DoIP_00180 */
    VALIDATE((DoIP_Status == DOIP_INIT), DOIP_SOAD_TP_TX_CONFIRMATION_SERVICE_ID, DOIP_E_UNINIT);

    for (conIndex = 0u; conIndex < DOIP_TCP_CON_NUM; conIndex++) {
        if (DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpTxPduRef == id) {
            /* Match! */
            break;
        }
    }
    /** @req SWS_DoIP_00181*/
    VALIDATE((conIndex != DOIP_TCP_CON_NUM), DOIP_SOAD_TP_TX_CONFIRMATION_SERVICE_ID, DOIP_E_INVALID_PDU_SDU_ID);

    if (TRUE == DoIP_TcpConAdmin[conIndex].closeSocketIndication) {
        /** @req SWS_DoIP_00058 */
        (void)SoAd_GetSoConId(DoIP_ConfigPtr->DoIP_UdpMsg[conIndex].DoIP_UdpSoADTxPduRef, &SoConId);
        (void)SoAd_CloseSoCon(SoConId, TRUE);
        resetTcpConnection(conIndex);
    } else {

        SchM_Enter_DoIP_EA_0();

        if (BUFFER_LOCK == DoIP_TcpConAdmin[conIndex].txBufferState) {

            if (DoIP_TcpConAdmin[conIndex].txPduIdUnderProgress == id) {

                chIndex = DoIP_TcpConAdmin[conIndex].channelIndex;

                if (chIndex != INVALID_CHANNEL_INDEX) {

                    if (DoIP_ConfigPtr->DoIP_Channel[chIndex].DoIP_PduTransmitRef != NULL) {

                        pduRTxId = DoIP_ConfigPtr->DoIP_Channel[chIndex].DoIP_UpperLayerTxPduId;

                        /** @req SWS_DoIP_00232 */ /** @req SWS_DoIP_00233 */
                        PduR_DoIPTpTxConfirmation(pduRTxId, result);

                        if (TRUE == DoIP_TcpConAdmin[conIndex].uLMsgTxInProgress) {
                            DoIP_TcpConAdmin[conIndex].uLMsgTxInProgress = FALSE;
                        }

                    } else {
                        DOIP_DET_REPORTERROR(DOIP_HANDLE_VEHICLE_ID_REQ_ID, DOIP_E_INVALID_PDU_SDU_ID);
                    }
                } else {
                    DOIP_DET_REPORTERROR(DOIP_CREATE_AND_SEND_NACK_ID, DOIP_E_INVALID_CH_INDEX);
                }
            }
        }

        /** @req SWS_DoIP_00229 */
        freeTcpTxBuffer(conIndex);
        SchM_Exit_DoIP_EA_0();
    }

}


/**
 * @brief The service DoIP_SoAdTpCopyRxData Provides the received data of an I-PDU segment (N-PDU) to the upper layer
 * @note Reentrant
 * @param[in] id - Identification of the transmitted I-PDU
 * @param[in] info - Provides the source buffer and the number of bytes to be copied
 * @param[out] bufferSizePtr - Available receive buffer after data has been copied
 * @return Result of the function
 */
/** @req SWS_DoIP_00033 */  /** @req SWS_DoIP_00219 */
BufReq_ReturnType DoIP_SoAdTpCopyRxData(PduIdType id,const PduInfoType* info,PduLengthType* bufferSizePtr) {
    static DoIPPayloadType payloadLength;
    static DoIPPayloadType sduDataIndex;
    PduLengthType sduLengthToProcess;
    SoAd_SoConIdType conIndex;
    BufReq_ReturnType ret;
    Std_ReturnType protocolHeader;

    /** @req SWS_DoIP_00183 */
    VALIDATE_W_RV((DoIP_Status == DOIP_INIT), DOIP_SOAD_TP_COPY_RX_DATA_SERVICE_ID, DOIP_E_UNINIT, BUFREQ_E_NOT_OK);

    for (conIndex = 0u; conIndex < DOIP_TCP_CON_NUM; conIndex++) {
        if (DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpRxPduRef == id) {
            break;
        }
    }
    /** @req SWS_DoIP_00036 */
    VALIDATE_W_RV((conIndex != DOIP_TCP_CON_NUM), DOIP_SOAD_TP_COPY_RX_DATA_SERVICE_ID, DOIP_E_INVALID_PDU_SDU_ID, BUFREQ_E_NOT_OK);

    /** @req SWS_DoIP_00184 */
    VALIDATE_W_RV((bufferSizePtr != NULL_PTR), DOIP_SOAD_TP_COPY_RX_DATA_SERVICE_ID, DOIP_E_PARAM_POINTER, BUFREQ_E_NOT_OK);

    VALIDATE_W_RV((info != NULL_PTR), DOIP_SOAD_TP_COPY_RX_DATA_SERVICE_ID, DOIP_E_PARAM_POINTER, BUFREQ_E_NOT_OK);

    sduLengthToProcess = info->SduLength;
    if ((BUFFER_IDLE == DoIP_TcpConRxBufferAdmin.bufferState) || \
            ((BUFFER_LOCK_START == DoIP_TcpConRxBufferAdmin.bufferState) && (id == DoIP_TcpConRxBufferAdmin.pduIdUnderProgress))) {
        if (0 == sduLengthToProcess) {
            /** @req SWS_DoIP_00208 */
            *bufferSizePtr = TCP_RX_BUFF_SIZE;
            ret = BUFREQ_OK;
        } else {
            if (sduLengthToProcess <= DoIP_TcpRxRemainingBufferSize) {
                protocolHeader = E_OK;
                if (TCP_RX_BUFF_SIZE == DoIP_TcpRxRemainingBufferSize) {
                    /** @req SWS_DoIP_00102 */
                    if ((PROTOCOL_VERSION == info->SduDataPtr[0u]) && (PROTOCOL_VERSION == (uint8)(~info->SduDataPtr[1u]))) {
                        protocolHeader = E_OK;
                        payloadLength = GET_PL_LEN_FROM_DOIP_MSG_PTR(info->SduDataPtr);
                        sduDataIndex = 0u;
                    } else {
                        protocolHeader = E_NOT_OK;
                    }
                }

                if (E_OK == protocolHeader) {
                    /** @req SWS_DoIP_00209 */ /** @req SWS_DoIP_00214 */
                    memcpy(&(DoIP_TcpConRxBufferAdmin.buffer[sduDataIndex]), &(info->SduDataPtr[0u]), sduLengthToProcess);
                    sduDataIndex = sduDataIndex + sduLengthToProcess;

                    if ((sduDataIndex - MSG_LEN_INCL_PL_LEN_FIELD) == payloadLength) {
                        handleTcpRx(conIndex, DoIP_TcpConRxBufferAdmin.buffer);
                        freeTcpRxBuffer();

                        sduDataIndex = 0u;
                        payloadLength = 0u;
                        DoIP_TcpRxRemainingBufferSize = TCP_RX_BUFF_SIZE ;
                    } else {
                        DoIP_TcpRxRemainingBufferSize = DoIP_TcpRxRemainingBufferSize - sduLengthToProcess;
                        *bufferSizePtr = (PduLengthType) DoIP_TcpRxRemainingBufferSize;
                    }

                    ret = BUFREQ_OK;
                } else {
                    ret = BUFREQ_E_NOT_OK;
                }
            } else {
                /** @req SWS_DoIP_00210 */
                ret = BUFREQ_E_NOT_OK;
            }
        }
    } else {
        ret = BUFREQ_E_NOT_OK;
    }

    return ret;
}

/**
 * @brief The service DoIP_SoAdTpStartOfReception Is called at the start of receiving an N-SDU
 * @note Reentrant
 * @param[in] id - Identification of the I-PDU.
 * @param[in] info - Pointer to a PduInfoType structure containing the payload data and payload length of
 *            of the first frame or the single frame
 * @param[in] TpSduLength - Total length of the N-SDU to be received
 * @param[out] bufferSizePtr - Available receive buffer in the receiving module
 * @return Result of the function
 */
/** @req SWS_DoIP_00037 */
BufReq_ReturnType DoIP_SoAdTpStartOfReception(PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr) {
    SoAd_SoConIdType conIndex;
    BufReq_ReturnType ret;

    /** @req SWS_DoIP_00186 */
    VALIDATE_W_RV(DoIP_Status == DOIP_INIT, DOIP_SOAD_TP_START_OF_RECEPTION_SERVICE_ID, DOIP_E_UNINIT, BUFREQ_E_NOT_OK);

    /** @req SWS_DoIP_00187 */
    for (conIndex = 0u; conIndex < DOIP_TCP_CON_NUM; conIndex++) {
        if (DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpRxPduRef == id) {
            break;
        }
    }
    VALIDATE_W_RV(conIndex != DOIP_TCP_CON_NUM, DOIP_SOAD_TP_START_OF_RECEPTION_SERVICE_ID, DOIP_E_INVALID_PDU_SDU_ID, BUFREQ_E_NOT_OK);

    /** @req SWS_DoIP_00188 */
    VALIDATE_W_RV((bufferSizePtr != NULL_PTR), DOIP_SOAD_TP_START_OF_RECEPTION_SERVICE_ID, DOIP_E_PARAM_POINTER, BUFREQ_E_NOT_OK);

    /** @req SWS_DoIP_00189 */
    VALIDATE_W_RV((TpSduLength != 0u), DOIP_SOAD_TP_START_OF_RECEPTION_SERVICE_ID, DOIP_E_INVALID_PARAMETER, BUFREQ_E_NOT_OK);

    VALIDATE_W_RV((info != NULL_PTR), DOIP_SOAD_TP_START_OF_RECEPTION_SERVICE_ID, DOIP_E_PARAM_POINTER, BUFREQ_E_NOT_OK);

    if (BUFFER_IDLE == DoIP_TcpConRxBufferAdmin.bufferState) {

        /** @req SWS_DoIP_00004 */ /** @req SWS_DoIP_00005 */ /** @req SWS_DoIP_00006 */
        if ((PROTOCOL_VERSION == info->SduDataPtr[0u]) && (PROTOCOL_VERSION == (uint8)(~info->SduDataPtr[1u]))) {
            if (info->SduLength <= TCP_RX_BUFF_SIZE) {
                /** @req SWS_DoIP_00207 */
                DoIP_TcpConRxBufferAdmin.bufferState = BUFFER_LOCK_START;
                DoIP_TcpConRxBufferAdmin.pduIdUnderProgress = id;

                *bufferSizePtr = TCP_RX_BUFF_SIZE;
                DoIP_TcpRxRemainingBufferSize = TCP_RX_BUFF_SIZE;

                ret = BUFREQ_OK;
            } else {
                createAndSendNackTp(conIndex, ERROR_OUT_OF_MEMORY);
                ret = BUFREQ_OVFL;
            }
        } else {

             /** @req SWS_DoIP_00012 */ /** @req SWS_DoIP_00013 */
            createAndSendNackTp(conIndex, ERROR_INCORRECT_PATTERN_FORMAT);
            closeSocket(TCP_TYPE, conIndex, DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpSoADTxPduRef);
            ret = BUFREQ_NOT_OK;
        }
    }
    else {
        ret = BUFREQ_NOT_OK;
    }

    return ret;
}

/**
 * @brief The service DoIP_SoAdTpRxIndication Is called after an I-PDU has been received via the TP API.
 * @note Reentrant
 * @param[in] id - Identification of the received I-PDU
 * @param[in] result - Result of the reception
 */
/** @req SWS_DoIP_00038 */
void DoIP_SoAdTpRxIndication(PduIdType id, Std_ReturnType result) {
    SoAd_SoConIdType conIndex;

    /** @req SWS_DoIP_00190 */
    VALIDATE((DoIP_Status == DOIP_INIT), DOIP_SOAD_TP_RX_INDICATION_SERVICE_ID, DOIP_E_UNINIT);

    /** @req SWS_DoIP_00191 */
    for (conIndex = 0u; conIndex < DOIP_TCP_CON_NUM; conIndex++) {
        if (DoIP_ConfigPtr->DoIP_TcpMsg[conIndex].DoIP_TcpRxPduRef == id) {
            break;
        }
    }
    VALIDATE(conIndex != DOIP_TCP_CON_NUM, DOIP_SOAD_TP_RX_INDICATION_SERVICE_ID, DOIP_E_INVALID_PDU_SDU_ID);

    /** @req SWS_DoIP_00200 */
    freeTcpRxBuffer();
}

/**
 * @brief The service DoIP_SoAdIfRxIndication Provides an indication of a received PDU from a lower layer communication interface module
 * @note Reentrant for different PduIds. Non reentrant for the same PduId
 * @param[in] RxPduId- ID of the received PDU
 * @param[in] PduInfoPtr- Contains the length and a pointer to a buffer of the received PDU.
 */
/** @req SWS_DoIP_00244 */
/*lint -e{9011} MISRA:OTHER:more than one 'break' terminates loop:[MISRA 2012 Rule 15.4, advisory] */
void DoIP_SoAdIfRxIndication(PduIdType RxPduId,const PduInfoType* PduInfoPtr) {
    DoIPPayloadType payloadLength;
    DoIPPayloadType sduDataIndex;
    DoIPPayloadType sduLengthToProcess;
    SoAd_SoConIdType conIndex;


    /* Check for DoIP initialization */
    /** @req SWS_DoIP_00246 */
     VALIDATE((DoIP_Status == DOIP_INIT), DOIP_SOAD_IF_RX_INDICATION_SERVICE_ID, DOIP_E_UNINIT);

    /** @req SWS_DoIP_00247 */
    for (conIndex = 0u; conIndex < DOIP_UDP_CON_NUM; conIndex++) {
        if(DoIP_ConfigPtr->DoIP_UdpMsg[conIndex].DoIP_UdpRxPduRef == RxPduId) {
            break;
        }
    }
    VALIDATE((conIndex != DOIP_UDP_CON_NUM), DOIP_SOAD_IF_RX_INDICATION_SERVICE_ID, DOIP_E_INVALID_PDU_SDU_ID);

    if (0u == PduInfoPtr->SduLength) {
        DOIP_DET_REPORTERROR(DOIP_SOAD_IF_RX_INDICATION_SERVICE_ID, DOIP_E_INVALID_PARAMETER);
    } else {
        sduLengthToProcess = PduInfoPtr->SduLength;
        sduDataIndex = 0u;

        while (sduLengthToProcess > 0u) {

            if ((DoIP_UdpConAdmin[conIndex].rxBufferStartIndex < DoIP_UdpConAdmin[conIndex].rxBufferEndIndex) ||
                    ((uint8)(DoIP_UdpConAdmin[conIndex].rxBufferStartIndex - DoIP_UdpConAdmin[conIndex].rxBufferEndIndex) < (uint8)(DOIP_MAX_UDP_REQUEST_MESSAGE - 1u)) ||
                        ((DoIP_UdpConAdmin[conIndex].rxBufferStartIndex == DoIP_UdpConAdmin[conIndex].rxBufferEndIndex) && (FALSE == DoIP_UdpConAdmin[conIndex].rxBufferPresent))) {

                /** @req SWS_DoIP_00004 */ /** @req SWS_DoIP_00005 */ /** @req SWS_DoIP_00006 */ /** @req SWS_DoIP_00015 */
                if ( ((PROTOCOL_VERSION == PduInfoPtr->SduDataPtr[sduDataIndex]) && (PROTOCOL_VERSION == (uint8)(~PduInfoPtr->SduDataPtr[sduDataIndex + 1u]))) ||
                      ((PROTOCOL_VERSION_VID == PduInfoPtr->SduDataPtr[sduDataIndex]) && (PROTOCOL_VERSION_VID == (uint8)(~PduInfoPtr->SduDataPtr[sduDataIndex + 1u])))) {

                    payloadLength = GET_PL_LEN_FROM_DOIP_MSG_PTR(PduInfoPtr->SduDataPtr);

                    if ( (payloadLength + MSG_LEN_INCL_PL_LEN_FIELD) <= UDP_RX_BUFF_SIZE) {

                        DoIP_UdpConAdmin[conIndex].rxPduIdUnderProgress = RxPduId;

                        /** @req SWS_DoIP_00197 */
                        memcpy(&(DoIP_UdpConAdmin[conIndex].rxBuffer[DoIP_UdpConAdmin[conIndex].rxBufferStartIndex][0u]), &(PduInfoPtr->SduDataPtr[sduDataIndex]), (payloadLength + MSG_LEN_INCL_PL_LEN_FIELD));

                        if (DoIP_UdpConAdmin[conIndex].rxBufferStartIndex < (DOIP_MAX_UDP_REQUEST_MESSAGE - 1u)) {
                            DoIP_UdpConAdmin[conIndex].rxBufferStartIndex ++;
                        } else {
                            DoIP_UdpConAdmin[conIndex].rxBufferStartIndex = UDP_RX_BUFFER_RESET_INDEX;
                        }

                        if ((UDP_RX_BUFFER_RESET_INDEX == DoIP_UdpConAdmin[conIndex].rxBufferStartIndex) && (UDP_RX_BUFFER_RESET_INDEX == DoIP_UdpConAdmin[conIndex].rxBufferEndIndex)) {
                            DoIP_UdpConAdmin[conIndex].rxBufferPresent = TRUE;
                        }

                        sduDataIndex = sduDataIndex + (MSG_LEN_INCL_PL_LEN_FIELD + payloadLength);
                        sduLengthToProcess = sduLengthToProcess - (MSG_LEN_INCL_PL_LEN_FIELD + payloadLength);
                    } else {
                        createAndSendNackIf(conIndex, ERROR_MESSAGE_TOO_LARGE);
                        break;
                    }
                } else {
                    /** @req SWS_DoIP_00012 */  /** @req SWS_DoIP_00013 */
                    createAndSendNackIf(conIndex, ERROR_INCORRECT_PATTERN_FORMAT);
                    closeSocket(UDP_TYPE, conIndex, DoIP_ConfigPtr->DoIP_UdpMsg[conIndex].DoIP_UdpSoADTxPduRef);
                    break;
                }
            } else {

                /** @req SWS_DoIP_00004 */ /** @req SWS_DoIP_00005 */ /** @req SWS_DoIP_00006 */ /** @req SWS_DoIP_00015 */
                if ( ((PROTOCOL_VERSION == PduInfoPtr->SduDataPtr[sduDataIndex]) && (PROTOCOL_VERSION == (uint8)(~PduInfoPtr->SduDataPtr[sduDataIndex + 1u]))) ||
                      ((PROTOCOL_VERSION_VID == PduInfoPtr->SduDataPtr[sduDataIndex]) && (PROTOCOL_VERSION_VID == (uint8)(~PduInfoPtr->SduDataPtr[sduDataIndex + 1u])))) {
                    /** @req SWS_DoIP_00276 */
                    createAndSendNackIf(conIndex, ERROR_OUT_OF_MEMORY);
                    break;
                }
            }
        }
    }
}

/**
 * @brief The service DoIP_SoAdIfTxConfirmation Confirms the transmission of a PDU, or the failure to transmit a PDU
 * @note Reentrant for different PduIds. Non reentrant for the same PduId
 * @param[in] TxPduId - ID of the PDU that has been transmitted
 */
/** @req SWS_DoIP_00245 */
void DoIP_SoAdIfTxConfirmation(PduIdType TxPduId) {
    SoAd_SoConIdType conIndex;

    /** @req SWS_DoIP_00249 */
    VALIDATE((DoIP_Status == DOIP_INIT), DOIP_SOAD_IF_TX_CONFIRMATION_SERVICE_ID, DOIP_E_UNINIT);

    /** @req SWS_DoIP_00250 */
    for (conIndex = 0u; conIndex < DOIP_UDP_CON_NUM; conIndex++) {
        if (DoIP_ConfigPtr->DoIP_UdpMsg[conIndex].DoIP_UdpTxPduRef == TxPduId) {
            /* Match! */
            break;
        }
    }
    VALIDATE((conIndex != DOIP_UDP_CON_NUM), DOIP_SOAD_IF_TX_CONFIRMATION_SERVICE_ID, DOIP_E_INVALID_PDU_SDU_ID);

    /** @req SWS_DoIP_00199 */
    freeUdpTxBuffer(conIndex);
}


/**
 * @brief The service DoIP_SoConModeChg Provides notification about a SoAd socket connection state change
 * @note Reentrant for different SoConIds. Non reentrant for the same SoConId.
 * @param[in] SoConId - socket connection index specifying the socket connection with the mode change
 * @param[in] Mode - new mode
 */
/** @req SWS_DoIP_0039 */
void DoIP_SoConModeChg(SoAd_SoConIdType SoConId, SoAd_SoConModeType Mode) {
    SoAd_SoConIdType conIndex;
    Std_ReturnType ret;
    DoIP_Internal_ConType con;

    /** @req SWS_DoIP_00193 */
    VALIDATE((DoIP_Status == DOIP_INIT), DOIP_SO_CON_MODE_CHG_SERVICE_ID, DOIP_E_UNINIT);

    ret = getConTypeConIndexFromSoConId(SoConId, &con, &conIndex);

    if (E_OK == ret) {
        switch (Mode) {
        case SOAD_SOCON_ONLINE:
        case SOAD_SOCON_RECONNECT:
            /* Note: Even though specification states if ONLINE is reported then connection has to
             * be established. But SWS_SOAD_00686 states TCP always reports RECONNECT and
             * UDP can report RECONNECT if either remote IP is 0.0.0.0 or remote port is 0.
             * RECONNECT is equivalent to ONLINE.
             */
            /** @req SWS_DoIP_00241 */
            if (TCP_TYPE == con) {
                DoIP_TcpConAdmin[conIndex].sockNr = SoConId;
                /** @req SWS_DoIP_00143 */
                DoIP_TcpConAdmin[conIndex].socketState = CONNECTION_INITIALIZED;
            } else if (UDP_TYPE == con) {
                DoIP_UdpConAdmin[conIndex].sockNr = SoConId;
                DoIP_UdpConAdmin[conIndex].socketState = CONNECTION_INITIALIZED;
                /** @req SWS_DoIP_00205 */
                DoIP_UdpConAdmin[conIndex].vehicleAnnounceTgr = TRUE;
            } else {

            }
            break;
        case SOAD_SOCON_OFFLINE:
        default:
            /** @req SWS_DoIP_00243 */
            if (TCP_TYPE == con) {
                resetTcpConnection(conIndex);
            } else {
                resetUdpConnection(conIndex);
            }
            break;
        }
    }
}


/**
 * @brief The service DoIP_LocalIpAddrAssignmentChg Is called by the SoAd if an IP address assignment related to a socket connection changes
 * @note Reentrant for different SoConIds. Non reentrant for the same SoConId.
 * @param[in] SoConId - socket connection index specifying the socket connection where the IP address assignment has changed
 * @param[in] State - state of IP address assignment
 */
/** @req SWS_DoIP_00040 */
void DoIP_LocalIpAddrAssignmentChg(SoAd_SoConIdType SoConId, TcpIp_IpAddrStateType State) {
    /** @req SWS_DoIP_00195 */
    VALIDATE((DoIP_Status == DOIP_INIT), DOIP_LOCAL_IP_ADDR_ASSIGN_CHG_SERVICE_ID, DOIP_E_UNINIT);

    /* Not implemented. Not supported in stack */
    switch (State) {
    case TCPIP_IPADDR_STATE_ASSIGNED:
        break;
    default:
        break;
    }
}

/**
 * @brief The service DoIP_ActivationLineSwitchActive Is used to notify the DoIP on a switch of the DoIPActivationLine to active
 * @note  Non Reentrant
 */
/** @req SWS_DoIP_00251 */
void DoIP_ActivationLineSwitchActive(void) {
    SoAd_SoConIdType SoConId;
    SoAd_SoConIdType conIndex;
    Std_ReturnType ret;

    /** @req SWS_DoIP_00252 */
    VALIDATE((DoIP_Status == DOIP_INIT), DOIP_ACTIVATION_LINE_SW_ACTIVE_SERVICE_ID, DOIP_E_UNINIT);

    if (ACTIVATION_LINE_INACTIVE == DoIP_ActivationLineStatus) {

        /** @req SWS_DoIP_00204 */
        ret = SoAd_GetSoConId(DoIP_ConfigPtr->DoIP_UdpMsg[0].DoIP_UdpSoADTxPduRef, &SoConId);
        if (E_OK == ret) {

#if (TCPIP_DHCP_CLIENT_ENABLED == STD_ON)
            uint8 netmask = 0u;
            TcpIp_SockAddrType DefaultRouter;
            DefaultRouter.domain = TCPIP_AF_INET;
            DefaultRouter.addr[0u] = 255u;
            DefaultRouter.addr[1u] = 255u;
            DefaultRouter.addr[2u] = 255u;
            DefaultRouter.addr[3u] = 255u;
            DefaultRouter.port = 13400u;

            (void)SoAd_RequestIpAddrAssignment(SoConId, TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP, NULL_PTR, netmask, &DefaultRouter);
            (void)SoAd_RequestIpAddrAssignment(SoConId, TCPIP_IPADDR_ASSIGNMENT_DHCP, NULL_PTR, netmask, &DefaultRouter);
#endif
            for (conIndex = 0u; conIndex < DOIP_UDP_CON_NUM; conIndex++) {

                ret = SoAd_GetSoConId(DoIP_ConfigPtr->DoIP_UdpMsg[conIndex].DoIP_UdpSoADTxPduRef, &SoConId);

                if (E_OK == ret) {
                    /** @req SWS_DoIP_00003 */
                    if (E_OK == SoAd_OpenSoCon(SoConId)) {
                        /** @req SWS_DoIP_00201 */
                        DoIP_ActivationLineStatus = ACTIVATION_LINE_ACTIVE;
                    } else {
                        /** @req SWS_DoIP_00201 */
                        DoIP_ActivationLineStatus = ACTIVATION_LINE_INACTIVE;
                    }
                } else {
                    /* Do nothing */
                }
            }
        }
    }
}

/**
 * @brief The service DoIP_ActivationLineSwitchInactive used to notify the DoIP on a
 *        switch of the DoIPActivationLine to inactive. API is supported 4.3.0 onwards
 * @note None Non Reentrant
 */
/** @req 4.3.1 SWS_DoIP_91001 */
void DoIP_ActivationLineSwitchInactive(void) {
    SoAd_SoConIdType SoConId;
    SoAd_SoConIdType conIndex;

    /** @req 4.3.1 SWS_DoIP_00285 */
    VALIDATE((DoIP_Status == DOIP_INIT), DOIP_ACTIVATION_LINE_SW_INACTIVE_SERVICE_ID, DOIP_E_UNINIT);

    /** @req SWS_DoIP_00201 */
    DoIP_ActivationLineStatus = ACTIVATION_LINE_INACTIVE;
#if (DOIPTGRGIDSYNCCALLBACK_CONFIGURED == TRUE) && (DOIP_VIN_GID_MASTER == STD_ON)
    DoIP_GidSyncDone = FALSE;
#endif

    /** @req SWS_DoIP_00234 */
    for (conIndex = 0u; conIndex < DOIP_UDP_CON_NUM; conIndex++) {
        closeSocket(UDP_TYPE, conIndex, DoIP_ConfigPtr->DoIP_UdpMsg[conIndex].DoIP_UdpSoADTxPduRef);
    }

    /** @req SWS_DoIP_00235*/
    (void)SoAd_GetSoConId(DoIP_ConfigPtr->DoIP_UdpMsg[0].DoIP_UdpSoADTxPduRef, &SoConId);
    (void)SoAd_ReleaseIpAddrAssignment(SoConId);
}

/**
 * @brief The service DoIP_MainFunction Schedules the Diagnostic over IP module. (Entry point for scheduling)
 * @param(in) None
 * @param(out) None
 * @return None
 */
/** @req SWS_DoIP_00041 */
void DoIP_MainFunction(void) {
    PduInfoType tempPduInfo;
    SoAd_SoConIdType conIndex;
    boolean sendAnnouncement;
    boolean handleTimeoutFlag = FALSE;
    

    VALIDATE((DoIP_Status == DOIP_INIT), DOIP_MAIN_FUNCTION_SERVICE_ID, DOIP_E_UNINIT);
    
    /** @req SWS_DoIP_00076 */
#if (DOIPTGRGIDSYNCCALLBACK_CONFIGURED == TRUE) && (DOIP_VIN_GID_MASTER == STD_ON)
    if ((FALSE == DoIP_GidSyncDone) && (ACTIVATION_LINE_INACTIVE == DoIP_ActivationLineStatus)) {
        if (E_OK == DOIP_GETTRIGGERGIDSYNCCALLBACK_FUNCTION()) {
            DoIP_GidSyncDone = TRUE;
        }
    }
#endif

    for (conIndex = 0u; conIndex < DOIP_UDP_CON_NUM; conIndex++) {
        
        /** @req SWS_DoIP_00071 */
        sendAnnouncement = FALSE;
        if ((TRUE == DoIP_UdpConAdmin[conIndex].vehicleAnnounceTgr) || (TRUE == DoIP_UdpConAdmin[conIndex].vehicleAnnounceInProgress)) {
            
            if (DoIP_UdpConAdmin[conIndex].vehicleAnnounceInitialTime < DOIP_INITIAL_VEHICLE_ANNOUNCEMENT_TIME) {

                DoIP_UdpConAdmin[conIndex].vehicleAnnounceInitialTime +=  DOIP_MAIN_FUNCTION_PERIOD;

            }else if (DoIP_UdpConAdmin[conIndex].vehicleAnnounceRepetition < DOIP_VEHICLE_ANNOUNCEMENT_REPETITION) {

                if (TRUE == DoIP_UdpConAdmin[conIndex].vehicleAnnounceTgr) {
                    
                    DoIP_UdpConAdmin[conIndex].vehicleAnnounceTgr = FALSE;
                    DoIP_UdpConAdmin[conIndex].vehicleAnnounceInProgress = TRUE;
                    sendAnnouncement = TRUE;
                    
                } else {

                    DoIP_UdpConAdmin[conIndex].vehicleAnnounceTimeInterval += DOIP_MAIN_FUNCTION_PERIOD;
                    if (DoIP_UdpConAdmin[conIndex].vehicleAnnounceTimeInterval >= DOIP_VEHICLE_ANNOUNCEMENT_INTERVAL) {
                        sendAnnouncement = TRUE;
                    }
                }
                
                if (TRUE == sendAnnouncement) {
                    sendVehicleAnnouncement(conIndex);
                    DoIP_UdpConAdmin[conIndex].vehicleAnnounceRepetition += 1u;
                    DoIP_UdpConAdmin[conIndex].vehicleAnnounceTimeInterval = 0u;
                }
                
            } else {

                DoIP_UdpConAdmin[conIndex].vehicleAnnounceTgr = FALSE;
                DoIP_UdpConAdmin[conIndex].vehicleAnnounceInProgress = FALSE;
                DoIP_UdpConAdmin[conIndex].vehicleAnnounceInitialTime = 0u;
                DoIP_UdpConAdmin[conIndex].vehicleAnnounceTimeInterval = 0u;
                DoIP_UdpConAdmin[conIndex].vehicleAnnounceRepetition = 0u;                

            }
        } else {
            if ((DoIP_UdpConAdmin[conIndex].rxBufferStartIndex != DoIP_UdpConAdmin[conIndex].rxBufferEndIndex) || 
                    ((DoIP_UdpConAdmin[conIndex].rxBufferStartIndex == DoIP_UdpConAdmin[conIndex].rxBufferEndIndex) && (TRUE == DoIP_UdpConAdmin[conIndex].rxBufferPresent))) {
                        
                handleUdpRx(conIndex, &(DoIP_UdpConAdmin[conIndex].rxBuffer[DoIP_UdpConAdmin[conIndex].rxBufferEndIndex][0]));
                
                if (DoIP_UdpConAdmin[conIndex].rxBufferEndIndex < (DOIP_MAX_UDP_REQUEST_MESSAGE - 1u)) {
                    DoIP_UdpConAdmin[conIndex].rxBufferEndIndex ++;
                } else {
                    DoIP_UdpConAdmin[conIndex].rxBufferEndIndex = UDP_RX_BUFFER_RESET_INDEX;
                }
                
                if ((UDP_RX_BUFFER_RESET_INDEX == DoIP_UdpConAdmin[conIndex].rxBufferEndIndex) && (UDP_RX_BUFFER_RESET_INDEX == DoIP_UdpConAdmin[conIndex].rxBufferEndIndex)) {
                    freeUdpRxBuffer(conIndex);
                }
            }
        }
    }
    
    for (conIndex = 0u; conIndex < DOIP_TCP_CON_NUM; conIndex++) {
        /* If buffer is Idle and the queue is not empty, then initiate transmission */
        if ((TRUE == DoIP_TcpQueueAdmin[conIndex].diagAckQueueActive) && (BUFFER_IDLE == DoIP_TcpConAdmin[conIndex].txBufferState)) {
            createAndSendDiagnosticAck(conIndex, DoIP_TcpQueueAdmin[conIndex].sa, DoIP_TcpQueueAdmin[conIndex].ta);
            DoIP_TcpQueueAdmin[conIndex].diagAckQueueActive = FALSE;
        } else if ((TRUE == DoIP_TcpQueueAdmin[conIndex].tpTransmitQueueActive) && (BUFFER_IDLE == DoIP_TcpConAdmin[conIndex].txBufferState)) {
            tempPduInfo.SduDataPtr = DoIP_TcpQueueAdmin[conIndex].SduDataPtr;
            tempPduInfo.SduLength = DoIP_TcpQueueAdmin[conIndex].SduLength;
            (void)DoIP_TpTransmit(DoIP_TcpQueueAdmin[conIndex].txPduId, &tempPduInfo);
            DoIP_TcpQueueAdmin[conIndex].tpTransmitQueueActive = FALSE;
        } else {
            /* Do nothing */
        }
    }

    /* Handle DoIP connection timeouts */
    for (conIndex = 0u; conIndex < DOIP_TCP_CON_NUM; conIndex++) {
        /** @req SWS_DoIP_00143 */
        if (CONNECTION_INITIALIZED == DoIP_TcpConAdmin[conIndex].socketState) {
            /* Handle initial inactivity timeouts */
            if (DoIP_TcpConAdmin[conIndex].initialInactivityTimer <= DOIP_GENERAL_INACTIVE_TIME) {
                /* Note: Timer functionality is disabled */
                /*DoIP_TcpConAdmin[conIndex].initialInactivityTimer += DOIP_MAIN_FUNCTION_PERIOD;*/
            } else {
                handleTimeoutFlag = TRUE;
            }
        } else if (CONNECTION_REGISTERED == DoIP_TcpConAdmin[conIndex].socketState) {
            /* Handle Alive check timeouts */
            if (TRUE == DoIP_TcpConAdmin[conIndex].awaitingAliveCheckResponse) {
                
                if (DoIP_TcpConAdmin[conIndex].aliveCheckTimer < DOIP_ALIVE_CHECK_RESPONSE_TIMEOUT) {
                    /* Note: Timer functionality is disabled */
                    /*DoIP_TcpConAdmin[conIndex].aliveCheckTimer += DOIP_MAIN_FUNCTION_PERIOD;*/
                } else {
                    handleTimeoutFlag = TRUE;
                }
            }

            /* Handle general inactivity timeouts */
            if (DoIP_TcpConAdmin[conIndex].generalInactivityTimer <= DOIP_GENERAL_INACTIVE_TIME) {
                /* Note: Timer functionality is disabled */
                /*DoIP_TcpConAdmin[conIndex].generalInactivityTimer += DOIP_MAIN_FUNCTION_PERIOD;*/
            } else {
                handleTimeoutFlag = TRUE;
            }
        } else {
              
        }
        
        if (TRUE == handleTimeoutFlag) {
            handleTimeout(conIndex);
        }
    }
}
