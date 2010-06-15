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
#include "EcuM_Internals.h"

void EcuM_MainFunction(void)
{
#if (ECUM_DEV_ERROR_DETECT == STD_ON)
	if (!internal_data->initiated)
	{
		Det_ReportError(MODULE_ID_ECUM, 1, ECUM_MAINFUNCTION_ID, ECUM_E_NOT_INITIATED);
		return;
	}
#endif

	// If coming from startup sequence, enter Run mode
	if (internal_data->current_state == ECUM_STATE_STARTUP_TWO)
		enter_run_mode();

	if (internal_data->current_state == ECUM_STATE_APP_RUN)
	{
		if (!hasRunRequests() && (internal_data_run_state_timeout == 0))
		{
			enter_post_run_mode();
			return;
		}
	}

	if (internal_data->current_state == ECUM_STATE_APP_POST_RUN)
	{
		if (hasRunRequests())
		{
			enter_run_mode(); // ECUM_2866
			return;
		}

		if (!hasPostRunRequests())
		{
			EcuM_OnExitPostRun(); // ECUM_2761
			enter_prep_shutdown_mode();
			return;
		}
	}
}
