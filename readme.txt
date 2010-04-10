
OsTick

   GENERATED
   OsTickType OsTickFreq = 1000;

	
	SYSTICK SETUP
	{
		uint32_t sys_freq = McuE_GetSystemClock();
		Os_SysTickInit();
		Os_SysTickStart(sys_freq/OsTickFreq);
	}

USE CASE
   I want to setup a 4ms alarm?
   
   








