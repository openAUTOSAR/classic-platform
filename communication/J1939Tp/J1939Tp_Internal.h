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

#ifndef J1939TP_INTERNAL_H_
#define J1939TP_INTERNAL_H_

#include "J1939Tp.h"

/** Service Ids */
#define J1939_TRANSMIT_ID 		0x05
#define J1939_INIT_ID 			0x01

/** Error Codes */
#define J1939TP_E_PARAM_ID		0x01
#define J1939TP_E_UNINIT		0x20
#define J1939TP_E_REINIT		0x21


#if (CANSM_DEV_ERROR_DETECT == STD_ON)
#define J1939TP_DET_REPORTERROR(serviceId, errorId)			\
	Det_ReportError(MODULE_ID_J1939TP, 0, serviceId, errorId)
#endif

#endif
