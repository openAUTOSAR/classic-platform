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
/* @req 4.2.2/SWS_EthSM_00004 */ /* The header file shall export EthSM module specific types and API´s */

#ifndef ETH_SM_H_
#define ETH_SM_H_

/* ASR does not mention these, but needed to be included */
#include "Eth_GeneralTypes.h"
#include "ComM.h"
#include "TcpIp_Types.h"

#define ETHSM_VENDOR_ID                  60u
#define ETHSM_MODULE_ID                 143u

#define ETHSM_AR_RELEASE_MAJOR_VERSION    4u
#define ETHSM_AR_RELEASE_MINOR_VERSION    2u
#define ETHSM_AR_RELEASE_PATCH_VERSION    2u

#define ETHSM_SW_MAJOR_VERSION            2u
#define ETHSM_SW_MINOR_VERSION            0u
#define ETHSM_SW_PATCH_VERSION            0u

/* Development errors */
#define ETHSM_E_INVALID_NETWORK_MODE      0x01u
#define ETHSM_E_UNINIT                    0x02u
#define ETHSM_E_PARAM_POINTER             0x03u
#define ETHSM_E_INVALID_NETWORK_HANDLE    0x04u
#define ETHSM_E_INVALID_TCP_IP_MODE       0x05u
#define ETHSM_E_INVALID_TRCV_LINK_STATE   0x06u
#define ETHSM_E_PARAM_CONTROLLER          0x07u

/* ArcCore extra Error ids */
#define ETHSM_E_GET_ETH_MODE_RET_NOK           0x20u
#define ETHSM_E_GET_ETHTRCV_MODE_RET_NOK       0x21u
#define ETHSM_E_REQ_TCPIP_MODE_CHG_RET_NOK     0x22u
#define ETHSM_E_REQ_ETH_MODE_CHG_RET_NOK       0x23u
#define ETHSM_E_REQ_ETHTRCV_MODE_CHG_RET_NOK   0x24u

/* API ids */
#define ETHSM_MAINFUNCTION_ID             0x01u
#define ETHSM_GETVERSIONINFO_ID           0x02u
#define ETHSM_GETCURRENTINTERNALMODE_ID   0x03u
#define ETHSM_GETCURRENTCOMMODE_ID        0x04u
#define ETHSM_REQUESTCOMMODE_ID           0x05u
#define ETHSM_TRCVLINKSTATECHG_ID         0x06u
#define ETHSM_INIT_ID                     0x07u
#define ETHSM_TCPIPMODEINDICATION_ID      0x08u
#define ETHSM_CTRLMODEINDICATION_ID       0x09u
#define ETHSM_TRCVMODEINDICATION_ID       0x10u
/* ArcCore extra API ids */
#define ETHSM_GLOBAL_ID                   0x20u

/* @req 4.2.2/SWS_EthSM_00041 */
/* This type shall define the states of the network mode state machine */
typedef enum{
    ETHSM_STATE_OFFLINE,     /* EthSM is initialized in this state. */
    ETHSM_STATE_WAIT_TRCVLINK, /* ComM requests COMM_FULL_COMMUNICATION in this state. Controller and transceiver will be initialized and set to ACTIVE. EthSM waits for transceiver link state (ACTIVE)*/
    ETHSM_STATE_WAIT_ONLINE, /*Transceiver link state is ACTIVE EthSM waits for IP communication (TcpIP state = ONLINE) */
    ETHSM_STATE_ONLINE, /* IP communication is available ComM state COMM_FULL_COMMUNICATION is reached */
    ETHSM_STATE_ONHOLD, /* EthSM lost active transceiver link state, TcpIP state is still ONLINE */
    ETHSM_STATE_WAIT_OFFLINE /* ComM requests COMM_NO_COMMUNICATION in this state. */
}EthSM_NetworkModeStateType;


#include "EthSM_Types.h"
#include "EthSM_Cfg.h"

/**
 * Function routine for ETH SM initialization, called first before calling any other of ETHSM.
 */
void EthSM_Init( void );
/**
 * Fetches version information of ETH SM module
 * @param versioninfo - address to hold the version
 * @return none
 */
void EthSM_GetVersionInfo( Std_VersionInfoType* versioninfo ); /* re-entrant */
/**
 * Function routine to request communication mode (FULL or NO)
 * @param NetworkHandle - Global Network handle
 * @param ComM_Mode - requested node
 * @return E_OK or E_NOT_OK
 */
Std_ReturnType EthSM_RequestComMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode );
/**
 * Function routine to read the current communication mode (FULL or NO)
 * @param NetworkHandle - Global Network handle
 * @param ComM_ModePtr - address to hold the mode
 * @return E_OK or E_NOT_OK
 */
Std_ReturnType EthSM_GetCurrentComMode( NetworkHandleType NetworkHandle, ComM_ModeType* ComM_ModePtr );
/**
 * Function routine to notice ETHSM about tranceiver link state changes
 * @param CtrlIdx - controller id
 * @param TransceiverLinkState - Trcv Link state
 * @return none
 */
void EthSM_TrcvLinkStateChg( uint8 CtrlIdx, EthTrcv_LinkStateType TransceiverLinkState );
/**
 * Function routine to get internal network mode of ETH SM
 * @param NetworkHandle - Global Network handle
 * @param EthSM_InternalMode - address to hold the internal mode
 * @return E_OK or E_NOT_OK
 */
Std_ReturnType EthSM_GetCurrentInternalMode( NetworkHandleType NetworkHandle, EthSM_NetworkModeStateType* EthSM_InternalMode );

/**
 * Main function to be called with a periodic time frame to carry out state FSM and others.
 * @param none
 * @return none
 */
void EthSM_MainFunction( void );

extern const EthSM_ConfigType EthSMConfig;
#endif /* ETH_SM_H_ */
