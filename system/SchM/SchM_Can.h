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

#ifndef SCHM_CAN_H_
#define SCHM_CAN_H_

#define SCHM_MAINFUNCTION_CAN_WRITE() 	SCHM_MAINFUNCTION(CAN_WRITE,Can_MainFunction_Write())
#define SCHM_MAINFUNCTION_CAN_READ() 	SCHM_MAINFUNCTION(CAN_READ,Can_MainFunction_Read())
#define SCHM_MAINFUNCTION_CAN_BUSOFF() 	SCHM_MAINFUNCTION(CAN_BUSOFF,Can_MainFunction_BusOff())
#define SCHM_MAINFUNCTION_CAN_WAKEUP() 	SCHM_MAINFUNCTION(CAN_WAKEUP,Can_MainFunction_Wakeup())
#define SCHM_MAINFUNCTION_CAN_ERROR() 	SCHM_MAINFUNCTION(CAN_ERROR,Can_MainFunction_Error())

#endif /* SCHM_CAN_H_ */
