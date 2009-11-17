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








#ifndef DEM_INTERRID_H_
#define DEM_INTERRID_H_

#endif /*DEM_INTERRID_H_*/
/*
 * Definition of event IDs used by BSW
 * NB! Must be unique for each event
 */

enum {
	// Event IDs from DEM module
	DEM_EVENT_ID_NULL = 0,			// Do not change!!!
	DEM_EVENT_ID_TEMP_HIGH,
	DEM_EVENT_ID_TEMP_LOW,
	DEM_EVENT_ID_BLOW,
	DEM_EVENT_ID_ETC_ETC,

	// Events IDs from PDUR
	PDUR_E_INSTANCE_LOST,

	// DEM last event id for BSW
	DEM_EVENT_ID_LAST_FOR_BSW
};
