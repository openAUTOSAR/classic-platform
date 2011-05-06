/*Those includes added by Cobb for test Dio*/
#include "Std_Types.h"
#include "Modules.h"
#include "mpc55xx.h"
#include "Dio.h"
/*************************END*************************/

void Dio_Test(void)
{
	/************Tested by Cobb***************/
	volatile uint32_t i = 0; /* Dummy idle counter */
	Dio_PortLevelType temp = 0;
	Dio_LevelType temp1 = 0;
	Dio_PortLevelType temp_PL1 = 0;
	Dio_PortLevelType temp_PL2 = 0;
	Dio_PortLevelType temp_PL3 = 0;
	Dio_PortLevelType temp_PL4 = 0;
	Dio_PortLevelType temp_PL5 = 0;
	Std_VersionInfoType versionInfo;

#if 1/*It is just test Dio.*/

	/*============Test_Dio_Case0============*/
	/*Test the function Dio_WriteChannel and Dio_ReadChannel*/
	Dio_WriteChannel(DIO_CHANNEL_A4,1);
	temp1 = Dio_ReadChannel(DIO_CHANNEL_A4);
	Dio_WriteChannel(DIO_CHANNEL_A4,0);
	temp1 = Dio_ReadChannel(DIO_CHANNEL_A4);
	Dio_WriteChannel(DIO_CHANNEL_A5,1);
	temp1 = Dio_ReadChannel(DIO_CHANNEL_A5);
	Dio_WriteChannel(DIO_CHANNEL_A5,0);
	temp1 = Dio_ReadChannel(DIO_CHANNEL_A5);
	Dio_WriteChannel(DIO_CHANNEL_A6,1);
	temp1 = Dio_ReadChannel(DIO_CHANNEL_A6);
	Dio_WriteChannel(DIO_CHANNEL_A6,0);
	temp1 = Dio_ReadChannel(DIO_CHANNEL_A6);
	Dio_WriteChannel(DIO_CHANNEL_A7,1);
	temp1 = Dio_ReadChannel(DIO_CHANNEL_A7);
	Dio_WriteChannel(DIO_CHANNEL_A7,0);
	temp1 = Dio_ReadChannel(DIO_CHANNEL_A7);
	/*==================END======================*/

	/*============Test_Dio_Case1============*/
	/*Test the function of Dio_WritePort and Dio_ReadPort.*/
	Dio_WritePort(DIO_PORT_A,0xabcd);
	temp = Dio_ReadPort(DIO_PORT_A);
	Dio_WritePort(DIO_PORT_A,0xacbd);
	temp = Dio_ReadPort(DIO_PORT_A);
	Dio_WritePort(DIO_PORT_A,0xdbca);
	temp = Dio_ReadPort(DIO_PORT_A);
	Dio_WritePort(DIO_PORT_A,0xbacd);
	temp = Dio_ReadPort(DIO_PORT_A);
	Dio_WritePort(DIO_PORT_A,0xabdc);
	temp = Dio_ReadPort(DIO_PORT_A);
	/*==================END======================*/

	/*============Test_Dio_Case2============*/
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
	/*==================END======================*/

	/*============Test_Dio_Case3============*/
	/*Test the function of Dio_GetVersionInfo*/
	Dio_GetVersionInfo(&versionInfo);
	/*==================END======================*/

#endif


/****************END*******************/

}
