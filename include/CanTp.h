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

/** @req CANTP157 */

#ifndef CANTP_H_
#define CANTP_H_

#include "Modules.h"

#define CANTP_MODULE_ID				MODULE_ID_CANTP	/** @req CANTP115 */
#define CANTP_VENDOR_ID				VENDOR_ID_ARCCORE

/** @req CANTP024 */
/** @req CANTP266 */
#define CANTP_SW_MAJOR_VERSION    	1
#define CANTP_SW_MINOR_VERSION 		0
#define CANTP_SW_PATCH_VERSION    	0
#define CANTP_AR_RELEASE_MAJOR_VERSION    	4
#define CANTP_AR_RELEASE_MINOR_VERSION    	0
#define CANTP_AR_RELEASE_PATCH_VERSION    	3

#define CANTP_AR_MAJOR_VERSION    	CANTP_AR_RELEASE_MAJOR_VERSION
#define CANTP_AR_MINOR_VERSION    	CANTP_AR_RELEASE_MINOR_VERSION
#define CANTP_AR_PATCH_VERSION    	CANTP_AR_RELEASE_PATCH_VERSION


#include "ComStack_Types.h" /** @req CANTP209 */ /** @req CANTP264.partially */
#include "Std_Types.h" /** @req CANTP209 */
#include "CanTp_Cfg.h"  /** @req CANTP221 */
/*
 *
 * Errors described by CanTp 7.4 Error classification.
 *
 ****************************/
/** @req CANTP101 */
#define CANTP_E_PARAM_CONFIG		0x01
#define CANTP_E_PARAM_ID			0x02
#define CANTP_E_PARAM_POINTER       0x03
#define CANTP_E_PARAM_ADDRESS		0x04

#define CANTP_E_UNINIT				0x20
#define CANTP_E_INVALID_TX_ID		0x30
#define CANTP_E_INVALID_RX_ID		0x40
#define CANTP_E_INVALID_TX_BUFFER	0x50
#define CANTP_E_INVALID_RX_BUFFER	0x60
#define CANTP_E_INVALID_TX_LENGHT	0x70
#define CANTP_E_INVALID_RX_LENGTH	0x80
#define CANTP_E_INVALID_TATYPE		0x90
#define CANTP_E_OPER_NOT_SUPPORTED  0xA0
#define CANTP_E_COM                 0xB0
#define CANTP_E_RX_COM              0xC0
#define CANTP_E_TX_COM              0xD0
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

void CanTp_Init(const CanTp_ConfigType* CfgPtr); /** @req CANTP208 **/

#if ( CANTP_VERSION_INFO_API == STD_ON ) /** @req CANTP162 *//** @req CANTP163 */ /** @req CANTP267 */ /** @req CANTP297 */
#define CanTp_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,CANTP) /** @req CANTP210 */ /* @req CANTP218 */ /** @req CANTP308 */
#endif /* CANTP_VERSION_INFO_API */

void CanTp_Shutdown(void); /** @req CANTP211 */

Std_ReturnType CanTp_Transmit( PduIdType CanTpTxSduId, const PduInfoType * CanTpTxInfoPtr ); /** @req CANTP212 */

void CanTp_MainFunction(void); /** @req CANTP213 */


#endif /* CANTP_H_ */
