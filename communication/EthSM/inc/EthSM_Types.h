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

#ifndef ETHSM_TYPES_H_
#define ETHSM_TYPES_H_

/* @req 4.2.2/SWS_EthSM_00006 *//*This header file EthSM_Types.h exports the EthSM specific types */

#if defined(USE_DEM)
#include "Dem.h"
#endif

/* Configuration Types are also included here */

typedef struct {
    const uint8   EthIfControllerId;      /* EthIf Controller Id */
    const uint8   ComMNetworkHandle;      /* ComM channel Id*/
    const boolean EthTrcvAvailable;       /* TRCV suppoerted */
#if defined(USE_DEM)
    Dem_EventIdType ETHSM_E_LINK_DOWN;   /* Reference to configured DEM event to report bus off errors for this Eth network.*/
#endif
}EthSM_NetworkConfigType;

typedef struct {
    const EthSM_NetworkConfigType *Networks;  /* Pointer to configured networks list */
    uint8 NofNetworks;                        /* No of configured networks */
}EthSM_ConfigSetType;

typedef struct {
    const EthSM_ConfigSetType *ConfigSet;    /* pointer EthSM config data */
}EthSM_ConfigType;

typedef enum {
    ETHSM_STATUS_UNINIT,   /* EthSM is in this state before EthSM_init.*/
    ETHSM_STATUS_INIT      /* EthSM is initialized in this state after EthSM_init. */
} EthSM_Internal_InitStatusType;

typedef struct {
    ComM_ModeType RequestedMode;              /* holds network Request mode */
    ComM_ModeType CurrentMode;                /* holds current mode of network */
    TcpIp_StateType TcpIpState;               /* holds TCP/IP state of network  */
    EthTrcv_LinkStateType TrcvLinkState;      /* holds Trcv Link state of network  */
    EthSM_NetworkModeStateType NetworkMode;   /* states of the network mode of state machine */
    Eth_ModeType CtrlMode;                    /* holds the Eth controller mode */
    EthTrcv_ModeType TrcvMode;                /* holds the EthTrcv  mode */
    uint8 RequestStream;                      /* holds request for network is done or not status*/
} EthSM_Internal_NetworkType;

typedef struct {
    EthSM_Internal_InitStatusType   InitStatus; /* Status of EthSM Module */
    EthSM_Internal_NetworkType*     Networks;   /* pointer to hold internal states of networks */
    const EthSM_ConfigType*         SMConfig;   /* pointer to hold configured data */
} EthSM_InternalType;


#endif /* ETHSM_TYPES_H_ */
