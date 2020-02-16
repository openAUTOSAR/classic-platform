/*
 * mpu_9150.h
 *
 *  Created on: 6 aug 2014
 *      Author: sse
 */

#ifndef MPU_9150_H_
#define MPU_9150_H_
#include "Std_Types.h"
#include "bcm2835.h"
#include "irq_types.h"

/**
 * @brief   Type of a structure representing an I2C driver.
 */
typedef struct i2cdriver i2cdriver;

/**
 * @brief   I2C status type
 */
typedef uint32_t i2cstatus_t;

/**
 * @brief   I2C flags type
 */
typedef uint32_t i2cflags_t;

/**
 * @brief   I2C address type
 * @details I2C address type. May support 10 bit addressing in the future.
 */
typedef uint16_t i2caddr_t;

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
	I2C_UNINIT = 0, /**< Not initialized.           */
	I2C_STOP = 1, /**< Stopped.                   */
	I2C_READY = 2, /**< Ready.                     */
	I2C_ACTIVE_TX = 3, /**< Transmitting. */
	I2C_TX_DONE = 4,
	I2C_TX_ERR = 5,
	I2C_ACTIVE_RX = 6, /**< Receiving.                 */
	I2C_RX_DONE = 7, /**< Receiving.                 */
	I2C_RX_ERR = 8,
    I2C_LOCKED = 9, /**> Bus or driver locked.      */
} i2cstate_t;

typedef struct {
  /** @brief I2C bus bit rate.*/
  uint32_t                  ic_speed;
  /* End of the mandatory fields.*/
} I2CConfig;

/**
 * @brief   Structure representing an I2C driver.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
struct i2cdriver {
  /** @brief Driver state.*/
  i2cstate_t                state;
  /** @brief Current configuration data.*/
  const I2CConfig           *config;
  /** @brief Error flags.*/
  i2cflags_t                errors;
  /** @brief BSC device registers.*/
  bscdevice_t               *device;
//#if I2C_USE_MUTUAL_EXCLUSION
//#if CH_USE_MUTEXES
//  Mutex                     mutex;
//#endif /* CH_USE_MUTEXES */
//#endif /* I2C_USE_MUTUAL_EXCLUSION */
//#if defined(I2C_DRIVER_EXT_FIELDS)
//  I2C_DRIVER_EXT_FIELDS
//#endif
  /* End of the mandatory fields.*/
//  /**
//   * @brief   Thread waiting for I/O completion.
//   */
//  Thread                    *thread;
  /**
   * @brief   Address of slave device.
   */
  i2caddr_t                 addr;
  /**
   * @brief   Pointer to the buffer with data to send.
   */
  const uint8_t             *txbuf;
  /**
   * @brief   Number of bytes of data to send.
   */
  uint32                    txbytes;
  /**
   * @brief   Current index in buffer when sending data.
   */
  uint32                    txidx;
  /**
   * @brief   Pointer to the buffer to put received data.
   */
  uint8_t                   *rxbuf;
  /**
   * @brief   Number of bytes of data to receive.
   */
  uint32                    rxbytes;
  /**
   * @brief   Current index in buffer when receiving data.
   */
  uint32                    rxidx;
};



void I2c_Init(void);

i2cstate_t I2c_Write(i2caddr_t addr, const uint8_t *txbuf, uint32 txbytes);

void I2c_Read(i2caddr_t addr, uint8_t *rxbuf, uint32 rxbytes);


#endif /* MPU_9150_H_ */
