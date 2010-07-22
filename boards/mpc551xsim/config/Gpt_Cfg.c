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
#if defined(USE_KERNEL)
   {
		.GptChannelId = GPT_CHANNEL_DEC,
		.GptChannelMode = GPT_MODE_CONTINUOUS,
		.GptChannelClkSrc = 0,
		.GptNotification = OsTick,
		.GptEnableWakeup = FALSE,
		.GptChannelPrescale = 0,
	},
#endif
   {
		// Last channel in list
		.GptChannelId = GPT_CHANNEL_ILL,
	}
};
