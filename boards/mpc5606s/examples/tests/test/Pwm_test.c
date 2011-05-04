#include "Pwm.h"


void Test_Pwm(void)
{
	Pwm_SetDutyCycle(PWM_CHANNEL_7,0x2000);
	Pwm_SetPeriodAndDuty( PWM_CHANNEL_2, 5000, 0x2000 );
	while(1);
	Pwm_DeInit();

}
