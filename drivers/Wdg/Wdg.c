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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */
/** @tagSettings DEFAULT_ARCHITECTURE=RH850F1H */

/* General Wdg module requirements */
/* @req SWS_Wdg_00086 Preprocessor check of import include files. */
/* @req SWS_Wdg_00031 The Wdg module shall not implement an interface for de-initialization/shutdown. */
/* @req SWS_Wdg_00161 Access to internal watchdog HW. */
/* @req SWS_Wdg_00105 Imported types. */
/* @req SWS_Wdg_00159 Config variant VARIANT-POST-BUILD. */

#include "Wdg.h"
#include "Wdg_Internal.h"
#ifdef CFG_TMS570
#include "os_trap.h"
#endif

/* Declared volatile since depending on the requirements supported (see SWS_Wdg_00035 and 52) it might be accessed from an ISR. */
/* @req SWS_Wdg_00152 */
volatile WdgInternalState Wdg_State = WDG_UNINIT;

const Wdg_ConfigType* Wdg_ConfigPtr = NULL;

/* @req SWS_Wdg_00153 */
volatile uint16 Wdg_TriggerCounter = 0;

/* @req SWS_Wdg_00154 */
WdgIf_ModeType Wdg_Mode = WDGIF_OFF_MODE;

/* @req SWS_Wdg_00106 */
/* @req SWS_Wdg_00171 */
void Wdg_Init(const Wdg_ConfigType* ConfigPtr)
{
    /* This requirement does not exist anymore in 4.2.1 but keeping it anyway. It was called WDG089 in 4.0. */
    /* @req SWS_Wdg_00089 */
    VALIDATE((NULL != ConfigPtr), WDG_INIT_SERVICE_ID, WDG_E_PARAM_POINTER);

    /* @req SWS_Wdg_00090 */
#if (WDG_DEV_ERROR_DETECT == STD_ON)
    /*lint -save -e9007 Note: Side effects on right hand of logical operator ||. OK because 2nd result is not needed if first fails. */
    if ((Wdg_Hw_ValidateTimeout(ConfigPtr->Wdg_ModeConfig->WdgSettingsFast) != E_OK) ||
            (Wdg_Hw_ValidateTimeout(ConfigPtr->Wdg_ModeConfig->WdgSettingsSlow) != E_OK))
    {
        VALIDATE_FAIL(WDG_INIT_SERVICE_ID, WDG_E_PARAM_CONFIG);
    }
    /*lint -restore -e9007 */
#endif

    /* @req SWS_Wdg_00051 */
    Wdg_ConfigPtr = ConfigPtr;

    /* @req SWS_Wdg_00001 */
    /* @req SWS_Wdg_00100 */
    /* @req SWS_Wdg_00101 */
    /* @req SWS_Wdg_00173 */
    if (Wdg_Hw_Init(Wdg_ConfigPtr) == E_OK)
    {
        /* @req SWS_Wdg_00019 */
        Wdg_State = WDG_IDLE;
    }
}

/* @req SWS_Wdg_00107 */
Std_ReturnType Wdg_SetMode(WdgIf_ModeType Mode)
{
    Std_ReturnType ret;

#if (WDG_DEV_ERROR_DETECT == STD_ON)

    /* @req SWS_Wdg_00017 */
    if (WDG_IDLE != Wdg_State)
    {
        (void) Det_ReportError(WDG_MODULE_ID, 0, WDG_SET_MODE_SERVICE_ID, WDG_E_DRIVER_STATE);

        return E_NOT_OK;
    }

    /* @req SWS_Wdg_00091 */
    if ((WDGIF_OFF_MODE != Mode) && (WDGIF_FAST_MODE != Mode) && (WDGIF_SLOW_MODE != Mode))
    {
        (void) Det_ReportError(WDG_MODULE_ID, 0, WDG_SET_MODE_SERVICE_ID, WDG_E_PARAM_MODE);

        return E_NOT_OK;
    }

    /* @req SWS_Wdg_00018 */
    Wdg_State = WDG_BUSY;

#endif /* WDG_DEV_ERROR_DETECT */

    /* @req SWS_Wdg_00016 */
    /* @req SWS_Wdg_00160 */
    /* @req SWS_Wdg_00145 */
    /* @req SWS_Wdg_00092 */
    ret = Wdg_Hw_SetMode(Mode, Wdg_ConfigPtr, WDG_SET_MODE_SERVICE_ID);

#if (WDG_DEV_ERROR_DETECT == STD_ON)
    /* @req SWS_Wdg_00018 */
    Wdg_State = WDG_IDLE;
#endif /* WDG_DEV_ERROR_DETECT */

    /* @req SWS_Wdg_00103 */
    return ret;
}

/* @req SWS_Wdg_00155 */
void Wdg_SetTriggerCondition(uint16 timeout)
{
    /* @req SWS_Wdg_00136 */
    /* @req SWS_Wdg_00138 */
    /* @req SWS_Wdg_00139 */
    /* @req SWS_Wdg_00140 */
    /* @req SWS_Wdg_00146 */
#ifdef CFG_TMS570
        OS_TRAP_Wdg_Hw_SetTriggerCondition(timeout);
#else
        Wdg_Hw_SetTriggerCondition(timeout);
#endif
}

#if (STD_ON == WDG_VERSION_INFO_API)
/* @req SWS_Wdg_00109 */
void Wdg_GetVersionInfo(Std_VersionInfoType* versioninfo)
{
    /* @req SWS_Wdg_00174 */
    VALIDATE((NULL != versioninfo), WDG_GET_VERSION_INFO_SERVICE_ID,WDG_E_PARAM_POINTER);
    versioninfo->vendorID = WDG_VENDOR_ID;
    versioninfo->moduleID = WDG_MODULE_ID;
    versioninfo->sw_major_version = WDG_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = WDG_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = WDG_SW_PATCH_VERSION;
}
#endif
