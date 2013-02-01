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



#include "Std_Types.h"
//#include "Det.h"
#include "Gpt.h"
#include <string.h>
#if defined(USE_KERNEL)
#include "Os.h"
#include "isr.h"
#include "arc.h"
#endif
#include "stm32f10x.h"



//TIM2->DIER =


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








#include "Std_Types.h"
#include "Gpt.h"
#include "Cpu.h"
#include <assert.h>
#include <string.h>
#include "Mcu.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#if defined(USE_KERNEL)
#include "Os.h"
#include "isr.h"
#endif

// Implementation specific
/* STM32 helper arrays
 *
 */
TIM_TypeDef * const TimAddr[] =
{
	((TIM_TypeDef *)TIM1_BASE),
	((TIM_TypeDef *)TIM2_BASE),
	((TIM_TypeDef *)TIM3_BASE),
	((TIM_TypeDef *)TIM4_BASE),
	((TIM_TypeDef *)TIM5_BASE),
	((TIM_TypeDef *)TIM6_BASE),
    ((TIM_TypeDef *)TIM7_BASE),
};

#if ( GPT_DEV_ERROR_DETECT == STD_ON )
#define VALIDATE(_exp,_api,_err ) \
		if( !(_exp) ) { \
			Det_ReportError(MODULE_ID_GPT,0,_api,_err); \
			return; \
		}

#define VALIDATE_W_RV(_exp,_api,_err,_rv ) \
		if( !(_exp) ) { \
			Det_ReportError(MODULE_ID_GPT,0,_api,_err); \
			return (_rv); \
		}

#define VALID_CHANNEL(_ch)		( Gpt_Global.configured & (1<<(_ch)) )

#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_W_RV(_exp,_api,_err,_rv )
#endif

const uint32 IrqVector[] =
{
		TIM1_UP_IRQn,
		TIM2_IRQn,
		TIM3_IRQn,
		TIM4_IRQn,
};

typedef enum
{
	GPT_STATE_STOPPED = 0,
			GPT_STATE_STARTED,
} Gpt_StateType;

/**
 * Type that holds all global data for Gpt
 */
typedef struct
{
	// Set if Gpt_Init() have been called
	boolean initRun;

	// Our config
	const Gpt_ConfigType *config;

	uint8 wakeupEnabled;

	// One bit for each channel that is configured.
	// Used to determine if validity of a channel
	// 1 - configured
	// 0 - NOT configured
	uint32 configured;

	// Maps the a channel id to a configured channel id
	uint8 channelMap[GPT_CHANNEL_CNT];

} Gpt_GlobalType;

/**
 * Type that holds data that are specific for a channel
 */
typedef struct
{
	Gpt_StateType state;
} Gpt_UnitType;

Gpt_UnitType Gpt_Unit[GPT_CHANNEL_CNT];

// Global config
Gpt_GlobalType Gpt_Global;

//-------------------------------------------------------------------


/**
 * ISR helper-function that handles the HW channels( 0 to 8 )
 *
 * @param channel - Channel that the raised the interrupt
 */
static void Gpt_IsrCh(Gpt_ChannelType channel)
{
	const Gpt_ConfigType *config;
	int confCh;

	confCh = Gpt_Global.channelMap[channel];
	assert(confCh != GPT_CHANNEL_ILL);

	config = &Gpt_Global.config[confCh];

	if (config->GptChannelMode == GPT_MODE_ONESHOT)
	{
		// Disable the channel
		TimAddr[channel]->CR1 &= ~TIM_CR1_CEN;

		Gpt_Unit[channel].state = GPT_STATE_STOPPED;
	}
	config->GptNotification();

	// Clear interrupt
	TimAddr[channel]->SR &= ~TIM_SR_UIF;
}

//-------------------------------------------------------------------
// Macro that counts leading zeroes.
#define CNTLZW_INV(x) (31-cntlzw(x))

/**
 * ISR that handles all interrupts
 */
static void Gpt_Isr(void)
{
	static int i;
	int found = 0;
	/* Find channel for this interrupt. */
	for (i = 0; ((i < GPT_CHANNEL_CNT) && !found);i++)
	{
		found = (((TimAddr[i]->SR & TIM_SR_UIF) == TIM_SR_UIF) &&
				 ((TimAddr[i]->CR1 & TIM_CR1_CEN) == TIM_CR1_CEN));
	}
	if (found)
	{
		Gpt_IsrCh(i - 1);
	}
}
//-------------------------------------------------------------------

void Gpt_Init(const Gpt_ConfigType *config)
{
	uint32_t i=0;
	const Gpt_ConfigType *cfg;
	VALIDATE( (Gpt_Global.initRun == STD_OFF), GPT_INIT_SERVICE_ID, GPT_E_ALREADY_INITIALIZED );
#if defined(GPT_VARIANT_PB)
	VALIDATE( (config != NULL ), GPT_INIT_SERVICE_ID, GPT_E_PARAM_CONFIG );
#elif 	defined(GPT_VARIANT_PC)
	// We don't support GPT_VARIANT_PC
	assert(0);
#endif
	Gpt_ChannelType ch;

	for (i=0; i<GPT_CHANNEL_CNT; i++)
	{
		Gpt_Global.channelMap[i] = GPT_CHANNEL_ILL;
	}

	i = 0;
	cfg = config;
	while (cfg->GptChannelId != GPT_CHANNEL_ILL)
	{
		ch = cfg->GptChannelId;

		// Assign the configuration channel used later..
		Gpt_Global.channelMap[cfg->GptChannelId] = i;
		Gpt_Global.configured |= (1<<ch);

		if (ch <= GPT_CHANNEL_CNT)
		{
			if (cfg->GptNotification != NULL)
			{
				switch (ch) {
					case 0: ISR_INSTALL_ISR2( "Gpt_Isr", Gpt_Isr, TIM1_UP_IRQn, 6, 0 ); break;
					case 1: ISR_INSTALL_ISR2( "Gpt_Isr", Gpt_Isr, TIM2_IRQn,    6, 0 ); break;
					case 2: ISR_INSTALL_ISR2( "Gpt_Isr", Gpt_Isr, TIM3_IRQn,    6, 0 ); break;
					case 3: ISR_INSTALL_ISR2( "Gpt_Isr", Gpt_Isr, TIM4_IRQn,    6, 0 ); break;
					default:
						break;
				}
			}
		}
#if defined(USE_KERNEL)
		// Don't install if we use kernel.. it handles that.
#else

#endif

		cfg++;
		i++;
	}

	Gpt_Global.config = config;

	Gpt_Global.initRun = STD_ON;
	//PIT.CTRL.B.MDIS = 0;
}

//-------------------------------------------------------------------

#if GPT_DEINIT_API == STD_ON
void Gpt_DeInit(void)
{
	Gpt_ChannelType channel;
	VALIDATE( (Gpt_Global.initRun == STD_ON), GPT_DEINIT_SERVICE_ID, GPT_E_UNINIT );
	for (channel=0; channel<GPT_CHANNEL_CNT; channel++) // Validate that all channels have been stopped
	{
		VALIDATE( (Gpt_Unit[channel].state == GPT_STATE_STOPPED), GPT_DEINIT_SERVICE_ID, GPT_E_BUSY );
		Gpt_StopTimer(channel); // Should this be done here?
	}
	Gpt_Global.initRun = STD_OFF;
	Gpt_Global.configured = 0;
	//_config.config = NULL;
}
#endif

//-------------------------------------------------------------------
// period is in "ticks" !!
void Gpt_StartTimer(Gpt_ChannelType channel, Gpt_ValueType period_ticks)
{
	int confCh;

	VALIDATE( (Gpt_Global.initRun == STD_ON), GPT_STARTTIMER_SERVICE_ID, GPT_E_UNINIT );
	VALIDATE( VALID_CHANNEL(channel), GPT_STARTTIMER_SERVICE_ID, GPT_E_PARAM_CHANNEL );
	VALIDATE( (Gpt_Unit[channel].state == GPT_STATE_STOPPED), GPT_STARTTIMER_SERVICE_ID, GPT_E_BUSY );
	// GPT_E_PARAM_VALUE, all have 32-bit so no need to check

	//DEBUG(DEBUG_HIGH, "Gpt_StartTimer ch=%d, period=%d [ticks]\n", channel, period_ticks);

	confCh = Gpt_Global.channelMap[channel];

	if (channel <= GPT_CHANNEL_CNT)
	{
		uint32 *tlval = (uint32 *)&(TimAddr[channel]->ARR);
		uint32 *tval = (uint32 *)&(TimAddr[channel]->CNT);

		*tlval = period_ticks;
		*tval = period_ticks;

		// always select interrupt
		//TimAddr[channel]->DIER |= TIM_DIER_UIE;

		// Make sure that no interrupt is pending.
		TimAddr[channel]->SR &= ~TIM_SR_UIF;

		// Set prescaler.
		TimAddr[channel]->PSC = Gpt_Global.config[confCh].GptChannelPrescale;

		// Enable timer
		TimAddr[channel]->CR1 |= (TIM_CR1_CEN | TIM_CR1_URS | TIM_CR1_DIR);
		TimAddr[channel]->CR1 &= ~TIM_CR1_UDIS;
	}
	#if ( GPT_ENABLE_DISABLE_NOTIFICATION_API == STD_ON )
	if( Gpt_Global.config[confCh].GptNotification != NULL )
	{
		// GPT275
		Gpt_EnableNotification(channel);
	}
	#endif
	Gpt_Unit[channel].state = GPT_STATE_STARTED;
}

void Gpt_StopTimer(Gpt_ChannelType channel)
{

	VALIDATE( (Gpt_Global.initRun == STD_ON), GPT_STOPTIMER_SERVICE_ID, GPT_E_UNINIT );
	VALIDATE( VALID_CHANNEL(channel), GPT_STOPTIMER_SERVICE_ID, GPT_E_PARAM_CHANNEL );

	if (channel <= GPT_CHANNEL_CNT)
	{
		// Disable timer
		TimAddr[channel]->CR1 &= ~TIM_CR1_CEN;
	}
	#if ( GPT_ENABLE_DISABLE_NOTIFICATION_API == STD_ON )
	Gpt_DisableNotification(channel);
	#endif
	Gpt_Unit[channel].state = GPT_STATE_STOPPED;
}

#if ( GPT_TIME_REMAINING_API == STD_ON )

Gpt_ValueType Gpt_GetTimeRemaining(Gpt_ChannelType channel)
{
	VALIDATE_W_RV( (Gpt_Global.initRun == STD_ON), GPT_GETTIMEREMAINING_SERVICE_ID, GPT_E_UNINIT, 0 );
	VALIDATE_W_RV( VALID_CHANNEL(channel),GPT_GETTIMEREMAINING_SERVICE_ID, GPT_E_PARAM_CHANNEL, 0 );
	VALIDATE_W_RV( (Gpt_Unit[channel].state == GPT_STATE_STARTED), GPT_GETTIMEREMAINING_SERVICE_ID, GPT_E_NOT_STARTED, 0 );
	Gpt_ValueType remaining;

	if (channel <= GPT_CHANNEL_CNT)
	{
		uint32 *tval = (uint32 *)&(TimAddr[channel]->CNT);
		// Time remaining is the time until it hits 0, so just return the current timer value
		remaining = *tval;
	}

	return remaining;
}
#endif

#if ( GPT_TIME_ELAPSED_API == STD_ON )
Gpt_ValueType Gpt_GetTimeElapsed(Gpt_ChannelType channel)
{
	Gpt_ValueType timer;

	VALIDATE_W_RV( (Gpt_Global.initRun == STD_ON), GPT_GETTIMEELAPSED_SERVICE_ID, GPT_E_UNINIT ,0 );
	VALIDATE_W_RV( VALID_CHANNEL(channel),GPT_GETTIMEELAPSED_SERVICE_ID, GPT_E_PARAM_CHANNEL, 0 );
	VALIDATE_W_RV( (Gpt_Unit[channel].state == GPT_STATE_STARTED),GPT_GETTIMEELAPSED_SERVICE_ID, GPT_E_NOT_STARTED, 0 );

	// NOTE!
	// These little creatures count down

	if (channel <= GPT_CHANNEL_CNT)
	{
		uint32 *tlval = (uint32 *)&(TimAddr[channel]->ARR);
	    uint32 *tval = (uint32 *)&(TimAddr[channel]->CNT);
		timer = *tlval - *tval;
	}

	return (timer);
}
#endif

#if ( GPT_ENABLE_DISABLE_NOTIFICATION_API == STD_ON )
void Gpt_EnableNotification(Gpt_ChannelType channel)
{

	VALIDATE( (Gpt_Global.initRun == STD_ON), 0x7, GPT_E_UNINIT );
	VALIDATE( VALID_CHANNEL(channel),0x7, GPT_E_PARAM_CHANNEL );

	if (channel <= GPT_CHANNEL_CNT)
	{
		// enable interrupts
		TimAddr[channel]->DIER |= TIM_DIER_UIE;
	}
}

void Gpt_DisableNotification(Gpt_ChannelType channel)
{

	VALIDATE( (Gpt_Global.initRun == STD_ON), 0x8, GPT_E_UNINIT );
	VALIDATE( VALID_CHANNEL(channel),0x8, GPT_E_PARAM_CHANNEL );

	if (channel <= GPT_CHANNEL_CNT)
	{
		TimAddr[channel]->DIER &= ~TIM_DIER_UIE;
	}

	return;
}

#endif

#if ( GPT_WAKEUP_FUNCTIONALITY_API == STD_ON )

void Gpt_SetMode(Gpt_ModeType mode)
{
	int i;

	VALIDATE( (Gpt_Global.initRun == STD_ON), GPT_SETMODE_SERVIVCE_ID, GPT_E_UNINIT );
	VALIDATE( ( mode <= GPT_MODE_SLEEP ), GPT_SETMODE_SERVIVCE_ID, GPT_E_PARAM_MODE );

	if (mode == GPT_MODE_NORMAL)
	{
		//PIT.CTRL.B.MDIS = 0; TODO
		// Do NOT restart channels
	}
	else if (mode == GPT_MODE_SLEEP)
	{

		//PIT.CTRL.B.MDIS = 1; TODO
		// Disable all
		for (i= 0; i <= GPT_CHANNEL_CNT; i++)
		{
			Gpt_StopTimer(i);
		}
	}
}

void Gpt_DisableWakeup(Gpt_ChannelType channel)
{
	VALIDATE( (Gpt_Global.initRun == STD_ON), GPT_DISABLEWAKEUP_SERVICE_ID, GPT_E_UNINIT );
	VALIDATE( VALID_CHANNEL(channel), GPT_DISABLEWAKEUP_SERVICE_ID, GPT_E_PARAM_CHANNEL );
	// Only RTI have system wakeup
	if (channel == GPT_CHANNEL_RTI)
	{
		Gpt_Global.wakeupEnabled = STD_OFF;
	}
	else
	{
		// TODO:
		//assert(0);
	}
}

void Gpt_EnableWakeup(Gpt_ChannelType channel)
{
	VALIDATE( (Gpt_Global.initRun == STD_ON), GPT_ENABLEWAKEUP_SERVICE_ID, GPT_E_UNINIT );
	VALIDATE( VALID_CHANNEL(channel),GPT_ENABLEWAKEUP_SERVICE_ID, GPT_E_PARAM_CHANNEL );
	if (channel == GPT_CHANNEL_RTI)
	{
		Gpt_Global.wakeupEnabled = STD_ON;
	}
	else
	{
		// TODO:
		//assert(0);
	}
}

void Gpt_Cbk_CheckWakeup(EcuM_WakeupSourceType wakeupSource)
{

}

#endif

