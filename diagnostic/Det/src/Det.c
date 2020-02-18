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
/* @req SWS_BSW_00005 include implementation file */
#include "Det.h"
#if (DET_SAFETYMONITOR_API == STD_ON)
#include <string.h>
#include "Safety_Queue.h"
#endif

#define DEBUG_LVL 1
#include "debug.h"

#include "SchM_Det.h"

#if (DET_FORWARD_TO_DLT == STD_ON)
#include "Dlt.h"
#endif

/* ----------------------------[Version check]------------------------------*/
#if !(((DET_SW_MAJOR_VERSION == 2) && (DET_SW_MINOR_VERSION == 0)) )
#error Det: Expected BSW module version to be 2.0.*
#endif

#if !(((DET_AR_MAJOR_VERSION == 4) && (DET_AR_MINOR_VERSION == 3)) )
#error Det: Expected AUTOSAR version to be 4.3.*
#endif

/* ----------------------------[private define]------------------------------*/
typedef enum
{
    DET_UNINITIALIZED = 0,
    DET_INITIALIZED,
    DET_STARTED
} Det_StateType;

#define DET_STATE_START_SEC_VAR_CLEARED_UNSPECIFIED
/*lint -e9019 MISRA:EXTERNAL_FILE:suppressed due to Det_BswMemMap.h include is needed:[MISRA 2012 Rule 20.1, advisory] */
/*lint -e451 MISRA:CONFIGURATION:suppressed due to Det_BswMemMap.h include is needed:[MISRA 2012 Directive 4.10, required] */
#include "Det_BswMemMap.h"
static Det_StateType detState = DET_UNINITIALIZED;
#define DET_STATE_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Det_BswMemMap.h"

/* @req SWS_BSW_00006 include BSW Memory mapping header*/
#if (DET_SAFETYMONITOR_API == STD_ON)
#define DET_SIZEOF_SMQUEUE (32)
#define DET_MONITOR_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Det_BswMemMap.h"
/*lint -e9003 MISRA:OTHER:To store error values:[MISRA 2012 Rule 8.9, advisory]*/
static Det_EntryType Det_SafetyMonitorBuffer[DET_SIZEOF_SMQUEUE];
#define DET_MONITOR_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Det_BswMemMap.h"
#define DET_MONITOR_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Det_BswMemMap.h"
static Safety_Queue_t Det_SafetyMonitorQueue;
#define DET_MONITOR_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Det_BswMemMap.h"

// Just a dummy to make the queue happy.
typedef int local_int;
static local_int DummyFunc(void *A, void *B, size_t size)
{
    (void)A; /*lint !e920 MISRA:STANDARDIZED_INTERFACE:to store dummy values:[MISRA 2012 Rule 1.3, required]*/
	(void)B;/*lint !e920 MISRA:STANDARDIZED_INTERFACE:to store dummy values:[MISRA 2012 Rule 1.3, required]*/
	(void)size;
	return 0;
}
#endif

#if ( DET_USE_RAMLOG == STD_ON )
// Ram log variables in uninitialized memory
SECTION_RAMLOG uint32 Det_RamlogIndex;
SECTION_RAMLOG Det_EntryType Det_RamLog[DET_RAMLOG_SIZE] ;
#endif

#if ( DET_USE_STATIC_CALLBACKS == STD_ON )
/*lint -e{9003} MISRA:EXTERNAL_FILE:variable declared outside:[MISRA 2012 Rule 8.9, advisory]*/
extern detCbk_t DetStaticHooks[];
#endif

#if ( DET_ENABLE_CALLBACKS == STD_ON )
#define DET_STATE_START_SEC_VAR_CLEARED_UNSPECIFIED

#include "Det_BswMemMap.h"
static detCbk_t detCbk_List[DET_NUMBER_OF_CALLBACKS];
#define DET_STATE_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Det_BswMemMap.h"


uint8 Det_AddCbk(detCbk_t detCbk)
{
    uint8 rv = DET_CBK_REGISTRATION_FAILED_INDEX;	 // Return DET_CBK_REGISTRATION_FAILED_INDEX if the registration fails

    if (detState != DET_UNINITIALIZED)
    {
        for (uint8 i = 0; i < DET_NUMBER_OF_CALLBACKS; i++)
        {
            if (NULL == detCbk_List[i])
            {
                detCbk_List[i] = detCbk;
                rv = i;
                break;
            }
        }
    }

    if (rv == DET_CBK_REGISTRATION_FAILED_INDEX)
    {
        /* Ignoring return value */
        (void)Det_ReportError(DET_MODULE_ID, 0, DET_CALLBACK_API, DET_E_CBK_REGISTRATION_FAILED);
    }

    return rv;
}


void Det_RemoveCbk(uint8 detCbkIndex)
{
    /* @req SWS_BSW_00049 Parameter checking*/
    // Validate the index
    if (detCbkIndex >= DET_NUMBER_OF_CALLBACKS)
    {
        /* Ignoring return value */
        (void)Det_ReportError(DET_MODULE_ID, 0, DET_CALLBACK_API, DET_E_INDEX_OUT_OF_RANGE);
    }
    else
    {
        detCbk_List[detCbkIndex]=NULL;
    }
}
#endif

/* @req SWS_Det_00020
 * Each call of the Det_Init function shall be used to set the Default Error Tracer to a defined initial status
 */
/* @req SWS_BSW_00071 initialize BSW module*/
void Det_Init(void /*const Det_ConfigType* ConfigPtr*/)
{

#if ( DET_ENABLE_CALLBACKS == STD_ON )
    for (uint32 i=0; i<DET_NUMBER_OF_CALLBACKS; i++)
    {
        detCbk_List[i]=NULL;
    }
#endif

#if ( DET_USE_RAMLOG == STD_ON )
    for(uint32 i=0; i < DET_RAMLOG_SIZE; i++)
    {
        Det_RamLog[i].moduleId = 0;
        Det_RamLog[i].instanceId = 0;
        Det_RamLog[i].apiId = 0;
        Det_RamLog[i].errorId = 0;
    }
    Det_RamlogIndex = 0;
#endif

#if (DET_SAFETYMONITOR_API == STD_ON)
    for(uint32 i=0; i < DET_SIZEOF_SMQUEUE; i++)
    {
    	Det_SafetyMonitorBuffer[i].moduleId = 0;
    	Det_SafetyMonitorBuffer[i].instanceId = 0;
    	Det_SafetyMonitorBuffer[i].apiId = 0;
    	Det_SafetyMonitorBuffer[i].errorId = 0;
    }
    memset(&Det_SafetyMonitorQueue, 0 , sizeof(Safety_Queue_t));

    if (E_OK == Safety_Queue_Init(&Det_SafetyMonitorQueue, Det_SafetyMonitorBuffer, DET_SIZEOF_SMQUEUE, sizeof(Det_EntryType), DummyFunc)) {
    	detState = DET_INITIALIZED;
    }

#else
    detState = DET_INITIALIZED;
#endif
}

/* @req SWS_BSW_00072 deinitialize BSW module */
#if DET_DEINIT_API == STD_ON
void Det_DeInit( void )
{
    detState = DET_UNINITIALIZED;
}
#endif

/* @req SWS_Det_00039 */
Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId)
{
	Std_ReturnType rVal = E_OK;

#if (DET_SAFETYMONITOR_API == STD_ON)
	Det_EntryType error;
#endif

    /* @req SWS_Det_00024 */
    if (detState == DET_STARTED) // No action is taken if the module is not started
    {
        /* @req SWS_Det_00207
         * @req SWS_Det_00015
         * @req SWS_Det_00014
         * @req SWS_Det_00501
         */
        // Call static error hooks here
#if ( DET_USE_STATIC_CALLBACKS == STD_ON )
        SchM_Enter_Det_EA_0();

        /*
         * @req SWS_Det_00035
         * @req SWS_Det_00181
         * @req SWS_Det_00018
         */
        for (uint32 i=0; i<DET_NUMBER_OF_STATIC_CALLBACKS; i++)
        {
            (*DetStaticHooks[i])(ModuleId, InstanceId, ApiId, ErrorId);
        }
        SchM_Exit_Det_EA_0();
#endif

        /* @req SWS_Det_00034 */
#if (DET_FORWARD_TO_DLT == STD_ON)
        Dlt_DetForwardErrorTrace(ModuleId, InstanceId, ApiId, ErrorId);
#endif

#if ( DET_ENABLE_CALLBACKS == STD_ON )
        /*lint -e534 MISRA:CONFIGURATION:ignoring return value:[MISRA 2012 Rule 17.7, required] */
        SchM_Enter_Det_EA_0();
        for (uint32 i=0; i<DET_NUMBER_OF_CALLBACKS; i++)
        {
            if (NULL!=detCbk_List[i])
            {
                (*detCbk_List[i])(ModuleId, InstanceId, ApiId, ErrorId);
            }
        }
        SchM_Exit_Det_EA_0();
#endif


#if ( DET_USE_RAMLOG == STD_ON )
        /*lint -e534 MISRA:CONFIGURATION:ignoring return value:[MISRA 2012 Rule 17.7, required] */
        SchM_Enter_Det_EA_0();
        if (Det_RamlogIndex < DET_RAMLOG_SIZE)
        {
            Det_RamLog[Det_RamlogIndex].moduleId = ModuleId;
            Det_RamLog[Det_RamlogIndex].instanceId = InstanceId;
            Det_RamLog[Det_RamlogIndex].apiId = ApiId;
            Det_RamLog[Det_RamlogIndex].errorId = ErrorId;
            Det_RamlogIndex++;
#if ( DET_WRAP_RAMLOG == STD_ON )
            if (Det_RamlogIndex == DET_RAMLOG_SIZE){
            	Det_RamlogIndex = 0;
            }
#endif
        }
        SchM_Exit_Det_EA_0();
#endif

#if (DET_SAFETYMONITOR_API == STD_ON)
        error.moduleId = ModuleId;
        error.instanceId = InstanceId;
        error.apiId = ApiId;
        error.errorId = ErrorId;
        rVal = Safety_Queue_Add(&Det_SafetyMonitorQueue, &error);
        if (rVal == E_OK) {
        	/* @req ARC_SWS_DET_00002 */
            rVal = SYS_CALL_SetEvent(DET_SAFETYMONITOR_TASK, DET_SAFETYMONITOR_EVENT);
        }
#endif

#if ( DET_USE_STDERR == STD_ON )
        printf("Det Error: ModuleId=%d, InstanceId=%d, ApiId=%d, ErrorId=%d\n", ModuleId, InstanceId, ApiId, ErrorId); /*lint !e586, OK deprecated */
#endif
    }
    return rVal;
}

void Det_Start(void)
{
    detState = DET_STARTED;
}

/* @req ARC_SWS_DET_00001*/
#if (DET_SAFETYMONITOR_API == STD_ON)
Det_ReturnType Det_GetNextError(Det_EntryType* entry)
{
	Std_ReturnType err;
	Det_ReturnType rVal;
	/* @req SWS_BSW_00049 Parameter checking*/
	/* @req SWS_BSW_00212 NULL pointer check */
	if (entry != NULL_PTR)
	{
	    SchM_Enter_Det_EA_0();
	    err = Safety_Queue_Next(&Det_SafetyMonitorQueue, entry);
	    SchM_Exit_Det_EA_0();

	    switch (err) {
	    case QUEUE_E_OK:
	    	rVal = DET_OK;
	    	break;
	    case QUEUE_E_NO_DATA:
	    	rVal = DET_EMPTY;
	    	break;
	    default:
	    	rVal = DET_ERROR;
	    	break;
	    }
	}
	else
	{
		rVal = DET_ERROR;
	}

    return rVal;
}
#endif

/* @req SWS_BSW_00064 GetVersionInfo shall execute synchonously */
/* @req SWS_BSW_00052 GetVersion info shall only have one parameter */
/* @req SWS_BSW_00164 No restriction on calling context */
#if (DET_VERSIONINFO_API == STD_ON)
void Det_GetVersionInfo(Std_VersionInfoType* vi)
{
    /* @req SWS_BSW_00049 Parameter checking*/
    /* @req SWS_BSW_00212 NULL pointer check */
    if(vi != NULL) {
        vi->vendorID = DET_VENDOR_ID;
        vi->moduleID = DET_MODULE_ID;
        vi->sw_major_version = DET_SW_MAJOR_VERSION;
        vi->sw_minor_version = DET_SW_MINOR_VERSION;
        vi->sw_patch_version = DET_SW_PATCH_VERSION;
    } else {
        /* @req SWS_Det_00301 */
        /* @req SWS_Det_00052 */
        (void)Det_ReportError(DET_MODULE_ID, 0, DET_GETVERSIONINFO_SERVICE_ID, DET_E_PARAM_POINTER);
    }
}
#endif
