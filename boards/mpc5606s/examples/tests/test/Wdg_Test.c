#include "Wdg.h"
#include "Std_Types.h"
#include "mpc55xx.h"



void Test_Wdg_Case0(void)
{
	uint32 i = 0;
	uint32 j = 0;

	SWT.TO.R = 0x7D00;
	StartWatchdog();

	for(i = 0; i < 100; i++)
	{
		for(j = 0; j < 300000; j++) ;
		KickWatchdog();
		SIU.GPDO[68].B.PDO = ~SIU.GPDO[68].B.PDO;

	}

	StopWatchdog();

	while(1) ;
}


void Test_Wdg_Case1(void)
{

	StartWatchdog();

	while(1) ;
}

