/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/

/** @addtogroup TBD
 *  @{ */

/** @file CanTp.h
 * TBD.
 */

/** @req CANTP157 */

#ifndef CANTP_H_
#define CANTP_H_

#define CANTP_SW_MAJOR_VERSION    	1
#define CANTP_SW_MINOR_VERSION 		0
#define CANTP_SW_PATCH_VERSION    	0
#define CANTP_AR_MAJOR_VERSION    	3
#define CANTP_AR_MINOR_VERSION    	0
#define CANTP_AR_PATCH_VERSION    	1


#include "ComStack_Types.h"
#include "Std_Types.h"
#include "CanTp_Cfg.h"  /** @req CANTP221 */
//#include "Dem.h"  // TODO: req: CanTp156

/*
 *
 * Errors described by CanTp 7.4 Error classification.
 *
 ****************************/

#define CANTP_E_PARAM_CONFIG		0x01
#define CANTP_E_PARAM_ID			0x02
#define CANTP_E_PARAM_ADDRESS		0x04

#define CANTP_E_UNINIT				0x20
#define CANTP_E_INVALID_TX_ID		0x30
#define CANTP_E_INVALID_RX_ID		0x40
#define CANTP_E_INVALID_TX_BUFFER	0x50
#define CANTP_E_INVALID_RX_BUFFER	0x60
#define CANTP_E_INVALID_TX_LENGHT	0x70
#define CANTP_E_INVALID_RX_LENGTH	0x80
#define CANTP_E_INVALID_TATYPE		0x90

/*
 * Service IDs for CanTP function definitions.
 */

#define SERVICE_ID_CANTP_INIT						0x01
#define SERVICE_ID_CANTP_GET_VERSION_INFO			0x07
#define SERVICE_ID_CANTP_SHUTDOWN					0x02
#define SERVICE_ID_CANTP_TRANSMIT					0x03
#define SERVICE_ID_CANTP_CANCEL_TRANSMIT_REQUEST	0x03
#define SERVICE_ID_CANTP_MAIN_FUNCTION				0x06
#define SERVICE_ID_CANTP_RX_INDICATION				0x04
#define SERVICE_ID_CANTP_TX_CONFIRMATION			0x05


/*
 * Structs
 ****************************/

typedef enum {
	FRTP_CNLDO,
	FRTP_CNLNB,
	FRTP_CNLOR
} FrTp_CancelReasonType;



/*
 * Implemented functions
 ****************************/

void CanTp_Init(); /** req : CanTp208 **/

void CanTp_GetVersionInfo( Std_VersionInfoType* versioninfo ); /** req : CanTp210 **/

void CanTp_Shutdown();

Std_ReturnType CanTp_Transmit( PduIdType CanTpTxSduId, const PduInfoType * CanTpTxInfoPtr );

Std_ReturnType FrTp_CancelTransmitRequest( PduIdType FrTpTxPduId, FrTp_CancelReasonType FrTpCancelReason );

void CanTp_MainFunction();


#endif /* CANTP_H_ */
