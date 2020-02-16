/*
 * bcm2835_Types.h
 *
 *  Created on: Mar 31, 2013
 *      Author: Zhang Shuzhou
 */

#ifndef BCM2835_TYPES_H_
#define BCM2835_TYPES_H_

#include "Std_Types.h"

typedef struct {
  /**
   * @brief   GPIO_LATCH register.
   * @details This register represents the output latch of the GPIO port.
   */
  uint32_t          latch;
  volatile uint32_t *gpset;
  volatile uint32_t *gpclr;
  volatile uint32_t *gplev;
  volatile uint32_t *gppudclk;
  unsigned int pin_base;
} gpio_port_t;

/**
 * @brief   First I/O port identifier.
 * @details Low level drivers can define multiple ports, it is suggested to
 *          use this naming convention.
 */
extern gpio_port_t IOPORT0;
extern gpio_port_t IOPORT1;

typedef uint32_t ioportmask_t;
typedef gpio_port_t *ioportid_t;



#endif /* BCM2835_TYPES_H_ */
