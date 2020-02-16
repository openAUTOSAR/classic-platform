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

/** @addtogroup Lin LIN Driver
 *  @{ */

/** @file Lin.h
 * API and type definitions for LIN Driver.
 */

#ifndef LIN_H_
#define LIN_H_

#define LIN_MODULE_ID            MODULE_ID_LIN
#define LIN_VENDOR_ID            VENDOR_ID_ARCCORE

#define LIN_SW_MAJOR_VERSION            2
#define LIN_SW_MINOR_VERSION   	        0
#define LIN_SW_PATCH_VERSION            0
#define LIN_AR_RELEASE_MAJOR_VERSION    4
#define LIN_AR_RELEASE_MINOR_VERSION    0
#define LIN_AR_RELEASE_PATCH_VERSION    3

#include "Std_Types.h"
#include "Lin_Cfg.h"
#include "ComStack_Types.h"

#if (LIN_VERSION_INFO_API == STD_ON)
void Lin_GetVersionInfo( Std_VersionInfoType *versionInfo );
#define Lin_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,LIN)
#endif

/* This is the type of the external data structure containing the overall
 * initialization data for the LIN driver and SFR settings affecting all
 * LIN channels. A pointer to such a structure is provided to the LIN driver
 * initialization routine for configuration of the driver and LIN hardware unit. */
typedef struct {
    const Lin_ChannelConfigType *LinChannelConfig;
    const uint8 *Lin_HwId2ChannelMap;
} Lin_ConfigType;

/** Represents all valid protected Identifier used by Lin_SendFrame(). */
typedef uint8 Lin_FramePidType;

/** This type is used to specify the Checksum model to be used for the LIN Frame. */
typedef enum {
	LIN_ENHANCED_CS,
	LIN_CLASSIC_CS,
} Lin_FrameCsModelType;

/** This type is used to specify whether the frame processor is required to transmit the
 *  response part of the LIN frame. */
typedef enum {
	/** Response is generated from this (master) node */
	LIN_MASTER_RESPONSE=0,
	/** Response is generated from a remote slave node */
	LIN_SLAVE_RESPONSE,
	/** Response is generated from one slave to another slave,
	 *  for the master the response will be anonymous, it does not
	 *  have to receive the response. */
	IN_SLAVE_TO_SLAVE,

} Lin_FrameResponseType;

/** This type is used to specify the number of SDU data bytes to copy. */
typedef uint8 Lin_FrameDIType;

/** This Type is used to provide PID, checksum model, data length and SDU pointer
 *  from the LIN Interface to the LIN driver. */
typedef struct {
	Lin_FrameCsModelType Cs;
	Lin_FramePidType  Pid;
	uint8* SduPtr;
	Lin_FrameDIType DI;
	Lin_FrameResponseType Drc;
} Lin_PduType;

typedef enum {
	LIN_UNINIT,
	LIN_INIT,
}Lin_DriverStatusType;

typedef enum {
	/** LIN frame operation return value.
	 *  Development or production error occurred */
	LIN_NOT_OK,

	/** LIN frame operation return value.
	 *  Successful transmission. */
	LIN_TX_OK,

	/** LIN frame operation return value.
	 *  Ongoing transmission (Header or Response). */
	LIN_TX_BUSY,

	/** LIN frame operation return value.
	 *  Erroneous header transmission such as:
	 *  - Mismatch between sent and read back data
	 *  - Identifier parity error or
	 *  - Physical bus error */
	LIN_TX_HEADER_ERRORLIN,

	/** LIN frame operation return value.
	 *  Erroneous response transmission such as:
	 *  - Mismatch between sent and read back data
	 *  - Physical bus error */
	LIN_TX_ERROR,

	/** LIN frame operation return value.
	 *  Reception of correct response. */
	LIN_RX_OK,

	/** LIN frame operation return value. Ongoing reception: at
	 *  least one response byte has been received, but the
	 *  checksum byte has not been received. */
	LIN_RX_BUSY,

	/** LIN frame operation return value.
	 *  Erroneous response reception such as:
	 *  - Framing error
	 *  - Overrun error
	 *  - Checksum error or
	 *  - Short response */
	LIN_RX_ERROR,


	/** LIN frame operation return value.
	 *  No response byte has been received so far. */
	LIN_RX_NO_RESPONSE,

	/** LIN channel state return value.
	 *  LIN channel not initialized. */
	LIN_CH_UNINIT,

	/** LIN channel state return value.
	 *  Normal operation; the related LIN channel is ready to
	 *  transmit next header. No data from previous frame
	 *  available (e.g. after initialization) */
	LIN_CH_OPERATIONAL,

	/** LIN channel state return value.
	 *  Sleep mode operation; in this mode wake-up detection
	 *  from slave nodes is enabled. */
	LIN_CH_SLEEP

} Lin_StatusType;

/** @name Service id's */
//@{
#define LIN_INIT_SERVICE_ID               0x00
#define LIN_GETVERSIONINFO_SERVICE_ID     0x01
#define LIN_WAKEUPVALIDATION_SERVICE_ID   0x0A
#define LIN_INIT_CHANNEL_SERVICE_ID       0x02
#define LIN_DEINIT_CHANNEL_SERVICE_ID     0x03
#define LIN_SEND_FRAME_SERVICE_ID         0x04
#define LIN_SEND_RESPONSE_SERVICE_ID      0x05
#define LIN_GO_TO_SLEEP_SERVICE_ID        0x06
#define LIN_WAKE_UP_SERVICE_ID            0x07
#define LIN_GETSTATUS_SERVICE_ID          0x08
#define LIN_GO_TO_SLEEP_INTERNAL_SERVICE_ID 0x09
//@}

/** @name Error Codes */
//@{
#define LIN_E_UNINIT                    0x00
#define LIN_E_CHANNEL_UNINIT			0x01
#define LIN_E_INVALID_CHANNEL			0x02
#define LIN_E_INVALID_POINTER			0x03
#define LIN_E_PARAM_POINTER             0x05
#define LIN_E_STATE_TRANSITION			0x04
//@}

void Lin_Init( const Lin_ConfigType* Config );

void Lin_DeInit();

void Lin_WakeupValidation( void );


Std_ReturnType Lin_SendFrame(  uint8 Channel,  Lin_PduType* PduInfoPtr );

Std_ReturnType Lin_GoToSleep(  uint8 Channel );

Std_ReturnType Lin_GoToSleepInternal(  uint8 Channel );

Std_ReturnType Lin_WakeUp( uint8 Channel );

Lin_StatusType Lin_GetStatus( uint8 Channel, uint8** Lin_SduPtr );

#endif
/** @} */




