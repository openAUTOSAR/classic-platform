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
#ifndef DOIP_H_
#define DOIP_H_

#include "DoIP_Types.h"
#include "SoAd_Types.h"

#define DOIP_AR_RELEASE_MAJOR_VERSION               4u
#define DOIP_AR_RELEASE_MINOR_VERSION               2u
#define DOIP_AR_RELEASE_REVISION_VERSION            2u
                
#define DOIP_VENDOR_ID		                        60u
#define DOIP_MODULE_ID                              (173u)
#define DOIP_AR_MAJOR_VERSION                       DOIP_AR_RELEASE_MAJOR_VERSION
#define DOIP_AR_MINOR_VERSION                       DOIP_AR_RELEASE_MINOR_VERSION
#define DOIP_AR_PATCH_VERSION                       DOIP_AR_RELEASE_REVISION_VERSION
                
#define DOIP_SW_MAJOR_VERSION                       1u
#define DOIP_SW_MINOR_VERSION                       0u
#define DOIP_SW_PATCH_VERSION                       0u

/** @name Service id's */
#define DOIP_TP_TRANSMIT_SERVICE_ID                 0x03u   /* In Autosar 4.3.0 spec, it is menioned as 49 which is repeated */
#define DOIP_TP_CANCEL_TRANSMIT_SERVICE_ID          0x04u   /* In Autosar 4.3.0 spec, it is menioned as 4A which is repeated */
#define DOIP_TP_CANCEL_RECEIVE_SERVICE_ID           0x05u
#define DOIP_IF_TRANSMIT_SERVICE_ID                 0x49u
#define DOIP_IF_CANCEL_TRANSMIT_SERVICE_ID          0x4Au
#define DOIP_INIT_SERVICE_ID                        0x01u
#define DOIP_GET_VERSION_INFO_SERVICE_ID            0x00u
#define DOIP_SOAD_TP_COPY_TX_DATA_SERVICE_ID        0x43u
#define DOIP_SOAD_TP_TX_CONFIRMATION_SERVICE_ID     0x48u
#define DOIP_SOAD_TP_COPY_RX_DATA_SERVICE_ID        0x44u
#define DOIP_SOAD_TP_START_OF_RECEPTION_SERVICE_ID  0x46u
#define DOIP_SOAD_TP_RX_INDICATION_SERVICE_ID       0x45u
#define DOIP_SOAD_IF_RX_INDICATION_SERVICE_ID       0x42u
#define DOIP_SOAD_IF_TX_CONFIRMATION_SERVICE_ID     0x40u
#define DOIP_SO_CON_MODE_CHG_SERVICE_ID             0x0Bu
#define DOIP_LOCAL_IP_ADDR_ASSIGN_CHG_SERVICE_ID    0x0Cu
#define DOIP_ACTIVATION_LINE_SW_ACTIVE_SERVICE_ID   0x0Fu
#define DOIP_ACTIVATION_LINE_SW_INACTIVE_SERVICE_ID 0x0Eu
#define DOIP_MAIN_FUNCTION_SERVICE_ID               0x02u

/* Local function IDs */
#define DOIP_HANDLE_VEHICLE_ID_REQ_ID               0x50u
#define DOIP_SEND_VEHICLE_ANNOUNCE_REQ_ID           0x51u
#define DOIP_HANDLE_ENTITY_STATUS_REQ_ID            0x52u
#define DOIP_CREATE_AND_SEND_ALIVE_CHECK_ID         0x53u  
#define DOIP_ROUTING_ACTIVATION_REQ_ID              0x54u 
#define DOIP_HANDLE_DIAG_MSG_ID                     0x55u   
#define DOIP_CREATE_AND_SEND_NACK_ID                0x56u
#define DOIP_CREATE_AND_SEND_D_ACK_ID               0x57u
#define DOIP_CREATE_AND_SEND_D_NACK_ID              0x58u
#define DOIP_HANDLE_POWER_MODE_REQ_ID               0x59u  
#define DOIP_HANDLE_ALIVECHECK_TIMEOUT_ID           0x5Au 
#define DOIP_HANDLE_ALIVECHECK_RESP_ID              0x5Bu
#define DOIP_HANDLE_TIMEOUT_ID                      0x5Cu
#define DOIP_GET_EID_ID                             0x5Du

/* Development errors as in DoIP 7.6 Error classification */
/** @req SWS_DoIP_00148 */
#define DOIP_E_UNINIT                               0x01u
#define DOIP_E_PARAM_POINTER                        0x02u
#define DOIP_E_INVALID_PDU_SDU_ID                   0x03u
#define DOIP_E_INVALID_PARAMETER                    0x04u
#define DOIP_E_INIT_FAILED                          0x05u

/* Development errors - Runtime */
/** @req SWS_DoIP_00282 */
#define DOIP_E_UNEXPECTED_EXECUTION                 0x06u
#define DOIP_E_BUFFER_BUSY                          0x07u
#define DOIP_E_INVALID_CH_INDEX                     0x08u
#define DOIP_E_IF_TRANSMIT_ERROR                    0x09u
#define DOIP_E_TP_TRANSMIT_ERROR                    0x0Au
#define DOIP_E_INVALID_SOCKET_STATE                 0x0Bu

/** @req SWS_DoIP_00025*/
typedef struct{
    const DoIP_ChannelType *DoIP_Channel;
    const DoIP_TcpType *DoIP_TcpMsg;
    const DoIP_UdpType *DoIP_UdpMsg;
    const DoIP_TargetAddrType *const DoIP_TargetAddress;
    const DoIP_RoutActType *const DoIP_RoutingActivation;
    const DoIP_TesterType *const DoIP_Tester;
}DoIP_ConfigType;

extern const DoIP_ConfigType DoIP_Config;

/* @req SWS_DoIP_00022 */
Std_ReturnType DoIP_TpTransmit(PduIdType DoIPPduRTxId, const PduInfoType* DoIPPduRTxInfoPtr);
/* @req SWS_DoIP_00023 */
Std_ReturnType DoIP_TpCancelTransmit(PduIdType DoIPPduRTxId);
/* @req SWS_DoIP_00277 */
Std_ReturnType DoIP_IfTransmit(PduIdType id, const PduInfoType* info);
/* @req SWS_DoIP_00278 */
Std_ReturnType DoIP_IfCancelTransmit(PduIdType id);
/* @req SWS_DoIP_00026 */
void DoIP_Init(const DoIP_ConfigType* DoIPConfigPtr);
/* @req SWS_DoIP_00027 */
void DoIP_GetVersionInfo(Std_VersionInfoType* versioninfo);
/* @req SWS_DoIP_00031 */
BufReq_ReturnType DoIP_SoAdTpCopyTxData( PduIdType id, const PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr );
/* @req SWS_DoIP_00032 */
void DoIP_SoAdTpTxConfirmation(PduIdType id, Std_ReturnType result);
/* @req SWS_DoIP_00033 */
BufReq_ReturnType DoIP_SoAdTpCopyRxData(PduIdType id,const PduInfoType* info,PduLengthType* bufferSizePtr);
/* @req SWS_DoIP_00037 */
BufReq_ReturnType DoIP_SoAdTpStartOfReception(PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr);
/* @req SWS_DoIP_00038 */
void DoIP_SoAdTpRxIndication(PduIdType id, Std_ReturnType result);
/* @req SWS_DoIP_00244 */
void DoIP_SoAdIfRxIndication(PduIdType RxPduId,const PduInfoType* PduInfoPtr);
/* @req SWS_DoIP_00245 */
void DoIP_SoAdIfTxConfirmation(PduIdType TxPduId);
/* @req SWS_DoIP_0039 */
void DoIP_SoConModeChg(SoAd_SoConIdType SoConId, SoAd_SoConModeType Mode);
/* @req SWS_DoIP_00040 */
void DoIP_LocalIpAddrAssignmentChg(SoAd_SoConIdType SoConId, TcpIp_IpAddrStateType State);
/* @req SWS_DoIP_00251 */
void DoIP_ActivationLineSwitchActive(void);
/* @req SWS_DoIP_91001 */
void DoIP_ActivationLineSwitchInactive(void);
/* @req SWS_DoIP_00041 */
void DoIP_MainFunction (void);

/* DoIP call-outs */
Std_ReturnType DoIP_Arc_GetVin(uint8* Data, uint8 noOfBytes);
void DoIP_Arc_TcpConnectionNotification(SoAd_SoConIdType id);

#endif /* DOIP_H_ */
