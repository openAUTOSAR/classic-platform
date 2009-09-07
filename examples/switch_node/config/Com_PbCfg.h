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








/*
* Module vendor:  Autocore
* Module version: 1.0.0
* Specification: Autosar v3.0.1, Final
*
*/

#ifndef COM_PBCFG_H_
#define COM_PBCFG_H_

#include "Com_Types.h"

extern const Com_ConfigType ComConfiguration;

// PDU group definitions
enum {
	TxGroup = 0,
};


// Signal definitions
enum {
	SetLedLevelTx = 0,
};


#endif /* COM_PBCFG_H_ */
