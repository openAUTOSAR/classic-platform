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

#if defined(USE_PDUR)
#include "PduR.h"
#endif

#if defined(USE_COM)
#include "Com.h"
#endif

/* @req 4.0.3/CANIF672 */
/* @req 4.0.3/CANIF725 */
/* @req 4.0.3/CANIF726 */
/* @req 4.0.3/CANIF727 */
/* @req 4.0.3/CANIF728 */
/* @req 4.0.3/CANIF729 */
/* @req 4.0.3/CANIF456 */
/* @req 4.0.3/CANIF563 */
/* @req 4.0.3/CANIF564 */
/* @req 4.0.3/CANIF659 */
/* @req 4.0.3/CANIF695 */
/* @req 4.0.3/CANIF696 */
/* @req 4.0.3/CANIF697 */
/* @req 4.0.3/CANIF698 */
/* @req 4.0.3/CANIF710 */
/* @req 4.0.3/CANIF730 */
/* @req 4.0.3/CANIF794 */
/* @req 4.0.3/CANIF795 */
/* @req 4.0.3/CANIF796 */
/* @req 4.0.3/CANIF797 */
/* @req 4.0.3/CANIF798 */
/* @req 4.0.3/CANIF800 */
/* @req 4.0.3/CANIF801 */
/* @req 4.0.3/CANIF802 */
/* @req 4.0.3/CANIF803 */
/* @req 4.0.3/CANIF804 */
/* @req 4.0.3/CANIF823 */
/* @req 4.0.3/CANIF824 */
/* @req 4.0.3/CANIF825 */
/* @req 4.0.3/CANIF826 */
/* @req 4.0.3/CANIF827 */
/* @req 4.0.3/CANIF788 */
/* @req 4.0.3/CANIF799 */
/* @req 4.0.3/CANIF814 */
/* @req 4.0.3/CANIF821 */
/* @req 4.0.3/CANIF822 */
/* @req 4.0.3/CANIF712 */
/* @req 4.0.3/CANIF655 */
/* @req 4.0.3/CANIF693 */
/* @req 4.0.3/CANIF694 */
/* @req 4.0.3/CANIF455 */
/* @req 4.0.3/CANIF532 */
/* @req 4.0.3/CANIF224 */
/* @req 4.0.3/CANIF793 */
#define CANIF_VENDOR_ID                     60u
#define CANIF_AR_RELEASE_MAJOR_VERSION      4u
#define CANIF_AR_RELEASE_MINOR_VERSION      0u
#define CANIF_AR_RELEASE_REVISION_VERSION   3u


#define CANIF_MODULE_ID                     60u
#define CANIF_AR_MAJOR_VERSION              CANIF_AR_RELEASE_MAJOR_VERSION
#define CANIF_AR_MINOR_VERSION              CANIF_AR_RELEASE_MINOR_VERSION
#define CANIF_AR_PATCH_VERSION              CANIF_AR_RELEASE_REVISION_VERSION

#define CANIF_SW_MAJOR_VERSION              5u
#define CANIF_SW_MINOR_VERSION              3u
#define CANIF_SW_PATCH_VERSION              0u

#if defined(USE_DET)
#include "Det.h"
#endif
#include "CanIf_Types.h"/* Part of CANIF643 */

/* @req 4.0.3/CANIF376 */
#include "CanIf_Cfg.h"

#if (( CANIF_CTRL_WAKEUP_SUPPORT == STD_ON ) || (CANIF_TRCV_WAKEUP_SUPPORT == STD_ON))
#include "EcuM.h"
#endif
#if (CANIF_ARC_TRANSCEIVER_API == STD_ON)
#include "CanTrcv.h"
#endif


// Service IDs
#define CANIF_INIT_ID                       0x01u
#define CANIF_INIT_CONTROLLER_ID            0x02u
#define CANIF_SET_CONTROLLER_MODE_ID        0x03u
#define CANIF_GET_CONTROLLER_MODE_ID        0x04u
#define CANIF_TRANSMIT_ID                   0x05u
#define CANIF_READTXPDUDATA_ID              0x06u
#define CANIF_READTXNOTIFSTATUS_ID          0x07u
#define CANIF_READRXNOTIFSTATUS_ID          0x08u
#define CANIF_SETPDUMODE_ID                 0x09u
#define CANIF_GETPDUMODE_ID                 0x0Au
#define CANIF_SETDYNAMICTX_ID               0x0Cu
#define CANIF_SET_TRANSCEIVERMODE_ID        0x0Du
#define CANIF_GET_TRANSCEIVERMODE_ID        0x0Eu
#define CANIF_GET_TRCVWAKEUPREASON_ID       0x0Fu
#define CANIF_SET_TRANSCEIVERWAKEMODE_ID    0x10u
#define CANIF_CHECKWAKEUP_ID                0x11u
#define CANIF_CHECKVALIDATION_ID            0x12u
#define CANIF_TXCONFIRMATION_ID             0x13u
#define CANIF_RXINDICATION_ID               0x14u
#define CANIF_CANCELTXCONFIRMATION_ID       0x15u
#define CANIF_CONTROLLER_BUSOFF_ID          0x16u
#define CANIF_TRANSCEIVER_MODE_INDICATION_ID 0x18u
#define CANIF_CONFIRM_PNAVAILABILITY_ID     0x1au
#define CANIF_CLEARTRCVWUFFLAG_ID           0x1eu
#define CANIF_CHECKTRCVWAKEFLAG_ID          0x1fu
#define CANIF_CLEARTRCV_WUFFLAG_INDICATION  0x20u
#define CANIF_CHECKTRCV_WAKEFLAG_INDICATION 0x21

#define CANIF_CONTROLLER_MODE_INDICATION_ID 0x17u

#define CANIF_SETWAKEUPEVENT_ID             0x40u
#define CANIF_ARCERROR_ID                   0x41u

/* @req 4.0.3/CANIF116 */
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

#if ( CANIF_ARC_TRANSCEIVER_API == STD_ON )
Std_ReturnType CanIf_SetTrcvMode( uint8 TransceiverId, CanTrcv_TrcvModeType TransceiverMode );
Std_ReturnType CanIf_GetTrcvMode( CanTrcv_TrcvModeType *TransceiverModePtr, uint8 TransceiverId);
Std_ReturnType CanIf_GetTrcvWakeupReason( uint8 TransceiverId, CanTrcv_TrcvWakeupReasonType *TrcvWuReasonPtr );
/* @req 4.0.3/CANIF290 */
Std_ReturnType CanIf_SetTrcvWakeupMode(uint8 TransceiverId,CanTrcv_TrcvWakeupModeType TrcvWakeupMode);
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
/* !req 4.0.3/CANIF658 */
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


#if (CANIF_PUBLIC_CANCEL_TRANSMIT_SUPPORT == STD_ON)
Std_ReturnType CanIf_CancelTransmit(PduIdType CanTxPduId);
#endif

#if ( CANIF_PUBLIC_PN_SUPPORT == STD_ON )
/* @req 4.0.3/CANIF815 */
void CanIf_ConfirmPnAvailability( uint8 TransceiverId );

/* @req 4.0.3/CANIF762 */
void CanIf_ClearTrcvWufFlagIndication( uint8 TransceiverId );

/* @req 4.0.3/CANIF763 */
void CanIf_CheckTrcvWakeFlagIndication( uint8 TransceiverId );
#endif

#endif /*CANIF_H_*/
