#include "Gpt.h"


void Test_Gpt(void)
{
	Gpt_ValueType value_1,value_2;

	Gpt_DisableNotification( GPT_CHANNEL_PIT_1 );

	Gpt_EnableNotification( GPT_CHANNEL_PIT_0 );



	Gpt_StartTimer( GPT_CHANNEL_PIT_0, 0x300000 );



	value_1 = Gpt_GetTimeElapsed( GPT_CHANNEL_PIT_0 );

	value_2 = Gpt_GetTimeRemaining( GPT_CHANNEL_PIT_0 );
	
	//Gpt_StopTimer( GPT_CHANNEL_PIT_0);

	while(1);

}
