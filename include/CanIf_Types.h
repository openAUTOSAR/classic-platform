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

/** @addtogroup CanIf CAN Interface
 *  @{ */

/** @file CanIf_Types.h
 *  Definitions of configuration parameters for CAN Interface.
 */

#ifndef CANIF_TYPES_H_
#define CANIF_TYPES_H_

#include "ComStack_Types.h"

// API service with wrong parameter
/** @name Error Codes */
//@{
#define CANIF_E_PARAM_CANID		     	10
#define CANIF_E_PARAM_DLC			 	11
#define CANIF_E_PARAM_HRH			  	12
#define CANIF_E_PARAM_LPDU            	13
#define CANIF_E_PARAM_CONTROLLER	  	14
#define CANIF_E_PARAM_CONTROLLERID	  	15
#define CANIF_E_PARAM_WAKEUPSOURCE		16
#define CANIF_E_PARAM_TRCV             	17
#define CANIF_E_PARAM_TRCVMODE          18
#define CANIF_E_PARAM_TRCVWAKEUPMODE    19
#define CANIF_E_PARAM_CTRLMODE 			21

#define CANIF_E_PARAM_POINTER 			20
#define CANIF_E_UNINIT 				    30
//#define CANIF_E_NOK_NOSUPPORT			40
#define CANIF_E_INVALID_TXPDUID		    50
#define CANIF_E_INVALID_RXPDUID 		60
#define CANIF_E_INVALID_DLC		 		61

#define CANIF_E_STOPPED			 		70
#define CANIF_E_NOT_SLEEP			 	71
//@}

typedef enum {
	/** UNINIT mode. Default mode of the CAN driver and all
	 *  CAN controllers connected to one CAN network after
	 *  power on. */
	CANIF_CS_UNINIT = 0,
	                           
	/**  STOPPED mode. At least one of all CAN controllers
	 *   connected to one CAN network are halted and does
	 *   not operate on the bus. */
	CANIF_CS_STOPPED,	                           
	                           
	/** STARTED mode. All CAN controllers connected to
	 *  one CAN network are started by the CAN driver and
	 *  in full-operational mode. */
	CANIF_CS_STARTED,
	
	/** SLEEP mode. At least one of all CAN controllers
	 *  connected to one CAN network are set into the
	 *  SLEEP mode and can be woken up by request of the
	 *  CAN driver or by a network event (must be supported
	 *  by CAN hardware) */
	CANIF_CS_SLEEP
} CanIf_ControllerModeType;

/** Status of the PDU channel group. Current mode of the channel defines its
 *  transmit or receive activity. Communication direction (transmission and/or
 *  reception) of the channel can be controlled separately or together by upper
 *  layers. */
typedef enum {
	/** Channel shall be set to the offline mode
	 *  => no transmission and reception */
	CANIF_SET_OFFLINE = 0,
	
	/** Receive path of the corresponding channel
	 *  shall be disabled */
	CANIF_SET_RX_OFFLINE,

	/** Receive path of the corresponding channel
	 *  shall be enabled */
	CANIF_SET_RX_ONLINE,
	
	/** Transmit path of the corresponding channel
	 *  shall be disabled */
	CANIF_SET_TX_OFFLINE,
	
	/** Transmit path of the corresponding channel
	 *  shall be enabled */
	CANIF_SET_TX_ONLINE,
	
	/** Channel shall be set to online mode
	 *  => full operation mode */
	CANIF_SET_ONLINE,
	
	/** Transmit path of the corresponding channel
	 *  shall be set to the offline active mode
	 *  => notifications are processed but transmit
	 *  requests are blocked. */
	CANIF_SET_TX_OFFLINE_ACTIVE
} CanIf_PduSetModeType;


typedef enum {
	/** Channel is in the offline mode ==> no transmission or reception */
  CANIF_GET_OFFLINE = 0,
  /** Receive path of the corresponding channel is enabled and
   *  transmit path is disabled */
  CANIF_GET_RX_ONLINE,
  /** Transmit path of the corresponding channel is enabled and
   *  receive path is disabled */
  CANIF_GET_TX_ONLINE,
  /** Channel is in the online mode ==> full operation mode */
  CANIF_GET_ONLINE,
  /** Transmit path of the corresponding channel is in
   *  the offline mode ==> transmit notifications are processed but
   *  transmit requests are blocked. The receiver path is disabled. */
  CANIF_GET_OFFLINE_ACTIVE,
  /** Transmit path of the corresponding channel is in the offline
   *  active mode ==> transmit notifications are processed but transmit
   *  requests are blocked. The receive path is enabled. */
  CANIF_GET_OFFLINE_ACTIVE_RX_ONLINE
	
} CanIf_PduGetModeType;

typedef enum {
	/** No transmit or receive event occurred for
	 *  the requested L-PDU. */
	CANIF_NO_NOTIFICATION = 0,	
	/** The requested Rx/Tx CAN L-PDU was
	 *  successfully transmitted or received. */
	CANIF_TX_RX_NOTIFICATION
	
} CanIf_NotifStatusType;

typedef enum {
	/** Transceiver mode NORMAL */
  CANIF_TRCV_MODE_NORMAL = 0,
  /** Transceiver mode STANDBY */
  CANIF_TRCV_MODE_STANDBY,
  /** Transceiver mode SLEEP */
  CANIF_TRCV_MODE_SLEEP
} CanIf_TrcvModeType;

typedef enum {
  /** Due to an error wake up reason was not detected.
   *  This value may only be reported when error was
   *  reported to DEM before. */
	CANIF_TRCV_WU_ERROR = 0,
	/** The transceiver does not support any information
	 *  for the wakeup reason. */
	CANIF_TRCV_WU_NOT_SUPPORTED,
	/** The transceiver has detected, that the network has
	 *  caused the wake up of the ECU */
	CANIF_TRCV_WU_BY_BUS,
	/** The transceiver detected, that the network has woken
	 *  the ECU via a request to NORMAL mode */
	CANIF_TRCV_WU_INTERNALLY,
	/** The transceiver has detected, that the "wake up"
	 *  is due to an ECU reset */
	CANIF_TRCV_WU_RESET,
	/** The transceiver has detected, that the "wake up"
	 *  is due to an ECU reset after power on. */
	CANIF_TRCV_WU_POWER_ON
} CanIf_TrcvWakeupReasonType;

typedef enum {
  /** The notification for wakeup events is enabled
   *  on the addressed network. */
	CANIF_TRCV_WU_ENABLE = 0,
	/** The notification for wakeup events is disabled
	 *  on the addressed network. */
	CANIF_TRCV_WU_DISABLE,
	/** A stored wakeup event is cleared on the addressed network */
	CANIF_TRCV_WU_CLEAR
} CanIf_TrcvWakeupModeType;

#endif /*CANIF_TYPES_H_*/
/** @} */
