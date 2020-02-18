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

#ifndef DOIP_TYPES_H_
#define DOIP_TYPES_H_

#include "Std_Types.h"
#include "ComStack_Types.h"

#define DOIP_E_PENDING          0x10u  /** @req SWS_DoIP_00272*/ /** @req SWS_DoIP_00273*/  //done

typedef enum {
    DOIP_TPPDU,
    DOIP_IFPDU
} DoIP_PduType;

typedef enum {
    DOIP_POWER_STATE_NOT_READY,
    DOIP_POWER_STATE_READY,
} DoIP_PowerStateType;

typedef Std_ReturnType (*DoIP_RoutActConfFuncType)(boolean* confirmed, uint8* confReqData, uint8* confResData);
typedef Std_ReturnType (*DoIP_RoutActAuthFuncType)(boolean* authenticated, uint8* authReqData, uint8* authResData);
typedef uint32 DoIPPayloadType;
typedef struct{
    DoIP_RoutActConfFuncType DoIP_RoutActAuthFunc;
    uint8 DoIP_RoutActAuthReqLen;
    uint8 DoIP_RoutActAuthResLen;
}DoIP_RoutActAuthType;

typedef struct{
    DoIP_RoutActAuthFuncType DoIP_RoutActConfFunc;
    uint8 DoIP_RoutActConfReqLen;
    uint8 DoIP_RoutActConfResLen;
}DoIP_RoutActConfType;

typedef struct {
    const uint16 *DoIP_TargetRef;
    const DoIP_RoutActAuthType *DoIP_RoutActAuthRef;
    const DoIP_RoutActConfType *DoIP_RoutActConfRef;
    uint8 DoIP_chnlTARef_Size;
    uint8 DoIP_RoutActNum;
}DoIP_RoutActType;

typedef struct {
    const DoIP_RoutActType **DoIP_RoutActRef;
    uint32 DoIP_NumByteDiagAckNack;
    uint16 DoIP_TesterSA;
    uint16 DoIp_RoutRefSize;
}DoIP_TesterType;

typedef struct{
    uint16 DoIP_TargetAddrValue;
}DoIP_TargetAddrType;

typedef struct{
  PduIdType DoIP_TxPduRef;
  DoIP_PduType DoIP_TxPduType;
}DoIP_PduTransmitType;

typedef struct{
    PduIdType DoIP_RxPduRef;
}DoIP_PduReceiveType;

typedef struct{
  const DoIP_TesterType *DoIP_ChannelSARef;
  const DoIP_TargetAddrType *DoIP_ChannelTARef;
  const DoIP_PduReceiveType *DoIP_PduReceiveRef;
  const DoIP_PduTransmitType *DoIP_PduTransmitRef;
  PduIdType DoIP_UpperLayerRxPduId;
  PduIdType DoIP_RxPduId;
  PduIdType DoIP_UpperLayerTxPduId;
  PduIdType DoIP_TxPduId;
}DoIP_ChannelType;

typedef struct{
 PduIdType DoIP_TcpSoADRxPduRef;
 PduIdType DoIP_TcpRxPduRef;
 PduIdType DoIP_TcpSoADTxPduRef;
 PduIdType DoIP_TcpTxPduRef;
}DoIP_TcpType;

typedef struct{
 PduIdType DoIP_UdpSoADRxPduRef;
 PduIdType DoIP_UdpRxPduRef;
 PduIdType DoIP_UdpSoADTxPduRef;
 PduIdType DoIP_UdpTxPduRef;
}DoIP_UdpType;

#endif /* DOIP_TYPES_H_ */
