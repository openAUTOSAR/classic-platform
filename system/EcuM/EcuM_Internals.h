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








// Structs and types used internal in the module

#ifndef _ECUM_INTERNALS_H_
#define _ECUM_INTERNALS_H_

#if  ( ECUM_DEV_ERROR_DETECT == STD_ON )
#if defined(USE_DET)
#include "Det.h"
#endif
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_ECUM, 0, _api, _err); \
        }

#define VALIDATE_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_ECUM, 0, _api, _err); \
          return _rv; \
        }

#define VALIDATE_NO_RV(_exp,_api,_err ) \
  if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_ECUM, 0, _api, _err); \
          return; \
        }
#define DET_REPORTERROR(_module,_instance,_api,_err) Det_ReportError(_module,_instance,_api,_err)

#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_RV(_exp,_api,_err,_rv )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_module,_instance,_api,_err)
#endif



typedef struct
{
	boolean 			initiated;
	EcuM_ConfigType *	config;
	EcuM_StateType 		shutdown_target;
	uint8 				sleep_mode;
	AppModeType 		app_mode;
	EcuM_StateType 		current_state;
#if defined(USE_COMM)
	uint32 				run_comm_requests;
#endif
	uint32 				run_requests;
	uint32 				postrun_requests;
	/* Events set by EcuM_SetWakeupEvent */
	EcuM_WakeupSourceType wakeupEvents;

	uint32 wakeupTimer;
	uint32 validationTimer;
	uint32 nvmReadAllTimer;
	/* Events set by EcuM_ValidateWakeupEvent */
	uint32 validEvents;
	boolean killAllRequest;
} EcuM_GlobalType;

extern EcuM_GlobalType EcuM_World;

void EcuM_enter_run_mode(void);

void set_current_state(EcuM_StateType state);

//#if defined(USE_LDEBUG_PRINTF)
char *GetMainStateAsString( EcuM_StateType state );
//#endif


#define DEBUG_ECUM_STATE(_state)						LDEBUG_PRINTF("STATE: %s\n",GetMainStateAsString(_state))
#define DEBUG_ECUM_CALLOUT(_call)    					LDEBUG_FPUTS( "  CALLOUT->: " _call "\n");
#define DEBUG_ECUM_CALLOUT_W_ARG(_call,_farg0,_arg0)    LDEBUG_PRINTF("  CALLOUT->: " _call " "_farg0 "\n",_arg0)
#define DEBUG_ECUM_CALLIN_W_ARG(_call,_farg0,_arg0)     LDEBUG_PRINTF("  <-CALLIN : " _call " "_farg0 "\n",_arg0)


#endif /*_ECUM_INTERNALS_H_*/
