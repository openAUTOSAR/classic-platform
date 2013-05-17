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



#ifndef IOHWAB_H_
#define IOHWAB_H_

#include "Modules.h"

#define IOHWAB_SW_MAJOR_VERSION	1
#define IOHWAB_SW_MINOR_VERSION	0
#define IOHWAB_SW_PATCH_VERSION	0

#define IOHWAB_MODULE_ID		0xAB
#define IOHWAB_VENDOR_ID		VENDOR_ID_ARCCORE

#if defined(CFG_IOHWAB_USE_SERVICE_COMPONENT)
#include "Rte_Type.h"
#endif

#include "IoHwAb_Cfg.h"
#include "IoHwAb_Types.h"
#include "IoHwAb_Analog.h"
#include "IoHwAb_Digital.h"
#include "IoHwAb_Pwm.h"


//#include "IoHwAb_Cbk.h"


#define IOHWAB_UNLOCKED		0
#define IOHWAB_LOCKED		1

/******************************************** API ids *********************************************/

#define IOHWAB_INIT_ID						0x10

#define IOHWAB_ANALOG_GET_ID				0x20

#define IOHWAB_DIGITAL_GET_ID				0x30
#define IOHWAB_DIGITAL_SET_ID				0x31

#define IOHWAB_PWMDUTY_SET_ID				0x40
#define IOHWAB_PWMFREQUENCYANDDUTY_SET_ID	0x41

#define IOHWAB_CAPTURE_GET_ID				0x50

/***************************************** DET error ids ******************************************/

#define IOHWAB_E_INIT				0x01

#define IOHWAB_E_PARAM_SIGNAL		0x11
#define IOHWAB_E_PARAM_DUTY			0x12

/******************************************* DET macros *******************************************/

#if (IOHWAB_DEV_ERROR_DETECT == STD_ON)

#define IOHWAB_DET_REPORT_ERROR(api, error)                   \
		do {                                                  \
			Det_ReportError(IOHWAB_MODULE_ID, 0, api, error); \
		} while(0)

#define IOHWAB_VALIDATE(expression, api, error)      \
		do {                                         \
			if ( !(expression) ) {                   \
				IOHWAB_DET_REPORT_ERROR(api, error); \
			}                                        \
		} while(0)

#define IOHWAB_VALIDATE_RETURN(expression, api, error, rv) \
		do {                                               \
			if ( !(expression) ) {                         \
				IOHWAB_DET_REPORT_ERROR(api, error);       \
				return rv;                                 \
			}                                              \
		} while(0)

#else  /* IOHWAB_DEV_ERROR_DETECT */

#define IOHWAB_DET_REPORT_ERROR(api, error)
#define IOHWAB_VALIDATE(expression, api, error)
#define IOHWAB_VALIDATE_RETURN(expression, api, error, rv)

#endif /* IOHWAB_DEV_ERROR_DETECT */


#define IoHwAb_LockSave(_x) 	Irq_Save(_x)
#define IoHwAb_LockRestore(_x) 	Irq_Restore(_x)

void IoHwAb_Init( void );
void IoHwAb_MainFunction( void );


#endif /* IOHWAB_H_ */
