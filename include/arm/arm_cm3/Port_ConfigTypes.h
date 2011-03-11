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

#ifndef PORT_CONFIGTYPES_H_
#define PORT_CONFIGTYPES_H_

#define GPIO_INPUT_MODE           (0)
#define GPIO_OUTPUT_10MHz_MODE    (1)
#define GPIO_OUTPUT_2MHz_MODE     (2)
#define GPIO_OUTPUT_50MHz_MODE    (3)

/* Valid for input modes. */
#define GPIO_ANALOG_INPUT_CNF     (0 << 2)
#define GPIO_FLOATING_INPUT_CNF   (1 << 2)
#define GPIO_INPUT_PULLUP_CNF     (2 << 2)
#define GPIO_RESERVED_CNF         (3 << 2)

/* Valid for output modes. */
#define GPIO_OUTPUT_PUSHPULL_CNF  (0 << 2)
#define GPIO_OUTPUT_OPENDRAIN_CNF (1 << 2)
#define GPIO_ALT_PUSHPULL_CNF     (2 << 2)
#define GPIO_ALT_OPENDRAIN_CNF    (3 << 2)

#define GPIO_OUTPUT_LOW           (0)
#define GPIO_OUTPUT_HIGH          (1)

typedef struct
{
  uint8_t GpioPinCnfMode_0:4;
  uint8_t GpioPinCnfMode_1:4;
  uint8_t GpioPinCnfMode_2:4;
  uint8_t GpioPinCnfMode_3:4;
  uint8_t GpioPinCnfMode_4:4;
  uint8_t GpioPinCnfMode_5:4;
  uint8_t GpioPinCnfMode_6:4;
  uint8_t GpioPinCnfMode_7:4;
  uint8_t GpioPinCnfMode_8:4;
  uint8_t GpioPinCnfMode_9:4;
  uint8_t GpioPinCnfMode_10:4;
  uint8_t GpioPinCnfMode_11:4;
  uint8_t GpioPinCnfMode_12:4;
  uint8_t GpioPinCnfMode_13:4;
  uint8_t GpioPinCnfMode_14:4;
  uint8_t GpioPinCnfMode_15:4;
}GpioPinCnfMode_Type;

typedef struct
{
  uint8_t GpioPinOutLevel_0:1;
  uint8_t GpioPinOutLevel_1:1;
  uint8_t GpioPinOutLevel_2:1;
  uint8_t GpioPinOutLevel_3:1;
  uint8_t GpioPinOutLevel_4:1;
  uint8_t GpioPinOutLevel_5:1;
  uint8_t GpioPinOutLevel_6:1;
  uint8_t GpioPinOutLevel_7:1;
  uint8_t GpioPinOutLevel_8:1;
  uint8_t GpioPinOutLevel_9:1;
  uint8_t GpioPinOutLevel_10:1;
  uint8_t GpioPinOutLevel_11:1;
  uint8_t GpioPinOutLevel_12:1;
  uint8_t GpioPinOutLevel_13:1;
  uint8_t GpioPinOutLevel_14:1;
  uint8_t GpioPinOutLevel_15:1;
}GpioPinOutLevel_Type;

/* To be compatible with Port.h */
typedef uint8_t Port_PinType;

/** Top level configuration container */
typedef struct
{
  /** Total number of pins */
  uint16_t padCnt;
  /** List of pin configurations */
  const GpioPinCnfMode_Type *padConfig;
  const GpioPinOutLevel_Type *outConfig;
  /** Total number of pin default levels */

  uint16_t remapCount;
  const uint32_t* remaps;

} Port_ConfigType;


#endif /* PORT_CONFIGTYPES_H_ */
