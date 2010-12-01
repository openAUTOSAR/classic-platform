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

#include "EcuM.h"

EcuM_ConfigType EcuMConfig =
{
	.EcuMDefaultShutdownTarget = ECUM_STATE_RESET,
	.EcuMDefaultShutdownMode = 0, // Don't care
	.EcuMDefaultAppMode = OSDEFAULTAPPMODE,
	.EcuMNvramReadAllTimeout = ECUM_NVRAM_READALL_TIMEOUT,
	.EcuMNvramWriteAllTimeout = ECUM_NVRAM_WRITEALL_TIMEOUT,
	.EcuMRunMinimumDuration = ECUM_NVRAM_MIN_RUN_DURATION,

#if defined(USE_MCU)
        .McuConfig = McuConfigData,
#endif
#if defined(USE_PORT)
        .PortConfig = &PortConfigData,
#endif
#if defined(USE_CAN)
        .CanConfig = &CanConfigData,
#endif
#if defined(USE_CANIF)
        .CanIfConfig = &CanIf_Config,
#endif
#if defined(USE_CANSM)
        .CanSMConfig = &CanSM_Config,
#endif
#if defined(USE_CANNM)
        .CanNmConfig = &CanNm_Config,
#endif
#if defined(USE_COM)
        .ComConfig = &ComConfiguration,
#endif
#if defined(USE_COMM)
        .ComMConfig = &ComM_Config,
#endif
#if defined(USE_NM)
        .NmConfig = &Nm_Config,
#endif
#if defined(USE_PDUR)
        .PduRConfig = &PduR_Config,
#endif
#if defined(USE_DMA)
        .DmaConfig = DmaConfig,
#endif
#if defined(USE_ADC)
        .AdcConfig = AdcConfig,
#endif
#if defined(USE_PWM)
        .PwmConfig = &PwmConfig,
#endif
#if defined(USE_GPT)
        .GptConfig = GptConfigData,
#endif
#if defined(USE_FLS)
	.FlashConfig = FlsConfigSet,
#endif
};
