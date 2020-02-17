/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 *
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with
 * the terms contained in the written license agreement between you and ArcCore,
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as
 * published by the Free Software Foundation and appearing in the file
 * LICENSE.GPL included in the packaging of this file or here
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/
/** @tagSettings DEFAULT_ARCHITECTURE=JACINTO6 */
/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */
#ifndef I2C_CFG_H_
#define I2C_CFG_H_

#include "Std_Types.h"

#define I2C_IRQ_PRIORITY              5uL
#define I2C_DEV_ERROR_DETECT          STD_ON   /* Generate from the generator */
#define I2C_VERSION_INFO_API          STD_ON   /* Generate from the generator */
#define I2C_USE_GPT                   STD_OFF
#define I2C_JOB_GPT_CHANNEL           0u       /* assign from the generator */
#define I2C_MAIN_FUNCTION_PERIOD      10u      /* Generate a non zero value and avoid divide by Zero, in msec */
#define I2C_TOTAL_CHANNEL_LENGTH      950u     /* Generate sum of all channel length */
#define I2C_TOTAL_CHANNEL_QUEUE_SIZE  22u      /* Generate sum of all channel queue size */

typedef uint8 EcucBooleanParamDef;
typedef float EcucFloatParamDef;
typedef uint32 EcucIntegerParamDef;

typedef void (*EcucFunctionNameDef) (Std_ReturnType result, uint16 requestHandle);

/* slave addresses */
#define I2C_ADDRESS_PCF8575 0x20
#define I2C_ADDRESS_PCF8575_1 0x21
#define I2C_ADDRESS_CHIP_X1 0x28
#define I2C_ADDRESS_CHIP_X2 0x38
#define I2C_ADDRESS_CHIP_X3 0x28
#define I2C_ADDRESS_CHIP_X4 0x28

/* Number of elements in each group */
#define I2C_CHANNEL_GROUP1_COUNT 2
#define I2C_CHANNEL_GROUP2_COUNT 1
#define I2C_CHANNEL_GROUP3_COUNT 1
#define I2C_CHANNEL_GROUP4_COUNT 1
#define I2C_CHANNEL_GROUP5_COUNT 1
#define I2C_SLAVE_GROUP1_COUNT 2
#define I2C_SLAVE_GROUP2_COUNT 1
#define I2C_SLAVE_GROUP3_COUNT 1
#define I2C_SLAVE_GROUP4_COUNT 1
#define I2C_SLAVE_GROUP5_COUNT 1

/* Max length of each channel data buffer */
#define I2C_CHANNEL1_MAX_LENGTH 100
#define I2C_CHANNEL2_MAX_LENGTH 200
#define I2C_CHANNEL3_MAX_LENGTH 300
#define I2C_CHANNEL4_MAX_LENGTH 200
#define I2C_CHANNEL5_MAX_LENGTH 150


/* Queue size of each channel */
#define I2C_CHANNEL1_QUEUE_SIZE 10
#define I2C_CHANNEL2_QUEUE_SIZE 5
#define I2C_CHANNEL3_QUEUE_SIZE 2
#define I2C_CHANNEL4_QUEUE_SIZE 3
#define I2C_CHANNEL5_QUEUE_SIZE 2

typedef enum
{
    I2C_ADDR_SIZE_7BIT = 0,
    I2C_ADDR_SIZE_10BIT,

    I2C_ADDR_SIZE_COUNT,

    I2C_ADDR_SIZE_WRONG
} EcucEnumerationParamDef;

/* Width of Destination Register in Bytes */
/* @req SWS_I2c_00005 */
enum
{
    I2C_SIZE_8BIT = 0,  /* Register size is 8 bit */
    I2C_SIZE_16BIT,     /* Register size is 16 bit */
    I2C_SIZE_24BIT,     /* Register size is 24 bit */
    I2C_SIZE_32BIT,     /* Register size is 32 bit */
    I2C_SIZE_NONE,      /* Ignore register argument, i.e. treat it as size 0 bits. */

    I2C_SIZE_COUNT,

    I2C_SIZE_WRONG
};

/* name of the individual I2C controller configured */
enum
{
    I2C_CONTROLLER_0 = 0,              /* I2C controller 0 */
    I2C_CONTROLLER_1,                  /* I2C controller 1 */
    I2C_CONTROLLER_2,                  /* I2C controller 2 */
    I2C_CONTROLLER_3,                  /* I2C controller 3 */

    I2C_CONTROLLER_COUNT,              /* number of I2C controllers */

    I2C_CONTROLLER_WRONG               /* no controller selected or wrong calculated value */
};

/* module enable / disable enumeration */
enum
{
    I2C_DISABLE = 0,
    I2C_ENABLE
}I2cEnumerationModulEnable;

/* Channel Names configured */
enum
{
    I2C_CH_NAME1 = 0,
    I2C_CH_NAME2,
    I2C_CH_NAME3,
    I2C_CH_NAME4,
    I2C_CH_NAME5,

    I2C_CH_COUNT,

    I2C_CH_WRONG
};

/* slave names configured */
enum
{
    I2C_SLAVE_NAME1 = 0,
    I2C_SLAVE_NAME2,
    I2C_SLAVE_NAME3,
    I2C_SLAVE_NAME4,
    I2C_SLAVE_NAME5,

    I2C_SLAVE_COUNT,

    I2C_SLAVE_WRONG
};

/* Handle type for identifying an I2C Channel */
/* @req SWS_I2c_00004 */
typedef uint16 I2c_ChannelHandleType;

/* Width of Destination Address in Bytes */
/* @req SWS_I2c_00005 */
typedef uint8 I2c_AddressSizeType;

/* Communication settings of an external I2C Slave */
typedef struct
{
    EcucIntegerParamDef I2cSlaveAddress;    /* The slave's unique I2C address */
    EcucEnumerationParamDef I2cSlaveAddressSize;
} I2c_SlaveType;

/* Logical I2C channel is defined as an interaction point between an user on
the AUTOSAR stack (master) and a functional entity located on a slave.
Subsequent request on the same channel need to be serialized, but
different channel requests are queued in the I2C Driver and will be handled
be handled in the order of channel priorities. */
typedef struct
{
    EcucIntegerParamDef I2cChannelId;          /* Identifier of the I2cChannel. Callers need to use this Id for issuing
                                                commands to an I2C channel. */
    EcucIntegerParamDef I2cChannelMaxLength;   /* Maximum number of bytes to be transferred on this channel. */
    EcucIntegerParamDef I2cChannelPriority;    /* The priority of the I2C channel.
                                                The value 0 is defined as the lowest priority of an I2C channel. Accordingly
                                                bigger numbers define higher priorities. */
    EcucIntegerParamDef I2cChannelQueueSize;   /* specifies the maximum number of requests that can be buffered for the channel.*/
    /* @req SWS_I2c_00012 */
    EcucFunctionNameDef I2cJobEndNotification; /* Callback notification for the given I2cChannel. The callback will be
                                                  executed either in case the job finished normally or if there was an error. */
    /* @req SWS_I2c_00032 */
    const I2c_SlaveType* I2cSlaveRef;          /* Reference to the I2cSlave this I2cChannel is communicating with. */
} I2c_ChannelType;

/*  Configuration of the individual i2c controller */
typedef struct
{
    EcucIntegerParamDef I2cBusSpeed;        /* Bus speed in kbit/s
                                               - Standard-mode (Sm): bit rate up to 100kbit/s - Fast-mode(Fm): bit rate up
                                               to 400kbit/s - High-speed mode (Hs): bit rate up to 3.4 MBit/s */
    EcucIntegerParamDef I2cControllerTimeout; /* Timeout in seconds. If a message could not be successfully transferred
                                               within the timeout, an error will be reported to Dem. */
    const I2c_ChannelType** I2cChannel;     /* A logical I2C channel is defined as an interaction point
                                               between an user on the AUTOSAR stack (master) and a
                                               functional entity located on a slave. Subsequent request on the
                                               same channel need to be serialized, but different channel
                                               requests are queued in the I2C Driver and will be handled be
                                               handled in the order of channel priorities. */
    const I2c_SlaveType** I2cSlave;         /* The communication settings of an external I2C Slave. */
} I2c_ControllerType;

/*  I2cConfigSet contains the global configuration parameters and sub containers of the I2c module */
typedef struct
{
    const I2c_ControllerType* I2cController; /* Configuration of the individual controller */
} I2c_ConfigSetType;

/* I2cGeneral contains the general configuration parameters of the I2c module. */
typedef struct
{
    EcucBooleanParamDef I2cDevErrorDetect;   /* Switches the Default Error Tracer (Det)
                                                detection and notification ON or OFF.
                                                true: enabled (ON).
                                                false: disabled (OFF). */
    EcucFloatParamDef I2cMainFunctionPeriod; /* Specifies the period of main function I2c_MainFunction in seconds.
                                                I2C driver does not require this information but the BSW scheduler. */
    EcucBooleanParamDef I2cVersionInfoApi;   /* Pre-processor switch for enabling version info API support. */
} I2c_GeneralType;

/* Configuration of the I2c (I2C Driver) module. */
typedef struct
{
    const I2c_ConfigSetType* I2cConfigSet;   /* This container contains the global configuration parameters
                                                and sub containers of the I2c module. */
    const I2c_GeneralType* I2cGeneral;       /* Contains the general configuration parameters of the I2c module. */
} I2c_I2cType;

/*  implementationSpecific */
typedef struct
{
    uint8 enable;                           /*  I2C_ENABLE: module in use; I2C_DISABLE: module not used*/
    uint8 I2cChannelCount;                  /* number of channels */
    uint8 I2cSlaveCount;                    /* number of slaves */
} I2c_ModuleType;

/* The type of the external data structure containing the initialization data for the I2c driver. */
/* @req SWS_I2c_00003 */
typedef struct
{
/*  implementationSpecific */
    const I2c_ModuleType *module;
} I2c_ConfigType;

/* publication of the I2C container */
extern const I2c_ConfigType I2c_config;
extern const I2c_ConfigSetType I2cConfigSet;

#endif /*I2C_CFG_H_*/

