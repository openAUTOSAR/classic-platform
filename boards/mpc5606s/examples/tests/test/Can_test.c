#include "Can.h"

void delayms(int ms){
    int i,j;
    if (ms<1) ms=1;
    for(i=0;i<ms;i++)
        for(j=0;j<1335;j++) ;
}

void Test_Can_Callback(uint8 Hrh, Can_IdType CanId, uint8 CanDlc, const uint8 *CanSduPtr ){
	int i;
	/* *Callback function for receive can massage'ISR of CAN
	 *  CanId = 0x80000000 Extended Frame
	 * 	LENGTH = 8
	 *  DATA = 0001020304050607
	 *  CanIdType = CAN_ID_TYPE_STANDARD
	 * */
	while(Hrh !=(1)) ;
	while(CanId != 0x80000000) ;				//canHw->BUF[0].ID.B.EXT_ID
	while(CanDlc != 0x08) ; 				//canHw->BUF[0].CS.B.LENGTH
	for (i = 0; i < CanDlc; i++){
	while(*(CanSduPtr+i) != i) ;			//(uint8 *)&canHw->BUF[0].DATA.W[0]
	}
}

void Test_Can_Case0(void){
	/* *TEST for receive can massage'ISR of CAN
	 *  CanId = 0x80000000 Extended Frame
	 * 	LENGTH = 8
	 *  DATA = 0001020304050607
	 *  CanIdType = CAN_ID_TYPE_STANDARD
	 * */
	Can_SetControllerMode(CAN_CTRL_A, CAN_T_START);
	Can_SetControllerMode(CAN_CTRL_B, CAN_T_START);

	while(1) ;
}

void Test_Can_Case1(void){
	/* *TEST for Can_SetControllerMode
	 * */
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


	Can_SetControllerMode(CAN_CTRL_A, CAN_T_START);
	Can_Write(CAN_HTH_A_1, &canFrame);

	Can_SetControllerMode(CAN_CTRL_B, CAN_T_START);
	Can_Write(CAN_HTH_B_1, &canFrame);

	delayms(10);

	Can_SetControllerMode(CAN_CTRL_A, CAN_T_STOP);
	Can_Write(CAN_HTH_A_1, &canFrame);

	Can_SetControllerMode(CAN_CTRL_B, CAN_T_STOP);
	Can_Write(CAN_HTH_B_1, &canFrame);

	delayms(10);
}

void Test_Can_Case2(void){
	/* *TEST for Can_Write
	 * */
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

	Can_SetControllerMode(CAN_CTRL_A, CAN_T_START);
	Can_SetControllerMode(CAN_CTRL_B, CAN_T_START);

	while(1){
	Can_Write(CAN_HTH_A_1, &canFrame);
	Can_Write(CAN_HTH_B_1, &canFrame);
	}
}
