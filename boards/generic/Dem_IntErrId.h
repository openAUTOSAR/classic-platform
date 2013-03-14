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








#warning "This default file may only be used as an example!"

#ifndef DEM_INTERRID_H_
#define DEM_INTERRID_H_

#endif /*DEM_INTERRID_H_*/
/*
 * Definition of event IDs used by BSW
 * NB! Must be unique for each event!
 */

enum {
	// Event IDs from DEM module
	DEM_EVENT_ID_NULL = 0,			// Do not change this entry!!!

	// Event IDs from MCU
	MCU_E_CLOCK_FAILURE,

	// Event IDs from CAN
	CANTRCV_E_NO_TRCV_CONTROL,
	CANTP_E_OPER_NOT_SUPPORTED,
	CANTP_E_COMM,
	CANNM_E_CANIF_TRANSMIT_ERROR,
	CANNM_E_NETWORK_TIMEOUT,
	CANIF_TRCV_E_TRANSCEIVER,
	CANIF_E_INVALID_DLC,
	CANIF_STOPPED,
	CANIF_E_FULL_TX_BUFFER,
	CAN_E_TIMEOUT,

	// Event IDs from EEPROM
	EEP_E_COM_FAILURE,

	// Event IDs from flash
	FLS_E_ERASE_FAILED,
	FLS_E_WRITE_FAILED,
	FLS_E_READ_FAILED,
	FLS_E_COMPARE_FAILED,
	FLS_E_UNEXPECTED_FLASH_ID,

	// Event IDs from LIN
	LIN_E_TIMEOUT,
	LINIF_E_RESPONSE,
	LINIF_E_NC_NO_RESPONSE,
	LINIF_E_CHANNEL_X_SLAVE_Y,

	// Event IDs from ECU
	ECUM_E_RAM_CHECK_FAILED,
	ECUM_E_ALL_RUN_REQUESTS_KILLED,
	ECUM_E_CONFIGURATION_DATA_INCONSISTENT,

	// Event IDs from COM
//	COMM_E_NET_START_IND_CHANNEL_<X>,

	// Event IDs from PDUR
	PDUR_E_PDU_INSTANCE_LOST,
	PDUR_E_INIT_FAILED,

	// Event IDs from WDGM
	WDGM_E_ALIVE_SUPERVISION,
	WDGM_E_SET_MODE,

	// DEM last event id for BSW
	DEM_EVENT_ID_LAST_FOR_BSW
};
