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

#ifndef MCU_CONFIGTYPES_H_
#define MCU_CONFIGTYPES_H_

#define RCC_AHBPeriph_DMA1               ((uint32_t)0x00000001)
#define RCC_AHBPeriph_DMA2               ((uint32_t)0x00000002)
#define RCC_AHBPeriph_SRAM               ((uint32_t)0x00000004)
#define RCC_AHBPeriph_FLITF              ((uint32_t)0x00000010)
#define RCC_AHBPeriph_CRC                ((uint32_t)0x00000040)

#ifndef STM32F10X_CL
 #define RCC_AHBPeriph_FSMC              ((uint32_t)0x00000100)
 #define RCC_AHBPeriph_SDIO              ((uint32_t)0x00000400)
#else
 #define RCC_AHBPeriph_OTG_FS            ((uint32_t)0x00001000)
 #define RCC_AHBPeriph_ETH_MAC           ((uint32_t)0x00004000)
 #define RCC_AHBPeriph_ETH_MAC_Tx        ((uint32_t)0x00008000)
 #define RCC_AHBPeriph_ETH_MAC_Rx        ((uint32_t)0x00010000)
#endif

#define RCC_APB1Periph_TIM2              ((uint32_t)0x00000001)
#define RCC_APB1Periph_TIM3              ((uint32_t)0x00000002)
#define RCC_APB1Periph_TIM4              ((uint32_t)0x00000004)
#define RCC_APB1Periph_TIM5              ((uint32_t)0x00000008)
#define RCC_APB1Periph_TIM6              ((uint32_t)0x00000010)
#define RCC_APB1Periph_TIM7              ((uint32_t)0x00000020)
#define RCC_APB1Periph_WWDG              ((uint32_t)0x00000800)
#define RCC_APB1Periph_SPI2              ((uint32_t)0x00004000)
#define RCC_APB1Periph_SPI3              ((uint32_t)0x00008000)
#define RCC_APB1Periph_USART2            ((uint32_t)0x00020000)
#define RCC_APB1Periph_USART3            ((uint32_t)0x00040000)
#define RCC_APB1Periph_UART4             ((uint32_t)0x00080000)
#define RCC_APB1Periph_UART5             ((uint32_t)0x00100000)
#define RCC_APB1Periph_I2C1              ((uint32_t)0x00200000)
#define RCC_APB1Periph_I2C2              ((uint32_t)0x00400000)
#define RCC_APB1Periph_USB               ((uint32_t)0x00800000)
#define RCC_APB1Periph_CAN1              ((uint32_t)0x02000000)
#define RCC_APB1Periph_BKP               ((uint32_t)0x08000000)
#define RCC_APB1Periph_PWR               ((uint32_t)0x10000000)
#define RCC_APB1Periph_DAC               ((uint32_t)0x20000000)
#define RCC_APB1Periph_CAN2              ((uint32_t)0x04000000)

#define RCC_APB2Periph_AFIO              ((uint32_t)0x00000001)
#define RCC_APB2Periph_GPIOA             ((uint32_t)0x00000004)
#define RCC_APB2Periph_GPIOB             ((uint32_t)0x00000008)
#define RCC_APB2Periph_GPIOC             ((uint32_t)0x00000010)
#define RCC_APB2Periph_GPIOD             ((uint32_t)0x00000020)
#define RCC_APB2Periph_GPIOE             ((uint32_t)0x00000040)
#define RCC_APB2Periph_GPIOF             ((uint32_t)0x00000080)
#define RCC_APB2Periph_GPIOG             ((uint32_t)0x00000100)
#define RCC_APB2Periph_ADC1              ((uint32_t)0x00000200)
#define RCC_APB2Periph_ADC2              ((uint32_t)0x00000400)
#define RCC_APB2Periph_TIM1              ((uint32_t)0x00000800)
#define RCC_APB2Periph_SPI1              ((uint32_t)0x00001000)
#define RCC_APB2Periph_TIM8              ((uint32_t)0x00002000)
#define RCC_APB2Periph_USART1            ((uint32_t)0x00004000)
#define RCC_APB2Periph_ADC3              ((uint32_t)0x00008000)

typedef struct {
	uint32 AHBClocksEnable;
	uint32 APB1ClocksEnable;
	uint32 APB2ClocksEnable;
} Mcu_PerClockConfigType;

extern const Mcu_PerClockConfigType McuPerClockConfigData;


#endif /* MCU_CONFIGTYPES_H_ */
