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








/*
 * Pwm.c
 *
 * TODO: Implement DMA support for PWM
 * TODO: Test PWM for MPC5567 and MPC5554
 *
 *  Created on: 2009-jul-09
 *      Author: nian
 */

//#define USE_KERNEL

#include <assert.h>
#include <string.h>

#include "Pwm.h"
#include "MemMap.h"
//#include "SchM_Pwm.h"
#include "Det.h"
#ifdef CFG_MPC5516
#include "mpc5516.h"
#elif defined(CFG_MPC5567)
#include "mpc5567.h"
#endif
#if defined(USE_KERNEL)
#include "Os.h"
#endif
#include "Mcu.h"

#if PWM_DEV_EROR_DETECT==STD_ON
	#define PWM_VALIDATE(_exp, _errid) \
		if (!(_exp)) { \
			Pwm_ReportError(_errid); \
			return; \
		}
	#define Pwm_VALIDATE_CHANNEL(_ch) PWM_VALIDATE(_ch <= 15, PWM_E_PARAM_CHANNEL)
	#define Pwm_VALIDATE_INITIALIZED() PWM_VALIDATE(Pwm_ModuleState == PWM_STATE_INITIALIZED, PWM_E_UNINIT)
	#define Pwm_VALIDATE_UNINITIALIZED() PWM_VALIDATE(Pwm_ModuleState != PWM_STATE_INITIALIZED, PWM_E_ALREADY_INITIALIZED)
#else
	#define Pwm_VALIDATE_CHANNEL(ch)
	#define Pwm_VALIDATE_INITIALIZED()
	#define Pwm_VALIDATE_UNINITIALIZED()
#endif


typedef enum {
	PWM_STATE_UNINITIALIZED, PWM_STATE_INITIALIZED
} Pwm_ModuleStateType;

static Pwm_ModuleStateType Pwm_ModuleState = PWM_STATE_UNINITIALIZED;

// Run-time variables
typedef struct {
	Pwm_ChannelClassType Class;

	#if PWM_NOTIFICATION_SUPPORTED==STD_ON
		Pwm_NotificationHandlerType NotificationRoutine;
		Pwm_EdgeNotificationType NotificationState;
	#endif
} Pwm_ChannelStructType;

// We use Pwm_ChannelType as index here
Pwm_ChannelStructType ChannelRuntimeStruct[16];

/* Local functions */
void inline Pwm_InitChannel(Pwm_ChannelType Channel);
void inline Pwm_DeInitChannel(Pwm_ChannelType Channel);

#if PWM_NOTIFICATION_SUPPORTED==STD_ON
static void Pwm_Isr(void);
#endif

void Pwm_Init(const Pwm_ConfigType* ConfigPtr) {
    Pwm_ChannelType channel_iterator;

    Pwm_VALIDATE_UNINITIALIZED();
    #if PWM_DEV_EROR_DETECT==STD_ON
        /*
         * PWM046: If development error detection is enabled for the Pwm module,
         * the function Pwm_Init shall raise development error PWM_E_PARAM_CONFIG
         * if ConfigPtr is a null pointer.
         *
         * PWM120: For pre-compile and link-time configuration variants, a NULL
         * pointer shall be passed to the initialization routine. In this case the
         * check for this NULL pointer has to be omitted.
         */
        #if PWM_STATICALLY_CONFIGURED==OFF
            if (ConfigPtr == NULL) {
                Pwm_ReportError(PWM_E_PARAM_CONFIG);
                return;
            }
        #endif
    #endif

    #if PWM_NOTIFICATION_SUPPORTED==STD_ON
        // Create a task for our interrupt service routine.
        TaskType tid = Os_CreateIsr(Pwm_Isr, PWM_ISR_PRIORITY /*prio*/, "PwmIsr");
    #endif

    /* Clock scaler uses system clock (~64MHz) as source, so prescaler 64 => 1MHz. */
    EMIOS.MCR.B.GPRE = PWM_PRESCALER - 1;

    /* Enable eMIOS clock */
    EMIOS.MCR.B.GPREN = 1;

    /* Stop channels when in debug mode */
    EMIOS.MCR.B.FRZ = PWM_FREEZE_ENABLE;

    /* Use global time base */
    EMIOS.MCR.B.GTBE = 1;

    Pwm_ModuleState = PWM_STATE_INITIALIZED;

    for (channel_iterator = 0; channel_iterator < PWM_NUMBER_OF_CHANNELS; channel_iterator++) {
        Pwm_ChannelType channel = ConfigPtr->Channels[channel_iterator].channel;

        // Set up the registers in hw
        memcpy((void*) &EMIOS.CH[channel],
                (void*) &ConfigPtr->Channels[channel_iterator].r,
                sizeof(Pwm_ChannelRegisterType));

        #if PWM_NOTIFICATION_SUPPORTED==STD_ON
                /*
                 * PWM052: The function Pwm_Init shall disable all notifications.
                 *
                 * This is now implemented in the configuration macro.
                 */
                // Pwm_DisableNotification(channel);

                // Install ISR
                IntCtrl_AttachIsr2(tid, NULL, EMISOS200_FLAG_F0 + channel);
                ChannelRuntimeStruct[channel].NotificationRoutine
                        = ConfigPtr->NotificationHandlers[channel_iterator];
        #endif
    }
}

#if PWM_DEINIT_API==STD_ON

// TODO: Test that this function in fact turns the channel off.
void inline Pwm_DeInitChannel(Pwm_ChannelType Channel) {
	Pwm_VALIDATE_CHANNEL(Channel);
	Pwm_VALIDATE_INITIALIZED();

    Pwm_SetOutputToIdle(Channel);

    #ifdef CFG_MPC5516
        // Set the disable bit for this channel
        EMIOS.UCDIS.R |= (1 << (31 - Channel));
    #endif

    /*
     * PWM052: The function Pwm_DeInit shall disable all notifications.
     */
    #if PWM_NOTIFICATION_SUPPORTED==STD_ON
        Pwm_DisableNotification(Channel);
    #endif
}

void Pwm_DeInit() {
	/* TODO: Implement Pwm_DeInit() */
	Pwm_ChannelType channel_iterator;

	Pwm_VALIDATE_INITIALIZED();

	for (channel_iterator = 0; channel_iterator < PWM_NUMBER_OF_CHANNELS; channel_iterator++) {
		Pwm_DeInitChannel(channel_iterator);

	}

	// Disable module
	EMIOS.MCR.B.MDIS = 1;

	Pwm_ModuleState = PWM_STATE_UNINITIALIZED;
}
#endif

void Pwm_GetVersionInfo(Std_VersionInfoType* VersionInfo) {
	/* TODO: Implement Pwm_GetVersionInfo */
}

/*
 * PWM083: The function Pwm_SetPeriodAndDuty shall be pre compile time
 * changeable ON/OFF by the configuration parameter PwmSetPeriodAndDuty.
 */
#if PWM_SET_PERIOD_AND_DUTY==STD_ON
	void Pwm_SetPeriodAndDuty(Pwm_ChannelType Channel, Pwm_PeriodType Period,
			Pwm_DutyCycleType DutyCycle) {

		Pwm_VALIDATE_INITIALIZED();
		Pwm_VALIDATE_CHANNEL(Channel);
		PWM_VALIDATE(ChannelRuntimeStruct[Channel].Class == PWM_VARIABLE_PERIOD, PWM_E_PERIOD_UNCHANGEABLE);

		uint16 leading_edge_position = (uint16) (((uint32) Period
				* (uint32) DutyCycle) >> 15);



		/* Timer instant for leading edge */
		EMIOS.CH[Channel].CADR.R = leading_edge_position;

		/* Timer instant for the period to restart */
		EMIOS.CH[Channel].CBDR.R = Period;

	}
#endif


/**
 * PWM013: The function Pwm_SetDutyCycle shall set the duty cycle of the PWM
 * channel.
 *
 * @TODO: How to conform with PWM018: "The driver shall forbid the spike on the PWM output signal"?
 *
 * @param Channel PWM channel to use. 0 <= Channel < PWM_NUMBER_OF_CHANNELS <= 16
 * @param DutyCycle 0 <= DutyCycle <= 0x8000
 */
void Pwm_SetDutyCycle(Pwm_ChannelType Channel, Pwm_DutyCycleType DutyCycle) {

	uint16 leading_edge_position = (uint16) ((EMIOS.CH[Channel].CBDR.R
				* (uint32) DutyCycle) >> 15);


	Pwm_VALIDATE_INITIALIZED();
	Pwm_VALIDATE_CHANNEL(Channel);

	/* Timer instant for leading edge */

	/*
	 * PWM017: The function Pwm_SetDutyCycle shall update the duty cycle at
	 * the end of the period if supported by the implementation and configured
	 * with PwmDutycycleUpdatedEndperiod. [ This is achieved in hardware since
	 * the A and B registers are double buffered ]
	 *
	 * PWM014: The function Pwm_SetDutyCycle shall set the output state according
	 * to the configured polarity parameter [which is already set from
	 * Pwm_InitChannel], when the duty parameter is 0% [=0] or 100% [=0x8000].
	 */
	if (DutyCycle == Pwm_100_Procent || DutyCycle == Pwm_0_Procent) {
		EMIOS.CH[Channel].CADR.R = 0;

	} else {
		EMIOS.CH[Channel].CADR.R = leading_edge_position;

	}
}

void Pwm_SetOutputToIdle(Pwm_ChannelType Channel) {
	Pwm_VALIDATE_CHANNEL(Channel);
	Pwm_VALIDATE_INITIALIZED();

	/* TODO: Make Pwm_SetOutputToIdle sensitive to PwmIdleState (currently uses PwmPolarity) */
	EMIOS.CH[Channel].CADR.R = 0;
}

/*
 * PWM085: The function Pwm_GetOutputState shall be pre compile configurable
 * ON/OFF by the configuration parameter PwmGetOutputState
 */
#if PWM_GET_OUTPUT_STATE==STD_ON
	/*
	 * PWM022: The function Pwm_GetOutputState shall read the internal state
	 * of the PWM output signal and return it.
	 */
	Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType Channel) {



		// We need to return something, even in presence of errors
		if (Channel >= 16) {
			Pwm_ReportError(PWM_E_PARAM_CHANNEL);

			/*
			 * Accordingly to PWM025, we should return PWM_LOW on failure.
			 */
			return PWM_LOW;

		} else if (Pwm_ModuleState != PWM_STATE_INITIALIZED) {
			Pwm_ReportError(PWM_E_UNINIT);

			/*
			 * Accordingly to PWM025, we should return PWM_LOW on failure.
			 */
			return PWM_LOW;

		}

		return EMIOS.CH[Channel].CSR.B.UCOUT;

	}
#endif

#if PWM_NOTIFICATION_SUPPORTED==STD_ON
	void Pwm_DisableNotification(Pwm_ChannelType Channel) {
		Pwm_VALIDATE_CHANNEL(Channel);
		Pwm_VALIDATE_INITIALIZED();

		// Disable flags on this channel
		EMIOS.CH[Channel].CCR.B.FEN = 0;
	}

	void Pwm_EnableNotification(Pwm_ChannelType Channel,
			Pwm_EdgeNotificationType Notification) {
		Pwm_VALIDATE_CHANNEL(Channel);
		Pwm_VALIDATE_INITIALIZED();

		ChannelRuntimeStruct[Channel].NotificationState = Notification;

		// Enable flags on this channel
		EMIOS.CH[Channel].CCR.B.FEN = 1;
	}

	static void Pwm_Isr(void) {
		// Find out which channel that triggered the interrupt
#ifdef CFG_MPC5516
		uint32_t flagmask = EMIOS.GFLAG.R;
#elif defined(CFG_MPC5567)
		uint32_t flagmask = EMIOS.GFR.R;
#endif

		// There are 24 channels specified in the global flag register, but
		// we only listen to the first 16 as only these support OPWM
		for (Pwm_ChannelType emios_ch = 0; emios_ch < 16; emios_ch++) {
			if (flagmask & (1 << emios_ch)) {

				if (ChannelRuntimeStruct[emios_ch].NotificationRoutine != NULL && EMIOS.CH[emios_ch].CCR.B.FEN) {

					Pwm_EdgeNotificationType notification = ChannelRuntimeStruct[emios_ch].NotificationState;
					if (notification == PWM_BOTH_EDGES ||
							notification == EMIOS.CH[emios_ch].CSR.B.UCOUT) {
							ChannelRuntimeStruct[emios_ch].NotificationRoutine();
					}
				}

				// Clear interrupt
				EMIOS.CH[emios_ch].CSR.B.FLAG = 1;
			}
		}
	}

#endif /* PWM_NOTIFICATION_SUPPORED == STD_ON */
