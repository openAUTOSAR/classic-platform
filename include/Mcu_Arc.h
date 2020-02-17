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

#ifndef MCU_ARC_H_
#define MCU_ARC_H_


#if defined(CFG_MCU_ARC_LP)
#include "Mcu_Arc_Cfg.h"
#endif

#if defined(CFG_MCU_MMU)

#if defined(CFG_PPC)
#include "mm.h"
#endif

#endif

/*lint -esym(754, core_info_t::name) Ok that it is not used  */
/*lint -esym(754, cpu_info_t::name) Ok that it is not used  */
/*lint -save -e970 Used for debugging  */
typedef struct{
    char_t *name;
    uint32 pvr;
    const char *info;
} core_info_t;

typedef struct{
    char_t *name;
    uint32 pvr;
    const char *info; //lint -esym(754, cpu_info_t::info)   PC-lint violation: Info only, might not be used
} cpu_info_t;
/*lint -restore */

extern const cpu_info_t cpu_info_list[];
extern const core_info_t core_info_list[];

const cpu_info_t * Mcu_Arc_GetCpuInfoList( uint32 *num );
const core_info_t * Mcu_Arc_GetCoreInfoList( uint32 *num );

void Mcu_Arc_WdgExtendTimeout( void );
void Mcu_Arc_PlatformInit( void );

/**
 * Called before initialization in Mcu_Init()
 * @param configPtr
 */
void 	 Mcu_Arc_InitPre( const Mcu_ConfigType *configPtr );

/**
 * Called after initialization in Mcu_Init()
 * @param configPtr
 */
void 	 Mcu_Arc_InitPost( const Mcu_ConfigType *configPtr );


/**
 * Called before initialization in Mcu_InitClock()
 * @param clockSettingsPtr
 */
void 	 Mcu_Arc_InitClockPre( const Mcu_ClockSettingConfigType *clockSettingsPtr );

/**
 * Called after initialization in Mcu_InitClock()
 * @param clockSettingsPtr
 */
void 	 Mcu_Arc_InitClockPost( const Mcu_ClockSettingConfigType *clockSettingsPtr );

/**
 * Called from Mcu_Arc_InitZero.
 */
void     Mcu_Arc_PlatformInit( void );


const core_info_t *Mcu_Arc_IdentifyCore(uint32 pvr);

const cpu_info_t *Mcu_Arc_IdentifyCpu(uint32 pvr);

#if defined(CFG_MCU_ARC_LP)
/**
 * Called in start of Mcu_SetMode()
 * @param mcuMode
 */
void  	 Mcu_Arc_SetModePre( Mcu_ModeType mcuMode);

/**
 * Called in end of Mcu_SetMode()
 * @param mcuMode
 */
void  	 Mcu_Arc_SetModePost( Mcu_ModeType mcuMode);
#endif

/* Sleep is usually unique for each MCU */
#if defined(CFG_MCU_ARC_LP)
void Mcu_Arc_SetModePre2( Mcu_ModeType mcuMode, const struct Mcu_Arc_SleepConfig *sleepCfg );
void Mcu_Arc_SetModePost2( Mcu_ModeType mcuMode, const struct Mcu_Arc_SleepConfig *sleepCfg );
void Mcu_Arc_LowPowerRecoverFlash( void );
#endif

/**
 * Intiialize MCU memory management
 */
void Mcu_Arc_InitMM( void );


#if defined(CFG_PPC)
/**
 * Called by lower level exception handlers and tries to handle
 * an exception.
 * @param exceptionVector The exception vector.
 * @return
 */
uint32_t Mpc5xxx_ExceptionHandler(uint32_t exceptionVector);


/**
 * Function called by exception handlers when there is a fatal error.
 * Will also call ShutdownOs(E_OS_PANIC).
 * @param error     The exception that caused the error
 * @param pData     Pointer to private data that may want to be dumped.
 */
void Mpc5xxx_Panic( uint32 error, void *pData);

void Mcu_Arc_SafeMode(void);

void Mcu_Arc_SwitchMode( uint32 mode );
uint32 Mcu_Arc_GetMode( void );

#endif

#endif /* MCU_ARC_H_ */
