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









/* Link time config */

#warning "This default file may only be used as an example!"

#include "Std_Types.h"
#include "Os.h"
#include "Mcu.h"
#include "Gpt.h"
#include <assert.h>
#include "Cpu.h"

//#include "ComM.h"
//#include "WdgM.h"

//#include "Dem.h"
//#include "Det.h"
//#include "NvM.h"
//#include "rte.h"

extern void os_exception_IVPR();
extern void os_exception_IVOR10();
// The OS startup hook

#if 0
void StartupHook( void ) {

	// There a several things we need to connect here
	// * counter id for the system counter is 0
	// * that need to be attached to Gpt
	// --> We need to tell the OS that what Gpt channel it's
	// counter is connected to.

	// TODO: We need counter information here..
	// How do I get that information from the OS???

	//?	GetCounterAlarmBase()

//	Gpt_StartTimer(GPT_CHANNEL_DEC, 100 );
//	Gpt_EnableNotification(GPT_CHANNEL_DEC);
//	Gpt_SetMode(0);
}
#endif

extern void os_system_timer( void );

void EcuM_Init( void );
int  main( void ) {

	EcuM_Init();
	return 0;
}

void EcuM_Init( void ) {
	/* Call all mandatory interfaces, see 8.7.1 */

	/* I'm giving up on the MCU configuration. From what I
	 * can tell from the documentation the Mcu_Init()
	 */

#if 0
	ComM_ConfigType comMConfig;
	WdgM_ConfigType wdgmConfig;
#endif

// Startup I
	{
		const Mcu_ConfigType mcuConfig;
		Mcu_Init(&mcuConfig);
	}
//	Mcu_GetResetReason();
#if 0
	Det_Init();
#if defined(USE_DEM)
	Dem_PreInit();
#endif
	EcuM_AL_DriverInitOne();
#endif


	{
		// If we assume the OS will require SYSTEM_COUNTER to function.
		// We should either get the values from SYSTEM_COUNTER

		Gpt_ConfigType GptConfigData[] =
			{ {
				.GptChannelId = GPT_CHANNEL_DEC,
				.GptChannelMode = GPT_MODE_CONTINUOUS,
				.GptChannelClkSrc = 0,
				.GptNotification = os_system_timer,
			}
			};
		Gpt_Init(&GptConfigData[0]);

		// The timer is actually started in StartupHook()
	}


//	EcuM_SelectShutdownTarget(target,mode);
	StartOS(OSDEFAULTAPPMODE);
// Startup II
	//EcuM_AL_DriverInitTwo();

#if 0
	ComM_Init(&comMConfig);
	WdgM_Init(&wdgmConfig);
	NvM_Init();
	NvM_ReadAll();
	NvM_WriteAll();
	NvM_CancelWriteAll();

#if defined(USE_DEM)
	Dem_PreInit();
	Dem_Init();
	{
		Dem_EventIdType id = 0;
		Dem_EventStatusType status = 0;
		Dem_ReportErrorStatus(id,status);
	}
#endif
	Rte_Start();
	Rte_Stop();
#endif

	ShutdownOS(E_OK);
	EnableAllInterrupts();
	DisableAllInterrupts();

//	Det_ReportError(1,2,3,4);
}

void EcuM_Shutdown( void ) {

}

void EcuM_GetVersionInfo( Std_VersionInfoType *versioninfo ) {

}

