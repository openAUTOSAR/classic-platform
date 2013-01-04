#ifndef _REGS_HCS12XD_H_
#define _REGS_HCS12XD_H_

#define IO_BASE	0

#define ienable() __asm("cli");
#define idisable() __asm("orcc #0x10")
#define xenable() __asm("andcc #0xbf")
#define xdisable()__asm("orcc #0x40")

/**
 * constant offsets to use where a C expression doesn't work
 * You may use it with GEL
 */
#define  M6811_PORTA     0x00
#define  M6811_PORTB     0x01
#define  M6811_PTT       0x240
#define  M6811_PTM       0x250
#define  M6811_PTP       0x258
#define  M6811_PTH       0x260

#define PORTIO_8		*(volatile unsigned char *)
#define PORTIO_16		*(volatile unsigned short int *)


/* 
 * Core HC12 Registers
 */
#define  PORTA     PORTIO_8(IO_BASE + 0x00)   /* port A */
#define  PORTB     PORTIO_8(IO_BASE + 0x01)   /* port B */
#define  DDRA      PORTIO_8(IO_BASE + 0x02)   /* data direction port A */
#define  DDRB      PORTIO_8(IO_BASE + 0x03)   /* data direction port B */
#define  PORTC     PORTIO_8(IO_BASE + 0x04)   /* port C */
#define  PORTD     PORTIO_8(IO_BASE + 0x05)   /* port D */
#define  DDRC      PORTIO_8(IO_BASE + 0x06)   /* data direction port C */
#define  DDRD      PORTIO_8(IO_BASE + 0x07)   /* data direction port D */
#define  PORTE     PORTIO_8(IO_BASE + 0x08)   /* port E */
#define  DDRE      PORTIO_8(IO_BASE + 0x09)   /* data direction port E */
#define  MMCCTL0   PORTIO_8(IO_BASE + 0x0a)   /* MMCTL register 0*/
#define  MODE      PORTIO_8(IO_BASE + 0x0b)   /* mode register */
#define  PUCR      PORTIO_8(IO_BASE + 0x0c)   /* pull-up control register */
#define  RDRIV     PORTIO_8(IO_BASE + 0x0d)   /* reduced drive of I/O lines */
#define  EBICTL0   PORTIO_8(IO_BASE + 0x0e)   /* external bus control 0 */
#define  EBICTL1   PORTIO_8(IO_BASE + 0x0f)   /* external bus control 1 */
#define  MMCCTL1   PORTIO_8(IO_BASE + 0x13)   /* MMCTL register 1*/
#define  ECLKCTL   PORTIO_8(IO_BASE + 0x1c)   /* ECLKCTL register */

// Bitflags - PEAR
#define NOACCE 0x80
#define PIPOE  0x20
#define NECLK  0x10
#define LSTRE  0x08
#define RDWE   0x04

// INTCR bitflags
#define IRQE 0x80
#define IRQEN 0x40
#define DLY 0x20

// BKP Module
#define  BKPCT0    PORTIO_8(IO_BASE + 0x28)   /* Breakpoint Control 0 */
#define  BKPCT1    PORTIO_8(IO_BASE + 0x29)   /* Breakpoint Control 1 */
#define  BKP0X     PORTIO_8(IO_BASE + 0x2a)   /* Breakpoint 0 address upper */
#define  BKP0      PORTIO_16(IO_BASE + 0x2b)   /* Breakpoint 0 address */
#define  BKP1X     PORTIO_8(IO_BASE + 0x2d)   /* Breakpoint 1 address upper */
#define  BKP1      PORTIO_16(IO_BASE + 0x2e)   /* Breakpoint 1 address */

// MEBI Module  
#define  PPAGE     PORTIO_8(IO_BASE + 0x30)   /* program page register */
#define  PORTK     PORTIO_8(IO_BASE + 0x32)   /* port K data register */
#define  DDRK      PORTIO_8(IO_BASE + 0x33)   /* port K data direction */

//  CRG Module  
#define  SYNR      PORTIO_8(IO_BASE + 0x34)   /* synthesizer register */
#define  REFDV     PORTIO_8(IO_BASE + 0x35)   /* reference divider register */
#define  CTFLG     PORTIO_8(IO_BASE + 0x36)   /* clock test flag register */
#define  CRGFLG    PORTIO_8(IO_BASE + 0x37)   /* clock generator flag register */
#define  CRGINT    PORTIO_8(IO_BASE + 0x38)   /* clock interrupt enable */
#define  CLKSEL    PORTIO_8(IO_BASE + 0x39)   /* clock select register */
#define  PLLCTL    PORTIO_8(IO_BASE + 0x3a)   /* PLL control register */
#define  RTICTL    PORTIO_8(IO_BASE + 0x3b)   /* clock real time control reg. */
#define  COPCTL    PORTIO_8(IO_BASE + 0x3c)   /* COP control register */
#define  FORBYP    PORTIO_8(IO_BASE + 0x3d)   /* clock force and bypass register */
#define  CTCTL     PORTIO_8(IO_BASE + 0x3e)   /* clock test control register */
#define  ARMCOP    PORTIO_8(IO_BASE + 0x3f)   /* COP arm/reset register with sequence 0x55,0xaa. */

// CRG bitflags
#define RTIF   0x80
#define RTIE   0x80
#define LOCK   0x08
#define AUTO   0x20
#define PLLSEL 0x80

// COPCTL bitflags
#define WCOP   0x80
#define RSBCK  0x40

//  ECT Module
#define  TIOS      PORTIO_8(IO_BASE + 0x40)   /* timer select register */
#define  TCFORC    PORTIO_8(IO_BASE + 0x41)   /* compare force register */
#define  TOC7M     PORTIO_8(IO_BASE + 0x42)   /* oc7 mask register */
#define  TOC7D     PORTIO_8(IO_BASE + 0x43)   /* oc7 data register */
#define  TCNT      PORTIO_16(IO_BASE + 0x44)   /* timer counter */
#define  TSCR1     PORTIO_8(IO_BASE + 0x46)   /* system control register 1 */
#define  TTOV      PORTIO_8(IO_BASE + 0x47)   /* toggle on overflow register */
#define  TCTL1     PORTIO_8(IO_BASE + 0x48)   /* control register 1 */
#define  TCTL2     PORTIO_8(IO_BASE + 0x49)   /* control register 2 */
#define  TCTL3     PORTIO_8(IO_BASE + 0x4a)   /* control register 3 */
#define  TCTL4     PORTIO_8(IO_BASE + 0x4b)   /* control register 4 */
#define  TIE	   PORTIO_8(IO_BASE + 0x4c)   /* interrupt enable register */
#define  TSCR2     PORTIO_8(IO_BASE + 0x4d)   /* system control register 2 */
#define  TFLG1     PORTIO_8(IO_BASE + 0x4e)   /* interrupt flag register 1 */
#define  TFLG2     PORTIO_8(IO_BASE + 0x4f)   /* interrupt flag register 2 */
#define  TC0       PORTIO_16(IO_BASE + 0x50)   /* capture/compare register 0 */
#define  TC1       PORTIO_16(IO_BASE + 0x52)   /* capture/compare register 0 */
#define  TC2       PORTIO_16(IO_BASE + 0x54)   /* capture/compare register 0 */
#define  TC3       PORTIO_16(IO_BASE + 0x56)   /* capture/compare register 0 */
#define  TC4       PORTIO_16(IO_BASE + 0x58)   /* capture/compare register 0 */
#define  TC5       PORTIO_16(IO_BASE + 0x5a)   /* capture/compare register 0 */
#define  TC6       PORTIO_16(IO_BASE + 0x5c)   /* capture/compare register 0 */
#define  TC7       PORTIO_16(IO_BASE + 0x5e)   /* capture/compare register 0 */
#define  PACTL     PORTIO_8(IO_BASE + 0x60)   /* pulse accumulator A control */
#define  PAFLG     PORTIO_8(IO_BASE + 0x61)   /* pulse accumulator A flag */
#define  PACN3     PORTIO_8(IO_BASE + 0x62)   /* pulse accumulator A3 count */
#define  PACN2     PORTIO_8(IO_BASE + 0x63)   /* pulse accumulator A2 count */
#define  PACN1     PORTIO_8(IO_BASE + 0x64)   /* pulse accumulator A1 count */
#define  PACN0     PORTIO_8(IO_BASE + 0x65)   /* pulse accumulator A0 count */
#define  MCCTL     PORTIO_8(IO_BASE + 0x66)   /* modulus counter control reg */
#define  MCFLG     PORTIO_8(IO_BASE + 0x67)   /* modulus counter flag reg */
#define  ICPAR     PORTIO_8(IO_BASE + 0x68)   /* input control pulse acc reg */
#define  DLYCT     PORTIO_8(IO_BASE + 0x69)   /* delay counter control reg */
#define  ICOVW     PORTIO_8(IO_BASE + 0x6a)   /* input control overwrite reg */
#define  ICSYS     PORTIO_8(IO_BASE + 0x6b)   /* input control system reg */
#define  TIMTST    PORTIO_8(IO_BASE + 0x6d)   /* timer test register */
#define  PBCTL     PORTIO_8(IO_BASE + 0x70)   /* pulse accumulator B control */
#define  PBFLG     PORTIO_8(IO_BASE + 0x71)   /* pulse accumulator B flag */
#define  PA3H      PORTIO_8(IO_BASE + 0x72)   /* pulse accumulator B3 count */
#define  PA2H      PORTIO_8(IO_BASE + 0x73)   /* pulse accumulator B2 count */
#define  PA1H      PORTIO_8(IO_BASE + 0x74)   /* pulse accumulator B1 count */
#define  PA0H      PORTIO_8(IO_BASE + 0x75)   /* pulse accumulator B0 count */
#define  MCCNT     PORTIO_16(IO_BASE + 0x76)   /* modulus counter count reg */
#define  TC0H      PORTIO_16(IO_BASE + 0x78)   /* timer input capture hold 0 */
#define  TC1H      PORTIO_16(IO_BASE + 0x7a)   /* timer input capture hold 1 */
#define  TC2H      PORTIO_16(IO_BASE + 0x7c)   /* timer input capture hold 2 */
#define  TC3H      PORTIO_16(IO_BASE + 0x7e)   /* timer input capture hold 3 */

#define  TEN       0x80
#define  TSFRZ     0x20
#define  TOI       0x80
#define  MCZI      0x80
#define  MODMC     0x40
#define  MCEN      0x4
#define  MCPR1     0x2
#define  MCPR0     0x1

#define MCPRE_VAL_1		0
#define MCPRE_VAL_4		MCPR0
#define MCPRE_VAL_8		MCPR1
#define MCPRE_VAL_16	MCPR0 | MCPR1

// ATD1 Module
#define  ATD1_BASE (IO_BASE + 0x80)
#define  ATD1CTL0  PORTIO_8(IO_BASE + 0x80)   /* A/D1 control register 0 */
#define  ATD1CTL1  PORTIO_8(IO_BASE + 0x81)   /* A/D1 control register 1 */
#define  ATD1CTL2  PORTIO_8(IO_BASE + 0x82)   /* A/D1 control register 2 */
#define  ATD1CTL3  PORTIO_8(IO_BASE + 0x83)   /* A/D1 control register 3 */
#define  ATD1CTL4  PORTIO_8(IO_BASE + 0x84)   /* A/D1 control register 4 */
#define  ATD1CTL5  PORTIO_8(IO_BASE + 0x85)   /* A/D1 control register 5 */
#define  ATD1STAT0 PORTIO_8(IO_BASE + 0x86)   /* A/D1 status register 0 */
#define  ATD1STAT1 PORTIO_8(IO_BASE + 0x87)   /* A/D1 status register 1 */
#define  ATD1TEST0 PORTIO_8(IO_BASE + 0x88)   /* A/D1 test register 0 */
#define  ATD1TEST1 PORTIO_8(IO_BASE + 0x89)   /* A/D1 test register 1 */
#define  ATD1DIEN  PORTIO_8(IO_BASE + 0x8d)   /* A/D1 interrupt enable */
#define  PORTAD1   PORTIO_8(IO_BASE + 0x8f)   /* port AD1 data input register */
#define  ATD1DR0   PORTIO_16(IO_BASE + 0x90)   /* A/D1 result 0 */
#define  ATD1DR0H  PORTIO_8(IO_BASE + 0x90)    /* A/D1 result 0 */
#define  ATD1DR1   PORTIO_16(IO_BASE + 0x92)   /* A/D1 result 1 */
#define  ATD1DR1H  PORTIO_8(IO_BASE + 0x92)    /* A/D1 result 1 */
#define  ATD1DR2   PORTIO_16(IO_BASE + 0x94)   /* A/D1 result 2 */
#define  ATD1DR2H  PORTIO_8(IO_BASE + 0x94)    /* A/D1 result 2 */
#define  ATD1DR3   PORTIO_16(IO_BASE + 0x96)   /* A/D1 result 3 */
#define  ATD1DR3H  PORTIO_8(IO_BASE + 0x96)    /* A/D1 result 3 */
#define  ATD1DR4   PORTIO_16(IO_BASE + 0x98)   /* A/D1 result 4 */
#define  ATD1DR4H  PORTIO_8(IO_BASE + 0x98)    /* A/D1 result 4 */
#define  ATD1DR5   PORTIO_16(IO_BASE + 0x9a)   /* A/D1 result 5 */
#define  ATD1DR5H  PORTIO_8(IO_BASE + 0x9a)    /* A/D1 result 5 */
#define  ATD1DR6   PORTIO_16(IO_BASE + 0x9c)   /* A/D1 result 6 */
#define  ATD1DR6H  PORTIO_8(IO_BASE + 0x9c)    /* A/D1 result 6 */
#define  ATD1DR7   PORTIO_16(IO_BASE + 0x9e)   /* A/D1 result 7 */
#define  ATD1DR7H  PORTIO_8(IO_BASE + 0x9e)    /* A/D1 result 7 */

// ATDnCTL5 bitflags
#define DJM  0x80
#define DSGN 0x40
#define SCAN 0x20
#define MULT 0x10

// ATDnSTAT0 bitflags
#define SCF   0x80
#define ETORF 0x20
#define FIFOR 0x10

// PWM Module  
#define  PWME      PORTIO_8(IO_BASE + 0x300)   /* PWM Enable */
#define  PWMPOL    PORTIO_8(IO_BASE + 0x301)   /* PWM Clock Polarity */
#define  PWMCLK    PORTIO_8(IO_BASE + 0x302)   /* PWM Clocks */
#define  PWMPRCLK  PORTIO_8(IO_BASE + 0x303)   /* PWM prescale clock select */
#define  PWMCAE    PORTIO_8(IO_BASE + 0x304)   /* PWM center align enable */
#define  PWMCTL    PORTIO_8(IO_BASE + 0x305)   /* PWM Control Register */
#define  PWMTST    PORTIO_8(IO_BASE + 0x306)   /* PWM Test Register */
#define  PWMPRSC   PORTIO_8(IO_BASE + 0x307)   /* PWM Test Register */
#define  PWMSCLA   PORTIO_8(IO_BASE + 0x308)   /* PWM scale A */
#define  PWMSCLB   PORTIO_8(IO_BASE + 0x309)   /* PWM scale B */
#define  PWMSCNTA  PORTIO_8(IO_BASE + 0x30a)   /* PWM Test Register A */
#define  PWMSCNTB  PORTIO_8(IO_BASE + 0x30b)   /* PWM Test Register B */
#define  PWMCNT0   PORTIO_8(IO_BASE + 0x30c)   /* PWM Channel Counter 0 */
#define  PWMCNT1   PORTIO_8(IO_BASE + 0x30d)   /* PWM Channel Counter 1 */
#define  PWMCNT2   PORTIO_8(IO_BASE + 0x30e)   /* PWM Channel Counter 2 */
#define  PWMCNT3   PORTIO_8(IO_BASE + 0x30f)   /* PWM Channel Counter 3 */
#define  PWMCNT4   PORTIO_8(IO_BASE + 0x310)   /* PWM Channel Counter 4 */
#define  PWMCNT5   PORTIO_8(IO_BASE + 0x311)   /* PWM Channel Counter 5 */
#define  PWMCNT6   PORTIO_8(IO_BASE + 0x312)   /* PWM Channel Counter 6 */
#define  PWMCNT7   PORTIO_8(IO_BASE + 0x313)   /* PWM Channel Counter 7 */
#define  PWMPER0   PORTIO_8(IO_BASE + 0x314)   /* PWM Channel Period 0 */
#define  PWMPER1   PORTIO_8(IO_BASE + 0x315)   /* PWM Channel Period 1 */
#define  PWMPER2   PORTIO_8(IO_BASE + 0x316)   /* PWM Channel Period 2 */
#define  PWMPER3   PORTIO_8(IO_BASE + 0x317)   /* PWM Channel Period 3 */
#define  PWMPER4   PORTIO_8(IO_BASE + 0x318)   /* PWM Channel Period 4 */
#define  PWMPER5   PORTIO_8(IO_BASE + 0x319)   /* PWM Channel Period 5 */
#define  PWMPER6   PORTIO_8(IO_BASE + 0x31a)   /* PWM Channel Period 6 */
#define  PWMPER7   PORTIO_8(IO_BASE + 0x31b)   /* PWM Channel Period 7 */
#define  PWMDTY0   PORTIO_8(IO_BASE + 0x31c)   /* PWM Channel Duty 0 */
#define  PWMDTY1   PORTIO_8(IO_BASE + 0x31d)   /* PWM Channel Duty 1 */
#define  PWMDTY2   PORTIO_8(IO_BASE + 0x31e)   /* PWM Channel Duty 2 */
#define  PWMDTY3   PORTIO_8(IO_BASE + 0x31f)   /* PWM Channel Duty 3 */
#define  PWMDTY4   PORTIO_8(IO_BASE + 0x320)   /* PWM Channel Duty 4 */
#define  PWMDTY5   PORTIO_8(IO_BASE + 0x321)   /* PWM Channel Duty 5 */
#define  PWMDTY6   PORTIO_8(IO_BASE + 0x322)   /* PWM Channel Duty 6 */
#define  PWMDTY7   PORTIO_8(IO_BASE + 0x323)   /* PWM Channel Duty 7 */
#define  PWMSDN    PORTIO_8(IO_BASE + 0x324)   /* PWM shutdown register */

#define     PWMCNT01_16BIT      PORTIO_16(IO_BASE + 0x30c)     /* pwm channel 0,1 counter, 16bit */
#define     PWMCNT23_16BIT      PORTIO_16(IO_BASE + 0x30e)     /* pwm channel 2,3 counter, 16bit */
#define     PWMCNT45_16BIT      PORTIO_16(IO_BASE + 0x310)     /* pwm channel 4,5 counter, 16bit */
#define     PWMCNT67_16BIT      PORTIO_16(IO_BASE + 0x312)     /* pwm channel 6,7 counter, 16bit */
#define     PWMPER01_16BIT      PORTIO_16(IO_BASE + 0x314)     /* pwm channel 0,1 period, 16bit */
#define     PWMPER23_16BIT      PORTIO_16(IO_BASE + 0x316)     /* pwm channel 2,3 period, 16bit */
#define     PWMPER45_16BIT      PORTIO_16(IO_BASE + 0x318)     /* pwm channel 4,5 period, 16bit */
#define     PWMPER67_16BIT      PORTIO_16(IO_BASE + 0x31a)     /* pwm channel 6,7 period, 16bit */
#define     PWMDTY01_16BIT      PORTIO_16(IO_BASE + 0x31c)     /* pwm channel 0,1 duty cycle, 16bit */
#define     PWMDTY23_16BIT      PORTIO_16(IO_BASE + 0x31e)     /* pwm channel 2,3 duty cycle, 16bit */
#define     PWMDTY45_16BIT      PORTIO_16(IO_BASE + 0x320)     /* pwm channel 4,5 duty cycle, 16bit */
#define     PWMDTY67_16BIT      PORTIO_16(IO_BASE + 0x322)     /* pwm channel 6,7 duty cycle, 16bit */



// SCI register offsets
#define  _SCIBD    0x0   /* SCI baud rate high */
#define  _SCIBDH   0x0   /* SCI baud rate high */
#define  _SCIBDL   0x1   /* SCI baud rate low */
#define  _SCICR1   0x2   /* SCI control register 1 */
#define  _SCICR2   0x3   /* SCI control register 2 */
#define  _SCISR1   0x4   /* SCI status register 1 */
#define  _SCISR2   0x5   /* SCI status register 2 */
#define  _SCIDRH   0x6   /* SCI data register high */
#define  _SCIDRL   0x7   /* SCI data register low */

// SCI0 Module
#define  SCI0_BASE (IO_BASE + 0xc8)
#define  SCI0BD    PORTIO_16(IO_BASE + 0xc8)   /* SCI 0 baud rate high */
#define  SCI0BDH   PORTIO_8(IO_BASE + 0xc8)   /* SCI 0 baud rate high */
#define  SCI0BDL   PORTIO_8(IO_BASE + 0xc9)   /* SCI 0 baud rate low */
#define  SCI0CR1   PORTIO_8(IO_BASE + 0xca)   /* SCI 0 control register 1 */
#define  SCI0CR2   PORTIO_8(IO_BASE + 0xcb)   /* SCI 0 control register 2 */
#define  SCI0SR1   PORTIO_8(IO_BASE + 0xcc)   /* SCI 0 status register 1 */
#define  SCI0SR2   PORTIO_8(IO_BASE + 0xcd)   /* SCI 0 status register 2 */
#define  SCI0DRH   PORTIO_8(IO_BASE + 0xce)   /* SCI 0 data register high */
#define  SCI0DRL   PORTIO_8(IO_BASE + 0xcf)   /* SCI 0 data register low */

// SCI1 Module  
#define  SCI1_BASE (IO_BASE + 0xd0)
#define  SCI1BD    PORTIO_16(IO_BASE + 0xd0)   /* SCI 1 16bit baud rate */
#define  SCI1BDH   PORTIO_8(IO_BASE + 0xd0)   /* SCI 1 baud rate high */
#define  SCI1BDL   PORTIO_8(IO_BASE + 0xd1)   /* SCI 1 baud rate low */
#define  SCI1CR1   PORTIO_8(IO_BASE + 0xd2)   /* SCI 1 control register 1 */
#define  SCI1CR2   PORTIO_8(IO_BASE + 0xd3)   /* SCI 1 control register 2 */
#define  SCI1SR1   PORTIO_8(IO_BASE + 0xd4)   /* SCI 1 status register 1 */
#define  SCI1SR2   PORTIO_8(IO_BASE + 0xd5)   /* SCI 1 status register 2 */
#define  SCI1DRH   PORTIO_8(IO_BASE + 0xd6)   /* SCI 1 data register high */
#define  SCI1DRL   PORTIO_8(IO_BASE + 0xd7)   /* SCI 1 data register low */

// SCInSR1
#define TDRE 0x80
#define RDRF 0x20
#define IDLE 0x10

//  SPI register offsets
#define  _SPICR1   PORTIO_8(IO_BASE + 0x0)   /* SPI control register 1 */
#define  _SPICR2   PORTIO_8(IO_BASE + 0x1)   /* SPI control register 2 */
#define  _SPIBR    PORTIO_8(IO_BASE + 0x2)   /* SPI baud rate register */
#define  _SPISR    PORTIO_8(IO_BASE + 0x3)   /* SPI status register */
#define  _SPIDR    PORTIO_8(IO_BASE + 0x5)   /* SPI data register */

// SPI0 Module  
#define  SPI0_BASE (IO_BASE + 0xd8)
#define  SPI0CR1   PORTIO_8(IO_BASE + 0xd8)   /* SPI 0 control register 1 */
#define  SPI0CR2   PORTIO_8(IO_BASE + 0xd9)   /* SPI 0 control register 2 */
#define  SPI0BR    PORTIO_8(IO_BASE + 0xda)   /* SPI 0 baud rate register */
#define  SPI0SR    PORTIO_8(IO_BASE + 0xdb)   /* SPI 0 status register */
#define  SPI0DR    PORTIO_8(IO_BASE + 0xdd)   /* SPI 0 data register */

// SPInCR1
#define SPIE  0x80
#define SPE   0x40
#define SPTIE 0x20
#define MSTR  0x10
#define CPOL  0x08
#define CPHA  0x04
#define SSOE  0x02
#define LSBFE 0x01

// SPInSR
#define SPIF  0x80
#define SPTEF 0x20
#define MODF  0x10

// I2C Module  
#define  IBAD      PORTIO_8(IO_BASE + 0xe0)   /* I2C address register */
#define  IBFD      PORTIO_8(IO_BASE + 0xe1)   /* I2C freqency divider reg */
#define  IBCR      PORTIO_8(IO_BASE + 0xe2)   /* I2C control register */
#define  IBSR      PORTIO_8(IO_BASE + 0xe3)   /* I2C status register */
#define  IBDR      PORTIO_8(IO_BASE + 0xe4)   /* I2C data register */

// IBSR
#define TCF  0x80
#define IAAS 0x40
#define IBB  0x20
#define IBAL 0x10
#define SRW  0x04
#define IBIF 0x02
#define RXAK 0x01

// SPI1 Module
#define  SPI1_BASE (IO_BASE + 0xf0)
#define  SPI1CR1   PORTIO_8(IO_BASE + 0xf0)   /* SPI 1 control register 1 */
#define  SPI1CR2   PORTIO_8(IO_BASE + 0xf1)   /* SPI 1 control register 2 */
#define  SPI1BR    PORTIO_8(IO_BASE + 0xf2)   /* SPI 1 baud rate register */
#define  SPI1SR    PORTIO_8(IO_BASE + 0xf3)   /* SPI 1 status register */
#define  SPI1DR    PORTIO_8(IO_BASE + 0xf5)   /* SPI 1 data register */

// SPI2 Module
#define  SPI2_BASE (IO_BASE + 0xf8)
#define  SPI2CR1   PORTIO_8(IO_BASE + 0xf8)   /* SPI 2 control register 1 */
#define  SPI2CR2   PORTIO_8(IO_BASE + 0xf9)   /* SPI 2 control register 2 */
#define  SPI2BR    PORTIO_8(IO_BASE + 0xfa)   /* SPI 2 baud rate register */
#define  SPI2SR    PORTIO_8(IO_BASE + 0xfb)   /* SPI 2 status register */
#define  SPI2DR    PORTIO_8(IO_BASE + 0xfd)   /* SPI 2 data register */

// FLC Module
#define  FCLKDIV      PORTIO_8(IO_BASE + 0x100)	/* flash clock divider */
#define  FSEC         PORTIO_8(IO_BASE + 0x101)	/* flash security register */
#define  FCNFG        PORTIO_8(IO_BASE + 0x103)	/* flash configuration register */
#define  FPROT        PORTIO_8(IO_BASE + 0x104)	/* flash protection register */
#define  FSTAT        PORTIO_8(IO_BASE + 0x105)	/* flash status register */
#define  FCMD         PORTIO_8(IO_BASE + 0x106)	/* flash command register */

//  EEPROM Module
#define  ECLKDIV      PORTIO_8(IO_BASE + 0x110)	/* eeprom clock divider */
#define  ECNFG        PORTIO_8(IO_BASE + 0x113)	/* eeprom configuration register */
#define  EPROT        PORTIO_8(IO_BASE + 0x114)	/* eeprom protection register */
#define  ESTAT        PORTIO_8(IO_BASE + 0x115)	/* eeprom status register */
#define  ECMD         PORTIO_8(IO_BASE + 0x116)	/* eeprom command register */

// ATD0 Module
#define ATD0_BASE    (IO_BASE + 0x2C0)
#define ATD0CTL0     PORTIO_8(IO_BASE + 0x2C0)	/* A/D0 control register 0 */
#define  ATD0CTL1     PORTIO_8(IO_BASE + 0x2C1)	/* A/D0 control register 1 */
#define  ATD0CTL2     PORTIO_8(IO_BASE + 0x2C2)	/* A/D0 control register 2 */
#define  ATD0CTL3     PORTIO_8(IO_BASE + 0x2C3)	/* A/D0 control register 3 */
#define  ATD0CTL4     PORTIO_8(IO_BASE + 0x2C4)	/* A/D0 control register 4 */
#define  ATD0CTL5     PORTIO_8(IO_BASE + 0x2C5)	/* A/D0 control register 5 */
#define  ATD0STAT0    PORTIO_8(IO_BASE + 0x2C6)	/* A/D0 status register 0 */
#define  ATD0STAT1    PORTIO_8(IO_BASE + 0x2C7)	/* A/D0 status register 1 */
#define  ATD0TEST0    PORTIO_8(IO_BASE + 0x2C8)	/* A/D0 test register 0 */
#define  ATD0TEST1    PORTIO_8(IO_BASE + 0x2C9)	/* A/D0 test register 1 */
#define  ATD0DIEN     PORTIO_8(IO_BASE + 0x2Cd)	/* A/D0 interrupt enable */
#define  PORTAD0      PORTIO_8(IO_BASE + 0x2Cf)	/* port AD0 data input register */
#define  ATD0DR0      PORTIO_16(IO_BASE + 0x2d0)	/* A/D0 result 0 */
#define  ATD0DR0H     PORTIO_8(IO_BASE + 0x2d0)		/* A/D0 result 0 */
#define  ATD0DR1      PORTIO_16(IO_BASE + 0x2d2)	/* A/D0 result 1 */
#define  ATD0DR1H     PORTIO_8(IO_BASE + 0x2d2)		/* A/D0 result 1 */
#define  ATD0DR2      PORTIO_16(IO_BASE + 0x2d4)	/* A/D0 result 2 */
#define  ATD0DR2H     PORTIO_8(IO_BASE + 0x2d4)		/* A/D0 result 2 */
#define  ATD0DR3      PORTIO_16(IO_BASE + 0x2d6)	/* A/D0 result 3 */
#define  ATD0DR3H     PORTIO_8(IO_BASE + 0x2d6)		/* A/D0 result 3 */
#define  ATD0DR4      PORTIO_16(IO_BASE + 0x2d8)	/* A/D0 result 4 */
#define  ATD0DR4H     PORTIO_8(IO_BASE + 0x2d8)		/* A/D0 result 4 */
#define  ATD0DR5      PORTIO_16(IO_BASE + 0x2da)	/* A/D0 result 5 */
#define  ATD0DR5H     PORTIO_8(IO_BASE + 0x2da)		/* A/D0 result 5 */
#define  ATD0DR6      PORTIO_16(IO_BASE + 0x2dc)	/* A/D0 result 6 */
#define  ATD0DR6H     PORTIO_8(IO_BASE + 0x2dc)		/* A/D0 result 6 */
#define  ATD0DR7      PORTIO_16(IO_BASE + 0x2de)	/* A/D0 result 7 */
#define  ATD0DR7H     PORTIO_8(IO_BASE + 0x2de)		/* A/D0 result 7 */

// CAN0 Module  
#define  CAN0_BASE    (IO_BASE + 0x140)
#define  CAN0CTL0     PORTIO_8(IO_BASE + 0x140)	/* CAN0 control register 0 */
#define  CAN0CTL1     PORTIO_8(IO_BASE + 0x141)	/* CAN0 control register 1 */
#define  CAN0BTR0     PORTIO_8(IO_BASE + 0x142)	/* CAN0 bus timing register 0 */
#define  CAN0BTR1     PORTIO_8(IO_BASE + 0x143)	/* CAN0 bus timing register 1 */
#define  CAN0RFLG     PORTIO_8(IO_BASE + 0x144)	/* CAN0 receiver flag register */
#define  CAN0RIER     PORTIO_8(IO_BASE + 0x145)	/* CAN0 receiver interrupt reg */
#define  CAN0TFLG     PORTIO_8(IO_BASE + 0x146)	/* CAN0 transmitter flag reg */
#define  CAN0TIER     PORTIO_8(IO_BASE + 0x147)	/* CAN0 transmitter control reg */
#define  CAN0TARQ     PORTIO_8(IO_BASE + 0x148)	/* CAN0 transmitter abort request */
#define  CAN0TAAK     PORTIO_8(IO_BASE + 0x149)	/* CAN0 transmitter abort acknowledge */
#define  CAN0TBSEL    PORTIO_8(IO_BASE + 0x14a)	/* CAN0 transmit buffer selection */
#define  CAN0IDAC     PORTIO_8(IO_BASE + 0x14b)	/* CAN0 identifier acceptance */
#define  CAN0RXERR    PORTIO_8(IO_BASE + 0x14e)	/* CAN0 receive error counter */
#define  CAN0TXERR    PORTIO_8(IO_BASE + 0x14f)	/* CAN0 transmit error counter */
#define  CAN0IDAR0    PORTIO_8(IO_BASE + 0x150)	/* CAN0 id acceptance reg 0 */
#define  CAN0IDAR1    PORTIO_8(IO_BASE + 0x151)	/* CAN0 id acceptance reg 1 */
#define  CAN0IDAR2    PORTIO_8(IO_BASE + 0x152)	/* CAN0 id acceptance reg 2 */
#define  CAN0IDAR3    PORTIO_8(IO_BASE + 0x153)	/* CAN0 id acceptance reg 3 */
#define  CAN0IDMR0    PORTIO_8(IO_BASE + 0x154)	/* CAN0 id mask register 0 */
#define  CAN0IDMR1    PORTIO_8(IO_BASE + 0x155)	/* CAN0 id mask register 1 */
#define  CAN0IDMR2    PORTIO_8(IO_BASE + 0x156)	/* CAN0 id mask register 2 */
#define  CAN0IDMR3    PORTIO_8(IO_BASE + 0x157)	/* CAN0 id mask register 3 */
#define  CAN0IDAR4    PORTIO_8(IO_BASE + 0x158)	/* CAN0 id acceptance reg 4 */
#define  CAN0IDAR5    PORTIO_8(IO_BASE + 0x159)	/* CAN0 id acceptance reg 5 */
#define  CAN0IDAR6    PORTIO_8(IO_BASE + 0x15a)	/* CAN0 id acceptance reg 6 */
#define  CAN0IDAR7    PORTIO_8(IO_BASE + 0x15b)	/* CAN0 id acceptance reg 7 */
#define  CAN0IDMR4    PORTIO_8(IO_BASE + 0x15c)	/* CAN0 id mask register 4 */
#define  CAN0IDMR5    PORTIO_8(IO_BASE + 0x15d)	/* CAN0 id mask register 5 */
#define  CAN0IDMR6    PORTIO_8(IO_BASE + 0x15e)	/* CAN0 id mask register 6 */
#define  CAN0IDMR7    PORTIO_8(IO_BASE + 0x15f)	/* CAN0 id mask register 7 */
#define  CAN0RXFG     PORTIO_8(IO_BASE + 0x160)	/* CAN0 receive buffer */
#define  CAN0TXFG     PORTIO_8(IO_BASE + 0x170)	/* CAN0 transmit buffer */

//  CAN1 Module  
#define  CAN1_BASE    (IO_BASE + 0x180)
#define  CAN1CTL0     PORTIO_8(IO_BASE + 0x180)	/* CAN1 control register 0 */
#define  CAN1CTL1     PORTIO_8(IO_BASE + 0x181)	/* CAN1 control register 1 */
#define  CAN1BTR0     PORTIO_8(IO_BASE + 0x182)	/* CAN1 bus timing register 0 */
#define  CAN1BTR1     PORTIO_8(IO_BASE + 0x183)	/* CAN1 bus timing register 1 */
#define  CAN1RFLG     PORTIO_8(IO_BASE + 0x184)	/* CAN1 receiver flag register */
#define  CAN1RIER     PORTIO_8(IO_BASE + 0x185)	/* CAN1 receiver interrupt reg */
#define  CAN1TFLG     PORTIO_8(IO_BASE + 0x186)	/* CAN1 transmitter flag reg */
#define  CAN1TIER     PORTIO_8(IO_BASE + 0x187)	/* CAN1 transmitter control reg */
#define  CAN1TARQ     PORTIO_8(IO_BASE + 0x188)	/* CAN1 transmitter abort request */
#define  CAN1TAAK     PORTIO_8(IO_BASE + 0x189)	/* CAN1 transmitter abort acknowledge */
#define  CAN1TBSEL    PORTIO_8(IO_BASE + 0x18a)	/* CAN1 transmit buffer selection */
#define  CAN1IDAC     PORTIO_8(IO_BASE + 0x18b)	/* CAN1 identifier acceptance */
#define  CAN1RXERR    PORTIO_8(IO_BASE + 0x18e)	/* CAN1 transmitter control reg */
#define  CAN1TXERR    PORTIO_8(IO_BASE + 0x18f)	/* CAN1 transmit error counter */
#define  CAN1IDAR0    PORTIO_8(IO_BASE + 0x190)	/* CAN1 id acceptance reg 0 */
#define  CAN1IDAR1    PORTIO_8(IO_BASE + 0x191)	/* CAN1 id acceptance reg 1 */
#define  CAN1IDAR2    PORTIO_8(IO_BASE + 0x192)	/* CAN1 id acceptance reg 2 */
#define  CAN1IDAR3    PORTIO_8(IO_BASE + 0x193)	/* CAN1 id acceptance reg 3 */
#define  CAN1IDMR0    PORTIO_8(IO_BASE + 0x194)	/* CAN1 id mask register 0 */
#define  CAN1IDMR1    PORTIO_8(IO_BASE + 0x195)	/* CAN1 id mask register 1 */
#define  CAN1IDMR2    PORTIO_8(IO_BASE + 0x196)	/* CAN1 id mask register 2 */
#define  CAN1IDMR3    PORTIO_8(IO_BASE + 0x197)	/* CAN1 id mask register 3 */
#define  CAN1IDAR4    PORTIO_8(IO_BASE + 0x198)	/* CAN1 id acceptance reg 4 */
#define  CAN1IDAR5    PORTIO_8(IO_BASE + 0x199)	/* CAN1 id acceptance reg 5 */
#define  CAN1IDAR6    PORTIO_8(IO_BASE + 0x19a)	/* CAN1 id acceptance reg 6 */
#define  CAN1IDAR7    PORTIO_8(IO_BASE + 0x19b)	/* CAN1 id acceptance reg 7 */
#define  CAN1IDMR4    PORTIO_8(IO_BASE + 0x19c)	/* CAN1 id mask register 4 */
#define  CAN1IDMR5    PORTIO_8(IO_BASE + 0x19d)	/* CAN1 id mask register 5 */
#define  CAN1IDMR6    PORTIO_8(IO_BASE + 0x19e)	/* CAN1 id mask register 6 */
#define  CAN1IDMR7    PORTIO_8(IO_BASE + 0x19f)	/* CAN1 id mask register 7 */
#define  CAN1RXFG     PORTIO_8(IO_BASE + 0x1a0)	/* CAN1 receive buffer */
#define  CAN1TXFG     PORTIO_8(IO_BASE + 0x1b0)	/* CAN1 transmit buffer */

//  CAN2 Module  
#define  CAN2_BASE    (IO_BASE + 0x1c0)
#define  CAN2CTL0     PORTIO_8(IO_BASE + 0x1c0)	/* CAN2 control register 0 */
#define  CAN2CTL1     PORTIO_8(IO_BASE + 0x1c1)	/* CAN2 control register 1 */
#define  CAN2BTR0     PORTIO_8(IO_BASE + 0x1c2)	/* CAN2 bus timing register 0 */
#define  CAN2BTR1     PORTIO_8(IO_BASE + 0x1c3)	/* CAN2 bus timing register 1 */
#define  CAN2RFLG     PORTIO_8(IO_BASE + 0x1c4)	/* CAN2 receiver flag register */
#define  CAN2RIER     PORTIO_8(IO_BASE + 0x1c5)	/* CAN2 receiver interrupt reg */
#define  CAN2TFLG     PORTIO_8(IO_BASE + 0x1c6)	/* CAN2 transmitter flag reg */
#define  CAN2TIER     PORTIO_8(IO_BASE + 0x1c7)	/* CAN2 transmitter control reg */
#define  CAN2TARQ     PORTIO_8(IO_BASE + 0x1c8)	/* CAN2 transmitter abort request */
#define  CAN2TAAK     PORTIO_8(IO_BASE + 0x1c9)	/* CAN2 transmitter abort acknowledge */
#define  CAN2TBSEL    PORTIO_8(IO_BASE + 0x1ca)	/* CAN2 transmit buffer selection */
#define  CAN2IDAC     PORTIO_8(IO_BASE + 0x1cb)	/* CAN2 identifier acceptance */
#define  CAN2RXERR    PORTIO_8(IO_BASE + 0x1ce)	/* CAN2 transmitter control reg */
#define  CAN2TXERR    PORTIO_8(IO_BASE + 0x1cf)	/* CAN2 transmit error counter */
#define  CAN2IDAR0    PORTIO_8(IO_BASE + 0x1d0)	/* CAN2 id acceptance reg 0 */
#define  CAN2IDAR1    PORTIO_8(IO_BASE + 0x1d1)	/* CAN2 id acceptance reg 1 */
#define  CAN2IDAR2    PORTIO_8(IO_BASE + 0x1d2)	/* CAN2 id acceptance reg 2 */
#define  CAN2IDAR3    PORTIO_8(IO_BASE + 0x1d3)	/* CAN2 id acceptance reg 3 */
#define  CAN2IDMR0    PORTIO_8(IO_BASE + 0x1d4)	/* CAN2 id mask register 0 */
#define  CAN2IDMR1    PORTIO_8(IO_BASE + 0x1d5)	/* CAN2 id mask register 1 */
#define  CAN2IDMR2    PORTIO_8(IO_BASE + 0x1d6)	/* CAN2 id mask register 2 */
#define  CAN2IDMR3    PORTIO_8(IO_BASE + 0x1d7)	/* CAN2 id mask register 3 */
#define  CAN2IDAR4    PORTIO_8(IO_BASE + 0x1d8)	/* CAN2 id acceptance reg 4 */
#define  CAN2IDAR5    PORTIO_8(IO_BASE + 0x1d9)	/* CAN2 id acceptance reg 5 */
#define  CAN2IDAR6    PORTIO_8(IO_BASE + 0x1da)	/* CAN2 id acceptance reg 6 */
#define  CAN2IDAR7    PORTIO_8(IO_BASE + 0x1db)	/* CAN2 id acceptance reg 7 */
#define  CAN2IDMR4    PORTIO_8(IO_BASE + 0x1dc)	/* CAN2 id mask register 4 */
#define  CAN2IDMR5    PORTIO_8(IO_BASE + 0x1dd)	/* CAN2 id mask register 5 */
#define  CAN2IDMR6    PORTIO_8(IO_BASE + 0x1de)	/* CAN2 id mask register 6 */
#define  CAN2IDMR7    PORTIO_8(IO_BASE + 0x1df)	/* CAN2 id mask register 7 */
#define  CAN2RXFG     PORTIO_8(IO_BASE + 0x1e0)	/* CAN2 receive buffer */
#define  CAN2TXFG     PORTIO_8(IO_BASE + 0x1f0)	/* CAN2 transmit buffer */

// CAN3 Module  
#define  CAN3_BASE    (IO_BASE + 0x200)
#define  CAN3CTL0     PORTIO_8(IO_BASE + 0x200)	/* CAN3 control register 0 */
#define  CAN3CTL1     PORTIO_8(IO_BASE + 0x201)	/* CAN3 control register 1 */
#define  CAN3BTR0     PORTIO_8(IO_BASE + 0x202)	/* CAN3 bus timing register 0 */
#define  CAN3BTR1     PORTIO_8(IO_BASE + 0x203)	/* CAN3 bus timing register 1 */
#define  CAN3RFLG     PORTIO_8(IO_BASE + 0x204)	/* CAN3 receiver flag register */
#define  CAN3RIER     PORTIO_8(IO_BASE + 0x205)	/* CAN3 receiver interrupt reg */
#define  CAN3TFLG     PORTIO_8(IO_BASE + 0x206)	/* CAN3 transmitter flag reg */
#define  CAN3TIER     PORTIO_8(IO_BASE + 0x207)	/* CAN3 transmitter control reg */
#define  CAN3TARQ     PORTIO_8(IO_BASE + 0x208)	/* CAN3 transmitter abort request */
#define  CAN3TAAK     PORTIO_8(IO_BASE + 0x209)	/* CAN3 transmitter abort acknowledge */
#define  CAN3TBSEL    PORTIO_8(IO_BASE + 0x20a)	/* CAN3 transmit buffer selection */
#define  CAN3IDAC     PORTIO_8(IO_BASE + 0x20b)	/* CAN3 identifier acceptance */
#define  CAN3RXERR    PORTIO_8(IO_BASE + 0x20e)	/* CAN3 transmitter control reg */
#define  CAN3TXERR    PORTIO_8(IO_BASE + 0x20f)	/* CAN3 transmit error counter */
#define  CAN3IDAR0    PORTIO_8(IO_BASE + 0x210)	/* CAN3 id acceptance reg 0 */
#define  CAN3IDAR1    PORTIO_8(IO_BASE + 0x211)	/* CAN3 id acceptance reg 1 */
#define  CAN3IDAR2    PORTIO_8(IO_BASE + 0x212)	/* CAN3 id acceptance reg 2 */
#define  CAN3IDAR3    PORTIO_8(IO_BASE + 0x213)	/* CAN3 id acceptance reg 3 */
#define  CAN3IDMR0    PORTIO_8(IO_BASE + 0x214)	/* CAN3 id mask register 0 */
#define  CAN3IDMR1    PORTIO_8(IO_BASE + 0x215)	/* CAN3 id mask register 1 */
#define  CAN3IDMR2    PORTIO_8(IO_BASE + 0x216)	/* CAN3 id mask register 2 */
#define  CAN3IDMR3    PORTIO_8(IO_BASE + 0x217)	/* CAN3 id mask register 3 */
#define  CAN3IDAR4    PORTIO_8(IO_BASE + 0x218)	/* CAN3 id acceptance reg 4 */
#define  CAN3IDAR5    PORTIO_8(IO_BASE + 0x219)	/* CAN3 id acceptance reg 5 */
#define  CAN3IDAR6    PORTIO_8(IO_BASE + 0x21a)	/* CAN3 id acceptance reg 6 */
#define  CAN3IDAR7    PORTIO_8(IO_BASE + 0x21b)	/* CAN3 id acceptance reg 7 */
#define  CAN3IDMR4    PORTIO_8(IO_BASE + 0x21c)	/* CAN3 id mask register 4 */
#define  CAN3IDMR5    PORTIO_8(IO_BASE + 0x21d)	/* CAN3 id mask register 5 */
#define  CAN3IDMR6    PORTIO_8(IO_BASE + 0x21e)	/* CAN3 id mask register 6 */
#define  CAN3IDMR7    PORTIO_8(IO_BASE + 0x21f)	/* CAN3 id mask register 7 */
#define  CAN3RXFG     PORTIO_8(IO_BASE + 0x220)	/* CAN3 receive buffer */
#define  CAN3TXFG     PORTIO_8(IO_BASE + 0x230)	/* CAN3 transmit buffer */


// Port T register offsets
#define  PTT          PORTIO_8(IO_BASE + 0x240)	/* port T data register */
#define  PTIT         PORTIO_8(IO_BASE + 0x241)	/* port T input register */
#define  DDRT         PORTIO_8(IO_BASE + 0x242)	/* port T data direction */
#define  RDRT         PORTIO_8(IO_BASE + 0x243)	/* port T reduce drive */
#define  PERT         PORTIO_8(IO_BASE + 0x244)	/* port T pull enable */
#define  PPST         PORTIO_8(IO_BASE + 0x245)	/* port T polarity select */

// Port S
#define  PTS          PORTIO_8(IO_BASE + 0x248)	/* port S data register */
#define  PTIS         PORTIO_8(IO_BASE + 0x249)	/* port S input register */
#define  DDRS         PORTIO_8(IO_BASE + 0x24a)	/* port S data direction */
#define  RDRS         PORTIO_8(IO_BASE + 0x24b)	/* port S reduce drive */
#define  PERS         PORTIO_8(IO_BASE + 0x24c)	/* port S pull enable */
#define  PPSS         PORTIO_8(IO_BASE + 0x24d)	/* port S polarity select */
#define  WOMS         PORTIO_8(IO_BASE + 0x24e)	/* port S wired-or mode */

// Port M
#define  PTM          PORTIO_8(IO_BASE + 0x250)	/* port M data register */
#define  PTIM         PORTIO_8(IO_BASE + 0x251)	/* port M input register */
#define  DDRM         PORTIO_8(IO_BASE + 0x252)	/* port M data direction */
#define  RDRM         PORTIO_8(IO_BASE + 0x253)	/* port M reduce drive */
#define  PERM         PORTIO_8(IO_BASE + 0x254)	/* port M pull enable */
#define  PPSM         PORTIO_8(IO_BASE + 0x255)	/* port M polarity select */
#define  WOMM         PORTIO_8(IO_BASE + 0x256)	/* port M wired-or mode */

#define  MODDR        PORTIO_8(IO_BASE + 0x257) /* Module routing register */

// Port P
#define  PTP          PORTIO_8(IO_BASE + 0x258)	/* port P data register */
#define  PTIP         PORTIO_8(IO_BASE + 0x259)	/* port P input register */
#define  DDRP         PORTIO_8(IO_BASE + 0x25a)	/* port P data direction */
#define  RDRP         PORTIO_8(IO_BASE + 0x25b)	/* port P reduce drive */
#define  PERP         PORTIO_8(IO_BASE + 0x25c)	/* port P pull enable */
#define  PPSP         PORTIO_8(IO_BASE + 0x25d)	/* port P polarity select */
#define  PIEP         PORTIO_8(IO_BASE + 0x25e)	/* port P interrupt enable */
#define  PIFP         PORTIO_8(IO_BASE + 0x25f)	/* port P interrupt flag */

// Port H
#define  PTH          PORTIO_8(IO_BASE + 0x260)	/* port H data register */
#define  PTIH         PORTIO_8(IO_BASE + 0x261)	/* port H input register */
#define  DDRH         PORTIO_8(IO_BASE + 0x262)	/* port H data direction */
#define  RDRH         PORTIO_8(IO_BASE + 0x263)	/* port H reduce drive */
#define  PERH         PORTIO_8(IO_BASE + 0x264)	/* port H pull enable */
#define  PPSH         PORTIO_8(IO_BASE + 0x265)	/* port H polarity select */
#define  PIEH         PORTIO_8(IO_BASE + 0x266)	/* port H interrupt enable */
#define  PIFH         PORTIO_8(IO_BASE + 0x267)	/* port H interrupt flag */

// 	Port J
#define  PTJ          PORTIO_8(IO_BASE + 0x268)	/* port J data register */
#define  PTIJ         PORTIO_8(IO_BASE + 0x269)	/* port J input register */
#define  DDRJ         PORTIO_8(IO_BASE + 0x26a)	/* port J data direction */
#define  RDRJ         PORTIO_8(IO_BASE + 0x26b)	/* port J reduce drive */
#define  PERJ         PORTIO_8(IO_BASE + 0x26c)	/* port J pull enable */
#define  PPSJ         PORTIO_8(IO_BASE + 0x26d)	/* port J polarity select */
#define  PIEJ         PORTIO_8(IO_BASE + 0x26e)	/* port J interrupt enable */
#define  PIFJ         PORTIO_8(IO_BASE + 0x26f)	/* port J interrupt flag */



// CAN4 Module  
#define  CAN4_BASE    (IO_BASE + 0x280)
#define  CAN4CTL0     PORTIO_8(IO_BASE + 0x280)	/* CAN4 control register 0 */
#define  CAN4CTL1     PORTIO_8(IO_BASE + 0x281)	/* CAN4 control register 1 */
#define  CAN4BTR0     PORTIO_8(IO_BASE + 0x282)	/* CAN4 bus timing register 0 */
#define  CAN4BTR1     PORTIO_8(IO_BASE + 0x283)	/* CAN4 bus timing register 1 */
#define  CAN4RFLG     PORTIO_8(IO_BASE + 0x284)	/* CAN4 receiver flag register */
#define  CAN4RIER     PORTIO_8(IO_BASE + 0x285)	/* CAN4 receiver interrupt reg */
#define  CAN4TFLG     PORTIO_8(IO_BASE + 0x286)	/* CAN4 transmitter flag reg */
#define  CAN4TIER     PORTIO_8(IO_BASE + 0x287)	/* CAN4 transmitter control reg */
#define  CAN4TARQ     PORTIO_8(IO_BASE + 0x288)	/* CAN4 transmitter abort request */
#define  CAN4TAAK     PORTIO_8(IO_BASE + 0x289)	/* CAN4 transmitter abort acknowledge */
#define  CAN4TBSEL    PORTIO_8(IO_BASE + 0x28a)	/* CAN4 transmit buffer selection */
#define  CAN4IDAC     PORTIO_8(IO_BASE + 0x28b)	/* CAN4 identifier acceptance */
#define  CAN4RXERR    PORTIO_8(IO_BASE + 0x28e)	/* CAN4 transmitter control reg */
#define  CAN4TXERR    PORTIO_8(IO_BASE + 0x28f)	/* CAN4 transmit error counter */
#define  CAN4IDAR0    PORTIO_8(IO_BASE + 0x290)	/* CAN4 id acceptance reg 0 */
#define  CAN4IDAR1    PORTIO_8(IO_BASE + 0x291)	/* CAN4 id acceptance reg 1 */
#define  CAN4IDAR2    PORTIO_8(IO_BASE + 0x292)	/* CAN4 id acceptance reg 2 */
#define  CAN4IDAR3    PORTIO_8(IO_BASE + 0x293)	/* CAN4 id acceptance reg 3 */
#define  CAN4IDMR0    PORTIO_8(IO_BASE + 0x294)	/* CAN4 id mask register 0 */
#define  CAN4IDMR1    PORTIO_8(IO_BASE + 0x295)	/* CAN4 id mask register 1 */
#define  CAN4IDMR2    PORTIO_8(IO_BASE + 0x296)	/* CAN4 id mask register 2 */
#define  CAN4IDMR3    PORTIO_8(IO_BASE + 0x297)	/* CAN4 id mask register 3 */
#define  CAN4IDAR4    PORTIO_8(IO_BASE + 0x298)	/* CAN4 id acceptance reg 4 */
#define  CAN4IDAR5    PORTIO_8(IO_BASE + 0x299)	/* CAN4 id acceptance reg 5 */
#define  CAN4IDAR6    PORTIO_8(IO_BASE + 0x29a)	/* CAN4 id acceptance reg 6 */
#define  CAN4IDAR7    PORTIO_8(IO_BASE + 0x29b)	/* CAN4 id acceptance reg 7 */
#define  CAN4IDMR4    PORTIO_8(IO_BASE + 0x29c)	/* CAN4 id mask register 4 */
#define  CAN4IDMR5    PORTIO_8(IO_BASE + 0x29d)	/* CAN4 id mask register 5 */
#define  CAN4IDMR6    PORTIO_8(IO_BASE + 0x29e)	/* CAN4 id mask register 6 */
#define  CAN4IDMR7    PORTIO_8(IO_BASE + 0x29f)	/* CAN4 id mask register 7 */
#define  CAN4RXFG     PORTIO_8(IO_BASE + 0x2a0)	/* CAN4 receive buffer */
#define  CAN4TXFG     PORTIO_8(IO_BASE + 0x2b0)	/* CAN4 transmit buffer */



# define SCIBD		PORTIO_16(SCI_BASE + _SCIBD)
# define SCICR1		PORTIO_8(SCI_BASE + _SCICR1)
# define SCICR2		PORTIO_8(SCI_BASE + _SCICR2)
# define SCISR1		PORTIO_8(SCI_BASE + _SCISR1)
# define SCISR2		PORTIO_8(SCI_BASE + _SCISR2)
# define SCIDRL		PORTIO_8(SCI_BASE + _SCIDRL)

#endif /* ifndef   _REGS_HCS12XD_H_ */
