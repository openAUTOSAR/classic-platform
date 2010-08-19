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

#include "Dem.h"

/*********************
 * DEM Configuration *
 *********************/


/*
 * Classes of extended data record
 */


/*
 * Classes of extended data
 */


/*
 * Classes of freeze frames
 */


/*
 * Classes of PreDebounce algorithms
 */


/*
 * Classes of event
 */


/*
 * Event parameter list
 */
const Dem_EventParameterType EventParameter[] = {
		{
				.Arc_EOL  = TRUE
		}
};


/*
 * DEM's config set
 */
const Dem_ConfigSetType DEM_ConfigSet = {
		.EventParameter = EventParameter,
//		.DTCClassType = NULL,		TODO: Add later
//		.GroupOfDtc = NULL,			TODO: Add later
//		.OemIdClass = NULL			TODO: Add later
};

/*
 * DEM's config
 */
const Dem_ConfigType DEM_Config = {
	.ConfigSet = &DEM_ConfigSet,
};
