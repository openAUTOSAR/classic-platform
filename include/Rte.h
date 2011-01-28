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



#ifndef RTE_H_
#define RTE_H_

#include "Std_Types.h"

// Errors
#define RTE_E_OK				((Std_ReturnType) 0)
#define RTE_E_INVALID			((Std_ReturnType) 1)
#define RTE_E_COMMS_ERROR		((Std_ReturnType) 128)
#define RTE_E_TIMEOUT			((Std_ReturnType) 129)
#define RTE_E_LIMIT				((Std_ReturnType) 130)
#define RTE_E_NO_DATA			((Std_ReturnType) 131)
#define RTE_E_TRANSMIT_ACK		((Std_ReturnType) 132)
// Overlayd errors
#define RTE_E_LOST_DATA			((Std_ReturnType) 64)
#define RTE_E_MAX_AGE_EXCEEDED	((Std_ReturnType) 64)


#endif /*RTE_H_*/
