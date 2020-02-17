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
#ifndef I2C_H_
#define I2C_H_

#include "I2c_Cfg.h"

#define I2C_VENDOR_ID               60u
#define I2C_MODULE_ID               81u

#define I2C_AR_MAJOR_VERSION        1u
#define I2C_AR_MINOR_VERSION        1u
#define I2C_AR_PATCH_VERSION        1u

#define I2C_SW_MAJOR_VERSION        1u
#define I2C_SW_MINOR_VERSION        2u
#define I2C_SW_PATCH_VERSION        0u

/* @req SWS_I2c_00014 */
/* Service IDs */
enum
{
    I2C_SERVICE_ID_INIT = 1,
    I2C_SERVICE_ID_GETVERSION,
    I2C_SERVICE_ID_WRITE,
    I2C_SERVICE_ID_READ,
    I2C_SERVICE_ID_MAIN
};

/* Development Errors */
enum
{
    I2C_E_NO = 0,               /* no error detected */
    I2C_E_UNINIT,               /* An API function was called while the module was uninitialized */
    I2C_E_PARAM,                /* A parameter given to the API function was invalid.*/
    I2C_E_PARAM_POINTER,        /* NULL pointer has been passed as an argument. */
    I2C_E_CHANNEL_UNAVAILABLE,  /* An operation was requested on a channel that does not exist. */
    I2C_E_TIMEOUT               /* An I2C message could not be finalized due to a communication timeout. */
};

enum
{
    I2C_JOB_STATE_READY,
    I2C_JOB_STATE_START,
    I2C_JOB_STATE_TRANSMIT_REGISTER,
    I2C_JOB_STATE_TRANSMIT_DATA,
    I2C_JOB_STATE_RECEIVE_DATA,
    I2C_JOB_STATE_FINISHED,
    I2C_JOB_STATE_ERROR,
    I2C_JOB_STATE_TIMEOUT,
    I2C_JOB_STATE_NONE
};



/*  Function Prototypes */

/**
 * Initializes the I2c Driver
 * Service ID[hex]:  0x01
 * @param ConfigPtr - Pointer to configuration set
 */
void I2c_Init(const I2c_ConfigType* ConfigPtr);

/**
 * Returns the version information of this module.
 * Service ID[hex]:  0x02
 * @param versioninfo - Pointer to where to store the version information of this module
 */
#if (I2C_VERSION_INFO_API == STD_ON)
void I2c_GetVersionInfo(Std_VersionInfoType* versioninfo);
#endif

/**
 * Writing of data to a destination register on the specified I2C channel.
 * Service ID[hex]:  0x03
 * @param channel       - A handle to a valid I2cChannel instance
 * @param reg           - Destination register in the slave ECU.
 * @param registerSize  - address size of destination register (8 bit, 16 bit, etc.)
 * @param data          - source data to be transmitted to slave
 * @param length        - number of bytes to be written to destination
 * @param requestHandle -
 *                        request handle assigned to this request by I2c.
                          The same handle will be used within JobEndNotification, so that
                          the user can match the notification with the previous request.
 * @return              -  E_OK: Write job successfully issued.
                           E_NOT_OK: I2c didn't accept write request, e.g. channel queue is full.
 */
Std_ReturnType I2c_Write(I2c_ChannelHandleType channel,
                         uint32 reg,
                         I2c_AddressSizeType registerSize,
                         const uint8* data,
                         uint16 length,
                         uint16* requestHandle);

/**
 * Reading of data from a source register on the specified I2C channel.
 * Service ID[hex]:  0x04
 * @param channel       - A handle to a valid I2cChannel instance
 * @param reg           - Source register to be read in the slave ECU.
 * @param registerSize  - address size of source register (8 bit, 16 bit, etc.)
 * @param data          - pointer to destination buffer for data read from slave
 * @param length        - number of bytes to be read from slave
 * @param requestHandle - request handle assigned to this request by I2c.
                          The same handle will be used within JobEndNotification, so that
                          the user can match the notification with the previous request.
 * @return              -  E_OK: Write job successfully issued.
                           E_NOT_OK: I2c didn't accept write request, e.g. channel queue is full.
 */
Std_ReturnType I2c_Read(I2c_ChannelHandleType channel,
                        uint32 reg,
                        I2c_AddressSizeType registerSize,
                        uint8* data,
                        uint16 length,
                        uint16* requestHandle);

/**
 * Main function of the I2c module. May be used for scheduling and monitoring
 * I2c message transfer and timeout supervision.
 * Service ID[hex]:  0x05
 */
void I2c_MainFunction(void);


/* @req SWS_I2c_00012 */
/**
 * This callback notifies the I2c user on the completion of a previously issued job.
 * @param result        - Status of job execution:
 *                        E_OK: Job executed successfully
 *                        E_NOT_OK: An error occurred during job execution
 * @param requestHandle - request handle, identifies the previous request being notified.
 */
void User_I2cJobEndNotification(Std_ReturnType result, uint16 requestHandle);

/**
 * This function returns the actual job state
 * @return              - Status of job
 *                        I2C_JOB_STATE_NONE:  No job was ever started
 *                        I2C_JOB_STATE_START: A job is started
 *                        I2C_JOB_STATE_TRANSMIT_REGISTER: A job is transmitting register data
 *                        I2C_JOB_STATE_TRANSMIT_DATA: A job is transmitting data
 *                        I2C_JOB_STATE_RECEIVE_DATA: A job ist receiving data
 *                        I2C_JOB_STATE_READY: Job executed successfully or ready for the next transfer *
 */
uint8 I2c_GetActualJobState(I2c_ChannelHandleType channel);

/**
 * This function is required to be configured for a callback upon timeout of configured timer / GPT
 * @return              - void
 */
void I2c_JobTimeoutNotification(void);

#endif /* I2C_H_ */
