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








#include "EcuM.h"
#include "Modules.h"
#include "string.h"
#include "Os.h"
#include "EcuM_Internals.h"
#include "EcuM_Cbk.h"
#include "Mcu.h"
#include "Det.h"
#include "irq.h"

EcuM_GobalType internal_data;

void EcuM_Init( void )
{
	internal_data.current_state = ECUM_STATE_STARTUP_ONE;

	// Initialize drivers that are needed to determine PostBuild configuration
	EcuM_AL_DriverInitZero();

	// Initialize the OS
	InitOS();

	// Enable interrupts
	Irq_Init();


	// Determine PostBuild configuration
	internal_data.config = EcuM_DeterminePbConfiguration();

	// Check consistency of PB configuration
	// TODO

	// Initialize drivers needed before the OS-starts
	EcuM_AL_DriverInitOne(internal_data.config);

	// Determine the reset/wakeup reason
	// TODO Mcu_ResetType type = Mcu_GetResetReason();

	// Set default shutdown target
	internal_data.shutdown_target = internal_data.config->EcuMDefaultShutdownTarget;
	internal_data.shutdown_mode = internal_data.config->EcuMDefaultShutdownMode;

	// Set default application mode
	internal_data.app_mode = internal_data.config->EcuMDefaultAppMode;

	internal_data.initiated = TRUE;

	// Start this baby up
	StartOS(internal_data.app_mode);
}

void EcuM_StartupTwo()
{
#if	(ECUM_INCLUDE_NVRAM_MGR == STD_ON)
	uint32 timer;
#endif

	internal_data.current_state = ECUM_STATE_STARTUP_TWO;

	// Initialize the BSW scheduler
	// TODO SchM_Init();

	// Initialize drivers that don't need NVRAM data
	EcuM_AL_DriverInitTwo(internal_data.config);

#if	(ECUM_INCLUDE_NVRAM_MGR == STD_ON)
	// Start timer to wait for NVM job to complete
	timer = Os_SysTickGetTimeElapsed();
#endif

	// Prepare the system to startup RTE
	// TODO EcuM_OnRTEStartup();
#if defined(USE_RTE)
	Rte_Start();
#endif

#if	(ECUM_INCLUDE_NVRAM_MGR == STD_ON)
	// Wait for the NVM job to terminate
	while(Os_SysTickGetTimeElapsed()-timer < internal_data.config.EcuMNvramReadAllTimeout)
	{
		//TODO
	}
#endif

	// Initialize drivers that need NVRAM data
	EcuM_AL_DriverInitThree(internal_data.config);

	// Indicate mode change to RTE
	// TODO
}

// Typically called from OS shutdown hook
void EcuM_Shutdown()
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
		for(;;);
#endif
	}
}

Std_ReturnType EcuM_GetState(EcuM_StateType* state)
{
#if (ECUM_DEV_ERROR_DETECT == STD_ON)
	if (state == NULL)
	{
		Det_ReportError(MODULE_ID_ECUM, 1, ECUM_GETSTATE_ID, ECUM_E_NULL_POINTER);
		return E_NOT_OK;
	}
#endif

	*state = internal_data.current_state;

	return E_OK;
}

Std_ReturnType EcuM_SelectApplicationMode(AppModeType appMode)
{
#if (ECUM_DEV_ERROR_DETECT == STD_ON)
	if (!internal_data.initiated)
	{
		Det_ReportError(MODULE_ID_ECUM, 1, ECUM_SELECTAPPMODE_ID, ECUM_E_NOT_INITIATED);
		return E_NOT_OK;
	}
#endif

	// TODO Save this application mode for next startup

	return E_NOT_OK;
}

Std_ReturnType EcuM_GetApplicationMode(AppModeType* appMode)
{
#if (ECUM_DEV_ERROR_DETECT == STD_ON)
	if (!internal_data.initiated)
	{
		Det_ReportError(MODULE_ID_ECUM, 1, ECUM_GETAPPMODE_ID, ECUM_E_NOT_INITIATED);
		return E_NOT_OK;
	}

	if (appMode == NULL)
	{
		Det_ReportError(MODULE_ID_ECUM, 1, ECUM_GETAPPMODE_ID, ECUM_E_NULL_POINTER);
		return E_NOT_OK;
	}
#endif

	*appMode = internal_data.app_mode;

	return E_OK;
}

Std_ReturnType EcuM_SelectBootTarget(EcuM_BootTargetType target)
{
#if (ECUM_DEV_ERROR_DETECT == STD_ON)
	if (!internal_data.initiated)
	{
		Det_ReportError(MODULE_ID_ECUM, 1, ECUM_SELECT_BOOTARGET_ID, ECUM_E_NOT_INITIATED);
		return E_NOT_OK;
	}
#endif

	// TODO Do something great here

	return E_NOT_OK;
}

Std_ReturnType EcuM_GetBootTarget(EcuM_BootTargetType* target)
{
#if (ECUM_DEV_ERROR_DETECT == STD_ON)
	if (!internal_data.initiated)
	{
		Det_ReportError(MODULE_ID_ECUM, 1, ECUM_GET_BOOTARGET_ID, ECUM_E_NOT_INITIATED);
		return E_NOT_OK;
	}

	if (target == NULL)
	{
		Det_ReportError(MODULE_ID_ECUM, 1, ECUM_GET_BOOTARGET_ID, ECUM_E_NULL_POINTER);
		return E_NOT_OK;
	}
#endif

	// TODO Return selected boot target here

	return E_NOT_OK;
}
