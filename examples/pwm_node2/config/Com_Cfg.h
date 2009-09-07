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


#ifndef COM_CFG_H_
#define COM_CFG_H_

#define COM_MODULE_ID 20
#define COM_INSTANCE_ID 1

#define COM_DEV_ERROR_DETECT

#define COM_MAX_NR_IPDU 5
#define COM_MAX_NR_SIGNAL 6
#define COM_MAX_NR_GROUPSIGNAL 10

#define COM_MAX_NR_SIGNALS_PER_IPDU 4
#define COM_MAX_NR_SIGNALGROUPS_PER_IPDU 4
#define COM_MAX_NR_SIGNALS_PER_SIGNAL_GROUP 5

#define COM_E_INVALID_FILTER_CONFIGURATION 101
#define COM_E_INITIALIZATION_FAILED 102
#define COM_E_INVALID_SIGNAL_CONFIGURATION 103
#define COM_INVALID_PDU_ID 104
#define COM_ERROR_SIGNAL_IS_SIGNALGROUP 105

#define COM_E_TOO_MANY_IPDU 106
#define COM_E_TOO_MANY_SIGNAL 107
#define COM_E_TOO_MANY_GROUPSIGNAL 108

#define CPU_ENDIANESS BIG_ENDIAN

/*
 * ComGeneral pre-compile time configuration parameters.
 */
#define ComConfigurationTimeBase NULL
#define ComConfigurationUseDet 
#define ComVersionInfoApi

#endif /*COM_CFG_H_*/
