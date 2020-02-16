#ifndef _BCM2835_H_
#define _BCM2835_H_

#include "Std_Types.h"
#include "bcm2835_Types.h"

/***********************************************************
 *-----------------------shuzhou add-----------------------
 **********************************************************/

void enable(void);
unsigned long disable(void);
void restore(unsigned long mask);

static inline void __disable_irq() {	__asm volatile("cpsid if");	}
static inline void __enable_irq()  {	__asm volatile("cpsie if"); }

static inline unsigned long _Irq_Save(void)
{
	unsigned long im;
	im = disable();
	return im;
}

static inline void _Irq_Restore(unsigned mask) {
	restore(mask);
}

typedef unsigned long irqmask;

/***********************************************************/

/* Macro for aligning an address to a given number of bytes */
#define ALIGN(addr, bytes) 		(((addr) + (bytes) - 1) & ~((bytes) - 1))

/** Current time, as given by the system timer register */
#define CURRENT_TIME			(((uint64_t)SYSTIMER_CHI << 32) + SYSTIMER_CLO)

/*
 * Physical addresses range from 0x20000000 to 0x20FFFFFF for peripherals.
 * The bus addresses for peripherals are set up to map onto the peripheral
 * bus address range starting at 0x7E000000. Sometimes, there is a need to
 * convert from physical to bus addresses (for example when using DMA).
 */
#define PHYS_TO_BUS_OFFSET		(0x5E000000)
#define MAP_TO_BUSADDRESS(x)	((uint32)&(x) + PHYS_TO_BUS_OFFSET)

/// This means pin HIGH, true, 3.3volts on a pin.
#define HIGH 					0x1
/// This means pin LOW, false, 0volts on a pin.
#define LOW  					0x0

#define REG(x) 					(*(volatile uint32 *)(x))
#define BIT(n) 					(1 << (n))

#define BCM2835_CLOCK_FREQ 		250000000

/********************************************************************************/
/*   		GPIO Registers (pp. 89-108 in BCM2835-ARM-Peripherals.pdf)			*/
/* ******************************************************************************/
#define GPFSEL0 	REG(0x20200000)
//#define GPFSEL1 	REG(0x20200004)
//#define GPFSEL2 	REG(0x20200008)
//#define GPFSEL3 	REG(0x2020000C)
//#define GPFSEL4 	REG(0x20200010)
//#define GPFSEL5 	REG(0x20200014)

#define GPSET0  	REG(0x2020001C)
//#define GPSET1  	REG(0x20200020)
#define GPCLR0  	REG(0x20200028)
//#define GPCLR1  	REG(0x2020002C)
#define GPLEV0		REG(0x20200034)
//#define GPLEV1		REG(0x20200038)

#define GPEDS0      REG(0x20200040) 		//GPIO Pin Event Detect Status 0
//#define GPEDS1      REG(0x20200044) 		//GPIO Pin Event Detect Status 1
#define GPREN0      REG(0x2020004c) 		//GPIO Pin Rising Edge Detect Enable 0
//#define GPREN1      REG(0x20200050) 		//GPIO Pin Rising Edge Detect Enable 1
#define GPFEN0      REG(0x20200058) 		//GPIO Pin Falling Edge Detect Enable 0
//#define GPFEN1      REG(0x2020005c) 		//GPIO Pin Falling Edge Detect Enable 1
#define GPHEN0      REG(0x20200064) 		//GPIO Pin High Detect Enable 0
//#define GPHEN1      REG(0x20200068) 		//GPIO Pin High Detect Enable 1
#define GPLEN0      REG(0x20200070) 		//GPIO Pin Low Detect Enable 0
//#define GPLEN1      REG(0x20200074) 		//GPIO Pin Low Detect Enable 1
#define GPAREN0     REG(0x2020007c) 		//GPIO Pin Async. Rising Edge Detect 0
//#define GPAREN1     REG(0x20200080) 		//GPIO Pin Async. Rising Edge Detect 1
#define GPAFEN0     REG(0x20200088) 		//GPIO Pin Async. Falling Edge Detect 0
//#define GPAFEN1     REG(0x2020008c) 		//GPIO Pin Async. Falling Edge Detect 1

#define GPPUD           REG(0x20200094)
#define GPPUDCLK0       REG(0x20200098)
//#define GPPUDCLK1       REG(0x2020009C)

#define GPIO_PUD_OFF      0x00
#define GPIO_PUD_TRISTATE 0x00
#define GPIO_PUD_PULLDOWN 0x01
#define GPIO_PUD_PULLUP   0x02

#define GPIO_IRQ0       BIT(17)
#define GPIO_IRQ1       BIT(18)
#define GPIO_IRQ2       BIT(19)
#define GPIO_IRQ3       BIT(20)

// -------- GPIO Functions --------
#define GPFN_IN	  		0x00
#define GPFN_OUT   		0x01
#define GPFN_ALT0  		0x04
#define GPFN_ALT1  		0x05
#define GPFN_ALT2  		0x06
#define GPFN_ALT3  		0x07
#define GPFN_ALT4  		0x03
#define GPFN_ALT5  		0x02

#define GPIO0_PAD       0
#define GPIO1_PAD       1
#define GPIO2_PAD       2
#define GPIO3_PAD       3
#define GPIO4_PAD       4
#define GPIO5_PAD       5
#define GPIO6_PAD       6
#define GPIO7_PAD       7
#define GPIO8_PAD       8
#define GPIO9_PAD       9
#define GPIO10_PAD      10
#define GPIO11_PAD      11
#define GPIO12_PAD      12
#define GPIO13_PAD      13
#define GPIO14_PAD      14
#define GPIO15_PAD      15
#define GPIO16_PAD      16
#define GPIO17_PAD      17
#define GPIO18_PAD      18
#define GPIO19_PAD      19
#define GPIO20_PAD      20
#define GPIO21_PAD      21
#define GPIO22_PAD      22
#define GPIO23_PAD      23
#define GPIO24_PAD      24
#define GPIO25_PAD      25
#define GPIO26_PAD      26
#define GPIO27_PAD      27
#define GPIO28_PAD      28
#define GPIO29_PAD      29
#define GPIO30_PAD      30
#define GPIO31_PAD      31
#define GPIO32_PAD      0
#define GPIO33_PAD      1
#define GPIO34_PAD      2
#define GPIO35_PAD      3
#define GPIO36_PAD      4
#define GPIO37_PAD      5
#define GPIO38_PAD      6
#define GPIO39_PAD      7
#define GPIO40_PAD      8
#define GPIO41_PAD      9
#define GPIO42_PAD      10
#define GPIO43_PAD      11
#define GPIO44_PAD      12
#define GPIO45_PAD      13
#define GPIO46_PAD      14
#define GPIO47_PAD      15
#define GPIO48_PAD      16
#define GPIO49_PAD      17
#define GPIO50_PAD      18
#define GPIO51_PAD      19
#define GPIO52_PAD      20
#define GPIO53_PAD      21

#define GPIO0_PORT      &IOPORT0
#define GPIO1_PORT      &IOPORT0
#define GPIO2_PORT      &IOPORT0
#define GPIO3_PORT      &IOPORT0
#define GPIO4_PORT      &IOPORT0
#define GPIO5_PORT      &IOPORT0
#define GPIO6_PORT      &IOPORT0
#define GPIO7_PORT      &IOPORT0
#define GPIO8_PORT      &IOPORT0
#define GPIO9_PORT      &IOPORT0
#define GPIO10_PORT     &IOPORT0
#define GPIO11_PORT     &IOPORT0
#define GPIO12_PORT     &IOPORT0
#define GPIO13_PORT     &IOPORT0
#define GPIO14_PORT     &IOPORT0
#define GPIO15_PORT     &IOPORT0
#define GPIO16_PORT     &IOPORT0
#define GPIO17_PORT     &IOPORT0
#define GPIO18_PORT     &IOPORT0
#define GPIO19_PORT     &IOPORT0
#define GPIO20_PORT     &IOPORT0
#define GPIO21_PORT     &IOPORT0
#define GPIO22_PORT     &IOPORT0
#define GPIO23_PORT     &IOPORT0
#define GPIO24_PORT     &IOPORT0
#define GPIO25_PORT     &IOPORT0
#define GPIO26_PORT     &IOPORT0
#define GPIO27_PORT     &IOPORT0
#define GPIO28_PORT     &IOPORT0
#define GPIO29_PORT     &IOPORT0
#define GPIO30_PORT     &IOPORT0
#define GPIO31_PORT     &IOPORT0
#define GPIO32_PORT     &IOPORT1
#define GPIO33_PORT     &IOPORT1
#define GPIO34_PORT     &IOPORT1
#define GPIO35_PORT     &IOPORT1
#define GPIO36_PORT     &IOPORT1
#define GPIO37_PORT     &IOPORT1
#define GPIO38_PORT     &IOPORT1
#define GPIO39_PORT     &IOPORT1
#define GPIO40_PORT     &IOPORT1
#define GPIO41_PORT     &IOPORT1
#define GPIO42_PORT     &IOPORT1
#define GPIO43_PORT     &IOPORT1
#define GPIO44_PORT     &IOPORT1
#define GPIO45_PORT     &IOPORT1
#define GPIO46_PORT     &IOPORT1
#define GPIO47_PORT     &IOPORT1
#define GPIO48_PORT     &IOPORT1
#define GPIO49_PORT     &IOPORT1
#define GPIO50_PORT     &IOPORT1
#define GPIO51_PORT     &IOPORT1
#define GPIO52_PORT     &IOPORT1
#define GPIO53_PORT     &IOPORT1

/****************************************************************************/
/*                         GPIO pin connection definitions					*/
/****************************************************************************/
/* Common GPIOs */
#define GPIO_SPI0_CE1			7				// SPI, chip select 1
#define GPIO_SPI0_CE0			8				// SPI, chip select 0
#define GPIO_SPI0_MISO			9				// SPI, master input
#define GPIO_SPI0_MOSI			10				// SPI, master output
#define GPIO_SPI0_CLK			11				// SPI, clock
#define GPIO_UART_TXD			14				// Serial communication transmit channel
#define GPIO_UART_RXD			15				// Serial communication receive channel
#define GPIO_CAN_IRQ        	25				// Can interrupt pin

/* Vehicle Control Unit (VCU) GPIOs */
#define GPIO_LED_RED			4				// Led light, red
#define GPIO_PWM_SPEED			17				// PWM-actuator for speed control
#define GPIO_PWM_STEERING		18				// PWM-actuator for steering control
#define GPIO_LED_YELLOW1		22				// Led light, yellow 1 (left)
#define GPIO_FRONT_SPEED  		23				// Front wheel speed sensor
#define GPIO_REAR_SPEED     	24				// Rear wheel speed sensor
#define GPIO_LED_YELLOW2		27				// Led light, yellow 2 (right)

/* Sensor Control Unit (SCU) GPIOs */
#define GPIO_I2C_SDA			2				// I2C, serial data line
#define GPIO_I2C_SCL			3				// I2C, serial clock
#define GPIO_ULTRA_TRIG  		22       		// Input (trigger signal) of the ultrasonic sensor
#define GPIO_ULTRA_ECHO 		27      		// Output (echo) of the ultrasonic sensor


// *****************************************************************************
//                          Timer (ARM Side)
// *****************************************************************************

#define ARM_TIMER_LOD 			REG(0x2000B400)
#define ARM_TIMER_VAL			REG(0x2000B404)
#define ARM_TIMER_CTL 			REG(0x2000B408)
#define ARM_TIMER_CLI 			REG(0x2000B40C)
#define ARM_TIMER_RIS 			REG(0x2000B410)
#define ARM_TIMER_MIS 			REG(0x2000B414)
#define ARM_TIMER_RLD 			REG(0x2000B418)
#define ARM_TIMER_DIV 			REG(0x2000B41C)
#define ARM_TIMER_CNT 			REG(0x2000B420)
#define ARM_TIMER_IRQ       	BIT(0)
#define ARM_TIMER_DISABLE 		0x003E0000
#define ARM_TIMER_ENABLE  		0x003E00A2

// *****************************************************************************
//                        System Timer
// *****************************************************************************

#define SYSTIMER_CS         	REG(0x20003000)
#define SYSTIMER_CLO        	REG(0x20003004)
#define SYSTIMER_CHI        	REG(0x20003008)

#define SYSTIMER_CMP0       	REG(0x2000300C)
#define SYSTIMER_CMP1       	REG(0x20003010)
#define SYSTIMER_CMP2       	REG(0x20003014)
#define SYSTIMER_CMP3       	REG(0x20003018)

#define SYSTIMER_CS_MATCH0  	0x01
#define SYSTIMER_CS_MATCH1  	0x02
#define SYSTIMER_CS_MATCH2  	0x04
#define SYSTIMER_CS_MATCH3  	0x08

#define SYSTIMER_IRQEN0    	 	0x01
#define SYSTIMER_IRQEN1     	0x02
#define SYSTIMER_IRQEN2     	0x04
#define SYSTIMER_IRQEN3     	0x08

#define SYSTIMER_CLOCK_FREQ 	1000000
#define CLKTICKS_PER_SEC  		1000
#define SYSTIMER_TIMER3_IRQ    	BIT(3)

// *****************************************************************************
//                         AUX Registers
// *****************************************************************************

#define AUX_IRQ         		BIT(29)

#define AUX_ENABLES     		REG(0x20215004)

// --- Mini UART Registers -----
#define AUX_MU_IO_REG   		REG(0x20215040)
#define AUX_MU_IIR_REG  		REG(0x20215044)
#define AUX_MU_IER_REG  		REG(0x20215048)
#define AUX_MU_LCR_REG  		REG(0x2021504C)
#define AUX_MU_MCR_REG  		REG(0x20215050)
#define AUX_MU_LSR_REG  		REG(0x20215054)
#define AUX_MU_MSR_REG  		REG(0x20215058)
#define AUX_MU_SCRATCH 			REG(0x2021505C)
#define AUX_MU_CNTL_REG 		REG(0x20215060)
#define AUX_MU_STAT_REG 		REG(0x20215064)
#define AUX_MU_BAUD_REG 		REG(0x20215068)

#define AUX_MU_IER_TX_IRQEN  	BIT(1)

#define AUX_MU_IIR_RX_IRQ     	((AUX_MU_IIR_REG & 0x07) == 0x02)
#define AUX_MU_IIR_TX_IRQ     	((AUX_MU_IIR_REG & 0x07) == 0x01)

#define AUX_MU_LSR_RX_RDY     	(AUX_MU_LSR_REG & BIT(0))
#define AUX_MU_LSR_TX_RDY     	(AUX_MU_LSR_REG & BIT(5))

//uint8 UART_TX[16];
//uint8 UART_RX[16];


// *****************************************************************************
//                        Interrupts
// *****************************************************************************

#define IRQ_BASIC         		REG(0x2000B200)
#define IRQ_PEND1         		REG(0x2000B204)
#define IRQ_PEND2         		REG(0x2000B208)
#define IRQ_FIQ_CONTROL   		REG(0x2000B20C)
#define IRQ_ENABLE1       		REG(0x2000B210)
#define IRQ_ENABLE2       		REG(0x2000B214)
#define IRQ_ENABLE_BASIC  		REG(0x2000B218)
#define IRQ_DISABLE1      		REG(0x2000B21C)
#define IRQ_DISABLE2      		REG(0x2000B220)
#define IRQ_DISABLE_BASIC 		REG(0x2000B224)

#define SPI_IRQ           		BIT(22)

// *****************************************************************************
//       Direct memory access (DMA)
// *****************************************************************************

/* 15 DMA channels are usable on the RPi (0..14) */
#define DMA_CHANNELS            (15)

/* DMA memory addresses (pp.39-47 in BCM2835-ARM-Peripherals.pdf) */
#define DMA_BASE        		(0x20007000)								// DMA channel base (channel 0 is located at this address)
#define DMA_CHANNEL_INC			(0x100)										// Offset to the next DMA channel
#define DMA_CS(n)              	REG(DMA_BASE + DMA_CHANNEL_INC*(n))			// Control status (CS) address of the n-th DMA channel
#define DMA_CONBLK_AD(n)        REG(DMA_BASE + DMA_CHANNEL_INC*(n) + 0x04)	// Control block (CB) address of the n-th DMA channel
#define DMA_DEBUG(n)            REG(DMA_BASE + DMA_CHANNEL_INC*(n) + 0x20)	// Debug register of the n-th DMA channel
#define DMA_GLOBAL_ENABLE       REG(0x20007FF0)								// Global enable bits for each DMA channel

/* DMA control status register bits (pp.47-50 in BCM2835-ARM-Peripherals.pdf) */
#define DMA_CS_ACTIVE			BIT(0)				// Activate the DMA
#define DMA_CS_END         		BIT(1)				// Clear previous end of transfer for this control block
#define DMA_CS_INT        		BIT(2)				// Clear previous interrupt status for this control block
#define DMA_CS_MIDPRIO			BIT(19)				// AXI priority level is set on bits 16:19. Only setting bit 19
													// leads to mid-priority
#define DMA_CS_PANICMIDPRIO		BIT(23)				// AXI panic priority level is set on bits 20:23. Only setting bit
													// 23 leads to mid-priority
#define DMA_CS_WAIT_WRITES		BIT(28)				// DMA will wait for outstanding writes, i.e. it wait for an ack
													// before sending next CB
#define DMA_CS_RESET       		BIT(31)				// DMA channel reset

/* DMA transfer information register bits (p.51 in BCM2835-ARM-Peripherals.pdf) */
#define DMA_WAIT_RESP   		BIT(3)				// Wait for a writer response. This ensures that multiple
													// writes cannot get stacked in the AXI bus pipeline.
#define DMA_D_DREQ      		BIT(6)				// Control destination writes with DREQ
#define DMA_PER_MAP(x)  		((x)<<16)			// Peripheral mapping (bits 16:20)
#define DMA_NO_WIDE_BURSTS  	BIT(26)				// This prevents the DMA from issuing wide writes as 2 beat
													// AXI bursts. Perhaps inefficient, but safe.

/* DMA debug register bits (p.55 in BCM2835-ARM-Peripherals.pdf) */
#define DMA_CLR_NOT_SET_ERROR	BIT(0)				// Clear AXI read last not set error from DEBUG register
#define DMA_CLR_FIFO_ERROR		BIT(1)				// Clear FIFO error from DEBUG register
#define DMA_CLR_READ_ERROR		BIT(2)				// Clear slave read response error from DEBUG register

/**
 * Max offset that needs to be added to a pointer array to
 * compensate for CB-alignment (256bits = 32 Bytes = 8 pointer sizes)
 */
#define DMA_ALIGN_OFFSET 		(32 / (sizeof(uint32*)))


// *****************************************************************************
//       Pulse Width Modulation (PWM)
// *****************************************************************************

/* Pulse width granularity (in microseconds),
 * PWM pulses cannot be altered more often than this limit */
#define PWM_GRANULARITY_US 		10
/* Max number of data samples in each cycle (per channel) */
#define PWM_SAMPLE_LIMIT  		20000

/* PWM control and status registers, 32 bit each (pp.141-147 in BCM2835-ARM-Peripherals.pdf)
 * PWM base address was not in BCM2835-ARM-Peripherals.pdf, but found in several PWM-examples. */
#define PWM_BASE       			(0x2020C000)			// PWM configuration base address
#define PWM_BASE_PHYS 			(0x7E000000 | PWM_BASE)	// PWM configuration base address
#define PWM_CTL        			REG(PWM_BASE)			// PWM control
#define PWM_STATUS     			REG(PWM_BASE + 0x04)	// PWM status
#define PWM_DMAC       			REG(PWM_BASE + 0x08)	// PWM DMA configuration
#define PWM_RNG1     			REG(PWM_BASE + 0x10)	// PWM channel 1 range
#define PWM_DAT1      			REG(PWM_BASE + 0x14)	// PWM channel 1 data
#define PWM_FIF1       			REG(PWM_BASE + 0x18)	// PWM FIFO input
#define PWM_RNG2     			REG(PWM_BASE + 0x20)	// PWM channel 2 range
#define PWM_DAT2      			REG(PWM_BASE + 0x24)	// PWM channel 2 data

/* PWM control bits */
#define PWMCTL_PWEN1    		BIT(0) 					// Enable channel 1
#define PWMCTL_USEF1    		BIT(5)					// Use FIFO for transmission
#define PWMCTL_CLRF1    		BIT(6)					// Clear FIFO

/* PWM DMA configuration bits */
#define PWMDMAC_ENAB    		BIT(31)					// Enable DMA
#define PWMDMAC_THRSHLD			15						// DMA threshold value for PANIC and DREQ signals
#define PWMDMAC_PANIC			(PWMDMAC_THRSHLD<<8)	// DMA threshold bits for PANIC signal
#define PWMDMAC_DREQ 			(PWMDMAC_THRSHLD<<0)	// DMA threshold bits for DREQ signal

/* DMA control block configuration parameters */
#define PWMDMA_TXFRLEN			4						// DMA transfer length (in bytes)
#define PWMDMA_STRIDE			0						// DMA stride (default value)

/* Default PWM channel range */
#define PWM_DEFAULT_RNG			50

/* Clock Manager (CM) audio clocks
 * (follows the same logic as "Clock Manager General Purpose Clocks Control"
 * on pp. 107-108 in BCM2835-ARM-Peripherals.pdf) */
#define CM_PWMBASE				(0x201010A0)			// CM audio clocks base address
#define CM_PWMCTL  				REG(CM_PWMBASE)			// Clock control
#define CM_PWMDIV 				REG(CM_PWMBASE + 0x04)	// Clock divisor

#define CM_PWD   				(0x5A000000)			// Clock manager password ("5a" for all clocks)
#define CMCTL_SRC_PLLD  		(0x06)					// Clock source is set to PLLD (phase locked loop alternative D, i.e. 500 MHz), this value should be high to avoid jitter
#define CMCTL_ENAB 				BIT(4) 					// Enable clock generator
#define CMDIV_DIVI				(50 << 12)				// Integer part of divisor (bits 12:23), the actual frequency is CM_SCR_FREQ / DIVI

// *****************************************************************************
//                 Broadcom Serial Controllers (BSC/I2C)
// *****************************************************************************
/** @brief Default I2C clock frequency (Hertz) */
#define I2C_DEFAULT_FREQ_HZ    	100000

struct bscdevice_t {
  volatile unsigned int control;
  volatile unsigned int status; 
  volatile unsigned int dataLength;
  volatile unsigned int slaveAddress;
  volatile unsigned int dataFifo;
  volatile unsigned int clockDivider;
  volatile unsigned int dataDelay;
  volatile unsigned int clockStretchTimeout;
};

typedef struct bscdevice_t bscdevice_t;

#define BSC0_ADDR 				((bscdevice_t *)0x20205000)
#define BSC1_ADDR 				((bscdevice_t *)0x20804000)
#define BSC2_ADDR 				((bscdevice_t *)0x20805000)

#define BSC_CLOCK_FREQ 			150000000

/* I2C control flags */
#define BSC_I2CEN 				BIT(15)
#define BSC_INTR  				BIT(10)
#define BSC_INTT  				BIT(9)
#define BSC_INTD  				BIT(8)
#define BSC_ST    				BIT(7)
#define BSC_CLEAR 				BIT(4)
#define BSC_READ  				BIT(0)

/* I2C status flags */
#define BSC_TA   				BIT(0) /** @brief Transfer active.*/
#define BSC_DONE 				BIT(1) /** @brief Transfer done.*/
#define BSC_TXW  				BIT(2) /** @brief FIFO needs writing.*/
#define BSC_RXR  				BIT(3) /** @brief FIFO needs reading.*/
#define BSC_TXD 	 			BIT(4) /** @brief FIFO can accept data.*/
#define BSC_RXD  				BIT(5) /** @brief FIFO contains data.*/
#define BSC_TXE  				BIT(6) /** @brief FIFO empty.*/
#define BSC_RXF  				BIT(7) /** @brief FIFO full.*/
#define BSC_ERR 				BIT(8) /** @brief ACK error.*/
#define BSC_CLKT 				BIT(9) /** @brief Clock stretch timeout.*/

/* Rising/Falling Edge Delay Defaults.*/
#define BSC_DEFAULT_FEDL       	0x30
#define BSC_DEFAULT_REDL       	0x30

/* Clock Stretch Timeout Defaults.*/
#define BSC_DEFAULT_CLKT       	0x40

#define CLEAR_STATUS  			(BSC_CLKT|BSC_ERR|BSC_DONE)

#define START_READ    			(BSC_I2CEN|BSC_ST|BSC_CLEAR|BSC_READ)
#define START_WRITE   			(BSC_I2CEN|BSC_ST)

#define I2C_IRQ       			BIT(21)

// *****************************************************************************
//                  Serial Peripheral Interface (SPI)
// *****************************************************************************

/// See 10.5 SPI Register Map
#define SPI0_CS        			REG(0x20204000) /* @brief SPI Master Control and Status.*/
#define SPI0_FIFO      			REG(0x20204004) /* @brief SPI Master TX and RX FIFOs.*/
#define SPI0_CLK       			REG(0x20204008) /* @brief SPI Master Clock Divider.*/
#define SPI0_DLEN      			REG(0x2020400C) /* @brief SPI Master Data Length.*/
#define SPI0_LTOH      			REG(0x20204010) /* @brief SPI LOSSI mode TOH.*/
#define SPI0_DC        			REG(0x20204014) /* @brief SPI DMA DREQ Controls.*/

// Register masks for SPI0_CS
#define SPI_CS_LEN_LONG         0x02000000 /* @brief Enable Long data word in Lossi mode if DMA_LEN is set.*/
#define SPI_CS_DMA_LEN          0x01000000 /* @brief Enable DMA mode in Lossi mode.*/
#define SPI_CS_CSPOL2           0x00800000 /* @brief Chip Select 2 Polarity.*/
#define SPI_CS_CSPOL1           0x00400000 /* @brief Chip Select 1 Polarity.*/
#define SPI_CS_CSPOL0           0x00200000 /* @brief Chip Select 0 Polarity.*/
#define SPI_CS_RXF              0x00100000 /* @brief RXF - RX FIFO Full.*/
#define SPI_CS_RXR              0x00080000 /* @brief RXR RX FIFO needs Reading ( full).*/
#define SPI_CS_TXD              BIT(18) 				/* @brief TXD TX FIFO can accept at least one byte */
#define SPI_CS_RXD              BIT(17) 				/* @brief RXD RX FIFO contains at least one byte */
#define SPI_CS_DONE             BIT(16) 				/* @brief Transfer is complete */
#define SPI_CS_TE_EN            0x00008000 /* @brief Unused.*/
#define SPI_CS_LMONO            0x00004000 /* @brief Unused.*/
#define SPI_CS_LEN              0x00002000 /* @brief LEN LoSSI enable.*/
#define SPI_CS_REN              BIT(12) 				/* @brief Enable read (in bidirectional mode) */
#define SPI_CS_ADCS             0x00000800 /* @brief ADCS Automatically Deassert Chip Select.*/
#define SPI_CS_INTR             0x00000400 /* @brief INTR Interrupt on RXR.*/
#define SPI_CS_INTD             0x00000200 /* @brief INTD Interrupt on Done.*/
#define SPI_CS_DMAEN            0x00000100 /* @brief DMAEN DMA Enable.*/
#define SPI_CS_TA               BIT(7) 					/* @brief Transfer Active */
#define SPI_CS_CSPOL            0x00000040 /* @brief Chip Select Polarity.*/
#define SPI_CS_CLEAR            0x00000030 /* @brief Clear FIFO Clear RX and TX */
#define SPI_CS_CLEAR_RX         BIT(5) 					/* @brief Clear Rx FIFO */
#define SPI_CS_CLEAR_TX         BIT(4) 					/* @brief Clear Tx FIFO */
#define SPI_CS_CPOL             0x00000008 /* @brief Clock Polarity.*/
#define SPI_CS_CPHA             0x00000004 /* @brief Clock Phase.*/
#define SPI_CS_CS0              (~(BIT(0) | BIT(1))) 	/* @brief Chip Select 0 */

/// \brief bcm2835SPIBitOrder SPI Bit order
/// Specifies the SPI data bit ordering for bcm2835_spi_setBitOrder()
typedef enum
{
    BCM2835_SPI_BIT_ORDER_LSBFIRST = 0,  ///< LSB First
    BCM2835_SPI_BIT_ORDER_MSBFIRST = 1   ///< MSB First
} bcm2835SPIBitOrder;

/// \brief SPI Data mode
/// Specify the SPI data mode to be passed to bcm2835_spi_setDataMode()
typedef enum
{
    BCM2835_SPI_MODE0 = 0,  ///< CPOL = 0, CPHA = 0
    BCM2835_SPI_MODE1 = 1,  ///< CPOL = 0, CPHA = 1
    BCM2835_SPI_MODE2 = 2,  ///< CPOL = 1, CPHA = 0
    BCM2835_SPI_MODE3 = 3,  ///< CPOL = 1, CPHA = 1
} bcm2835SPIMode;

/// \brief bcm2835SPIChipSelect
/// Specify the SPI chip select pin(s)
typedef enum
{
    BCM2835_SPI_CS0 = 0,     ///< Chip Select 0
    BCM2835_SPI_CS1 = 1,     ///< Chip Select 1
    BCM2835_SPI_CS2 = 2,     ///< Chip Select 2 (ie pins CS1 and CS2 are asserted)
    BCM2835_SPI_CS_NONE = 3, ///< No CS, control it yourself
} bcm2835SPIChipSelect;

/// \brief bcm2835SPIClockDivider
/// Specifies the divider used to generate the SPI clock from the system clock.
/// Figures below give the divider, clock period and clock frequency.
/// Clock divided is based on nominal base clock rate of 250MHz
/// It is reported that (contrary to the documentation) any even divider may used.
/// The frequencies shown for each divider have been confirmed by measurement
typedef enum
{
    BCM2835_SPI_CLOCK_DIVIDER_65536 = 0,       ///< 65536 = 262.144us = 3.814697260kHz
    BCM2835_SPI_CLOCK_DIVIDER_32768 = 32768,   ///< 32768 = 131.072us = 7.629394531kHz
    BCM2835_SPI_CLOCK_DIVIDER_16384 = 16384,   ///< 16384 = 65.536us = 15.25878906kHz
    BCM2835_SPI_CLOCK_DIVIDER_8192  = 8192,    ///< 8192 = 32.768us = 30/51757813kHz
    BCM2835_SPI_CLOCK_DIVIDER_4096  = 4096,    ///< 4096 = 16.384us = 61.03515625kHz
    BCM2835_SPI_CLOCK_DIVIDER_2048  = 2048,    ///< 2048 = 8.192us = 122.0703125kHz
    BCM2835_SPI_CLOCK_DIVIDER_1024  = 1024,    ///< 1024 = 4.096us = 244.140625kHz
    BCM2835_SPI_CLOCK_DIVIDER_512   = 512,     ///< 512 = 2.048us = 488.28125kHz
    BCM2835_SPI_CLOCK_DIVIDER_256   = 256,     ///< 256 = 1.024us = 976.5625MHz
    BCM2835_SPI_CLOCK_DIVIDER_128   = 128,     ///< 128 = 512ns = = 1.953125MHz
    BCM2835_SPI_CLOCK_DIVIDER_64    = 64,      ///< 64 = 256ns = 3.90625MHz
    BCM2835_SPI_CLOCK_DIVIDER_32    = 32,      ///< 32 = 128ns = 7.8125MHz
    BCM2835_SPI_CLOCK_DIVIDER_16    = 16,      ///< 16 = 64ns = 15.625MHz
    BCM2835_SPI_CLOCK_DIVIDER_8     = 8,       ///< 8 = 32ns = 31.25MHz
    BCM2835_SPI_CLOCK_DIVIDER_4     = 4,       ///< 4 = 16ns = 62.5MHz
    BCM2835_SPI_CLOCK_DIVIDER_2     = 2,       ///< 2 = 8ns = 125MHz, fastest you can get
    BCM2835_SPI_CLOCK_DIVIDER_1     = 1,       ///< 1 = 262.144us = 3.814697260kHz, same as 0/65536
} bcm2835SPIClockDivider;

// *****************************************************************************
//       Power Management, Reset controller and Watchdog registers 
// *****************************************************************************

#define PM_BASE                  		(0x20100000)
#define PM_RSTC                  		REG(PM_BASE+0x1c)
#define PM_WDOG                  		REG(PM_BASE+0x24)

#define PM_WDOG_RESET            		0000000000
#define PM_PASSWORD              		0x5a000000
#define PM_WDOG_TIME_SET         		0x000fffff
#define PM_RSTC_WRCFG_CLR        		0xffffffcf
#define PM_RSTC_WRCFG_SET        		0x00000030
#define PM_RSTC_WRCFG_FULL_RESET 		0x00000020
#define PM_RSTC_RESET           		0x00000102

#define PM_WDOG_UNITS_PER_SECOND      	(1 << 16)
#define PM_WDOG_UNITS_PER_MILLISECOND 	(PM_WDOG_UNITS_PER_SECOND / 1000)

//      USB
#define    USB_IRQ                		BIT(9)

/************************************
 * Board-specific power management  *
 ************************************/
enum board_power_feature {
    POWER_SD     = 0,
    POWER_UART_0 = 1,
    POWER_UART_1 = 2,
    POWER_USB    = 3,
};

/* Synopsys DesignWare Hi-Speed USB 2.0 On-The-Go Controller  */
#define DWC_REGS_BASE         	(0x20980000)


// *****************************************************************************
// 			BCM2835 mailbox registers (used here for the ethernet driver)
// *****************************************************************************
#define MAILBOX_READ            		REG(0x2000B880)
#define MAILBOX_STATUS          		REG(0x2000B898)
#define MAILBOX_WRITE           		REG(0x2000B8A0)

/* BCM2835 mailbox status flags  */
#define MAILBOX_FULL            		BIT(31)
#define MAILBOX_EMPTY           		BIT(30)

/* BCM2835 mailbox channels  */
#define MAILBOX_CHANNEL_POWER_MGMT 		0

/* The BCM2835 mailboxes are used for passing 28-bit messages.  The low 4 bits
 * of the 32-bit value are used to specify the channel over which the message is
 * being transferred  */
#define MAILBOX_CHANNEL_MASK       		0xF

// TODO: This should be removed
#include "extern_defs.h"

// *****************************************************************************
// 						CAN configuration
// *****************************************************************************

//TODO: Don't use these
#define VCU   				1
#define VCU_ID              1
#define VCU_SIDH_MASK       0x00
#define VCU_SIDL_MASK       0xE0
#define VCU_SIDH_FILT       0x00
#define VCU_SIDL_FILT       0x20

#define SCU    				0
#define SCU_ID				2
#define SCU_SIDH_MASK       0x00
#define SCU_SIDL_MASK       0xE0
#define SCU_SIDH_FILT       0x00
#define SCU_SIDL_FILT       0x40

#define TCU_ID              3

//TODO: We should get CAN-communication mode (polling or interrupt) from a configuration file (instead of bcm2835.h)
#define CAN_INTERRUPT		1

/**
 * Select which function a certain GPIO pin should have (pp. 91-94 in BCM2835-ARM-Peripherals.pdf).
 *
 * @param gpio_pin	    	-----    GPIO pin number
 * @param gpio_fn	       	-----    3 bits describing which function to select
 */
void bcm2835_GpioFnSel(uint32 gpio_pin, uint32 gpio_fn);

/**
 * Set/clear only the bits in value covered by the mask
 *
 * @param paddr		    	----- register address
 * @param value	   			----- value that should be set in masked bits
 * @param mask 				----- bits that should be updated (if 1 then update)
 */
void bcm2835_SetBitsInMask(volatile uint32* paddr, uint32 value, uint32 mask);

/**
 * Set a GPIO pin
 *
 * @param gpio_pin	    	-----    GPIO pin number (0-53)
 */
void bcm2835_SetGpioPin(uint32 gpio_pin);

/**
 * Clear a GPIO pin
 *
 * @param gpio_pin	    	-----    GPIO pin number (0-53)
 */
void bcm2835_ClearGpioPin(uint32 gpio_pin);

/**
 * Clear event detect status for a GPIO pin
 *
 * @param gpio_pin	    	-----    GPIO pin number (0-53)
 */
void bcm2835_ClearEventDetectPin(uint32 gpio_pin);

/**
 * Set one bit to 1 in 32-bit GPIO-related registers.
 *
 * @param baseReg	    	-----    address of the first control register for this functionality
 * 									 (the second one follows 32-bit later)
 * @param gpio_pin	    	-----    GPIO pin number (0-53)
 */
void bcm2835_SetReadWriteGpioReg(volatile uint32* baseReg, uint32 gpio_pin);

/**
 * Set one bit to 0 in 32-bit GPIO-related registers.
 *
 * @param baseReg	    	-----    address of the first control register for this functionality
 * 									 (the second one follows 32-bit later)
 * @param gpio_pin	    	-----    GPIO pin number (0-53)
 */
void bcm2835_UnsetReadWriteGpioReg(volatile uint32* baseReg, uint32 gpio_pin);

/**
 * Read the bit corresponding to a given GPIO pin from one of its control registers.
 *
 * @param baseReg	    	----- address of the first control register for this functionality
 * @param gpio_pin	    	----- GPIO pin number (0-53)
 * @return value			----- Binary (ON/OFF) value on this GPIO pin
 */
uint32 bcm2835_ReadGpioPin(volatile uint32* baseReg, uint32 gpio_pin);

/**
 * Sleep for some microseconds
 *
 * @param micros	    	----- number of microseconds to wait in a loop
 */
void bcm2835_Sleep(uint64_t micros);

/**
 * Power on or power off BCM2835 hardware.
 *
 * @param feature			----- Device or hardware to power on or off.
 * @param on				----- ::TRUE to power on; ::FALSE to power off.
 *
 * @return					----- ::OK if successful; ::SYSERR otherwise.
 */
int bcm2835_SetPower(enum board_power_feature feature, boolean on);

void bcm2835_GpioIsr(void);
#endif
