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








#warning "This default file may only be used as an example!"

#ifndef GPT_CFG_H_
#define GPT_CFG_H_
#include "Std_Types.h"


// Pre-compile only
#define GPT_VARIANT_PC	STD_OFF
// Mix of pre-compile and post-build
#define GPT_VARIANT_PB	STD_ON

//#define DEC_TEST
//#define GPT_TEST

/* Std PIT channels */
#define GPT_CHANNEL_RTI	0
#define GPT_CHANNEL_PIT_0	GPT_CHANNEL_RTI
#define GPT_CHANNEL_PIT_1	1
#define GPT_CHANNEL_PIT_2	2
#define GPT_CHANNEL_PIT_3	3
#define GPT_CHANNEL_PIT_4	4
#define GPT_CHANNEL_PIT_5	5
#define GPT_CHANNEL_PIT_6	6
#define GPT_CHANNEL_PIT_7	7
#define GPT_CHANNEL_PIT_8	8

#define GPT_PIT_CNT		(GPT_CHANNEL_PIT_8 + 1)

/* Mcu channels */
#define GPT_CHANNEL_DEC	9

#define GPT_CHANNEL_CNT	(GPT_CHANNEL_DEC+1)

// Illegal channel
#define GPT_CHANNEL_ILL	31


#define GPT_DEV_ERROR_DETECT		STD_ON
// Enables/Disables wakeup source reporting
#define GPT_REPORT_WAKEUP_SOURCE	STD_OFF

#define GPT_DEINIT_API							STD_ON
#define GPT_ENABLE_DISABLE_NOTIFICATION_API	STD_ON
#define GPT_TIME_REMAINING_API					STD_ON
#define GPT_TIME_ELAPSED_API                    STD_ON
#define GPT_VERSION_INFO_API					STD_ON
// TODO: EcuM things missing to get this API working properly
#define GPT_WAKEUP_FUNCTIONALITY_API			STD_OFF


// This container contains the channel-wide configuration (parameters) of the
// GPT Driver
typedef struct  {
	// GPT187: The GPT module specific clock input for the timer unit can
	// statically be configured and allows  to select different clock sources
	// (external clock, internal GPT specific clock) per channel
	uint32 GptChannelClkSrc;

	//	Channel Id of the GPT channel. This value will be assigned to the symbolic
	//	name derived of the GptChannelConfiguration container short name.
	Gpt_ChannelType GptChannelId;

	// Specifies the behaviour of the timerchannel after the timeout has expired
	Gpt_ChannelMode GptChannelMode;

	// Function pointer to callback function
	void (*GptNotification)();

	// GPT module specific prescaler factor per channel
	uint32 GptChannelPrescale;

	//  GPT188: Enables wakeup capability of CPU for a channel when timeout
	// period expires. This might be different to enabling the notification
	// depending on hardware capabilities
	boolean GptEnableWakeup;
} Gpt_ConfigType;

extern const Gpt_ConfigType GptConfigData[];

#endif /*GPT_CFG_H_*/
