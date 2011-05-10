#include "Pwm.h"

void Test_Pwm_Case0()
{
	Pwm_SetDutyCycle(PWM_CHANNEL_13,0x1000);
	Pwm_SetDutyCycle(PWM_CHANNEL_14,0x4000);
	Pwm_SetDutyCycle(PWM_CHANNEL_15,0x0000);
	Pwm_SetDutyCycle(PWM_CHANNEL_16,0x8000);
	while(1) ;
}

void Test_Pwm_Case1()
{
	Pwm_SetPeriodAndDuty( PWM_CHANNEL_13, 500,   0x0000 );
	Pwm_SetPeriodAndDuty( PWM_CHANNEL_14, 60000, 0x8000 );
	Pwm_SetPeriodAndDuty( PWM_CHANNEL_15, 200,   0x5000 );
	Pwm_SetPeriodAndDuty( PWM_CHANNEL_16, 5000,  0x2000 );
	while(1) ;
}

void Test_Pwm_Case2()
{
	uint32 i = 0, j = 0;
	for(i = 0; i <1000; i++)
	{
		for(j = 0; j < 3000; j++) ;
	}

	Pwm_SetOutputToIdle(PWM_CHANNEL_13);
	Pwm_SetOutputToIdle(PWM_CHANNEL_14);
	Pwm_SetOutputToIdle(PWM_CHANNEL_15);
	Pwm_SetOutputToIdle(PWM_CHANNEL_16);
	while(1) ;
}

void Test_Pwm_Case3()
{
	Pwm_OutputStateType Output1=0,Output2=0,Output3=0,Output4=0;
	while(1)
	{
		Output1 = Pwm_GetOutputState(PWM_CHANNEL_13);
		Output2 = Pwm_GetOutputState(PWM_CHANNEL_14);
		Output3 = Pwm_GetOutputState(PWM_CHANNEL_15);
		Output4 = Pwm_GetOutputState(PWM_CHANNEL_16);
		Dio_WriteChannel(66,Output1);
		Dio_WriteChannel(67,Output2);
		Dio_WriteChannel(68,Output3);
		Dio_WriteChannel(69,Output4);
	}
}
void Test_Pwm_Case4()
{
	uint32 i = 0, j = 0;
	for(i = 0; i <1000; i++)
	{
		for(j = 0; j < 30000; j++) ;
	}

	Pwm_SetOutputToIdle(PWM_CHANNEL_13);
	Pwm_SetOutputToIdle(PWM_CHANNEL_14);
	Pwm_SetOutputToIdle(PWM_CHANNEL_15);
	Pwm_SetOutputToIdle(PWM_CHANNEL_16);
	for(i = 0; i <1000; i++)
		{
			for(j = 0; j < 30000; j++) ;
	}
	    Pwm_SetPeriodAndDuty( PWM_CHANNEL_13, 500,   0x0000 );
		Pwm_SetPeriodAndDuty( PWM_CHANNEL_14, 60000, 0x8000 );
		Pwm_SetPeriodAndDuty( PWM_CHANNEL_15, 200,   0x5000 );
		Pwm_SetPeriodAndDuty( PWM_CHANNEL_16, 5000,  0x2000 );
	while(1) ;
}
void Test_Pwm_Case5()
{
	uint32 i = 0, j = 0;
	for(i = 0; i <1000; i++)
	{
		for(j = 0; j < 30000; j++) ;
	}

	Pwm_SetOutputToIdle(PWM_CHANNEL_13);
	Pwm_SetOutputToIdle(PWM_CHANNEL_14);
	Pwm_SetOutputToIdle(PWM_CHANNEL_15);
	Pwm_SetOutputToIdle(PWM_CHANNEL_16);
	for(i = 0; i <1000; i++)
		{
			for(j = 0; j < 30000; j++) ;
	}
	Pwm_SetDutyCycle(PWM_CHANNEL_13,0x1000);
	Pwm_SetDutyCycle(PWM_CHANNEL_14,0x4000);
	Pwm_SetDutyCycle(PWM_CHANNEL_15,0x5000);
	Pwm_SetDutyCycle(PWM_CHANNEL_16,0x6000);
	while(1) ;
}
