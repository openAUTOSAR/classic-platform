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


#ifndef CANIF_H_
#define CANIF_H_
#include "Modules.h"

#if defined(USE_PDUR)
#include "PduR.h"
#endif

#if defined(USE_COM)
#include "Com.h"
#endif

#define CANIF_VENDOR_ID          VENDOR_ID_ARCCORE
#define CANIF_AR_RELEASE_MAJOR_VERSION   	4
#define CANIF_AR_RELEASE_MINOR_VERSION   	0
#define CANIF_AR_RELEASE_REVISION_VERSION   3


#define CANIF_MODULE_ID          MODULE_ID_CANIF
#define CANIF_AR_MAJOR_VERSION   CANIF_AR_RELEASE_MAJOR_VERSION
#define CANIF_AR_MINOR_VERSION   CANIF_AR_RELEASE_MINOR_VERSION
#define CANIF_AR_PATCH_VERSION   CANIF_AR_RELEASE_REVISION_VERSION

#define CANIF_SW_MAJOR_VERSION   3
#define CANIF_SW_MINOR_VERSION   0
#define CANIF_SW_PATCH_VERSION   0

#if defined(USE_DET)
#include "Det.h"
#endif
#include "CanIf_Types.h"
#include "CanIf_Cfg.h"


// Service IDs
#define CANIF_INIT_ID                0x01
#define CANIF_INIT_CONTROLLER_ID     0x02
#define CANIF_SET_CONTROLLER_MODE_ID 0x03
#define CANIF_GET_CONTROLLER_MODE_ID 0x04
#define CANIF_TRANSMIT_ID            0x05
#define CANIF_READTXPDUDATA_ID       0x06
#define CANIF_READTXNOTIFSTATUS_ID   0x07
#define CANIF_READRXNOTIFSTATUS_ID   0x08
#define CANIF_SETPDUMODE_ID          0x09
#define CANIF_GETPDUMODE_ID          0x0A
#define CANIF_SETDYNAMICTX_ID        0x0C
#define CANIF_SET_TRANSCEIVERMODE_ID     0x0D
#define CANIF_GET_TRANSCEIVERMODE_ID     0x0E
#define CANIF_GET_TRCVMODEREASON_ID      0x0F
#define CANIF_SET_TRANSCEIVERWAKEMODE_ID 0x10
#define CANIF_CHECKWAKEUP_ID          0x11
#define CANIF_CHECKVALIDATION_ID      0x12
#define CANIF_TXCONFIRMATION_ID       0x13
#define CANIF_RXINDICATION_ID         0x14
#define CANIF_CANCELTXCONFIRMATION_ID 0x15
#define CANIF_CONTROLLER_BUSOFF_ID    0x16

#define CANIF_CONTROLLER_MODE_INDICATION_ID 0x17

#define CANIF_SETWAKEUPEVENT_ID       0x40
#define CANIF_ARCERROR_ID           0x41

void CanIf_Init(const CanIf_ConfigType *ConfigPtr);

Std_ReturnType CanIf_SetControllerMode(uint8 Controller,
		CanIf_ControllerModeType ControllerMode);

Std_ReturnType CanIf_GetControllerMode(uint8 Controller,
		CanIf_ControllerModeType *ControllerModePtr);

Std_ReturnType CanIf_Transmit(PduIdType CanTxPduId,
		const PduInfoType *PduInfoPtr);

#if ( CANIF_PUBLIC_READRXPDU_DATA_API == STD_ON )
Std_ReturnType CanIf_ReadRxPduData(PduIdType CanRxPduId,
		PduInfoType *PduInfoPtr);
#endif

#if ( CANIF_PUBLIC_READTXPDU_NOTIFY_STATUS_API == STD_ON )
CanIf_NotifStatusType CanIf_ReadTxNotifStatus(PduIdType CanTxPduId);
#endif

#if ( CANIF_PUBLIC_READRXPDU_NOTIFY_STATUS_API == STD_ON )
CanIf_NotifStatusType CanIf_ReadRxNotifStatus(PduIdType CanRxPduId);
#endif

Std_ReturnType CanIf_SetPduMode( uint8 Controller, CanIf_PduSetModeType PduModeRequest );
Std_ReturnType CanIf_GetPduMode( uint8 Controller, CanIf_PduGetModeType *PduModePtr );

#if ( CANIF_PUBLIC_SETDYNAMICTXID_API == STD_ON )
void CanIf_SetDynamicTxId( PduIdType CanTxPduId, Can_IdType CanId );
#endif

#if ( CANIF_ARC_RUNTIME_PDU_CONFIGURATION == STD_ON )
CanIf_TxPduConfigType * CanIf_FindTxPduEntry(PduIdType id);
CanIf_RxPduConfigType * CanIf_FindRxPduEntry(PduIdType id);
const CanIf_HrhConfigType* CanIf_Arc_GetReceiveHandler(CanIf_Arc_ChannelIdType Channel);
const CanIf_HthConfigType* CanIf_Arc_GetTransmitHandler(CanIf_Arc_ChannelIdType Channel);
#endif

#if ( CANIF_ARC_TRANSCEIVER_API == STD_ON )
Std_ReturnType CanIf_SetTrcvMode( uint8 Transceiver, CanIf_TrcvModeType TransceiverMode );
Std_ReturnType CanIf_GetTrcvMode( uint8 Transceiver, CanIf_TrcvModeType *TransceiverModePtr );
Std_ReturnType CanIf_GetTrcvWakeupReason( uint8 Transceiver, CanIf_TrcvWakeupReasonType *TrcvWuReasonPtr );
Std_ReturnType CanIf_SetTransceiverWakeupMode( uint8 Transceiver, CanIf_TrcvWakeupModeType *TrcvWakeupMode );
#endif

#if ( CANIF_CTRL_WAKEUP_SUPPORT == STD_ON ) || (CANIF_TRCV_WAKEUP_SUPPORT == STD_ON)
Std_ReturnType CanIf_CheckWakeup( EcuM_WakeupSourceType WakeupSource );
#if ( CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON)
Std_ReturnType CanIf_CheckValidation( EcuM_WakeupSourceType WakeupSource );
#endif
#endif

/* @req 4.0.3/CANIF351 */
#if ( CANIF_PUBLIC_VERSION_INFO_API == STD_ON )
/* @req 4.0.3/CANIF158 */
/* @req 4.0.3/CANIF350 */
#define CanIf_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,CANIF)
#endif

#if ( CANIF_PUBLIC_CHANGE_BAUDRATE_SUPPORT == STD_ON )
Std_ReturnType CanIf_CheckBaudrate(uint8 ControllerId, const uint16 Baudrate);
Std_ReturnType CanIf_ChangeBaudrate(uint8 ControllerId, const uint16 Baudrate);
#endif

#if ( CANIF_PUBLIC_TXCONFIRM_POLLING_SUPPORT == STD_ON )
CanIf_NotifStatusType CanIf_GetTxConfirmationState( uint8 ControllerId );
#endif

#if ( CANIF_PUBLIC_PN_SUPPORT == STD_ON )
Std_ReturnType CanIf_ClearTrcvWufFlag( uint8 TransceiverId );
Std_ReturnType CanIf_CheckTrcvWakeFlag( uint8 TransceiverId );
#endif

void CanIf_ControllerModeIndication(uint8 ControllerId,  CanIf_ControllerModeType ControllerMode);

uint8 CanIf_Arc_GetChannelDefaultConfIndex(CanIf_Arc_ChannelIdType Channel);

#if (CANIF_PUBLIC_CANCEL_TRANSMIT_SUPPORT == STD_ON)
Std_ReturnType CanIf_CancelTransmit(PduIdType CanTxPduId);
#endif

#endif /*CANIF_H_*/
