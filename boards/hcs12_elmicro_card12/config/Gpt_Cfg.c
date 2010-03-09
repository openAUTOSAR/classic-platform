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









#include "Gpt.h"
#include "Gpt_Cfg.h"
#include <stdlib.h>

#if defined(USE_KERNEL)
extern void OsTick( void );
#endif

const Gpt_ConfigType GptConfigData[] =
{
  {
    .GptChannelId = GPT_CHANNEL_0,
    .GptChannelMode = GPT_MODE_CONTINUOUS,
    .GptChannelClkSrc = 0,
    .GptNotification = NULL,
    .GptEnableWakeup = TRUE,
  },{
    .GptChannelId = GPT_CHANNEL_1,
    .GptChannelMode = GPT_MODE_CONTINUOUS,
    .GptChannelClkSrc = 0,
    .GptNotification = NULL,
    .GptEnableWakeup = FALSE,
  },{
		.GptChannelId = GPT_CHANNEL_2,
		.GptChannelMode = GPT_MODE_ONESHOT,
		.GptChannelClkSrc = 0,
		.GptNotification = NULL,
		.GptEnableWakeup = FALSE,
  },{
    .GptChannelId = GPT_CHANNEL_3,
    .GptChannelMode = GPT_MODE_CONTINUOUS,
    .GptChannelClkSrc = 0,
    .GptNotification = NULL,
    .GptEnableWakeup = FALSE,
  },{
    .GptChannelId = GPT_CHANNEL_4,
    .GptChannelMode = GPT_MODE_ONESHOT,
    .GptChannelClkSrc = 0,
    .GptNotification = NULL,
    .GptEnableWakeup = FALSE,
  },{
    .GptChannelId = GPT_CHANNEL_5,
    .GptChannelMode = GPT_MODE_CONTINUOUS,
    .GptChannelClkSrc = 0,
    .GptNotification = NULL,
    .GptEnableWakeup = FALSE,
  },{
    .GptChannelId = GPT_CHANNEL_6,
    .GptChannelMode = GPT_MODE_ONESHOT,
    .GptChannelClkSrc = 0,
    .GptNotification = NULL,
    .GptEnableWakeup = FALSE,
  },{
    .GptChannelId = GPT_CHANNEL_7,
    .GptChannelMode = GPT_MODE_CONTINUOUS,
    .GptChannelClkSrc = 0,
    .GptNotification = NULL,
    .GptEnableWakeup = FALSE,
  },{
    .GptChannelId = GPT_CHANNEL_8,
    .GptChannelMode = GPT_MODE_ONESHOT,
    .GptChannelClkSrc = 0,
    .GptNotification = NULL,
    .GptEnableWakeup = FALSE,
  },
  {
	// Last channel in list
	.GptChannelId = GPT_CHANNEL_ILL,
  }
};
