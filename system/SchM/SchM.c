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


/* ----------------------------[information]----------------------------------*/
/*
 * Author: mahi
 *
 * Part of Release:
 *   3.1.5
 *
 * Description:
 *   Implements the SchM module
 *
 * Support:
 *   General				  Have Support
 *   -------------------------------------------
 *   SCHM_DEV_ERROR_DETECT	 	N
 *   SCHM_VERSION_INFO_API		N
 *
 *   General				  Have Support
 *   -------------------------------------------
 *   SCHM_POSITION_IN_TASK		N
 *   SCHM_MAINFUNCTION_REF		N
 *   SCHM_MAPPED_TO_TASK		N
 *   .....
 *
 * Implementation Notes:
 *   There are a lot of examples in SchM about scheduling and it
 *   all seems very complicated. What is boils down to is that
 *   the BSW MainFunctions have different requirements. Most modules
 *   have only periodic needs for timeouts and other things. But there
 *   are also module that needs extra iterations at certain points in time, to
 *   be really efficient.
 *
 *
 *   BSW Module Code:
 *    SchM_Enter_NvM(EXCLUSIVE_AREA_0);
 *    ..Do stuff...
 *    SchM_Enter_NvM(EXCLUSIVE_AREA_0);
 *
 *   but today we have Irq_Save(state), Irq_Restore(state).
 *   ArcCore Irq_Save/Irq_Restore is almost the same as SuspendAllInterrupts/ResumeAllInterrupts,
 *   since they can both be nested and saves state. But the OSEK (Susp../Resume..) can't do it locally, it
 *   assumes some kind of local FIFO, that is bad.
 *
 *
 * BSW Modules with generated mainfunction period times. Checked Only 3.1.5
 *
 *          Specification                        Studio   Core=Generator
 *  --------------------------------------------------------------------------------------
 *  Adc     N/A *1
 *  Can     CanMainFunctionReadPerdiod             No      No
 *          CanMainFunctionWritePerdiod
 *          ..
 *  CanIf   Have mainf. but no period              No
 *  CanNm   CanNmMainFunctionPeriod                Yes     Accessible in struct.. not as define
 *  CanSm   Have mainf. but no period              Yes*2   Nothing is generated
 *  CanTp   CanTpMainFunctionPeriod                Yes     CANTP_MAIN_FUNCTION_PERIOD_TIME_MS
 *  CanTrcv Have mainf. but no period              N/A
 *  Com     Have mainf. but no period              No*3
 *  ComM    ComMMainFunctionPeriod                 Yes     Accessible in struct.. not as define
 *  Dcm     Have MainF. DcmTaskTime                Yes     DCM_MAIN_FUNCTION_PERIOD_TIME_MS
 *  Dem     Have MainF. DemTaskTime				    No
 *  EcuM    Have MainF.EcuMMainFunctionPeriod      Yes     ECUM_MAIN_FUNCTION_PERIOD
 *  Ea      Have MainF. ON_PRE_CONDITION (ie not cyclic)
 *  Eep     Have MainF. VARIABLE_CYCLIC
 *  Fee     Have MainF. ON_PRE_CONDITION
 *  Fls     Have MainF. FIXED_CYCLIC
 *  IoHwAb  Have no mainfunction
 *  ..
 *  Nm      Have MainF. FIXED_CYCLIC ,            No
 *          NmCycletimeMainFunction
 *  NvM     Have MainF. VARIABLE_CYCLIC			  No
 *  PduR    Have no MainF.
 *  Spi     Have MainF. FIXED_CYCLIC, no period
 *  WdgM    Have MainF. WdgMTriggerCycle           *4
 *
 * *1 No MainFunction
 * *2 What is it used for?
 * *3 Com have lots of timing... it's related to what? (reads timer?)
 * *4 Probably not.
 *
 * ----->>>>
 *
 * Conclusion:
 * * Support in generator is extremely limited.
 * * Support in specification is limited
 * * Support in studio is limited
 *
 *  Write scheduling information directly in the SchM_<mod>.h files.
 *  OR
 *  Write scheduling information in SchM_cfg.h....better (keeps information in one place)
 *
 *     #if defined(USE_SCHM)
 *     assert( SCHM_TIMER(x) == <period> )
 *     #endif
 *
 */

#include "SchM.h"
#include "SchM_cfg.h"
#if defined(USE_ECUM)
#include "EcuM.h"
#include "SchM_EcuM.h"
#endif

SCHM_DECLARE(ECUM);
SCHM_DECLARE(NVM);
SCHM_DECLARE(FEE);
SCHM_DECLARE(EA);

void SchM_Init( void ) {

}

void SchM_Deinit( void ) {

}

void SchM_GetVersionInfo( Std_VersionInfoType *versionInfo ) {

}

void SchM_MainFunction( void ) {

#if 0
#define EXCLUSIVE_AREA_0	0
	SchM_Enter_EcuM(EXCLUSIVE_AREA_0);
	SchM_Exit_EcuM(EXCLUSIVE_AREA_0);
#endif

	SCHM_MAINFUNCTION(ECUM,EcuM_MainFunction());
	SCHM_MAINFUNCTION(EA,Ea_MainFunction());
}

/*
 * Implement
 */
// Critical sections
// void SchM_Enter_<ModulePrefix>( uint8 instance, uint8 exclusiveArea )
// void SchM_Exit_<ModulePrefix>( uint8 instance, uint8 exclusiveArea )

// Triggers
// SchM_ReturnType SchM_ActMainFunction_<ModulePrefix>( uint8 instance, uint8 activationPoint );
// SchM_ReturnType SchM_CancelMainFunction_<ModulePrefix>( uint8 instance, uint8 activationPoint );

/*
 * Callable functions in the <ModulePrefix>
 */
// <ModulePrefix>_MainFunction_<name>()
// <ModulePrefix>_MainFunction_<name>()



