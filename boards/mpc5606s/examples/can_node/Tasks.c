/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/


#include "Os.h"

#include "EcuM.h"
#include "Com.h"
#include "CanIf.h"

//#define USE_LDEBUG_PRINTF // Uncomment this to turn debug statements on.
#include "debug.h"


void OsIdle( void ) {
	for(;;) ;
}

/*
 * This basic task should execute every 10 OS ticks
 */

void bTask10( void ) {

	TerminateTask();
}

uint16_t RxPdu1_Signal1_buf = 0;
uint16_t RxPdu2_Signal1_buf = 0;
uint16_t TxPdu1_Signal1_buf = 0;
uint16_t TxPdu2_Signal1_buf = 0;

/*
 * This basic task should execute every 25 OS ticks
 */
void bTask25( void ) {

	Com_MainFunctionRx();
	Com_MainFunctionTx();


	Com_ReceiveSignal( RxPdu1_Signal1, (void*)&RxPdu1_Signal1_buf );
	Com_ReceiveSignal( RxPdu2_Signal1, (void*)&RxPdu2_Signal1_buf );

	uint16_t TxPdu1_Signal1_buf = RxPdu1_Signal1_buf;
	uint16_t TxPdu2_Signal1_buf = RxPdu2_Signal1_buf;

	Com_SendSignal( TxPdu1_Signal1, (void*)&TxPdu1_Signal1_buf );
	Com_SendSignal( TxPdu2_Signal1, (void*)&TxPdu2_Signal1_buf );


	TerminateTask();
}

/*
 * This basic task should execute every 100 OS ticks
 */

void bTask100( void ) {

	TerminateTask();
}
/*
 * This is the startup task. It is activated once immediately after the OS i started.
 */
void Startup( void ) {

	// Call second phase of startup sequence.
	EcuM_StartupTwo();

	CanIf_InitController( CANIF_Channel_1, CANIF_Channel_1_CONFIG_0 );
	CanIf_SetControllerMode( CANIF_Channel_1, CANIF_CS_STARTED );

	Com_IpduGroupStart(ComPduGroup, true);

	/*
	 * Activate scheduled tasks. OS tick is 1ms.
	 * The Blink is run every 25 ms with an offset of 25ms.
	 */
	SetRelAlarm(ALARM_ID_alarm10, 10, 10);
	SetRelAlarm(ALARM_ID_alarm25, 25, 1000);
	SetRelAlarm(ALARM_ID_alarm100, 100, 100);

	// End of startup_task().
	TerminateTask();
}


