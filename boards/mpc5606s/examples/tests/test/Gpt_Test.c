#include "Gpt.h"
#include "Std_Types.h"

void Test_Gpt_Case0(void)
{
	Gpt_ValueType elapsed_value[4]= {0,0,0,0};
	Gpt_ValueType remaining_value[4]= {0,0,0,0};

	Gpt_EnableNotification( GPT_CHANNEL_PIT_0 );
	Gpt_StartTimer( GPT_CHANNEL_PIT_0, 0x300000 );

	Gpt_EnableNotification( GPT_CHANNEL_PIT_1 );
	Gpt_StartTimer( GPT_CHANNEL_PIT_1, 0x400000 );

	Gpt_EnableNotification( GPT_CHANNEL_PIT_2 );
	Gpt_StartTimer( GPT_CHANNEL_PIT_2, 0x500000 );

	Gpt_EnableNotification( GPT_CHANNEL_PIT_3 );
	Gpt_StartTimer( GPT_CHANNEL_PIT_3, 0x600000 );


	elapsed_value[0] = Gpt_GetTimeElapsed( GPT_CHANNEL_PIT_0 );
	remaining_value[0] = Gpt_GetTimeRemaining( GPT_CHANNEL_PIT_0 );

	elapsed_value[1] = Gpt_GetTimeElapsed( GPT_CHANNEL_PIT_1 );
	remaining_value[1] = Gpt_GetTimeRemaining( GPT_CHANNEL_PIT_1 );

	elapsed_value[2] = Gpt_GetTimeElapsed( GPT_CHANNEL_PIT_2 );
	remaining_value[2] = Gpt_GetTimeRemaining( GPT_CHANNEL_PIT_2 );

	elapsed_value[3] = Gpt_GetTimeElapsed( GPT_CHANNEL_PIT_3 );
	remaining_value[3] = Gpt_GetTimeRemaining( GPT_CHANNEL_PIT_3 );

	while(1) ;
}

void Test_Gpt_Case1(void)
{
	uint32 i = 0, j = 0;

	Gpt_EnableNotification( GPT_CHANNEL_PIT_0 );
	Gpt_StartTimer( GPT_CHANNEL_PIT_0, 0x300000 );

	for(i = 0; i <100; i++)
	{
		for(j = 0; j < 300000; j++) ;
	}

	Gpt_StopTimer( GPT_CHANNEL_PIT_0 );

	while(1) ;
}

void Test_Gpt_Case2(void)
{
	Gpt_EnableNotification( GPT_CHANNEL_PIT_1 );
	Gpt_StartTimer( GPT_CHANNEL_PIT_1, 0x300000 );

	Gpt_DisableNotification( GPT_CHANNEL_PIT_1 );

	while(1) ;

}
