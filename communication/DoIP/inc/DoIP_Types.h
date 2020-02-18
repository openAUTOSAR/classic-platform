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
typedef Std_ReturnType (*DoIPRoutingActivationConfirmationType)( boolean* Confirmed, uint8* ConfirmationReqData, uint8* ConfirmationResData );
typedef Std_ReturnType (*DoIPRoutingActivationAuthentication)( boolean* Authentified, uint8* AuthenticationReqData, uint8* AuthenticationResData);
typedef struct {
    uint32 DoIP_NumByteDiagAckNack;
    uint16 DoIP_TesterSA;
    uint8 * DoIP_RoutingActivationRef;
}DoIP_TesterType;

typedef struct {
    uint8 DoIP_RoutingActivationNum;
    const DoIP_RoutingActivationAuthType * DoIP_RoutingActivationAuthRef;
    const DoIP_RoutingActivationCnfrmType *DoIP_RoutingActivationCnfrmRef;
}DoIP_RoutingActivationType;

typedef struct{
    DoIPRoutingActivationConfirmationType DoIP_RouteActivationAuthFunc;
    uint8 DoIP_Route_Activation_Auth_ReqLen;
    uint8 DoIP_Route_Activation_Auth_ResLen;
}DoIP_RoutingActivationAuthType;

typedef struct{
    DoIPRoutingActivationAuthentication DoIP_Route_Activation_Cnfrm_Func;
    uint8 DoIP_Route_Activation_Cnfrm_ReqLen;
    uint8 DoIP_Route_Activation_Cnfrm_ResLen;
}DoIP_RoutingActivationCnfrmType;

typedef struct{
  uint8* DoIP_chanlSARef;
  uint8* DoIP_chnlTARef;
  const DoIP_PDUReceiveType *DoIP_PDUReceiveRef;
  const DoIP_PDUTransmitType *DoIP_PDUTransmitRef;
}DoIPChannelsType;

typedef struct{
  uint16 DoIP_RXPDUID;
  uint8* DoIP_RXPDURef;

}DoIP_PDUReceiveType;

typedef struct{
  uint16 DoIP_TXPDUID;
  uint8* DoIP_TXPDURef;
  enum EcucEnumerationParamDef {DOIP_TPPDU,DOIP_IFPDUs};

}DoIP_PDUTransmitType;

typedef struct{
    uint16  DoIPTargetAddressValues;
}DoIP_TargetAddressType;

typedef struct{
 const DoIP_TCPSoADRxPDUType* DoIP_TCPSoADRxPDURef;
 const DoIP_TCPSoADTxPDUType* DoIP_TCPSoADTxPDURef;
}DoIPTCPType;

typedef struct{
 uint16 DoIP_SoADRx;
 uint8 *DoIP_SoADRxRef;
}DoIP_TCPSoADRxPDUType;

typedef struct{
 uint16 DoIP_SoADTx;
 uint8 *DoIP_SoADTxRef;
}DoIP_TCPSoADTxPDUType;

typedef struct{
 const DoIP_UDPSoADRxPDUType* DoIP_UDPSoADRxPDURef;
 const DoIP_UDPSoADTxPDUType* DoIP_UDPSoADTxPDURef;
}DoIPUDPType;

typedef struct{
  uint16 DoIP_SoADRx;
  uint8 *DoIP_SoADRxRef;
}DoIP_UDPSoADRxPDUType;

typedef struct{
 uint16 DoIP_SoADTx;
 uint8 *DoIP_SoADTxRef;
}DoIP_UDPSoADTxPDUType;

typedef struct{
    const DoIPChannelsType *const DoIPChanl;
    const DoIPTCPType *const DoIPTCPmsg;
    const DoIPUDPType *const DoIPUDPmsg;
    const DoIP_TargetAddressType *const DoIP_TargetAdrss;
    const DoIP_RoutingActivationType *const DoIP_RouteActivateType;
    const DoIP_TesterType *const DoIP_TestType;
}DoIP_configType;
#endif /* DOIP_TYPES_H_ */

