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
#include "I2c.h"
/* Configuration of the I2C hardware */

static const I2c_ModuleType ModuleList[I2C_CONTROLLER_COUNT] = {
    {
        .enable = (uint8)I2C_ENABLE,
        .I2cChannelCount = I2C_CHANNEL_GROUP1_COUNT,
        .I2cSlaveCount   = I2C_SLAVE_GROUP1_COUNT
    },
    {
        .enable = (uint8)I2C_ENABLE,
        .I2cChannelCount = I2C_CHANNEL_GROUP2_COUNT,
        .I2cSlaveCount   = I2C_SLAVE_GROUP2_COUNT
    },
    {
        .enable = (uint8)I2C_DISABLE,
        .I2cChannelCount = I2C_CHANNEL_GROUP3_COUNT,
        .I2cSlaveCount   = I2C_SLAVE_GROUP3_COUNT
    },
    {
        .enable = (uint8)I2C_DISABLE,
        .I2cChannelCount = I2C_CHANNEL_GROUP4_COUNT,
        .I2cSlaveCount   = I2C_SLAVE_GROUP4_COUNT
    }

};

const I2c_ConfigType I2c_config =
{
    .module = ModuleList
};

/* I2cGeneral contains the general configuration parameters of the I2c module. */
static const I2c_GeneralType I2cGeneral =
{
    .I2cDevErrorDetect = TRUE,
    .I2cMainFunctionPeriod = 0.01f,
    .I2cVersionInfoApi = TRUE
};

/* The communication settings of an external I2C Slave. */
static const I2c_SlaveType I2cSlave[I2C_SLAVE_COUNT] =
{
    {   /* I2C_SLAVE_NAME1 */
        .I2cSlaveAddress = I2C_ADDRESS_PCF8575,
        .I2cSlaveAddressSize = I2C_ADDR_SIZE_7BIT
    },
    {   /* I2C_SLAVE_NAME2 */
        .I2cSlaveAddress = I2C_ADDRESS_CHIP_X1,
        .I2cSlaveAddressSize = I2C_ADDR_SIZE_10BIT
    },
    {   /* I2C_SLAVE_NAME3 */
        .I2cSlaveAddress = I2C_ADDRESS_CHIP_X2,
        .I2cSlaveAddressSize = I2C_ADDR_SIZE_7BIT
    },
    {   /* I2C_SLAVE_NAME4 */
        .I2cSlaveAddress = I2C_ADDRESS_PCF8575_1,
        .I2cSlaveAddressSize = I2C_ADDR_SIZE_7BIT
    },
    {   /* I2C_SLAVE_NAME5 */
        .I2cSlaveAddress = I2C_ADDRESS_CHIP_X3,
        .I2cSlaveAddressSize = I2C_ADDR_SIZE_7BIT
    }
};

/* A logical I2C channel is defined as an interaction point between an user on
   the AUTOSAR stack (master) and a functional entity located on a slave.
   Subsequent request on the same channel need to be serialized, but
   different channel requests are queued in the I2C Driver and will be handled
   be handled in the order of channel priorities. */
static const I2c_ChannelType I2cChannel[I2C_CH_COUNT] =
{
    {
        .I2cChannelId = I2C_CH_NAME1,
        .I2cChannelMaxLength = I2C_CHANNEL1_MAX_LENGTH,
        .I2cChannelPriority = 5,
        .I2cChannelQueueSize = I2C_CHANNEL1_QUEUE_SIZE,
        .I2cJobEndNotification = NULL,
        .I2cSlaveRef = &I2cSlave[I2C_SLAVE_NAME1]
    },
    {
        .I2cChannelId = I2C_CH_NAME2,
        .I2cChannelMaxLength = I2C_CHANNEL2_MAX_LENGTH,
        .I2cChannelPriority = 1,
        .I2cChannelQueueSize = I2C_CHANNEL2_QUEUE_SIZE,
        .I2cJobEndNotification = NULL,
        .I2cSlaveRef = &I2cSlave[I2C_SLAVE_NAME2]
    },
    {
        .I2cChannelId = I2C_CH_NAME3,
        .I2cChannelMaxLength = I2C_CHANNEL3_MAX_LENGTH,
        .I2cChannelPriority = 4,
        .I2cChannelQueueSize = I2C_CHANNEL3_QUEUE_SIZE,
        .I2cJobEndNotification = NULL,
        .I2cSlaveRef = &I2cSlave[I2C_SLAVE_NAME3]
    },
    {
        .I2cChannelId = I2C_CH_NAME4,
        .I2cChannelMaxLength = I2C_CHANNEL4_MAX_LENGTH,
        .I2cChannelPriority = 2,
        .I2cChannelQueueSize = I2C_CHANNEL4_QUEUE_SIZE,
        .I2cJobEndNotification = NULL,
        .I2cSlaveRef = &I2cSlave[I2C_SLAVE_NAME4]
    },
    {
        .I2cChannelId = I2C_CH_NAME5,
        .I2cChannelMaxLength = I2C_CHANNEL5_MAX_LENGTH,
        .I2cChannelPriority = 0,
        .I2cChannelQueueSize = I2C_CHANNEL5_QUEUE_SIZE,
        .I2cJobEndNotification = NULL,
        .I2cSlaveRef = &I2cSlave[I2C_SLAVE_NAME5]
    }
};

/* channel groups, needed for the controller */
static const I2c_ChannelType* I2c_channelGroup1[I2C_CHANNEL_GROUP1_COUNT] =
{
    &I2cChannel[I2C_CH_NAME1],
    &I2cChannel[I2C_CH_NAME2]
};
static const I2c_ChannelType* I2c_channelGroup2[I2C_CHANNEL_GROUP2_COUNT] =
{
    &I2cChannel[I2C_CH_NAME3]
};
static const I2c_ChannelType* I2c_channelGroup3[I2C_CHANNEL_GROUP3_COUNT] =
{
    &I2cChannel[I2C_CH_NAME4]
};
static const I2c_ChannelType* I2c_channelGroup4[I2C_CHANNEL_GROUP4_COUNT] =
{
    &I2cChannel[I2C_CH_NAME5]
};


/* slave groups, needed for the controller */
static const I2c_SlaveType* I2c_SlaveGroup1[I2C_SLAVE_GROUP1_COUNT] =
{
    &I2cSlave[I2C_SLAVE_NAME1],
    &I2cSlave[I2C_SLAVE_NAME2]
};
static const I2c_SlaveType* I2c_SlaveGroup2[I2C_SLAVE_GROUP2_COUNT] =
{
    &I2cSlave[I2C_SLAVE_NAME3]
};
static const I2c_SlaveType* I2c_SlaveGroup3[I2C_SLAVE_GROUP3_COUNT] =
{
    &I2cSlave[I2C_SLAVE_NAME4]
};
static const I2c_SlaveType* I2c_SlaveGroup4[I2C_SLAVE_GROUP4_COUNT] =
{
    &I2cSlave[I2C_SLAVE_NAME5]
};

/* Configuration of the I2C controllers. */
static const I2c_ControllerType I2cController[I2C_CONTROLLER_COUNT] =
{
    {
        .I2cBusSpeed = 400u,
        .I2cControllerTimeout = 10000,
        .I2cChannel = I2c_channelGroup1,
        .I2cSlave = I2c_SlaveGroup1
    },
    {
        .I2cBusSpeed = 400u,
        .I2cControllerTimeout = 1000,
        .I2cChannel = I2c_channelGroup2,
        .I2cSlave = I2c_SlaveGroup2
    },
    {
        .I2cBusSpeed = 100u,
        .I2cControllerTimeout = 100,
        .I2cChannel = I2c_channelGroup3,
        .I2cSlave = I2c_SlaveGroup3
    },
    {
        .I2cBusSpeed = 100u,
        .I2cControllerTimeout = 2200,
        .I2cChannel = I2c_channelGroup4,
        .I2cSlave = I2c_SlaveGroup4
    }
};


/* This container contains the global configuration parameters and sub containers of the I2c module.*/
const I2c_ConfigSetType I2cConfigSet =
{
    .I2cController = I2cController
};

/* Configuration of the I2c (I2C Driver) module. */
const I2c_I2cType I2c =
{
    .I2cConfigSet = &I2cConfigSet,
    .I2cGeneral = &I2cGeneral
};

