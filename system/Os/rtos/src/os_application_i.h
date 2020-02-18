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

#ifndef APPLICATION_H_
#define APPLICATION_H_

/*
 * IMPLEMENTATION NOTES:
 *
 *  OS448:  The  Operating  System  module  shall  prevent  access  of
 *  OS-Applications, trusted or non-trusted, to objects not belonging
 *  to this OS-Application, except access rights for such objects are
 *  explicitly granted by configuration.
 *
 *  OS509:  If  a  service  call  is  made  on  an  Operating  System
 *  object  that  is  owned  by another OS-Application without state
 *  APPLICATION_ACCESSIBLE, then the Operating System module shall return E_OS_ACCESS.
 *
 *  OS056: If an OS-object identifier is the parameter of an Operating System module�s
 *  system service, and no sufficient access rights have been assigned to this OS-object
 *  at  configuration  time  (Parameter  Os[...]AccessingApplication)  to  the  calling
 *  Task/Category  2  ISR,  the  Operating  System  module�s  system  service  shall  return
 *  E_OS_ACCESS.
 *
 *  OS311: If OsScalabilityClass is SC3 or SC4 AND a Task OR Category 2 ISR OR
 *  Resources OR Counters OR Alarms OR Schedule tables does not belong to exactly
 *  one OS-Application the consistency check shall issue an error.
 *
 *  Page 52:
 *  It is assumed that the Operating System module itself is trusted.
 *
 *  Sooo, that gives us:
 *  1. For each
 *
 *
 *  1. App1, NT
 *     Task11
 *  2. App2, NT
 *     Task21
 *  3. App3, T
 *     Task31
 *  4. App4, T
 *     Task41
 *
 *  * App2->App1: ActivateTask(Task11)
 *    This is OK as long as Task11 have granted access to it during configuration
 *  * App4->App3: ActivateTask(Task31)
 *    This is OK as long as Task31 have granted access to it during configuration
 *  * App1->App4: ActivateTask(Task41)
 *    It's not really clear if the OS automagically exports all services..
 *    But this could also be CallTrustedFunction(ServiceId_AcivateTask,???)
 *
 */

#define APPL_ID_TO_MASK(_x)   (1UL<<(_x))
#define APPL_REGS_CNT 16 // aligning with task register count

#if ( OS_SC3 == STD_ON ) || ( OS_SC4 == STD_ON )
#define OS_APP_CALL_ERRORHOOKS( x ) \
    for(uint32 i=0;i<OS_APPLICATION_CNT;i++) { \
        if( Os_AppConst[i].ErrorHook != NULL ) { \
            Os_AppConst[i].ErrorHook(x); \
        } \
    }
#else
#define OS_APP_CALL_ERRORHOOKS( x )
#endif


typedef struct OsAppHooks {
    void (*startup)( void );
    void (*shutdown)( Std_ReturnType Error );
    void (*error)( Std_ReturnType Error );
} OsAppHooksType;

typedef struct {
    void *      bottom;             /**< @brief Bottom of the stack (high address) */
    void *      top;                /**< @brief Top of the stack (low address) */
    uint32      size;               /**< @brief Size of the stack in bytes */
} OsAppStackType;

typedef struct OsAppVar {
    _Bool trusted;                  /**< @brief just a copy from const */
    uint32      appId;              /**< @brief just a copy from const */
    ApplicationStateType state;		/**< @brief The current state of the application */
    uint32  regs[APPL_REGS_CNT];               /**< @brief Precalc register used by the MPU */
#if ( OS_SC3 == STD_ON ) || ( OS_SC4 == STD_ON )
    uint32  nestCnt;                /**< @brief Interrupt nest counter */
    void *  intStack;               /**< @brief Pointer to bottom of Interrupt stack */
    void *  savedIntStack;          /**< @brief Pointer to current int stack */
#endif
} OsAppVarType;

/*
 * Application configuration parameters. While generating Os configuration, parameters of the
 * configured Os application is filled in this structure type.
 */
typedef struct OsApplication {
    uint32 	    appId;			/* The ID of the application */
    const char 	*name;		    /* Name of the application */
    _Bool	    trusted;		/* Trusted or not, 0 - Non-trusted */ /** @req SWS_Os_00446 */
    uint8       core;			/* The core number on which this application runs */

    /* hooks, the names are StartupHook_<name>(), etc. */
#if ( OS_SC3 == STD_ON ) || ( OS_SC4 == STD_ON )
    void (*StartupHook)( void );                /** @req SWS_Os_00543 Available in SC3 and SC4 (and other SCs (SWS_Os_00240)) */
    void (*ShutdownHook)( StatusType Error );   /** @req SWS_Os_545 Available in SC3 and SC4 (and other SCs (SWS_Os_00240)) */
    void (*ErrorHook)( StatusType Error );      /** @req SWS_Os_544 Available in SC3 and SC4 (and other SCs (SWS_Os_00240)) */

    int 	restartTaskId;

    /* Memory Protection regions */
    void * dataStart;
    void * dataEnd;

    void * bssStart;
    void * bssEnd;

    OsAppStackType intStack;         /**< @brief Interrupt stack */
#endif
} OsAppConstType;

#if ( OS_SC1 == STD_ON ) || ( OS_SC4 == STD_ON )
typedef void ( * trusted_func_t)( TrustedFunctionIndexType , TrustedFunctionParameterRefType );
#endif


#if OS_APPLICATION_CNT!=0
/*lint -e9003 MISRA:EXTERNAL_FILE:Os_AppVar is declared out of scope since it is used in Os_ApplGet:[MISRA 2012 Rule 8.9, advisory]*/
extern OsAppVarType Os_AppVar[OS_APPLICATION_CNT];

extern const OsAppConstType Os_AppConst[OS_APPLICATION_CNT];

#if !(defined(CFG_SAFETY_PLATFORM) || defined(BUILD_OS_SAFETY_PLATFORM))

static inline ApplicationStateType Os_ApplGetState (ApplicationType applId) {
	return Os_AppVar[applId].state;
}

static inline void Os_ApplSetState (ApplicationType applId, ApplicationStateType applState) {
	Os_AppVar[applId].state = applState;
}
#endif

static inline OsAppVarType *Os_ApplGet(ApplicationType id) {
    return &Os_AppVar[id];
}

static inline const OsAppConstType *Os_ApplGetConst(ApplicationType id) {
    return &Os_AppConst[id];
}
#endif // OS_APPLICATION_CNT!=0

static inline uint8 Os_ApplGetCore( ApplicationType appl )
{
    return Os_AppConst[appl].core;
}

/**
 *
 * @param mask  Target accessing application mask
 * @return
 */
static inline StatusType Os_ApplHaveAccess( uint32 mask ) {

    /* @req SWS_Os_00056 */
    if( (APPL_ID_TO_MASK(OS_SYS_PTR->currApplId) & mask) == 0 ) {
        return E_OS_ACCESS;
    }

    /* @req SWS_Os_00504/ActivateTask
     * The Operating System module shall deny access to Operating System
     * objects from other OS-Applications to an OS-Application which is not in state
     * APPLICATION_ACCESSIBLE.
     * */

    /* We are activating a task in another application */
    // Currently only APPLICATION_ACCESSIBLE is supported 
#if 0
    if( !( Os_ApplGetState(OS_SYS_PTR->currApplId) == APPLICATION_ACCESSIBLE)) {
        return E_OS_ACCESS;
    }
#endif

    return E_OK;
}

/* Sanity checks */
static inline StatusType Os_ApplCheckState( ApplicationType appId ) {
    StatusType status = E_OK;
    (void)appId;    /*lint !e920 MISRA:STANDARDIZED_INTERFACE */
    // Currently only APPLICATION_ACCESSIBLE is supported 
#if 0
    if( Os_ApplGetState(appId) != APPLICATION_ACCESSIBLE ) {
    	status = E_OS_ACCESS;
    }
#endif
    return status;
}
static inline StatusType Os_ApplCheckAccess( ApplicationType appId, uint32 mask ) {
    StatusType status = E_OK;
    if( (APPL_ID_TO_MASK(appId) & mask) == 0 ) {
    	status = E_OS_ACCESS;
    }
    return status;
}

/**
 * Starting Os Applications during Os Init.
 */
void Os_ApplStart( void );
#if !(defined(CFG_SAFETY_PLATFORM) || defined(BUILD_OS_SAFETY_PLATFORM))
/**
 * Setting Os Application to accessible
 */
StatusType AllowAccess( void );

/**
 * Getting state of the application
 */
StatusType GetApplicationState( ApplicationType applId, ApplicationStateRefType value );
#endif //CFG_SAFETY_PLATFORM


#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)

/**
 * @brief     Set endmark for application interrupt stack.
 * @param aP  Pointer to application
 */
static inline void Os_AppSetIsrEndmark( ApplicationType id ) {
    uint8 *end = Os_AppConst[id].intStack.top;
    *end = STACK_PATTERN;
}

/**
 * @brief     Set startmark for application interrupt stack.
 * @param aP  Pointer to application
 */
static inline void Os_AppSetIsrStartmark( ApplicationType id ) {
    /* Get high address */
    uint8 *bottom = Os_AppConst[id].intStack.bottom;
    *(bottom-1ul) = STACK_PATTERN;
}


/**
   @brief    Check endmark for application interrupt stack.
 * @param aP Pointer to application
 * @return FALSE if the end-mark is not ok.
 */
static inline boolean Os_AppIsIsrEndmarkOk( ApplicationType id ) {
    boolean rv = FALSE;
    uint8 *end = Os_AppConst[id].intStack.top;

    if( *end == STACK_PATTERN ) {
        rv = TRUE;
    }

    return rv;
}

/**
   @brief    Check endmark for application interrupt stack.
 * @param aP Pointer to application
 * @return FALSE if the end-mark is not ok.
 */
static inline boolean Os_AppIsIsrStartmarkOk( ApplicationType id ) {
    boolean rv = FALSE;
    uint8 *bottom = Os_AppConst[id].intStack.bottom;

    if( *(bottom-1UL) == STACK_PATTERN ) {
        rv = TRUE;
    }

    return rv;
}



void Os_AppIsrStackPerformCheck( ApplicationType id );

void Os_AppIsrGetStackInfo( ApplicationType id, OsAppStackType *stack );

#endif  /* (OS_SC3==STD_ON) || (OS_SC4==STD_ON) */


#endif /* APPLICATION_H_ */
