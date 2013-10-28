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








#ifndef LIN_CFG_H_
#define LIN_CFG_H_

#include "Std_Types.h"

/* ERRATA for REV_A of 551x chip. Uncomment to include. Will use a GPT timer for timeout handling */
//#define MPC551X_ERRATA_REV_A

/* Switches the Development Error Detection and Notification
ON or OFF. */
#define LIN_DEV_ERROR_DETECT STD_ON

/* Specifies the InstanceId of this module instance. If only one
instance is present it shall have the Id 0. */
#define LIN_INDEX 0

/* Specifies the maximum number of loops for blocking function
 * until a timeout is raised in short term wait loops */
#define LIN_TIMEOUT_DURATION 10

/* Switches the Lin_GetVersionInfo function ON or OFF. */
#define LIN_VERSION_INFO_API STD_ON

typedef enum {
  LIN_CTRL_A = 0,
  LIN_CTRL_B,
  LIN_CTRL_C,
  LIN_CTRL_D,
  LIN_CONTROLLER_CNT
}LinControllerIdType;

typedef struct {
	/* Switches the Development Error Detection and Notification
	ON or OFF. */
	boolean LinDevErrorDetect;
	/* Specifies the InstanceId of this module instance. If only one
	instance is present it shall have the Id 0. */
	uint8 LinIndex;
	/* Specifies the maximum number of loops for blocking function
	 * until a timeout is raised in short term wait loops */
	uint16 LinTimeoutDuration;
	/* Switches the Lin_GetVersionInfo function ON or OFF. */
	boolean LinVersionInfoApi;
}Lin_GeneralType;

typedef struct {
	/* Specifies the baud rate of the LIN channel */
	uint16 LinChannelBaudRate;
	/* Identifies the LIN channel.*/
	uint8 LinChannelId;
	/* Specifies if the LIN hardware channel supports wake up functionality */
	boolean LinChannelWakeUpSupport;
	/* This parameter contains a reference to the Wakeup Source
	 * for this controller as defined in the ECU State Manager.
	 * Implementation Type: reference to EcuM_WakeupSourceType */
	uint32 LinChannelEcuMWakeUpSource;
	/* Reference to the LIN clock source configuration, which is set
	 * 	in the MCU driver configuration.*/
	uint32 LinClockRef;
#ifdef MPC551X_ERRATA_REV_A
	/* Errata forces us to use a Gpt channel for timouts */
	uint8 LinTimeOutGptChannelId;
#endif
} Lin_ChannelConfigType;


#endif /*LIN_CFG_H_*/
