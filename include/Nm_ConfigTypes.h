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


#ifndef NM_CONFIGTYPES_H_
#define NM_CONFIGTYPES_H_

/** Data structure shall be pre-compile time configurable. */
typedef struct {
#if (NM_BUSNM_CANNM_ENABLED == STD_ON)
	const CanNm_ConfigType* CanNmConfigPtr
#endif
#if (NM_BUSNM_FRNM_ENABLED == STD_ON)
	const FrNm_ConfigType * FrNmConfigPtr
#endif
}Nm_BusNm_ConfigType;



#endif /* NM_CONFIGTYPES_H_ */
