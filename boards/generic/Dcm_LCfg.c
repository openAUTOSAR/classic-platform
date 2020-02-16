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








#warning "This default file may only be used as an example!"

#include "Dcm.h"




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
