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

/** @tagSettings DEFAULT_ARCHITECTURE=GENERIC */
/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2|4.3.0*/


/** @addtogroup Mcu MCU Driver
 *  @{ */

/** @file Mcu.h
 * API and type definitions for MCU Driver.
 */

/* @req 4.1.2|4.3.0/SWS_Mcu_00215 */

#ifndef MCU_H_
#define MCU_H_

#define MCU_MODULE_ID               101
#define MCU_VENDOR_ID               60

#define MCU_SW_MAJOR_VERSION        2
#define MCU_SW_MINOR_VERSION   	    0
#define MCU_SW_PATCH_VERSION        0

#define MCU_AR_RELEASE_MAJOR_VERSION        4
#define MCU_AR_RELEASE_MINOR_VERSION        1
#define MCU_AR_RELEASE_PATCH_VERSION        2

/* @req 4.1.2|4.3.0/SWS_Mcu_00211 */
#include "Cpu.h"
#include "irq_types.h"
#include "Std_Types.h"
#include "Mcu_Cfg.h"

#if defined(USE_DEM)
#include "Dem.h"
#endif

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
#define MCU_LOSS_OF_CLOCK_SERVICE_ID        0x0B // Not in spec but follows pattern
//@}


/* @req SWS_Mcu_000012 */
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
#define MCU_GETRESETRAWVALUE_NORESETREG_RV  0x00         /* Not used for PPC. Raw reset value is supported */
#define MCU_GETRESETRAWVALUE_UNINIT_RV      0xffffffffU  /* Not used for PPC. Raw reset value is supported */


/* @req 4.1.2|4.3.0/SWS_Mcu_00250 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00231 */
typedef enum {
    MCU_PLL_LOCKED,
    MCU_PLL_UNLOCKED,
    MCU_PLL_STATUS_UNDEFINED
} Mcu_PllStatusType;

/* @req 4.1.2|4.3.0/SWS_Mcu_00255 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00240 */
typedef uint8 Mcu_RamSectionType;

/* @req 4.1.2|4.3.0/SWS_Mcu_00253 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00236 */
typedef uint32 Mcu_RawResetType;

/* @req 4.1.2|4.3.0/SWS_Mcu_00251 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00233 */
typedef uint8 Mcu_ClockType;

/* @req 4.1.2|4.3.0/SWS_Mcu_00254 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00238 */
typedef uint8 Mcu_ModeType;

/* @req 4.1.2|4.3.0/SWS_Mcu_00252 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00234 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00134 */
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
/* @req 4.1.2|4.3.0/SWS_Mcu_00256 */
typedef enum {
    MCU_RAMSTATE_INVALID,
    MCU_RAMSTATE_VALID
} Mcu_RamStateType;
#endif


/* @req 4.1.2|4.3.0/SWS_Mcu_00249 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00131 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00054 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00031 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00030 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00035 */
typedef struct {

    //	This parameter shall represent the number of Modes available for the
    //	MCU. calculationFormula = Number of configured McuModeSettingConf
    uint8 McuNumberOfMcuModes;

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

    // This parameter holds the DemEvent used when reporting McuClockFailure.
#ifdef USE_DEM
    Dem_EventIdType                 McuClockFailure;
#endif

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


/* @req 4.1.2|4.3.0/SWS_Mcu_00153 */
void Mcu_Init( const Mcu_ConfigType *ConfigPtr );

/* @req 4.1.2|4.3.0/SWS_Mcu_00154 */
Std_ReturnType Mcu_InitRamSection(Mcu_RamSectionType RamSection );

/* @req 4.1.2|4.3.0/SWS_Mcu_00207 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00209 */
#if ( MCU_GET_RAM_STATE == STD_ON )
Mcu_RamStateType Mcu_GetRamState( void );
#endif 

/* @req 4.1.2|4.3.0/SWS_Mcu_00248*/
/* @req 4.1.2|4.3.0/SWS_Mcu_00155 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00210 */
#if ( MCU_INIT_CLOCK == STD_ON )
Std_ReturnType Mcu_InitClock( Mcu_ClockType ClockSetting );
#endif

/* @req 4.1.2|4.3.0/SWS_Mcu_00156 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00205 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00230 */
#if ( MCU_NO_PLL == STD_OFF )
Std_ReturnType Mcu_DistributePllClock( void );
#endif

/* @req 4.1.2|4.3.0/SWS_Mcu_00157 */
Mcu_PllStatusType Mcu_GetPllStatus( void );

/* @req 4.1.2|4.3.0/SWS_Mcu_00052*/
/* @req 4.1.2|4.3.0/SWS_Mcu_00158 */
Mcu_ResetType Mcu_GetResetReason( void );

/* @req 4.1.2|4.3.0/SWS_Mcu_00159 */
Mcu_RawResetType Mcu_GetResetRawValue( void );

/* @req 4.1.2|4.3.0/SWS_Mcu_00055 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00146 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00160 */
#if ( MCU_PERFORM_RESET_API == STD_ON )
void Mcu_PerformReset( void );
#endif

/* @req 4.1.2|4.3.0/SWS_Mcu_00164 */
/* @req 4.1.2|4.3.0/SWS_Mcu_00161 */
void Mcu_SetMode( Mcu_ModeType McuMode );

/* @req 4.1.2|4.3.0/SWS_Mcu_00162*/
#if ( MCU_VERSION_INFO_API == STD_ON )
void Mcu_GetVersionInfo( Std_VersionInfoType* versioninfo);
#endif

/* ArcCore extensions functions */

uint32 Mcu_Arc_GetSystemClock( void );
#if defined(CFG_MPC55XX) || defined(CFG_TMS570)
uint32 Mcu_Arc_GetPeripheralClock( Mcu_Arc_PeriperalClock_t type );
uint32 Mcu_Arc_GetClockReferencePointFrequency(void);
#endif

void Mcu_Arc_EnterLowPower( int mode );
void Mcu_Arc_LowPowerRecoverFlash( void );
void Mcu_Arc_InitZero(void);
Mcu_ResetType Mcu_Arc_GetResetReason(void);

void Mcu_Arc_DeInit(void);
void Mcu_Arc_LossOfLock(void);
void Mcu_Arc_LossOfClock(void);

#if defined(CFG_PPC)
void Mcu_Arc_GetECCError( uint8 *err );
#endif

#endif /*MCU_H_*/
/** @} */
