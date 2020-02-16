/*
 * Uart.h
 *
 *  Created on: May 11, 2013
 *      Author: Zhang Shuzhou
 */

#ifndef UART_H_
#define UART_H_
#include "Platform_Types.h"



void Uart_Init(void);
void mini_uart_send ( uint32 c );
void mini_uart_sendstr (char *s);
void mini_uart_sendhex ( uint32 d, bool newline );
void mini_uart_sendDec(uint32 n);
uint32 mini_uart_recv ( void );
uint32 mini_uart_lcr ( void );

void chprintf(const char *fmt, ...);

char* Int_To_Chars(int i);
void Uart_Send_Int(int i);
int Chars_To_Int(char header[]);


void pi_printf(char* s);
void pi_print(uint32 s, uint32 n);
#endif /* UART_H_ */
