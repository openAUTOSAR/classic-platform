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

#ifndef LIN_CONFIGTYPES_H
#define LIN_CONFIGTYPES_H

#include "Std_Types.h"

/* ERRATA for REV_A of 551x chip. Uncomment to include.
 * Will use a GPT timer for timeout handling */

//#define MPC551X_ERRATA_REV_A

typedef struct
{
    /* Switches the Development Error Detection and Notification
    ON or OFF. */
    boolean LinDevErrorDetect;

    /* Specifies the InstanceId of this module instance. If only one
    instance is present it shall have the Id 0. */
    uint8   LinIndex;

    /* Specifies the maximum number of loops for blocking function
     * until a timeout is raised in short term wait loops */
    uint16  LinTimeoutDuration;

    /* Switches the Lin_GetVersionInfo function ON or OFF. */
    boolean LinVersionInfoApi;
}
Lin_GeneralType;

typedef struct
{
    /* Specifies the baud rate of the LIN channel */
    uint16  LinChannelBaudRate;

    /* Specifies if the LIN hardware channel supports wake up functionality */
    boolean LinChannelWakeUpSupport;

    /* Specifies the LIN channel id */
    uint8   LinChannelId;

    /* This parameter contains a reference to the Wakeup Source
     * for this controller as defined in the ECU State Manager.
     * Implementation Type: reference to EcuM_WakeupSourceType */
    uint32  LinChannelEcuMWakeUpSource;

    /* Reference to the LIN clock source configuration, which is set
     *  in the MCU driver configuration.*/
    uint32  LinClockRef;

#ifdef MPC551X_ERRATA_REV_A
    /* Errata forces us to use a Gpt channel for timouts */
    uint8   LinTimeOutGptChannelId;
#endif
}
Lin_ChannelConfigType;


#endif
