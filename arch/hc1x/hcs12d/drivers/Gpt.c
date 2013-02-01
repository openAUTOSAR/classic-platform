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
#include "regs.h"
#include "Mcu.h"
#include "debug.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#include "Os.h"
#include "arc.h"
#include "isr.h"

#define FIRST_OC_REG 0x50

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
  uint8 initRun;

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


// Holds the enable/disable status of notifications.
uint8 GptNotificationStatuses[8] = {
	STD_ON,
	STD_ON,
	STD_ON,
	STD_ON,
	STD_ON,
	STD_ON,
	STD_ON,
	STD_ON
};

uint16 GptPeriods[GPT_CHANNEL_CNT];

#if (GPT_TIME_ELAPSED_API == STD_ON)
uint16 GptPrevOc[GPT_CHANNEL_CNT];
#endif


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

  if (GptNotificationStatuses[channel] == STD_ON) {
  	 config->GptNotification();
  }

  if (config->GptChannelMode == GPT_MODE_ONESHOT)
  {
    // Disable the channel
	Gpt_StopTimer(channel);
    Gpt_Unit[channel].state = GPT_STATE_STOPPED;

  } else {
	  // Start the next period.
	  uint16 curr_oc = PORTIO_16((FIRST_OC_REG + (2 * channel)));
	  PORTIO_16((FIRST_OC_REG + (2*channel))) = curr_oc + GptPeriods[channel];

	  #if (GPT_TIME_ELAPSED_API == STD_ON)
	  GptPrevOc[channel] = curr_oc;
	  #endif
  }

}

//-------------------------------------------------------------------
// Macro that counts leading zeroes.
#define CNTLZW_INV(x) (31-cntlzw(x))


/**
 * ISR that handles all interrupts to the PIT channels
 * ( NOT the decrementer )
 */
#define DECLARE_GPT_ISR_PROTOTYPE(x)			\
void Gpt_Isr_##x (void) {   					\
	Gpt_IsrCh(x);								\
}

#define GPT_IRQ_TYPE(x) IRQ_TYPE_TC(x)

DECLARE_GPT_ISR_PROTOTYPE(0)
DECLARE_GPT_ISR_PROTOTYPE(1)
DECLARE_GPT_ISR_PROTOTYPE(2)
DECLARE_GPT_ISR_PROTOTYPE(3)
DECLARE_GPT_ISR_PROTOTYPE(4)
DECLARE_GPT_ISR_PROTOTYPE(5)
DECLARE_GPT_ISR_PROTOTYPE(6)
DECLARE_GPT_ISR_PROTOTYPE(7)


//-------------------------------------------------------------------

void Gpt_Init(const Gpt_ConfigType *config)
{
  uint32_t i=0;
  const Gpt_ConfigType *cfg;
  VALIDATE( (Gpt_Global.initRun == STD_OFF), GPT_INIT_SERVICE_ID, GPT_E_ALREADY_INITIALIZED );
  VALIDATE( (config != NULL ), GPT_INIT_SERVICE_ID, GPT_E_PARAM_CONFIG );

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

    if (ch <= GPT_CHANNEL_7)
    {
      if (cfg->GptNotification != NULL)
      {
    	switch (ch) {
    	case 0:
        	ISR_INSTALL_ISR2("Gpt_Isr_0",Gpt_Isr_0,IRQ_NR_TC0,1,0);
    		break;
    	case 1:
        	ISR_INSTALL_ISR2("Gpt_Isr_1",Gpt_Isr_1,IRQ_NR_TC1,1,0);
    	    break;
    	case 2:
	    	ISR_INSTALL_ISR2("Gpt_Isr_2",Gpt_Isr_2,IRQ_NR_TC2,1,0);
			break;
    	case 3:
	    	ISR_INSTALL_ISR2("Gpt_Isr_3",Gpt_Isr_3,IRQ_NR_TC3,1,0);
			break;
    	case 4:
	    	ISR_INSTALL_ISR2("Gpt_Isr_4",Gpt_Isr_4,IRQ_NR_TC4,1,0);
			break;
    	case 5:
	    	ISR_INSTALL_ISR2("Gpt_Isr_5",Gpt_Isr_5,IRQ_NR_TC5,1,0);
			break;
    	case 6:
        	ISR_INSTALL_ISR2("Gpt_Isr_6",Gpt_Isr_6,IRQ_NR_TC6,1,0);
    		break;
    	case 7:
        	ISR_INSTALL_ISR2("Gpt_Isr_7",Gpt_Isr_7,IRQ_NR_TC7,1,0);
    	    break;
    	}
      }
    }

    cfg++;
    i++;
  }

  Gpt_Global.config = config;

  Gpt_Global.initRun = STD_ON;

  TSCR1 |= TEN; // Turn timer on.
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
  TSCR1 &= ~TEN; // Turn timer off.
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

  DEBUG(DEBUG_HIGH, "Gpt_StartTimer ch=%d, period=%d [ticks]\n", channel, period_ticks);

  confCh = Gpt_Global.channelMap[channel];

  if (channel <= GPT_CHANNEL_7) {



	// Setup channel for output compare (OC).
	TIOS |= (1 << channel);

	// Enable interrupt for timer
	TIE |= (1 << channel);

	// Set OC value.
	uint16 curr_cnt = TCNT;
	PORTIO_16((FIRST_OC_REG + (2*channel))) = curr_cnt + period_ticks;

	#if (GPT_TIME_ELAPSED_API == STD_ON)
	GptPrevOc[channel] = curr_cnt;
	#endif
  }

  #if ( GPT_ENABLE_DISABLE_NOTIFICATION_API == STD_ON )
  if( Gpt_Global.config[confCh].GptNotification != NULL )
  {
    // GPT275
    Gpt_EnableNotification(channel);
  }
  #endif

  GptPeriods[channel] = period_ticks;
  Gpt_Unit[channel].state = GPT_STATE_STARTED;
}

void Gpt_StopTimer(Gpt_ChannelType channel)
{

  VALIDATE( (Gpt_Global.initRun == STD_ON), GPT_STOPTIMER_SERVICE_ID, GPT_E_UNINIT );
  VALIDATE( VALID_CHANNEL(channel), GPT_STOPTIMER_SERVICE_ID, GPT_E_PARAM_CHANNEL );

  if (channel <= GPT_CHANNEL_7)
  {
	 TIE &= ~(1 << channel);
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
  Gpt_ValueType remaining = 0;

  if (channel <= GPT_CHANNEL_7)
  {
	  uint16 now = TCNT;
	  uint16 next = PORTIO_16((FIRST_OC_REG + (2*channel)));
	  if (next > now) {
		  remaining = next - now;

	  } else {
		  remaining = next + (0xFF - now) ;
	  }
  }

  return remaining;
}
#endif

#if ( GPT_TIME_ELAPSED_API == STD_ON )
Gpt_ValueType Gpt_GetTimeElapsed(Gpt_ChannelType channel)
{
  Gpt_ValueType elapsed = 0;

  VALIDATE_W_RV( (Gpt_Global.initRun == STD_ON), GPT_GETTIMEELAPSED_SERVICE_ID, GPT_E_UNINIT ,0 );
  VALIDATE_W_RV( VALID_CHANNEL(channel),GPT_GETTIMEELAPSED_SERVICE_ID, GPT_E_PARAM_CHANNEL, 0 );
  VALIDATE_W_RV( (Gpt_Unit[channel].state == GPT_STATE_STARTED),GPT_GETTIMEELAPSED_SERVICE_ID, GPT_E_NOT_STARTED, 0 );

  if (channel <= GPT_CHANNEL_7) {
    uint16 now = TCNT;
    if (now > GptPrevOc[channel]) {
    	elapsed = now - GptPrevOc[channel];

    } else {
    	elapsed = (0xFF - GptPrevOc[channel]) + now;
    }
  }

  return (elapsed);
}
#endif

#if ( GPT_ENABLE_DISABLE_NOTIFICATION_API == STD_ON )
void Gpt_EnableNotification(Gpt_ChannelType channel)
{

  VALIDATE( (Gpt_Global.initRun == STD_ON), 0x7, GPT_E_UNINIT );
  VALIDATE( VALID_CHANNEL(channel),0x7, GPT_E_PARAM_CHANNEL );

  if (channel <= GPT_CHANNEL_7)
  {
	  GptNotificationStatuses[channel] = STD_ON;
  }
}

void Gpt_DisableNotification(Gpt_ChannelType channel)
{
  VALIDATE( (Gpt_Global.initRun == STD_ON), 0x8, GPT_E_UNINIT );
  VALIDATE( VALID_CHANNEL(channel),0x8, GPT_E_PARAM_CHANNEL );

  if (channel <= GPT_CHANNEL_7)
  {
	  GptNotificationStatuses[channel] = STD_OFF;
  }
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

  }
  else if (mode == GPT_MODE_SLEEP)
  {

  }
}

void Gpt_DisableWakeup(Gpt_ChannelType channel)
{
  VALIDATE( (Gpt_Global.initRun == STD_ON), GPT_DISABLEWAKEUP_SERVICE_ID, GPT_E_UNINIT );
  VALIDATE( VALID_CHANNEL(channel), GPT_DISABLEWAKEUP_SERVICE_ID, GPT_E_PARAM_CHANNEL );


}

void Gpt_EnableWakeup(Gpt_ChannelType channel)
{
  VALIDATE( (Gpt_Global.initRun == STD_ON), GPT_ENABLEWAKEUP_SERVICE_ID, GPT_E_UNINIT );
  VALIDATE( VALID_CHANNEL(channel),GPT_ENABLEWAKEUP_SERVICE_ID, GPT_E_PARAM_CHANNEL );

}

void Gpt_Cbk_CheckWakeup(EcuM_WakeupSourceType wakeupSource)
{

}

#endif

