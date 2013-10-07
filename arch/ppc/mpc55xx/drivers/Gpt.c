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

/* ----------------------------[information]----------------------------------*/
/*
 * Author: mahi
 *
 * Part of Release:
 *   3.1.5 (ok, it says 3.1.1 but, thats wrong)
 *
 * Description:
 *   Implements the Gpt module
 *
 * Support:
 *   General				  			  Support
 *   -------------------------------------------
 *   GPT_DEV_ERROR_DETECT	 			    Y
 *
 *
 *  GptConfigurationOfOptApiServices 	  Support
 *   -------------------------------------------
 *   GPT_DEINIT_API	 						Y
 *   GPT_ENABLE_DISABLE_NOTIFICATION_API	Y
 *   GPT_TIME_ELAPSED_API					Y
 *   GPT_TIME_REMAINING_API					Y
 *   GPT_VERSION_INFO_API					Y
 *   GPT_WAKEUP_FUNCTIONALITY_API			Y
 *
 * Implementation Notes:
 *
 * Things left:
 *   Lots of tagging still to do.
 */

/* ----------------------------[includes]------------------------------------*/

#include "Std_Types.h"
#include "Gpt.h"
#include "Cpu.h"
#include <assert.h>
#include <string.h>
#include "mpc55xx.h"
#include "Mcu.h"
#include "debug.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#include "Os.h"
#include "isr.h"

/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/

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

/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/

typedef enum {
	GPT_STATE_STOPPED = 0, GPT_STATE_STARTED,
} Gpt_StateType;

/**
 * Type that holds all global data for Gpt
 */
typedef struct {
	// Set if Gpt_Init() have been called
	boolean initRun;

	// Our config
	const Gpt_ConfigType *config;

#if ( GPT_WAKEUP_FUNCTIONALITY_API == STD_ON )
	uint8 wakeupEnabled;
#endif

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
typedef struct {
	Gpt_StateType state;
} Gpt_UnitType;


/* GPT_CHANNEL_CNT represents the number of physical channels */
Gpt_UnitType Gpt_Unit[GPT_CHANNEL_CNT];

// Global config
Gpt_GlobalType Gpt_Global;

//-------------------------------------------------------------------

/*
 * ISR for a given PIT channel (macro)
 */
#if defined(CFG_MPC560X) || defined(CFG_MPC563XM)
#define GPT_ISR( _channel )                                \
	void Gpt_Isr_Channel##_channel( void )          \
	{                                                      \
    	const Gpt_ConfigType *config;                      \
    	int confCh;                                        \
                                                           \
        /* Find the config entry for the PIT channel. */   \
		confCh = Gpt_Global.channelMap[ _channel ];        \
		assert(confCh != GPT_CHANNEL_ILL);                 \
		config = &Gpt_Global.config[ confCh ];             \
                                                           \
        if( config->GptChannelMode == GPT_MODE_ONESHOT )   \
        {                                                  \
    	    /* Disable the channel. */                     \
		    PIT.CH[_channel].TCTRL.B.TEN = 0;              \
		    Gpt_Unit[_channel].state = GPT_STATE_STOPPED;  \
        }                                                  \
        config->GptNotification();                         \
                                                           \
        /* Clear interrupt. */                             \
        PIT.CH[_channel].TFLG.B.TIF = 1;                   \
	}
#else
#define GPT_ISR( _channel )                            \
  void Gpt_Isr_Channel##_channel( void )        \
  {                                                    \
    const Gpt_ConfigType *config;                      \
    int confCh;                                        \
                                                       \
    /* Find the config entry for the PIT channel. */   \
    confCh = Gpt_Global.channelMap[ _channel ];        \
    assert(confCh != GPT_CHANNEL_ILL);                 \
    config = &Gpt_Global.config[ confCh ];             \
                                                       \
    if( config->GptChannelMode == GPT_MODE_ONESHOT )   \
    {                                                  \
      /* Disable the channel. */                       \
      PIT.EN.R &= ~( 1 << _channel );                  \
                                                       \
      Gpt_Unit[_channel].state = GPT_STATE_STOPPED;    \
    }                                                  \
    config->GptNotification();                         \
                                                       \
    /* Clear interrupt. */                             \
    PIT.FLG.R = ( 1 << _channel );                     \
  }
#endif

#define STR__(x)	#x
#define XSTR__(x) STR__(x)

/*
 * Create instances of the ISR for each PIT channel.
 */
GPT_ISR( 0 )
GPT_ISR( 1 )
GPT_ISR( 2 )
#if !defined(CFG_MPC5604P)
GPT_ISR( 3 )
#if !defined(CFG_MPC5606S) && !defined(CFG_MPC563XM)
GPT_ISR( 4 )
GPT_ISR( 5 )
#if ! (defined(CFG_MPC5604B) || defined (CFG_MPC5602B))
GPT_ISR( 6 )
GPT_ISR( 7 )
#if !defined(CFG_MPC5606B)
GPT_ISR( 8 )
#endif
#endif
#endif
#endif

#if defined(CFG_MPC560X) || defined(CFG_MPC563XM)
	#if defined(CFG_MPC5606S) || defined(CFG_MPC563XM)
		#define GPT_CHANNEL_PIT_LAST GPT_CHANNEL_PIT_3
	#elif defined(CFG_MPC5604P)
		#define GPT_CHANNEL_PIT_LAST GPT_CHANNEL_PIT_2
    #elif defined(CFG_MPC5604B) || defined (CFG_MPC5602B)
	    #define GPT_CHANNEL_PIT_LAST GPT_CHANNEL_PIT_5
    #elif defined(CFG_MPC5606B)
	    #define GPT_CHANNEL_PIT_LAST GPT_CHANNEL_PIT_7
	#endif
#else
	#define GPT_CHANNEL_PIT_LAST GPT_CHANNEL_PIT_8
	#define PIT_INT0 PIT_PITFLG_RTIF
	#define PIT_INT1 PIT_PITFLG_PIT1
	#define PIT_INT2 PIT_PITFLG_PIT2
	#define PIT_INT3 PIT_PITFLG_PIT3
	#define PIT_INT4 PIT_PITFLG_PIT4
	#define PIT_INT5 PIT_PITFLG_PIT5
	#define PIT_INT6 PIT_PITFLG_PIT6
	#define PIT_INT7 PIT_PITFLG_PIT7
#endif

//-------------------------------------------------------------------

void Gpt_Init(const Gpt_ConfigType *config) {
	/** @req 3.1.5/GPT280 */
	/** @req 3.1.5/GPT006 */
	/** !req 3.1.5/GPT272 Post build not supported */
	/** @req 3.1.5/GPT107 The module is disabled */
	/** @req 3.1.5/GPT068 Global registers must be touched */
	/** @req 3.1.5/GPT205 */
	/** !req 3.1.5/GPT294 Post build not supported */
	/** !req 3.1.5/GPT309 TODO: Check*/

	uint32_t i = 0;
	const Gpt_ConfigType *cfg;
	/** @req 3.1.5/GPT307 */
	VALIDATE( (Gpt_Global.initRun == STD_OFF), GPT_INIT_SERVICE_ID, GPT_E_ALREADY_INITIALIZED );
#if defined(GPT_VARIANT_PB)
	VALIDATE( (config != NULL ), GPT_INIT_SERVICE_ID, GPT_E_PARAM_CONFIG );
#elif 	defined(GPT_VARIANT_PC)
	// We don't support GPT_VARIANT_PC
	assert(0);
#endif
	Gpt_ChannelType ch;

	for (i = 0; i < GPT_CHANNEL_CNT; i++) {
		Gpt_Global.channelMap[i] = GPT_CHANNEL_ILL;
	    Gpt_Unit[i].state = GPT_STATE_STOPPED;
	}

	i = 0;
	cfg = config;
	while (cfg->GptChannelId != GPT_CHANNEL_ILL) {
		ch = cfg->GptChannelId;

		// Assign the configuration channel used later..
		Gpt_Global.channelMap[cfg->GptChannelId] = i;
		Gpt_Global.configured |= (1 << ch);

		if (ch <= GPT_CHANNEL_PIT_LAST)
		{
			if (cfg->GptNotification != NULL)
			{
				switch( ch )
				{
					case 0: ISR_INSTALL_ISR2( "Gpt_0", Gpt_Isr_Channel0, PIT_INT0, 2, 0 ); break;
					case 1: ISR_INSTALL_ISR2( "Gpt_1", Gpt_Isr_Channel1, PIT_INT1, 2, 0 ); break;
					case 2: ISR_INSTALL_ISR2( "Gpt_2", Gpt_Isr_Channel2, PIT_INT2, 2, 0 ); break;
#if !defined(CFG_MPC5604P)
					case 3: ISR_INSTALL_ISR2( "Gpt_3", Gpt_Isr_Channel3, PIT_INT3, 2, 0 ); break;
#if !defined(CFG_MPC5606S) && !defined(CFG_MPC5634M)
					case 4: ISR_INSTALL_ISR2( "Gpt_4", Gpt_Isr_Channel4, PIT_INT4, 2, 0 ); break;
					case 5: ISR_INSTALL_ISR2( "Gpt_5", Gpt_Isr_Channel5, PIT_INT5, 2, 0 ); break;
#if ! (defined(CFG_MPC5604B) || defined (CFG_MPC5602B))
					case 6: ISR_INSTALL_ISR2( "Gpt_6", Gpt_Isr_Channel6, PIT_INT6, 2, 0 ); break;
					case 7: ISR_INSTALL_ISR2( "Gpt_7", Gpt_Isr_Channel7, PIT_INT7, 2, 0 ); break;
#if !defined(CFG_MPC5606B)
					case 8: ISR_INSTALL_ISR2( "Gpt_8", Gpt_Isr_Channel8, PIT_PITFLG_PIT8, 2, 0 );break;
#endif
#endif
#endif
#endif
					default:
					{
						// Unknown PIT channel.
						assert( 0 );
						break;
					}
				}
			}
		}
		cfg++;
		i++;
	}

	Gpt_Global.config = config;

	Gpt_Global.initRun = STD_ON;

#if defined(CFG_MPC560X) || defined(CFG_MPC563XM)
	for(i = 0; i <= GPT_CHANNEL_PIT_LAST; i++)
	{
		PIT.CH[i].TCTRL.B.TIE = 0;
	}
	PIT.MCR.B.MDIS = 0;
	PIT.MCR.B.FRZ = 1;
#else
	/* @req 3.1.5|GPT258 */
	PIT.CTRL.B.MDIS = 0;
#endif

}

//-------------------------------------------------------------------

/** @req 3.1.5/GPT194 */
#if GPT_DEINIT_API == STD_ON

void Gpt_DeInit(void)
{
	/** @req 3.1.5/GPT281 */
	/** !req 3.1.5/GPT008 TODO: Shouldn't MDIS be set to 1 */
	/** @req 3.1.5/GPT161 */
	/** @req 3.1.5/GPT105 */
	/** @req 3.1.5/GPT162 */
	/** !req 3.1.5/GPT308 Post build not supported */
	/** !req 3.1.5/GPT234 It has sideaffects now */

	Gpt_ChannelType channel;

	/* @req 3.1.5/GPT220 */
	VALIDATE( (Gpt_Global.initRun == STD_ON), GPT_DEINIT_SERVICE_ID, GPT_E_UNINIT );
	for (channel=0; channel<GPT_CHANNEL_CNT; channel++) // Validate that all channels have been stopped
	{
		VALIDATE( (Gpt_Unit[channel].state == GPT_STATE_STOPPED), GPT_DEINIT_SERVICE_ID, GPT_E_BUSY );
	}
	Gpt_Global.initRun = STD_OFF;
	Gpt_Global.configured = 0;
}
#endif

//-------------------------------------------------------------------
// period is in "ticks" !!
void Gpt_StartTimer(Gpt_ChannelType channel, Gpt_ValueType period_ticks) {
#if !defined(CFG_MPC560X) && !defined(CFG_MPC563XM)
	uint32_t tmp;
#endif
	int confCh;

	VALIDATE( (Gpt_Global.initRun == STD_ON), GPT_STARTTIMER_SERVICE_ID, GPT_E_UNINIT );
	VALIDATE( VALID_CHANNEL(channel), GPT_STARTTIMER_SERVICE_ID, GPT_E_PARAM_CHANNEL );
	VALIDATE( (Gpt_Unit[channel].state == GPT_STATE_STOPPED), GPT_STARTTIMER_SERVICE_ID, GPT_E_BUSY );
	// GPT_E_PARAM_VALUE, all have 32-bit so no need to check

	DEBUG(DEBUG_HIGH, "Gpt_StartTimer ch=%d, period=%d [ticks]\n", channel, period_ticks);

	confCh = Gpt_Global.channelMap[channel];
#if defined(CFG_MPC560X) || defined(CFG_MPC563XM)
	if (channel <= GPT_CHANNEL_PIT_LAST)
	{
		PIT.CH[channel].LDVAL.R = period_ticks;

		// Make sure that no interrupt is pending.
		PIT.CH[channel].TFLG.B.TIF = 1;
		// Enable timer
		PIT.CH[channel].TCTRL.B.TEN = 1;
	}
#else
	if (channel <= GPT_CHANNEL_PIT_LAST) {
		uint32 *tlval = (uint32 *) &PIT.TLVAL0;
		uint32 *tval = (uint32 *) &PIT.TVAL0;

		tlval[channel] = period_ticks;
		tval[channel] = period_ticks;

		// always select interrupt
		if (channel != GPT_CHANNEL_RTI) {
			PIT.INTSEL.R |= (1 << channel);
		}

		// Make sure that no interrupt is pending.
		PIT.FLG.R = (1 << channel);

		// Enable timer
		PIT.EN.R |= (1 << channel);
	} else if (channel == GPT_CHANNEL_DEC) {
		// Enable the TB
		tmp = get_spr(SPR_HID0);
		tmp |= HID0_TBEN;
		set_spr(SPR_HID0,tmp);

		/* Initialize the Decrementer */
		set_spr(SPR_DEC, period_ticks);
		set_spr(SPR_DECAR, period_ticks);

		if (Gpt_Global.config[confCh].GptChannelMode == GPT_MODE_CONTINUOUS) {
			/* Set autoreload */
			tmp = get_spr(SPR_TCR);
			tmp |= TCR_ARE;
			set_spr(SPR_TCR,tmp);
		}
	}
#endif
	if (Gpt_Global.config[confCh].GptNotification != NULL) {
		// GPT275
		Gpt_EnableNotification(channel);
	}

	Gpt_Unit[channel].state = GPT_STATE_STARTED;
}

void Gpt_StopTimer(Gpt_ChannelType channel) {

	VALIDATE( (Gpt_Global.initRun == STD_ON), GPT_STOPTIMER_SERVICE_ID, GPT_E_UNINIT ); VALIDATE( VALID_CHANNEL(channel), GPT_STOPTIMER_SERVICE_ID, GPT_E_PARAM_CHANNEL );

#if defined(CFG_MPC560X) || defined(CFG_MPC563XM)
	if (channel <= GPT_CHANNEL_PIT_LAST)
	{
		// Disable timer
		PIT.CH[channel].TCTRL.B.TEN = 0;
	}
#else
	if (channel <= GPT_CHANNEL_PIT_LAST) {
		// Disable timer
		PIT.EN.R &= ~(1 << channel);
	} else if (channel == GPT_CHANNEL_DEC) {
		uint32 tb;
		tb = get_spr(SPR_HID0);
		tb &= ~HID0_TBEN;
		set_spr(SPR_HID0,tb);
	}
#endif
	Gpt_DisableNotification(channel);
	Gpt_Unit[channel].state = GPT_STATE_STOPPED;
}

#if ( GPT_TIME_REMAINING_API == STD_ON )

Gpt_ValueType Gpt_GetTimeRemaining(Gpt_ChannelType channel)
{
	VALIDATE_W_RV( (Gpt_Global.initRun == STD_ON), GPT_GETTIMEREMAINING_SERVICE_ID, GPT_E_UNINIT, 0 );
	VALIDATE_W_RV( VALID_CHANNEL(channel),GPT_GETTIMEREMAINING_SERVICE_ID, GPT_E_PARAM_CHANNEL, 0 );
	VALIDATE_W_RV( (Gpt_Unit[channel].state == GPT_STATE_STARTED), GPT_GETTIMEREMAINING_SERVICE_ID, GPT_E_NOT_STARTED, 0 );
	Gpt_ValueType remaining;

#if defined(CFG_MPC560X) || defined(CFG_MPC563XM)
	if (channel <= GPT_CHANNEL_PIT_LAST)
	{
		// Time remaining is the time until it hits 0, so just return the current timer value
		remaining = PIT.CH[channel].CVAL.R;
	}
#else
	if (channel <= GPT_CHANNEL_PIT_LAST)
	{
		uint32 *tval = (uint32 *)&PIT.TVAL0;
		// Time remaining is the time until it hits 0, so just return the current timer value
		remaining = tval[channel];
	}
	else if (channel == GPT_CHANNEL_DEC)
	{
		remaining = get_spr(SPR_DEC);
	}
	else
	{
		/* We have written a fault in the fault log. Return 0. */
		remaining = 0;
	}
#endif

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

#if defined(CFG_MPC560X) || defined(CFG_MPC563XM)
	if (channel <= GPT_CHANNEL_PIT_LAST)
	{
		uint32 tval = PIT.CH[channel].CVAL.R;
		uint32 tlval = PIT.CH[channel].LDVAL.R;
		timer = tlval - tval;
	}
	else
	{
		timer = 0;
	}
#else
	if (channel <= GPT_CHANNEL_PIT_LAST)
	{
		uint32 *tval = (uint32 *)&PIT.TVAL0;
		uint32 *tlval = (uint32 *)&PIT.TLVAL0;
		timer = tlval[channel] - tval[channel];
	}
	else if (channel == GPT_CHANNEL_DEC)
	{
		timer = get_spr(SPR_DECAR) - get_spr(SPR_DEC);
	}
	else
	{
		/* We have written a fault in the fault log. Return 0. */
		timer = 0;
	}
#endif

	return (timer);
}
#endif

#if ( GPT_ENABLE_DISABLE_NOTIFICATION_API == STD_ON )
void Gpt_EnableNotification(Gpt_ChannelType channel)
{

	VALIDATE( (Gpt_Global.initRun == STD_ON), 0x7, GPT_E_UNINIT );
	VALIDATE( VALID_CHANNEL(channel),0x7, GPT_E_PARAM_CHANNEL );

#if defined(CFG_MPC560X) || defined(CFG_MPC563XM)
	if (channel <= GPT_CHANNEL_PIT_LAST)
	{
		PIT.CH[channel].TCTRL.B.TIE = 1;
	}
#else
	if (channel <= GPT_CHANNEL_PIT_LAST)
	{
		// enable interrupts
		PIT.INTEN.R |= ( 1 << channel );
	}
	else if (channel == GPT_CHANNEL_DEC)
	{
		uint32 tmp;
		tmp = get_spr(SPR_TCR);
		tmp |= TCR_DIE;
		set_spr(SPR_TCR, tmp );
	}
#endif
}

void Gpt_DisableNotification(Gpt_ChannelType channel)
{

	VALIDATE( (Gpt_Global.initRun == STD_ON), 0x8, GPT_E_UNINIT );
	VALIDATE( VALID_CHANNEL(channel),0x8, GPT_E_PARAM_CHANNEL );

#if defined(CFG_MPC560X) || defined(CFG_MPC563XM)
	if (channel <= GPT_CHANNEL_PIT_LAST)
	{
		PIT.CH[channel].TCTRL.B.TIE = 0;
	}
#else
	if (channel <= GPT_CHANNEL_PIT_LAST)
	{
		PIT.INTEN.R &= ~( 1 << channel );
	}
	else if (channel == GPT_CHANNEL_DEC)
	{
		uint32 tmp;
		tmp = get_spr(SPR_TCR);
		tmp &= ~TCR_DIE;
		set_spr(SPR_TCR, tmp );
	}
#endif
	return;
}

#endif

#if ( GPT_WAKEUP_FUNCTIONALITY_API == STD_ON )

void Gpt_SetMode(Gpt_ModeType mode)
{
	int i;

	VALIDATE( (Gpt_Global.initRun == STD_ON), GPT_SETMODE_SERVIVCE_ID, GPT_E_UNINIT );
	VALIDATE( ( mode <= GPT_MODE_SLEEP ), GPT_SETMODE_SERVIVCE_ID, GPT_E_PARAM_MODE );

#if defined(CFG_MPC560X) || defined(CFG_MPC563XM)
	if (mode == GPT_MODE_NORMAL)
	{
		PIT.PITMCR.B.MDIS = 0;
		// Do NOT restart channels
	}
	else if (mode == GPT_MODE_SLEEP)
	{
		PIT.PITMCR.B.MDIS = 0;
		// Disable all but RTI
		for (i= 0; i <= GPT_CHANNEL_PIT_LAST; i++)
		{
			Gpt_StopTimer(i);
		}
	}
#else
	if (mode == GPT_MODE_NORMAL)
	{
		PIT.CTRL.B.MDIS = 0;
		// Do NOT restart channels
	}
	else if (mode == GPT_MODE_SLEEP)
	{

		PIT.CTRL.B.MDIS = 1;
		// Disable all but RTI
		for (i= 0; i <= GPT_CHANNEL_PIT_LAST; i++)
		{
			Gpt_StopTimer(i);
		}
	}
#endif
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



