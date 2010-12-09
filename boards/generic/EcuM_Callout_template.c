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
#include "Det.h"
#if defined(USE_DEM)
#include "Dem.h"
#endif
#if defined(USE_MCU)
#include "Mcu.h"
#endif
#if defined(USE_GPT)
#include "Gpt.h"
#endif
#if defined(USE_CAN)
#include "Can.h"
#endif
#if defined(USE_CANIF)
#include "CanIf.h"
#endif
#if defined(USE_PDUR)
#include "PduR.h"
#endif
#if defined(USE_COM)
#include "Com.h"
#endif
#if defined(USE_CANTP)
#include "CanTp.h"
#endif
#if defined(USE_DCM)
#include "Dcm.h"
#endif
#if defined(USE_PWM)
#include "Pwm.h"
#endif
#if defined(USE_IOHWAB)
#include "IoHwAb.h"
#endif
#if defined(USE_FLS)
#include "Fls.h"
#endif
#if defined(USE_EEP)
#include "Eep.h"
#endif
#if defined(USE_FEE)
#include "Fee.h"
#endif
#if defined(USE_EA)
#include "Ea.h"
#endif
#if defined(USE_NVM)
#include "NvM.h"
#endif
#if defined(USE_COMM)
#include "ComM.h"
#endif
#if defined(USE_NM)
#include "Nm.h"
#endif
#if defined(USE_CANNM)
#include "CanNm.h"
#endif
#if defined(USE_CANSM)
#include "CanSM.h"
#endif
#if defined(USE_LINSM)
#include "LinSM.h"
#endif

void EcuM_AL_DriverInitZero()
{
	Det_Init();
    Det_Start();
}

EcuM_ConfigType* EcuM_DeterminePbConfiguration()
{
	return &EcuMConfig;
}

void EcuM_AL_DriverInitOne(const EcuM_ConfigType *ConfigPtr)
{
#if defined(USE_MCU)
	Mcu_Init(ConfigPtr->McuConfig);

	// Set up default clock (Mcu_InitClock requires initRun==1)
	Mcu_InitClock( ConfigPtr->McuConfig->McuDefaultClockSettings );

	// Wait for PLL to sync.
	while (Mcu_GetPllStatus() != MCU_PLL_LOCKED)
	  ;
#endif

#if defined(USE_DEM)
	// Preinitialize DEM
	Dem_PreInit();
#endif

#if defined(USE_PORT)
	// Setup Port
	Port_Init(ConfigPtr->PortConfig);
#endif


#if defined(USE_GPT)
	// Setup the GPT
	Gpt_Init(ConfigPtr->GptConfig);
#endif

	// Setup watchdog
	// TODO

#if defined(USE_DMA)
	// Setup DMA
	Dma_Init(ConfigPtr->DmaConfig);
#endif

#if defined(USE_ADC)
	// Setup ADC
	Adc_Init(ConfigPtr->AdcConfig);
#endif

	// Setup ICU
	// TODO

	// Setup PWM
#if defined(USE_PWM)
	// Setup PWM
	Pwm_Init(ConfigPtr->PwmConfig);
#endif
}

void EcuM_AL_DriverInitTwo(const EcuM_ConfigType* ConfigPtr)
{
#if defined(USE_SPI)
	// Setup SPI
	Spi_Init(ConfigPtr->SpiConfig);
#endif

#if defined(USE_EEP)
	// Setup EEP
	Eep_Init(ConfigPtr->EEpConfig);
#endif

#if defined(USE_FLS)
	// Setup Flash
	Fls_Init(ConfigPtr->FlashConfig);
#endif

#if defined(USE_FEE)
	// Setup FEE
	Fee_Init();
#endif

#if defined(USE_EA)
	// Setup EA
	Ea_init();
#endif

#if defined(USE_NVM)
	// Setup NVRAM Manager and start the read all job
	NvM_Init();
	NvM_ReadAll();
#endif

	// Setup CAN tranceiver
	// TODO

#if defined(USE_CAN)
	// Setup Can driver
	Can_Init(ConfigPtr->CanConfig);
#endif

#if defined(USE_CANIF)
	// Setup CanIf
	CanIf_Init(ConfigPtr->CanIfConfig);
#endif

#if defined(USE_CANTP)
	// Setup CAN TP
	CanTp_Init();
#endif

#if defined(USE_CANSM)
        CanSM_Init(ConfigPtr->CanSMConfig);
#endif

	// Setup LIN
	// TODO

#if defined(USE_PDUR)
	// Setup PDU Router
	PduR_Init(ConfigPtr->PduRConfig);
#endif

#if defined(USE_CANNM)
        // Setup Can Network Manager
        CanNm_Init(ConfigPtr->CanNmConfig);
#endif

#if defined(USE_NM)
        // Setup Network Management Interface
        Nm_Init(ConfigPtr->NmConfig);
#endif

#if defined(USE_COM)
	// Setup COM layer
	Com_Init(ConfigPtr->ComConfig);
#endif

#if defined(USE_DCM)
	// Setup DCM
	Dcm_Init();
#endif

#if defined(USE_IOHWAB)
	// Setup IO hardware abstraction layer
	IoHwAb_Init();
#endif

}

void EcuM_AL_DriverInitThree(const EcuM_ConfigType ConfigPtr)
{
#if defined(USE_DEM)
	// Setup DEM
	Dem_Init();
#endif

#if defined(USE_COMM)
        // Setup Communication Manager
        ComM_Init(ConfigPtr->ComMConfig);
#endif
}

void EcuM_OnEnterRUN(void)
{

}

void EcuM_OnExitRun(void)
{

}

void EcuM_OnExitPostRun(void)
{

}

void EcuM_OnPrepShutdown(void)
{

}

void EcuM_OnGoSleep(void)
{

}

void EcuM_OnGoOffOne(void)
{

}

void EcuM_OnGoOffTwo(void)
{

}

void EcuM_AL_SwitchOff(void)
{

}
