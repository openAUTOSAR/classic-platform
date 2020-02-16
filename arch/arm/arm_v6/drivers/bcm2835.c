#include "bcm2835.h"

#include "mcp2515.h"
#include "Uart.h"

#include "Sensors.h"
#include "Can.h"

#include "IoHwAb_Cfg.h"

/**
 * Set one bit to 1 in 32-bit write-only GPIO-related registers.
 *
 * In this function, the pin bit is written directly, since we cannot read
 * the register value. By definition, 0's don't affect the actual values
 * of write-only register.
 *
 * There are several GPIO control registers, with one one bit per GPIO pin.
 * Typically, there are two registers per control functionality (to cover all 54 pins).
 * Some examples:
 * 		Set output pin (GPSETn)
 * 		Clear output pin (GPCLRn)
 *
 * @param baseReg	    	-----    address of the first control register for this functionality
 * 									 (the second one follows 32-bit later)
 * @param gpio_pin	    	-----    GPIO pin number (0-53)
 */
static void bcm2835_SetWriteOnlyGpioReg(volatile uint32* baseReg, uint32 gpio_pin)
{
	uint32 regOffset = gpio_pin / 32;							/* 32 pins per register */
	uint32 bitOffset = gpio_pin % 32;							/* One bit per pin within a register */

	*(baseReg + regOffset) = (1 << bitOffset);					/* Set the pin bit in the corresponding register to the new value  */
}

/**
 * Select which function a certain GPIO pin should have (pp. 91-94 in BCM2835-ARM-Peripherals.pdf).
 *
 * There are 54 GPIO pins on BCM2835. Each of them has at least
 * two alternative functions, in addition to acting simply as an
 * input or an output pin.
 *
 * There are 6 registers for pin function selection (GPFSELx), each
 * handling at most 10 pins (for example, GPFSEL3 handles pins 30-39).
 * Further, each pin is represented by three bits in these registers
 * (for example, pin 34 is represented by bits 12-14 in GPFSEL3).
 * Bit values have the following meaning:
 * 		000 = GPIO Pin x is an input
 * 		001 = GPIO Pin x is an output
 * 		100 = GPIO Pin x takes alternate function 0
 * 		101 = GPIO Pin x takes alternate function 1
 * 		110 = GPIO Pin x takes alternate function 2
 * 		111 = GPIO Pin x takes alternate function 3
 * 		011 = GPIO Pin x takes alternate function 4
 * 		010 = GPIO Pin x takes alternate function 5
 *
 * @param gpio_pin	    	-----    GPIO pin number
 * @param gpio_fn	       	-----    3 bits describing which function to select (as above)
 */
void bcm2835_GpioFnSel(uint32 gpio_pin, uint32 gpio_fn)
{
  uint32 regOffset = gpio_pin / 10; 							/* 10 pins per GPIO select register (GPFSELx) */
  uint32 bitOffset = (gpio_pin % 10) * 3; 						/* 3 bits per pin (or FSELx) in each GPFSELx */
  volatile uint32 *gpfnsel = &GPFSEL0 + regOffset;				/* Get the right GPFSELx address */

  *gpfnsel &= ~(0x07 << bitOffset); 							/* Clear all previous selection settings for this pin */
  *gpfnsel |= (gpio_fn << bitOffset); 							/* Set the pin to the new value  */
}

/**
 * Set/clear only the bits in value covered by the mask
 *
 * @param paddr		    	----- register address
 * @param value	   			----- value that should be set in masked bits
 * @param mask 				----- bits that should be updated (if 1 then update)
 */
void bcm2835_SetBitsInMask(volatile uint32* paddr, uint32 value, uint32 mask)
{
    uint32 temp = *paddr;										/* Read from register */

    temp &= ~mask;												/* Clear the bits defined by mask */
    temp |= (value & mask);										/* Set the bits defined in mask to new values */

    *paddr = temp;												/* Write to register */
}

/**
 * Set a GPIO pin to high
 *
 * @param gpio_pin	    	-----    GPIO pin number (0-53)
 */
void bcm2835_SetGpioPin(uint32 gpio_pin)
{
	bcm2835_SetWriteOnlyGpioReg(&GPSET0, gpio_pin);
}

/**
 * Clear a GPIO pin (set it to low)
 *
 * @param gpio_pin	    	-----    GPIO pin number (0-53)
 */
void bcm2835_ClearGpioPin(uint32 gpio_pin)
{
	bcm2835_SetWriteOnlyGpioReg(&GPCLR0, gpio_pin);
}

/**
 * Clear event detect status for a GPIO pin
 *
 * This must be done in a write-only way, otherwise all event statuses would be cleared.
 *
 * @param gpio_pin	    	-----    GPIO pin number (0-53)
 */
void bcm2835_ClearEventDetectPin(uint32 gpio_pin) {
	bcm2835_SetWriteOnlyGpioReg(&GPEDS0, gpio_pin);
}

/**
 * Set one bit to 1 in 32-bit R/W GPIO-related registers.
 *
 * In this function, only the pin bit to be set is modified (i.e. current register
 * value is first read and then modified).
 *
 * There are several GPIO control registers, with one one bit per GPIO pin.
 * Typically, there are two registers per control functionality (to cover all 54 pins).
 * Some examples:
 * 		Enable rising edge detection (GPRENn)
 * 		Clear the event detect status register bit (GPEDSn)
 *
 * @param baseReg	    	-----    address of the first control register for this functionality
 * 									 (the second one follows 32-bit later)
 * @param gpio_pin	    	-----    GPIO pin number (0-53)
 */
void bcm2835_SetReadWriteGpioReg(volatile uint32* baseReg, uint32 gpio_pin)
{
	uint32 regOffset = gpio_pin / 32;							/* 32 pins per register */
	uint32 bitOffset = gpio_pin % 32;							/* One bit per pin within a register */

	*(baseReg + regOffset) |= (1 << bitOffset);					/* Modify the pin bit in the corresponding register to the new value  */
}

/**
 * Set one bit to 0 in 32-bit GPIO-related registers.
 *
 * There are several GPIO control registers, with one one bit per GPIO pin.
 * Typically, there are two registers per control functionality (to cover all 54 pins).
 *
 * @param baseReg	    	-----    address of the first control register for this functionality
 * 									 (the second one follows 32-bit later)
 * @param gpio_pin	    	-----    GPIO pin number (0-53)
 */
void bcm2835_UnsetReadWriteGpioReg(volatile uint32* baseReg, uint32 gpio_pin)
{
	uint32 regOffset = gpio_pin / 32;							/* 32 pins per register */
	uint32 bitOffset = gpio_pin % 32;							/* One bit per pin within a register */

	*(baseReg + regOffset) &= ~(1 << bitOffset);				/* Set the right bit in the right register to the new value  */
}

/**
 * Read the bit corresponding to a given GPIO pin from one of its control registers.
 *
 * There are several GPIO control registers, with one bit per GPIO pin.
 * Typically, there are two registers per control functionality (to cover all 54 pins).
 * Examples:
 * 		Read if an event (rising or falling edge) has been detected (GPEDSn)
 * 		Read pin level (GPLEVn)
 *
 * @param baseReg	    	----- address of the first control register for this functionality
 * @param gpio_pin	    	----- GPIO pin number (0-53)
 * @return value			----- Binary (ON/OFF) value on this GPIO pin
 */
uint32 bcm2835_ReadGpioPin(volatile uint32* baseReg, uint32 gpio_pin)
{
	uint32 regOffset = gpio_pin / 32;							/* 32 pins per register */
	uint32 bitOffset = gpio_pin % 32;							/* One bit per pin within a register */

	uint32 value = *(baseReg + regOffset);						/* Get the pin value from the right register */

	return (value & (1 << bitOffset)) ? HIGH : LOW;				/* Convert the return value to a boolean */
}

/**
 * Sleep for some microseconds
 *
 * Loop, doing nothing, until the system timer register says that the
 * specified delay has passed.
 *
 * @param micros	    	----- number of microseconds to wait in a loop
 */
void bcm2835_Sleep(uint64_t micros)
{
    uint64_t start = CURRENT_TIME;
    while (CURRENT_TIME < (start + micros));
}

/**
 * Handle interrupts on GPIO pins
 */
void bcm2835_GpioIsr(void) {
	/* Declare the mask for GPIO pins with enabled interrupts */
	uint32 irqMask = 0x0;
#if GPIODEF_SPEEDSENS
	irqMask |= (BIT(GPIO_FRONT_SPEED) |
			    BIT(GPIO_REAR_SPEED));
#endif
#if defined(USE_CAN) && CAN_INTERRUPT
	irqMask |= BIT(GPIO_CAN_IRQ);
#endif

	/* If there is a edge flank on one of interrupt-enabled pins,
	 * go to the appropriate interrupt handler. Repeat until all
	 * interrupts have been handled. */
	while (GPEDS0 & irqMask) {
#if GPIODEF_SPEEDSENS
		SpeedSensor_Isr();
#endif

#if defined(USE_CAN) && CAN_INTERRUPT
		Can_MainFunction_Read();
#endif
	}
}

/****************************************************************************************/
/*					Mailbox (for Ethernet) - borrowed from Xinu	(not used)				*/
/****************************************************************************************/

/**
 * Write to the specified channel of the mailbox.
 */
static void bcm2835_mailbox_write(uint32 channel, uint32 value)
{
    while (MAILBOX_STATUS & MAILBOX_FULL){}

    MAILBOX_WRITE = (value & ~MAILBOX_CHANNEL_MASK) | channel;
}

/**
 * Read from the specified channel of the mailbox.
 */
static uint32 bcm2835_mailbox_read(uint32 channel)
{
	uint32 value;

    while (MAILBOX_STATUS & MAILBOX_EMPTY){}

    do
    {
    	value = MAILBOX_READ;
    } while ((value & MAILBOX_CHANNEL_MASK) != channel);

    return (value & ~MAILBOX_CHANNEL_MASK);
}

/**
 * Retrieve the bitmask of power on/off state.
 */
static uint32 bcm2835_get_power_mask(void)
{
    return (bcm2835_mailbox_read(MAILBOX_CHANNEL_POWER_MGMT) >> 4);
}

/**
 * Set the bitmask of power on/off state.
 */
static void bcm2835_set_power_mask(uint32 mask)
{
    bcm2835_mailbox_write(MAILBOX_CHANNEL_POWER_MGMT, mask << 4);
}

/**
 * Power on or power off BCM2835 hardware.
 *
 * @param feature			----- Device or hardware to power on or off.
 * @param on				----- ::TRUE to power on; ::FALSE to power off.
 *
 * @return					----- ::OK if successful; ::SYSERR otherwise.
 */
int bcm2835_SetPower(enum board_power_feature feature, boolean on)
{
	/* Bitmask that gives the current on/off state of the BCM2835 hardware.
	 * This is a cached value.  */
	static uint32 bcm2835_power_mask;
	uint32 bit;
	uint32 newmask;
    boolean is_on;

    bit = 1 << feature;
    is_on = (bcm2835_power_mask & bit) != 0;

    if (on != is_on)
    {
        newmask = bcm2835_power_mask ^ bit;
        bcm2835_set_power_mask(newmask);
        bcm2835_power_mask = bcm2835_get_power_mask();

        if (bcm2835_power_mask != newmask)
        {
        	pi_printf("error - Powers on BCM2835 hardware\r\n");
            return (-1);
        }
    }

    return 1;
}
