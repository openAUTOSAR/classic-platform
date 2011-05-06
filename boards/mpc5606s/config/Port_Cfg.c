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









#warning "This default file may only be used as an example!"

#include "Port_Cfg.h"

#define 	PCR_PWM			PORT_PCR_BOOTCFG
#define 	PCR_ADC			(0)
#define		PCR_IO_INPUT	PORT_PCR_BOOTCFG
#define		PCR_IO_OUTPUT	PORT_OBE_ENABLE
#define		PCR_CAN			(0)
#define 	PCR_SPI			PORT_PCR_BOOTCFG
#define 	PCR_I2C			PORT_PCR_BOOTCFG


const uint16_t PortPadConfigData[] =
{
	// pad 0-15
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE), /* PA0, AN0 */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE), /* PA1      */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE), /* PA2      */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE),/* PA3      */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE), /* PA4      */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE), /* PA5 		*/
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE), /* PA6,     */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE), /* PA7(input only) */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE), /* PA8(input only) */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE), /* PA9(input only) */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE), /* PA10(input only) */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE), /* PA11(input only) */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE), /* PA12(input only) */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE), /* PA13(input only) */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE), /* PA14(input only) */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE), /* PA15(input only) */

	// pad 16-29
	(PORT_PA_FUNC1 | PORT_OBE_ENABLE | PORT_SLEW_RATE_MED | PORT_ODE_ENABLE), /* PB0 */
	(PORT_PA_FUNC1 | PORT_IBE_ENABLE), /* PB1 */
	PORT_PCR_RESET, /* PB2 */
	PORT_PCR_RESET, /* PB3 */
	PORT_PCR_RESET|PORT_IBE_ENABLE, /* PB4 */
	PORT_PCR_RESET|PORT_IBE_ENABLE, /* PB5 */
	PORT_PCR_RESET|PORT_IBE_ENABLE, /* PB6 */
	PORT_PCR_RESET|PORT_IBE_ENABLE, /* PB7 */
	PORT_PCR_RESET|PORT_IBE_ENABLE, /* PB8 */
	PORT_PCR_RESET|PORT_IBE_ENABLE, /* PB9 */
	(PORT_PA_FUNC1 | PORT_IBE_ENABLE), /* PB10 */
	(PORT_PA_FUNC1 | PORT_OBE_ENABLE | PORT_SLEW_RATE_MED | PORT_ODE_ENABLE), /* PB11 */
	PORT_PCR_RESET, /* PB12 */
	PORT_PCR_RESET, /* PB13 */
	//PORT_PCR_RESET, /* PB14 ,Reserved*/
	//PORT_PCR_RESET, /* PB15 ,Reserved*/

	// pad 30-45
	PORT_PCR_RESET, /* PC0/emios0 */
	PORT_PCR_RESET, // PC1
	PORT_PCR_RESET, // PC2
	PORT_PCR_RESET, /* PC3 */
	PORT_PCR_RESET, /* PC4 */
	PORT_PCR_RESET, /* PC5 */
	PORT_PCR_RESET, /* PC6 */
	PORT_PCR_RESET, /* PC7 */
	PORT_PCR_RESET, /* PC8 */
    PORT_PCR_RESET, /* PC9 */
	PORT_PCR_RESET, /* PC10 */
	PORT_PCR_RESET, /* PC11 */
	PORT_PCR_RESET, /* PC12 */
	PORT_PCR_RESET, /* PC13 */
	PORT_PCR_RESET, /* PC14 */
	PORT_PCR_RESET, /* PC15 */

	// pad 46-61
	( PORT_PA_FUNC3 | PORT_OBE_ENABLE ), /* PD0, CAN_A, Tx */
	( PORT_PA_FUNC3 | PORT_OBE_ENABLE ), /* PD1, CAN_A, Rx */
	( PORT_PA_FUNC3 | PORT_OBE_ENABLE ),/* PD2 */
	( PORT_PA_FUNC3 | PORT_OBE_ENABLE ), /* PD3 */
	( PORT_PA_FUNC3 | PORT_OBE_ENABLE ), /* PD4, LED */
	( PORT_PA_FUNC3 | PORT_OBE_ENABLE ), /* PD5, LED */
	( PORT_PA_FUNC3 | PORT_OBE_ENABLE ), /* PD6 , SCI_RS232??*/
	( PORT_PA_FUNC3 | PORT_OBE_ENABLE ), /* PD7 , SCI_RS232??*/
	PORT_PCR_RESET|PORT_IBE_ENABLE, /* PD8 LIN TX */
	PORT_PCR_RESET|PORT_IBE_ENABLE,/* PD9 LIN RX */
	PORT_PCR_RESET|PORT_IBE_ENABLE, /* PD10 */
	PORT_PCR_RESET|PORT_IBE_ENABLE, /* PD11 */
	(PORT_PA_FUNC1 | PORT_OBE_ENABLE | PORT_SLEW_RATE_MAX), /* PD12 SPIB_CS0 */
	(PORT_PA_FUNC1 | PORT_OBE_ENABLE | PORT_SLEW_RATE_MAX), /* PD13 SPIB_CLK*/
	(PORT_PA_FUNC1 | PORT_OBE_ENABLE | PORT_SLEW_RATE_MAX), /* PD14 SPIB_SO*/
	(PORT_PA_FUNC1 | PORT_IBE_ENABLE),                 /* PD15 SPIB_SI*/

	// pad 62-69
	(PORT_PA_FUNC1 | PORT_OBE_ENABLE | PORT_SLEW_RATE_MAX), /* PE0 SPIA_CS1 */
	(PORT_PA_FUNC1 | PORT_OBE_ENABLE | PORT_SLEW_RATE_MAX), /* PE1 SPIA_CS2*/
	PORT_PCR_RESET, /* PE2 */
	(PORT_PA_FUNC1 | PORT_OBE_ENABLE | PORT_SLEW_RATE_MAX), /* PE3 SPIA_CLK*/
	( PORT_PCR_RESET | PORT_OBE_ENABLE), /* PE4 SPIA_SO*/
	( PORT_PCR_RESET | PORT_OBE_ENABLE), /* PE5 SPIA_SI*/
	( PORT_PCR_RESET | PORT_OBE_ENABLE), /* PE6 */
	( PORT_PCR_RESET | PORT_OBE_ENABLE), /* PE7 */
	//PORT_PCR_RESET, /* PE8 ,Reserved*/
	//PORT_PCR_RESET, /* PE9 ,Reserved*/
	//PORT_PCR_RESET, /* PE10 ,Reserved*/
	//PORT_PCR_RESET, /* PE11 ,Reserved*/
	//PORT_PCR_RESET, /* PE12 ,Reserved*/
	//PORT_PCR_RESET, /* PE13 ,Reserved*/
	//PORT_PCR_RESET, /* PE14 ,Reserved*/
	//PORT_PCR_RESET, /* PE15 ,Reserved*/

	// pad 70-85
	PORT_PCR_RESET, /* PF0 */
	PORT_PCR_RESET, /* PF1 */
	PORT_PCR_RESET, /* PF2 */
	PORT_PCR_RESET, /* PF3 */
	PORT_PCR_RESET, /* PF4 */
	PORT_PCR_RESET, /* PF5 */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE ), /* PF6 */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE ), /* PF7 */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE ), /* PF8 */
	( PORT_PCR_RESET | PORT_OBE_ENABLE | PORT_IBE_ENABLE ), /* PF9 */
	PORT_PCR_RESET, /* PF10 */
	PORT_PCR_RESET, /* PF11 */
	PORT_PA_FUNC1|PORT_OBE_ENABLE|PORT_PULL_UP, /* PF12 SCI_D Transmit, J1708 Tx*/
	PORT_PA_FUNC1|PORT_IBE_ENABLE|PORT_PULL_UP, /* PF13 SCI_D Receive, J1708 Rx*/
	PORT_PCR_RESET, /* PF14 */
	PORT_PCR_RESET, /* PF15 */

	// pad 86-98
	PORT_PCR_RESET, /* PG0  */
	PORT_PA_FUNC2|PORT_IBE_ENABLE|PORT_PULL_UP, /* PG1 eMIOS[17], J1708 Edgedetect*/
	PORT_PCR_RESET, /* PG2 */
	PORT_PCR_RESET, /* PG3 */
	PORT_PCR_RESET, /* PG4 */
	PORT_PCR_RESET, /* PG5 */
	PORT_PCR_RESET, /* PG6 */
	PORT_PCR_RESET, /* PG7 */
	PORT_PCR_RESET, /* PG8 */
	PORT_PCR_RESET, /* PG9 */
	PORT_PCR_RESET, /* PG10 */
	PORT_PCR_RESET, /* PG11 */
	( PORT_PA_FUNC2 | PORT_OBE_ENABLE ), /* PG12 */
	//PORT_PCR_RESET, /* PG13 ,Reserved*/
	//PORT_PCR_RESET, /* PG14 ,Reserved*/
	//PORT_PCR_RESET, /* PG15 ,Reserved*/

	// pad 99-104
	PORT_PCR_RESET, /* PH0 ,As TCK of JTAG*/
	PORT_PCR_RESET, /* PH1 ,As TCK of JTAG*/
	PORT_PCR_RESET, /* PH2 ,As TCK of JTAG*/
	PORT_PCR_RESET, /* PH3 ,As TCK of JTAG*/
	(PORT_PA_FUNC3|PORT_OBE_ENABLE), /* PH4 */
	PORT_PCR_RESET, /* PH5 */
	//PORT_PCR_RESET, /* PH6 ,Reserved*/
	//PORT_PCR_RESET, /* PH7 ,Reserved*/
	//PORT_PCR_RESET,  /* PH8 ,Reserved*/
	//PORT_PCR_RESET, /* PH9 ,Reserved*/
	//PORT_PCR_RESET, /* PH10 ,Reserved*/

	//PORT_PCR_RESET, /* PH11 ,Reserved*/
	//PORT_PCR_RESET, /* PH12 ,Reserved*/
	//PORT_PCR_RESET, /* PH13 ,Reserved*/
	//PORT_PCR_RESET, /* PH14 ,Reserved*/
	//PORT_PCR_RESET, /* PH15 ,Reserved*/

	// pad 105-120
	PORT_PCR_RESET, /* PJ0 */
	PORT_PCR_RESET, /* PJ1 */
	PORT_PCR_RESET, /* PJ2 */
	PORT_PCR_RESET, /* PJ3 */
	PORT_PCR_RESET, /* PJ4 */
	PORT_PCR_RESET, /* PJ5 */
	PORT_PCR_RESET, /* PJ6 */
	PORT_PCR_RESET, /* PJ7 */
	PORT_PCR_RESET, /* PJ8 */
	PORT_PCR_RESET, /* PJ9 */
	PORT_PCR_RESET, /* PJ10 */
	PORT_PCR_RESET, /* PJ11 */
	PORT_PCR_RESET, /* PJ12 */
	PORT_PCR_RESET, /* PJ13 */
	PORT_PCR_RESET, /* PJ14 */
	PORT_PCR_RESET, /* PJ15 */

	// pad 121-132
	PORT_PCR_RESET, /* PK0 */
	PORT_PCR_RESET, /* PK1 */
	/*Added by Cobb Song for MPC5606S*/
	PORT_PCR_RESET, /* PK2 */
	PORT_PCR_RESET, /* PK3 */
	PORT_PCR_RESET, /* PK4 */
	PORT_PCR_RESET, /* PK5 */
	PORT_PCR_RESET, /* PK6 */
	PORT_PCR_RESET, /* PK7 */
	PORT_PCR_RESET, /* PK8 */
	PORT_PCR_RESET, /* PK9 */
	PORT_PCR_RESET, /* PK10 */
	PORT_PCR_RESET, /* PK11 */
	//PORT_PCR_RESET, /* PK12 ,Reserved*/
	//PORT_PCR_RESET, /* PK13 ,Reserved*/
	//PORT_PCR_RESET, /* PK14 ,Reserved*/
	//PORT_PCR_RESET  /* PK15 ,Reserved*/
};

#define PORT_GPDO_RESET 0

const uint8_t PortOutConfigData[]=
{
	// pad 0-15 added by Cobb
	PORT_GPDO_RESET, /* PA0 */
	PORT_GPDO_RESET, /* PA1 */
	PORT_GPDO_RESET, /* PA2 */
	PORT_GPDO_RESET, /* PA3 */
	PORT_GPDO_RESET, /* PA4 */
	PORT_GPDO_RESET, /* PA5 */
	PORT_GPDO_RESET, /* PA6 */
	PORT_GPDO_RESET, /* PA7 */
	PORT_GPDO_RESET, /* PA8 */
	PORT_GPDO_RESET, /* PA9 */
	PORT_GPDO_RESET, /* PA10 */
	PORT_GPDO_RESET, /* PA11 */
	PORT_GPDO_RESET, /* PA12 */
	PORT_GPDO_RESET, /* PA13 */
	PORT_GPDO_RESET, /* PA14 */
	PORT_GPDO_RESET, /* PA15 */

	// pad 16-29
	PORT_GPDO_RESET, /* PB0 */
	PORT_GPDO_RESET, /* PB1 */
	PORT_GPDO_RESET, /* PB2 */
	PORT_GPDO_RESET, /* PB3 */
	PORT_GPDO_RESET, /* PB4 */
	PORT_GPDO_RESET, /* PB5 */
	PORT_GPDO_RESET, /* PB6 */
	PORT_GPDO_RESET, /* PB7 */
	PORT_GPDO_RESET, /* PB8 */
	PORT_GPDO_RESET, /* PB9 */
	PORT_GPDO_RESET, /* PB10 */
	PORT_GPDO_RESET, /* PB11 */
	PORT_GPDO_RESET, /* PB12 */
	PORT_GPDO_RESET, /* PB13 */
	//PORT_GPDO_RESET, /* PB14 ,Reserved*/
	//PORT_GPDO_RESET, /* PB15 ,Reserved*/

	// pad 30-45
	PORT_GPDO_RESET, /* PC0 */
	PORT_GPDO_RESET, /* PC1 */
	PORT_GPDO_RESET, /* PC2 */
	PORT_GPDO_RESET, /* PC3 */
	PORT_GPDO_RESET, /* PC4 */
	PORT_GPDO_RESET, /* PC5 */
	PORT_GPDO_RESET, /* PC6 */
	PORT_GPDO_RESET, /* PC7 */
	PORT_GPDO_RESET, /* PC8 */
	PORT_GPDO_RESET, /* PC9 */
	PORT_GPDO_RESET, /* PC10 */
	PORT_GPDO_RESET, /* PC11 */
	PORT_GPDO_RESET, /* PC12 */
	PORT_GPDO_RESET, /* PC13 */
	PORT_GPDO_RESET, /* PC14 */
	PORT_GPDO_RESET, /* PC15 */

	// pad 46-61
	PORT_GPDO_RESET, /* PD0 */
	PORT_GPDO_RESET, /* PD1 */
	PORT_GPDO_RESET, /* PD2 */
	PORT_GPDO_RESET, /* PD3 */
	PORT_GPDO_RESET, /* PD4 */
	PORT_GPDO_RESET, /* PD5 */
	PORT_GPDO_RESET, /* PD6 */
	PORT_GPDO_RESET, /* PD7 */
	PORT_GPDO_RESET, /* PD8 */
	PORT_GPDO_RESET, /* PD9 */
	PORT_GPDO_RESET, /* PD10 */
	PORT_GPDO_RESET, /* PD11 */
	PORT_GPDO_RESET, /* PD12 */
	PORT_GPDO_RESET, /* PD13 */
	PORT_GPDO_RESET, /* PD14 */
	PORT_GPDO_RESET, /* PD15 */

	// pad 62-69
	PORT_GPDO_RESET, /* PE0 */
	PORT_GPDO_RESET, /* PE1 */
	PORT_GPDO_RESET, /* PE2 */
	PORT_GPDO_RESET, /* PE3 */
	PORT_GPDO_RESET, /* PE4 */
	PORT_GPDO_RESET, /* PE5 */
	PORT_GPDO_RESET, /* PE6 */
	PORT_GPDO_RESET, /* PE7 */
	//PORT_GPDO_RESET, /* PE8 ,Reserved*/
	//PORT_GPDO_RESET, /* PE9 ,Reserved*/
	//PORT_GPDO_RESET, /* PE10,Reserved*/
	//PORT_GPDO_RESET, /* PE11 ,Reserved*/
	//PORT_GPDO_RESET, /* PE12 ,Reserved*/
	//PORT_GPDO_RESET, /* PE13 ,Reserved*/
	//PORT_GPDO_RESET, /* PE14 ,Reserved*/
	//PORT_GPDO_RESET, /* PE15 ,Reserved*/

	// pad 70-85
	PORT_GPDO_RESET, /* PF0 */
	PORT_GPDO_RESET, /* PF1 */
	PORT_GPDO_RESET, /* PF2 */
	PORT_GPDO_RESET, /* PF3 */
	PORT_GPDO_RESET, /* PF4 */
	PORT_GPDO_RESET, /* PF5 */
	PORT_GPDO_RESET, /* PF6 */
	PORT_GPDO_RESET, /* PF7 */
	PORT_GPDO_RESET, /* PF8 */
	PORT_GPDO_RESET, /* PF9 */
	PORT_GPDO_RESET, /* PF10 */
	PORT_GPDO_RESET, /* PF11 */
	PORT_GPDO_RESET, /* PF12 */
	PORT_GPDO_RESET, /* PF13 */
	PORT_GPDO_RESET, /* PF14 */
	PORT_GPDO_RESET, /* PF15 */

	// pad 86-98
	PORT_GPDO_RESET, /* PG0 */
	PORT_GPDO_RESET, /* PG1 */
	PORT_GPDO_RESET, /* PG2 */
	PORT_GPDO_RESET, /* PG3 */
	PORT_GPDO_RESET, /* PG4 */
	PORT_GPDO_RESET, /* PG5 */
	PORT_GPDO_RESET, /* PG6 */
	PORT_GPDO_RESET, /* PG7 */
	PORT_GPDO_RESET, /* PG8 */
	PORT_GPDO_RESET, /* PG9 */
	PORT_GPDO_RESET, /* PG10 */
	PORT_GPDO_RESET, /* PG11 */
	PORT_GPDO_RESET, /* PG12 */
	//PORT_GPDO_RESET, /* PG13 ,Reserved*/
	//PORT_GPDO_RESET, /* PG14 ,Reserved*/
	//PORT_GPDO_RESET, /* PG15 ,Reserved*/

	// pad 99-104
	PORT_GPDO_RESET, /* PH0 ,As TCK of JTAG*/
	PORT_GPDO_RESET, /* PH1 ,As TDI of JTAG*/
	PORT_GPDO_RESET, /* PH2 ,As TDO of JTAG*/
	PORT_GPDO_RESET, /* PH3 ,As TMS of JTAG*/
	PORT_GPDO_RESET, /* PH4 */
	PORT_GPDO_RESET, /* PH5 */
	//PORT_GPDO_RESET, /* PH6 ,Reserved*/
	//PORT_GPDO_RESET, /* PH7 ,Reserved*/
	//PORT_GPDO_RESET,  /* PH8 ,Reserved*/
	//PORT_GPDO_RESET, /* PH9 ,Reserved*/
	//PORT_GPDO_RESET, /* PH10 ,Reserved*/
	//PORT_GPDO_RESET, /* PH11 ,Reserved*/
	//PORT_GPDO_RESET, /* PH12 ,Reserved*/
	//PORT_GPDO_RESET, /* PH13 ,Reserved*/
	//PORT_GPDO_RESET, /* PH14 ,Reserved*/
	//PORT_GPDO_RESET, /* PH15 ,Reserved*/

	// pad 105-120
	PORT_GPDO_RESET, /* PJ0 */
	PORT_GPDO_RESET, /* PJ1 */
	PORT_GPDO_RESET, /* PJ2 */
	PORT_GPDO_RESET, /* PJ3 */
	PORT_GPDO_RESET, /* PJ4 */
	PORT_GPDO_RESET, /* PJ5 */
	PORT_GPDO_RESET, /* PJ6 */
	PORT_GPDO_RESET, /* PJ7 */
	PORT_GPDO_RESET, /* PJ8 */
	PORT_GPDO_RESET, /* PJ9 */
	PORT_GPDO_RESET, /* PJ10 */
	PORT_GPDO_RESET, /* PJ11 */
	PORT_GPDO_RESET, /* PJ12 */
	PORT_GPDO_RESET, /* PJ13 */
	PORT_GPDO_RESET, /* PJ14 */
	PORT_GPDO_RESET, /* PJ15 */

	// pad 121-132
	PORT_GPDO_RESET, /* PK0 */
	PORT_GPDO_RESET, /* PK1 */
	PORT_GPDO_RESET, /* PK2 */
	PORT_GPDO_RESET, /* PK3 */
	PORT_GPDO_RESET, /* PK4 */
	PORT_GPDO_RESET, /* PK5 */
	PORT_GPDO_RESET, /* PK6 */
	PORT_GPDO_RESET, /* PK7 */
	PORT_GPDO_RESET, /* PK8 */
	PORT_GPDO_RESET, /* PK9 */
	PORT_GPDO_RESET, /* PK10 */
	PORT_GPDO_RESET, /* PK11 */
	//PORT_GPDO_RESET, /* PK12 ,Reserved*/
	//PORT_GPDO_RESET, /* PK13 ,Reserved*/
	//PORT_GPDO_RESET, /* PK14 ,Reserved*/
	//PORT_GPDO_RESET,  /* PK15 ,Reserved*/
};

const Port_ConfigType PortConfigData =
{
	//.padCnt = sizeof(PortPadConfigData),
	.padCnt = sizeof(PortPadConfigData)/sizeof(PortPadConfigData[0]),
	.padConfig = PortPadConfigData,
	//.outCnt = sizeof(PortOutConfigData),
	.outCnt = sizeof(PortOutConfigData)/sizeof(PortOutConfigData[0]),
	.outConfig = PortOutConfigData,
	//.inCnt = sizeof(PortInConfigData),
	//.inConfig = PortInConfigData,
};

