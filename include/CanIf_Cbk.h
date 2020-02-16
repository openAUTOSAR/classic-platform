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








#ifndef CANIF_CBK_H_
#define CANIF_CBK_H_

void CanIf_TxConfirmation( PduIdType canTxPduId );
#if 0
void CanIf_RxIndication( Can_HwHandleType Hrh, Can_IdType CanId, uint8 CanDlc, const uint8 *CanSduPtr );
#endif
/* TODO: hrh should not be uint8 but Can_HwHandleType */
void CanIf_RxIndication( uint8 Hrh, Can_IdType CanId, uint8 CanDlc, const uint8 *CanSduPtr );

void CanIf_CancelTxConfirmation(PduIdType canTxPduId,  const PduInfoType *pduInfoPtr);

void CanIf_ControllerBusOff( uint8 Controller );
void CanIf_SetWakeupEvent( uint8 Controller );

#if ( CANIF_PUBLIC_PN_SUPPORT == STD_ON )
void CanIf_ConfirmPnAvailability( uint8 TransceiverId );
void CanIf_ClearTrcvWufFlagIndication( uint8 TransceiverId );
void CanIf_CheckTrcvWakeFlagIndication( uint8 TransceiverId );
#endif

#if ( CANIF_TRANSCEIVER_API == STD_ON )
void CanIf_TrcvModeIndication( uint8 TransceiverId, CanIf_TrcvModeType TransceiverMode );
#endif

/* ArcCore extensions */
void CanIf_Arc_Error( uint8 Controller, Can_Arc_ErrorType Error );

/* CANIF699 */
void CanIf_ControllerModeIndication( uint8 ControllerId, CanIf_ControllerModeType ControllerMode);

#endif /*CANIF_CBK_H_*/
