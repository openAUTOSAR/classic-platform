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

/** @addtogroup Mcu MCU Driver
 *  @{ */

/** @file Mcu.h
 * API and type definitions for MCU Driver.
 */

#ifndef MCU_H_
#define MCU_H_

#include "Modules.h"

#define MCU_MODULE_ID		 	    MODULE_ID_MCU
#define MCU_VENDOR_ID			    60

#define MCU_SW_MAJOR_VERSION    	1
#define MCU_SW_MINOR_VERSION   	    0
#define MCU_SW_PATCH_VERSION    	0

#define MCU_AR_MAJOR_VERSION     	4
#define MCU_AR_MINOR_VERSION     	0
#define MCU_AR_PATCH_VERSION     	3

/* @req MCU211 */
#include "Cpu.h"
#include "irq_types.h"
#include "Std_Types.h"
#include "Mcu_Cfg.h"


/** @name Service id's */
//@{
#define MCU_INIT_SERVICE_ID                 0x00
#define MCU_INITRAMSECTION_SERVICE_ID       0x01
#define MCU_INITCLOCK_SERVICE_ID            0x02
#define MCU_DISTRIBUTEPLLCLOCK_SERVICE_ID   0x03
#define MCU_GETPLLSTATUS_SERVICE_ID         0x04
#define MCU_GETRESETREASON_SERVICE_ID       0x05
#define MCU_GETRESETRAWVALUE_SERVICE_ID     0x06
#define MCU_PERFORMRESET_SERVICE_ID         0x07
#define MCU_SETMODE_SERVICE_ID              0x08
#define MCU_GETVERSIONINFO_SERVICE_ID       0x09
#define MCU_INTCVECTORINSTALL_SERVICE_ID    0x0A // Not in spec but follows pattern
//@}


/* @req MCU012 */
/* @req MCU112 */
/** @name Error Codes */
//@{
#define MCU_E_PARAM_CONFIG                  0x0A
#define MCU_E_PARAM_CLOCK                   0x0B
#define MCU_E_PARAM_MODE                    0x0C
#define MCU_E_PARAM_RAMSECTION              0x0D
#define MCU_E_PLL_NOT_LOCKED                0x0E
#define MCU_E_UNINIT                        0x0F
#define MCU_E_PARAM_POINTER                 0x10
//@}

/* Specific return values */
#define MCU_GETRESETRAWVALUE_NORESETREG_RV  0x00         /* @req MCU006 */
#define MCU_GETRESETRAWVALUE_UNINIT_RV      0xffffffffU  /* @req MCU135 */


/* @req MCU231 */
typedef enum {
	MCU_PLL_LOCKED,
	MCU_PLL_UNLOCKED,
	MCU_PLL_STATUS_UNDEFINED
} Mcu_PllStatusType;


/* @req MCU240 */
typedef uint8_t Mcu_RamSectionType;

/* @req MCU236 */
typedef uint32_t Mcu_RawResetType;


/* @req MCU134 */
/* @req MCU234 */
typedef enum {
	MCU_POWER_ON_RESET,
	MCU_WATCHDOG_RESET,
	MCU_SW_RESET,
	MCU_RESET_UNDEFINED,
	MCU_OSC_FAILURE_RESET,
    MCU_CPU_RESET,
    MCU_EXT_RESET,
    MCU_VSW_RESET
} Mcu_ResetType;

typedef struct {
	//	This is the frequency for the specific instance of the McuClockReference-
	//	Point container. It shall be givn in Hz.
	uint32 McuClockReferencePointFrequency;

	uint8 Pll1; // PLL setting 1
	uint8 Pll2; // PLL setting 2
	uint8 Pll3; // PLL setting 3
	uint8 Pll1_1; // PLL setting 1
	uint8 Pll2_1; // PLL setting 2
	uint8 Pll3_1; // PLL setting 3
	uint8 Pll4; // PLL setting 4

} Mcu_ClockSettingConfigType;

typedef struct {
	// This parameter shall represent the Data pre-setting to be initialized
	uint32 McuRamDefaultValue;

	// This parameter shall represent the MCU RAM section base address
	uint32 McuRamSectionBaseAddress;

	// This parameter shall represent the MCU RAM Section size
	uint32 McuRamSectionSize;

} Mcu_RamSectorSettingConfigType;


#if ( MCU_GET_RAM_STATE == STD_ON )
typedef enum {
	MCU_RAMSTATE_INVALID,
	MCU_RAMSTATE_VALID
} Mcu_RamStateType;
#endif


/* @req MCU131 */
/* @req MCU054 */
/* @req MCU030 */
typedef struct {

	//	This parameter shall represent the number of Modes available for the
	//	MCU. calculationFormula = Number of configured McuModeSettingConf
	//uint8 McuNumberOfMcuModes; /* Not supported */

	//  This parameter shall represent the number of RAM sectors available for
	//  the MCU. calculationFormula = Number of configured McuRamSectorSet-
	//  tingConf
	uint8 McuRamSectors;

	//  This parameter shall represent the number of clock setting available for
	//  the MCU.
	uint8 McuClockSettings;

	// This parameter defines the default clock settings that should be used
	// It is an index into the McuClockSettingsConfig
	Mcu_ClockType McuDefaultClockSettings;

	//	This parameter relates to the MCU specific reset configuration. This ap-
	//	plies to the function Mcu_PerformReset, which performs a microcontroller
	//	reset using the hardware feature of the microcontroller.
	//uint32 McuResetSetting;

	//	This container contains the configuration (parameters) for the
	//	Clock settings of the MCU. Please see MCU031 for more in-
	//	formation on the MCU clock settings.
	const Mcu_ClockSettingConfigType * McuClockSettingConfig;

	//	This container contains the configuration (parameters) for the
	//	Mode setting of the MCU. Please see MCU035 for more information
	//  on the MCU mode settings.
	//Mcu_ModeSettingConfigType  *McuModeSettingConfig;

	//	This container contains the configuration (parameters) for the
	//	RAM Sector setting. Please see MCU030 for more information
	//	on RAM sec-tor settings.
	const Mcu_RamSectorSettingConfigType *McuRamSectorSettingConfig;

} Mcu_ConfigType;

extern const Mcu_ConfigType McuConfigData[];


/* @req MCU153 */
/* @req MCU126 */
void Mcu_Init( const Mcu_ConfigType *configPtr );
//void Mcu_DeInit( void );
/* @req MCU154 */
Std_ReturnType Mcu_InitRamSection(Mcu_RamSectionType RamSection );

/* @req MCU207 */
/* @req MCU209 */
#if ( MCU_GET_RAM_STATE == STD_ON )
Mcu_RamStateType Mcu_GetRamState( void );
#endif 


/* @req MCU248 */
/* @req MCU155 */
/* @req MCU210 */
#if ( MCU_INIT_CLOCK == STD_ON )
Std_ReturnType Mcu_InitClock( Mcu_ClockType ClockSetting );
#endif

/* @req MCU230 */
/* @req MCU156 */
/* @req MCU157 */
/* @req MCU205 */
/* @req MCU206 */
#if ( MCU_NO_PLL == STD_OFF )
void Mcu_DistributePllClock( void );
#endif
Mcu_PllStatusType Mcu_GetPllStatus( void );

/* Functions related to reset */
/* @req MCU052 */
/* @req MCU055 */
/* @req MCU146 */
/* @req MCU158 */
/* @req MCU159 */
/* @req MCU160 */
Mcu_ResetType Mcu_GetResetReason( void );
Mcu_RawResetType Mcu_GetResetRawValue( void );
#if ( MCU_PERFORM_RESET_API == STD_ON )
void Mcu_PerformReset( void );
#endif


/* @req MCU164 */
/* @req MCU161 */
void Mcu_SetMode( Mcu_ModeType McuMode );

/* @req MCU103*/
/* @req MCU104*/
/* @req MCU149*/
/* @req MCU162*/
#if ( MCU_VERSION_INFO_API == STD_ON )
#define Mcu_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,MCU)
#endif

/* ArcCore extensions */

void Irq_InstallVector(void (*func)(void), IrqType vector, uint8_t priority, Cpu_t cpu );
void Irq_GenerateSoftInt( IrqType vector );
uint8_t Irq_GetCurrentPriority( Cpu_t cpu);
uint32_t McuE_GetSystemClock( void );
#if defined(CFG_MPC55XX) || defined(CFG_ARM_CR4)
uint32_t McuE_GetPeripheralClock( McuE_PeriperalClock_t type );
uint32_t McuE_GetClockReferencePointFrequency(void);
#endif

void Mcu_Arc_EnterLowPower( int mode );
void McuE_LowPowerRecoverFlash( void );
void McuE_InitZero(void);

#if defined(CFG_PPC)
void McuE_GetECCError( uint8 *err );
#endif
#endif /*MCU_H_*/
/** @} */
