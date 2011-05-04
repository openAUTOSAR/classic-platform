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
#include "Mcu.h"
#include "arc.h"
#include "EcuM.h"

//#define USE_LDEBUG_PRINTF // Uncomment this to turn debug statements on.
#include "debug.h"

// How many errors to keep in error log.
#define ERROR_LOG_SIZE 20
int i;
int k = 0;
/**
 * Just an example of a basic task.
 */

void btask_3( void ) {
	StackInfoType si;
	TaskType currTask;
	LDEBUG_PRINTF("[%08u] btask_3 start\n", (unsigned)GetOsTick() );

	GetTaskID(&currTask);
	Os_Arc_GetStackInfo(currTask,&si);
	LDEBUG_PRINTF("btask_3: %u%% stack usage\n",
			(unsigned)OS_STACK_USAGE(&si));

	TerminateTask();
}

/**
 * An extended task is auto-started and is also triggered by an alarm
 * that sets event 2.
 */
#if 0
void main(void){
	int i;
	ME.MER.R = 0x0000001D;          /* Enable DRUN, RUN0, SAFE, RESET modes */
		                                  /* Initialize PLL before turning it on: */
		  //CGM.FMPLL_CR.R = 0x02400100;    /* 8 MHz xtal: Set PLL0 to 64 MHz */
		  ME.RUN[0].R = 0x001F0074;       /* RUN0 cfg: 16MHzIRCON,OSC0ON,PLL0ON,syclk=PLL */
		  ME.RUNPC[1].R = 0x00000010; 	  /* Peri. Cfg. 1 settings: only run in RUN0 mode */
		  ME.PCTL[4].R = 0x01;            /* MPC56xxB/P/S DSPI0:  select ME.RUNPC[1] */
		  ME.PCTL[5].R = 0x01;            /* MPC56xxB/P/S DSPI1:  select ME.RUNPC[1] */
		  ME.PCTL[68].R = 0x01;           /* MPC56xxB/S SIUL:  select ME.RUNPC[0] */
		                                  /* Mode Transition to enter RUN0 mode: */
		  ME.MCTL.R = 0x40005AF0;         /* Enter RUN0 Mode & Key */
		  ME.MCTL.R = 0x4000A50F;         /* Enter RUN0 Mode & Inverted Key */
		  while (ME.GS.B.S_MTRANS) {}     /* Wait for mode transition to complete */
		                                  /* Note: could wait here using timer and/or I_TC IRQ */
		  while(ME.GS.B.S_CURRENTMODE != 4) {} /* Verify RUN0 is the current mode */

	SIU.PCR[69].R = 0x200;
	for(i=0;i<100000;i++)
	{
		if(i==99999)
		{
		SIU.GPDO[69].B.PDO = ~SIU.GPDO[69].B.PDO;
		i = 0;
		}
		else;
	}
	while(1);
}
#endif
void etask_1( void ) {
	volatile float tryFloatingPoint = 0.0F;
	StackInfoType si;
	TaskType currTask;

	//EcuM_StartupTwo();
	LDEBUG_FPUTS("etask_1 start\n");
	for(;;) {
		for(i=0;i<300000;i++);
		SIU.GPDO[69].B.PDO = ~SIU.GPDO[69].B.PDO;
		SetEvent(TASK_ID_etask_2,EVENT_MASK_EVENT_1);
		WaitEvent(EVENT_MASK_EVENT_2);
		ClearEvent(EVENT_MASK_EVENT_2);
		tryFloatingPoint += 1.0F;
		GetTaskID(&currTask);
		Os_Arc_GetStackInfo(currTask,&si);
		LDEBUG_PRINTF("etask_1: %u%% stack usage\n",
				(unsigned)OS_STACK_USAGE(&si));

	}
}

/**
 * An extended task that receives events from someone
 * and activates task: btask_3.
 */
void etask_2( void ) {

	LDEBUG_FPUTS("etask_2 start\n");
	Test_Gpt();
	SIU.PCR[69].B.OBE = 1;

	for(;;) {
		for(i=0;i<300000;i++);
		SIU.GPDO[69].B.PDO = ~SIU.GPDO[69].B.PDO;
		WaitEvent(EVENT_MASK_EVENT_1);
		ClearEvent(EVENT_MASK_EVENT_1);
		ActivateTask(TASK_ID_btask_3);
		{
			StackInfoType si;
			TaskType currTask;
			GetTaskID(&currTask);
			Os_Arc_GetStackInfo(currTask,&si);
			LDEBUG_PRINTF("etask_2: %u%% stack usage\n",
					(unsigned)OS_STACK_USAGE(&si));
		}
	}


}


/*
 * Functions that must be supplied by the example
 */
//extern OsTickType OsTickFreq;
//extern const OsTickType OsTickFreq ;
//long xyyy __attribute__((section(".rodata"))) = 1000;
//long abc = 0;
void OsIdle( void ) {

int i;
//OsTickFreq++;
		//INTC.PSR[4].R = 2;
		//INTC.SSCIR[4].R = 2;
//abc = xyyy;
while(1){
		//INTC.CPR.B.PRI = 0;
		//PIT.CH[1].TFLG.B.TIF = 1;
		//INTC.PSR[3].R = 2;
		//INTC.SSCIR[3].R = 2;
	//abc++;

}
for(i=0;i<100000;i++){}

	INTC.PSR[4].R = 2;
	INTC.SSCIR[4].R = 2;
while(1);
}


/* Global hooks */
ProtectionReturnType ProtectionHook( StatusType FatalError ) {
	LDEBUG_FPUTS("## ProtectionHook\n");
	return PRO_KILLAPPL;
}

void StartupHook( void ) {
	LDEBUG_FPUTS("## StartupHook\n");

	LDEBUG_PRINTF("Sys clock %u Hz\n",(unsigned)McuE_GetSystemClock());
}

void ShutdownHook( StatusType Error ) {
	LDEBUG_FPUTS("## ShutdownHook\n");
	while(1);
}

struct LogBad_s {
	uint32_t param1;
	uint32_t param2;
	uint32_t param3;
	TaskType taskId;
	OsServiceIdType serviceId;
	StatusType error;
};

void ErrorHook( StatusType Error ) {

	TaskType task;
	static struct LogBad_s LogBad[ERROR_LOG_SIZE];
	static uint8_t ErrorCount = 0;

	GetTaskID(&task);


	OsServiceIdType service = OSErrorGetServiceId();

	/* Grab the arguments to the functions
	 * This is the standard way, see 11.2 in OSEK spec
	 */
	switch(service) {
	case OSServiceId_SetRelAlarm:
	{
		// Read the arguments to the faulty functions...
		AlarmType alarm_id = OSError_SetRelAlarm_AlarmId;
		TickType increment = OSError_SetRelAlarm_Increment;
		TickType cycle = OSError_SetRelAlarm_Cycle;
		(void)alarm_id;
		(void)increment;
		(void)cycle;

		// ... Handle this some way.
		break;
	}
	/*
	 * The same pattern as above applies for all other OS functions.
	 * See Os.h for names and definitions.
	 */

	default:
		break;
	}

	LDEBUG_PRINTF("## ErrorHook err=%u\n",Error);

	/* Log the errors in a buffer for later review */
	LogBad[ErrorCount].param1 = os_error.param1;
	LogBad[ErrorCount].param2 = os_error.param2;
	LogBad[ErrorCount].param3 = os_error.param3;
	LogBad[ErrorCount].serviceId = service;
	LogBad[ErrorCount].taskId = task;
	LogBad[ErrorCount].error = Error;

	ErrorCount++;

	// Stall if buffer is full.
	while(ErrorCount >= ERROR_LOG_SIZE);
}

void PreTaskHook( void ) {
	TaskType task;
	GetTaskID(&task);
//	LDEBUG_PRINTF("## PreTaskHook, taskid=%u\n",task);
}

void PostTaskHook( void ) {
	TaskType task;
	GetTaskID(&task);
//	LDEBUG_PRINTF("## PostTaskHook, taskid=%u\n",task);
}

