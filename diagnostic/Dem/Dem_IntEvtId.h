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








#ifndef DEMINTEVTID_H_
#define DEMINTEVTID_H_

/*
 * Definition of event IDs used by SW-C
 * NB! Must be unique for each event
 */
enum {
	DEM_EVENT_ID_NULL = 0,			// Do not change!!!
	DEM_EVENT_ID_TEMP_HIGH,
	DEM_EVENT_ID_TEMP_LOW,
	DEM_EVENT_ID_BLOW,
	DEM_EVENT_ID_ETC_ETC
};


#endif /*DEMINTEVTID_H_*/
