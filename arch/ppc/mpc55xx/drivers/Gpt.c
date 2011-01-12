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
#include "mpc55xx.h"
#include "Mcu.h"
#include "debug.h"
#include "Det.h"
#if defined(USE_KERNEL)
#include "Os.h"
#include "irq.h"
#endif

// Implementation specific

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

/*
 * ISR for a given PIT channel (macro)
 */
#define GPT_ISR( _channel )                            \
  static void Gpt_Isr_Channel##_channel( void )        \
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

#define STR__(x)	#x
#define XSTR__(x) STR__(x)

/*
 * Create instances of the ISR for each PIT channel.
 */
GPT_ISR( 0 );
GPT_ISR( 1 );
GPT_ISR( 2 );
GPT_ISR( 3 );
GPT_ISR( 4 );
GPT_ISR( 5 );
GPT_ISR( 6 );
GPT_ISR( 7 );
GPT_ISR( 8 );

#define GPT_ISR_INSTALL( _channel, _prio )													\
{																					\
	TaskType tid;																	\
	tid = Os_Arc_CreateIsr(Gpt_Isr_Channel##_channel, _prio, XSTR__(Gpt_##_channel));	\
	Irq_AttachIsr2(tid, NULL, PIT_PITFLG_RTIF + _channel);							\
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

    if (ch <= GPT_CHANNEL_PIT_8)
    {
      if (cfg->GptNotification != NULL)
      {
        switch( ch )
        {
          case 0: GPT_ISR_INSTALL( 0, cfg->GptNotificationPriority ); break;
          case 1: GPT_ISR_INSTALL( 1, cfg->GptNotificationPriority ); break;
          case 2: GPT_ISR_INSTALL( 2, cfg->GptNotificationPriority ); break;
          case 3: GPT_ISR_INSTALL( 3, cfg->GptNotificationPriority ); break;
          case 4: GPT_ISR_INSTALL( 4, cfg->GptNotificationPriority ); break;
          case 5: GPT_ISR_INSTALL( 5, cfg->GptNotificationPriority ); break;
          case 6: GPT_ISR_INSTALL( 6, cfg->GptNotificationPriority ); break;
          case 7: GPT_ISR_INSTALL( 7, cfg->GptNotificationPriority ); break;
          case 8: GPT_ISR_INSTALL( 8, cfg->GptNotificationPriority ); break;
          default:
          {
            // Unknown PIT channel.
            assert( 0 );
            break;
          }
        }
      }
    }
#if defined(USE_KERNEL)
    // Don't install if we use kernel.. it handles that.
#else
    else if (ch == GPT_CHANNEL_DEC)
    {
      // Decrementer event is default an exception. Use software interrupt 7 as wrapper.
      Irq_InstallVector(config[i].GptNotification, INTC_SSCIR0_CLR7, 1, CPU_Z1);
    }
#endif

    cfg++;
    i++;
  }

  Gpt_Global.config = config;

  Gpt_Global.initRun = STD_ON;
  PIT.CTRL.B.MDIS = 0;
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
  uint32_t tmp;
  int confCh;

  VALIDATE( (Gpt_Global.initRun == STD_ON), GPT_STARTTIMER_SERVICE_ID, GPT_E_UNINIT );
  VALIDATE( VALID_CHANNEL(channel), GPT_STARTTIMER_SERVICE_ID, GPT_E_PARAM_CHANNEL );
  VALIDATE( (Gpt_Unit[channel].state == GPT_STATE_STOPPED), GPT_STARTTIMER_SERVICE_ID, GPT_E_BUSY );
  // GPT_E_PARAM_VALUE, all have 32-bit so no need to check

  DEBUG(DEBUG_HIGH, "Gpt_StartTimer ch=%d, period=%d [ticks]\n", channel, period_ticks);

  confCh = Gpt_Global.channelMap[channel];

  if (channel <= GPT_CHANNEL_PIT_8)
  {
    uint32 *tlval = (uint32 *)&PIT.TLVAL0;
    uint32 *tval = (uint32 *)&PIT.TVAL0;

    tlval[channel] = period_ticks;
    tval[channel] = period_ticks;

    // always select interrupt
    if (channel != GPT_CHANNEL_RTI)
    {
      PIT.INTSEL.R |= ( 1 << channel );
    }

    // Make sure that no interrupt is pending.
    PIT.FLG.R = ( 1 << channel );

    // Enable timer
    PIT.EN.R |= ( 1 << channel );
  }
  else if (channel == GPT_CHANNEL_DEC)
  {
    // Enable the TB
    tmp = get_spr(SPR_HID0);
    tmp |= HID0_TBEN;
    set_spr(SPR_HID0,tmp);

    /* Initialize the Decrementer */
    set_spr(SPR_DEC, period_ticks);
    set_spr(SPR_DECAR, period_ticks);

    if( Gpt_Global.config[confCh].GptChannelMode == GPT_MODE_CONTINUOUS )
    {
      /* Set autoreload */
      tmp = get_spr(SPR_TCR);
      tmp |= TCR_ARE;
      set_spr(SPR_TCR,tmp);
    }
  }

  if( Gpt_Global.config[confCh].GptNotification != NULL )
  {
    // GPT275
    Gpt_EnableNotification(channel);
  }

  Gpt_Unit[channel].state = GPT_STATE_STARTED;
}

void Gpt_StopTimer(Gpt_ChannelType channel)
{

  VALIDATE( (Gpt_Global.initRun == STD_ON), GPT_STOPTIMER_SERVICE_ID, GPT_E_UNINIT );
  VALIDATE( VALID_CHANNEL(channel), GPT_STOPTIMER_SERVICE_ID, GPT_E_PARAM_CHANNEL );

  if (channel <= GPT_CHANNEL_PIT_8)
  {
    // Disable timer
    PIT.EN.R &= ~( 1 << channel );
  }
  else if (channel == GPT_CHANNEL_DEC)
  {
    uint32 tb;
    tb = get_spr(SPR_HID0);
    tb &= ~HID0_TBEN;
    set_spr(SPR_HID0,tb);
  }

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

  if (channel <= GPT_CHANNEL_PIT_8)
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

  if (channel <= GPT_CHANNEL_PIT_8)
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

  return (timer);
}
#endif

#if ( GPT_ENABLE_DISABLE_NOTIFICATION_API == STD_ON )
void Gpt_EnableNotification(Gpt_ChannelType channel)
{

  VALIDATE( (Gpt_Global.initRun == STD_ON), 0x7, GPT_E_UNINIT );
  VALIDATE( VALID_CHANNEL(channel),0x7, GPT_E_PARAM_CHANNEL );

  if (channel <= GPT_CHANNEL_PIT_8)
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
}

void Gpt_DisableNotification(Gpt_ChannelType channel)
{

  VALIDATE( (Gpt_Global.initRun == STD_ON), 0x8, GPT_E_UNINIT );
  VALIDATE( VALID_CHANNEL(channel),0x8, GPT_E_PARAM_CHANNEL );

  if (channel <= GPT_CHANNEL_PIT_8)
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
    PIT.CTRL.B.MDIS = 0;
    // Do NOT restart channels
  }
  else if (mode == GPT_MODE_SLEEP)
  {

    PIT.CTRL.B.MDIS = 1;
    // Disable all but RTI
    for (i= 0; i <= GPT_CHANNEL_PIT_8; i++)
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

