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









#define PDUR_MODULE_ID		51
#define PDUR_INSTANCE_ID	0

#define MUST_BE_OFF 		0

/* Turns on/off debug print statements in the PDU router code. */
//#define PDUR_PRINT_DEBUG_STATEMENTS

/* Turns on/off reentrancy checks */
#define PDUR_REENTRANCY_CHECK


#define PDUR_DEV_ERROR_DETECT 1 // Should this be enables in DASA configuration?

#define PDUR_VERSION_INFO_API

#if (PDUR_ZERO_COST_OPERATION == STD_ON)
	// CanIf, FrIf, LinIf
	#define PDUR_SINGLE_IF					CAN_IF
	// CanTp, FrTp, LinTp
	//#define PDUR_SINGLE_TP					CanTp
	#define PDUR_MULTICAST_TOIF_SUPPORT		MUST_BE_OFF
	#define PDUR_MULTICAST_FROMIF_SUPPORT	MUST_BE_OFF
	#define PDUR_MULTICAST_TOTP_SUPPORT		MUST_BE_OFF
	#define PDUR_MULTICAST_FROMTP_SUPPORT	MUST_BE_OFF

#else
	#define PDUR_GATEWAY_OPERATION
	#ifdef PDUR_GATEWAY_OPERATION
		#define PDUR_MEMORY_SIZE
		//#define PDUR_SB_TX_BUFFER_SUPPORT
		#define PDUR_FIFO_TX_BUFFER_SUPPORT
		#define PDUR_MAX_TX_BUFFER_NUMBER 10
	#endif

	//#define PDUR_IPDUM_SUPPORT

	// NOTE: Support for minimum routing not implemented yet.
	/*
	#define PDUR_MINIMUM_ROUTING_UP_MODULE	COM
	#define PDUR_MINIMUM_ROUTING_LO_MODULE	CAN_IF
	#define PDUR_MINIMUM_ROUTING_UP_RXPDUID	((PduIdType)100)
	#define PDUR_MINIMUM_ROUTING_LO_RXPDUID ((PduIdType)255)
	#define PDUR_MINIMUM_ROUTING_UP_TXPDUID ((PduIdType)255)
	#define PDUR_MINIMUM_ROUTING_LO_TXPDUID ((PduIdType)255)
	*/
#endif

// Interfaces
#define PDUR_CANIF_SUPPORT			STD_ON
#define PDUR_CANTP_SUPPORT			STD_OFF
#define PDUR_FRIF_SUPPORT			STD_OFF
#define PDUR_FRTP_SUPPORT			STD_OFF
#define PDUR_LINIF_SUPPORT			STD_OFF
#define PDUR_LINTP_SUPPORT			STD_OFF
#define PDUR_COM_SUPPORT			STD_ON
#define PDUR_DCM_SUPPORT			STD_OFF


//#define PDUR_MAX_TX_BUFFER_NUMBER

// ERROR TYPES
#define PDUR_E_CONFIG_PTR_INVALID 	0x06
#define PDUR_E_INVALID_REQUEST 		0x01
#define PDUR_E_PDU_ID_INVALID		0x02
#define PDUR_E_TP_TX_REQ_REJECTED	0x03
#define PDUR_E_DATA_PTR_INVALID		0x05
