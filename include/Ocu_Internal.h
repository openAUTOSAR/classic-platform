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

#ifndef OCU_INTERNAL_H_
#define OCU_INTERNAL_H_

#include "mpc55xx.h"

typedef volatile struct EMIOS_tag emios_t;

#if defined (CFG_MPC5645S)
#define CHANNELS_PER_EMIOS_UNIT     24u
#elif defined (CFG_MPC5606B) || defined(CFG_SPC560B54)
#define CHANNELS_PER_EMIOS_UNIT     32u
#endif
#define MCB_CHANNEL_MODE            0x50u
#define SAOC_CHANNEL_MODE           3u
#define GPIO_OUTPUT_CHANNEL_MODE    1u
#define GPIO_INPUT_CHANNEL_MODE     0u
#define SELECT_C_D_COUNTER_BUS      1u
#define SELECT_INTERNAL_COUNTER     3u
#define SET_OUTPUT_FF_HIGH          1u
#define SET_OUTPUT_FF_LOW           0u
#define DISABLE_EDSEL               0u
#define ENABLE_EDSEL_OUTPUT_TOGGLE  1u
#define UNIFIED_CHANNEL_D           16u
#define UNIFIED_CHANNEL_C           8u
#define OCU_FREEZE_ENABLE           1u
#define OCU_PRESCALER_ENABLE        1u
#define OCU_PRESCALER_DISABLE       0u
#define OCU_INTERRUPT_ENABLE        1u
#define OCU_INTERRUPT_DISABLE       0u

#define INVALID_COUNTERVALUE        0xFFFFu

#define GET_EMIOS_UNIT(_chnExp) (((_chnExp) > 0) ? &EMIOS_1 : &EMIOS_0)
#define EMIOS_UNITS 2
#define GET_EMIOS_CHANNEL(_ch) ((_ch)%CHANNELS_PER_EMIOS_UNIT)


#ifdef HOST_TEST
/*lint -esym(9003, Ocu_enablDelayGlobalVariable) Used by test framework */
extern boolean Ocu_enablDelayGlobalVariable;
#endif

/*lint -esym(9003, OcuConfigPtr) Global configuration pointer */
extern const Ocu_ConfigType * OcuConfigPtr;

#if (OCU_NOTIFICATION_SUPPORTED == STD_ON)
void Ocu_Arc_InstallInterrupts (void);
#endif


#endif /*OCU_INTERNAL_H_ */
