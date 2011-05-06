/*Those includes added by Cobb for test Port*/
#include "Std_Types.h"
#include "Modules.h"
#include "mpc55xx.h"
#include "Port.h"
/*************************END*************************/

void Port_Test(void)
{
	/************Tested by Cobb***************/
	volatile uint32_t i = 0; /* Dummy idle counter */
	Std_VersionInfoType versionInfo;

#if 1 /*It is just test Port.*/

	/*============Test_Port_Case0============*/
	/* If LED1...LED4 are light, which means the Port_Init succeed.*/
	//Port_Init(&PortConfigData);
	/*==================END======================*/

	/*============Test_Port_Case3============*/
	/*Turn off the LED1...LED4 = PE[4]...PE[7]
	 * If LED1...LED4 are turn off,which means the Port_SetPinMode succeed.*/
	Port_SetPinMode(66,0x0100);
	Port_SetPinMode(67,0x0100);
	Port_SetPinMode(68,0x0100);
	Port_SetPinMode(69,0x0100);
	/*Turn on the LED1...LED4 = PE[4]...PE[7]
	 * If LED1...LED4 are turn on,which means the Port_SetPinMode succeed.*/
	Port_SetPinMode(66,0x0200);
	Port_SetPinMode(67,0x0200);
	Port_SetPinMode(68,0x0200);
	Port_SetPinMode(69,0x0200);

	/*Test setting the normal pins mode.
	 * If you want to judge the function right or not,please check registers.*/
	Port_SetPinMode(76,0x0103);
	Port_SetPinMode(77,0x0103);
	Port_SetPinMode(78,0x0103);
	Port_SetPinMode(79,0x0103);
	/*==================END======================*/

	/*============Test_Port_Case1============*/
	/*Test the function of Port_SetPinDirection.
	 * If you want to judge the function right or not,please check registers.*/
	Port_SetPinDirection(76,PORT_PIN_OUT);
	Port_SetPinDirection(77,PORT_PIN_OUT);
	Port_SetPinDirection(78,PORT_PIN_OUT);
	Port_SetPinDirection(79,PORT_PIN_OUT);
	/*Turn off the LED1...LED4
	 * If LED1...LED4 are turn off,which means the Port_SetPinDirection succeed.*/
	Port_SetPinDirection(66,PORT_PIN_IN);
	Port_SetPinDirection(67,PORT_PIN_IN);
	Port_SetPinDirection(68,PORT_PIN_IN);
	Port_SetPinDirection(69,PORT_PIN_IN);
	/*Turn on the LED1...LED4
	 * If LED1...LED4 are turn on,which means the Port_SetPinDirection succeed.*/
	Port_SetPinDirection(66,PORT_PIN_OUT);
	Port_SetPinDirection(67,PORT_PIN_OUT);
	Port_SetPinDirection(68,PORT_PIN_OUT);
	Port_SetPinDirection(69,PORT_PIN_OUT);
	/*==================END======================*/

	/*The function couldn't been tested,or PH[0:3] should been excluded. Out of reset pins PH[0:3](PCR99~PCR102) are available as JTAG pins(TCK,TDI,TDO and TMS respectively)
	 * It is up to the user to configure pins PH[0:3]when need.*/
	/*============Test_Port_Case2============*/
	Port_RefreshPortDirection();
	/*==================END======================*/

	/*============Test_Port_Case4============*/
	/*Test the function of Port_GetVersionInfo*/
	Port_GetVersionInfo(&versionInfo);
	/*==================END======================*/

#endif

/****************END*******************/

}
