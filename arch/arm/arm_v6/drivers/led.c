/*
 * led.c
 *
 *  Created on: Apr 3, 2013
 *      Author: Zhang Shuzhou
 */
#include "led.h"
#include "bcm2835_Types.h"
#include "bcm2835.h"
#include "Std_Types.h"

void Task1_off(){
	bcm2835_GpioFnSel(17, GPFN_OUT);
	*((&IOPORT0)->gpclr) = 0x00020000;
}

void Task1_on(){
	*((&IOPORT0)->gpset) = 0x00020000;
}

void Task2_off(){
	bcm2835_GpioFnSel(2, GPFN_OUT);
	*((&IOPORT0)->gpclr) = 0x00000004;
}

void Task2_on(){
	*((&IOPORT0)->gpset) = 0x00000004;
}

void Task3_off(){
	bcm2835_GpioFnSel(3, GPFN_OUT);
	*((&IOPORT0)->gpclr) = 0x00000008;
}

void Task3_on(){
	*((&IOPORT0)->gpset) = 0x00000008;
}

void Idle_off(){
	bcm2835_GpioFnSel(4, GPFN_OUT);
	*((&IOPORT0)->gpclr) = 0x00000010;
}

void Idle_on(){
	*((&IOPORT0)->gpset) = 0x00000010;
}


void led_on(){
	*((&IOPORT0)->gpclr) = 0x00010000;
}

void led_off(){
	*((&IOPORT0)->gpset) = 0x00010000;
}
void delay(int n){
    int i = 0;
    for(i = 0; i < n; i++);
}

/*
 * Make LED blink.
 *
 * @param n - nr of blinks
 * @param m - delay time between blinks (approx micros)
 */
void led_blink(uint32 n,uint32 m)
{
     int i;
	 for(i = 0; i < n; i++)
	 {
	    	led_on();
	    	bcm2835_Sleep(m);
	    	led_off();
	    	bcm2835_Sleep(m);
	 }
}


void led_asm(uint32 n)
{

   __asm
		(
				"Led_on:     				\
				        ldr r0,=0x20200000;	\
				        mov r1,#1;			\
						lsl r1,#18;			\
						str r1,[r0,#4];		\
						mov r1,#1;			\
						lsl r1,#16;"
        );

   for(int i=0; i<n; i++){
       __asm
       (
				"loop$:						\
						str r1,[r0,#40];    \
						mov r2,#0x3F000;"
				"wait1$:   					\
						sub r2,#1;          \
						cmp r2,#0;			\
						bne wait1$;			\
						str r1,[r0,#28];	\
						mov r2,#0x3F000;"
				"wait2$:					\
					    sub r2,#1;			\
					    cmp r2,#0;			\
					    bne wait2$;"

		);
   }

}
