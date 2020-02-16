/*
 * Wdg.c
 *
 *  Created on: Nov 21, 2014
 *      Author: avenir
 */

#include "Wdg.h"

void Wdg_Init (const Wdg_ConfigType* ConfigPtr) {}

/**
 * Set the watchdog timer.
 *
 * @param msecs				----- milliseconds to wait until the system resets itself.
 */
void Wdg_SetTriggerCondition(uint32 msecs)
{
    PM_WDOG = PM_PASSWORD | (msecs * PM_WDOG_UNITS_PER_MILLISECOND);
    uint32 cur = PM_RSTC;
    PM_RSTC = PM_PASSWORD | (cur & PM_RSTC_WRCFG_CLR) | PM_RSTC_WRCFG_FULL_RESET;
}

Std_ReturnType Wdg_SetMode (WdgIf_ModeType Mode)
{
	return E_OK;
}





