/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/


/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0 */
/** @fileSafetyClassification ASIL **/ /* Common for whole module  */

// Structs and types used internal in the module

#ifndef _ECUM_INTERNALS_H_
#define _ECUM_INTERNALS_H_

#include "EcuM_Generated_Types.h"
/* @req SWS_BSW_00203 API parameter checking enablement. */
#if defined(USE_DET)
/* @req SWS_BSW_00042 Detection of DevErrors should only be performed if configuration parameter for Development errors is set. */
#if  ( ECUM_DEV_ERROR_DETECT == STD_ON )
#include "Det.h"

/*lint -emacro(904,VALIDATE,VALIDATE_RV,VALIDATE_NO_RV) MISRA:ARGUMENT_CHECK:Macros used for checking arguments before performing any functionality:[MISRA 2012 Rule 15.5, advisory] */
/*lint -emacro(9027,VALIDATE,VALIDATE_RV,VALIDATE_NO_RV) MISRA:ARGUMENT_CHECK:usage of '!' in macros allowed:[MISRA 2012 Rule 10.1, required] */
/* @req SWS_BSW_00045 Development errors should be reported to DET module */
/* @req SWS_BSW_00049 API parameter checking */
#define VALIDATE(_exp,_api,_err ) \
        if(!(_exp) ) { \
          (void)Det_ReportError(ECUM_MODULE_ID, 0, _api, _err); \
        }

#define VALIDATE_RV(_exp,_api,_err,_rv ) \
        if(!(_exp)) { \
          (void)Det_ReportError(ECUM_MODULE_ID, 0, _api, _err); \
          return _rv; \
        }

#define VALIDATE_NO_RV(_exp,_api,_err) \
        if(!(_exp)){ \
          (void)Det_ReportError(ECUM_MODULE_ID, 0, _api, _err); \
          return; \
        }

// MISRA 2012 14.4 : Inhibit lint error 9036 for macro ended with while(0) condition
//lint -emacro(9036,ECUM_DET_REPORT_ERROR)
#define ECUM_DET_REPORT_ERROR(_api, _err)                   \
        (void)Det_ReportError(ECUM_MODULE_ID, 0, _api, _err); \


#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_RV(_exp,_api,_err,_rv )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define ECUM_DET_REPORT_ERROR(_api, _error)
#endif
#endif

#define SchM_Enter_EcuM(_area) SchM_Enter_EcuM_##_area)
#define SchM_Exit_EcuM(_area) SchM_Exit_EcuM_##_area)


typedef struct
{
    boolean                           initiated;
    EcuM_ConfigType                   *config;
    EcuM_StateType                    shutdown_target;
    AppModeType                       app_mode;
    EcuM_StateType                    current_state;
    uint32                            run_requests;
    uint32                            postrun_requests;
    boolean                           killAllRequest;
    boolean                           killAllPostrunRequest;
#if (defined(USE_ECUM_FLEXIBLE))
    EcuM_ShutdownCauseType            shutdown_cause;
#endif
    uint8                             sleep_mode;
    /* Events set by EcuM_SetWakeupEvent */
    uint32                            wakeupEvents;
    uint32                            wakeupTimer;
    /* Events set by EcuM_ValidateWakeupEvent */
    uint32                            validEvents;
    uint32                            validationTimer;
#if (ECUM_ARC_SAFETY_PLATFORM == STD_ON)
    EcuM_SP_SyncStatus                syncStatusMaster;
#endif
} EcuM_GlobalType;

typedef struct
{
    uint32                            nvmReadAllTimer;
#if defined(USE_COMM) || (defined(USE_ECUM_COMM) && (ECUM_AR_VERSION < 40000))
    uint32                            run_comm_requests;
#endif
#if (ECUM_ARC_SAFETY_PLATFORM == STD_ON)
    EcuM_SP_SyncStatus                syncStatusPartition_QM;
    EcuM_StateType                    current_state_partition_QM;
#endif
} EcuM_GlobalType_Partition_COMMNVM;

extern EcuM_GlobalType EcuM_World;
extern EcuM_GlobalType_Partition_COMMNVM EcuM_World_ComM_NVM; /*lint -save -e9003 MISRA:OTHER:Blockscope variables definitions warning is neglected:[MISRA 2012 Rule 8.9, advisory] */

typedef enum
{
    ALL,
    ALL_WO_LIN,
    ONLY_LIN
} EcuM_ComMCommunicationGroupsType;


void EcuM_enter_run_mode(void);

void SetCurrentState(EcuM_StateType state);

// MISRA 2004 6.3, 2012 4.6 : Inhibit lint error 970 for function GetMainStateAsString
// to allow usage of type char, as this function uses string constants.
char *GetMainStateAsString( EcuM_StateType state ); //lint !e970

// Functions for EcuMFixed and Safety Platform
void SetComMCommunicationAllowed(EcuM_ComMCommunicationGroupsType group, boolean Allowed);
#if (ECUM_ARC_SAFETY_PLATFORM == STD_ON)
void EcuM_StartupTwo_Partition_A0(void);
void EcuM_StartupTwo_Partition_QM(void);
#endif

#define DEBUG_ECUM_STATE(_state)						LDEBUG_PRINTF("STATE: %s\n",GetMainStateAsString(_state))
#define DEBUG_ECUM_CALLOUT(_call)    					LDEBUG_FPUTS( "  CALLOUT->: " _call "\n");
#define DEBUG_ECUM_CALLOUT_W_ARG(_call,_farg0,_arg0)    LDEBUG_PRINTF("  CALLOUT->: " _call " "_farg0 "\n",_arg0)
#define DEBUG_ECUM_CALLIN_W_ARG(_call,_farg0,_arg0)     LDEBUG_PRINTF("  <-CALLIN : " _call " "_farg0 "\n",_arg0)


#endif /*_ECUM_INTERNALS_H_*/
