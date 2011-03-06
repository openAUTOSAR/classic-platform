
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
 *  OS056: If an OS-object identifier is the parameter of an Operating System module’s
 *  system service, and no sufficient access rights have been assigned to this OS-object
 *  at  configuration  time  (Parameter  Os[...]AccessingApplication)  to  the  calling
 *  Task/Category  2  ISR,  the  Operating  System  module’s  system  service  shall  return
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



#if (OS_USE_APPLICATIONS == STD_ON)

/* STD container : OsApplicationHooks
 * class: 3,4
 * OsAppErrorHook:				1    Bool
 * OsAppShutdownHook:			1    Bool
 * OsAppStartupHook:			1    Bool
 */

typedef struct OsAppHooks {
	void (*startup)( void );
	void (*shutdown)( Std_ReturnType Error );
	void (*error)( Std_ReturnType Error );
} OsAppHooksType;


/* STD container : OsApplication
 * OsTrusted:					1    Bool
 * OsAppAlarmRef:				0..*
 * OsAppCounterRef:				0..*
 * OsAppIsrRef:					0..*
 * OsAppResourceRef:			0..*
 * OsAppScheduleTableRef:		0..*
 * OsAppTaskRef:				0..*
 * OsRestartTask:				0..1 Ref to system restart task
 * OsApplicatinHooks[C]			1
 * OsApplictionTrustedFunction[C]:	0..*
 *
 * Note! referenced objects also can be accessed by multiple applications.
 * Note! speed is needed when looking up the accessing application, not vice versa.
 * Note! The application knowledge about what objects belong to it is just a memory
 *       management issue, not an access issue.
 * */

typedef struct OsAppVar {
	/* 0 - Non-trusted application
	 * 1 - Trusted application */
	_Bool trusted;

	/* NOTE! Refs here is memory management issue */

	/* The current state of the application */
	ApplicationStateType state;

	/* Trusted functions */
	/* .... */
} OsAppVarType;


/* NON standard type.
 * Used for ROM based parameters.... TODO
 */
typedef struct OsApplication {
	uint32 	appId;
	char 	name[16];
	_Bool	trusted;

	/* hooks, the names are StartupHook_<name>(), etc. */
	void (*StartupHook)( void );
	void (*ShutdownHook)( StatusType Error );
	void (*ErrorHook)( StatusType Error );

	/* The application hooks */
//	OsAppHooksType hooks;


	/* Ref is ~0U terminated */
#if 0
	const uint8_t *alarmRef;
	const uint8_t *counterRef;
	const uint8_t *isrRef;
	const uint8_t *resourceRef;
	const uint8_t *schtblRef;
	const uint8_t *taskRef;
#endif

	int 	restartTaskId;
} OsAppConstType;

#if OS_APPLICATION_CNT!=0
extern OsAppVarType Os_AppVar[OS_APPLICATION_CNT];
#endif

#endif /*  (OS_USE_APPLICATIONS == STD_ON) */


#endif /* APPLICATION_H_ */
