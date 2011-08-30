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

#ifndef EA_TYPES_H_
#define EA_TYPES_H_

#include "Std_Types.h"

/*
	ITEM NAME:		<Ea_JobType>
	SCOPE:			<EA Module>
	DESCRIPTION:
		define all job in EA module 	
*/
typedef enum 
{	
	EA_JOB_NONE,
	EA_JOB_COMPARE,
	EA_JOB_ERASE,
	EA_JOB_READ,
	EA_JOB_WRITE
} Ea_JobType;

#endif
