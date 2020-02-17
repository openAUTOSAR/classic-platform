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

#ifndef ETHIF_H_
#define ETHIF_H_

#include "Eth_GeneralTypes.h" /* @req 4.2.2/SWS_EthIf_00152 */
#include "EcuM_Types.h"
#include "EthSM.h"
#include "EthSM_Cbk.h"

/* General requirements */
/* @req 4.2.2/SWS_EthIf_00011 */ /* header files shall not define global variables */

#define ETHIF_VENDOR_ID                     60u
#define ETHIF_MODULE_ID                     65u

/* @req 4.2.2/SWS_EthIf_00006 */
#define ETHIF_AR_RELEASE_MAJOR_VERSION      4u
#define ETHIF_AR_RELEASE_MINOR_VERSION      2u
#define ETHIF_AR_RELEASE_REVISION_VERSION   2u

#define ETHIF_AR_MAJOR_VERSION              ETHIF_AR_RELEASE_MAJOR_VERSION
#define ETHIF_AR_MINOR_VERSION              ETHIF_AR_RELEASE_MINOR_VERSION
#define ETHIF_AR_PATCH_VERSION              ETHIF_AR_RELEASE_REVISION_VERSION

#define ETHIF_SW_MAJOR_VERSION              1u
#define ETHIF_SW_MINOR_VERSION              1u
#define ETHIF_SW_PATCH_VERSION              0u

#include "EthIf_Types.h"
#include "EthIf_Cfg.h"

/** Error IDs  */
/* @req 4.2.2/SWS_EthIf_00017 */
#define ETHIF_E_INV_CTRL_IDX		 		1u
#define ETHIF_E_INV_TRCV_IDX	      		2u
#define ETHIF_E_NOT_INITIALIZED 		    3u
#define ETHIF_E_PARAM_POINTER		      	4u
#define ETHIF_E_INV_PARAM    				5u
#define ETHIF_E_INIT_FAILED  				6u

/** Service IDs */
#define ETHIF_SERVICE_ID_INIT						0x01u
#define ETHIF_SERVICE_ID_SET_CTRL_MODE   			0x03u
#define ETHIF_SERVICE_ID_GET_CTRL_MODE		        0x04u
#define ETHIF_SERVICE_ID_SET_TRCV_MODE			    0x06u
#define ETHIF_SERVICE_ID_GET_TRCV_MODE				0x07u
#define ETHIF_SERVICE_ID_SET_TRCV_WAKEUP_MODE		0x2Eu
#define ETHIF_SERVICE_ID_GET_TRCV_WAKEUP_MODE		0x2Fu
#define ETHIF_SERVICE_ID_CHECK_WAKE_UP              0x30u
#define ETHIF_SERVICE_ID_GET_PHY_ADDR               0x08u
#define ETHIF_SERVICE_ID_SET_PHY_ADDR               0x0Du
#define ETHIF_SERVICE_ID_UPDATE_PHY_ADDR_FILTER     0x0Cu
#define ETHIF_SERVICE_ID_GET_PORT_MAC_ADDR          0x28u
#define ETHIF_SERVICE_ID_GET_ARL_TABEL              0x29u
#define ETHIF_SERVICE_ID_GET_BUF_LEVEL              0x2Au
#define ETHIF_SERVICE_ID_GET_DROP_COUNT             0x2Bu
#define ETHIF_SERVICE_ID_STORE_CONFIG               0x2Cu
#define ETHIF_SERVICE_ID_RESET_CONFIG               0x2Du
#define ETHIF_SERVICE_ID_GET_CURRENT_TIME           0x22u
#define ETHIF_SERVICE_ID_ENABLE_EGRESS_TIME_STAMP   0x23u
#define ETHIF_SERVICE_ID_GET_EGRESS_TIME_STAMP      0x24u
#define ETHIF_SERVICE_ID_GET_INGRESS_TIME_STAMP     0x25u
#define ETHIF_SERVICE_ID_SET_CORRECTION_TIME        0x26u
#define ETHIF_SERVICE_ID_SET_GLOBAL_TIME            0x27u
#define ETHIF_SERVICE_ID_PROVIDE_TX_BUFFER          0x09u
#define ETHIF_SERVICE_ID_TRANSMIT                   0x0Au
#define ETHIF_SERVICE_ID_GET_VERSION_INFO           0x0Bu

/* Callback notifications function service Ids */
#define ETHIF_SERVICE_ID_RX_INDICATION              0x10u
#define ETHIF_SERVICE_ID_TX_CONFIRMATION            0x11u
#define ETHIF_SERVICE_ID_CTRL_MODE_INDICATION       0x0Eu
#define ETHIF_SERVICE_ID_TRCV_MODE_INDICATION       0x0Fu

/* Scheduled functions service IDs */
#define ETHIF_SERVICE_ID_MAIN_FUNCTION_RX           0x20u
#define ETHIF_SERVICE_ID_MAIN_FUNCTION_TX           0x21u

/**
 * Function routine for ETH IF initialization, called first before calling any other of ETHIF functions.
 */
/* @req 4.2.2/SWS_EthIf_00024 */
void EthIf_Init( const EthIf_ConfigType* CfgPtr );/** Init function for ETHIF */
/* @req 4.2.2/SWS_EthIf_00034 */
Std_ReturnType EthIf_SetControllerMode( uint8 CtrlIdx, Eth_ModeType CtrlMode );
/* @req 4.2.2/SWS_EthIf_00039 */
Std_ReturnType EthIf_GetControllerMode( uint8 CtrlIdx, Eth_ModeType* CtrlModePtr );
/* @req 4.2.2/SWS_EthIf_00050 */
Std_ReturnType EthIf_SetTransceiverMode( uint8 TrcvIdx, EthTrcv_ModeType TrcvMode );
/* @req 4.2.2/SWS_EthIf_00055 */
Std_ReturnType EthIf_GetTransceiverMode( uint8 TrcvIdx, EthTrcv_ModeType* TrcvModePtr );
/* @req 4.2.2/SWS_EthIf_00233 */
Std_ReturnType EthIf_SetTransceiverWakeupMode( uint8 TrcvIdx, EthTrcv_WakeupModeType TrcvWakeupMode );
/* @req 4.2.2/SWS_EthIf_00238 */
Std_ReturnType EthIf_GetTransceiverWakeupMode( uint8 TrcvIdx, EthTrcv_WakeupModeType* TrcvWakeupModePtr );
/* @req 4.2.2/SWS_EthIf_00244 */
Std_ReturnType EthIf_CheckWakeup( EcuM_WakeupSourceType WakeupSource );
/* @req 4.2.2/SWS_EthIf_00061 */
void EthIf_GetPhysAddr( uint8 CtrlIdx, uint8* PhysAddrPtr );
/* @req 4.2.2/SWS_EthIf_00132 */
void EthIf_SetPhysAddr( uint8 CtrlIdx, const uint8* PhysAddrPtr );
/* @req 4.2.2/SWS_EthIf_00139 */
Std_ReturnType EthIf_UpdatePhysAddrFilter( uint8 CtrlIdx, const uint8* PhysAddrPtr, Eth_FilterActionType Action );
/* @req 4.2.2/SWS_EthIf_00190 */
Std_ReturnType EthIf_GetPortMacAddr( const uint8* MacAddrPtr, uint8* SwitchIdxPtr, uint8* PortIdxPtr );
/* @req 4.2.2/SWS_EthIf_00196 */
Std_ReturnType EthIf_GetArlTable( uint8 SwitchIdx, EthSwt_MacVlanType* ArlTable );
/* @req 4.2.2/SWS_EthIf_00202 */
Std_ReturnType EthIf_GetBufferLevel( uint8 SwitchIdx, uint32* SwitchBufferLevelPtr );
/* @req 4.2.2/SWS_EthIf_00208 */
Std_ReturnType EthIf_GetDropCount( uint8 SwitchIdx, uint32* DropCount );
/* @req 4.2.2/SWS_EthIf_00214 */
Std_ReturnType EthIf_StoreConfiguration( uint8 SwitchIdx );
/* @req 4.2.2/SWS_EthIf_00219 */
Std_ReturnType EthIf_ResetConfiguration( uint8 SwitchIdx );
/* @req 4.2.2/SWS_EthIf_00154 */
Std_ReturnType EthIf_GetCurrentTime( uint8 CtrlIdx, Eth_TimeStampQualType* timeQualPtr, Eth_TimeStampType* timeStampPtr );
/* @req 4.2.2/SWS_EthIf_00160 */
void EthIf_EnableEgressTimeStamp( uint8 CtrlIdx, Eth_BufIdxType BufIdx );
/* @req 4.2.2/SWS_EthIf_00166 */
void EthIf_GetEgressTimeStamp(uint8 CtrlIdx, Eth_BufIdxType BufIdx, Eth_TimeStampQualType* timeQualPtr, Eth_TimeStampType* timeStampPtr );
/* @req 4.2.2/SWS_EthIf_00172 */
void EthIf_GetIngressTimeStamp( uint8 CtrlIdx, Eth_DataType* DataPtr, Eth_TimeStampQualType* timeQualPtr, Eth_TimeStampType* timeStampPtr );
/* @req 4.2.2/SWS_EthIf_00178 */
void EthIf_SetCorrectionTime( uint8 CtrlIdx, const Eth_TimeIntDiffType* timeOffsetPtr, const Eth_RateRatioType* rateRatioPtr );
/* @req 4.2.2/SWS_EthIf_00184 */
Std_ReturnType EthIf_SetGlobalTime( uint8 CtrlIdx, const Eth_TimeStampType* timeStampPtr );
/* @req 4.2.2/SWS_EthIf_00067 */
BufReq_ReturnType EthIf_ProvideTxBuffer( uint8 CtrlIdx, Eth_FrameType FrameType, uint8 Priority, Eth_BufIdxType* BufIdxPtr, uint8** BufPtr, uint16* LenBytePtr );
/* @req 4.2.2/SWS_EthIf_00075 */
Std_ReturnType EthIf_Transmit( uint8 CtrlIdx, Eth_BufIdxType BufIdx, Eth_FrameType FrameType, boolean TxConfirmation, uint16 LenByte, const uint8* PhysAddrPtr );

/* @req 4.2.2/SWS_EthIf_00082 */
#if (ETHIF_VERSION_INFO_API == STD_ON)
#if (ETHIF_VERSION_INFO_API_MACRO == STD_ON)
#define EthIf_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,ETHIF) /* @req 4.2.2/SWS_EthIf_00127 */
#else
void EthIf_GetVersionInfo( Std_VersionInfoType* VersionInfoPtr );
#endif
#endif

/* @req 4.2.2/SWS_EthIf_00097 */
void EthIf_MainFunctionRx( void );
/* @req  4.2.2/SWS_EthIf_00113 */
void EthIf_MainFunctionTx( void );

#endif /* ETHIF_H_ */
