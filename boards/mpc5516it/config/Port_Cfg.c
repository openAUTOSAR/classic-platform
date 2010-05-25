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









#include "Port_Cfg.h"

// All: 	PA,OBE,IBE,ODE,HYS,SRC,WPE,WPS
// Input: 	PA,OBE,IBE,HYS,WPE,WPS
// Output:  PA,OBE,IBE,ODE,SRC,(WPE,WPS)

#define 	PCR_PWM			PCR_BOOTCFG
#define 	PCR_ADC			(0)
#define	PCR_IO_INPUT	PCR_BOOTCFG
#define	PCR_IO_OUTPUT	OBE_ENABLE
#define	PCR_CAN			(0)
#define 	PCR_SPI			PCR_BOOTCFG
#define 	PCR_I2C			PCR_BOOTCFG


const uint16_t PortPadConfigData[] =
{
  // pad 0-15
  PCR_ADC|PA_FUNC1, /* PA0, AN0 */
  PCR_RESET, /* PA1      */
  PCR_RESET, /* PA2      */
  PCR_RESET, /* PA3      */
  PCR_RESET, /* PA4      */
  PCR_RESET, /* PA5 		*/
  PCR_RESET, /* PA6,     */
  PCR_RESET, /* PA7(input only) */
  PCR_RESET, /* PA8(input only) */
  PCR_RESET, /* PA9(input only) */
  PCR_RESET, /* PA10(input only) */
  PCR_RESET, /* PA11(input only) */
  PCR_RESET, /* PA12(input only) */
  PCR_RESET, /* PA13(input only) */
  PCR_RESET, /* PA14(input only) */
  PCR_RESET, /* PA15(input only) */

  // pad 16-31
  PCR_RESET, /* PB0 */
  PCR_RESET, /* PB1 */
  PCR_RESET, /* PB2 */
  PCR_RESET, /* PB3 */
  PCR_RESET, /* PB4 */
  PCR_RESET, /* PB5 */
  PCR_RESET, /* PB6 */
  PCR_RESET, /* PB7 */
  PCR_RESET, /* PB8 */
  PCR_RESET, /* PB9 */
  PCR_RESET, /* PB10 */
  PCR_RESET, /* PB11 */
  PCR_RESET, /* PB12 */
  PCR_RESET, /* PB13 */
  PCR_RESET, /* PB14 */
  PCR_RESET, /* PB15 */

  // pad 32-47
  PCR_RESET, /* PC0/emios0 */
  PCR_RESET, // PC1
  PCR_RESET, // PC2
  PCR_RESET, /* PC3 */
  PCR_RESET, /* PC4 */
  PCR_RESET, /* PC5 */
  PCR_RESET, /* PC6 */
  PCR_RESET, /* PC7 */
  PCR_RESET, /* PC8 */
  PCR_RESET, /* PC9 */
  PCR_RESET, /* PC10 */
  PCR_RESET, /* PC11 */
  PCR_RESET, /* PC12 */
  PCR_RESET, /* PC13 */
  PCR_RESET, /* PC14 */
  PCR_RESET, /* PC15 */

  // pad 48-63
  (PCR_CAN|PA_FUNC1), /* PD0, CAN_A, Tx */
  (PCR_CAN|PA_FUNC1), /* PD1, CAN_A, Rx */
  PCR_BOOTCFG, /* PD2 */
  PCR_RESET, /* PD3 */
  PCR_IO_OUTPUT, /* PD4, LED */
  PCR_IO_OUTPUT, /* PD5, LED */
  PCR_RESET, /* PD6 , SCI_RS232??*/
  PCR_RESET, /* PD7 , SCI_RS232??*/
  (PA_FUNC1 | OBE_ENABLE | SLEW_RATE_MAX), /* PD8 LIN TX */
  (PA_FUNC1 | IBE_ENABLE), /* PD9 LIN RX */
  PCR_RESET, /* PD10 */
  PCR_RESET, /* PD11 */
  (PA_FUNC1 | OBE_ENABLE | SLEW_RATE_MAX), /* PD12 SPIB_CS0 */
  (PA_FUNC1 | OBE_ENABLE | SLEW_RATE_MAX), /* PD13 SPIB_CLK*/
  (PA_FUNC1 | OBE_ENABLE | SLEW_RATE_MAX), /* PD14 SPIB_SO*/
  (PA_FUNC1 | IBE_ENABLE),                 /* PD15 SPIB_SI*/

  // pad 64-79
  (PA_FUNC1 | OBE_ENABLE | SLEW_RATE_MAX), /* PE0 SPIA_CS1 */
  (PA_FUNC1 | OBE_ENABLE | SLEW_RATE_MAX), /* PE1 SPIA_CS2*/
  PCR_RESET, /* PE2 */
  (PA_FUNC1 | OBE_ENABLE | SLEW_RATE_MAX), /* PE3 SPIA_CLK*/
  (PA_FUNC1 | OBE_ENABLE | SLEW_RATE_MAX), /* PE4 SPIA_SO*/
  (PA_FUNC1 | IBE_ENABLE),                 /* PE5 SPIA_SI*/
  PCR_RESET, /* PE6 */
  PCR_RESET, /* PE7 */
  PCR_RESET, /* PE8 */
  PCR_RESET, /* PE9 */
  PCR_RESET, /* PE10 */
  PCR_RESET, /* PE11 */
  PCR_RESET, /* PE12 */
  PCR_RESET, /* PE13 */
  PCR_RESET, /* PE14 */
  PCR_RESET, /* PE15 */

  // pad 80-95
  PCR_RESET, /* PF0 */
  PCR_RESET, /* PF1 */
  PCR_RESET, /* PF2 */
  PCR_RESET, /* PF3 */
  PCR_RESET, /* PF4 */
  PCR_RESET, /* PF5 */
  PCR_RESET, /* PF6 */
  PCR_RESET, /* PF7 */
  PCR_RESET, /* PF8 */
  PCR_RESET, /* PF9 */
  PCR_RESET, /* PF10 */
  PCR_RESET, /* PF11 */
  PA_FUNC2|OBE_ENABLE|PULL_UP, /* PF12 SCI_D Transmit, J1708 Tx*/
  PA_FUNC2|IBE_ENABLE|PULL_UP, /* PF13 SCI_D Receive, J1708 Rx*/
  PCR_RESET, /* PF14 */
  PCR_RESET, /* PF15 */

  // pad 96-111
  PCR_RESET, /* PG0  */
  PA_FUNC2|IBE_ENABLE|PULL_UP, /* PG1 eMIOS[17], J1708 Edgedetect*/
  PCR_RESET, /* PG2 */
  PCR_RESET, /* PG3 */
  PCR_RESET, /* PG4 */
  PCR_RESET, /* PG5 */
  PCR_RESET, /* PG6 */
  PCR_RESET, /* PG7 */
  PCR_RESET, /* PG8 */
  PCR_RESET, /* PG9 */
  PCR_RESET, /* PG10 */
  PCR_RESET, /* PG11 */
  PCR_RESET, /* PG12 */
  PCR_RESET, /* PG13 */
  PCR_RESET, /* PG14 */
  PCR_RESET, /* PG15 */

  // pad 112-127
  PCR_RESET, /* PH0 */
  PCR_RESET, /* PH1 */
  PCR_RESET, /* PH2 */
  PCR_RESET, /* PH3 */
  PCR_RESET, /* PH4 */
  PCR_RESET, /* PH5 */
  PCR_RESET, /* PH6 */
  PCR_RESET, /* PH7 */
  PCR_RESET, /* PH8 */
  PCR_RESET, /* PH9 */
  PCR_RESET, /* PH10 */
  PCR_RESET, /* PH11 */
  PCR_RESET, /* PH12 */
  PCR_RESET, /* PH13 */
  PCR_RESET, /* PH14 */
  PCR_RESET, /* PH15 */

  // pad 128-143
  PCR_RESET, /* PJ0 */
  PCR_RESET, /* PJ1 */
  PCR_RESET, /* PJ2 */
  PCR_RESET, /* PJ3 */
  PCR_RESET, /* PJ4 */
  PCR_RESET, /* PJ5 */
  PCR_RESET, /* PJ6 */
  PCR_RESET, /* PJ7 */
  PCR_RESET, /* PJ8 */
  PCR_RESET, /* PJ9 */
  PCR_RESET, /* PJ10 */
  PCR_RESET, /* PJ11 */
  PCR_RESET, /* PJ12 */
  PCR_RESET, /* PJ13 */
  PCR_RESET, /* PJ14 */
  PCR_RESET, /* PJ15 */

  // pad 144-145
  PCR_RESET, /* PK0 (input only) */
  PCR_RESET, /* PK1 (input only)*/
};


#define GPDO_RESET 0

const uint8_t PortOutConfigData[] =
{
  // pad 16-31
  GPDO_RESET, /* PB0 */
  GPDO_RESET, /* PB1 */
  GPDO_RESET, /* PB2 */
  GPDO_RESET, /* PB3 */
  GPDO_RESET, /* PB4 */
  GPDO_RESET, /* PB5 */
  GPDO_RESET, /* PB6 */
  GPDO_RESET, /* PB7 */
  GPDO_RESET, /* PB8 */
  GPDO_RESET, /* PB9 */
  GPDO_RESET, /* PB10 */
  GPDO_RESET, /* PB11 */
  GPDO_RESET, /* PB12 */
  GPDO_RESET, /* PB13 */
  GPDO_RESET, /* PB14 */
  GPDO_RESET, /* PB15 */

  // pad 32-47
  GPDO_RESET, /* PC0 */
  GPDO_RESET, /* PC1 */
  GPDO_RESET, /* PC2 */
  GPDO_RESET, /* PC3 */
  GPDO_RESET, /* PC4 */
  GPDO_RESET, /* PC5 */
  GPDO_RESET, /* PC6 */
  GPDO_RESET, /* PC7 */
  GPDO_RESET, /* PC8 */
  GPDO_RESET, /* PC9 */
  GPDO_RESET, /* PC10 */
  GPDO_RESET, /* PC11 */
  GPDO_RESET, /* PC12 */
  GPDO_RESET, /* PC13 */
  GPDO_RESET, /* PC14 */
  GPDO_RESET, /* PC15 */

  // pad 48-63
  GPDO_RESET, /* PD0 */
  GPDO_RESET, /* PD1 */
  GPDO_RESET, /* PD2 */
  GPDO_RESET, /* PD3 */
  GPDO_RESET, /* PD4 */
  GPDO_RESET, /* PD5 */
  GPDO_RESET, /* PD6 */
  GPDO_RESET, /* PD7 */
  GPDO_RESET, /* PD8 */
  GPDO_RESET, /* PD9 */
  GPDO_RESET, /* PD10 */
  GPDO_RESET, /* PD11 */
  GPDO_RESET, /* PD12 */
  GPDO_RESET, /* PD13 */
  GPDO_RESET, /* PD14 */
  GPDO_RESET, /* PD15 */

  // pad 64-79
  GPDO_RESET, /* PE0 */
  GPDO_RESET, /* PE1 */
  GPDO_RESET, /* PE2 */
  GPDO_RESET, /* PE3 */
  GPDO_RESET, /* PE4 */
  GPDO_RESET, /* PE5 */
  GPDO_RESET, /* PE6 */
  GPDO_RESET, /* PE7 */
  GPDO_RESET, /* PE8 */
  GPDO_RESET, /* PE9 */
  GPDO_RESET, /* PE10 */
  GPDO_RESET, /* PE11 */
  GPDO_RESET, /* PE12 */
  GPDO_RESET, /* PE13 */
  GPDO_RESET, /* PE14 */
  GPDO_RESET, /* PE15 */

  // pad 80-95
  GPDO_RESET, /* PF0 */
  GPDO_RESET, /* PF1 */
  GPDO_RESET, /* PF2 */
  GPDO_RESET, /* PF3 */
  GPDO_RESET, /* PF4 */
  GPDO_RESET, /* PF5 */
  GPDO_RESET, /* PF6 */
  GPDO_RESET, /* PF7 */
  GPDO_RESET, /* PF8 */
  GPDO_RESET, /* PF9 */
  GPDO_RESET, /* PF10 */
  GPDO_RESET, /* PF11 */
  GPDO_RESET, /* PF12 */
  GPDO_RESET, /* PF13 */
  GPDO_RESET, /* PF14 */
  GPDO_RESET, /* PF15 */

  // pad 96-111
  GPDO_RESET, /* PG0 */
  GPDO_RESET, /* PG1 */
  GPDO_RESET, /* PG2 */
  GPDO_RESET, /* PG3 */
  GPDO_RESET, /* PG4 */
  GPDO_RESET, /* PG5 */
  GPDO_RESET, /* PG6 */
  GPDO_RESET, /* PG7 */
  GPDO_RESET, /* PG8 */
  GPDO_RESET, /* PG9 */
  GPDO_RESET, /* PG10 */
  GPDO_RESET, /* PG11 */
  GPDO_RESET, /* PG12 */
  GPDO_RESET, /* PG13 */
  GPDO_RESET, /* PG14 */
  GPDO_RESET, /* PG15 */

  // pad 112-127
  GPDO_RESET, /* PH0 */
  GPDO_RESET, /* PH1 */
  GPDO_RESET, /* PH2 */
  GPDO_RESET, /* PH3 */
  GPDO_RESET, /* PH4 */
  GPDO_RESET, /* PH5 */
  GPDO_RESET, /* PH6 */
  GPDO_RESET, /* PH7 */
  GPDO_RESET, /* PH8 */
  GPDO_RESET, /* PH9 */
  GPDO_RESET, /* PH10 */
  GPDO_RESET, /* PH11 */
  GPDO_RESET, /* PH12 */
  GPDO_RESET, /* PH13 */
  GPDO_RESET, /* PH14 */
  GPDO_RESET, /* PH15 */

  // pad 128-143
  GPDO_RESET, /* PJ0 */
  GPDO_RESET, /* PJ1 */
  GPDO_RESET, /* PJ2 */
  GPDO_RESET, /* PJ3 */
  GPDO_RESET, /* PJ4 */
  GPDO_RESET, /* PJ5 */
  GPDO_RESET, /* PJ6 */
  GPDO_RESET, /* PJ7 */
  GPDO_RESET, /* PJ8 */
  GPDO_RESET, /* PJ9 */
  GPDO_RESET, /* PJ10 */
  GPDO_RESET, /* PJ11 */
  GPDO_RESET, /* PJ12 */
  GPDO_RESET, /* PJ13 */
  GPDO_RESET, /* PJ14 */
  GPDO_RESET, /* PJ15 */
};

//#define GPDI_RESET		0
//const uint8_t PortInConfigData[] =
//{
//  // pad 0-15
//  GPDI_RESET, /* PA0(input only) */
//  GPDI_RESET, /* PA1(input only) */
//  GPDI_RESET, /* PA2(input only) */
//  GPDI_RESET, /* PA3(input only) */
//  GPDI_RESET, /* PA4(input only) */
//  GPDI_RESET, /* PA5(input only) */
//  GPDI_RESET, /* PA6(input only) */
//  GPDI_RESET, /* PA7(input only) */
//  GPDI_RESET, /* PA8(input only) */
//  GPDI_RESET, /* PA9(input only) */
//  GPDI_RESET, /* PA10(input only) */
//  GPDI_RESET, /* PA11(input only) */
//  GPDI_RESET, /* PA12(input only) */
//  GPDI_RESET, /* PA13(input only) */
//  GPDI_RESET, /* PA14(input only) */
//  GPDI_RESET, /* PA15(input only) */
//
//  // pad 16-31
//  GPDI_RESET, /* PB0 */
//  GPDI_RESET, /* PB1 */
//  GPDI_RESET, /* PB2 */
//  GPDI_RESET, /* PB3 */
//  GPDI_RESET, /* PB4 */
//  GPDI_RESET, /* PB5 */
//  GPDI_RESET, /* PB6 */
//  GPDI_RESET, /* PB7 */
//  GPDI_RESET, /* PB8 */
//  GPDI_RESET, /* PB9 */
//  GPDI_RESET, /* PB10 */
//  GPDI_RESET, /* PB11 */
//  GPDI_RESET, /* PB12 */
//  GPDI_RESET, /* PB13 */
//  GPDI_RESET, /* PB14 */
//  GPDI_RESET, /* PB15 */
//
//  // pad 32-47
//  GPDI_RESET, /* PC0 */
//  GPDI_RESET, /* PC1 */
//  GPDI_RESET, /* PC2 */
//  GPDI_RESET, /* PC3 */
//  GPDI_RESET, /* PC4 */
//  GPDI_RESET, /* PC5 */
//  GPDI_RESET, /* PC6 */
//  GPDI_RESET, /* PC7 */
//  GPDI_RESET, /* PC8 */
//  GPDI_RESET, /* PC9 */
//  GPDI_RESET, /* PC10 */
//  GPDI_RESET, /* PC11 */
//  GPDI_RESET, /* PC12 */
//  GPDI_RESET, /* PC13 */
//  GPDI_RESET, /* PC14 */
//  GPDI_RESET, /* PC15 */
//
//  // pad 48-63
//  GPDI_RESET, /* PD0 */
//  GPDI_RESET, /* PD1 */
//  GPDI_RESET, /* PD2 */
//  GPDI_RESET, /* PD3 */
//  GPDI_RESET, /* PD4 */
//  GPDI_RESET, /* PD5 */
//  GPDI_RESET, /* PD6 */
//  GPDI_RESET, /* PD7 */
//  GPDI_RESET, /* PD8 */
//  GPDI_RESET, /* PD9 */
//  GPDI_RESET, /* PD10 */
//  GPDI_RESET, /* PD11 */
//  GPDI_RESET, /* PD12 */
//  GPDI_RESET, /* PD13 */
//  GPDI_RESET, /* PD14 */
//  GPDI_RESET, /* PD15 */
//
//  // pad 64-79
//  GPDI_RESET, /* PE0 */
//  GPDI_RESET, /* PE1 */
//  GPDI_RESET, /* PE2 */
//  GPDI_RESET, /* PE3 */
//  GPDI_RESET, /* PE4 */
//  GPDI_RESET, /* PE5 */
//  GPDI_RESET, /* PE6 */
//  GPDI_RESET, /* PE7 */
//  GPDI_RESET, /* PE8 */
//  GPDI_RESET, /* PE9 */
//  GPDI_RESET, /* PE10 */
//  GPDI_RESET, /* PE11 */
//  GPDI_RESET, /* PE12 */
//  GPDI_RESET, /* PE13 */
//  GPDI_RESET, /* PE14 */
//  GPDI_RESET, /* PE15 */
//
//  // pad 80-95
//  GPDI_RESET, /* PF0 */
//  GPDI_RESET, /* PF1 */
//  GPDI_RESET, /* PF2 */
//  GPDI_RESET, /* PF3 */
//  GPDI_RESET, /* PF4 */
//  GPDI_RESET, /* PF5 */
//  GPDI_RESET, /* PF6 */
//  GPDI_RESET, /* PF7 */
//  GPDI_RESET, /* PF8 */
//  GPDI_RESET, /* PF9 */
//  GPDI_RESET, /* PF10 */
//  GPDI_RESET, /* PF11 */
//  GPDI_RESET, /* PF12 */
//  GPDI_RESET, /* PF13 */
//  GPDI_RESET, /* PF14 */
//  GPDI_RESET, /* PF15 */
//
//  // pad 96-111
//  GPDI_RESET, /* PG0 */
//  GPDI_RESET, /* PG1 */
//  GPDI_RESET, /* PG2 */
//  GPDI_RESET, /* PG3 */
//  GPDI_RESET, /* PG4 */
//  GPDI_RESET, /* PG5 */
//  GPDI_RESET, /* PG6 */
//  GPDI_RESET, /* PG7 */
//  GPDI_RESET, /* PG8 */
//  GPDI_RESET, /* PG9 */
//  GPDI_RESET, /* PG10 */
//  GPDI_RESET, /* PG11 */
//  GPDI_RESET, /* PG12 */
//  GPDI_RESET, /* PG13 */
//  GPDI_RESET, /* PG14 */
//  GPDI_RESET, /* PG15 */
//
//  // pad 112-127
//  GPDI_RESET, /* PH0 */
//  GPDI_RESET, /* PH1 */
//  GPDI_RESET, /* PH2 */
//  GPDI_RESET, /* PH3 */
//  GPDI_RESET, /* PH4 */
//  GPDI_RESET, /* PH5 */
//  GPDI_RESET, /* PH6 */
//  GPDI_RESET, /* PH7 */
//  GPDI_RESET, /* PH8 */
//  GPDI_RESET, /* PH9 */
//  GPDI_RESET, /* PH10 */
//  GPDI_RESET, /* PH11 */
//  GPDI_RESET, /* PH12 */
//  GPDI_RESET, /* PH13 */
//  GPDI_RESET, /* PH14 */
//  GPDI_RESET, /* PH15 */
//
//  // pad 128-143
//  GPDI_RESET, /* PJ0 */
//  GPDI_RESET, /* PJ1 */
//  GPDI_RESET, /* PJ2 */
//  GPDI_RESET, /* PJ3 */
//  GPDI_RESET, /* PJ4 */
//  GPDI_RESET, /* PJ5 */
//  GPDI_RESET, /* PJ6 */
//  GPDI_RESET, /* PJ7 */
//  GPDI_RESET, /* PJ8 */
//  GPDI_RESET, /* PJ9 */
//  GPDI_RESET, /* PJ10 */
//  GPDI_RESET, /* PJ11 */
//  GPDI_RESET, /* PJ12 */
//  GPDI_RESET, /* PJ13 */
//  GPDI_RESET, /* PJ14 */
//  GPDI_RESET, /* PJ15 */
//
//  // pad 144-145
//  GPDI_RESET, /* PK0 (input only) */
//  GPDI_RESET, /* PK1 (input only) */
//};

const Port_ConfigType PortConfigData =
{
	.padCnt = sizeof(PortPadConfigData),
	.padConfig = PortPadConfigData,
	.outCnt = sizeof(PortOutConfigData),
	.outConfig = PortOutConfigData,
//	.inCnt = sizeof(PortInConfigData),
//	.inConfig = PortInConfigData,
};

