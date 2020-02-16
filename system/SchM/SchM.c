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
 *  CanIf   Have No mainf                          N/A
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
 *  It seems it's mandatory to include SchM_<mod>.h for each BSW module.
 *  So,
 *  - <mod>.c ALWAYS include SchM_<mod.h>
 *  - SchM.c have condidional include on SchM_<mod>.h, e.g must define it's MainFunctions.
 *
 *
 *
 */

#include "SchM.h"
#include "SchM_cfg.h"


#if defined(USE_KERNEL)
#include "Os.h"
#endif

#if defined(USE_MCU)
#include "Mcu.h"
#endif
#if defined(USE_GPT)
#include "Gpt.h"
#endif

#if defined(USE_CAN)
#include "Can.h"
#include "SchM_Can.h"
#else
#define	SCHM_MAINFUNCTION_CAN_WRITE()
#define	SCHM_MAINFUNCTION_CAN_READ()
#define	SCHM_MAINFUNCTION_CAN_BUSOFF()
#define	SCHM_MAINFUNCTION_CAN_ERROR()
#define	SCHM_MAINFUNCTION_CAN_WAKEUP()
#endif

#if defined(USE_CANIF)
#include "CanIf.h"
#include "SchM_CanIf.h"
#endif

#if defined(USE_PDUR)
#include "PduR.h"
#include "SchM_PduR.h"
#endif

#if defined(USE_COM)
#include "Com.h"
#include "SchM_Com.h"
#else
#define SCHM_MAINFUNCTION_COMRX()
#define SCHM_MAINFUNCTION_COMTX()
#endif

#if defined(USE_CANTP)
#include "CanTp.h"
#include "SchM_CanTp.h"
#else
#define SCHM_MAINFUNCTION_CANTP()
#endif

#if defined(USE_J1939TP)
#include "J1939Tp.h"
#include "SchM_J1939TP.h"
#else
#define SCHM_MAINFUNCTION_J1939TP()
#endif


#if defined(USE_DCM)
#include "Dcm.h"
#include "SchM_Dcm.h"
#else
#define SCHM_MAINFUNCTION_DCM()
#endif

#if defined(USE_DEM)
#include "Dem.h"
#include "SchM_Dem.h"
#else
#define SCHM_MAINFUNCTION_DEM()
#endif

#if defined(USE_PWM)
#include "Pwm.h"
#include "SchM_Pwm.h"
#endif


#if defined(USE_IOHWAB)
#include "IoHwAb.h"
#include "SchM_IoHwAb.h"
#else
#define SCHM_MAINFUNCTION_IOHWAB()
#endif

#if defined(USE_FLS)
#include "Fls.h"
#include "SchM_Fls.h"
#else
#define SCHM_MAINFUNCTION_FLS()
#endif

#if defined(USE_ECUM)
#include "EcuM.h"
#include "SchM_EcuM.h"
#else
#define SCHM_MAINFUNCTION_ECUM()
#endif

#if defined(USE_EEP)
#include "Eep.h"
#include "SchM_Eep.h"
#else
#define SCHM_MAINFUNCTION_EEP()
#endif

#if defined(USE_FEE)
#include "Fee.h"
#include "SchM_Fee.h"
#else
#define SCHM_MAINFUNCTION_FEE()
#endif

#if defined(USE_EA)
#include "Ea.h"
#include "SchM_Ea.h"
#else
#define SCHM_MAINFUNCTION_EA()
#endif

#if defined(USE_NVM)
#include "NvM.h"
#include "SchM_NvM.h"
#else
#define SCHM_MAINFUNCTION_NVM()
#endif

#if defined(USE_COMM)
#include "ComM.h"
#include "SchM_ComM.h"
#else
#define SCHM_MAINFUNCTION_COMM()
#endif

#if defined(USE_NM)
#include "Nm.h"
#include "SchM_Nm.h"
#else
#define SCHM_MAINFUNCTION_NM()
#endif

#if defined(USE_CANNM)
#include "CanNm.h"
#include "SchM_CanNm.h"
#else
#define SCHM_MAINFUNCTION_CANNM()
#endif

#if defined(USE_CANSM)
#include "CanSM.h"
#include "SchM_CanSM.h"
#else
#define SCHM_MAINFUNCTION_CANSM()
#endif

#if defined(USE_UDPNM)
#include "UdpNm.h"
#endif

#if defined(USE_LINIF)
#include "LinIf.h"
#else
#define SCHM_MAINFUNCTION_LINIF()
#endif

#if defined(USE_LINSM)
#include "LinSM.h"
#else
#define SCHM_MAINFUNCTION_LINSM()
#endif

#if defined(USE_SPI)
#include "Spi.h"
#include "SchM_Spi.h"
#else
#define SCHM_MAINFUNCTION_SPI()
#endif

#if defined(USE_WDG)
#include "Wdg.h"
#endif

#if defined(USE_WDGM)
#include "WdgM.h"
#include "SchM_WdgM.h"
#else
#define SCHM_MAINFUNCTION_WDMG()
#define SCHM_MAINFUNCTION_WDGM_TRIGGER()
#define SCHM_MAINFUNCTION_WDGM_ALIVESUPERVISION()
#endif

SCHM_DECLARE(CAN_WRITE);
SCHM_DECLARE(CAN_READ);
SCHM_DECLARE(CAN_BUSOFF);
SCHM_DECLARE(CAN_WAKEUP);
SCHM_DECLARE(CAN_ERROR);
SCHM_DECLARE(LINIF);
SCHM_DECLARE(COMRX);
SCHM_DECLARE(COMTX);
SCHM_DECLARE(CANTP);
SCHM_DECLARE(CANNM);
SCHM_DECLARE(DCM);
SCHM_DECLARE(DEM);
SCHM_DECLARE(IOHWAB);
SCHM_DECLARE(COMM);
SCHM_DECLARE(NM);
SCHM_DECLARE(CANSM);
SCHM_DECLARE(LINSM);
SCHM_DECLARE(ECUM);
SCHM_DECLARE(NVM);
SCHM_DECLARE(FEE);
SCHM_DECLARE(EEP);
SCHM_DECLARE(EA);
SCHM_DECLARE(FLS);
SCHM_DECLARE(WDGM_TRIGGER);
SCHM_DECLARE(WDGM_ALIVESUPERVISION);



void SchM_Init( void ) {

}

void SchM_Deinit( void ) {

}

void SchM_GetVersionInfo( Std_VersionInfoType *versionInfo ) {

}


static void runMemory( void ) {
	SCHM_MAINFUNCTION_NVM();
	SCHM_MAINFUNCTION_EA();
	SCHM_MAINFUNCTION_FEE();
	SCHM_MAINFUNCTION_EEP();
	SCHM_MAINFUNCTION_FLS();
	SCHM_MAINFUNCTION_SPI();
}

/**
 * Startup task.
 */
TASK(SchM_Startup){

	/* At this point EcuM ==  ECUM_STATE_STARTUP_ONE */

	/* Set events on TASK_ID_BswService_Mem */
	SetRelAlarm(ALARM_ID_Alarm_BswService, 10, 2);

	/*
	 * Call EcuM_StartupTwo that do:
	 * - Startup RTE,
	 * - Wait for Nvm to complete
	 * - Call EcuM_AL_DriverInitThree() to initiate Nvm dependent modules.
	 */
	EcuM_StartupTwo();

	/* Start to schedule BSW parts */
	CancelAlarm(ALARM_ID_Alarm_BswService);
	SetRelAlarm(ALARM_ID_Alarm_BswService, 10, 5);

#if !defined(CFG_SCHM_DISABLE_ECUM_REQUEST_RUN)
	EcuM_RequestRUN(ECUM_USER_User_1);
#endif

	TerminateTask();

}


TASK(SchM_BswService) {
	EcuM_StateType  state;

	EcuM_GetState(&state);

	switch( state ) {
	case ECUM_STATE_STARTUP_ONE:
		/* Nothing to schedule */
		break;
	case ECUM_STATE_STARTUP_TWO:
		runMemory();
		break;
	default:
		runMemory();

		SCHM_MAINFUNCTION_ECUM();

		SCHM_MAINFUNCTION_CAN_WRITE();
		SCHM_MAINFUNCTION_CAN_READ();
		SCHM_MAINFUNCTION_CAN_BUSOFF();
		SCHM_MAINFUNCTION_CAN_ERROR();
		SCHM_MAINFUNCTION_CAN_WAKEUP();


		SCHM_MAINFUNCTION_COMRX();
		SCHM_MAINFUNCTION_COMTX();

		SCHM_MAINFUNCTION_CANTP();
		SCHM_MAINFUNCTION_J1939TP();
		SCHM_MAINFUNCTION_DCM();
		SCHM_MAINFUNCTION_DEM();

		SCHM_MAINFUNCTION_IOHWAB();
		SCHM_MAINFUNCTION_COMM();
		SCHM_MAINFUNCTION_NM();
		SCHM_MAINFUNCTION_CANNM();
		SCHM_MAINFUNCTION_CANSM();
        SCHM_MAINFUNCTION_LINIF();
		SCHM_MAINFUNCTION_LINSM();
		SCHM_MAINFUNCTION_WDGM_TRIGGER();
		SCHM_MAINFUNCTION_WDGM_ALIVESUPERVISION();
		break;
	}

	TerminateTask();
}

void SchM_MainFunction( void ) {

}



