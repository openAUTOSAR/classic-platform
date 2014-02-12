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

#include "Std_Types.h"
#include "EcuM.h"
#include "EcuM_Generated_Types.h"
#include "EcuM_Internals.h"

#if defined(USE_DEM)
#include "Dem.h"
#endif
#if defined(USE_NVM)
#include "NvM.h"
#endif
#if defined(USE_COMM)
#include "ComM.h"
#include "ComM_EcuM.h"
#endif

//#define USE_LDEBUG_PRINTF
#include "debug.h"

#define ECUM_STR   "ECUM:"

#if defined(USE_LDEBUG_PRINTF)
char *GetMainStateAsString( EcuM_StateType state ) {
	char *str = NULL;

	switch(state) {
	case ECUM_STATE_APP_RUN:
		str = "ECUM_STATE_APP_RUN";
		break;
	case ECUM_STATE_SHUTDOWN:
		str = "ECUM_STATE_SHUTDOWN";
		break;
	case ECUM_STATE_WAKEUP:
		str = "ECUM_STATE_WAKEUP";
		break;
//	case ECUM_SUBSTATE_MASK:
//		str = "ECUM_SUBSTATE_MASK";
//		break;
	case ECUM_STATE_WAKEUP_WAKESLEEP:
		str = "ECUM_STATE_WAKEUP_WAKESLEEP";
		break;
	case ECUM_STATE_WAKEUP_ONE:
		str = "ECUM_STATE_WAKEUP_ONE";
		break;
	case ECUM_STATE_OFF:
		str = "ECUM_STATE_OFF";
		break;
	case ECUM_STATE_STARTUP:
		str = "ECUM_STATE_STARTUP";
		break;
	case ECUM_STATE_PREP_SHUTDOWN:
		str = "ECUM_STATE_PREP_SHUTDOWN";
		break;
	case ECUM_STATE_RUN:
		str = "ECUM_STATE_RUN";
		break;
	case ECUM_STATE_STARTUP_TWO:
		str = "ECUM_STATE_STARTUP_TWO";
		break;
	case ECUM_STATE_WAKEUP_TTII:
		str = "ECUM_STATE_WAKEUP_TTII";
		break;
	case ECUM_STATE_WAKEUP_VALIDATION:
		str = "ECUM_STATE_WAKEUP_VALIDATION";
		break;
	case ECUM_STATE_GO_SLEEP:
		str = "ECUM_STATE_GO_SLEEP";
		break;
	case ECUM_STATE_STARTUP_ONE:
		str = "ECUM_STATE_STARTUP_ONE";
		break;
	case ECUM_STATE_WAKEUP_TWO:
		str = "ECUM_STATE_WAKEUP_TWO";
		break;
	case ECUM_STATE_SLEEP:
		str = "ECUM_STATE_SLEEP";
		break;
	case ECUM_STATE_WAKEUP_REACTION:
		str = "ECUM_STATE_WAKEUP_REACTION";
		break;
	case ECUM_STATE_APP_POST_RUN:
		str = "ECUM_STATE_APP_POST_RUN";
		break;
	case ECUM_STATE_GO_OFF_TWO:
		str = "ECUM_STATE_GO_OFF_TWO";
		break;
	case ECUM_STATE_RESET:
		str = "ECUM_STATE_RESET";
		break;
	case ECUM_STATE_GO_OFF_ONE:
		str = "ECUM_STATE_GO_OFF_ONE";
		break;
	default:
		assert(0);
	}
	return str;
}

const char *GetWakeupReactionAsString( EcuM_WakeupReactionType reaction ) {
	const char *WakeupReactionAsString[] = {
			"ECUM_WKACT_RUN",
			"??????",
			"ECUM_WKACT_TTII",
			"ECUM_WKACT_SHUTDOWN",
	};
	return  WakeupReactionAsString[reaction];
}

#endif

static uint32 EcuM_World_run_state_timeout = 0;
#if defined(USE_NVM)
static uint32 EcuM_World_go_off_one_state_timeout = 0;
static NvM_RequestResultType writeAllResult;
#endif

static uint32 EcuM_World_go_sleep_state_timeout = 0;

#ifdef CFG_ECUM_USE_SERVICE_COMPONENT
/** @req EcuM2749 */
static Rte_ModeType_EcuM_Mode currentMode = RTE_MODE_EcuM_Mode_STARTUP;
#endif


void set_current_state(EcuM_StateType state) {
	imask_t irqMask = 0;

	/* Update the state */
	EcuM_World.current_state = state;

#ifdef CFG_ECUM_USE_SERVICE_COMPONENT
	Rte_ModeType_EcuM_Mode newMode = currentMode;
	switch( state ) {
	case ECUM_STATE_WAKEUP:
	case ECUM_STATE_WAKEUP_ONE:
	case ECUM_STATE_WAKEUP_VALIDATION:
	case ECUM_STATE_WAKEUP_REACTION:
	case ECUM_STATE_WAKEUP_TWO:
	case ECUM_STATE_SLEEP:
	case ECUM_STATE_SHUTDOWN:
		newMode = RTE_MODE_EcuM_Mode_SLEEP;
		break;
	case ECUM_STATE_GO_SLEEP:
		if( EcuM_World.shutdown_target == ECUM_STATE_SLEEP ) {
			newMode = RTE_MODE_EcuM_Mode_SLEEP; /** @req EcuM2752 */
		}
		break;
	case ECUM_STATE_GO_OFF_ONE:
	case ECUM_STATE_GO_OFF_TWO:
		newMode = RTE_MODE_EcuM_Mode_SHUTDOWN;
		break;
	case ECUM_STATE_WAKEUP_TTII:
		if( EcuM_World.shutdown_target == ECUM_STATE_SLEEP ) {
			newMode = RTE_MODE_EcuM_Mode_WAKE_SLEEP; /** @req EcuM2752 */
		}
		break;
	case ECUM_STATE_PREP_SHUTDOWN:
	case ECUM_STATE_APP_POST_RUN: /* Assuming this is same as RUN_III */
		newMode = RTE_MODE_EcuM_Mode_POST_RUN;
		break;
	case ECUM_STATE_APP_RUN: /* Assuming this is same as RUN_II */
		Irq_Save(irqMask);
		newMode = RTE_MODE_EcuM_Mode_RUN;
		/* We have a configurable minimum time (EcuMRunMinimumDuration) we have to stay in RUN state  */
		/* This should not be done in EcuM_enter_run_mode() because RTE_MODE_EcuM_Mode_RUN & EcuM_World_run_state_timeout  are
		 * not set at the same time and this might lead to immediate timeout
		 */
		EcuM_World_run_state_timeout = EcuM_World.config->EcuMRunMinimumDuration / ECUM_MAIN_FUNCTION_PERIOD; /** @req EcuM2310 */
		Irq_Restore(irqMask);
		break;
	case ECUM_STATE_STARTUP_TWO:
		newMode = RTE_MODE_EcuM_Mode_STARTUP;
		break;
	default:
		/* Do nothing */
		break;
	}

	if( newMode != currentMode ) {
		currentMode = newMode;
		Rte_Switch_EcuM_CurrentMode_currentMode(currentMode); /** @req EcuM2750 */
	}
}
#else
	if (ECUM_STATE_APP_RUN == state) {
		Irq_Save(irqMask);
		/* We have a configurable minimum time (EcuMRunMinimumDuration) we have to stay in RUN state  */
		EcuM_World_run_state_timeout = EcuM_World.config->EcuMRunMinimumDuration / ECUM_MAIN_FUNCTION_PERIOD; /** @req EcuM2310 */
		Irq_Restore(irqMask);
	}
}
#endif


/**
 * RUN II entry
 * - Called from EcuM_StartupTwo()
 * - Called from
 *
 *
 */
void EcuM_enter_run_mode(void){
	set_current_state(ECUM_STATE_APP_RUN);
	EcuM_OnEnterRun(); /** @req EcuM2308 */

#if defined(USE_WDGM)
	/* This seems strange, should be in FW instead */
	if( EcuM_World.config->EcuMWdgMConfig != NULL ) {
		WdgM_SetMode(EcuM_World.config->EcuMWdgMConfig->EcuMWdgMRunMode);
	}
#endif

#if defined(USE_COMM)
	/*
	 * Loop over all channels that have requested run,
	 * ie EcuM_ComM_RequestRUN()
	 */
	{
		uint32 cMask = EcuM_World.run_comm_requests;
		uint8  channel;

		for (; cMask; cMask &= ~(1ul << channel)) {
			channel = ilog2(cMask);
			DEBUG_ECUM_CALLOUT_W_ARG("ComM_EcuM_RunModeIndication","%ld",(uint32)channel);
			ComM_EcuM_RunModeIndication(channel);
		}
	}
#endif

}


//--------- Local functions ------------------------------------------------------------------------------------------------



/**
 * Enter GO SLEEP state ( soon in state ECUM_STATE_GO_SLEEP)
 */
static inline void enter_go_sleep_mode(void){
	EcuM_WakeupSourceType wakeupSource;
	set_current_state(ECUM_STATE_GO_SLEEP);

	DEBUG_ECUM_CALLOUT( ECUM_STR "EcuM_OnGoSleep");
	EcuM_OnGoSleep();

#if defined(USE_NVM)
	NvM_WriteAll();

	/* Start timer */
	EcuM_World_go_sleep_state_timeout = EcuM_World.config->EcuMNvramWriteAllTimeout / ECUM_MAIN_FUNCTION_PERIOD;

	wakeupSource = EcuM_GetPendingWakeupEvents();
#else
	wakeupSource = EcuM_GetPendingWakeupEvents();
#endif
}

/**
  In GO SLEEP state (in state ECUM_STATE_GO_SLEEP)
 */
static void in_state_goSleep( void ) {

	/* We only wait for NvM_WriteAll() for so long */
	if (EcuM_World_go_sleep_state_timeout){
		EcuM_World_go_sleep_state_timeout--;
#if defined(USE_NVM)
		{
		  NvM_RequestResultType nvmResult;
		  NvM_GetErrorStatus(0, &nvmResult);

      if( nvmResult != NVM_REQ_PENDING) {
        /* Done or something is wrong...continue */
        EcuM_World_go_sleep_state_timeout = 0;
      }
		}
#endif
	}

	if( (EcuM_World_go_sleep_state_timeout == 0) ) {
		/*
		 * We should go to sleep , enable source that should wake us
		 * */
		uint32 cMask;
		uint8  source;
		const EcuM_SleepModeType *sleepModePtr;

		/* Get the current sleep mode */

		sleepModePtr = &EcuM_World.config->EcuMSleepModeConfig[EcuM_World.sleep_mode];

		cMask = sleepModePtr->EcuMWakeupSourceMask;

		/* Loop over the WKSOURCE for this sleep mode */
		for (; cMask; cMask &= ~(1ul << source)) {
			source = ilog2(cMask);
			/* @req 3.1.5/ECUM2389 */
//			DEBUG_ECUM_CALLOUT_W_ARG("EcuM_EnableWakeupSources","0x%lx",(1ul<< source));
			EcuM_EnableWakeupSources( 1<< source );

		}

#if defined(USE_WDGM)
		if( EcuM_World.config->EcuMWdgMConfig != NULL &&
		    sleepModePtr->EcuMSleepModeWdgMMode != ECUM_SLEEP_MODE_WDGM_MODE_ILL )
		{
        DEBUG_ECUM_CALLOUT_W_ARG("WdgM_SetMode","%d",sleepModePtr->EcuMSleepModeWdgMMode);
        WdgM_SetMode(sleepModePtr->EcuMSleepModeWdgMMode);
		}
#endif

		/* Let no one else run */
		GetResource(RES_SCHEDULER);
		set_current_state(ECUM_STATE_SLEEP);

	} else if( EcuM_GetPendingWakeupEvents() != 0 ) {
		/* We have pending wakeup events, need to startup again */
#if defined(USE_NVM)
		NvM_CancelWriteAll();
		set_current_state(ECUM_STATE_SLEEP);
#endif
	}
}


/**
 * In "Sleep Sequence I"  (in state ECUM_STATE_SLEEP)
 */
static void in_state_sleep ( void ) {
	const EcuM_SleepModeType *sleepModePtr;
	sleepModePtr = &EcuM_World.config->EcuMSleepModeConfig[EcuM_World.sleep_mode];

	DEBUG_ECUM_CALLOUT( ECUM_STR "EcuM_GenerateRamHash");
	EcuM_GenerateRamHash();

	DEBUG_ECUM_CALLOUT_W_ARG("Mcu_SetMode","%d",sleepModePtr->EcuMSleepModeMcuMode);
	Mcu_SetMode(sleepModePtr->EcuMSleepModeMcuMode);

	/* @req 3.1.5/ECUM2863 */
	if( EcuM_CheckRamHash() == 0) {
#if defined(USE_DEM)
		//
		EcuM_ErrorHook(ECUM_E_RAM_CHECK_FAILED);
#endif
	}

	set_current_state(ECUM_STATE_WAKEUP_ONE);
}

static inline void enter_go_off_one_mode(void){
	set_current_state(ECUM_STATE_GO_OFF_ONE);
	EcuM_OnGoOffOne();

#if defined(USE_COMM)
	ComM_DeInit();
#endif

#if defined(USE_NVM)

	// Start NvM_WriteAll and timeout timer
	NvM_WriteAll();

	EcuM_World_go_off_one_state_timeout = EcuM_World.config->EcuMNvramWriteAllTimeout / ECUM_MAIN_FUNCTION_PERIOD;
#endif
}


static inline boolean hasRunRequests(void){
	uint32 result = EcuM_World.run_requests;

#if defined(USE_COMM)
	result |= EcuM_World.run_comm_requests;
#endif

	return (result != 0);
}

static inline boolean hasPostRunRequests(void){
	return (EcuM_World.postrun_requests != 0);
}


/**
 * RUN II Loop (in state ECUM_STATE_APP_RUN)
 * - The entry to RUN II is done in
 */
static inline void in_state_appRun(void){
	if (EcuM_World_run_state_timeout){
		EcuM_World_run_state_timeout--;
		LDEBUG_PRINTF( ECUM_STR "RUN Timeout=%ld\n",EcuM_World_run_state_timeout);
	}

	if ((!hasRunRequests()) && (EcuM_World_run_state_timeout == 0)){
		DEBUG_ECUM_CALLOUT( ECUM_STR "EcuM_OnExitRun");
		EcuM_OnExitRun();	/** @req EcuM2865 */

#if defined(USE_WDGM)
		if( EcuM_World.config->EcuMWdgMConfig != NULL ) {
			WdgM_SetMode(EcuM_World.config->EcuMWdgMConfig->EcuMWdgMPostRunMode);
		}
#endif

		set_current_state(ECUM_STATE_APP_POST_RUN);/** @req EcuM2865 */
	}
}


/**
 * RUN III states (in state ECUM_STATE_APP_POST_RUN)
 */
static inline void in_state_appPostRun(void){

	/* @req 3.1.5/ECUM2866 */
	if (hasRunRequests()){
		/* We have run requests, return to RUN II */
		DEBUG_ECUM_CALLOUT( ECUM_STR "EcuM_enter_run_mode");
		EcuM_enter_run_mode();

	} else if (!hasPostRunRequests()){
		DEBUG_ECUM_CALLOUT( ECUM_STR "EcuM_OnExitPostRun");
		EcuM_OnExitPostRun(); /** @req EcuM2761 */
		set_current_state(ECUM_STATE_PREP_SHUTDOWN);/** @req EcuM2761 */
	} else {
		/* TODO: We have postrun requests */
	}
}


/**
 * PREP SHUTDOWN state (in state ECUM_STATE_PREP_SHUTDOWN)
 */
static inline void in_state_prepShutdown(void){

	// TODO: The specification does not state what events to clear
	EcuM_ClearWakeupEvent(ECUM_WKSTATUS_NONE);

	DEBUG_ECUM_CALLOUT( ECUM_STR "EcuM_OnPrepShutdown");
	EcuM_OnPrepShutdown();

#if defined(USE_DEM)
	// DEM shutdown
	Dem_Shutdown();
#endif

	// Switch shutdown mode
	switch(EcuM_World.shutdown_target){
		//If in state Off or Reset go into Go_Off_One:
		case ECUM_STATE_OFF:
		case ECUM_STATE_RESET:
			enter_go_off_one_mode();
			break;
		case ECUM_STATE_SLEEP:
			enter_go_sleep_mode();
			break;
		default:
			//TODO: Report error.
			break;
	}
}

static inline void in_state_goOffOne(void){
#if defined(USE_NVM)
		if (EcuM_World_go_off_one_state_timeout){
			EcuM_World_go_off_one_state_timeout--;
		}
		// Wait for the NVM job (NvmWriteAll) to terminate
		NvM_GetErrorStatus(0, &writeAllResult);
		if ((writeAllResult != NVM_REQ_PENDING) || (EcuM_World_go_off_one_state_timeout == 0)){

#if defined(USE_WDGM)
			if( EcuM_World.config->EcuMWdgMConfig != NULL ) {
				DEBUG_ECUM_CALLOUT( ECUM_STR "WdgM_SetMode");
				WdgM_SetMode(EcuM_World.config->EcuMWdgMConfig->EcuMWdgMShutdownMode);
			}
#endif
			ShutdownOS(E_OK);
		}
#else

#if defined(USE_WDGM)
		if( EcuM_World.config->EcuMWdgMConfig != NULL ) {
			DEBUG_ECUM_CALLOUT( ECUM_STR "WdgM_SetMode");
			WdgM_SetMode(EcuM_World.config->EcuMWdgMConfig->EcuMWdgMShutdownMode);
		}
#endif
		ShutdownOS(E_OK);
#endif
}


//----- MAIN -----------------------------------------------------------------------------------------------------------------
void EcuM_MainFunction(void) {
	EcuM_WakeupSourceType wMask;
	static uint32 validationMask;
	static uint32 validationMaxTime;
	static uint32 pendingWkupMask = 0;

	VALIDATE_NO_RV(EcuM_World.initiated, ECUM_MAINFUNCTION_ID, ECUM_E_NOT_INITIATED);

#if defined(USE_LDEBUG_PRINTF)
	{
	  static EcuM_StateType oldEcuMState = 0xff;
	  if( oldEcuMState != EcuM_World.current_state) {
	    DEBUG_ECUM_STATE(EcuM_World.current_state);
	    oldEcuMState = EcuM_World.current_state;
	  }
	}
#endif

	switch (EcuM_World.current_state) {
	case ECUM_STATE_APP_RUN:
		/* RUN II state */
		in_state_appRun();
		break;

	case ECUM_STATE_APP_POST_RUN:
		/* RUN III state */
		in_state_appPostRun();
		break;
	case ECUM_STATE_PREP_SHUTDOWN:
		in_state_prepShutdown();
		break;

	case ECUM_STATE_GO_OFF_ONE:
		in_state_goOffOne();
		break;

	case ECUM_STATE_GO_SLEEP:
	{

	  /* 4 cases:
	   * 1. Wait for the NvM_WriteAll() - Stay in state
	   * 2. Timeout on NvM_WriteAll()   - go to ECUM_STATE_SLEEP (Scheduler is locked)
	   * 3. NvM_WriteAll() is done      - go to ECUM_STATE_SLEEP (Scheduler is locked)
	   * 4. Run request                 - Call NvM_CancelAll() and go to ECUM_STATE_WAKEUP_ONE.
	   */

	  in_state_goSleep();

	  if( EcuM_World.current_state != ECUM_STATE_SLEEP ) {
	    break;
	  }

		/* Flow Through, Scheduler is Locked */
	}
	case ECUM_STATE_SLEEP:
		in_state_sleep();
		/* Flow Through, Scheduler is Locked */

	case ECUM_STATE_WAKEUP_ONE: {
		DEBUG_ECUM_STATE(EcuM_World.current_state);

		/* TODO: we must have a normal RUN mode.. can't find any
		 * in the A3.1.5 spec. */
		Mcu_SetMode(MCU_MODE_NORMAL);
#if defined(USE_WDGM)
		if( EcuM_World.config->EcuMWdgMConfig != NULL ) {
			WdgM_SetMode(EcuM_World.config->EcuMWdgMConfig->EcuMWdgMWakeupMode);
		}
#endif

		wMask = EcuM_GetPendingWakeupEvents();

		DEBUG_ECUM_CALLOUT_W_ARG("EcuM_DisableWakeupSources", "0x%lx", (uint32) wMask);
		EcuM_DisableWakeupSources(wMask);

		EcuM_AL_DriverRestart();

		EcuM_World.killAllRequest = false;	/* Enable run request again */

		ReleaseResource(RES_SCHEDULER);

		validationMask = 0;
		validationMaxTime = 0;
		const EcuM_WakeupSourceConfigType *wkupCfgPtr;

		set_current_state(ECUM_STATE_WAKEUP_VALIDATION);

		/*-------------- ECUM_STATE_WAKEUP_VALIDATION -------------------- */

		DEBUG_ECUM_CALLOUT(ECUM_STR "EcuM_GetPendingWakeupEvents");
		pendingWkupMask = EcuM_GetPendingWakeupEvents();

		DEBUG_ECUM_CALLOUT_W_ARG("EcuM_StartWakeupSources", "0x%lx",(uint32) pendingWkupMask);

		EcuM_StartWakeupSources(pendingWkupMask);
		EcuM_World.validationTimer = 0;

#if 0
		/* Validate Pre-defined events right away */
		wMask = (pendingWkupMask & (ECUM_WKSOURCE_POWER|ECUM_WKSOURCE_RESET | ECUM_WKSOURCE_INTERNAL_RESET |
						ECUM_WKSOURCE_INTERNAL_WDG | ECUM_WKSOURCE_EXTERNAL_WDG));

		if( wMask != 0 ) {
			EcuM_ValidateWakeupEvent(wMask);
		}
#endif

		/* Calculate the validation timing , if any*/
		for (int i = 0; i < ECUM_WKSOURCE_USER_CNT; i++) {
			wkupCfgPtr = &EcuM_World.config->EcuMWakeupSourceConfig[i];

			/* Can't validate something that is not pending */
			if (wMask & wkupCfgPtr->EcuMWakeupSourceId) {

				/* No validation timeout == ECUM_VALIDATION_TIMEOUT_ILL */
				if ((wkupCfgPtr->EcuMValidationTimeout
						!= ECUM_VALIDATION_TIMEOUT_ILL)
						&& (wMask & wkupCfgPtr->EcuMWakeupSourceId)) {
					/* Build a mask with the sources that need validation */
					validationMask |= wkupCfgPtr->EcuMWakeupSourceId;
					/* Use one validation timeout, take the longest */
					validationMaxTime =
							MAX( wkupCfgPtr->EcuMValidationTimeout / ECUM_MAIN_FUNCTION_PERIOD,
							validationMaxTime);
					EcuM_World.validationTimer = validationMaxTime;
				} else {
					LDEBUG_PRINTF(ECUM_STR "No Validation for event:0x%lx\n",
							(uint32) wkupCfgPtr->EcuMWakeupSourceId);

					/* Validate right away */
					EcuM_ValidateWakeupEvent(wkupCfgPtr->EcuMWakeupSourceId);
				}
			}
		}

		break;
	}

	case ECUM_STATE_WAKEUP_VALIDATION: {
		/* !req 3.1.5/EcuM2566 */
		boolean done = 0;

		if (EcuM_World.validationTimer != 0) {
			/*
			 * Call EcuM_CheckValidation() while all events have not been validated and
			 * timeout have not expired. The call to EcuM_CheckValidation(..) triggers a call
			 * to EcuM_ValidateWakeupEvent(..) from the driver when validated.
			 */

			/* Check validation for the events that do not match, ie not yet validated */
			DEBUG_ECUM_CALLOUT_W_ARG(
					"EcuM_CheckValidation",
					"0x%lx",
					(uint32)(EcuM_GetValidatedWakeupEvents() ^ pendingWkupMask));

			EcuM_CheckValidation( EcuM_GetValidatedWakeupEvents() ^ pendingWkupMask);

			if (0 == (EcuM_GetValidatedWakeupEvents() ^ pendingWkupMask)) {
				/* All events have been validated */
				done = 1;
			} else {
				LDEBUG_PRINTF( ECUM_STR "  Awaiting validation for mask: pending=%lx, expected=%lx\n",
								pendingWkupMask, validationMask);
				LDEBUG_PRINTF(ECUM_STR "  Validation Timer            : %lu\n", EcuM_World.validationTimer);
			}

		} else {
			uint32 notValidatedMask = EcuM_GetValidatedWakeupEvents() ^ pendingWkupMask;

			/* Stop wakeupSources that are not validated */
			if (notValidatedMask) {
				DEBUG_ECUM_CALLOUT_W_ARG("EcuM_StopWakeupSources", "0x%lx",
						(uint32) notValidatedMask);
				EcuM_StopWakeupSources(notValidatedMask);
			}
			done = 1;
		}

		/* @req 3.1.5/EcuM2710 */
		if (EcuM_World.validationTimer) {
			EcuM_World.validationTimer--;
		}

		if (done) {
#if defined(USE_COMM)
			const EcuM_WakeupSourceConfigType *wkupCfgPtr;
			uint32 validated = EcuM_GetValidatedWakeupEvents();

			for(int i=0;i<ECUM_WKSOURCE_USER_CNT;i++) {
				wkupCfgPtr = &EcuM_World.config->EcuMWakeupSourceConfig[i];

				/* Call wakeup indication for all validated events with a channel assigned */
				if ( (wkupCfgPtr->EcuMComMChannel != ECUM_COMM_CHANNEL_ILL) &&
						wkupCfgPtr->EcuMWakeupSourceId & validated ) {
					ComM_EcuM_WakeUpIndication(wkupCfgPtr->EcuMComMChannel);
				}
			}
#endif

			set_current_state(ECUM_STATE_WAKEUP_REACTION);
		}
		break;
	}

	case ECUM_STATE_WAKEUP_REACTION: {
		/*
		 * At this stage we want to know how to react to the wakeup, e.g. go
		 * back to RUN or SHUTDOWN, etc.
		 */
		EcuM_WakeupReactionType wReaction;

		wMask = EcuM_GetValidatedWakeupEvents();
		LDEBUG_PRINTF(ECUM_STR "EcuM_GetValidatedWakeupEvents() : %x\n", wMask);

		/* TODO: We have skipped the TTII timer here */

		/* If the wakeup mask here is != 0 we have a validated wakeup event ->
		 * go back to RUN */
		wReaction = (0 == wMask) ? ECUM_WKACT_SHUTDOWN : ECUM_WKACT_RUN;
		wReaction = EcuM_OnWakeupReaction(wReaction);

		LDEBUG_PRINTF(ECUM_STR "Wakeup Reaction: %s\n",
				GetWakeupReactionAsString(wReaction));
		if (wReaction == ECUM_WKACT_RUN) {
			set_current_state(ECUM_STATE_WAKEUP_TWO);
		} else {
			/* From figure 28 it seems that we should go to SHUTDOWN/GO SLEEP) again from wakeup
			 * not going up to RUN/RUN II state again. */
			set_current_state(ECUM_STATE_GO_SLEEP);
		}
		break;
	}

	case ECUM_STATE_WAKEUP_TWO:
#if defined(USE_DEM)
		Dem_Init();
#endif
		EcuM_enter_run_mode();
//		set_current_state(ECUM_STATE_APP_RUN);
		break;

	default:
		//TODO: Report error.
		break;
	}
}
