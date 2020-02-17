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
#ifndef ETHTSYN_H_
#define ETHTSYN_H_

/* @req 4.2.2/SWS_EthTSyn_00031 */
#include "Eth_GeneralTypes.h"
#include "EthTSyn_Types.h"
#include "StbM.h"

#define ETHTSYN_VENDOR_ID                  60u
#define ETHTSYN_MODULE_ID                 164u

#define ETHTSYN_AR_RELEASE_MAJOR_VERSION    4u
#define ETHTSYN_AR_RELEASE_MINOR_VERSION    2u
#define ETHTSYN_AR_RELEASE_PATCH_VERSION    2u

#define ETHTSYN_SW_MAJOR_VERSION            1u
#define ETHTSYN_SW_MINOR_VERSION            1u
#define ETHTSYN_SW_PATCH_VERSION            0u

#include "EthIf.h"
/* Development errors */
/* @req 4.2.2/SWS_EthTSyn_00030 */
#define ETHTSYN_E_NOT_INITIALIZED         0x20u
/* ArcCore extra Error ids */
/* @req 4.2.2/SWS_EthTSyn_00029 */
#define ETHTSYN_E_INIT_FAILED             0x30u
#define ETHTSYN_E_PARAM_POINTER           0x31u
#define ETHTSYN_E_INVALID_TIMEBASE_ID     0X32u
#define ETHTSYN_E_INVALID_CTRL_ID         0X33u
#define ETHTSYN_E_INV_MODE                0x34u

/* API ids */
#define ETHTSYN_SERVICE_ID_INIT           0x01u
#define ETHTSYN_GETVERSIONINFO_ID         0x02u
#define ETHTSYN_GETCURRENTTIME_ID         0x03u
#define ETHTSYN_SETGLOBALTIME_ID          0x04u
#define ETHTSYN_SETTRANSMISSIONMODE_ID    0x05u

/* Call back Api Id */
#define ETHTSYN_RXINDICATION_ID           0x06u
#define ETHTSYN_TXCONFIRMATION_ID         0x07u
#define ETHTSYN_TRCVLINKSTATECHG_ID       0x08u

/* Schedule function API id */
#define ETHTSYN_MAINFUNCTION_ID           0x09u

#include "EthTSyn_Cfg.h"

/* API's prototype */
/* @req 4.2.2/SWS_EthTSyn_00035 */
void EthTSyn_Init( const EthTSyn_ConfigType* configPtr );

#if (ETHTSYN_GET_VERSION_INFO == STD_ON)
/* @req 4.2.2/SWS_EthTSyn_00036 */
void EthTSyn_GetVersionInfo( Std_VersionInfoType* versioninfo );
#endif


#if (ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON)
/* @req 4.2.2/SWS_EthTSyn_00037 */
Std_ReturnType EthTSyn_GetCurrentTime( StbM_SynchronizedTimeBaseType timeBaseId, StbM_TimeStampType* timeStampPtr, EthTSyn_SyncStateType* syncState );

/* @req 4.2.2/SWS_EthTSyn_00038 */
Std_ReturnType EthTSyn_SetGlobalTime( StbM_SynchronizedTimeBaseType timeBaseId, const StbM_TimeStampType* timeStampPtr );
#endif


/* @req 4.2.2/SWS_EthTSyn_00039 */
void EthTSyn_SetTransmissionMode( uint8 CtrlIdx, EthTSyn_TransmissionModeType Mode );

/* @req 4.2.2/SWS_EthTSyn_00040 */
void EthTSyn_RxIndication( uint8 CtrlIdx, Eth_FrameType FrameType, boolean IsBroadcast, const uint8* PhysAddrPtr, uint8* DataPtr, uint16 LenByte );

/* @req 4.2.2/SWS_EthTSyn_00042 */
void EthTSyn_TxConfirmation( uint8 CtrlIdx, Eth_BufIdxType BufIdx );

/* @req 4.2.2/SWS_EthTSyn_00043 */
Std_ReturnType EthTSyn_TrcvLinkStateChg( uint8 CtrlIdx, EthTrcv_LinkStateType TrcvLinkState );

/* @req 4.2.2/SWS_EthTSyn_00044 */
void EthTSyn_MainFunction( void );

extern const EthTSyn_ConfigType EthTSynConfigData;
#endif  /* ETHTSYN_H_ */
