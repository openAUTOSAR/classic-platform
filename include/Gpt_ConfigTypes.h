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

#ifndef GPT_CONFIGTYPES_H
#define GPT_CONFIGTYPES_H

typedef struct  {
	uint32 GptChannelClkSrc;
	Gpt_ChannelType GptChannelId;
	Gpt_ChannelMode GptChannelMode;
	void (*GptNotification)();
	uint8 GptNotificationPriority;
	uint32 GptChannelPrescale;
	boolean GptEnableWakeup;		// ?
#if (GPT_REPORT_WAKEUP_SOURCE == STD_ON)
	EcuM_WakeupSourceType GptWakeupSource;
#endif
} Gpt_ConfigType;

#endif /* GPT_CONFIGTYPES_H */
