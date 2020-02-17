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


#ifndef CANSM_CBK_H_
#define CANSM_CBK_H_

/** @req CANSM011 */

/** The CanSM is notified about a bus-off event on a certain CAN controller with this
  * call-out function. It shall execute the bus-off recovery state machine for the
  * corresponding network handle. */
/* @req CANSM064 */
void CanSM_ControllerBusOff( uint8 ControllerId );

/* @req CANSM396 */
void CanSM_ControllerModeIndication( uint8 ControllerId, CanIf_ControllerModeType ControllerMode );

#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
/* @req CANSM399 */
void CanSM_TransceiverModeIndication( uint8 TransceiverId, CanTrcv_TrcvModeType TransceiverMode );
#endif


#endif /* CANSM_CBK_H_ */
