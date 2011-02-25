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

//lint -emacro(904,VALIDATE,VALIDATE_RV,VALIDATE_NO_RV) //904 PC-Lint exception to MISRA 14.7 (validate macros).

#include "EcuM.h"
#include "Modules.h"
#include <string.h>
#include <Os.h>
#include "EcuM_Internals.h"
#include "EcuM_Cbk.h"
#include "Mcu.h"
#include "Det.h"
#include "irq.h"
#if defined(USE_NVM)
#include "Nvm.h"
#endif
#if defined(USE_RTE)
#include "Rte_Main.h"
#endif

EcuM_GlobalType internal_data;

void EcuM_Init( void )
{
	Std_ReturnType status;
	internal_data.current_state = ECUM_STATE_STARTUP_ONE;

	// Initialize drivers that are needed to determine PostBuild configuration
	EcuM_AL_DriverInitZero();

	// Initialize the OS
	InitOS();

	// Enable interrupts
	Irq_Init();

	// Determine PostBuild configuration
	internal_data.config = EcuM_DeterminePbConfiguration();

	// TODO: Check consistency of PB configuration

	// Initialize drivers needed before the OS-starts
	EcuM_AL_DriverInitOne(internal_data.config);

	// Determine the reset/wakeup reason
	// TODO Mcu_ResetType type = Mcu_GetResetReason();

	// Moved this here because EcuM_SelectShutdownTarget needs us to be initilized.
	internal_data.initiated = TRUE;

	// Set default shutdown target
	status = EcuM_SelectShutdownTarget(internal_data.config->EcuMDefaultShutdownTarget,internal_data.config->EcuMDefaultSleepMode);/** @req EcuM2181 */
	if(status!=E_OK){
		//TODO: Report error.
	}


	// Set default application mode
	status =  EcuM_SelectApplicationMode(internal_data.config->EcuMDefaultAppMode);
	if(status!=E_OK){
		//TODO: Report error.
	}

#if defined(USE_COMM)
	internal_data.run_comm_requests = 0;
#endif
	internal_data.run_requests = 0;
	internal_data.postrun_requests = 0;

	// Start this baby up
	AppModeType appMode;
	EcuM_GetApplicationMode(&appMode);
	StartOS(appMode); /** @req EcuM2141 */
}

void EcuM_StartupTwo(void)
{
	//TODO:  Validate that we are in state STARTUP_ONE.
#if defined(USE_NVM)
	extern CounterType Os_Arc_OsTickCounter;
	TickType tickTimerStart, tickTimer, tickTimerElapsed;
	StatusType tickTimerStatus;
	static NvM_RequestResultType readAllResult;
#endif

	internal_data.current_state = ECUM_STATE_STARTUP_TWO;

	// Initialize the BSW scheduler
	// TODO SchM_Init();

	// Initialize drivers that don't need NVRAM data
	EcuM_AL_DriverInitTwo(internal_data.config);

#if defined(USE_NVM)
	// Start timer to wait for NVM job to complete
	tickTimerStatus = GetCounterValue(Os_Arc_OsTickCounter , &tickTimerStart);
	if (tickTimerStatus != E_OK) {
		Det_ReportError(MODULE_ID_ECUM, 0, ECUM_ARC_STARTUPTWO_ID, ECUM_E_ARC_TIMERERROR);
	}
#endif

	// Prepare the system to startup RTE
	// TODO EcuM_OnRTEStartup();
#if defined(USE_RTE)
	Rte_Start();
#endif

#if defined(USE_NVM)
	// Wait for the NVM job (NvmReadAll) to terminate
	do {
		NvM_GetErrorStatus(0, &readAllResult);	// Read the multiblock status
		tickTimer = tickTimerStart;	// Save this because the GetElapsedCounterValue() will destroy it.
		tickTimerStatus =  GetElapsedCounterValue(Os_Arc_OsTickCounter, &tickTimer, &tickTimerElapsed);
		if (tickTimerStatus != E_OK) {
			Det_ReportError(MODULE_ID_ECUM, 0, ECUM_ARC_STARTUPTWO_ID, ECUM_E_ARC_TIMERERROR);
		}
	} while( (readAllResult == NVM_REQ_PENDING) && (tickTimerElapsed < internal_data.config->EcuMNvramReadAllTimeout) );

	// Initialize drivers that need NVRAM data
	EcuM_AL_DriverInitThree(internal_data.config);
#endif

	// TODO: Indicate mode change to RTE

	// If coming from startup sequence, enter Run mode
//	if (internal_data.current_state == ECUM_STATE_STARTUP_TWO)
		EcuM_enter_run_mode();

}

// Typically called from OS shutdown hook
void EcuM_Shutdown(void)
{
	internal_data.current_state = ECUM_STATE_GO_OFF_TWO;

	// Let the last drivers do a nice shutdown
	EcuM_OnGoOffTwo();

	if (internal_data.shutdown_target == ECUM_STATE_OFF)
	{
		EcuM_AL_SwitchOff();
	}
	else
	{
#if (MCU_PERFORM_RESET_API == STD_ON)
		Mcu_PerformReset();
#else
		for(;;)
                {
                  ;
                }
#endif
	}
}

Std_ReturnType EcuM_GetState(EcuM_StateType* state)
{
	VALIDATE_RV(internal_data.initiated, ECUM_GETSTATE_ID, ECUM_E_NOT_INITIATED, E_NOT_OK);
	VALIDATE_RV(state != NULL, ECUM_GETSTATE_ID, ECUM_E_NULL_POINTER, E_NOT_OK);

	*state = internal_data.current_state;

	return E_OK;
}

Std_ReturnType EcuM_SelectApplicationMode(AppModeType appMode)
{
	VALIDATE_RV(internal_data.initiated, ECUM_SELECTAPPMODE_ID, ECUM_E_NOT_INITIATED, E_NOT_OK);

	internal_data.app_mode = appMode;

	return E_OK;
}

Std_ReturnType EcuM_GetApplicationMode(AppModeType* appMode)
{
	VALIDATE_RV(internal_data.initiated, ECUM_GETAPPMODE_ID, ECUM_E_NOT_INITIATED, E_NOT_OK);
	VALIDATE_RV(appMode != NULL, ECUM_GETAPPMODE_ID, ECUM_E_NULL_POINTER, E_NOT_OK);

	*appMode = internal_data.app_mode;

	return E_OK;
}

Std_ReturnType EcuM_SelectBootTarget(EcuM_BootTargetType target)
{
	VALIDATE_RV(internal_data.initiated, ECUM_SELECT_BOOTARGET_ID, ECUM_E_NOT_INITIATED, E_NOT_OK);

	// TODO Do something great here
	(void) target;

	return E_NOT_OK;
}

Std_ReturnType EcuM_GetBootTarget(EcuM_BootTargetType* target)
{
	VALIDATE_RV(internal_data.initiated, ECUM_GET_BOOTARGET_ID, ECUM_E_NOT_INITIATED, E_NOT_OK);
	VALIDATE_RV(target != NULL, ECUM_GET_BOOTARGET_ID, ECUM_E_NULL_POINTER, E_NOT_OK);

	// TODO Return selected boot target here
	(void) target;

	return E_NOT_OK;
}


Std_ReturnType EcuM_SelectShutdownTarget(EcuM_StateType shutdownTarget, uint8 sleepMode)
{
	VALIDATE_RV(internal_data.initiated, ECUM_SELECTSHUTDOWNTARGET_ID, ECUM_E_NOT_INITIATED, E_NOT_OK);
	VALIDATE_RV((shutdownTarget == ECUM_STATE_OFF) || (shutdownTarget == ECUM_STATE_RESET) || (shutdownTarget == ECUM_STATE_SLEEP), ECUM_SELECTSHUTDOWNTARGET_ID, ECUM_E_INVALID_PAR, E_NOT_OK);

	internal_data.shutdown_target = shutdownTarget;
	internal_data.sleep_mode = sleepMode;

	return E_OK;
}


Std_ReturnType EcuM_GetShutdownTarget(EcuM_StateType* shutdownTarget, uint8* sleepMode) /** @req EcuM2824 */
{
	VALIDATE_RV(internal_data.initiated, ECUM_GETSHUTDOWNTARGET_ID, ECUM_E_NOT_INITIATED, E_NOT_OK);
	VALIDATE_RV(shutdownTarget != NULL, ECUM_GETSHUTDOWNTARGET_ID, ECUM_E_NULL_POINTER, E_NOT_OK);
	VALIDATE_RV(sleepMode != NULL, ECUM_GETSHUTDOWNTARGET_ID, ECUM_E_NULL_POINTER, E_NOT_OK);

	*shutdownTarget = internal_data.shutdown_target;
	*sleepMode = internal_data.sleep_mode;

	return E_OK;
}


Std_ReturnType EcuM_RequestRUN(EcuM_UserType user)
{
	VALIDATE_RV(internal_data.initiated, ECUM_REQUESTRUN_ID, ECUM_E_NOT_INITIATED, E_NOT_OK);
	VALIDATE_RV(user < ECUM_USER_ENDMARK, ECUM_REQUESTRUN_ID, ECUM_E_INVALID_PAR, E_NOT_OK);

	internal_data.run_requests |= (uint32)1 << user;

	return E_OK;
}

Std_ReturnType EcuM_ReleaseRUN(EcuM_UserType user)
{
	VALIDATE_RV(internal_data.initiated, ECUM_RELEASERUN_ID, ECUM_E_NOT_INITIATED, E_NOT_OK);
	VALIDATE_RV(user < ECUM_USER_ENDMARK, ECUM_RELEASERUN_ID, ECUM_E_INVALID_PAR, E_NOT_OK);

	internal_data.run_requests &= ~((uint32)1 << user);

	return E_OK;
}

#if defined(USE_COMM)
Std_ReturnType EcuM_ComM_RequestRUN(NetworkHandleType channel)
{
	VALIDATE_RV(internal_data.initiated, ECUM_COMM_REQUESTRUN_ID, ECUM_E_NOT_INITIATED, E_NOT_OK);
	VALIDATE_RV(channel < 32, ECUM_COMM_REQUESTRUN_ID, ECUM_E_INVALID_PAR, E_NOT_OK);

	internal_data.run_comm_requests |= (uint32)1 << channel;

	return E_OK;
}

Std_ReturnType EcuM_ComM_ReleaseRUN(NetworkHandleType channel)
{
	VALIDATE_RV(internal_data.initiated, ECUM_COMM_RELEASERUN_ID, ECUM_E_NOT_INITIATED, E_NOT_OK);
	VALIDATE_RV(channel < 32, ECUM_COMM_RELEASERUN_ID, ECUM_E_INVALID_PAR, E_NOT_OK);

	internal_data.run_comm_requests &= ~((uint32)1 << channel);

	return E_OK;
}

boolean EcuM_ComM_HasRequestedRUN(NetworkHandleType channel)
{
	VALIDATE_RV(internal_data.initiated, ECUM_COMM_HASREQUESTEDRUN_ID, ECUM_E_NOT_INITIATED, FALSE);
	VALIDATE_RV(channel < 32, ECUM_COMM_HASREQUESTEDRUN_ID, ECUM_E_INVALID_PAR, FALSE);

	return (internal_data.run_comm_requests &((uint32)1 << channel)) != 0;
}
#endif

Std_ReturnType EcuM_RequestPOST_RUN(EcuM_UserType user)
{
	VALIDATE_RV(internal_data.initiated, ECUM_REQUESTPOSTRUN_ID, ECUM_E_NOT_INITIATED, E_NOT_OK);
	VALIDATE_RV(user < ECUM_USER_ENDMARK, ECUM_REQUESTPOSTRUN_ID, ECUM_E_INVALID_PAR, E_NOT_OK);

	internal_data.postrun_requests |= (uint32)1 << user;

	return E_OK;
}

Std_ReturnType EcuM_ReleasePOST_RUN(EcuM_UserType user)
{
	VALIDATE_RV(internal_data.initiated, ECUM_RELEASEPOSTRUN_ID, ECUM_E_NOT_INITIATED, E_NOT_OK);
	VALIDATE_RV(user < ECUM_USER_ENDMARK, ECUM_RELEASEPOSTRUN_ID, ECUM_E_INVALID_PAR, E_NOT_OK);

	internal_data.postrun_requests &= ~((uint32)1 << user);

	return E_OK;
}

