/*Belows includes added by Cobb for test Port and Dio*/
#include "mpc55xx.h"
#include "Port.h"
#include "Dio.h"
/*************************END*************************/

void PortDio_Test(void)
{
	/************Tested by Cobb***************/
	volatile uint32_t i = 0,i1 = 0,i2 = 0,i3 = 0,i4 = 0; /* Dummy idle counter */
	Dio_PortLevelType temp = 0;
	Dio_LevelType temp1 = 0;
	Dio_PortLevelType temp_PL1 = 0;
	Dio_PortLevelType temp_PL2 = 0;
	Dio_PortLevelType temp_PL3 = 0;
	Dio_PortLevelType temp_PL4 = 0;
	Dio_PortLevelType temp_PL5 = 0;
	Std_VersionInfoType versionInfo;

#if 1 /*It is just test Port.*/
	Port_Init(&PortConfigData);

	/*LED1...LED4*/
	Port_SetPinMode(68,0x0200);
	Port_SetPinMode(69,0x0200);
	Port_SetPinMode(70,0x0200);
	Port_SetPinMode(71,0x0200);

	/*Test setting the normal pins mode.*/
	Port_SetPinMode(51,0x0103);
	Port_SetPinMode(52,0x0103);
	Port_SetPinMode(53,0x0103);
	Port_SetPinMode(54,0x0103);
	Port_SetPinMode(55,0x0103);

	/*Test the function of Port_SetPinDirection.*/
	Port_SetPinDirection(51,PORT_PIN_OUT);
	Port_SetPinDirection(52,PORT_PIN_OUT);
	Port_SetPinDirection(53,PORT_PIN_OUT);
	Port_SetPinDirection(54,PORT_PIN_OUT);
	Port_SetPinDirection(55,PORT_PIN_OUT);

	/*The function couldn't been tested. Out of reset pins PH[0:3](PCR99~PCR102) are available as JTAG pins(TCK,TDI,TDO and TMS respectively)
	 * It is up to the user to configure pins PH[0:3]when need.
	*/
	//Port_RefreshPortDirection();

	/*Test the function of Port_GetVersionInfo*/
	Port_GetVersionInfo(&versionInfo);

#endif

#if 1/*It is just test Dio.*/

	/*Test the function Dio_WriteChannel and Dio_ReadChannel*/
	Dio_WriteChannel(4,1);
	temp1 = Dio_ReadChannel(4);
	Dio_WriteChannel(5,1);
	temp1 = Dio_ReadChannel(5);
	Dio_WriteChannel(6,1);
	temp1 = Dio_ReadChannel(6);
	Dio_WriteChannel(7,1);
	temp1 = Dio_ReadChannel(7);

	/*Test the function of Dio_WritePort and Dio_ReadPort.*/
	Dio_WritePort(0,0xabcd);
	temp = Dio_ReadPort(0);
	Dio_WritePort(0,0xacbd);
	temp = Dio_ReadPort(0);
	Dio_WritePort(0,0xdbca);
	temp = Dio_ReadPort(0);
	Dio_WritePort(0,0xbacd);
	temp = Dio_ReadPort(0);
	Dio_WritePort(0,0xabdc);
	temp = Dio_ReadPort(0);

	/*Test the function Dio_WriteChannelGroup and Dio_ReadChannelGroup.*/
	Dio_WriteChannelGroup(&DioConfigData[0],0xff01);
	temp_PL1 = Dio_ReadChannelGroup(&DioConfigData[0]);

	Dio_WriteChannelGroup(&DioConfigData[0],0xdcba);
	temp_PL2 = Dio_ReadChannelGroup(&DioConfigData[0]);

	Dio_WriteChannelGroup(&DioConfigData[0],0xcdab);
	temp_PL3 = Dio_ReadChannelGroup(&DioConfigData[0]);

	Dio_WriteChannelGroup(&DioConfigData[0],0xdddc);
	temp_PL4 = Dio_ReadChannelGroup(&DioConfigData[0]);

	Dio_WriteChannelGroup(&DioConfigData[0],0xaded);
	temp_PL5 = Dio_ReadChannelGroup(&DioConfigData[0]);

#endif


/****************END*******************/

}
