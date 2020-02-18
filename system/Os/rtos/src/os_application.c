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

/* ----------------------------[includes]------------------------------------*/

#include "os_i.h"
/* ----------------------------[private define]------------------------------*/
#define BACKCHAIN_SIZE	(16uL)
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
#if OS_APPLICATION_CNT!=0
OsAppVarType Os_AppVar[OS_APPLICATION_CNT];
#endif

/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/
/* ----------------------------[General requirements handled in this functions]----------------------------*/
/* @req SWS_Os_00547
 *   Availability of AllowAccess(): Available in Scalability Classes 3 and 4.
 * OS536
 *    Availability of TerminateApplication(): Available in Scalability Classes 3 and 4.
 * OS520
 *    Availability of CheckObjectOwnership():Available in Scalability Classes 3 and 4.
 * @req SWS_Os_00514
 *    Availability of GetApplicationID():Available in Scalability Classes 3 and 4.
 * @req SWS_Os_00519
 *    Availability of CheckObjectAccess():Available in Scalability Classes 3 and 4.
 * @req SWS_Os_00537
 *    Availability of GetApplicationState(): Available in Scalability Classes 3 and 4.
 *
 */

#if (OS_APPLICATION_CNT > 1) && ((OS_SC1 == STD_ON )||(OS_SC2 == STD_ON )) && (OS_NUM_CORES==1)
#error Cant configure more than one application on SC1 and SC1 if on single-core
#endif

#if	(OS_USE_APPLICATIONS == STD_ON)
#if ((OS_SC3 == STD_ON) || (OS_SC4 == STD_ON))
/**
 * This service determines the currently running OS-Application (a unique
 * identifier has to be allocated to each application).
 *
 * @return <identifier of running OS-Application> or INVALID_OSAPPLICATION
 */
/** @req SWS_Os_00016  OS provides the service GetApplicationID() */
ApplicationType GetApplicationID( void ) {
    /** @req SWS_Os_00261 *//** !req SWS_Os_00262 */
    return OS_SYS_PTR->currApplId;
}

/**
 * This service checks if a memory region is write/read/execute accessible
 * and also returns information if the memory region is part of the stack
 * space.
 *
 * @param ISRID		ISR reference
 * @param Address   Start of memory area
 * @param Size      Size of memory area
 * @return
 */
AccessType CheckISRMemoryAccess( ISRType isrId,
        MemoryStartAddressType address,
        MemorySizeType size )
{
    /*lint -e{923} MISRA:OTHER:Allow pointer to ptrdiff_t cast:[MISRA 2012 Rule 11.4, advisory]*/

    ptrdiff_t addr = (ptrdiff_t)address;
    (void)addr;
    (void)size;

    if( isrId > (ISRType)OS_ISR_CNT ) {
        /*lint -e{904} MISRA:OTHER:Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check:[MISRA 2012 Rule 15.5, advisory]*/
        return 0;
    }

    /** !req SWS_Os_00449 */
    return 0;
}
/**
 * This service checks if a memory region is write/read/execute accessible
 * and also returns information if the memory region is part of the stack
 * space.
 *
 * Check returned accesstype with:
 *   OSMEMORY_IS_READABLE(<AccessType>)
 *   OSMEMORY_IS_WRITEABLE(<AccessType>)
 *   OSMEMORY_IS_EXECUTABLE(<AccessType>)
 *   OSMEMORY_IS_STACKSPACE(<AccessType>)
 *
 * NOTE: Not really sure what this function is actually good for? Add a use-case!
 *
 * @param TaskID   Task reference
 * @param Address  Start of memory area
 * @param Size     Size of memory area
 * @return
 */
AccessType CheckTaskMemoryAccess( TaskType taskId,
        MemoryStartAddressType address,
        MemorySizeType size )
{
    /*lint -e{923} MISRA:STANDARDIZED_INTERFACE:Allow pointer to ptrdiff_t cast:[MISRA 2012 Rule 11.6, required]*/
    ptrdiff_t addr = (ptrdiff_t)address;
    (void)addr;
    (void)size;

    if( taskId > OS_TASK_CNT ) {
        /*lint -e{904} MISRA:OTHER:Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check:[MISRA 2012 Rule 15.5, advisory]*/
        return 0;
    }

    /* IMPROVEMENT: Add body */
    return 0;
}

/**
 * This service determines if the OS-Applications, given by ApplID,
 * is allowed to use the IDs of a Task, ISR, Resource, Counter,
 * Alarm or Schedule Table in API calls.
 *
 * @param ApplID      OS-Application identifier
 * @param ObjectType  Type of the following parameter
 * @param object      The object to be examined
 * @return ACCESS if the ApplID has access to the object
 * NO_ACCESS otherwise
 */
/** @req SWS_Os_00256 OS provides the service CheckObjectAccess() */
/** @req SWS_Os_00450 CheckObjectAccess checks access rights for Operating System objects. */
ObjectAccessType CheckObjectAccess( ApplicationType ApplId,
        ObjectTypeType ObjectType,
        uint32 objectId )
{
    uint32 appMask = APPL_ID_TO_MASK(ApplId);
    ObjectAccessType rv = NO_ACCESS;
    uint32 rvMask = 0U;

    /* @req SWS_Os_00423
     * If in a call of CheckObjectAccess() the object to  be examined
     * is not avalid object OR <ApplID> is invalid OR <ObjectType> is
     * invalid THEN CheckObjectAccess() shall return NO_ACCESS.
     */
    /* @req SWS_Os_00272
     * If the OS-Application <ApplID> in a call of CheckObjectAccess() has no
     * access to the queried object, CheckObjectAccess() shall return NO_ACCESS.
     */
    if (( ApplId < (ApplicationType)OS_APPLICATION_CNT ))
    {

		/*lint -e{685} MISRA:CONFIGURATION:Allow MISRA violations depending on configuration:[MISRA 2012 Rule 14.3, required]*/
        /*lint -e{568} MISRA:CONFIGURATION:Allow MISRA violations depending on configuration:[MISRA 2004 Info, advisory]*/
		switch( ObjectType ) {
			case OBJECT_ALARM:
#if (OS_ALARM_CNT != 0)
				if (objectId < OS_ALARM_CNT) {
					const OsAlarmType *lPtr = Os_AlarmGet((AlarmType)objectId);
                    rvMask = lPtr->accessingApplMask & (appMask);
				}
#endif
				break;
			case OBJECT_COUNTER:
#if (OS_COUNTER_CNT != 0)
				if (objectId < OS_COUNTER_CNT) {
					const OsCounterType *lPtr = Os_CounterGet((CounterType)objectId);
                    rvMask = lPtr->accessingApplMask & (appMask);
				}
#endif
			break;
			case OBJECT_ISR:
			{
				/* An ISR do not have accessingApplicationMask, just check if owner */
				const OsIsrVarType *lPtr = Os_IsrGet((ISRType)objectId);
                rvMask = (lPtr->constPtr->appOwner == ApplId);
			}
			break;
			case OBJECT_RESOURCE:
#if (OS_RESOURCE_CNT != 0)
				if (objectId < OS_RESOURCE_CNT) {
					const OsResourceType *lPtr = Os_ResourceGet((ResourceType)objectId);
                    rvMask = lPtr->accessingApplMask & (appMask);
				}
#endif
			break;
			case OBJECT_SCHEDULETABLE:
#if (OS_SCHTBL_CNT != 0)
				if (objectId < OS_SCHTBL_CNT) {
					const OsSchTblType *lPtr = Os_SchTblGet((ScheduleTableType)objectId);
                    rvMask = lPtr->accessingApplMask & (appMask);
				}
#endif
			break;
			case OBJECT_TASK:
				if (objectId < OS_TASK_CNT) {
					const OsTaskVarType *lPtr = Os_TaskGet((TaskType)objectId);
					rvMask = lPtr->constPtr->accessingApplMask & (appMask);
				}
			break;
			default:
			/* @req SWS_Os_00423 */
			break;
		}
    }
    else
    {
		/** @req SWS_Os_00423 *//** @req SWS_Os_00272 */
    }
    /** @req SWS_Os_00271 *//** @req SWS_Os_00272 */
    if (rvMask != 0U) {
    	rv = ACCESS;
    }

    /* @req SWS_Os_00367
     * Operating System module’s services which do not return a
     * StatusType shall not raise the error hook(s).
     */

    return rv;
}

#if !(defined(CFG_SAFETY_PLATFORM) || defined(BUILD_OS_SAFETY_PLATFORM))
/**
 * This service sets the own state of an OS-Application from
 * APPLICATION_RESTARTING to APPLICATION_ACCESSIBLE.
 *
 * @return  E_OK : No errors
 * 			E_OS_STATE : The OS-Application of the caller is in the wrong
 state
 */
/** @req SWS_Os_00501 OS provides the service AllowAccess() */
StatusType AllowAccess( void ) {
    StatusType rv = E_OK;
    ApplicationType applId = OS_SYS_PTR->currApplId;

    /* @req SWS_Os_00497
     * If the state of the OS-Application of the caller of AllowAccess() is not APPLICATION_RESTARTING
     * AllowAccess() shall return E_OS_STATE. */

    /* Validation of parameters, if it fails, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD( !(Os_ApplGetState(applId) != APPLICATION_RESTARTING), E_OS_STATE, OSServiceId_AllowAccess);

    /* @req SWS_Os_00498
     * If the state of the OS-Application of the caller of AllowAccess() is APPLICATION_RESTARTING,
     * AllowAccess() shall set the state to APPLICATION_ACCESSIBLE and
     * allow other OS-Applications to access the configured objects of the callers OS-Application. */
    Os_ApplSetState(applId, APPLICATION_ACCESSIBLE);

    return rv;
}

#endif //CFG_SAFETY_PLATFORM
#endif // #if ((OS_SC3 == STD_ON) || (OS_SC4 == STD_ON))

/**
 * This service determines to which OS-Application a given Task, ISR, Resource,
 * Counter, Alarm or Schedule Table belongs
 *
 * @param ObjectType Type of the following parameter
 * @param object     The object to be examined
 * @return The OS-Application to which the object ObjectType belongs or
 * INVALID_OSAPPLICATION if the object does not exists
 */
/** @req SWS_Os_00017 OS provides the service CheckObjectOwnership() */
ApplicationType CheckObjectOwnership( ObjectTypeType ObjectType,
        uint32 objectId )
{
    ApplicationType rv = (uint32)INVALID_OSAPPLICATION;

    switch( ObjectType ) {
        case OBJECT_ALARM:
        rv = Os_AlarmGetApplicationOwner((AlarmType)objectId);
        break;
        case OBJECT_COUNTER:
        rv = Os_CounterGetApplicationOwner((CounterType)objectId);
        break;
        case OBJECT_ISR:
        rv = Os_IsrGetApplicationOwner((ISRType)objectId);
        break;
        case OBJECT_RESOURCE:
        rv = Os_ResourceGetApplicationOwner((ResourceType)objectId);
        break;
        case OBJECT_SCHEDULETABLE:
        rv = Os_SchTblGetApplicationOwner((ScheduleTableType)objectId);
        break;
        case OBJECT_TASK:
        rv = Os_TaskGetApplicationOwner((TaskType)objectId);
        break;
        default:
        /** @req SWS_Os_00274 */
        rv = (uint32)INVALID_OSAPPLICATION;
        break;
    }

    /** @req SWS_Os_00273 */ /** @req SWS_Os_00367 */
    return rv;
}

#if !(defined(CFG_SAFETY_PLATFORM) || defined(BUILD_OS_SAFETY_PLATFORM))
/**
 * IMPROVEMENT: Move somewhere else
 * @param mode
 * @return
 */
/* @req OSEK_SWS_SS_00001 */
StatusType GetActiveApplicationMode( AppModeType* mode) {
    *mode = OS_SYS_PTR->appMode;
    return E_OK;
}
#endif //CFG_SAFETY_PLATFORM


/**
 * During Os Initialization this API will be called.
 * 		Interrupt stack is set and applications are set to accessible.
 */
void Os_ApplStart( void ) {
    uint16_t i;

    /* Call startuphooks for all applications */
    for(i=0;i<OS_APPLICATION_CNT;i++) {
        /** @req SWS_Os_00500 */
        Os_AppVar[i].state = APPLICATION_ACCESSIBLE;

        Os_AppVar[i].trusted = Os_AppConst[i].trusted;
        Os_AppVar[i].appId = Os_AppConst[i].appId;

#if ((OS_SC3 == STD_ON) || (OS_SC4 == STD_ON))

/* OsApplication startup hook is not allowed in safety platform*/
#if !(defined(CFG_SAFETY_PLATFORM) || defined(BUILD_OS_SAFETY_PLATFORM))
        if( Os_AppConst[i].StartupHook != NULL ) {
            /** @req SWS_Os_00060 */
            Os_AppConst[i].StartupHook();
        }
#endif


        /* Trusted application have no interrupt stack */
        if( Os_AppVar[i].trusted == FALSE ) {
            Os_AppVar[i].intStack = (uint8*)Os_AppConst[i].intStack.bottom - BACKCHAIN_SIZE;
            Os_AppSetIsrEndmark(i);
            Os_AppSetIsrStartmark(i);
        }
#endif
    }
}

#if ((OS_SC3 == STD_ON) || (OS_SC4 == STD_ON))

/**
 * @brief   Perform stack check on ISR that belongs to an application
 *
 * @param id  The ID of the application
 */
void Os_AppIsrStackPerformCheck( ApplicationType id ) {
#if (OS_STACK_MONITORING == 1)
    /*lint --e{9036} -e{9007} MISRA:OTHER:Os_AppIsIsrEndmarkOk and Os_AppIsIsrStartmarkOk return boolean value:[MISRA 2012 Rule 14.4, required] [MISRA 2012 Rule 13.5, required]*/
    if( (Os_AppIsIsrEndmarkOk(id) == FALSE) || (Os_AppIsIsrStartmarkOk(id) == FALSE)  ) {
        /** @req SWS_Os_00396
         * If a stack fault is detected by stack monitoring AND the configured scalability
         * class is 3 or 4, the Operating System module shall call the ProtectionHook() with
         * the status E_OS_STACKFAULT.
         * */

        Os_CallProtectionHook(E_OS_STACKFAULT);
    }
#else
    (void)id;
#endif  /* (OS_STACK_MONITORING == 1) */
}


#if !(defined(CFG_SAFETY_PLATFORM) || defined(BUILD_OS_SAFETY_PLATFORM))
/**
 * @brief   Function to get stack information
 * @param id       Application Id
 * @param stack    Pointer to stack information to fill.
 */
void Os_AppIsrGetStackInfo( ApplicationType id, OsAppStackType *stack ) {
    stack->top = Os_AppConst[id].intStack.top;
    stack->size = Os_AppConst[id].intStack.size;
}

#endif

#endif

#endif // #if	(OS_USE_APPLICATIONS == STD_ON)
