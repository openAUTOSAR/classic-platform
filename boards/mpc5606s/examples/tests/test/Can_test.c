#include "Can.h"

void delayms(int ms)
{
    int i,j;
    if (ms<1) ms=1;
    for(i=0;i<ms;i++)
        for(j=0;j<1335;j++);
}

void Test_Can(void)
{	
	uint8 i;
	Can_PduType canFrame = {0};
	uint8 TxData[6] = {0};

	for(i = 0; i < 6; i++)
	{
		TxData[i] = (i + 1);
	}

	canFrame.id = 0x22B;
	canFrame.length = 6;
	canFrame.sdu = TxData;
	canFrame.swPduHandle = 1;

	SIU.PSMI[0].R = 0x00;           /* MPC56xxS: Select PCR17 for CAN0RX input */
	SIU.PSMI[1].R = 0x00;

	Can_SetControllerMode(CAN_CTRL_A, CAN_T_START);
	Can_SetControllerMode(CAN_CTRL_B, CAN_T_START);

	while(1)
	{
		Can_Write(CAN_HTH_A_1, &canFrame);
		Can_Write(CAN_HTH_B_1, &canFrame);
		delayms(1000);
	}
}
