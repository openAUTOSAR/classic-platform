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

#ifndef ETHIF_TYPES_H_
#define ETHIF_TYPES_H_

#include "Eth_GeneralTypes.h" /* @req 4.2.2/SWS_EthIf_00153 */

/* @req 4.2.2/SWS_EthIf_00150 */
typedef enum {
    ETHIF_STATE_UNINIT,   /* Status of EthIf module before EthIf_Init function*/
    ETHIF_STATE_INIT,     /* Status of EthIf module after EthIf_Init function called*/
}EthIf_StateType;


typedef struct {
    uint8     EthIfCtrlId;         /* Index of the Ethernet controller within the context of the Ethernet Interface */
    uint8     EthIfMaxTxBufsTotal; /* total number of transmit buffers. */
    uint16    EthIfCtrlMtu;        /* maximum Payload size */
    uint8     EthIfEthCtrlId;      /* Index of Eth controller within the context of the Ethernet Driver */
    uint8     EthIfEthTrcvId;      /* Index of Eth Trcv */
    uint16    EthIfVlanId;         /* 12 bit VLAN ID */
}EthIf_Controller_type;

/* Function pointer for User defined Tx Confirmation callback */
typedef void (*EthIfULTxConfirmationType) ( uint8 CtrlIdx, Eth_BufIdxType BufIdx );

/* Function pointer for User defined Rx Indication callback */
typedef void (*EthIfULRxIndicationType) ( uint8 CtrlIdx, Eth_FrameType FrameType, boolean IsBroadcast, const uint8* PhysAddrPtr, uint8* DataPtr, uint16 LenByte );

/* Function pointer for User defined TrcvLinkState callback */
typedef void (*EthIfTrcvLinkStateChgType) ( uint8 CtrlIdx, EthTrcv_LinkStateType TrcvLinkState );

typedef struct {
    uint32 EthIfFrameType;   /* Frame Type ex: ARP, iV4 */
    uint8 EthIfRxIndicationHandle; /* RxIndication handle */
    uint8 EthIfTxConfirmationHandle; /* TxConfirmation handle */
}EthIf_Frame_Owner_type;

typedef struct {
    uint8 EthIfEthSwitchId; /* Index of Switches in context of Eth Interface*/
    uint8 EthIfSwitchIdx;   /* Index of Switches in context of Eth Switch module*/
}EthIf_Switch_Type;

/* @req 4.2.2/SWS_EthIf_00149 */
typedef struct {
    const EthIf_Controller_type*  EthIfCtrlCfg;   /*pointer to hold controller config data*/
    const EthIf_Frame_Owner_type* EthIfOwnerCfg;  /* pointer to hold Owner config data */
    const EthIf_Switch_Type*      EthIfSwitchCfg; /* pointer to hold switches config data */
    const EthIfULTxConfirmationType*    EthIfULTxConfirmation; /* pointer to hold Tx confirmation functions list */
    const EthIfULRxIndicationType *     EthIfULRxIndication; /* Ptr to Rx indication function list */
    /*const EthIfTrcvLinkStateChgType*  EthIfTrcvLink;       pointer to hold Trcv Link state functions list */
    uint8                         EthIfCtrlCount;     /* No of Controllers configure */
    uint8                         EthIfTrcvCount;     /* No of Trcv configured */
    uint8                         EthIfSwitchCount;   /* No of Switches configured */
    uint8                         EthIfOwnersCount;   /* No fo Owners configured */
}EthIf_ConfigType;

#endif /* ETHIF_TYPES_H_ */
