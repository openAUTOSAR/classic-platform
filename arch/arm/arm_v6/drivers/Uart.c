/*
 * Uart.c
 *
 *  Created on: May 11, 2013
 *      Author: Zhang Shuzhou
 */
#include <stdio.h>
#include <stdlib.h>
#include "led.h"
#include "bcm2835_Types.h"
#include "Std_Types.h"
#include "bcm2835.h"
#include "Uart.h"
#include "isr.h"
#include "irq_types.h"
#include "Os.h"


//if define the interrupt of uart, should be
boolean UART_FLAG = false;
boolean SPEEDEVENT = false;
boolean SERVOEVENT = false;

int result_channel = 2;
int result_value = 0;

char* s = NULL;
char channel[4];
char value[4];
int isLegalReceive = false;
int totalValidDataSize = 4 * 2;
int index = 0;
int channel_index = 0;
int value_index = 0;
char tmp = '0';

//static void Uart_Data_Handler(uint8 data){
//
//    pi_printf("uart data handler"); mini_uart_sendDec(data);pi_printf("\r\n");
//	tmp = data;
//	if (isLegalReceive == false) {
//		if (tmp == '^') {
//			isLegalReceive = true;
//			index = 0;
//			channel_index = 0;
//			value_index = 0;
//			pi_printf("input ^");
//		}
//	} else {
//
//		if (index < sizeof(int)) {
//			channel[channel_index] = tmp;
//			channel_index++;
//			pi_printf("channel_index\r\n");
//		} else if (index < totalValidDataSize && index >= sizeof(int)) {
//			value[value_index] = tmp;
//			value_index++;
//			pi_printf("value_index\r\n");
//		} else {
//			if (tmp == '$') {
//				tmp = '0';
//				isLegalReceive = false;
//
//				result_channel = Chars_To_Int(channel);
//				pi_printf("result_channel"); mini_uart_sendDec(result_channel);pi_printf("\r\n");
//				result_value = Chars_To_Int(value);
//				pi_printf("result_value"); mini_uart_sendDec(result_value);pi_printf("\r\n");
//				if (result_channel == SPEEDCH) {
//					SPEEDEVENT = true;
//					SERVOEVENT = false;
//					pi_printf("Activate speed\r\n");
//					ActivateTask(TASK_ID_PirteTask);       // ^^00001234$
//				}
//				if (result_channel == SERVOCH) {
//					SERVOEVENT = true;
//					SPEEDEVENT = false;
//					pi_printf("Activate servo\r\n");
//					ActivateTask(TASK_ID_PirteTask);
//				}
//			}
//		}
//		index++;
//	} // end if
//}

//static void Uart_Isr(void) {
////	if (AUX_MU_IIR_RX_IRQ) {
////		while (!AUX_MU_LSR_RX_RDY);
////		do {
////			Uart_Data_Handler(AUX_MU_IO_REG & 0xFF);
////		} while (AUX_MU_LSR_RX_RDY);
////
////	}
////	if (AUX_MU_IIR_TX_IRQ) {
////		while (!AUX_MU_LSR_TX_RDY)
////			;
////		uint32_t data = UART_TX[0];
////		if (data < 1) {
////			/* Disable tx interrupts.*/
////			AUX_MU_IER_REG &= ~AUX_MU_IER_TX_IRQEN;
////		} else {
////			mini_uart_send((uint32_t) data);
////			UART_TX[0] = 0;
////		}
////	}
//}



void Uart_Init(void) {

  IRQ_DISABLE1 = BIT(29);

  AUX_ENABLES = 1;

  AUX_MU_IER_REG  = 0x00;
  AUX_MU_CNTL_REG = 0x00;
  AUX_MU_LCR_REG  = 0x03; // Bit 1 must be set
  AUX_MU_MCR_REG  = 0x00;
  AUX_MU_IER_REG  = 0x05;
  AUX_MU_IIR_REG  = 0xC6;
  AUX_MU_BAUD_REG = ((BCM2835_CLOCK_FREQ / (8 * (115200))) - 1);

  bcm2835_GpioFnSel(14, GPFN_ALT5);
  bcm2835_GpioFnSel(15, GPFN_ALT5);

  GPPUD = 0;
  bcm2835_Sleep(50);
  GPPUDCLK0 = (1<<14)|(1<<15);
  bcm2835_Sleep(50);
  GPPUDCLK0 = 0;
  AUX_MU_CNTL_REG = 0x03;

//  ISR_INSTALL_ISR2("UART", Uart_Isr, BCM2835_IRQ_ID_AUX_UART, 12, 0);

//  IRQ_ENABLE1 = BIT(29);

}

void mini_uart_send ( uint32 c )
{

  while((AUX_MU_LSR_REG & 0x20) == 0);
  AUX_MU_IO_REG = c;

}

void mini_uart_sendstr (char *s)
{

  unsigned char c;
  while((c = *s++)) mini_uart_send(c);

}

void mini_uart_sendhex ( uint32 d, boolean newline )
{
  uint32 rb;
  uint32 rc;

  rb=32;
  while(1)
  {
    rb-=4;
    rc=(d>>rb)&0xF;
    if(rc>9)
      rc+=0x37;
    else
      rc+=0x30;
    mini_uart_send(rc);
    if(rb==0) break;
  }

  mini_uart_send(0x20);

  if (newline) {
    mini_uart_send(0x0D);
    mini_uart_send(0x0A);
  }
}

uint32 mini_uart_recv ( void )
{
    while ((AUX_MU_LSR_REG & 0x01) == 0);
	return (AUX_MU_IO_REG & 0xFF);
}

uint32 mini_uart_lcr ( void )
{
	uint32 result = 0;
	result = AUX_MU_LSR_REG;
    return result;
}

/*Int to chars
 *
 */
char* Int_To_Chars(int i)
{
	char *result = NULL;
	char str[4] = { 0, 0, 0, 0 };
	str[0] = (char) i;
	str[1] = (char) (i >> 8);
	str[2] = (char) (i >> 16);
	str[3] = (char) (i >> 24);
	result = str;
	return result;
}

/*
 * Send Int
 */
void Uart_Send_Int(int i)
{
    char *data = Int_To_Chars(i);
    int index;
    for(index=0; index < 4; index++)
    {
    	mini_uart_send(*data);
    	data++;
    }
    data = NULL;
}

int Chars_To_Int(char header[4])
{
  unsigned int result = 0;
  result = header[0];
  result = result + (header[1] << 8);
  result = result + (header[2] << 16);
  result = result + (header[3] << 24);
  return result;
}

void mini_uart_sendDec(uint32 n){
  if(n >= 10){
	mini_uart_sendDec(n/10);
    n = n%10;
  }
  mini_uart_send(n+'0'); /* n is between 0 and 9 */
}



void pi_printf(char* s){


	if(UART_FLAG == false ){
		Uart_Init();
		UART_FLAG = true;
	}

	mini_uart_sendstr (s);

}

void pi_print(uint32 s, uint32 n){


	if(UART_FLAG == false ){
		Uart_Init();
		UART_FLAG = true;
	}

	mini_uart_sendhex (s, n);

}

//// reverses a string 'str' of length 'len'
//static void reverse(char *str, int len)
//{
//    int i=0, j=len-1, temp;
//    while (i<j)
//    {
//        temp = str[i];
//        str[i] = str[j];
//        str[j] = temp;
//        i++; j--;
//    }
//}
//
//// Converts a given integer x to string str[].  d is the number
//// of digits required in output. If d is more than the number
//// of digits in x, then 0s are added at the beginning.
//static int intToStr(int x, char str[], int d)
//{
//    int i = 0;
//    while (x)
//    {
//        str[i++] = (x%10) + '0';
//        x = x/10;
//    }
//
//    // If number of digits required is more, then
//    // add 0s at the beginning
//    while (i < d)
//        str[i++] = '0';
//
//    reverse(str, i);
//    str[i] = '\0';
//    return i;
//}
//
//// Converts a floating point number to string.
//void ftoa(float n, char *res)
//{
//	// Extract integer part
//	int ipart = (int) n;
//
//	// Extract floating part
//	float fpart = n - (float) ipart;
//
//	// convert integer part to string
//	int i = intToStr(ipart, res, 0);
//
//	// check for display option after point
//
//	res[i] = '.';  // add dot
//
//	// Get the value of fraction part upto given no.
//	// of points after dot. The third parameter is needed
//	// to handle cases like 233.007
//	fpart = fpart * 1000;
//
//	intToStr((int) fpart, res + i + 1, 3);
//}
