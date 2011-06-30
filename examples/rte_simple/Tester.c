/*
 * Tester.c
 *
 *  Created on: 16 jun 2011
 *      Author: maek
 */

#include "Rte_Tester.h"
#include "Os.h"

void TesterRunnable() {
	UInt8 arg1 = Rte_IRead_TesterRunnable_Arguments_arg1();
	UInt8 arg2 = Rte_IRead_TesterRunnable_Arguments_arg2();
	UInt16 result = 0;

	Std_ReturnType retVal = Rte_Call_Tester_Calculator_Multiply(arg1, arg2, &result);
	if (retVal == RTE_E_OK) {
		Rte_IWrite_TesterRunnable_Result_result(result);
	} else {
		Rte_IWrite_TesterRunnable_Result_result(0);
	}
}

void FreqReqRunnable() {
	// Get frequency from COM stack.
	UInt32 freqReq = Rte_IRead_FreqReqRunnable_FreqReq_freq();

	// Set the alarm that triggers the blinker.
	CancelAlarm(ALARM_ID_BlinkerAlarm);
	SetRelAlarm(ALARM_ID_BlinkerAlarm, 1, freqReq);

	// Echo the frequency back to COM stack.
	Rte_IWrite_FreqReqRunnable_FreqReqInd_freq(freqReq);
}
