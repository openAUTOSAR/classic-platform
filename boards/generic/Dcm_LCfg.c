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








#warning "This default file may only be used as an example!"

#include "Dcm.h"
#include "Rte_Dcm.h"




/*********************
 * DCM Configuration *
 *********************/

/*
 * DSP configurations
 */
const Dcm_DspType Dsp = {
		.DspDid = NULL,
		.DspDidInfo = NULL,
		.DspEcuReset = NULL,
		.DspPid = NULL,
		.DspReadDTC = NULL,
		.DspRequestControl = NULL,
		.DspRoutine = NULL,
		.DspRoutineInfo = NULL,
		.DspSecurity = NULL,
		.DspSession = NULL,
		.DspTestResultByObdmid = NULL,
		.DspVehInfo = NULL,
};


/*
 * DSD configurations
 */
const Dcm_DsdType Dsd = {
		.DsdServiceTable = NULL
};


/*
 * DSL configurations
 */
const Dcm_DslType Dsl = {
		.DslBuffer = NULL,
		.DslCallbackDCMRequestService = NULL,
		.DslDiagResp = NULL,
		.DslProtocol = NULL,
		.DslProtocolTiming = NULL,
		.DslServiceRequestIndication = NULL,
		.DslSessionControl = NULL
};

/*
 * DCM configurations
 */
const Dcm_ConfigType DCM_Config = {
		.Dsp = &Dsp,
		.Dsd = &Dsd,
		.Dsl = &Dsl
};
