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


#ifndef COMM_CONFIGTYPES_H_
#define COMM_CONFIGTYPES_H_
#include "Com_Types.h"

typedef enum {
    COMM_BUS_TYPE_CAN,
    COMM_BUS_TYPE_FR,
    COMM_BUS_TYPE_INTERNAL,
    COMM_BUS_TYPE_LIN,
    COMM_BUS_TYPE_ETH
} ComM_BusTypeType;

typedef enum {
    COMM_NM_VARIANT_NONE,
    COMM_NM_VARIANT_LIGHT,
    COMM_NM_VARIANT_PASSIVE,
    COMM_NM_VARIANT_FULL
} ComM_NmVariantType;

typedef enum {
    COMM_GATEWAY_TYPE_NONE,
    COMM_GATEWAY_TYPE_ACTIVE,
    COMM_GATEWAY_TYPE_PASSIVE
} ComM_PncGatewayType;

typedef struct {
    const ComM_BusTypeType			BusType;
    const NetworkHandleType			ComMChannelId;
    const NetworkHandleType			NmChannelHandle;
    const ComM_NmVariantType		NmVariant;
    const uint32					MainFunctionPeriod;
    const uint32					LightTimeout;
    const ComM_PncGatewayType		PncGatewayType;
} ComM_ChannelType;

typedef Std_ReturnType (*ComM_RteSwitchUM)(/*IN*/uint8 mode);
typedef struct {
    const ComM_ChannelType* const*  ChannelList;  /* Channels referred by User */
    const ComM_ChannelType* const*  PncChnlList;  /* Channels referred by User via Pnc configuration */
    ComM_RteSwitchUM                ComMRteSwitchUM; /* Report mode change to users*/
    const uint8						ChannelCount; /* No. of Channels mapped to user */
    const uint8                     PncChnlCount; /* No. of channels referenced by user via Pnc configuration */
} ComM_UserType;


/* configuration of PncComSignal */
typedef struct{
    Com_SignalIdType					ComMPncSignalId;      /* Signal handle */
}ComM_PncComSignalType;

/* configuration of Pnc */
typedef struct{
    const NetworkHandleType* ComMPncChnlRef;    /* reference to Channels */
    const uint8*            ComMPncUserRef;     /* reference to users */
    PNCHandleType           ComMPncId;          /* Identifier for PNC */
    uint8                   ComMPncChnlRefNum;  /* No of reference channels*/
    uint8                   ComMPncUserRefNum;  /* No of reference users */
}ComM_PncType;

/* configuration collection for partial network*/
typedef struct{
    const ComM_PncType*     ComMPnc;            /* reference to PNC config */
    const uint8*            ComMPncIdToPncCountVal; /* mapping from Pnc Id to PNC count(index) of the PNC */
    const ComM_PncComSignalType*  ComMPncEIRARxComSigIdRef;  /* Signal Id Ref for Rx EIRA */
    const ComM_PncComSignalType*  ComMPncTxComSigIdRef; /* Signal Id reference for Tx signals */
    const ComM_PncComSignalType*  ComMPncERARxComSigIdRefs; /* Signal Id Refs for Rx ERA */
    uint8                   ComMPncNum;         /* number of PNC config */
    boolean                 ComMPncEnabled;     /* switch to enable PN processing */
}ComM_PNConfigType;

typedef struct {
    const ComM_ChannelType*			Channels;
    const ComM_UserType*			Users;
    const ComM_PNConfigType*        ComMPncConfig;
} ComM_ConfigType;

#endif /* COMM_CONFIGTYPES_H_ */
