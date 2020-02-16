/*
 * led.h
 *
 *  Created on: Apr 3, 2013
 *      Author: Zhang Shuzhou
 */

#ifndef LED_H_
#define LED_H_
#include "Std_Types.h"

void Task1_on(void);
void Task1_off(void);

void Task2_on(void);
void Task2_off(void);

void Task3_on(void);
void Task3_off(void);

void Idle_on(void);
void Idle_off(void);


void led_on(void);
void led_off(void);
void delay(int n);
void led_asm(uint32 n);
/*
 * Make LED blink.
 *
 * @param n - nr of blinks
 * @param m - delay time between blinks (ms)
 */
void led_blink(uint32  n, uint32 m);
#endif /* LED_H_ */
