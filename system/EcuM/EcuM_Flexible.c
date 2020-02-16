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
#include "Std_Types.h"
#include "EcuM.h"
#include "EcuM_Generated_Types.h"
#include "EcuM_Internals.h"
#include "EcuM_Cbk.h"

/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/

extern EcuM_GlobalType EcuM_World;

/* ----------------------------[private functions]---------------------------*/

static void goSleepSequence(void) {
    uint32 cMask;
    uint8 source;
    const EcuM_SleepModeType *sleepModePtr;

    /* Get the current sleep mode */
    sleepModePtr = &EcuM_World.config->EcuMSleepModeConfig[EcuM_World.sleep_mode];

    cMask = sleepModePtr->EcuMWakeupSourceMask;

    /* Loop over the WKSOURCE for this sleep mode */
    for (; cMask; cMask &= ~(1ul << source)) {
        source = ilog2(cMask);
        EcuM_EnableWakeupSources(1 << source);
        BswM_EcuM_CurrentWakeup(source, ECUM_WKSTATUS_NONE);
    }

    GetResource(RES_SCHEDULER);
}

static void haltSequence(void) {
    const EcuM_SleepModeType *sleepModePtr;

    EcuM_GenerateRamHash();

    sleepModePtr =
            &EcuM_World.config->EcuMSleepModeConfig[EcuM_World.sleep_mode];
    Mcu_SetMode(sleepModePtr->EcuMSleepModeMcuMode);

    if (EcuM_CheckRamHash() == 0) {
#if defined(USE_DEM)
        EcuM_ErrorHook(EcuM_World.config->EcuMDemRamCheckFailedEventId);
#endif
    }
}

static void pollingSequence(void) {
    const EcuM_WakeupSourceConfigType *wkupCfgPtr;
    bool noPendingWakeupEvents = true;
    uint8 source = 0;

    /* @req EcuM2962 */
    /* @req EcuM3020 */
    while (noPendingWakeupEvents) {
        EcuM_SleepActivity();
        for (int i = 0; i < ECUM_WKSOURCE_USER_CNT; i++) {
            wkupCfgPtr = &EcuM_World.config->EcuMWakeupSourceConfig[i];
            EcuM_CheckWakeup(wkupCfgPtr->EcuMWakeupSourceId);
            if (EcuM_GetPendingWakeupEvents() != 0) {
                noPendingWakeupEvents = false;
                source |= wkupCfgPtr->EcuMWakeupSourceId;
            }
        }
    }

    BswM_EcuM_CurrentWakeup(source, ECUM_WKSTATUS_NONE);
}

static void wakeupRestartSequence(void) {
    EcuM_WakeupSourceType wMask = 0;

    Mcu_SetMode(MCU_MODE_NORMAL);

    wMask = EcuM_GetPendingWakeupEvents();

    EcuM_DisableWakeupSources(wMask);

    EcuM_AL_DriverRestart(EcuM_World.config);

    ReleaseResource(RES_SCHEDULER);
}

#if ECUM_DEFENSIVE_BEHAVIOR == STD_ON
static boolean isCallerAllowed(uint16 caller) {
    for (int i = 0; i < ECUM_GO_DOWN_ALLOWED_USERS_CNT; i++) {
        if (EcuM_World.config->EcuMGoDownAllowedUsers[i] == caller) return true;
    }
    return false;
}
#endif

/* ----------------------------[public functions]----------------------------*/

void EcuM_StartupTwo(void) {
#if defined(USE_SCHM)
    SchM_Init();
#endif

#if defined(USE_BSWM)
    BswM_Init(EcuM_World.config->BswMConfig);
#endif
}

/* @req EcuM4050 */
Std_ReturnType EcuM_SelectShutdownCause(EcuM_ShutdownCauseType shutdownCause) {
    VALIDATE_RV(EcuM_World.initiated, ECUM_SELECTSHUTDOWNCAUSE_ID, ECUM_E_UNINIT, E_NOT_OK);

    EcuM_World.shutdown_cause = shutdownCause;

    return E_OK ;
}

/* @req EcuM4051 */
Std_ReturnType EcuM_GetShutdownCause(EcuM_ShutdownCauseType *shutdownCause) {
    VALIDATE_RV(EcuM_World.initiated, ECUM_GETSHUTDOWNCAUSE_ID, ECUM_E_UNINIT, E_NOT_OK);

    *shutdownCause = EcuM_World.shutdown_cause;

    return E_OK ;
}

/* @req EcuM4046 */
Std_ReturnType EcuM_GoDown(uint16 caller) {
    VALIDATE_RV(EcuM_World.initiated, ECUM_GODOWN_ID, ECUM_E_UNINIT, E_NOT_OK);

    EcuM_WakeupSourceType wakeupEvents;
    Std_ReturnType rv = E_OK;
    /* !req EcuM4047 */
#if ECUM_DEFENSIVE_BEHAVIOR == STD_ON
    if (!isCallerAllowed(caller)) {
#if defined(USE_DEM)
        //TODO: Report ECUM_E_IMPROPER_CALLER to DEM when we have the 4.1 config
#endif
        return E_NOT_OK;
    }
#endif

#if defined(USE_BSWM)
    BswM_Deinit();
#endif

#if defined(USE_SCHM)
    SchM_Deinit();
#endif

    wakeupEvents = EcuM_GetPendingWakeupEvents();

    if (wakeupEvents != 0) {
        rv = EcuM_SelectShutdownTarget(ECUM_STATE_RESET, 0);
    }

    /* @req EcuM2952 */
    ShutdownOS(E_OK);

    return rv;
}

/* @req EcuM4048 */
Std_ReturnType EcuM_GoHalt(void) {
    VALIDATE_RV(EcuM_World.initiated, ECUM_GOHALT_ID, ECUM_E_UNINIT, E_NOT_OK);

    goSleepSequence();

    haltSequence();

    wakeupRestartSequence();

    return E_OK ;
}

/* @req EcuM4049 */
Std_ReturnType EcuM_GoPoll(void) {
    VALIDATE_RV(EcuM_World.initiated, ECUM_GOPOLL_ID, ECUM_E_UNINIT, E_NOT_OK);

    goSleepSequence();

    pollingSequence();

    wakeupRestartSequence();

    return E_OK ;
}

void EcuM_MainFunction(void) {
    static uint32 pendingWkupMask = 0;
    static boolean validationOngoing = false;
    static uint32 validationMaxTime;

    if (!validationOngoing) {
        pendingWkupMask = EcuM_GetPendingWakeupEvents();

        EcuM_StartWakeupSources(pendingWkupMask);
        EcuM_World.validationTimer = 0;

        /* Calculate the validation timing , if any*/

        /* @req EcuM2494 */
        /* @req EcuM2479 */
        const EcuM_WakeupSourceConfigType *wkupCfgPtr;

        for (int i = 0; i < ECUM_WKSOURCE_USER_CNT; i++) {
            wkupCfgPtr = &EcuM_World.config->EcuMWakeupSourceConfig[i];

            /* Can't validate something that is not pending */
            if (pendingWkupMask & wkupCfgPtr->EcuMWakeupSourceId) {

                /* No validation timeout == ECUM_VALIDATION_TIMEOUT_ILL */
                if (wkupCfgPtr->EcuMValidationTimeout
                        != ECUM_VALIDATION_TIMEOUT_ILL) {
                    /* Use one validation timeout, take the longest */
                    validationMaxTime =
                        MAX( wkupCfgPtr->EcuMValidationTimeout / ECUM_MAIN_FUNCTION_PERIOD,
                             validationMaxTime);
                    EcuM_World.validationTimer = validationMaxTime;
                }
                else {
                    /* Validate right away */
                    /* @req EcuM2976 */
                    EcuM_ValidateWakeupEvent(wkupCfgPtr->EcuMWakeupSourceId);
                }
            }
        }

        validationOngoing = true;
    }

    if (EcuM_World.validationTimer != 0) {
        /*
         * Call EcuM_CheckValidation() while all events have not been validated and
         * timeout have not expired. The call to EcuM_CheckValidation(..) triggers a call
         * to EcuM_ValidateWakeupEvent(..) from the driver when validated.
         */

        /* Check validation for the events that do not match, ie not yet validated */
        EcuM_CheckValidation(EcuM_GetValidatedWakeupEvents() ^ pendingWkupMask);
        /* !req EcuM2495*/

        if (0 == (EcuM_GetValidatedWakeupEvents() ^ pendingWkupMask)) {
            /* All events have been validated */
            validationOngoing = false;
        }
    }
    else {
        uint32 notValidatedMask = EcuM_GetValidatedWakeupEvents()
                ^ pendingWkupMask;
        /* Stop wakeupSources that are not validated */
        if (notValidatedMask) {
            EcuM_StopWakeupSources(notValidatedMask);
            BswM_EcuM_CurrentWakeup((EcuM_WakeupSourceType)notValidatedMask, ECUM_WKSTATUS_EXPIRED);
        }
        validationOngoing = false;
    }

    /* @req 3.1.5/EcuM2710 */
    if (EcuM_World.validationTimer) {
        EcuM_World.validationTimer--;
    }
}
