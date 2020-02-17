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

#ifndef PWM_CONFIGTYPES_H_
#define PWM_CONFIGTYPES_H_
#if defined(CFG_STM32F1X)
typedef uint16 Pwm_PeriodType;

typedef enum {
  PWM_CHANNEL_11 = 0, // TIM1 Channel 1
  PWM_CHANNEL_12,
  PWM_CHANNEL_13,
  PWM_CHANNEL_14,
  PWM_CHANNEL_21,     // TIM2 Channel 1
  PWM_CHANNEL_22,
  PWM_CHANNEL_23,
  PWM_CHANNEL_24,
  PWM_CHANNEL_31,     // TIM3 Channel 1
  PWM_CHANNEL_32,
  PWM_CHANNEL_33,
  PWM_CHANNEL_34,
  PWM_CHANNEL_41,     // TIM4 Channel 1
  PWM_CHANNEL_42,
  PWM_CHANNEL_43,
  PWM_CHANNEL_44,
  PWM_TOTAL_NOF_CHANNELS,
} Pwm_ChannelType;

typedef enum {
    PWM_CHANNEL_PRESCALER_1=0,
    PWM_CHANNEL_PRESCALER_2,
    PWM_CHANNEL_PRESCALER_3,
    PWM_CHANNEL_PRESCALER_4,
} Pwm_ChannelPrescalerType;
#define DUTY_AND_PERIOD(_duty,_period) .duty = (_duty*_period)>>15, .period = _period

typedef struct {
	/* Number of duty ticks */
	uint32_t                 duty:32;
	/* Length of period, in ticks */
	uint32_t                 period:32;
	/* Counter */
	uint32_t                 counter:32;
	/* Enable freezing the channel when in debug mode */
	uint32_t                 freezeEnable:1;
	/* Disable output */
	uint32_t                 outputDisable:1;
	/* Select which bus disables the bus
	 * NOTE: Figure out how this works, i.e. what bus does it refer to? */
	uint32_t 				 outputDisableSelect:2;
	/* Prescale the emios clock some more? */
	uint32_t                 prescaler:2;
	/* Prescale the emios clock some more? */
	uint32_t				 usePrescaler:1;
	/* Whether to use DMA. Currently unsupported */
	uint32_t				 useDma:1;
	uint32_t				 reserved_2:1;
	/* Input filter. Ignored in output mode. */
	uint32_t				 inputFilter:4;
	/* Input filter clock source. Ignored in output mode */
	uint32_t				 filterClockSelect:1;
	/* Enable interrupts/flags on this channel? Required for DMA as well. */
	uint32_t				 flagEnable:1;
	uint32_t				 reserved_3:3;
	/* Trigger a match on channel A */
	uint32_t				 forceMatchA:1;
	/* Triggers a match on channel B */
	uint32_t				 forceMatchB:1;
	uint32_t				 reserved_4:1;
	/* We can use different buses for the counter. Use the internal counter */
	uint32_t				 busSelect:2;
	/* What edges to flag on? */
	uint32_t				 edgeSelect:1;
	/* Polarity of the channel */
	uint32_t				 edgePolarity:1;
	/* EMIOS mode. 0x58 for buffered output PWM */
	uint32_t				 mode:7;
} Pwm_ChannelRegisterType;

typedef struct {
    Pwm_ChannelRegisterType r;
    Pwm_ChannelType channel;
} Pwm_ChannelConfigurationType;



// Channel configuration macro.
#define PWM_CHANNEL_CONFIG(_hwchannel, _period, _duty, _prescaler, _polarity) \
    {\
        .channel = _hwchannel,\
        .r = {\
            DUTY_AND_PERIOD(_duty, _period),\
            .freezeEnable = 1u,\
            .outputDisable = 0u,\
            .usePrescaler = 1u,\
            .prescaler = _prescaler,\
            .useDma = 0u,\
            .flagEnable = 0u, /* See PWM052 */ \
            .busSelect = 3u, /* Use the internal counter bus */\
            .edgePolarity =  _polarity,\
            .mode = 0u\
        }\
    }
#elif defined(CFG_JACINTO)

// Channel configuration macro.
#define PWM_CHANNEL_CONFIG(_hwchannel, _frequency, _duty, _divisor, _polarity, _idleState, _class) \
    {\
        .channel = _hwchannel,\
        .frequency = _frequency,\
        .duty = _duty,  \
        .divisor = _divisor,\
        .polarity = _polarity,\
        .idleState = _idleState,\
        .class = _class\
    }

#endif
#endif /* PWM_CONFIGTYPES_H_ */
