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
