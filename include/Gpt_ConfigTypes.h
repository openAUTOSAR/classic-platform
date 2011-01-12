/*
 * Gpt_ConfigTypes.h
 *
 *  Created on: 13 dec 2010
 *      Author: Niclas
 */

#ifndef GPT_CONFIGTYPES_H_
#define GPT_CONFIGTYPES_H_

typedef struct  {
	uint32 GptChannelClkSrc;
	Gpt_ChannelType GptChannelId;
	Gpt_ChannelMode GptChannelMode;
	void (*GptNotification)();
	uint8 GptNotificationPriority;
	uint32 GptChannelPrescale;
	boolean GptEnableWakeup;
} Gpt_ConfigType;

#endif /* GPT_CONFIGTYPES_H_ */
