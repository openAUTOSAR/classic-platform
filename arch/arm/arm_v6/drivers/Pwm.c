/*
 * Pwm.c
 *
 *  Created on:  Sep 3, 2013
 *      Author:  Zhang Shuzhou
 *  Reviewed on: Nov 7, 2014
 *     Reviewer: Avenir Kobetski
 *
 * This module allows to configure a Pulse Width Modulation (PWM) cycle and
 * send repeated pulses. The actual pulse transfer is done using Direct Memory
 * Access (DMA).
 *
 * Important parameters are the cycle period (in microseconds) and the width
 * of the duty cycle (percentage of the cycle that the pulse should be high).
 * The duty cycle values range between 0x0000 and 0x8000, which in practice
 * represents 0% and 100%.
 *
 * It is also possible to set up at which point in the cycle that the pulse
 * should be activated. However it is currently not used.
 *
 * A typical example usage (sets up PWM channel 0 to have a 20ms cycle and a
 * pulse starting at the beginning of each cycle and lasting for half a period):
 * 		PwmSetPeriodAndDuty(0, 20000, 0x4000)
 * If the period has been set up, either PwmSetPeriodAndDuty can continue to be
 * used or only the duty can be changed (for example to reduce the pulse to
 * being active only a quarter of the period):
 * 		PwmSetDuty(0, 0x2000)
 */
#include <stdio.h>

#include "Pwm.h"
#include "Dma.h"
#include "bcm2835.h"

/************************************************************************************/
/*									Definitions 	 		 						*/
/************************************************************************************/

/* PWM module state types */
typedef enum {
	PWM_STATE_UNINITIALIZED,
	PWM_STATE_INITIALIZED
} Pwm_ModuleStateType;

/**
 * To avoid memory overlaps, each channels data memory
 * may not exceed the pre-defined data sample limit
 */
#define MAX_DATA_LENGTH 	(PWM_NUMBER_OF_CHANNELS * PWM_SAMPLE_LIMIT)

/**
 *  To avoid memory overlaps, each channels control block
 * memory may not exceed twice the pre-defined sample limit
 * (each sample needs two control blocks). Also maximal
 * DMA alignment offset must be taken into consideration.
 */
#define MAX_CB_LENGTH		(PWM_NUMBER_OF_CHANNELS * (2 * PWM_SAMPLE_LIMIT + DMA_ALIGN_OFFSET))

/************************************************************************************/
/*									Local variables 		 						*/
/************************************************************************************/

/* PWM module state */
static Pwm_ModuleStateType Pwm_ModuleState = PWM_STATE_UNINITIALIZED;

/* All PWM channel control structures */
static struct PwmChannel pwmChannels[PWM_NUMBER_OF_CHANNELS];

/* Pulse data for each sample in a cycle (each sample is PWM_GRANULARITY_US microseconds long) */
static uint32 data[MAX_DATA_LENGTH];

/* DMA control block data (2 blocks per data sample) */
static Dma_CBType dmaCbs[MAX_CB_LENGTH];

/* Connection between PWM channels and GPIO pins */
static uint8 pwmGpios[PWM_NUMBER_OF_CHANNELS] = {GPIO_PWM_SPEED, GPIO_PWM_STEERING};

/************************************************************************************/
/*									Local method declarations 						*/
/************************************************************************************/

/**
 * Configure necessary DMA structures (e.g. control blocks and channels).
 *
 * @param channel 		-----   PWM channel id
 * @param nr_samples	-----   number of data samples in each PWM cycle
 */
static void Pwm_ConfigureDma(Pwm_ChannelType channel,
							 uint32 nr_samples);
/**
 * Configure PWM channel parameters and
 * setup necessary DMA control structures.
 *
 * @param channel 		-----   PWM channel id
 * @param cb_address	-----	base address of DMA control blocks for this channel
 * @param data_address	-----	base address of data samples for this channel
 * @param period		-----	cycle period (in microseconds)
 */
static void Pwm_ConfigureChannel(Pwm_ChannelType channel,
								 Dma_CBType* cb_address,
								 uint32* data_address,
								 uint32 period);
/**
 * This function sets up necessary GPIO pins and generates
 * a PWM pulse on a given channel.
 * Previous pulses are cleared in the process.
 *
 * @param channel	       -----    PWM channel id
 * @param pulse_start      -----    sample number (relative to cycle start) for pulse start
 * @param pulse_width      -----    width of the pulse (counted in samples)
 *
 */
static void Pwm_GeneratePulse(Pwm_ChannelType channel,
							  int pulse_start, int pulse_width);

/************************************************************************************/
/*									Local methods 									*/
/************************************************************************************/

/**
 * Configure necessary DMA structures (e.g. control blocks and channels).
 *
 * @param channel 		-----   PWM channel id
 * @param nr_samples	-----   number of data samples in each PWM cycle
 */
static void Pwm_ConfigureDma(Pwm_ChannelType channel,
							 uint32 nr_samples)
{
	int i;
	uint32 sourceAddr;									// Address of each data sample, used as source by DMA
	uint32 bus_gpclr0 = MAP_TO_BUSADDRESS(GPCLR0);		// GPIO Pin Output Clear 0 (p.90 in BCM2835-ARM-Peripherals.pdf)
	uint32 bus_fif1 = MAP_TO_BUSADDRESS(PWM_FIF1);		// PWM FIFO Input (p.141 in BCM2835-ARM-Peripherals.pdf)
    uint32 information1 = DMA_NO_WIDE_BURSTS |			// DMA transfer info configuration (see pp.50-52 in BCM2835-ARM-Peripherals.pdf)
    					  DMA_WAIT_RESP;
    uint32 information2 = DMA_NO_WIDE_BURSTS |			// DMA transfer info configuration (see pp.50-52 in BCM2835-ARM-Peripherals.pdf)
    					  DMA_WAIT_RESP |
						  DMA_D_DREQ |
						  DMA_PER_MAP(5);
    Dma_CBType* cb_address = pwmChannels[channel].cb_base; // Local CB-pointer, used to loop through all nested control blocks of this channel

	/* Set up control blocks for each DMA-sample of the duty cycle.
	 * This is done by alternating control blocks for gpio-reset and fifo-write. */
	for (i = 0; i < nr_samples; i++) {
		pwmChannels[channel].data_base[i] = 0;
		sourceAddr  = (uint32) (pwmChannels[channel].data_base + i);

		/* Before each sending control block, make sure that the GPIO pin is cleared */
		Dma_ConfigureCB(cb_address,
						information1,
						sourceAddr,
						bus_gpclr0,
						PWMDMA_TXFRLEN,
						PWMDMA_STRIDE,
						(uint32)(cb_address+1));
		cb_address++;

		/* Setup DMA so that each data sample is sent to the FIFO register */
		Dma_ConfigureCB(cb_address,
						information2,
						sourceAddr,
						bus_fif1,
						PWMDMA_TXFRLEN,
						PWMDMA_STRIDE,
						(uint32)(cb_address+1));
		cb_address++;
	}

	/* Go back to the last data sample control block, and direct it to the first data sample (loop) */
	cb_address--;
	cb_address->next = (uint32)pwmChannels[channel].cb_base;

	/* Configure the channel */
	Dma_ConfigureChannel(pwmChannels[channel].cb_base, channel);
}

/**
 * Configure PWM channel parameters and
 * setup necessary DMA control structures.
 *
 * @param channel 		-----   PWM channel id
 * @param cb_address	-----	base address of DMA control blocks for this channel
 * @param data_address	-----	base address of data samples for this channel
 * @param period		-----	cycle period (in microseconds)
 */
static void Pwm_ConfigureChannel(Pwm_ChannelType channel,
								 Dma_CBType* cb_address,
								 uint32* data_address,
								 uint32 period)
{
	/* The number of data samples in each PWM cycle */
	uint32 nr_samples = period / PWM_GRANULARITY_US;

	/* Print error information (normally the RPi will hang some lines further down) */
	if(nr_samples > PWM_SAMPLE_LIMIT) {
		printf("ERROR: The chosen period requires %d samples, which exceeds the PWM_SAMPLE LIMIT (%d)\r\n", nr_samples, PWM_SAMPLE_LIMIT);
	}

	/* Set ut basic PWM channel data */
	pwmChannels[channel].cb_base = cb_address;
	pwmChannels[channel].data_base = data_address;
	pwmChannels[channel].cycle_time_us = period;
	pwmChannels[channel].prev_pulse_start = 0;
	pwmChannels[channel].prev_pulse_width = 0;

	/* Configure necessary DMA structures (e.g. control blocks) */
	Pwm_ConfigureDma(channel, nr_samples);
}

/**
 * This function sets up necessary GPIO pins and generates
 * a PWM pulse on a given channel.
 * Previous pulses are cleared in the process.
 *
 * @param channel	       -----    PWM channel id
 * @param pulse_start      -----    sample number (relative to cycle start) for pulse start
 * @param pulse_width      -----    width of the pulse (counted in samples)
 */
static void Pwm_GeneratePulse(Pwm_ChannelType channel,
							  int pulse_start, int pulse_width)
{
	static uint32 selected_pins;
	uint32 bus_gpclr0 = MAP_TO_BUSADDRESS(GPCLR0);
	uint32 bus_gpset0 = MAP_TO_BUSADDRESS(GPSET0);
	uint32 *dp = pwmChannels[channel].data_base;
	uint8 gpio = pwmGpios[channel];

	/* If this method call doesn't change previous pulse, then do nothing
	 * (since old values are repeatedly sent by DMA until they are changed). */
	if ((pulse_start == pwmChannels[channel].prev_pulse_start) &&
			(pulse_width == pwmChannels[channel].prev_pulse_width))
		return;

	/* If the gpio pin corresponding to this channel has not yet been activated
	 * for PWM operation, then select an appropriate GPIO function for it
	 * (see pp.91-94 in BCM2835-ARM-Peripherals.pdf) */
	if ((selected_pins & BIT(gpio)) == 0) {
		bcm2835_GpioFnSel(gpio, GPFN_OUT);
		selected_pins |= BIT(gpio);
	}

	/* First, clear (unselect) previous data samples (on the rising and falling pulse edges) */
	*(dp + pwmChannels[channel].prev_pulse_start) &= ~BIT(gpio);
	*(dp + pwmChannels[channel].prev_pulse_start + pwmChannels[channel].prev_pulse_width) &= ~BIT(gpio);

	/* Enable the gpio pin in the data sample corresponding to pulse start. */
	*(dp + pulse_start) |= BIT(gpio);
	/* Make sure that this sample is sent by the DMA to the GPIO_SET-register.
	 * (* 2) is due to the fact that we have two DMA control blocks per sample,
	 * one for gpio clearing and one for writing to fifo. */
	(pwmChannels[channel].cb_base + (pulse_start * 2))->dst = bus_gpset0;

	/* Enable the gpio pin in the data sample corresponding to pulse finish. */
	*(dp + pulse_start + pulse_width) |= BIT(gpio);
	/* Make sure that this sample is sent by the DMA to the GPIO_CLEAR-register.
	 * (* 2) is due to the fact that we have two DMA control blocks per sample,
	 * one for gpio clearing and one for writing to fifo. */
	(pwmChannels[channel].cb_base + ((pulse_start + pulse_width) * 2))->dst = bus_gpclr0;

	/* Store the start and width of this pulse (uniquely for each PWM channel) */
	pwmChannels[channel].prev_pulse_start = pulse_start;
	pwmChannels[channel].prev_pulse_width = pulse_width;
}

/************************************************************************************/
/*									Global methods 									*/
/************************************************************************************/

/**
 * [SWS_Pwm_00095] - Service for PWM initialization
 *
 * Called by EcuM at startup.
 *
 * @param ConfigPtr			-----	pointer to global configuration data
 */
void Pwm_Init(const Pwm_ConfigType *ConfigPtr)
{
	/* This variable is used to set up base CB address for each channel */
	Dma_CBType * cb_address = dmaCbs;
	/* This variable is used to set up base data sample address for each channel */
	uint32* data_address = data;

	/* Don't do anything if PWM has already been initialized */
	if( Pwm_ModuleState == PWM_STATE_INITIALIZED ) {
	  return;
	}

	/* Initialize the hardware clock */
	PWM_CTL = 0; // Stop/reset PWM
	CM_PWMCTL = CM_PWD | CMCTL_SRC_PLLD; // Set the clock source to PLL alternative D (i.e. 500MHz). This is a good choice to avoid jitter
	bcm2835_Sleep(4);
	CM_PWMDIV = CM_PWD | CMDIV_DIVI; // Divide the actual frequency by 50, reducing it to 10 MHz
	bcm2835_Sleep(35);
	CM_PWMCTL = CM_PWD | CMCTL_SRC_PLLD | CMCTL_ENAB; // Enable the clock generator
	bcm2835_Sleep(35);

	/* Initialize PWM channel 1 */
	PWM_RNG1 = PWM_DEFAULT_RNG;	// Set our default PWM channel range
	bcm2835_Sleep(4);
	PWM_DMAC = PWMDMAC_ENAB | PWMDMAC_PANIC | PWMDMAC_DREQ; // Set thresholds for DMA and enable it
	bcm2835_Sleep(4);
	PWM_CTL = PWMCTL_CLRF1; // Clear FIFO
	bcm2835_Sleep(4);
	PWM_CTL = PWMCTL_USEF1 | PWMCTL_PWEN1; // Enable channel 1 and use FIFO for transmission
	bcm2835_Sleep(4);

	/* Initialize PWM and DMA structures for each channel */
	for (Pwm_ChannelType channelId = 0; channelId < PWM_NUMBER_OF_CHANNELS; channelId++) {
		const Pwm_ChannelConfigurationType *chCfgPtr = &ConfigPtr->channels[channelId];

		/* Align the cb_address to 256 bit (required by DMA) */
		cb_address = Dma_AlignCB(cb_address);

		/* Init the PWM channel structure and set up DMA control blocks */
		Pwm_ConfigureChannel(chCfgPtr->channel, cb_address, data_address, chCfgPtr->period);

		/* Allocate memory for control block and data for this channel
		 * (limited by the maximum defined number of samples) */
		cb_address 	 += (2 * PWM_SAMPLE_LIMIT);
		data_address += PWM_SAMPLE_LIMIT;
	}

	/* Update the PWM state */
	Pwm_ModuleState = PWM_STATE_INITIALIZED;
}

/**
 * [SWS_Pwm_00096] - Service for PWM De-Initialization
 *
 * Reset PWM control structures. This also triggers DMA de-initialization.
 */
void Pwm_DeInit(void)
{
	int i, j;
	Pwm_NamedChannelsType channel;

	/* De-initialize DMA */
	Dma_DeInit();

	/* Reset the PWM channel control structures */
	for (channel = 0; channel < PWM_NUMBER_OF_CHANNELS; channel++) {
		pwmChannels[channel].cb_base = (Dma_CBType*) 0;
		pwmChannels[channel].data_base = 0;
		pwmChannels[channel].cycle_time_us = 0;
		pwmChannels[channel].prev_pulse_start = 0;
		pwmChannels[channel].prev_pulse_width = 0;
	}

	/* Reset control block and data memories */
	for (i = 0; i < PWM_SAMPLE_LIMIT; i++) {
		data[i] = 0;

		for (j=0; j<PWM_NUMBER_OF_CHANNELS; j++) {
			dmaCbs[PWM_NUMBER_OF_CHANNELS*i+j].dst = 0;
			dmaCbs[PWM_NUMBER_OF_CHANNELS*i+j].info = 0;
			dmaCbs[PWM_NUMBER_OF_CHANNELS*i+j].length = 0;
			dmaCbs[PWM_NUMBER_OF_CHANNELS*i+j].next = 0;
			dmaCbs[PWM_NUMBER_OF_CHANNELS*i+j].src = 0;
			dmaCbs[PWM_NUMBER_OF_CHANNELS*i+j].stride = 0;
		}
	}

	/* Update the PWM state */
	Pwm_ModuleState = PWM_STATE_UNINITIALIZED;
}

/**
 * [SWS_Pwm_00097] - Service sets the duty cycle of a PWM channel
 *
 * @param channel 		-----	PWM channel id
 * @param dutyCycle 	-----	percentage of the cycle, during which the pulse should be high
 * 								[0-100%] represented as [0x0000-0x8000]
 */
void Pwm_SetDutyCycle(Pwm_ChannelType channel, Pwm_DutyCycleType dutyCycle)
{
	/* Set duty, using the already defined period */
	Pwm_SetPeriodAndDuty(channel, pwmChannels[channel].cycle_time_us, dutyCycle);
}

/**
 * [SWS_Pwm_00098] - Service sets the period and the duty cycle of a PWM channel
 *
 * Typically called by the upper layers (e.g. SW-C)
 * Example: Pwm_SetPeriodAndDuty(0/1, 20000, scaledPercValue);
 * If the period has changed, the channel is first re-initialized.
 * PWM pulses always start at the beginning of each cycle in this implementation.
 *
 * @param channel	    -----   PWM channel id
 * @param period		-----	cycle period (in microseconds)
 * @param dutyCycle		-----	percentage of the cycle, during which the pulse should be high
 * 								[0-100%] represented as [0x0000-0x8000]
 */
void Pwm_SetPeriodAndDuty(Pwm_ChannelType channel, Pwm_PeriodType period, Pwm_DutyCycleType dutyCycle)
{
	Dma_CBType * cb_address = pwmChannels[channel].cb_base;
	uint32* sample_address = pwmChannels[channel].data_base;

	/* Set up new cycle settings, if the period has changed and re-initialize DMA control blocks */
	if (pwmChannels[channel].cycle_time_us != period){
		Pwm_ConfigureChannel(channel, cb_address, sample_address, period);
	}

	/*
	 * dutyCycle is defined as the percentage of a cycle, during which the PWM pulse should be high.
	 * [0-100%] are represented by a number in the range [0x0000-0x8000]. Thus,
	 * to convert it from "percentage" to a share of a cycle, it must be divided by 2^15.
	 * In theory, the algorithm is as follows:
	 * 		pulseHighPartOfCycle = dutyCycle >> 15; // Gives a number between 0 and 1
	 * 		pulseHighInMicroSeconds = period * pulseHighPartOfCycle;
	 * 		pulseHighInNrSamples = pulseHighInMicroSeconds / microsecondsPerSample
	 * However, dealing with integers, the first step will almost always yield 0.
	 * Thus the second step is done first, see below, resulting in a (quite good) approximation.
	 */
	uint32 pulse_width = (((uint32)dutyCycle * (uint32)period) >> 15) / PWM_GRANULARITY_US;
	Pwm_GeneratePulse(channel, 0, pulse_width);
}

/************************************************************************************/
/*					Default (empty) methods, required by the starndard 				*/
/************************************************************************************/

/**
 * [SWS_Pwm_0099] - Service sets the PWM output to the configured Idle state.
 *
 * Empty implementation
 */
void Pwm_SetOutputToIdle(Pwm_ChannelType Channel) {}

/**
 * [SWS_Pwm_00100] - Service to read the internal state of the PWM output signal.
 *
 * Empty implementation
 */
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType ChannelNumber) { return PWM_LOW; }

/**
 * [SWS_Pwm_00101] - Service to disable the PWM signal edge notification.
 *
 * Empty implementation
 */
void Pwm_DisableNotification(Pwm_ChannelType Channel) {}

/**
 * [SWS_Pwm_00102] - Service to enable the PWM signal edge notification
 * 					 according to notification parameter.
 *
 * Empty implementation
 */
void Pwm_EnableNotification(Pwm_ChannelType Channel, Pwm_EdgeNotificationType Notification) {}
