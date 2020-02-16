/*
 * sys_tick.h
 *
 *  Created on:  Mar 4, 2013
 *      Author:  Zhang Shuzhou
 *  Reviewed on:
 *     Reviewer:
 *
 */

#include "Os.h"
#include "internal.h"
#include "isr.h"
#include "arc.h"
#include "counter_i.h"
#include "bcm2835.h"
#include "irq_types.h"
#include "led.h"
#include "Uart.h"

boolean led_flag = true;
uint32 led_count = 0;

/*
 *to proof the code is running by using the
 *led blink 500ms/once
 */
static void led_proof(void){
	led_count++;
	if ((led_flag == true) && (led_count / 500 == 1)){// turn on
		*((&IOPORT0)->gpclr) = 0x00010000;
		led_flag = false;
		led_count = 0;
	}else if ((led_flag == false) && (led_count / 500 == 1)) { // turn off
		*((&IOPORT0)->gpset) = 0x00010000;
		led_flag = true;
		led_count = 0;
	}
}

void Bcm2835OsTick(void) {
	OsTick();
	led_proof();
}

/**
 * Init of free running timer.
 */
void Os_SysTickInit( void ) {
	//TEMPORARY COMMENTED
//	bcm2835_gpio_fnsel(ONBOARD_LED_PAD, GPFN_OUT);
	ISR_INSTALL_ISR2("OsTick",Bcm2835OsTick,BCM2835_IRQ_ID_TIMER_0/*BCM2835_IRQ_ID_SYSTEM_TIMER3*/,6,0);
}

/**
 * Start the Sys Tick timer
 *
 * @param period_ticks How long the period in timer ticks should be.
 *
 */

void Os_SysTickStart(uint32_t period_ticks) {

	    ARM_TIMER_CTL = 0x003E0000;
		ARM_TIMER_LOD = 1000-1;
		ARM_TIMER_RLD = 1000-1;     //should be 1000, then get 1us per once, the system tick is 1ms
	    ARM_TIMER_DIV = 0x000000F9;  //source 250Mhz, divide 250, get 1Mhz
		ARM_TIMER_CLI = 0;
		ARM_TIMER_CTL = 0x003E00A2;  //23-bit counter, enable

		IRQ_ENABLE_BASIC = 1;
//	    IRQ_ENABLE1 = SYSTIMER_IRQEN3;
//	    clkupdate(SYSTIMER_CLOCK_FREQ / CLKTICKS_PER_SEC);
}


TickType Os_SysTickGetValue( void )
{

	uint32 SysTick;
	SysTick = ARM_TIMER_LOD - ARM_TIMER_VAL;
	return SysTick;

}


TickType Os_SysTickGetElapsedValue(uint32 preValue ) {
	uint32 curr;
	uint32 max;

	curr = ARM_TIMER_VAL;
	max  = ARM_TIMER_LOD;
	return Os_CounterDiff((max - curr),preValue,max);
}

