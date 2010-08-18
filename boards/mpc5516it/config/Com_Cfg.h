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


/** @addtogroup Com COM module
 *  @{ */

/** @file Com_Cfg.h
 * Definitions of configuration parameters for the COM module.
 */


#warning "This default file may only be used as an example!"

#ifndef COM_CFG_H_
#define COM_CFG_H_

#define COM_MODULE_ID 20
#define COM_INSTANCE_ID 1

#define COM_DEV_ERROR_DETECT

/** Max number of I-PDUs allowed in the configuration. */
#define COM_MAX_NR_IPDU 30
/** Max number of signals allowed in the configuration. */
#define COM_MAX_NR_SIGNAL 30
/** Max number of group signals allowed in the configuration. */
#define COM_MAX_NR_GROUPSIGNAL 30

/** Max number of signals allowed per I-PDU. */
#define COM_MAX_NR_SIGNALS_PER_IPDU 10
/** Max number of signal groups allowed per I-PDU. */
#define COM_MAX_NR_SIGNALGROUPS_PER_IPDU 10
/** Max number of group signals allowd per signal group. */
#define COM_MAX_NR_SIGNALS_PER_SIGNAL_GROUP 10


#define COM_E_INVALID_FILTER_CONFIGURATION 101
#define COM_E_INITIALIZATION_FAILED 102
#define COM_E_INVALID_SIGNAL_CONFIGURATION 103
#define COM_INVALID_PDU_ID 104
#define COM_ERROR_SIGNAL_IS_SIGNALGROUP 105

#define COM_E_TOO_MANY_IPDU 106
#define COM_E_TOO_MANY_SIGNAL 107
#define COM_E_TOO_MANY_GROUPSIGNAL 108

#define CPU_ENDIANESS BIG_ENDIAN


#define ComConfigurationTimeBase
#define ComConfigurationUseDet
#define ComVersionInfoApi


#endif /*COM_CFG_H_*/
/** @} */
