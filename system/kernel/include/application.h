
#ifndef APPLICATION_H_
#define APPLICATION_H_

#if ( OS_SC3 == STD_ON) || ( OS_SC4 == STD_ON )

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

typedef struct OsApplication {
	/* 0 - Non-trusted application
	 * 1 - Trusted application */
	_Bool trusted;

	/* NOTE! Refs here is memory management issue */

	/* The application hooks */
	OsAppHooksType hooks;

	/* Trusted functions */
	/* .... */
} OsApplicationType;


/* NON standard type.
 * Used for ROM based parameters.... TODO
 */
typedef struct OsRomApplication {
	uint32 	application_id;
	char 	name[16];
	uint8	trusted;

	/* hooks */
	void (*StartupHook)( void );
	void (*ShutdownHook)( Std_ReturnType Error );
	void (*ErrorHook)( Std_ReturnType Error );

	uint32 isr_mask;
	uint32 scheduletable_mask;
	uint32 alarm_mask;
	uint32 counter_mask;
	uint32 resource_mask;
	uint32 message_mask;

} OsRomApplicationType;

#endif /*  ( OS_SC1 == STD_ON ) || ( OS_SC4 == STD_ON ) */


#endif /* APPLICATION_H_ */
