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

/**
 * @file LinSM_ConfigTypes.h Configuration types for LinSM.
 *
 * This file defines the LinSm type LinSM_ConfigType and all other needed types.
 * They reflect the structure of the configuration parameters described in the
 * LinSM specification and their relations.
 * These types are used in the generated configuration files for LinSM.
 */

#ifndef LINSM_CONFIGTYPES_H_
#define LINSM_CONFIGTYPES_H_

#include "Std_Types.h"
#include "LinIf_Types.h"


/*-----------[LinSMSchedule]--------------------------------------------------*/

/* Stores the configuration data of a LinSMSchedule object.
 * See item LINSM146_Conf. */
typedef struct
{
    /* LinIf schedule table identifier.
     * The value of the LinSMScheduleIndex shall be the same
     * as the value from the LinIf. */

    const LinIf_SchHandleType LinSMScheduleIndex;
}
LinSM_ScheduleType;


/*-----------[LinSMChannel]---------------------------------------------------*/

/* Stores the configuration data of a LinSMChannel object.
 * See item LINSM142_Conf. */
typedef struct
{
    /* Timeout in milliseconds for the goto sleep and wakeup calls to LinIf. */
    uint32  LinSMConfirmationTimeout;

    boolean LinSMSleepSupport;

    /* Selects STANDBY (true) or SLEEP (false) transceiver mode when
     * entering LINSM_NO_COM. */
    boolean LinSMTransceiverPassiveMode;

    /* Unique handle to identify one certain LIN network.
     * The network index may be used directly to the LinIf module APIs. */
    uint16  LinSMComMNetworkHandleRef;

    /* List of LinSMSchedule objects assigned to this channel. */
    const LinSM_ScheduleType *LinSMSchedules;

    /* Number of LinSMSchedule (size of the array LinSMSchedule) */
    const uint8 LinSMSchedule_Cnt;
}
LinSM_ChannelType;


/*-----------[LinSM_ConfigType]-----------------------------------------------*/

/* Configuration of the LinSm module.
 * See item LINSM0221 */
typedef struct
{
    /* Array of setup LinSMChannel objects */
    const LinSM_ChannelType *LinSMChannels;

    /* Number of setup LinSMChannel objects (size of array LinSMChannels) */
    const uint32             LinSMChannels_Size;
}
LinSM_ConfigType;


#endif // LINSM_CONFIGTYPES_H_
